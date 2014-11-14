# CMakeLists.txt for Mantis/Server
# Author: N. Oblath
# Created: Mar 20, 2014

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Server/Digitizers )
include( DigitizerConfig )

####################################################
# ADD DIGITIZERS HERE
####################################################
add_digitizer( Test TRUE )
add_digitizer( Test16 TRUE )
add_digitizer( PX1500 )
add_digitizer( PX14400 )
add_digitizer( U1084A )
add_digitizer( RoachSnap )
####################################################
####################################################

if( Mantis_BUILD_SERVER )

    set( MANTIS_SERVER_HEADERFILES
        mt_destroyer.hh
        mt_digitizer.hh
        #mt_digitizer_test16.hh
        mt_factory.hh
        mt_network_writer.hh
        mt_run_queue.hh
        mt_request_receiver.hh
        mt_server_config.hh
        mt_server_worker.hh
        mt_singleton.hh
        ${DIGITIZER_HEADERFILES}
    )
    
    set( MANTIS_SERVER_SOURCEFILES
        mt_digitizer.cc
        #mt_digitizer_test16.cc
        mt_network_writer.cc
        mt_run_queue.cc
        mt_request_receiver.cc
        mt_server_config.cc
        mt_server_worker.cc
        ${DIGITIZER_SOURCEFILES}
    )
    
    add_library( MantisServer ${MANTIS_SERVER_SOURCEFILES} )
    target_link_libraries( MantisServer MantisCommon MantisProto ${EXTERNAL_LIBRARIES} )
    
    pbuilder_install_headers( ${MANTIS_SERVER_HEADERFILES} )
    pbuilder_install_libraries( MantisServer )
    
endif( Mantis_BUILD_SERVER )