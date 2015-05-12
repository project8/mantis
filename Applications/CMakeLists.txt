# CMakeLists.txt for Mantis/Applications
# Author: N. Oblath
# Created: Mar 20, 2014

######################
# Common Applications
######################

if( Mantis_ENABLE_TESTING )
    add_executable( test_mantis_parser test_mantis_parser.cc )
    target_link_libraries( test_mantis_parser MantisCommon ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( test_mantis_parser )
endif( Mantis_ENABLE_TESTING )


######################
# Client Applications
######################

if( Mantis_BUILD_CLIENT )

    if( Mantis_ENABLE_TESTING )
        #add_executable( test_mantis_client test_mantis_client.cc )
        #target_link_libraries( test_mantis_client MantisClient MantisCommon ${EXTERNAL_LIBRARIES} )
        #pbuilder_install_executables( test_mantis_client )

        add_executable( test_mantis_config test_mantis_config.cc )
        target_link_libraries( test_mantis_config MantisClient MantisCommon ${EXTERNAL_LIBRARIES} )
        pbuilder_install_executables( test_mantis_config )
    endif( Mantis_ENABLE_TESTING )
    
    add_executable( mantis_client mantis_client.cc )
    target_link_libraries( mantis_client MantisClient MantisCommon ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( mantis_client )
    
endif( Mantis_BUILD_CLIENT )


######################
# Server Applications
######################

if( Mantis_BUILD_SERVER )

    if( Mantis_ENABLE_TESTING )
        add_executable( test_mantis_digitizer test_mantis_digitizer.cc )
        target_link_libraries( test_mantis_digitizer MantisCommon MantisServer ${EXTERNAL_LIBRARIES} )
        pbuilder_install_executables( test_mantis_digitizer )
        
        #add_executable( test_mantis_server test_mantis_server.cc )
        #target_link_libraries( test_mantis_server MantisCommon MantisServer ${EXTERNAL_LIBRARIES} )
        #pbuilder_install_executables( test_mantis_server )
    endif( Mantis_ENABLE_TESTING )
    
    add_executable( mantis_server mantis_server.cc )
    target_link_libraries( mantis_server MantisCommon MantisServer ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( mantis_server )

endif( Mantis_BUILD_SERVER )
