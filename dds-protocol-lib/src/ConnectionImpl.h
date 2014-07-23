// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__Connection__
#define __DDS__Connection__
// STD
#include <iostream>
#include <deque>
// BOOST
#include "boost/noncopyable.hpp"
#include "boost/asio.hpp"
// DDS
#include "ProtocolMessage.h"
#include "ProtocolCommands.h"
#include "Logger.h"

#define BEGIN_MSG_MAP(theClass)                                          \
  public:                                                                \
    friend CConnectionImpl<theClass>;                                    \
    void processMessage(const CProtocolMessage& _currentMsg, int& _nRes) \
    {                                                                    \
        switch (_currentMsg.header().m_cmd)                              \
        {

#define MESSAGE_HANDLER(msg, func) \
    case msg:                      \
        _nRes = func(_currentMsg); \
        break;

#define END_MSG_MAP()                                                                                        \
    default:                                                                                                 \
        LOG(MiscCommon::error) << "The received message doesn't have a handler: " << _currentMsg.toString(); \
        }                                                                                                    \
        }

namespace dds
{
    template <class T>
    class CConnectionImpl : public boost::noncopyable
    {
      protected:
        CConnectionImpl<T>(boost::asio::io_service& _service)
            : m_io_service(_service)
            , m_socket(_service)
            , m_started(false)
            , m_currentMsg()
            , m_outputMessageQueue()
        {
        }

      public:
        ~CConnectionImpl<T>()
        {
            close();
        }

      public:
        typedef std::shared_ptr<T> connectionPtr_t;
        typedef std::vector<connectionPtr_t> connectionPtrVector_t;
        typedef std::deque<CProtocolMessage> messageQueue_t;

      public:
        static connectionPtr_t makeNew(boost::asio::io_service& _service)
        {
            connectionPtr_t newObject(new T(_service));
            return newObject;
        }

      public:
        void connect(boost::asio::ip::tcp::resolver::iterator _endpoint_iterator)
        {
            doConnect(_endpoint_iterator);
        }

        void start()
        {
            m_started = true;

            if (m_outputMessageQueue.empty())
                readHeader();
            else
                send();
        }

        void stop()
        {
            if (!m_started)
                return;
            m_started = false;
            m_socket.close();
        }

        void send()
        {
            //      writeMessages();
        }

        void close()
        {
            m_io_service.post([this]()
                              { m_socket.close(); });
        }

        boost::asio::ip::tcp::socket& socket()
        {
            return m_socket;
        }

        void pushMsg(const CProtocolMessage& _msg)
        {
            m_io_service.post([this, _msg]()
                              {
                                  bool write_in_progress = !m_outputMessageQueue.empty();
                                  m_outputMessageQueue.push_back(_msg);
                                  if (!write_in_progress)
                                  {
                                      writeMessages();
                                  }
                              });
            // m_outputMessageQueue.push_back(_msg);
        }

      private:
        void doConnect(boost::asio::ip::tcp::resolver::iterator _endpoint_iterator)
        {
            boost::asio::async_connect(m_socket,
                                       _endpoint_iterator,
                                       [this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
                                       {
                if (!ec)
                {
                    LOG(MiscCommon::log_stdout) << "Connection established.";
                    start();
                }
                else
                {
                    LOG(MiscCommon::log_stderr) << "Failed to connect: " << ec.message();
                }
            });
        }

        void readHeader()
        {
            m_currentMsg.clear();

            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_currentMsg.data(), CProtocolMessage::header_length),
                                    [this](boost::system::error_code ec, std::size_t /*length*/)
                                    {
                if (!ec && m_currentMsg.decode_header())
                {
                    // If the header is ok, recieve the body of the message
                    readBody();
                }
                else
                {
                    stop();
                }
            });
        }

        void readBody()
        {
            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_currentMsg.body(), m_currentMsg.body_length()),
                                    [this](boost::system::error_code ec, std::size_t /*length*/)
                                    {
                if (!ec)
                {
                    LOG(MiscCommon::debug) << "Received from Agent: " << m_currentMsg.toString();
                    // processe recieved message
                    int nRes(0);
                    T* pThis = static_cast<T*>(this);
                    pThis->processMessage(m_currentMsg, nRes);
                    // Read next message
                    readHeader();
                }
                else
                {
                    stop();
                }
            });
        }

        void writeMessages()
        {
            LOG(MiscCommon::debug) << "Sending message: " << m_outputMessageQueue.front().toString();
            boost::asio::async_write(
                m_socket,
                boost::asio::buffer(m_outputMessageQueue.front().data(), m_outputMessageQueue.front().length()),
                [this](boost::system::error_code ec, std::size_t /*_bytesTransferred*/)
                {
                    if (!ec)
                    {
                        LOG(MiscCommon::debug) << "Data successfully sent";
                        m_outputMessageQueue.pop_front();
                        if (!m_outputMessageQueue.empty())
                        {
                            writeMessages();
                        }
                        else
                        {
                            // If there is no notghing to send, we return to read
                            readHeader();
                        }
                    }
                    else
                    {
                        LOG(MiscCommon::error) << "Error sending data: " << ec.message();
                        m_socket.close();
                    }
                });
        }

      private:
        boost::asio::io_service& m_io_service;
        boost::asio::ip::tcp::socket m_socket;
        bool m_started;
        CProtocolMessage m_currentMsg;
        messageQueue_t m_outputMessageQueue;
    };
}

#endif /* defined(__DDS__Connection__) */