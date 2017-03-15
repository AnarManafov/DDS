# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds_intercom-test_multiple_instances )


include_directories(
	${Boost_INCLUDE_DIRS}
)

##################################################################
# main executable
##################################################################

set( MAIN_SOURCE_FILES
	./main.cpp
)

add_executable(dds_intercom-test_multiple_instances ${MAIN_SOURCE_FILES})

install(TARGETS dds_intercom-test_multiple_instances DESTINATION tests)

##################################################################
# key-value
##################################################################

set( KV_SOURCE_FILES
	./dds_intercom-test-multiple-instances-kv.cpp
)

add_library(dds_intercom-test_multiple_instances_kv ${KV_SOURCE_FILES})

target_link_libraries (
	dds_intercom-test_multiple_instances_kv
	dds_intercom_lib
)

target_include_directories(dds_intercom-test_multiple_instances_kv
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds_intercom-test_multiple_instances_kv DESTINATION tests)

##################################################################
# custom cmd
##################################################################

set( CC_SOURCE_FILES
	./dds_intercom-test-multiple-instances-cc.cpp
)

add_library(dds_intercom-test_multiple_instances_cc ${CC_SOURCE_FILES})

target_link_libraries (
	dds_intercom-test_multiple_instances_cc
	dds_intercom_lib
)

target_include_directories(dds_intercom-test_multiple_instances_cc
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds_intercom-test_multiple_instances_cc DESTINATION tests)
