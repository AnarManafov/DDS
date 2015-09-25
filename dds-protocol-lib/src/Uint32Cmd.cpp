// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#include "Uint32Cmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

void ::SUint32Cmd::normalizeToLocal() const
{
    m_nUint32 = inet::normalizeRead(m_nUint32);
}

void SUint32Cmd::normalizeToRemote() const
{
    m_nUint32 = inet::normalizeWrite(m_nUint32);
}

void SUint32Cmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "Int32Cmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    inet::readData(&m_nUint32, &_data, &idx);
}

void SUint32Cmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    inet::pushData(m_nUint32, _data);
}
