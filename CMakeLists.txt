cmake_minimum_required( VERSION 2.8 )

#########
# setup #
#########

project( Mantis )
include( ${PROJECT_SOURCE_DIR}/PackageBuilder.cmake )
pbuilder_prepare_project( 2 1 1 )
include_directories( ${PROJECT_SOURCE_DIR}/include )

#######################
# mantis dependencies #
#######################

# Option for a non-px1500 build, which includes only the client application
set( Mantis_USE_PX1500 TRUE CACHE BOOL "Flag to optionally use the px1500 library" )

add_subdirectory (monarch)
pbuilder_add_ext_libraries (${Monarch_LIBRARIES})
# add to the RPATH to be used when installing, but only if it's not a system directory
list (FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${Monarch_LIBRARY_DIR}" isSystemDir)
if ("${isSystemDir}" STREQUAL "-1")
   list (APPEND CMAKE_INSTALL_RPATH ${Monarch_LIBRARY_DIR})
endif ("${isSystemDir}" STREQUAL "-1")
include_directories (${PROJECT_SOURCE_DIR}/monarch/Include)
include_directories (${PROJECT_SOURCE_DIR}/monarch/libthorax/cpp)
include_directories (${PROJECT_SOURCE_DIR}/monarch/libthorax)

if (Mantis_USE_PX1500)
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
else (Mantis_USE_PX1500)
    message( STATUS "Client only (non-px1500) build")
endif (Mantis_USE_PX1500)

find_package( Protobuf )
include_directories( ${PROTOBUF_INCLUDE_DIR} )
pbuilder_add_ext_libraries( ${PROTOBUF_LIBRARIES} )

protobuf_generate_cpp( Mantis_PB_Request_Source Mantis_PB_Request_Header ${CMAKE_CURRENT_SOURCE_DIR}/proto/request.proto )
protobuf_generate_cpp( Mantis_PB_Status_Source Mantis_PB_Status_Header ${CMAKE_CURRENT_SOURCE_DIR}/proto/status.proto )
protobuf_generate_cpp( Mantis_PB_Response_Source Mantis_PB_Response_Header ${CMAKE_CURRENT_SOURCE_DIR}/proto/response.proto )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )


##################
# mantis library #
##################

set( MANTISCORE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/exception.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/parser.hh
    
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mutex.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/condition.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/callable.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/thread.hh
    
    ${CMAKE_CURRENT_SOURCE_DIR}/include/server.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/client.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/connection.hh
    
    ${CMAKE_CURRENT_SOURCE_DIR}/include/context.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/queue.hh

    ${CMAKE_CURRENT_SOURCE_DIR}/include/block.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/buffer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/iterator.hh

    ${CMAKE_CURRENT_SOURCE_DIR}/include/receiver.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/writer.hh

    ${Mantis_PB_Request_Header}
    ${Mantis_PB_Status_Header}
    ${Mantis_PB_Response_Header}
)

set( MANTISCORE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/exception.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/parser.cc

    ${CMAKE_CURRENT_SOURCE_DIR}/source/mutex.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/condition.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/callable.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/thread.cc
    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/server.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/client.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/connection.cc
    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/context.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/queue.cc

    ${CMAKE_CURRENT_SOURCE_DIR}/source/block.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/buffer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/iterator.cc
    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/receiver.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/writer.cc

    ${Mantis_PB_Request_Source}
    ${Mantis_PB_Status_Source}
    ${Mantis_PB_Response_Source}
)

if (Mantis_USE_PX1500)
    set( MANTISCORE_HEADERFILES
        ${MANTISCORE_HEADERFILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/digitizer.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/include/worker.hh
    )
    
    set( MANTISCORE_SOURCEFILES
        ${MANTISCORE_SOURCEFILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/source/digitizer.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/source/worker.cc
    )
endif (Mantis_USE_PX1500)

add_library( mantis_core ${MANTISCORE_SOURCEFILES} )
target_link_libraries( mantis_core ${EXTERNAL_LIBRARIES} )

pbuilder_install_headers( ${MANTISCORE_HEADERFILES} )
pbuilder_install_libraries( mantis_core )

######################
# mantis executables #
######################

add_executable( test_mantis_client ${CMAKE_CURRENT_SOURCE_DIR}/application/test_mantis_client.cc )
target_link_libraries( test_mantis_client mantis_core ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( test_mantis_client )

add_executable( mantis_client ${CMAKE_CURRENT_SOURCE_DIR}/application/mantis_client.cc )
target_link_libraries( mantis_client mantis_core ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( mantis_client )

if (Mantis_USE_PX1500)
    add_executable( test_mantis_server ${CMAKE_CURRENT_SOURCE_DIR}/application/test_mantis_server.cc )
    target_link_libraries( test_mantis_server mantis_core ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( test_mantis_server )
    
    add_executable( mantis_standalone ${CMAKE_CURRENT_SOURCE_DIR}/application/mantis_standalone.cc )
    target_link_libraries( mantis_standalone mantis_core ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( mantis_standalone )
    
    add_executable( mantis_server ${CMAKE_CURRENT_SOURCE_DIR}/application/mantis_server.cc )
    target_link_libraries( mantis_server mantis_core ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( mantis_server )
endif (Mantis_USE_PX1500)

