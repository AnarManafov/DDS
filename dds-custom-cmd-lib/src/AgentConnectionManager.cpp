// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
// silence "Unused typedef" warning using clang 3.7+ and boost < 1.59
#if BOOST_VERSION < 105900
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/property_tree/ini_parser.hpp>
#if BOOST_VERSION < 105900
#pragma clang diagnostic pop
#endif

// DDS
#include "AgentConnectionManager.h"
#include "Logger.h"
#include "MonitoringThread.h"

using namespace boost::asio;
using namespace std;
using namespace dds::user_defaults_api;
using namespace dds::custom_cmd_api;
using namespace dds::protocol_api;
using namespace MiscCommon;
namespace sp = std::placeholders;
namespace fs = boost::filesystem;
using boost::asio::ip::tcp;

CAgentConnectionManager::CAgentConnectionManager()
    : m_syncHelper(nullptr)
    , m_bStarted(false)
{
}

CAgentConnectionManager::~CAgentConnectionManager()
{
    stop();
}

void CAgentConnectionManager::start()
{
    if (m_bStarted)
        return;

    m_bStarted = true;
    try
    {
        // Read server info file
        // First check if agent info file exists.
        // If it does not exist we take the commander server info file.
        const string sAgentCfg(CUserDefaults::instance().getAgentInfoFileLocation());
        const string sCommanderCfg(CUserDefaults::instance().getServerInfoFileLocation());
        string sHost;
        string sPort;
        EChannelType channelType(EChannelType::UNKNOWN);
        if (fs::exists(sAgentCfg))
        {
            LOG(info) << "Reading server info from: " << sAgentCfg;
            boost::property_tree::ptree pt;
            boost::property_tree::ini_parser::read_ini(sAgentCfg, pt);
            sHost = pt.get<string>("agent.host");
            sPort = pt.get<string>("agent.port");
            channelType = EChannelType::CUSTOM_CMD_GUARD;
        }
        else if (fs::exists(sCommanderCfg))
        {
            LOG(info) << "Reading server info from: " << sCommanderCfg;
            boost::property_tree::ptree pt;
            boost::property_tree::ini_parser::read_ini(sCommanderCfg, pt);
            sHost = pt.get<string>("ui.host");
            sPort = pt.get<string>("ui.port");
            channelType = EChannelType::UI;
        }
        else
        {
            throw runtime_error("Cannot find agent or commander info file.");
        }

        LOG(info) << "Contacting DDS agent on " << sHost << ":" << sPort;

        // Resolve endpoint iterator from host and port
        tcp::resolver resolver(m_service);
        tcp::resolver::query query(sHost, sPort);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Create new communication channel and push handshake message
        m_channel = CAgentChannel::makeNew(m_service);
        m_channel->setChannelType(channelType);
        // Subscribe to Shutdown command
        std::function<bool(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, CAgentChannel * _channel)>
            fSHUTDOWN = [this](SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, CAgentChannel* _channel) -> bool
        {
            // TODO: adjust the algorithm if we would need to support several agents
            // we have only one agent (newAgent) at the moment
            return this->on_cmdSHUTDOWN(_attachment, m_channel);
        };
        m_channel->registerMessageHandler<cmdSHUTDOWN>(fSHUTDOWN);

        m_channel->subscribeOnEvent(EChannelEvents::OnConnected,
                                    [this](CAgentChannel* _channel)
                                    {
                                        m_channel->m_syncHelper = m_syncHelper;
                                    });
        m_channel->connect(endpoint_iterator);

        // Don't block main thread, start transport service on a thread-pool
        const int nConcurrentThreads(2);
        LOG(MiscCommon::info) << "Starting DDS transport engine using " << nConcurrentThreads << " concurrent threads.";
        for (int x = 0; x < nConcurrentThreads; ++x)
        {
            m_workerThreads.create_thread(boost::bind(&boost::asio::io_service::run, &(m_service)));
        }
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
}

void CAgentConnectionManager::stop()
{
    if (!m_bStarted)
        return;

    m_bStarted = false;

    LOG(info) << "Shutting down DDS transport...";

    try
    {
        m_service.stop();
        m_workerThreads.join_all();
        if (!getAgentChannel().expired())
        {
            auto p = getAgentChannel().lock();
            p->stop();
        }
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
    LOG(info) << "Shutting down DDS transport - DONE";
}

bool CAgentConnectionManager::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                                             CAgentChannel::weakConnectionPtr_t _channel)
{
    stop();
    return true;
}

int CAgentConnectionManager::sendCmd(const protocol_api::SCustomCmdCmd& _command)
{
    try
    {
        if (getAgentChannel().expired())
            throw runtime_error("Agent channel is offline");

        auto p = getAgentChannel().lock();
        p->pushMsg<cmdCUSTOM_CMD>(_command);
        return 0;
    }
    catch (const exception& _e)
    {
        LOG(fatal) << "Fail to push the custom command: " << _command << "; Error: " << _e.what();
    }
    LOG(fatal) << "Fail to push the custom command: " << _command;
    return 1;
}
