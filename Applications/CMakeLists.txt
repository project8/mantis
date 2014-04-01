# CMakeLists.txt for Mantis/Applications
# Author: N. Oblath
# Created: Mar 20, 2014

######################
# Client Applications
######################

if( Mantis_ENABLE_TESTING )
    add_executable( test_mantis_client test_mantis_client.cc )
    target_link_libraries( test_mantis_client MantisClient MantisProto ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( test_mantis_client )
    
    add_executable( test_mantis_config test_mantis_config.cc )
    target_link_libraries( test_mantis_config MantisClient MantisProto ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( test_mantis_config )
endif( Mantis_ENABLE_TESTING )

add_executable( mantis_client mantis_client.cc )
target_link_libraries( mantis_client MantisClient MantisProto ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( mantis_client )



######################
# Server Applications
######################

if( Mantis_BUILD_SERVER )

    if( Mantis_ENABLE_TESTING )
        add_executable( test_mantis_digitizer test_mantis_digitizer.cc )
        target_link_libraries( test_mantis_digitizer MantisClient MantisServer MantisProto ${EXTERNAL_LIBRARIES} )
        pbuilder_install_executables( test_mantis_digitizer )
        
        add_executable( test_mantis_server test_mantis_server.cc )
        target_link_libraries( test_mantis_server MantisClient MantisServer MantisProto ${EXTERNAL_LIBRARIES} )
        pbuilder_install_executables( test_mantis_server )
    endif( Mantis_ENABLE_TESTING )
    
    add_executable( mantis_server mantis_server.cc )
    target_link_libraries( mantis_server MantisClient MantisServer MantisProto ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( mantis_server )

endif( Mantis_BUILD_SERVER )