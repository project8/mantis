# CMakeLists.txt for Mantis/Protobuf
# Author: N. Oblath
# Created: March 26, 2014
# For older versions of FindProtobuf.cmake, the protobuf_generate_cpp is supposed to be used in the same directory as 
# the relevant add_library or add_executable.  Therefore a separate proto library is made.

set( PROTO_FILES
    request.proto
    status.proto
    client_status.proto
    response.proto
    block_header.proto
)

protobuf_generate_cpp( Mantis_Protobuf_Sources Mantis_Protobuf_Headers ${PROTO_FILES} )

add_library( MantisProto ${Mantis_Protobuf_Sources} )
target_link_libraries( MantisProto ${EXTERNAL_LIBRARIES} )
pbuilder_install_libraries( MantisProto )
pbuilder_install_headers( ${Mantis_Protobuf_Headers} )

