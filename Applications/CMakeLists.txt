# CMakeLists.txt for Mantis/Applications
# Author: N. Oblath
# Created: Mar 20, 2014

#############
# Validation
#############

if( Mantis_ENABLE_TESTING )
    add_subdirectory( Validation )
endif( Mantis_ENABLE_TESTING )


######################
# Common Applications
######################



######################
# Client Applications
######################

if( Mantis_BUILD_CLIENT )

    add_executable( mantis_client mantis_client.cc )
    target_link_libraries( mantis_client MantisClient MantisCommon ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( mantis_client )
    
endif( Mantis_BUILD_CLIENT )


######################
# Server Applications
######################

if( Mantis_BUILD_SERVER )

    add_executable( mantis_server mantis_server.cc )
    target_link_libraries( mantis_server MantisCommon MantisServer ${EXTERNAL_LIBRARIES} )
    pbuilder_install_executables( mantis_server )

endif( Mantis_BUILD_SERVER )
