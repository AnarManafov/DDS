// Copyright 2018 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "DDSHelper.h"
#include "ErrorCode.h"
#include "MiscSetup.h"
#include "Options.h"
#include "Start.h"
#include "Stop.h"
// BOOST
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

using namespace std;
using namespace dds::misc;
using namespace dds;
using namespace dds::session_cmd;
using namespace dds::user_defaults_api;
namespace fs = boost::filesystem;

bool IsSessionRunning(const string& _sid)
{
    CUserDefaults::instance().reinit(boost::uuids::string_generator()(_sid), CUserDefaults::instance().currentUDFile());

    return CUserDefaults::instance().IsSessionRunning();
}

void setDefaultSession(const StringVector_t& _sessions, const string& _sessionID)
{
    if (_sessionID.empty())
        throw runtime_error("Default session ID can't be NULL or empty");

    auto found = find(std::begin(_sessions), std::end(_sessions), _sessionID);
    if (found == std::end(_sessions))
    {
        stringstream ss;
        ss << "Failed to set default sessions ID. Bad ID: " << _sessionID;
        throw runtime_error(ss.str());
    }

    CUserDefaults::instance().setDefaultSID(boost::uuids::string_generator()(_sessionID));
}

void rebuildSessions(vector<fs::path>& _session_dirs, StringVector_t& _sessions)
{
    _session_dirs.clear();
    _sessions.clear();

    fs::path pathSessions(CUserDefaults::instance().getSessionsRootDir());
    pathSessions /= CUserDefaults::instance().getSessionsHolderDirName();

    if (!fs::is_directory(pathSessions))
        return; // Sessions holder dir doesn't exists

    for (auto& dir : boost::make_iterator_range(fs::directory_iterator(pathSessions), {}))
    {
        _session_dirs.push_back(dir.path());
        // Workaround: using .leaf().string(), instead of just .leaf(), vecasue we want to avoid double quotes
        // in output.
        _sessions.push_back(dir.path().leaf().string());
    }
}

void listSessions(const vector<fs::path>& _session_dirs, SSessionsSorting::ETypes _sortingType)
{
    // Sort dirs by time
    typedef std::multimap<std::time_t, fs::path> Dirs_t;
    Dirs_t sortedDirs;
    for (auto& dir : _session_dirs)
    {
        if (fs::exists(dir) && fs::is_directory(dir))
        {
            sortedDirs.insert(Dirs_t::value_type(fs::last_write_time(dir), dir));
        }
    }

    const string sid = CUserDefaults::instance().getDefaultSID();

    for (auto& dir : sortedDirs)
    {
        string sSID = dir.second.leaf().string();
        time_t tmLastUseTime = fs::last_write_time(dir.second);
        char sLastUseTime[1000];
        struct tm* p = localtime(&tmLastUseTime);
        strftime(sLastUseTime, 1000, "%FT%TZ", p);

        if (_sortingType == SSessionsSorting::sort_all)
            LOG(log_stdout_clean) << (sSID == sid ? " * " : "   ") << sSID << " \t [" << sLastUseTime << "] \t "
                                  << (IsSessionRunning(sSID) ? "RUNNING" : "STOPPED");
        else if (_sortingType == SSessionsSorting::sort_running && IsSessionRunning(sSID))
            LOG(log_stdout_clean) << (sSID == sid ? " * " : "   ") << sSID << " \t [" << sLastUseTime << "] \t "
                                  << (IsSessionRunning(sSID) ? "RUNNING" : "STOPPED");
    }
}
//=============================================================================
int main(int argc, char* argv[])
{
    SOptions_t options;
    if (dds::misc::defaultExecSetup<SOptions_t>(argc, argv, &options, &ParseCmdLine) == EXIT_FAILURE)
        return EXIT_FAILURE;

    try
    {
        // ++++++++++++++++++
        // Start NEW session
        // ++++++++++++++++++
        if (SOptions_t::cmd_start == options.m_Command)
        {
            CStart start;
            start.start(options.m_bMixed);

            try
            {
                vector<fs::path> session_dirs;
                StringVector_t sessions;
                rebuildSessions(session_dirs, sessions);
                setDefaultSession(sessions, start.getSessionID());
                LOG(log_stdout_clean) << "Default DDS session is set to " << start.getSessionID();
                LOG(log_stdout_clean) << "Currently running DDS sessions:";
                listSessions(session_dirs, SSessionsSorting::sort_running);
            }
            catch (exception& e)
            {
                LOG(warning) << "Failed to perform checks after commander started: " << e.what();
            }

            return EXIT_SUCCESS;
        }

        // ++++++++++++++++++
        // Stop session
        // ++++++++++++++++++
        if (SOptions_t::cmd_stop == options.m_Command)
        {
            string sid = options.m_sSessionID;
            if (sid.empty())
            {
                sid = CUserDefaults::instance().getDefaultSID();
                char answer(0);
                if (!options.m_bForce)
                {
                    do
                    {
                        cout << "Stopping the default session: " << sid << endl;
                        cout << "Are you sure you want to proceed? [y/n]" << endl;
                        cin >> answer;
                    } while (!cin.fail() && answer != 'y' && answer != 'n');
                }

                if (options.m_bForce || answer == 'y')
                {
                    CStop stop;
                    stop.stop(sid);
                }

                return EXIT_SUCCESS;
            }

            CStop stop;
            stop.stop(sid);

            return EXIT_SUCCESS;
        }

        // ++++++++++++++++++
        // Stop ALL session
        // ++++++++++++++++++
        if (SOptions_t::cmd_stop_all == options.m_Command)
        {
            vector<fs::path> session_dirs;
            StringVector_t sessions;
            rebuildSessions(session_dirs, sessions);
            for (const auto& i : sessions)
            {
                if (!IsSessionRunning(i))
                    continue;

                CStop stop;
                stop.stop(i);
            }
            return EXIT_SUCCESS;
        }

        vector<fs::path> session_dirs;
        StringVector_t sessions;
        rebuildSessions(session_dirs, sessions);

        // ++++++++++++++++++
        // List All sessions
        // ++++++++++++++++++
        if (SOptions_t::cmd_list == options.m_Command &&
            options.m_ListSessions.m_typedValue != SSessionsSorting::sort_none)
        {
            listSessions(session_dirs, options.m_ListSessions.m_typedValue);
            return EXIT_SUCCESS;
        }
        // ++++++++++++++++++
        // Set default
        // ++++++++++++++++++
        if (SOptions_t::cmd_set_default == options.m_Command)
        {
            setDefaultSession(sessions, options.m_sSessionID);
        }
        // ++++++++++++++++++
        // Remove All STOPPED sessions
        // ++++++++++++++++++
        if (SOptions_t::cmd_clean == options.m_Command)
        {
            for (auto& dir : session_dirs)
            {
                const string sSID(dir.leaf().string());

                if (IsSessionRunning(sSID))
                    continue;

                LOG(log_stdout_clean) << "\n\nRemoving: " << sSID;

                string sWrkDir(CUserDefaults::instance().getValueForKey("server.work_dir"));
                smart_path(&sWrkDir);
                string sLogDir(CUserDefaults::instance().getValueForKey("server.log_dir"));
                smart_path(&sLogDir);
                string sSandboxDir(CUserDefaults::instance().getValueForKey("server.server.sandbox_dir"));
                smart_path(&sSandboxDir);

                if (!options.m_bForce)
                {
                    LOG(log_stdout_clean) << "\nThere following session directories will be removed.";
                    if (!sWrkDir.empty())
                        LOG(log_stdout_clean) << "DDS Work dir: " << sWrkDir;
                    if (!sLogDir.empty())
                        LOG(log_stdout_clean) << "DDS Log dir: " << sLogDir;
                    if (!sSandboxDir.empty())
                        LOG(log_stdout_clean) << "DDS Sandbox dir: " << sSandboxDir;

                    char answer(0);
                    do
                    {
                        cout << "Are you sure you want to proceed? [y/n]" << endl;
                        cin >> answer;
                    } while (!cin.fail() && answer != 'y' && answer != 'n');

                    if (answer == 'n')
                        return EXIT_SUCCESS;
                }

                if (!sWrkDir.empty())
                {
                    LOG(log_stdout_clean) << "\tDDS Work dir: " << sWrkDir << "\n"
                                          << "\tremoved files count: " << fs::remove_all(sWrkDir);
                }
                if (!sLogDir.empty())
                {
                    LOG(log_stdout_clean) << "\tDDS Log dir: " << sLogDir << "\n"
                                          << "\tremoved files count: " << fs::remove_all(sLogDir);
                }
                if (!sSandboxDir.empty())
                {
                    LOG(log_stdout_clean) << "\tDDS Sandbox dir: " << sSandboxDir << "\n"
                                          << "\tremoved files count: " << fs::remove_all(sSandboxDir);
                }
            }
            return EXIT_SUCCESS;
        }
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
