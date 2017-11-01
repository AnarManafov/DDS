// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CSMUIChannel__
#define __DDS__CSMUIChannel__
// DDS
#include "BaseSMChannelImpl.h"

namespace dds
{
    class CSMUIChannel : public protocol_api::CBaseSMChannelImpl<CSMUIChannel>
    {
      protected:
        CSMUIChannel(boost::asio::io_service& _service,
                     const std::string& _inputName,
                     const std::string& _outputName,
                     uint64_t _protocolHeaderID,
                     protocol_api::EMQOpenType _inputOpenType,
                     protocol_api::EMQOpenType _outputOpenType);

      public:
        ~CSMUIChannel();

      public:
        SM_RAW_MESSAGE_HANDLER(CSMUIChannel, on_rawMessage)

      private:
        bool on_rawMessage(protocol_api::CProtocolMessage::protocolMessagePtr_t _currentMsg,
                           const protocol_api::SSenderInfo& _sender);
    };
}
#endif
