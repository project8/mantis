# CMakeLists.txt for Mantis/Applications/Validation
# Author: N. Oblath
# Created: Jun 16, 2015

####################
# Common Validation
####################

add_executable( test_mantis_parser test_mantis_parser.cc )
target_link_libraries( test_mantis_parser MantisCommon ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( test_mantis_parser )

add_executable( test_mantis_slack test_mantis_slack.cc )
target_link_libraries( test_mantis_slack MantisCommon ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( test_mantis_slack )


####################
# Client Validation
####################

if( Mantis_BUILD_CLIENT )

    #add_executable( test_mantis_client test_mantis_client.cc )
    #target_link_libraries( test_mantis_client MantisClient MantisCommon ${EXTERNAL_LIBRARIES} )
    #pbuilder_install_executables( test_mantis_client )

    add_executable( test_mantis_config test_mantis_config.cc )
    target_link_libraries( test_mantis_config MantisClient MantisCommon ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( test_mantis_config )
    
endif( Mantis_BUILD_CLIENT )


####################
# Server Validation
####################

if( Mantis_BUILD_SERVER )

    add_executable( test_mantis_digitizer test_mantis_digitizer.cc )
    target_link_libraries( test_mantis_digitizer MantisCommon MantisServer ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( test_mantis_digitizer )
    
    #add_executable( test_mantis_server test_mantis_server.cc )
    #target_link_libraries( test_mantis_server MantisCommon MantisServer ${EXTERNAL_LIBRARIES} )
    #pbuilder_install_executables( test_mantis_server )

endif( Mantis_BUILD_SERVER )
