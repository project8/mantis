# CMakeLists.txt for Mantis/Server
# Author: N. Oblath
# Created: Mar 20, 2014

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Server/Devices )
include( DigitizerConfig )

####################################################
# ADD DIGITIZERS HERE
####################################################
add_digitizer( Test TRUE )
add_digitizer( Test16 TRUE )
add_digitizer( PXIe5122 )
add_digitizer( PX1500 )
add_digitizer( PX14400 )
add_digitizer( U1084A )
####################################################
####################################################

if( Mantis_BUILD_SERVER )

    set( MANTIS_SERVER_HEADERFILES
        mt_acq_request_db.hh
        mt_acq_request.hh
        mt_atomic.hh
        #mt_bit_shift_modifier.hh
        mt_block.hh
        mt_buffer.hh
        mt_device_manager.hh
        mt_digitizer.hh
        mt_factory.hh
        mt_file_writer.hh
        mt_iterator.hh
        mt_iterator_timer.hh
        mt_modifier.hh
        mt_request_receiver.hh
        mt_server_config.hh
        mt_server_worker.hh
        mt_writer.hh
        ${DIGITIZER_HEADERFILES}
    )
    
    set( MANTIS_SERVER_SOURCEFILES
        mt_acq_request_db.cc
        mt_acq_request.cc
        #mt_bit_shift_modifier.cc
        mt_block.cc
        mt_buffer.cc
        mt_device_manager.cc
        mt_digitizer.cc
        mt_file_writer.cc
        mt_iterator.cc
        mt_iterator_timer.cc
        mt_modifier.cc
        mt_request_receiver.cc
        mt_server_config.cc
        mt_server_worker.cc
        mt_writer.cc
        ${DIGITIZER_SOURCEFILES}
    )
    
    add_library( MantisServer ${MANTIS_SERVER_SOURCEFILES} )
    target_link_libraries( MantisServer MantisCommon ${EXTERNAL_LIBRARIES} )
    
    pbuilder_install_headers( ${MANTIS_SERVER_HEADERFILES} )
    pbuilder_install_libraries( MantisServer )
    
endif( Mantis_BUILD_SERVER )