// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "TestCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

void STestCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data)
        .get(m_uint16)
        .get(m_uint32)
        .get(m_uint64)
        .get(m_string1)
        .get(m_string2)
        .get(m_vuint16)
        .get(m_vuint32)
        .get(m_vstring1)
        .get(m_string3)
        .get(m_vuint64)
        .get(m_vstring2)
        .get(m_string4);
}

void STestCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data)
        .put(m_uint16)
        .put(m_uint32)
        .put(m_uint64)
        .put(m_string1)
        .put(m_string2)
        .put(m_vuint16)
        .put(m_vuint32)
        .put(m_vstring1)
        .put(m_string3)
        .put(m_vuint64)
        .put(m_vstring2)
        .put(m_string4);
}
