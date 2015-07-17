# CMakeLists.txt for Mantis/Common
# Author: N. Oblath
# Created: Nov 13, 2014

set( MANTIS_COMMON_HEADERFILES
    mt_amqp.hh
    mt_amqp_relayer.hh
    mt_atomic.hh
    mt_authentication.hh
    mt_broker.hh
    mt_callable.hh
    mt_concurrent_queue.hh
    mt_condition.hh
    mt_configurator.hh
    mt_constants.hh
    mt_destroyer.hh
    mt_exception.hh
    mt_logger.hh
    mt_message.hh
    mt_mutex.hh
    mt_param.hh
    mt_param_json.hh
    mt_param_msgpack.hh
    mt_parser.hh
    mt_signal_handler.hh
    mt_singleton.hh
    mt_thread.hh
)

set( MANTIS_COMMON_SOURCEFILES
    mt_amqp_relayer.cc
    mt_authentication.cc
    mt_broker.cc
    mt_callable.cc
    mt_condition.cc
    mt_configurator.cc    
    mt_exception.cc
    mt_logger.cc
    mt_message.cc
    mt_mutex.cc
    mt_param.cc
    mt_param_json.cc
    mt_param_msgpack.cc
    mt_parser.cc
    mt_signal_handler.cc
    mt_thread.cc
    mt_version.cc
)

configure_file( mt_version.hh.in ${CMAKE_CURRENT_BINARY_DIR}/mt_version.hh )
set( MANTIS_COMMON_HEADERFILES
    ${MANTIS_COMMON_HEADERFILES}
    ${CMAKE_CURRENT_BINARY_DIR}/mt_version.hh
)

add_library( MantisCommon ${MANTIS_COMMON_SOURCEFILES} )
target_link_libraries( MantisCommon ${EXTERNAL_LIBRARIES} )

pbuilder_install_headers( ${MANTIS_COMMON_HEADERFILES} )
pbuilder_install_libraries( MantisCommon )

