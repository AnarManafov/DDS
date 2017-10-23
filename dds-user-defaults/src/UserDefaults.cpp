// Copyright 2014 GSI, Inc. All rights reserved.
//
// TODO: Describe
//
#include "UserDefaults.h"
// BOOST
#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/property_tree/ptree.hpp>

// silence "Unused typedef" warning using clang 3.7+ and boost < 1.59
#if BOOST_VERSION < 105900
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/property_tree/ini_parser.hpp>
#if BOOST_VERSION < 105900
#pragma clang diagnostic pop
#endif

// MiscCommon
#include "FindCfgFile.h"
#include "SessionIDFile.h"
// DDS
#include "version.h"

using namespace std;
using namespace dds;
using namespace dds::user_defaults_api;
using namespace MiscCommon;
namespace fs = boost::filesystem;

CUserDefaults::CUserDefaults()
{
    init();
}

CUserDefaults::~CUserDefaults()
{
}

CUserDefaults& CUserDefaults::instance()
{
    static CUserDefaults instance;
    return instance;
}

void CUserDefaults::reinit(const string& _cfgFileName, bool _get_default)
{
    init(_cfgFileName, _get_default);
}

void CUserDefaults::init(const string& _cfgFileName, bool _get_default)
{
    m_keys.clear();
    boost::program_options::options_description config_file_options("DDS user defaults options");
    config_file_options.add_options()(
        "server.work_dir",
        boost::program_options::value<string>(&m_options.m_server.m_workDir)->default_value("$HOME/.DDS"),
        "");
    config_file_options.add_options()(
        "server.sandbox_dir",
        boost::program_options::value<string>(&m_options.m_server.m_sandboxDir)->default_value("$HOME/.DDS"),
        "");
    config_file_options.add_options()(
        "server.log_dir",
        boost::program_options::value<string>(&m_options.m_server.m_logDir)->default_value("$HOME/.DDS/log"),
        "");
    config_file_options.add_options()(
        "server.log_severity_level",
        boost::program_options::value<MiscCommon::ELogSeverityLevel>(&m_options.m_server.m_logSeverityLevel)
            ->default_value(MiscCommon::info));
    config_file_options.add_options()(
        "server.log_rotation_size",
        boost::program_options::value<unsigned int>(&m_options.m_server.m_logRotationSize)->default_value(10));
    config_file_options.add_options()(
        "server.log_has_console_output",
        boost::program_options::value<bool>(&m_options.m_server.m_logHasConsoleOutput)->default_value(true));
    config_file_options.add_options()(
        "server.commander_port_range_min",
        boost::program_options::value<unsigned int>(&m_options.m_server.m_ddsCommanderPortRangeMin)
            ->default_value(20000),
        "");
    config_file_options.add_options()(
        "server.commander_port_range_max",
        boost::program_options::value<unsigned int>(&m_options.m_server.m_ddsCommanderPortRangeMax)
            ->default_value(21000),
        "");
    config_file_options.add_options()(
        "server.idle_time",
        boost::program_options::value<unsigned int>(&m_options.m_server.m_idleTime)->default_value(1800));

    if (!_get_default)
    {
        ifstream ifs(_cfgFileName.c_str());
        if (!ifs.good())
        {
            string msg("Could not open a DDS configuration file: ");
            msg += _cfgFileName;
            throw runtime_error(msg);
        }
        // Parse the config file
        boost::program_options::store(boost::program_options::parse_config_file(ifs, config_file_options, true),
                                      m_keys);
    }
    else
    {
        // we fake reading of arguments, just to get a default values of all keys
        char* arg[1];
        arg[0] = new char[1];
        arg[0][0] = '\0';
        boost::program_options::store(
            boost::program_options::basic_command_line_parser<char>(1, arg).options(config_file_options).run(), m_keys);
        delete[] arg[0];
    }

    boost::program_options::notify(m_keys);
}

void CUserDefaults::init(bool _get_default)
{
    // Use the default look up algorithm
    string sDDSCfgFileName(currentUDFile());
    init(sDDSCfgFileName, _get_default);
}

void CUserDefaults::printDefaults(ostream& _stream)
{
    CUserDefaults ud;
    ud.init(true);

    _stream << "[server]\n"
            << "work_dir=" << ud.getValueForKey("server.work_dir") << "\n"
            << "sandbox_dir=" << ud.getValueForKey("server.sandbox_dir") << "\n"
            << "log_dir=" << ud.getValueForKey("server.log_dir") << "\n"
            << "#\n"
            << "# Log severity can be one of the following values:\n"
            << "# p_l, p_m, p_h, dbg, inf, wrn, err, fat\n"
            << "# p_l - protocol low level events and higher\n"
            << "# p_m - protocol middle level events and higher\n"
            << "# p_h - protocol high level events and higher\n"
            << "# dbg - general debug events and higher\n"
            << "# inf - info events and higher\n"
            << "# wrn - warning events and higher\n"
            << "# err - error events and higher\n"
            << "# fat - fatal errors and higher\n"
            << "#\n"
            << "log_severity_level=" << ud.getValueForKey("server.log_severity_level") << "\n"
            << "log_rotation_size=" << ud.getValueForKey("server.log_rotation_size") << "\n"
            << "log_has_console_output=" << ud.getValueForKey("server.log_has_console_output") << "\n"
            << "commander_port_range_min=" << ud.getValueForKey("server.commander_port_range_min") << "\n"
            << "commander_port_range_max=" << ud.getValueForKey("server.commander_port_range_max") << "\n"
            << "idle_time=" << ud.getValueForKey("server.idle_time") << "\n";
}

string CUserDefaults::convertAnyToString(const boost::any& _any) const
{
    if (_any.type() == typeid(string))
        return boost::any_cast<string>(_any);

    ostringstream ss;
    if (_any.type() == typeid(int))
        ss << boost::any_cast<int>(_any);
    else if (_any.type() == typeid(unsigned int))
        ss << boost::any_cast<unsigned int>(_any);
    else if (_any.type() == typeid(unsigned char))
        ss << boost::any_cast<unsigned char>(_any);
    else if (_any.type() == typeid(unsigned short))
        ss << boost::any_cast<unsigned short>(_any);
    else if (_any.type() == typeid(bool))
        ss << boost::any_cast<bool>(_any);
    else if (_any.type() == typeid(ELogSeverityLevel))
        ss << reinterpret_cast<ELogSeverityLevel>(boost::any_cast<ELogSeverityLevel>(_any));

    return ss.str();
}

string CUserDefaults::getValueForKey(const string& _Key) const
{
    return convertAnyToString(m_keys[_Key].value());
}

/// Returns strings "yes" or "no". Returns an empty string (if key is not of type bool)
string CUserDefaults::getUnifiedBoolValueForBoolKey(const string& _Key) const
{
    if (m_keys[_Key].value().type() != typeid(bool))
        return ("");

    return (m_keys[_Key].as<bool>() ? "yes" : "no");
}

const SDDSUserDefaultsOptions_t CUserDefaults::getOptions() const
{
    return m_options;
}

string CUserDefaults::currentUDFile()
{
    CFindCfgFile<string> cfg;
    cfg.SetOrder("$HOME/.DDS/DDS.cfg")("$DDS_LOCATION/etc/DDS.cfg")("$DDS_LOCATION/DDS.cfg");
    string val;
    cfg.GetCfg(&val);

    smart_path(&val);

    return val;
}

string CUserDefaults::getDDSPath()
{
    char* dds_location;
    dds_location = getenv("DDS_LOCATION");
    if (NULL == dds_location)
        return string();

    string sDDSPath(dds_location);
    smart_path(&sDDSPath);
    smart_append(&sDDSPath, '/');
    return sDDSPath;
}

string CUserDefaults::getServerInfoFileLocationSrv() const
{
    const string sFileName(getServerInfoFileName());
    string sWrkDir(getValueForKey("server.work_dir"));
    smart_path(&sWrkDir);
    smart_append(&sWrkDir, '/');
    return (sWrkDir + sFileName);
}

string CUserDefaults::getServerInfoFileName() const
{
    return ("server_info.cfg");
}

string CUserDefaults::getServerInfoFileLocation() const
{
    const string sFileName(getServerInfoFileName());
    CFindCfgFile<string> cfg;
    const string p1("$DDS_LOCATION/" + sFileName);
    const string p2(getServerInfoFileLocationSrv());

    cfg.SetOrder(p1)(p2);
    string val;
    cfg.GetCfg(&val);

    smart_path(&val);
    return val;
}

string CUserDefaults::getWrkPkgDir() const
{
    string sSandboxDir;
    sSandboxDir = getValueForKey("server.sandbox_dir");
    if (sSandboxDir.empty())
        sSandboxDir = getValueForKey("server.work_dir");

    smart_path(&sSandboxDir);
    smart_append(&sSandboxDir, '/');
    return (sSandboxDir + "wrk/");
}

string CUserDefaults::getWrkPkgPath() const
{
    return (getWrkPkgDir() + "dds-worker");
}

string CUserDefaults::getWrkScriptPath() const
{
    return (getWrkPkgDir() + "DDSWorker.sh");
}

string CUserDefaults::getUserEnvScript() const
{
    CFindCfgFile<string> cfg;
    cfg.SetOrder("$HOME/.DDS/user_worker_env.sh")("$DDS_LOCATION/etc/user_worker_env.sh");
    string val;
    cfg.GetCfg(&val);
    smart_path(&val);
    return val;
}

string CUserDefaults::getAgentIDFile()
{
    const string sFileName("dds-agent.client.id");
    return (getDDSPath() + sFileName);
}

string CUserDefaults::getLogFile() const
{
    char* dds_log_location;
    // DDS_LOG_LOCATION is used only by DDS commander server
    dds_log_location = getenv("DDS_LOG_LOCATION");
    string sLogDir((nullptr == dds_log_location) ? getDDSPath() : dds_log_location);

    if (sLogDir.empty())
        throw runtime_error("Can't init Log engine. Log location is not specified. Make sure DDS environment is "
                            "properly initialised (by using DDS_env.sh).");

    smart_append<string>(&sLogDir, '/');
    string sLogFile(sLogDir);
    sLogFile += "dds_%Y-%m-%d.%N.log";
    smart_path<string>(&sLogFile);
    return sLogFile;
}

string CUserDefaults::getAgentLogStorageDir() const
{
    const string sAgentLogDir("log/agents/");
    string sWrkDir(getValueForKey("server.work_dir"));
    smart_path(&sWrkDir);
    smart_append(&sWrkDir, '/');
    return (sWrkDir + sAgentLogDir);
}

pid_t CUserDefaults::getScoutPid() const
{
    ifstream f(getDDSPath() + "DDSWorker.pid");
    pid_t nDDSScoutPid;
    f >> nDDSScoutPid;
    return nDDSScoutPid;
}

string CUserDefaults::getSMInputName() const
{
    string storageName(to_string(getScoutPid()));
    storageName += "_DDSSMI";
    return storageName;
}

string CUserDefaults::getSMOutputName() const
{
    string storageName(to_string(getScoutPid()));
    storageName += "_DDSSMO";
    return storageName;
}

std::string CUserDefaults::getSMAgentInputName() const
{
    string storageName(to_string(getScoutPid()));
    storageName += "_DDSSMAI";
    return storageName;
}

std::string CUserDefaults::getSMAgentOutputName() const
{
    string storageName(to_string(getScoutPid()));
    storageName += "_DDSSMAO";
    return storageName;
    //    string smName("DDSAGENTO-");
    //    smName += getSID();
    //    return smName;
}

string CUserDefaults::getPluginsRootDir() const
{
    stringstream ss;
    ss << getDDSPath() << "plugins/";
    return (ss.str());
}

string CUserDefaults::getPluginDir(const string& _path, const string& _pluginName) const
{
    stringstream ss;
    std::string path;
    if (!_path.empty())
    {
        path = _path;
        smart_path(&path);
        smart_append(&path, '/');
    }
    else
    {
        path = getPluginsRootDir();
    }
    ss << path << "dds-submit-" << _pluginName << "/";
    return (ss.str());
}

string CUserDefaults::getSIDName() const
{
    return "dds.sid";
}

/// Returns the full path to the main Session ID file
/// The function doesn't check wheather the file exists or not
string CUserDefaults::getMainSIDFileName() const
{
    string sWorkDir(m_options.m_server.m_workDir);
    smart_path(&sWorkDir);

    // Main SID file is the file located on the commander's host
    fs::path pathMainSid(sWorkDir);
    pathMainSid /= getSIDName();
    return pathMainSid.string();
}

/// Returns Session ID full file path (return main SID if exists. If there is no main, it checks for a clone SID. If
/// none of SID exist, the fucntions returns an empty string)
string CUserDefaults::getSIDFile() const
{
    string sWorkDir(m_options.m_server.m_workDir);
    smart_path(&sWorkDir);

    // Main SID file is the file located on the commander's host
    fs::path pathMainSid(sWorkDir);
    pathMainSid /= getSIDName();
    if (fs::is_regular_file(pathMainSid))
        return pathMainSid.string();

    // SID clone file is the copy of the main SID for agents and other external process.
    fs::path pathCloneSid(getDDSPath());
    pathCloneSid /= getSIDName();
    if (fs::is_regular_file(pathCloneSid))
        return pathCloneSid.string();

    // could fined any SID file
    return string();
}

string CUserDefaults::getSID() const
{
    // Get session ID from the local environment
    string sessionID("");
    std::string sidFile = getSIDFile();
    if (sidFile.empty())
    {
        throw runtime_error("Can't find SID file on the local system");
    }
    else
    {
        MiscCommon::CSessionIDFile sid(sidFile);
        sessionID = sid.getSID();
        if (sessionID.empty())
            throw runtime_error("Avaliable SID is empty");
    }
    return sessionID;
}

string CUserDefaults::getAgentNamedMutexName() const
{
    // TODO: FIXME: maximum length of the SM name
    string smName("DDSAGENTMTX-");
    smName += getSID();
    return smName.substr(0, 24);
}
