// Copyright 2014 GSI, Inc. All rights reserved.
//
// TODO: Describe
//
#include "UserDefaults.h"
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
// MiscCommon
#include "FindCfgFile.h"

using namespace std;
using namespace dds;
using namespace MiscCommon;

void CUserDefaults::init(const string& _DDSCfgFileName, bool _get_default)
{
    m_keys.clear();
    boost::program_options::options_description config_file_options("DDS user defaults options");
    config_file_options.add_options()(
        "server.work_dir", boost::program_options::value<string>(&m_options.m_general.m_workDir)->default_value("$HOME/.DDS"), "");
    config_file_options.add_options()(
        "server.log_dir", boost::program_options::value<string>(&m_options.m_general.m_logDir)->default_value("$HOME/.DDS/log"), "");
    config_file_options.add_options()("server.log_severity_level",
                                      boost::program_options::value<unsigned int>(&m_options.m_general.m_logSeverityLevel)->default_value(0));
    config_file_options.add_options()("server.log_rotation_size",
                                      boost::program_options::value<unsigned int>(&m_options.m_general.m_logRotationSize)->default_value(10 * 1024 * 1024));
    config_file_options.add_options()("server.log_has_console_output",
                                      boost::program_options::value<bool>(&m_options.m_general.m_logHasConsoleOutput)->default_value(false));
    config_file_options.add_options()("server.commander_port_range_min",
                                      boost::program_options::value<unsigned int>(&m_options.m_general.m_ddsCommanderPortRangeMin)->default_value(20000),
                                      "");
    config_file_options.add_options()("server.commander_port_range_max",
                                      boost::program_options::value<unsigned int>(&m_options.m_general.m_ddsCommanderPortRangeMax)->default_value(21000),
                                      "");
    if (!_get_default)
    {
        std::ifstream ifs(_DDSCfgFileName.c_str());
        if (!ifs.good())
        {
            string msg("Could not open a DDS configuration file: ");
            msg += _DDSCfgFileName;
            throw runtime_error(msg);
        }
        // Parse the config file
        boost::program_options::store(boost::program_options::parse_config_file(ifs, config_file_options, true), m_keys);
    }
    else
    {
        // we fake reading of arguments, just to get a default values of all keys
        char* arg[1];
        arg[0] = new char[1];
        arg[0][0] = '\0';
        boost::program_options::store(boost::program_options::basic_command_line_parser<char>(1, arg).options(config_file_options).run(), m_keys);
        delete[] arg[0];
    }

    boost::program_options::notify(m_keys);
}

void CUserDefaults::printDefaults(ostream& _stream)
{
    CUserDefaults ud;
    ud.init("", true);

    _stream << "[server]\n"
            << "work_dir=" << ud.getValueForKey("server.work_dir") << "\n"
            << "log_dir=" << ud.getValueForKey("server.log_dir") << "\n"
            << "log_severity_level=" << ud.getValueForKey("server.log_severity_level") << "\n"
            << "log_rotation_size=" << ud.getValueForKey("server.log_rotation_size") << "\n"
            << "log_has_console_output=" << ud.getValueForKey("server.log_has_console_output") << "\n"
            << "commander_port_range_min=" << ud.getValueForKey("server.commander_port_range_min") << "\n"
            << "commander_port_range_max=" << ud.getValueForKey("server.commander_port_range_max") << "\n";
}

// TODO: we use boost 1.32. This is the only method I found to convert boost::any to string.
// In the next version of boost its solved.
string CUserDefaults::convertAnyToString(const boost::any& _any) const
{
    if (_any.type() == typeid(string))
        return boost::any_cast<string>(_any);

    ostringstream ss;
    if (_any.type() == typeid(int))
        ss << boost::any_cast<int>(_any);

    if (_any.type() == typeid(unsigned int))
        ss << boost::any_cast<unsigned int>(_any);

    if (_any.type() == typeid(unsigned char))
        ss << boost::any_cast<unsigned char>(_any);

    if (_any.type() == typeid(unsigned short))
        ss << boost::any_cast<unsigned short>(_any);

    if (_any.type() == typeid(bool))
        ss << boost::any_cast<bool>(_any);

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

string CUserDefaults::currentUDFile()
{
    MiscCommon::CFindCfgFile<string> cfg;
    cfg.SetOrder("$HOME/.DDS/DDS.cfg")("$DDS_LOCATION/etc/DDS.cfg")("$DDS_LOCATION/DDS.cfg");
    string val;
    cfg.GetCfg(&val);

    smart_path(&val);

    return val;
}

std::string getDDSPath()
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