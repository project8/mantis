# CMakeLists.txt for Mantis/Client
# Author: N. Oblath
# Created: Mar 20, 2014

protobuf_generate_cpp( Mantis_PB_Request_Source Mantis_PB_Request_Header ${PROJECT_SOURCE_DIR}/Protobuf/request.proto )
protobuf_generate_cpp( Mantis_PB_Status_Source Mantis_PB_Status_Header ${PROJECT_SOURCE_DIR}/Protobuf/status.proto )
protobuf_generate_cpp( Mantis_PB_Client_Status_Source Mantis_PB_Client_Status_Header ${PROJECT_SOURCE_DIR}/Protobuf/client_status.proto )
protobuf_generate_cpp( Mantis_PB_Response_Source Mantis_PB_Response_Header ${PROJECT_SOURCE_DIR}/Protobuf/response.proto )
protobuf_generate_cpp( Mantis_PB_Block_Header_Source Mantis_PB_Block_Header_Header ${PROJECT_SOURCE_DIR}/Protobuf/block_header.proto )

set( MANTIS_CLIENT_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_atomic.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_block.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_buffer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_callable.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_client_config.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_client_file_writing.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_client_worker.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_client.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_condition.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_configurator.hh    
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_connection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_distribution.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_exception.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_file_writer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_iterator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_logger.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_mutex.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_param.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_parser.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_record_dist.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_record_receiver.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_run_client.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_run_context_dist.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_run_description.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_server.hh    
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_signal_handler.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_thread.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_writer.hh

    ${Mantis_PB_Request_Header}
    ${Mantis_PB_Status_Header}
    ${Mantis_PB_Client_Status_Header}
    ${Mantis_PB_Response_Header}
    ${Mantis_PB_Block_Header_Header}
)

set( MANTIS_CLIENT_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_block.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_buffer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_callable.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_client_config.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_client_file_writing.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_client_worker.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_client.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_condition.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_configurator.cc    
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_connection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_distribution.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_exception.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_file_writer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_iterator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_logger.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_mutex.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_param.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_parser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_record_dist.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_record_receiver.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_run_client.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_run_context_dist.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_run_description.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_server.cc    
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_signal_handler.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_thread.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_writer.cc

    ${Mantis_PB_Request_Source}
    ${Mantis_PB_Status_Source}
    ${Mantis_PB_Client_Status_Source}
    ${Mantis_PB_Response_Source}
    ${Mantis_PB_Block_Header_Source}
)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/mt_version.hh.in ${CMAKE_CURRENT_BINARY_DIR}/mt_version.hh )
set( MANTIS_CLIENT_HEADERFILES
    ${MANTIS_CLIENT_HEADERFILES}
    ${CMAKE_CURRENT_BINARY_DIR}/mt_version.hh
)

add_library( MantisClient ${MANTIS_CLIENT_SOURCEFILES} )
target_link_libraries( MantisClient ${EXTERNAL_LIBRARIES} )

pbuilder_install_headers( ${MANTIS_CLIENT_HEADERFILES} )
pbuilder_install_libraries( MantisClient )

