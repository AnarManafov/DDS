// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__MasterAgent__
#define __DDS__MasterAgent__

// DDS
#include "CommandAttachmentImpl.h"
#include "AgentChannel.h"

namespace dds
{
    class CMasterAgent
    {
        typedef std::list<boost::uuids::uuid> IDVector_t;
        typedef std::map<std::string, IDVector_t> HostToAgentIDMap_t;

      public:
        bool addAgent(CAgentChannel::weakConnectionPtr_t _channel);
        boost::uuids::uuid getMasterAgent(const std::string& _host) throw(std::runtime_error);

      private:
        // TODO: Use SQLite back-end when avaliable
        HostToAgentIDMap_t m_HostToAgent;
        std::mutex m_mutex;
    };
}

#endif /* defined(__DDS__MasterAgent__) */
