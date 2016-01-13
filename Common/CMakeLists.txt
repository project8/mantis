# CMakeLists.txt for Mantis/Common
# Author: N. Oblath
# Created: Nov 13, 2014

set( MANTIS_COMMON_HEADERFILES
    mt_api.hh
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
    mt_version.hh
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
)

configure_file( mt_version.cc.in ${CMAKE_CURRENT_BINARY_DIR}/mt_version.cc )
set( MANTIS_COMMON_SOURCEFILES
    ${MANTIS_COMMON_SOURCEFILES}
    ${CMAKE_CURRENT_BINARY_DIR}/mt_version.cc
)

pbuilder_library( MantisCommon MANTIS_COMMON_SOURCEFILES "" )
pbuilder_install_headers( ${MANTIS_COMMON_HEADERFILES} )

