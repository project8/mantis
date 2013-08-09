cmake_minimum_required( VERSION 2.8 )

#########
# setup #
#########

project( Mantis )
include( ${CMAKE_SOURCE_DIR}/PackageBuilder.cmake )
pbuilder_prepare_project( 2 1 1 )
include_directories( ${CMAKE_SOURCE_DIR}/include )

#######################
# mantis dependencies #
#######################

pbuilder_add_ext_libraries( "rt" )

if( NOT DEFINED MONARCH_CONFIG )
    set( MONARCH_CONFIG "." )
endif( NOT DEFINED MONARCH_CONFIG )
set( MONARCH_CONFIG_ABSOLUTE "${CMAKE_BINARY_DIR}/${MONARCH_CONFIG}" )
if( EXISTS ${MONARCH_CONFIG_ABSOLUTE} AND NOT IS_DIRECTORY ${MONARCH_CONFIG_ABSOLUTE} )
    message( STATUS "Found Monarch config file: ${MONARCH_CONFIG_ABSOLUTE}" )
    include( ${CMAKE_BINARY_DIR}/${MONARCH_CONFIG} )
    include_directories( ${Monarch_INCLUDE_DIRS} )
    link_directories( ${Monarch_LIBRARY_DIRS} )
    pbuilder_add_ext_libraries( ${Monarch_LIBRARIES} )
else( EXISTS ${MONARCH_CONFIG_ABSOLUTE} AND NOT IS_DIRECTORY ${MONARCH_CONFIG_ABSOLUTE} )
    message( FATAL_ERROR "Did not find the Monarch configuration file; set MONARCH_CONFIG to specify the configuration file relative to the build directory [e.g. cmake -DMONARCH_CONFIG=../../path/to/MonarchConfig.cmake ..]")
endif( EXISTS ${MONARCH_CONFIG_ABSOLUTE} AND NOT IS_DIRECTORY ${MONARCH_CONFIG_ABSOLUTE} )

find_path( PX1500_INCLUDE_DIR px1500.h PATHS ${PX1500_PREFIX} )
if( NOT PX1500_INCLUDE_DIR )
    message( FATAL_ERROR "Did not find px1500 header; set PX1500_PREFIX to suggest a path" )
endif( NOT PX1500_INCLUDE_DIR )
include_directories( ${PX1500_INCLUDE_DIR} )
link_directories( ${PX1500_PREFIX} )
    
find_library( PX1500_LIBRARIES sig_px1500 PATHS ${PX1500_PREFIX} )
if( NOT PX1500_LIBRARIES )
    message( FATAL_ERROR "Did not find px1500 library; set PX1500_PREFIX to suggest a path" )
endif( NOT PX1500_LIBRARIES )
message( STATUS "Found px1500 library: ${PX1500_LIBRARIES}" )
pbuilder_add_ext_libraries( ${PX1500_LIBRARIES} )

find_package( Protobuf )
include_directories( ${PROTOBUF_INCLUDE_DIR} )
pbuilder_add_ext_libraries( ${PROTOBUF_LIBRARIES} )

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/include/request.hh ${CMAKE_CURRENT_SOURCE_DIR}/source/request.cc
    COMMAND echo ARGS building request...
    COMMAND protoc ARGS --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto --cpp_out=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/request.proto
    COMMAND mv ARGS ${CMAKE_CURRENT_SOURCE_DIR}/request.pb.h ${CMAKE_CURRENT_SOURCE_DIR}/include/request.hh
    COMMAND mv ARGS ${CMAKE_CURRENT_SOURCE_DIR}/request.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/source/request.cc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/include/response.hh ${CMAKE_CURRENT_SOURCE_DIR}/source/response.cc
    COMMAND echo ARGS building response...
    COMMAND protoc ARGS --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto --cpp_out=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/response.proto
    COMMAND mv ARGS ${CMAKE_CURRENT_SOURCE_DIR}/response.pb.h ${CMAKE_CURRENT_SOURCE_DIR}/include/response.hh
    COMMAND mv ARGS ${CMAKE_CURRENT_SOURCE_DIR}/response.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/source/response.cc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

##################
# mantis library #
##################

set( MANTISCORE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/types.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/time.hh
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

    ${CMAKE_CURRENT_SOURCE_DIR}/include/block.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/buffer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/iterator.hh

    ${CMAKE_CURRENT_SOURCE_DIR}/include/receiver.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/driver.hh
    #${CMAKE_CURRENT_SOURCE_DIR}/include/sender.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/digitizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/writer.hh
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

    ${CMAKE_CURRENT_SOURCE_DIR}/source/block.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/buffer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/iterator.cc
    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/receiver.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/driver.cc
    #${CMAKE_CURRENT_SOURCE_DIR}/source/sender.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/digitizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/writer.cc
)

add_library( mantis_core ${MANTISCORE_SOURCEFILES} )
target_link_libraries( mantis_core ${EXTERNAL_LIBRARIES} )

pbuilder_install_headers( ${MANTISCORE_HEADERFILES} )
pbuilder_install_libraries( mantis_core )

######################
# mantis executables #
######################

add_executable( mantis_standalone ${CMAKE_CURRENT_SOURCE_DIR}/application/mantis_standalone.cc )
target_link_libraries( mantis_standalone mantis_core ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( mantis_standalone )
