# CMakeLists.txt for Mantis/Server
# Author: N. Oblath
# Created: Mar 20, 2014

if( Mantis_BUILD_SERVER )

    set( MANTIS_SERVER_HEADERFILES
        mt_destroyer.hh
        mt_digitizer.hh
        mt_digitizer_test.hh
        mt_digitizer_test16.hh
        mt_factory.hh
        mt_network_writer.hh
        mt_run_queue.hh
        mt_request_receiver.hh
        mt_server_config.hh
        mt_server_worker.hh
        mt_singleton.hh
    )
    
    set( MANTIS_SERVER_SOURCEFILES
        mt_digitizer.cc
        mt_digitizer_test.cc
        mt_digitizer_test16.cc
        mt_network_writer.cc
        mt_run_queue.cc
        mt_request_receiver.cc
        mt_server_config.cc
        mt_server_worker.cc
    )
    
    if( Mantis_INCLUDE_PX1500_SERVER )
        set( MANTIS_SERVER_HEADERFILES
            ${MANTIS_SERVER_HEADERFILES}
            mt_digitizer_px1500.hh
        )
        
        set( MANTIS_SERVER_SOURCEFILES
            ${MANTIS_SERVER_SOURCEFILES}
            mt_digitizer_px1500.cc
        )
    endif( Mantis_INCLUDE_PX1500_SERVER )

    if( Mantis_INCLUDE_PX14400_SERVER )
        set( MANTIS_SERVER_HEADERFILES
            ${MANTIS_SERVER_HEADERFILES}
            mt_digitizer_px14400.hh
        )
        
        set( MANTIS_SERVER_SOURCEFILES
            ${MANTIS_SERVER_SOURCEFILES}
            mt_digitizer_px14400.cc
        )
    endif( Mantis_INCLUDE_PX14400_SERVER )

    if( Mantis_INCLUDE_ROACH_SERVER )
        set( MANTIS_SERVER_HEADERFILES
            ${MANTIS_SERVER_HEADERFILES}
            mt_digitizer_roach.hh
        )

        set( MANTIS_SERVER_SOURCEFILES
            ${MANTIS_SERVER_SOURCEFILES}
            mt_digitizer_roach.cc
        )
    endif( Mantis_INCLUDE_ROACH_SERVER )

    if( Mantis_INCLUDE_U1084A_SERVER )
        set( MANTIS_SERVER_HEADERFILES
            ${MANTIS_SERVER_HEADERFILES}
            mt_digitizer_u1084a.hh
        )
        
        set( MANTIS_SERVER_SOURCEFILES
            ${MANTIS_SERVER_SOURCEFILES}
            mt_digitizer_u1084a.cc
        )
    endif( Mantis_INCLUDE_U1084A_SERVER )

    add_library( MantisServer ${MANTIS_SERVER_SOURCEFILES} )
    target_link_libraries( MantisServer MantisClient MantisProto ${EXTERNAL_LIBRARIES} )
    
    pbuilder_install_headers( ${MANTIS_SERVER_HEADERFILES} )
    pbuilder_install_libraries( MantisServer )
    
endif( Mantis_BUILD_SERVER )