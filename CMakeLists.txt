cmake_minimum_required( VERSION 2.8 )

#########
# setup #
#########

project( Mantis )

include( ${CMAKE_SOURCE_DIR}/PackageBuilder.cmake )
pbuilder_prepare_project( 2 1 1 )

include_directories( ${CMAKE_SOURCE_DIR}/Include )


#######################
# mantis dependencies #
#######################

#pbuilder_add_ext_libraries( "rt" )

#if( NOT DEFINED MONARCH_CONFIG )
#    set( MONARCH_CONFIG "." )
#endif( NOT DEFINED MONARCH_CONFIG )
#set( MONARCH_CONFIG_ABSOLUTE "${CMAKE_BINARY_DIR}/${MONARCH_CONFIG}" )
#if( EXISTS ${MONARCH_CONFIG_ABSOLUTE} AND NOT IS_DIRECTORY ${MONARCH_CONFIG_ABSOLUTE} )
#    message( STATUS "Found Monarch config file: ${MONARCH_CONFIG_ABSOLUTE}" )
#    include( ${CMAKE_BINARY_DIR}/${MONARCH_CONFIG} )
#    include_directories( ${Monarch_INCLUDE_DIRS} )
#    link_directories( ${Monarch_LIBRARY_DIRS} )
#    pbuilder_add_ext_libraries( ${Monarch_LIBRARIES} )
#else( EXISTS ${MONARCH_CONFIG_ABSOLUTE} AND NOT IS_DIRECTORY ${MONARCH_CONFIG_ABSOLUTE} )
#    message( FATAL_ERROR "Did not find the Monarch configuration file; set MONARCH_CONFIG to specify the configuration file relative to the build directory [e.g. cmake -DMONARCH_CONFIG=../../path/to/MonarchConfig.cmake ..]")
#endif( EXISTS ${MONARCH_CONFIG_ABSOLUTE} AND NOT IS_DIRECTORY ${MONARCH_CONFIG_ABSOLUTE} )

#find_path( PX1500_INCLUDE_DIR px1500.h PATHS ${PX1500_PREFIX} )
#if( NOT PX1500_INCLUDE_DIR )
#    message( FATAL_ERROR "Did not find px1500 header; set PX1500_PREFIX to suggest a path" )
#endif( NOT PX1500_INCLUDE_DIR )
#include_directories( ${PX1500_INCLUDE_DIR} )
#link_directories( ${PX1500_PREFIX} )
    
#find_library( PX1500_LIBRARIES sig_px1500 PATHS ${PX1500_PREFIX} )
#if( NOT PX1500_LIBRARIES )
#    message( FATAL_ERROR "Did not find px1500 library; set PX1500_PREFIX to suggest a path" )
#endif( NOT PX1500_LIBRARIES )
#message( STATUS "Found px1500 library: ${PX1500_LIBRARIES}" )
#pbuilder_add_ext_libraries( ${PX1500_LIBRARIES} )

find_package( Protobuf )
include_directories( ${PROTOBUF_INCLUDE_DIR} )
protobuf_generate_cpp( request_source request_header ${CMAKE_CURRENT_SOURCE_DIR}/proto/request.proto )
protobuf_generate_cpp( response_source response_header ${CMAKE_CURRENT_SOURCE_DIR}/proto/response.proto )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
pbuilder_add_ext_libraries( ${PROTOBUF_LIBRARIES} )

##################
# mantis library #
##################

set( MANTISCORE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/exception.hh
    
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mutex.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/condition.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/callable.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/thread.hh
    
    ${CMAKE_CURRENT_SOURCE_DIR}/include/request.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/response.hh
    
    ${CMAKE_CURRENT_SOURCE_DIR}/include/server.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/client.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/connection.hh
    
    ${CMAKE_CURRENT_SOURCE_DIR}/include/run.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/queue.hh

    ${CMAKE_CURRENT_SOURCE_DIR}/include/receiver.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/driver.hh
)

set( MANTISCORE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/exception.cc

    ${CMAKE_CURRENT_SOURCE_DIR}/source/mutex.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/condition.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/callable.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/thread.cc
    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/request.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/response.cc
    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/server.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/client.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/connection.cc
    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/run.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/queue.cc
    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/receiver.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/driver.cc
)

add_library( MantisCore ${MANTISCORE_SOURCEFILES} )
target_link_libraries( MantisCore ${EXTERNAL_LIBRARIES} )

pbuilder_install_headers( ${MANTISCORE_HEADERFILES} )
pbuilder_install_libraries( MantisCore )

######################
# mantis executables #
######################

#add_executable( Mantis ${CMAKE_CURRENT_SOURCE_DIR}/Source/Mantis.cpp )
#target_link_libraries( Mantis MantisCore ${EXTERNAL_LIBRARIES} )
#pbuilder_install_executables( Mantis )

add_executable( MantisClient ${CMAKE_CURRENT_SOURCE_DIR}/Source/MantisClient.cc )
target_link_libraries( MantisClient MantisCore ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( MantisClient )

add_executable( MantisServer ${CMAKE_CURRENT_SOURCE_DIR}/Source/MantisServer.cc )
target_link_libraries( MantisServer MantisCore ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( MantisServer )
