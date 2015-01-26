# CMakeLists.txt for Mantis/Common
# Author: N. Oblath
# Created: Nov 13, 2014

set( MANTIS_COMMON_HEADERFILES
    mt_atomic.hh
    #mt_bit_shift_modifier.hh
    mt_block.hh
    mt_broker.hh
    mt_buffer.hh
    mt_callable.hh
    #mt_client_tcp.hh
    mt_condition.hh
    mt_configurator.hh    
    mt_connection.hh
    #mt_distribution.hh
    mt_exception.hh
    mt_iterator.hh
    mt_iterator_timer.hh
    mt_logger.hh
    mt_modifier.hh
    mt_mutex.hh
    mt_param.hh
    mt_parser.hh
    #mt_record_dist.hh
    #mt_record_receiver.hh
    #mt_run_context_dist.hh
    #mt_server_tcp.hh    
    mt_signal_handler.hh
    mt_thread.hh
    mt_writer.hh
)

set( MANTIS_COMMON_SOURCEFILES
    #mt_bit_shift_modifier.cc
    mt_block.cc
    mt_broker.cc
    mt_buffer.cc
    mt_callable.cc
    #mt_client_tcp.cc
    mt_condition.cc
    mt_configurator.cc    
    mt_connection.cc
    #mt_distribution.cc
    mt_exception.cc
    mt_iterator.cc
    mt_iterator_timer.cc
    mt_logger.cc
    mt_modifier.cc
    mt_mutex.cc
    mt_param.cc
    mt_parser.cc
    #mt_record_dist.cc
    #mt_record_receiver.cc
    #mt_run_context_dist.cc
    #mt_server_tcp.cc    
    mt_signal_handler.cc
    mt_thread.cc
    mt_version.cc
    mt_writer.cc
)

configure_file( mt_version.hh.in ${CMAKE_CURRENT_BINARY_DIR}/mt_version.hh )
set( MANTIS_COMMON_HEADERFILES
    ${MANTIS_COMMON_HEADERFILES}
    ${CMAKE_CURRENT_BINARY_DIR}/mt_version.hh
)

add_library( MantisCommon ${MANTIS_COMMON_SOURCEFILES} )
target_link_libraries( MantisCommon MantisProto ${EXTERNAL_LIBRARIES} )

pbuilder_install_headers( ${MANTIS_COMMON_HEADERFILES} )
pbuilder_install_libraries( MantisCommon )

