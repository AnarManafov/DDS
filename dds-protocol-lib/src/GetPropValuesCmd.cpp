// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "GetPropValuesCmd.h"
#include <stdexcept>
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SGetPropValuesCmd::SGetPropValuesCmd()
    : m_sPropertyID()
{
}
size_t SGetPropValuesCmd::size() const
{
    size_t s = (m_sPropertyID.size() + sizeof(uint32_t));
    return s;
}

bool SGetPropValuesCmd::operator==(const SGetPropValuesCmd& val) const
{
    return (m_sPropertyID == val.m_sPropertyID);
}

void SGetPropValuesCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_sPropertyID);
}

void SGetPropValuesCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_sPropertyID);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SGetPropValuesCmd& val)
{
    return _stream << "propertyID: " << val.m_sPropertyID;
}

bool dds::protocol_api::operator!=(const SGetPropValuesCmd& lhs, const SGetPropValuesCmd& rhs)
{
    return !(lhs == rhs);
}