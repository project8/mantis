cmake_minimum_required( VERSION 2.8 )

#########
# setup #
#########

project( Mantis )
include( ${PROJECT_SOURCE_DIR}/PackageBuilder.cmake )
pbuilder_prepare_project( 2 1 1 )
include_directories( ${PROJECT_SOURCE_DIR}/Client )
include_directories( ${PROJECT_SOURCE_DIR}/Server )

##################
# mantis options #
##################

# Option for a non-px1500 build, which includes only the client application
set( Mantis_BUILD_SERVER FALSE CACHE BOOL "Flag to enable building of the server library and applications" )
set( Mantis_INCLUDE_PX1500_SERVER TRUE CACHE BOOL "Flat to include the px1500 server; ignored unless the build-server flag is enabled" )


#######################
# mantis dependencies #
#######################

include_directories ("${PROJECT_SOURCE_DIR}/RapidJSON")

add_subdirectory( Monarch )
pbuilder_add_ext_libraries( ${Monarch_LIBRARIES} )
# add to the RPATH to be used when installing, but only if it's not a system directory
list( FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${Monarch_LIBRARY_DIR}" isSystemDir )
if( "${isSystemDir}" STREQUAL "-1" )
   list( APPEND CMAKE_INSTALL_RPATH ${Monarch_LIBRARY_DIR} )
endif( "${isSystemDir}" STREQUAL "-1" )
include_directories( ${PROJECT_SOURCE_DIR}/Monarch/Include )
include_directories( ${PROJECT_SOURCE_DIR}/Monarch/libthorax/cpp )
include_directories( ${PROJECT_SOURCE_DIR}/Monarch/libthorax )

if( Mantis_BUILD_SERVER )
    if( Mantis_INCLUDE_PX1500_SERVER )
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
        message( STATUS "Will include the PX1500 server" )
    endif (Mantis_INCLUDE_PX1500_SERVER)
else( Mantis_BUILD_SERVER )
    message( STATUS "Will perform a client-only build")
endif( Mantis_BUILD_SERVER )

find_package( Protobuf )
include_directories( ${PROTOBUF_INCLUDE_DIR} )
pbuilder_add_ext_libraries( ${PROTOBUF_LIBRARIES} )

protobuf_generate_cpp( Mantis_PB_Request_Source Mantis_PB_Request_Header ${CMAKE_CURRENT_SOURCE_DIR}/Protobuf/request.proto )
protobuf_generate_cpp( Mantis_PB_Status_Source Mantis_PB_Status_Header ${CMAKE_CURRENT_SOURCE_DIR}/Protobuf/status.proto )
protobuf_generate_cpp( Mantis_PB_Client_Status_Source Mantis_PB_Client_Status_Header ${CMAKE_CURRENT_SOURCE_DIR}/Protobuf/client_status.proto )
protobuf_generate_cpp( Mantis_PB_Response_Source Mantis_PB_Response_Header ${CMAKE_CURRENT_SOURCE_DIR}/Protobuf/response.proto )
protobuf_generate_cpp( Mantis_PB_Block_Header_Source Mantis_PB_Block_Header_Header ${CMAKE_CURRENT_SOURCE_DIR}/Protobuf/block_header.proto )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# Threads

find_package( Threads )
pbuilder_add_ext_libraries( ${CMAKE_THREAD_LIBS_INIT} )


# Boost as included with mantis

include_directories (
    ${PROJECT_SOURCE_DIR}
)

add_subdirectory (boost)

pbuilder_add_ext_libraries( boost_atomic )



#################
# mantis client #
#################

set( MANTIS_CLIENT_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_atomic.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_block.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_buffer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_callable.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_client_config.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_client_file_writing.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_client_worker.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_client.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_condition.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_configuration.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_configurator.hh    
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_connection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_distribution.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_exception.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_file_writer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_iterator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_mutex.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_parser.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_record_dist.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_record_receiver.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_run_context_dist.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_server.hh    
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_signal_handler.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_thread.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_writer.hh

    ${Mantis_PB_Request_Header}
    ${Mantis_PB_Status_Header}
    ${Mantis_PB_Client_Status_Header}
    ${Mantis_PB_Response_Header}
    ${Mantis_PB_Block_Header_Header}
)

set( MANTIS_CLIENT_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_block.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_buffer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_callable.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_client_config.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_client_file_writing.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_client_worker.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_client.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_condition.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_configuration.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_configurator.cc    
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_connection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_distribution.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_exception.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_file_writer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_iterator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_mutex.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_parser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_record_dist.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_record_receiver.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_run_context_dist.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_server.cc    
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_signal_handler.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_thread.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_writer.cc

    ${Mantis_PB_Request_Source}
    ${Mantis_PB_Status_Source}
    ${Mantis_PB_Client_Status_Source}
    ${Mantis_PB_Response_Source}
    ${Mantis_PB_Block_Header_Source}
)

add_library( MantisClient ${MANTIS_CLIENT_SOURCEFILES} )
target_link_libraries( MantisClient ${EXTERNAL_LIBRARIES} )

pbuilder_install_headers( ${MANTIS_CLIENT_HEADERFILES} )
pbuilder_install_libraries( MantisClient )

add_executable( test_mantis_client ${CMAKE_CURRENT_SOURCE_DIR}/Applications/test_mantis_client.cc )
target_link_libraries( test_mantis_client MantisClient ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( test_mantis_client )

add_executable( test_mantis_config ${CMAKE_CURRENT_SOURCE_DIR}/Applications/test_mantis_config.cc )
target_link_libraries( test_mantis_config MantisClient ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( test_mantis_config )

add_executable( mantis_client ${CMAKE_CURRENT_SOURCE_DIR}/Applications/mantis_client.cc )
target_link_libraries( mantis_client MantisClient ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( mantis_client )


#################
# mantis server #
#################

if( Mantis_BUILD_SERVER )

    set( MANTIS_SERVER_HEADERFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_destroyer.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_digitizer.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_digitizer_test.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_factory.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_network_writer.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_run_queue.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_request_receiver.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_server_config.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_server_worker.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_singleton.hh
    )
    
    set( MANTIS_SERVER_SOURCEFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_digitizer.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_digitizer_test.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_network_writer.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_run_queue.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_request_receiver.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_server_config.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_server_worker.cc
    )
    
    if( Mantis_INCLUDE_PX1500_SERVER )
        set( MANTIS_SERVER_HEADERFILES
            ${MANTIS_SERVER_HEADERFILES}
            ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_digitizer_px1500.hh
        )
        
        set( MANTIS_SERVER_SOURCEFILES
            ${MANTIS_SERVER_SOURCEFILES}
            ${CMAKE_CURRENT_SOURCE_DIR}/Server/mt_digitizer_px1500.cc
        )
    endif( Mantis_INCLUDE_PX1500_SERVER )

    add_library( MantisServer ${MANTIS_SERVER_SOURCEFILES} )
    target_link_libraries( MantisServer MantisClient ${EXTERNAL_LIBRARIES} )
    
    pbuilder_install_headers( ${MANTIS_SERVER_HEADERFILES} )
    pbuilder_install_libraries( MantisServer )
    
    add_executable( test_mantis_server ${CMAKE_CURRENT_SOURCE_DIR}/Applications/test_mantis_server.cc )
    target_link_libraries( test_mantis_server MantisClient MantisServer ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( test_mantis_server )
    
    add_executable( mantis_server ${CMAKE_CURRENT_SOURCE_DIR}/Applications/mantis_server.cc )
    target_link_libraries( mantis_server MantisClient MantisServer ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( mantis_server )

endif( Mantis_BUILD_SERVER )
