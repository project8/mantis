# CMakeLists.txt for Mantis/Applications
# Author: N. Oblath
# Created: Mar 20, 2014

######################
# Client Applications
######################

if( Mantis_ENABLE_TESTING )
    add_executable( test_mantis_client ${CMAKE_CURRENT_SOURCE_DIR}/test_mantis_client.cc )
    target_link_libraries( test_mantis_client MantisClient ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( test_mantis_client )
    
    add_executable( test_mantis_config ${CMAKE_CURRENT_SOURCE_DIR}/test_mantis_config.cc )
    target_link_libraries( test_mantis_config MantisClient ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( test_mantis_config )
endif( Mantis_ENABLE_TESTING )

add_executable( mantis_client ${CMAKE_CURRENT_SOURCE_DIR}/mantis_client.cc )
target_link_libraries( mantis_client MantisClient ${EXTERNAL_LIBRARIES} )
pbuilder_install_executables( mantis_client )



######################
# Server Applications
######################

if( Mantis_BUILD_SERVER )

    if( Mantis_ENABLE_TESTING )
        add_executable( test_mantis_digitizer ${CMAKE_CURRENT_SOURCE_DIR}/test_mantis_digitizer.cc )
        target_link_libraries( test_mantis_digitizer MantisClient MantisServer ${EXTERNAL_LIBRARIES} )
        pbuilder_install_executables( test_mantis_digitizer )
        
        add_executable( test_mantis_server ${CMAKE_CURRENT_SOURCE_DIR}/test_mantis_server.cc )
        target_link_libraries( test_mantis_server MantisClient MantisServer ${EXTERNAL_LIBRARIES} )
        pbuilder_install_executables( test_mantis_server )
    endif( Mantis_ENABLE_TESTING )
    
    add_executable( mantis_server ${CMAKE_CURRENT_SOURCE_DIR}/mantis_server.cc )
    target_link_libraries( mantis_server MantisClient MantisServer ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( mantis_server )

endif( Mantis_BUILD_SERVER )
