# CMakeLists.txt for Mantis/Client
# Author: N. Oblath
# Created: Mar 20, 2014

if( Mantis_BUILD_CLIENT )

    set( MANTIS_LIB_DEPS
        MantisCommon
    )

    set( MANTIS_CLIENT_HEADERFILES
        mt_client_config.hh
        #mt_client_file_writing.hh
        #mt_client_worker.hh
        mt_run_client.hh
    )
    
    set( MANTIS_CLIENT_SOURCEFILES
        mt_client_config.cc
        #mt_client_file_writing.cc
        #mt_client_worker.cc
        mt_run_client.cc
    )
    
    pbuilder_library( MantisClient MANTIS_CLIENT_SOURCEFILES MANTIS_LIB_DEPS )
    pbuilder_install_headers( ${MANTIS_CLIENT_HEADERFILES} )

endif( Mantis_BUILD_CLIENT )
