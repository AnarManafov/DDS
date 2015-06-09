// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "MasterAgent.h"

using namespace dds;
using namespace std;

bool CMasterAgent::addAgent(CAgentChannel::weakConnectionPtr_t _channel)
{
    auto p = _channel.lock();
    const boost::uuids::uuid id = p->getId();
    const string sAgentHost = p->getRemoteHostInfo().m_host;

    lock_guard<mutex> lock(m_mutex);
    auto found = m_HostToAgent.find(sAgentHost);
    if (found != m_HostToAgent.end())
        found->second.push_back(id);
    else
    {
        IDVector_t vec = { id };
        m_HostToAgent.insert(make_pair(sAgentHost, vec));
    }

    return true;
}

boost::uuids::uuid CMasterAgent::getMasterAgent(const std::string& _host) throw(std::runtime_error)
{
    lock_guard<mutex> lock(m_mutex);
    auto found = m_HostToAgent.find(_host);
    if (found == m_HostToAgent.end())
        throw runtime_error("there are no agents on host: " + _host);

    // the master agent is always the first agent in the list
    return found->second.front();
}