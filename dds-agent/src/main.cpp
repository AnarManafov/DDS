// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentConnectionManager.h"
#include "IntercomServiceCore.h"
#include "Logger.h"
#include "Options.h"
#include "SessionIDFile.h"
#include "UserDefaults.h"

using namespace std;
using namespace dds;
using namespace dds::agent_cmd;
using namespace dds::user_defaults_api;
using namespace dds::misc;
namespace bi = boost::interprocess;
namespace bf = boost::filesystem;

void clean()
{
    // Cleaning shared memory of agent's shared memory channel
    const CUserDefaults& userDefaults = CUserDefaults::instance();
    for (const auto& inputName : userDefaults.getSMLeaderInputNames())
    {
        const bool inputRemoved = bi::message_queue::remove(inputName.c_str());
        LOG(info) << "Message queue " << inputName << " remove status: " << inputRemoved;
    }

    bf::path pathWrkDir(userDefaults.getSlotsRootDir());
    for (auto& dir : boost::make_iterator_range(bf::directory_iterator(pathWrkDir), {}))
    {
        try
        {
            if (bf::is_directory(dir.path()))
            {
                string filename = dir.path().filename().string();
                uint64_t slotID = stoull(filename);
                const string outputName(userDefaults.getSMLeaderOutputName(slotID));
                const bool outputRemoved = bi::message_queue::remove(outputName.c_str());
                LOG(info) << "Message queue " << outputName << " remove status: " << outputRemoved;
            }
        }
        catch (...)
        {
            continue;
        }
    }
}

int main(int argc, char* argv[])
{
    // Command line parser
    SOptions_t options;
    try
    {
        CUserDefaults::instance(); // Initialize user defaults
        Logger::instance().init(); // Initialize log

        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
        return EXIT_FAILURE;
    }

    const string pidfile_name(CUserDefaults::instance().getAgentPidFile());

    // Checking for "start" option
    if (SOptions_t::cmd_start == options.m_Command)
    {
        try
        {
            CPIDFile pidfile(pidfile_name, ::getpid());
            CSessionIDFile sid(dds::user_defaults_api::CUserDefaults::instance().getSIDFile());
            LOG(info) << "SESSION ID file: " << dds::user_defaults_api::CUserDefaults::instance().getSIDFile();
            if (sid.getLockedSID().empty())
            {
                LOG(fatal) << "Failed to create session ID. Stopping the session...";
                return EXIT_FAILURE;
            }
            LOG(info) << "SESSION ID: " << sid.getLockedSID();

            // Export DDS session ID
            if (::setenv("DDS_SESSION_ID", sid.getLockedSID().c_str(), 1) == -1)
                throw dds::misc::system_error("Failed to set up $DDS_SESSION_ID");

            shared_ptr<CAgentConnectionManager> agentptr = make_shared<CAgentConnectionManager>(options);
            agentptr->start();
        }
        catch (exception& e)
        {
            LOG(fatal) << e.what();
            return EXIT_FAILURE;
        }
        catch (...)
        {
            LOG(fatal) << "Unexpected Exception occurred.";
            return EXIT_FAILURE;
        }
    }

    // Checking for "clean" option
    if (SOptions_t::cmd_clean == options.m_Command)
    {
        try
        {
            dds::internal_api::CIntercomServiceCore::clean();
            clean();
        }
        catch (exception& e)
        {
            LOG(fatal) << e.what();
            return EXIT_FAILURE;
        }
        catch (...)
        {
            LOG(fatal) << "Unexpected Exception occurred.";
            return EXIT_FAILURE;
        }
    }

    // Checking for "check" option
    if (SOptions_t::cmd_check == options.m_Command)
    {
        try
        {
            const string agentIdFile(CUserDefaults::instance().getAgentIDFilePath());
            // max wait interval is 10*100 ms = 5 sec
            for (int i = 0; i < 50; ++i)
            {
                ifstream f(pidfile_name);
                if (!f.is_open())
                {
                    this_thread::sleep_for(chrono::milliseconds(100));
                    continue;
                }
                pid_t nAgentPid{ 0 };
                f >> nAgentPid;
                const bool bRunning = IsProcessRunning(nAgentPid);
                // if process running, let's check it recieved a client ID from the commander
                if (bRunning)
                {
                    ifstream fIdFile(agentIdFile);
                    if(!fIdFile.is_open())
                    {
                        this_thread::sleep_for(chrono::milliseconds(100));
                        continue;
                    }
                    uint64_t nAgentID{ 0 };
                    fIdFile >> nAgentID;
                    return (nAgentID > 0 ? EXIT_SUCCESS : EXIT_FAILURE);
                }
            }
        }
        catch (exception& e)
        {
            LOG(fatal) << e.what();
            return EXIT_FAILURE;
        }
        catch (...)
        {
            LOG(fatal) << "Unexpected Exception occurred.";
            return EXIT_FAILURE;
        }
    }

    LOG(info) << "DDS agent is Done. Bye, Bye!";

    return EXIT_SUCCESS;
}
