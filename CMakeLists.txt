cmake_minimum_required( VERSION 2.8 )

#########
# setup #
#########

project( Mantis )
include( ${PROJECT_SOURCE_DIR}/PackageBuilder.cmake )
pbuilder_prepare_project( 2 1 1 )
include_directories( ${PROJECT_SOURCE_DIR}/Client )
include_directories( ${PROJECT_SOURCE_DIR}/Server )

#######################
# mantis dependencies #
#######################

# Option for a non-px1500 build, which includes only the client application
set( Mantis_BUILD_SERVER FALSE CACHE BOOL "Flag to enable building of the server library and applications" )
set( Mantis_INCLUDE_PX1500_SERVER TRUE CACHE BOOL "Flat to include the px1500 server; ignored unless the build-server flag is enabled" )

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
protobuf_generate_cpp( Mantis_PB_Response_Source Mantis_PB_Response_Header ${CMAKE_CURRENT_SOURCE_DIR}/Protobuf/response.proto )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )


#################
# mantis client #
#################

set( MANTIS_CLIENT_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_exception.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_parser.hh
    
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_client.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_connection.hh
    
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_context.hh

    ${Mantis_PB_Request_Header}
    ${Mantis_PB_Status_Header}
    ${Mantis_PB_Response_Header}
)

set( MANTIS_CLIENT_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_exception.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_parser.cc

    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_client.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_connection.cc
    
    ${CMAKE_CURRENT_SOURCE_DIR}/Client/mt_context.cc

    ${Mantis_PB_Request_Source}
    ${Mantis_PB_Status_Source}
    ${Mantis_PB_Response_Source}
)

add_library( MantisClient ${MANTIS_CLIENT_SOURCEFILES} )
target_link_libraries( MantisClient ${EXTERNAL_LIBRARIES} )

pbuilder_install_headers( ${MANTIS_CLIENT_HEADERFILES} )
pbuilder_install_libraries( MantisClient )

add_executable( test_mantis_client ${CMAKE_CURRENT_SOURCE_DIR}/Applications/test_mantis_client.cc )
target_link_libraries( test_mantis_client MantisClient ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( test_mantis_client )

add_executable( mantis_client ${CMAKE_CURRENT_SOURCE_DIR}/Applications/mantis_client.cc )
target_link_libraries( mantis_client MantisClient ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( mantis_client )


#################
# mantis server #
#################

if( Mantis_BUILD_SERVER )

    set( MANTIS_SERVER_HEADERFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mutex.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/condition.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/callable.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/thread.hh
        
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/server.hh
        
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/queue.hh
    
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/block.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/buffer.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/iterator.hh
    
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/receiver.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/writer.hh
    )
    
    set( MANTIS_SERVER_SOURCEFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/mutex.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/condition.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/callable.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/thread.cc
        
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/server.cc
        
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/queue.cc
    
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/block.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/buffer.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/iterator.cc
        
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/receiver.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/writer.cc
    )
    
    if( Mantis_INCLUDE_PX1500_SERVER )
        set( MANTIS_SERVER_HEADERFILES
            ${MANTIS_SERVER_HEADERFILES}
            ${CMAKE_CURRENT_SOURCE_DIR}/Server/digitizer_px1500.hh
            ${CMAKE_CURRENT_SOURCE_DIR}/Server/worker.hh
        )
        
        set( MANTIS_SERVER_SOURCEFILES
            ${MANTIS_SERVER_SOURCEFILES}
            ${CMAKE_CURRENT_SOURCE_DIR}/Server/digitizer_px1500.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/Server/worker.cc
        )
    endif( Mantis_INCLUDE_PX1500_SERVER )

    add_library( MantisServer ${MANTIS_SERVER_SOURCEFILES} )
    target_link_libraries( MantisServer MantisClient ${EXTERNAL_LIBRARIES} )
    
    pbuilder_install_headers( ${MANTIS_SERVER_HEADERFILES} )
    pbuilder_install_libraries( MantisServer )
    
    add_executable( test_mantis_server ${CMAKE_CURRENT_SOURCE_DIR}/Applications/test_mantis_server.cc )
    target_link_libraries( test_mantis_server MantisClient MantisServer ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( test_mantis_server )
    
    if( Mantis_INCLUDE_PX1500_SERVER )
        add_executable( mantis_px1500_standalone ${CMAKE_CURRENT_SOURCE_DIR}/Applications/mantis_px1500_standalone.cc )
        target_link_libraries( mantis_px1500_standalone MantisClient MantisServer ${EXTERNAL_LIBRARIES} )
        pbuilder_install_executables( mantis_px1500_standalone )
        
        add_executable( mantis_px1500_server ${CMAKE_CURRENT_SOURCE_DIR}/Applications/mantis_px1500_server.cc )
        target_link_libraries( mantis_px1500_server MantisClient MantisServer ${EXTERNAL_LIBRARIES} )
        pbuilder_install_executables( mantis_px1500_server )
    endif( Mantis_INCLUDE_PX1500_SERVER )
    
endif( Mantis_BUILD_SERVER )
