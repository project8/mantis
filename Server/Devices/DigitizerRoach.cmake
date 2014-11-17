##########################################
# Find libraries, include directories, etc
##########################################
if( NOT KATCP_FOUND )
    add_subdirectory( Devices/katcp_devel )
    pbuilder_add_ext_libraries( ${katcp_devel_LIBRARIES} dl )
    # add to the RPATH to be used when installing, but only if it's not a system directory
    list( FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${katcp_devel_LIBRARIES}" isSystemDir )
    if( "${isSystemDir}" STREQUAL "-1" )
        list( APPEND CMAKE_INSTALL_RPATH ${katcp_devel_LIBRARIES} )
    endif( "${isSystemDir}" STREQUAL "-1" )
    include_directories( Devices/katcp_devel/katcp )
    set( KATCP_FOUND TRUE )
endif( NOT KATCP_FOUND )


#############################
# Add header and source files
#############################

add_digitizer_header_files(
    mt_digitizer_roach_snap.hh
)

add_digitizer_source_files(
    mt_digitizer_roach_snap.cc
)


######
# Done
######

message( STATUS "Including the Roach digitizers" )
