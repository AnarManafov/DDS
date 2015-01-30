// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef PROTOCOLCOMMANDS_H_
#define PROTOCOLCOMMANDS_H_
// MiscCommon
#include "def.h"

#define NAME_TO_STRING(NAME) #NAME

// define current protocol version version
//
// We wanted to track protocol changes by versions, but at the moment it is a kinda overkill.
// At the moment it doesn't matter which versions peers have, once the version is different, we break the connection.
// In the future we might want to support backward compatibility. In this case protocol version, command will be
// organized in separate structures and enums.
//
const uint16_t g_protocolCommandsVersion = 2;

namespace dds
{
    enum ECmdType
    {
        cmdUNKNOWN = -1,
        cmdSHUTDOWN = 1,
        cmdHANDSHAKE,  // attachment: SVersionCmd
        cmdSUBMIT,     // attachment: SSubmitCmd
        cmdSIMPLE_MSG, // attachment: SSimpleMsgCmd
        cmdREPLY_HANDSHAKE_OK,
        cmdREPLY_HANDSHAKE_ERR, // attachment: SSimpleMsgCmd
        cmdGET_HOST_INFO,
        cmdREPLY_HOST_INFO, // attachment: SHostInfoCmd
        cmdDISCONNECT,
        cmdGED_PID,
        cmdREPLY_PID,                  // attachment: SSimpleMsgCmd. The message contains the pid of the responder.
        cmdBINARY_ATTACHMENT,          // attachment: SBinanryAttachmentCmd. The message contains binary attachment.
        cmdBINARY_ATTACHMENT_RECEIVED, // attachment: SBinaryAttachmentReceivedCmd.
        cmdBINARY_ATTACHMENT_START,    // attachment: SBinaryAttachmentStartCmd.
        cmdGET_UUID,
        cmdREPLY_UUID, // attachment: SUUIDCmd
        cmdSET_UUID,   // attachment: SUUIDCmd
        cmdGET_LOG,
        cmdGET_AGENTS_INFO,
        cmdREPLY_AGENTS_INFO, // attachment: SAgentsInfoCmd
        cmdASSIGN_USER_TASK,  // attachment: SAssignUserTaskCmd
        cmdACTIVATE_AGENT,    // this command activates a given agent and triggers a start of an assigned user task
        cmdTRANSPORT_TEST,
        cmdUPDATE_KEY, // attachment: SUpdateKeyCmd
        cmdPROGRESS    // attachment: SProgressCmd
    };

    static std::map<uint16_t, std::string> g_cmdToString{
        { cmdUNKNOWN, NAME_TO_STRING(cmdUNKNOWN) },
        { cmdSHUTDOWN, NAME_TO_STRING(cmdSHUTDOWN) },
        { cmdHANDSHAKE, NAME_TO_STRING(cmdHANDSHAKE) },
        { cmdSUBMIT, NAME_TO_STRING(cmdSUBMIT) },
        { cmdSIMPLE_MSG, NAME_TO_STRING(cmdSIMPLE_MSG) },
        { cmdREPLY_HANDSHAKE_OK, NAME_TO_STRING(cmdREPLY_HANDSHAKE_OK) },
        { cmdREPLY_HANDSHAKE_ERR, NAME_TO_STRING(cmdREPLY_HANDSHAKE_ERR) },
        { cmdGET_HOST_INFO, NAME_TO_STRING(cmdGET_HOST_INFO) },
        { cmdREPLY_HOST_INFO, NAME_TO_STRING(cmdREPLY_HOST_INFO) },
        { cmdDISCONNECT, NAME_TO_STRING(cmdDISCONNECT) },
        { cmdGED_PID, NAME_TO_STRING(cmdGED_PID) },
        { cmdREPLY_PID, NAME_TO_STRING(cmdREPLY_PID) },
        { cmdBINARY_ATTACHMENT, NAME_TO_STRING(cmdBINARY_ATTACHMENT) },
        { cmdBINARY_ATTACHMENT_RECEIVED, NAME_TO_STRING(cmdBINARY_ATTACHMENT_RECEIVED) },
        { cmdBINARY_ATTACHMENT_START, NAME_TO_STRING(cmdBINARY_ATTACHMENT_START) },
        { cmdGET_UUID, NAME_TO_STRING(cmdGET_UUID) },
        { cmdREPLY_UUID, NAME_TO_STRING(cmdREPLY_UUID) },
        { cmdSET_UUID, NAME_TO_STRING(cmdSET_UUID) },
        { cmdGET_LOG, NAME_TO_STRING(cmdGET_LOG) },
        { cmdGET_AGENTS_INFO, NAME_TO_STRING(cmdGET_AGENTS_INFO) },
        { cmdREPLY_AGENTS_INFO, NAME_TO_STRING(cmdREPLY_AGENTS_INFO) },
        { cmdASSIGN_USER_TASK, NAME_TO_STRING(cmdASSIGN_USER_TASK) },
        { cmdACTIVATE_AGENT, NAME_TO_STRING(cmdACTIVATE_AGENT) },
        { cmdTRANSPORT_TEST, NAME_TO_STRING(cmdTRANSPORT_TEST) },
        { cmdUPDATE_KEY, NAME_TO_STRING(cmdUPDATE_KEY) },
        { cmdPROGRESS, NAME_TO_STRING(cmdPROGRESS) }
    };
}

#endif /* PROTOCOLMESSAGES_H_ */
