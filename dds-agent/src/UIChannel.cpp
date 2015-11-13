// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "UIChannel.h"

using namespace std;
using namespace dds;
using namespace dds::agent_cmd;
using namespace dds::protocol_api;
using namespace MiscCommon;

CUIChannel::CUIChannel(boost::asio::io_service& _service)
    : CServerChannelImpl<CUIChannel>(_service, { EChannelType::KEY_VALUE_GUARD, EChannelType::CUSTOM_CMD_GUARD })
{
}

std::string CUIChannel::_remoteEndIDString()
{
    return "key-value-guard";
}

bool CUIChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    return false; // the connection manager should process this message
}

bool CUIChannel::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment)
{
    return false; // the connection manager should process this message
}
