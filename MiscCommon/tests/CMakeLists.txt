# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(MiscCommon-tests)

set(prefix MiscCommon_test)

#=============================================================================

set(test ${prefix}_MiscUtils)
add_executable(${test} Test_MiscUtils.cpp)

target_link_libraries(${test}
  PUBLIC
  MiscCommon
  Boost::boost
  Boost::unit_test_framework
  Boost::system
  Boost::filesystem
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_MiscCli)
add_executable(${test} Test_MiscCli.cpp)

target_link_libraries(${test}
  PUBLIC
  MiscCommon
  Boost::boost
  Boost::unit_test_framework
  Boost::system
  Boost::filesystem
  Boost::program_options
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_Process)
add_executable(${test} Test_Process.cpp)

target_link_libraries(${test}
  PUBLIC
  MiscCommon
  Boost::boost
  Boost::unit_test_framework
  Boost::system
  Boost::filesystem
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_ProcessExecutable)
add_executable(${test} Test_ProcessExecutable.cpp)

target_link_libraries(${test}
  PUBLIC
  MiscCommon
  Boost::boost
  Boost::system
  Boost::filesystem
  Boost::program_options
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_SysHelper)
add_executable(${test} Test_SysHelper.cpp)

target_link_libraries(${test}
  PUBLIC
  MiscCommon
  Boost::boost
  Boost::unit_test_framework
  Boost::system
  Boost::filesystem
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_FindCfgFile)
add_executable(${test} Test_FindCfgFile.cpp)

target_link_libraries(${test}
  PUBLIC
  MiscCommon
  Boost::boost
  Boost::unit_test_framework
  Boost::system
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_Logger)
add_executable(${test} Test_Logger.cpp)

target_link_libraries(${test}
  PUBLIC
  MiscCommon
  Boost::boost
  Boost::unit_test_framework
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

