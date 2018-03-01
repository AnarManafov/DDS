// Copyright 2018 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDSOPTIONS_H
#define DDSOPTIONS_H
//=============================================================================
// BOOST
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

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
// DDS
#include "BOOSTHelper.h"
#include "MiscUtils.h"
#include "ProtocolCommands.h"
#include "Res.h"
#include "SubmitCmd.h"
#include "SysHelper.h"
#include "version.h"

namespace bpo = boost::program_options;

namespace dds
{
    namespace submit_cmd
    {
        struct SSessionsSorting
        {
            enum ETypes
            {
                sort_none,
                sort_all,
                sort_running
            };
            SSessionsSorting(const std::string& _val)
                : m_value(_val)
            {
                if (_val == "all")
                    m_typedValue = sort_all;
                else if (_val == "run")
                    m_typedValue = sort_running;
                else
                    m_typedValue = sort_none;
            }
            std::string m_value;
            ETypes m_typedValue;
        };

        void validate(boost::any& _v, std::vector<std::string> const& _values, SSessionsSorting*, int)
        {
            // Make sure no previous assignment to 'v' was made.
            bpo::validators::check_first_occurrence(_v);

            // Extract the first string from 'values'. If there is more than
            // one string, it's an error, and exception will be thrown.
            const std::string& s = bpo::validators::get_single_string(_values);

            if (s == "all" || s == "run")
            {
                _v = boost::any(SSessionsSorting(s));
            }
            else
            {
                throw bpo::validation_error(bpo::validation_error::invalid_option_value);
            }
        }

        /// \brief dds-commander's container of options
        typedef struct SOptions
        {
            SOptions()
                : m_ListSessions("")
                , m_bRemoveAllStopped(false)
                , m_bForce(false)
            {
            }
            SSessionsSorting m_ListSessions;
            bool m_bRemoveAllStopped;
            bool m_bForce;
            std::string m_sDefault;
        } SOptions_t;
        //=============================================================================
        inline void PrintVersion()
        {
            LOG(MiscCommon::log_stdout) << " v" << PROJECT_VERSION_STRING << "\n"
                                        << "DDS configuration"
                                        << " v" << USER_DEFAULTS_CFG_VERSION << "\n"
                                        << MiscCommon::g_cszReportBugsAddr;
        }
        //=============================================================================
        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-submit options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()("list,l",
                                  bpo::value<SSessionsSorting>(&_options->m_ListSessions),
                                  "List DDS sessions.\n\n"
                                  "Values:\n"
                                  " all: list all sessions\n"
                                  " run: list only running sessions\n");
            options.add_options()("set-default",
                                  bpo::value<std::string>(&_options->m_sDefault),
                                  "Set a giving session id as a default DDS session");
            options.add_options()(
                "remove,r", bpo::bool_switch(&_options->m_bRemoveAllStopped), "Remove all STOPPED DDS sessions");
            options.add_options()("force,f",
                                  bpo::bool_switch(&_options->m_bForce),
                                  "Force commands without prompting for a confirmation.\n"
                                  "For example, can be used with the \"remove\" command.");

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
            bpo::notify(vm);

            // check for non-defaulted arguments
            bpo::variables_map::const_iterator found =
                find_if(vm.begin(), vm.end(), [](const bpo::variables_map::value_type& _v) {
                    return (!_v.second.defaulted());
                });

            if (vm.count("help") || vm.end() == found)
            {
                LOG(MiscCommon::log_stdout) << options;
                return false;
            }
            if (vm.count("version"))
            {
                PrintVersion();
                return false;
            }

            return true;
        }
    }
}
#endif