// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__ConnectionManager__
#define __DDS__ConnectionManager__
// DDS
#include "AgentChannel.h"
#include "ConnectionManagerImpl.h"
#include "Options.h"
#include "SSHScheduler.h"
#include "Topology.h"
#include "UIChannelInfo.h"
// STD
#include <condition_variable>
#include <mutex>

namespace dds
{
    namespace commander_cmd
    {
        class CConnectionManager : public protocol_api::CConnectionManagerImpl<CAgentChannel, CConnectionManager>,
                                   public std::enable_shared_from_this<CConnectionManager>
        {
          public:
            CConnectionManager(const SOptions_t& _options);

            ~CConnectionManager();

          public:
            void newClientCreated(CAgentChannel::connectionPtr_t _newClient);
            void _start();
            void _stop();
            void _createInfoFile(const std::vector<size_t>& _ports) const;
            void _deleteInfoFile() const;

          private:
            void on_cmdGET_AGENTS_INFO(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_AGENTS_INFO>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdGET_LOG(const protocol_api::SSenderInfo& _sender,
                               protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_LOG>::ptr_t _attachment,
                               CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdBINARY_ATTACHMENT_RECEIVED(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdSUBMIT(const protocol_api::SSenderInfo& _sender,
                              protocol_api::SCommandAttachmentImpl<protocol_api::cmdSUBMIT>::ptr_t _attachment,
                              CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdTRANSPORT_TEST(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdTRANSPORT_TEST>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdREPLY(const protocol_api::SSenderInfo& _sender,
                             protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY>::ptr_t _attachment,
                             CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdUPDATE_KEY(const protocol_api::SSenderInfo& _sender,
                                  protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment,
                                  CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdUSER_TASK_DONE(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUSER_TASK_DONE>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdGET_PROP_LIST(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_PROP_LIST>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdGET_PROP_VALUES(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_PROP_VALUES>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdUPDATE_TOPOLOGY(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_TOPOLOGY>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdREPLY_ID(const protocol_api::SSenderInfo& _sender,
                                protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_ID>::ptr_t _attachment,
                                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdENABLE_STAT(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdENABLE_STAT>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdDISABLE_STAT(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdDISABLE_STAT>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdGET_STAT(const protocol_api::SSenderInfo& _sender,
                                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_STAT>::ptr_t _attachment,
                                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdCUSTOM_CMD(const protocol_api::SSenderInfo& _sender,
                                  protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment,
                                  CAgentChannel::weakConnectionPtr_t _channel);

          private:
            void parseExe(const std::string& _exeStr,
                          std::string& _filePath,
                          std::string& _filename,
                          std::string& _cmdStr);

            template <protocol_api::ECmdType _cmd, class... Args>
            void broadcastUpdateTopologyAndWait(weakChannelInfo_t::container_t agents,
                                                CAgentChannel::weakConnectionPtr_t _channel,
                                                const std::string& _msg,
                                                Args&&... args);
            template <protocol_api::ECmdType _cmd>
            void broadcastUpdateTopologyAndWait_impl(size_t index, weakChannelInfo_t _agent);
            template <protocol_api::ECmdType _cmd>
            void broadcastUpdateTopologyAndWait_impl(
                size_t index,
                weakChannelInfo_t _agent,
                typename protocol_api::SCommandAttachmentImpl<_cmd>::ptr_t _attachment);
            template <protocol_api::ECmdType _cmd>
            void broadcastUpdateTopologyAndWait_impl(size_t index,
                                                     weakChannelInfo_t _agent,
                                                     const std::string& _filePath,
                                                     const std::string& _filename);
            template <protocol_api::ECmdType _cmd>
            void broadcastUpdateTopologyAndWait_impl(size_t index,
                                                     weakChannelInfo_t _agent,
                                                     const std::vector<std::string>& _filePaths,
                                                     const std::vector<std::string>& _filenames);
            template <protocol_api::ECmdType _cmd>
            void broadcastUpdateTopologyAndWait_impl(
                size_t index,
                weakChannelInfo_t _agent,
                const std::vector<typename protocol_api::SCommandAttachmentImpl<_cmd>::ptr_t>& _attachments);

            void activateTasks(const CSSHScheduler& _scheduler, CAgentChannel::weakConnectionPtr_t _channel);
            void enableDisableStatForChannels(bool _enable);
            void _createWnPkg(bool _needInlineBashScript) const;

          private:
            CGetLogChannelInfo m_getLog;
            CTestChannelInfo m_transportTest;
            CUpdateTopologyChannelInfo m_updateTopology;
            CSubmitAgentsChannelInfo m_SubmitAgents;
            topology_api::CTopology m_topo;

            // TODO: This is temporary storage only. Store this information as a part of scheduler.
            typedef std::map<uint64_t, weakChannelInfo_t> TaskIDToAgentChannelMap_t;
            TaskIDToAgentChannelMap_t m_taskIDToAgentChannelMap;
            std::mutex m_mapMutex;

            std::mutex m_updateTopoMutex;
            std::condition_variable m_updateTopoCondition;

            // Statistic on/off flag
            bool m_statEnabled;
        };
    } // namespace commander_cmd
} // namespace dds
#endif /* defined(__DDS__ConnectionManager__) */
