# CMakeLists.txt for Mantis/Common
# Author: N. Oblath
# Created: Nov 13, 2014

set( MANTIS_COMMON_HEADERFILES
    mt_callable.hh
    mt_condition.hh
    mt_configurator.hh
    mt_constants.hh
    mt_destroyer.hh
    mt_exception.hh
    mt_mutex.hh
    mt_parser.hh
    mt_signal_handler.hh
    mt_singleton.hh
    mt_thread.hh
    mt_uuid.hh
)

set( MANTIS_COMMON_SOURCEFILES
    mt_callable.cc
    mt_condition.cc
    mt_configurator.cc    
    mt_exception.cc
    mt_parser.cc
    mt_signal_handler.cc
    mt_thread.cc
    mt_uuid.cc
    mt_version.cc
)

configure_file( mt_version.hh.in ${CMAKE_CURRENT_BINARY_DIR}/mt_version.hh )
set( MANTIS_COMMON_HEADERFILES
    ${MANTIS_COMMON_HEADERFILES}
    ${CMAKE_CURRENT_BINARY_DIR}/mt_version.hh
)

pbuilder_library( MantisCommon MANTIS_COMMON_SOURCEFILES "" )
pbuilder_install_headers( ${MANTIS_COMMON_HEADERFILES} )

