// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ConnectionManagerImpl__
#define __DDS__ConnectionManagerImpl__
// DDS
#include "ChannelInfo.h"
#include "CommandAttachmentImpl.h"
#include "MonitoringThread.h"
#include "Options.h"
#include "ProtocolMessage.h"
// STD
#include <mutex>
// BOOST
#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/thread/thread.hpp>
// MiscCommon
#include "INet.h"

namespace dds
{
    namespace protocol_api
    {
#if BOOST_VERSION >= 107000
        typedef boost::asio::basic_socket_acceptor<boost::asio::ip::tcp, boost::asio::io_context::executor_type>
            asioAcceptor_t;
#else
        typedef boost::asio::basic_socket_acceptor<boost::asio::ip::tcp> asioAcceptor_t;
#endif
        typedef std::shared_ptr<asioAcceptor_t> asioAcceptorPtr_t;

        /// \class CConnectionManagerImpl
        /// \brief Base class for connection managers.
        template <class T, class A>
        class CConnectionManagerImpl : public std::enable_shared_from_this<A>
        {
          public:
            typedef SChannelInfo<T> channelInfo_t;
            typedef SWeakChannelInfo<T> weakChannelInfo_t;
            typedef std::function<bool(const channelInfo_t& _channelInfo, bool& /*_stop*/)> conditionFunction_t;
            using channelContainerCache_t = std::map<uint64_t, weakChannelInfo_t>;

          public:
            CConnectionManagerImpl(size_t _minPort, size_t _maxPort, bool _useUITransport)
                : m_minPort(_minPort)
                , m_maxPort(_maxPort)
                , m_useUITransport(_useUITransport)
            {
                // Create and register signals
                m_signals = std::make_shared<boost::asio::signal_set>(m_ioContext);

                // Register to handle the signals that indicate when the server should exit.
                // It is safe to register for the same signal multiple times in a program,
                // provided all registration for the specified signal is made through Asio.
                m_signals->add(SIGINT);
                m_signals->add(SIGTERM);
#if defined(SIGQUIT)
                m_signals->add(SIGQUIT);
#endif // defined(SIGQUIT)

                m_signals->async_wait(
                    [this](boost::system::error_code /*ec*/, int signo)
                    {
                        // The server is stopped by cancelling all outstanding asynchronous
                        // operations. Once all operations have finished the io_context::run()
                        // call will exit.
                        LOG(dds::misc::info) << "Received a signal: " << signo;
                        LOG(dds::misc::info) << "Stopping DDS transport server...";

                        stop();
                    });
            }

            ~CConnectionManagerImpl()
            {
                // Delete server info file
                deleteInfoFile();
                stop();
            }

            void start(bool _join = true, unsigned int _nThreads = 0 /*0 - auto; min. number is 4*/)
            {
                try
                {
                    // Call _start of the "child"
                    A* pThis = static_cast<A*>(this);
                    pThis->_start();

                    const double maxIdleTime =
                        user_defaults_api::CUserDefaults::instance().getOptions().m_server.m_idleTime;

                    CMonitoringThread::instance().start(maxIdleTime,
                                                        []() { LOG(dds::misc::info) << "Idle callback called."; });

                    bindPortAndListen(m_acceptor);
                    createClientAndStartAccept(m_acceptor);

                    // If we use second channel for communication with UI we have to start accepting connection on that
                    // channel.
                    if (m_useUITransport)
                    {
                        bindPortAndListen(m_acceptorUI);
                        createClientAndStartAccept(m_acceptorUI);
                    }

                    // Create a server info file
                    createInfoFile();

                    // a thread pool for the DDS transport engine
                    // may return 0 when not able to detect
                    unsigned int concurrentThreads = (0 == _nThreads) ? std::thread::hardware_concurrency() : _nThreads;
                    // we need at least 2 threads
                    if (concurrentThreads < 2)
                        concurrentThreads = 2;
                    LOG(dds::misc::info) << "Starting DDS transport engine using " << concurrentThreads
                                         << " concurrent threads.";
                    for (unsigned int x = 0; x < concurrentThreads; ++x)
                    {
                        m_workerThreads.create_thread([this]()
                                                      { runService(10, m_acceptor->get_executor().context()); });
                    }

                    // Starting service for UI transport engine
                    if (m_acceptorUI != nullptr)
                    {
                        const unsigned int concurrentThreads = 2;
                        LOG(dds::misc::info)
                            << "Starting DDS UI transport engine using " << concurrentThreads << " concurrent threads.";
                        for (unsigned int x = 0; x < concurrentThreads; ++x)
                        {
                            m_workerThreads.create_thread([this]()
                                                          { runService(10, m_acceptorUI->get_executor().context()); });
                        }
                    }

                    if (_join)
                        m_workerThreads.join_all();
                }
                catch (std::exception& e)
                {
                    LOG(dds::misc::fatal) << e.what();
                }
            }

            void runService(short _counter, boost::asio::io_context& _io_context)
            {
                if (_counter <= 0)
                {
                    LOG(dds::misc::error) << "CConnectionManagerImpl: can't start another io_context.";
                }
                try
                {
                    _io_context.run();
                }
                catch (std::exception& ex)
                {
                    LOG(dds::misc::error) << "CConnectionManagerImpl exception: " << ex.what();
                    LOG(dds::misc::info) << "CConnectionManagerImpl restarting io_context";
                    runService(--_counter, _io_context);
                }
            }

            void stop()
            {
                try
                {
                    // Call _stop of the "child"
                    A* pThis = static_cast<A*>(this);
                    pThis->_stop();

                    // Send shutdown signal to all client connections.
                    typename weakChannelInfo_t::container_t channels(getChannels());

                    for (const auto& v : channels)
                    {
                        if (v.m_channel.expired())
                            continue;
                        auto ptr = v.m_channel.lock();
                        ptr->template pushMsg<cmdSHUTDOWN>();
                    }

                    auto condition = [](const channelInfo_t& _v, bool& /*_stop*/) { return (_v.m_channel->started()); };

                    size_t counter = 0;
                    while (true)
                    {
                        ++counter;
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        if (countNofChannels(condition) == 0)
                            break;
                        if (counter > 300)
                        {
                            LOG(dds::misc::warning) << "Some channels were not shut down properly. Exiting in anyway.";
                            break;
                        }
                    }

                    m_acceptor->close();
                    m_acceptor->get_executor().context().stop();

                    if (m_acceptorUI != nullptr)
                    {
                        m_acceptorUI->close();
                        m_acceptorUI->get_executor().context().stop();
                    }

                    for (const auto& v : channels)
                    {
                        if (v.m_channel.expired())
                            continue;
                        auto ptr = v.m_channel.lock();
                        ptr->stop();
                    }

                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_channels.clear();
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }
                catch (std::exception& e)
                {
                    LOG(dds::misc::fatal) << e.what();
                }
            }

          protected:
            void updateChannelProtocolHeaderID(const weakChannelInfo_t& _channelInfo)
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                auto p = _channelInfo.m_channel.lock();
                if (p == nullptr)
                    return;

                for (auto& inf : m_channels)
                {
                    if (inf.m_channel.get() == p.get() && inf.m_protocolHeaderID == 0)
                    {
                        inf.m_protocolHeaderID = _channelInfo.m_protocolHeaderID;
                        return;
                    }
                }
                LOG(dds::misc::error) << "Failed to update protocol channel header ID <"
                                      << _channelInfo.m_protocolHeaderID << "> . Channel is not registered";
            }

            weakChannelInfo_t getChannelByID(uint64_t _protocolHeaderID)
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                // Initial creation of the cache
                if (m_channelsCache.empty())
                {
                    for (auto& v : m_channels)
                    {
                        if (v.m_protocolHeaderID == 0)
                            continue;
                        //    commander_cmd::SAgentInfo inf = v.m_channel->getAgentInfo(v.m_protocolHeaderID);
                        m_channelsCache.insert(std::make_pair(
                            v.m_protocolHeaderID, weakChannelInfo_t(v.m_channel, v.m_protocolHeaderID, v.m_isSlot)));
                    }
                }

                // Value found in the cache
                auto it = m_channelsCache.find(_protocolHeaderID);
                if (it != m_channelsCache.end())
                    return it->second;

                for (auto& v : m_channels)
                {
                    //  commander_cmd::SAgentInfo inf = v.m_channel->getAgentInfo(v.m_protocolHeaderID);
                    if (v.m_protocolHeaderID == _protocolHeaderID)
                    {
                        // Add the item into the cache
                        m_channelsCache.insert(std::make_pair(
                            v.m_protocolHeaderID, weakChannelInfo_t(v.m_channel, v.m_protocolHeaderID, v.m_isSlot)));
                        // TODO: need to clean cache from dead channels
                        return weakChannelInfo_t(v.m_channel, v.m_protocolHeaderID, v.m_isSlot);
                    }
                }
                return weakChannelInfo_t();
            }

            typename weakChannelInfo_t::container_t getChannels(conditionFunction_t _condition = nullptr)
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                typename weakChannelInfo_t::container_t result;
                result.reserve(m_channels.size());
                for (auto& v : m_channels)
                {
                    bool stop = false;
                    if (_condition == nullptr || _condition(v, stop))
                    {
                        result.push_back(weakChannelInfo_t(v.m_channel, v.m_protocolHeaderID, v.m_isSlot));
                        if (stop)
                            break;
                    }
                }
                return result;
            }

            template <ECmdType _cmd, class AttachmentType>
            void broadcastMsg(const AttachmentType& _attachment, conditionFunction_t _condition = nullptr)
            {
                try
                {
                    typename weakChannelInfo_t::container_t channels(getChannels(_condition));

                    for (const auto& v : channels)
                    {
                        if (v.m_channel.expired())
                            continue;
                        auto ptr = v.m_channel.lock();
                        ptr->template pushMsg<_cmd>(_attachment, v.m_protocolHeaderID);
                    }
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }
            }

            template <ECmdType _cmd, class AttachmentType>
            void accumulativeBroadcastMsg(const AttachmentType& _attachment, conditionFunction_t _condition = nullptr)
            {
                try
                {
                    typename weakChannelInfo_t::container_t channels(getChannels(_condition));

                    for (const auto& v : channels)
                    {
                        if (v.m_channel.expired())
                            continue;
                        auto ptr = v.m_channel.lock();
                        ptr->template accumulativePushMsg<_cmd>(_attachment, v.m_protocolHeaderID);
                    }
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }
            }

            template <ECmdType _cmd>
            void broadcastSimpleMsg(conditionFunction_t _condition = nullptr)
            {
                SEmptyCmd cmd;
                broadcastMsg<_cmd>(cmd, _condition);
            }

            void broadcastBinaryAttachmentCmd(const std::string& _srcFilePath,
                                              const std::string& _fileName,
                                              uint16_t _cmdSource,
                                              conditionFunction_t _condition = nullptr)
            {
                dds::misc::BYTEVector_t data;

                std::string srcFilePath(_srcFilePath);
                // Resolve environment variables
                dds::misc::smart_path(&srcFilePath);

                std::ifstream f(srcFilePath);
                if (!f.is_open() || !f.good())
                {
                    throw std::runtime_error("Could not open the source file: " + srcFilePath);
                }
                f.seekg(0, std::ios::end);
                data.reserve(f.tellg());
                f.seekg(0, std::ios::beg);
                data.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

                broadcastBinaryAttachmentCmd(data, _fileName, _cmdSource, _condition);
            }

            void broadcastBinaryAttachmentCmd(const dds::misc::BYTEVector_t& _data,
                                              const std::string& _fileName,
                                              uint16_t _cmdSource,
                                              conditionFunction_t _condition = nullptr)
            {
                try
                {
                    typename weakChannelInfo_t::container_t channels(getChannels(_condition));

                    for (const auto& v : channels)
                    {
                        // Post each push call, otherwise it will block other pushes until "post" each block of the
                        // binary file if the file is big.
                        boost::asio::post(this->m_ioContext,
                                          [&]
                                          {
                                              if (v.m_channel.expired())
                                                  return;
                                              auto ptr = v.m_channel.lock();
                                              ptr->pushBinaryAttachmentCmd(
                                                  _data, _fileName, _cmdSource, v.m_protocolHeaderID);
                                          });
                    }
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }
            }

            size_t countNofChannels(conditionFunction_t _condition = nullptr)
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                if (_condition == nullptr)
                    return m_channels.size();
                size_t counter = 0;
                for (auto& v : m_channels)
                {
                    bool stop = false;
                    if (_condition(v, stop))
                    {
                        counter++;
                        if (stop)
                            break;
                    }
                }
                return counter;
            }

            void removeClient(T* _client)
            {
                // TODO: fix getTypeName call
                LOG(dds::misc::debug) << "Removing " /*<< _client->getTypeName()*/
                                      << " client from the list of active";
                std::lock_guard<std::mutex> lock(m_mutex);
                // FIXME: Delete all connections of the channel if the primary protocol header ID is deleted
                m_channels.erase(remove_if(m_channels.begin(),
                                           m_channels.end(),
                                           [&](const channelInfo_t& i) { return (i.m_channel.get() == _client); }),
                                 m_channels.end());

                // Clear the channels cache whenever a client is removed
                // to ensure the cache stays consistent with the actual channels list
                m_channelsCache.clear();
            }

          private:
            static boost::system::error_code _canceled()
            {
                return boost::asio::error::operation_aborted;
            }
            bool _is_canceled(const boost::system::error_code& _ec) const
            {
                return _ec == _canceled();
            }

            void acceptHandler(typename T::connectionPtr_t _client,
                               asioAcceptorPtr_t _acceptor,
                               const boost::system::error_code& _ec)
            {
                if (!_ec)
                {
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        m_channels.push_back(channelInfo_t(_client, _client->getProtocolHeaderID(), false));
                    }
                    _client->start();
                    createClientAndStartAccept(_acceptor);
                }
                else
                {
                    if (_is_canceled(_ec))
                        LOG(dds::misc::info) << "Connection manager is stopping. All operations are canceled.";
                    else
                        LOG(dds::misc::error) << "Can't accept new connection: " << _ec.message();
                }
            }

            void createClientAndStartAccept(asioAcceptorPtr_t& _acceptor)
            {
                typename T::connectionPtr_t newClient = T::makeNew(_acceptor->get_executor().context(), 0);

                A* pThis = static_cast<A*>(this);
                pThis->newClientCreated(newClient);

                // Subscribe on task slot handshake
                newClient->template registerHandler<EChannelEvents::OnReplyAddSlot>(
                    [this, newClient](const SSenderInfo& _sender) -> void
                    {
                        {
                            std::lock_guard<std::mutex> lock(m_mutex);
                            m_channels.push_back(channelInfo_t(newClient, _sender.m_ID, true));
                        }

                        LOG(dds::misc::info)
                            << "Adding new slot to " << newClient->getId() << " with id " << _sender.m_ID;
                    });

                // Subscribe on the disconnect event
                newClient->template registerHandler<EChannelEvents::OnRemoteEndDissconnected>(
                    [this, newClient](const SSenderInfo& /*_sender*/) -> void { this->removeClient(newClient.get()); });

                _acceptor->async_accept(newClient->socket(),
                                        std::bind(&CConnectionManagerImpl::acceptHandler,
                                                  this->shared_from_this(),
                                                  newClient,
                                                  _acceptor,
                                                  std::placeholders::_1));
            }

            void createInfoFile()
            {
                // The child needs to have that method
                A* pThis = static_cast<A*>(this);

                std::vector<size_t> ports;
                ports.push_back(m_acceptor->local_endpoint().port());
                if (m_acceptorUI != nullptr)
                    ports.push_back(m_acceptorUI->local_endpoint().port());

                pThis->_createInfoFile(ports);
            }

            void deleteInfoFile()
            {
                // The child needs to have that method
                A* pThis = static_cast<A*>(this);
                pThis->_deleteInfoFile();
            }

            void bindPortAndListen(asioAcceptorPtr_t& _acceptor)
            {
                const int nMaxCount = 20; // Maximum number of attempts to open the port
                int nCount = 0;
                // Start monitoring thread
                while (true)
                {
                    int nSrvPort =
                        (m_minPort == 0 && m_maxPort == 0) ? 0 : dds::misc::INet::get_free_port(m_minPort, m_maxPort);
                    try
                    {
                        _acceptor = std::make_shared<asioAcceptor_t>(
                            m_ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), nSrvPort));

                        _acceptor->listen();
                    }
                    catch (std::exception& _e)
                    {
                        if (++nCount >= nMaxCount)
                            throw _e;

                        LOG(dds::misc::info) << "Can't bind port " << nSrvPort << ". Will try another port.";
                        // If multiple commanders are started in the same time, then let's give them a chnce to find
                        // a free port.
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        continue;
                    }

                    break;
                }
            }

          private:
            size_t m_minPort;
            size_t m_maxPort;
            bool m_useUITransport;
            /// The signal_set is used to register for process termination notifications.
            std::shared_ptr<boost::asio::signal_set> m_signals;
            std::mutex m_mutex;
            typename channelInfo_t::container_t m_channels;
            channelContainerCache_t m_channelsCache;

            /// Used for the main communication
            boost::asio::io_context m_ioContext;
            asioAcceptorPtr_t m_acceptor;

            // Used for UI (priority) communication
            boost::asio::io_context m_ioContext_UI;
            asioAcceptorPtr_t m_acceptorUI;

            boost::thread_group m_workerThreads;
        };
    } // namespace protocol_api
} // namespace dds
#endif /* defined(__DDS__ConnectionManagerImpl__) */
