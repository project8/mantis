# CMakeLists.txt for Mantis/Server
# Author: N. Oblath
# Created: Mar 20, 2014

if( Mantis_BUILD_SERVER )

    set( MANTIS_SERVER_HEADERFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_destroyer.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_digitizer.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_digitizer_test.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_digitizer_test16.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_factory.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_network_writer.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_run_queue.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_request_receiver.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_server_config.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_server_worker.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_singleton.hh
    )
    
    set( MANTIS_SERVER_SOURCEFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_digitizer.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_digitizer_test.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_digitizer_test16.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_network_writer.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_run_queue.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_request_receiver.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_server_config.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/mt_server_worker.cc
    )
    
    if( Mantis_INCLUDE_PX1500_SERVER )
        set( MANTIS_SERVER_HEADERFILES
            ${MANTIS_SERVER_HEADERFILES}
            ${CMAKE_CURRENT_SOURCE_DIR}/mt_digitizer_px1500.hh
        )
        
        set( MANTIS_SERVER_SOURCEFILES
            ${MANTIS_SERVER_SOURCEFILES}
            ${CMAKE_CURRENT_SOURCE_DIR}/mt_digitizer_px1500.cc
        )
    endif( Mantis_INCLUDE_PX1500_SERVER )

    if( Mantis_INCLUDE_PX14400_SERVER )
        set( MANTIS_SERVER_HEADERFILES
            ${MANTIS_SERVER_HEADERFILES}
            ${CMAKE_CURRENT_SOURCE_DIR}/mt_digitizer_px14400.hh
        )
        
        set( MANTIS_SERVER_SOURCEFILES
            ${MANTIS_SERVER_SOURCEFILES}
            ${CMAKE_CURRENT_SOURCE_DIR}/mt_digitizer_px14400.cc
        )
    endif( Mantis_INCLUDE_PX14400_SERVER )

    add_library( MantisServer ${MANTIS_SERVER_SOURCEFILES} )
    target_link_libraries( MantisServer MantisClient ${EXTERNAL_LIBRARIES} )
    
    pbuilder_install_headers( ${MANTIS_SERVER_HEADERFILES} )
    pbuilder_install_libraries( MantisServer )
    
endif( Mantis_BUILD_SERVER )