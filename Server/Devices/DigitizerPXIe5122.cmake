##########################################
# Find libraries, include directories, etc
##########################################

if( WIN32 )

    # For Windows
    # The following environment variables can be used:
    #     NIIVIPATH to specify the location niScope.h and Lib_x64/msc/niScope.lib
    #     VXIPNPPATH to specify the location of WinNT/Include/vpptype.h

	find_path( NISCOPE_INCLUDE_DIR $NIScopeHeaderh PATHS $ENV{NIIVIPATH}/Include )
	message( STATUS "NI-Scope include dir: ${NISCOPE_INCLUDE_DIR}")
	include_directories( ${NISCOPE_INCLUDE_DIR} )

	find_path( NISCOPE_VXI_INCLUDE_DIR vpptype.h PATHS $ENV{VXIPNPPATH}/WinNT/Include )
	message( STATUS "NI-Scope VXI include dir: ${NISCOPE_VXI_INCLUDE_DIR}")
	include_directories( ${NISCOPE_VXI_INCLUDE_DIR} )

	find_library( NISCOPE_LIBRARIES $NIScopeHeader PATHS $ENV{NIIVIPATH}/Lib_x64/msc )
	if( NOT NISCOPE_LIBRARIES )
    	message( FATAL_ERROR "Did not find NI-Scope library" )
	endif( NOT NISCOPE_LIBRARIES )
	message( STATUS "Found NI-Scope library: ${NISCOPE_LIBRARIES}" )
	pbuilder_add_ext_libraries( ${NISCOPE_LIBRARIES} )

	add_definitions( -DNISCOPEWIN32 )

else( WIN32 )

    # For Linux (tested with CentOS 5)
    #     NISCOPEPATH to specify the location of include/niScope.h

    find_path( NISCOPE_INCLUDE_DIR niScope.h PATHS $ENV{NISCOPEPATH}/include )
    message( STATUS "NI-Scope include dir: ${NISCOPE_INCLUDE_DIR}" )
	include_directories( ${NISCOPE_INCLUDE_DIR} )

    find_library( NISCOPE_LIBRARIES niScope_32 PATHS $ENV{NISCOPEPATH}/lib )
    if( NOT NISCOPE_LIBRARIES )
        message( FATAL_ERROR "Did not find NI-Scope library" )
    endif( NOT NISCOPE_LIBRARIES )
    message( STATUS "Found NI-Scope library: ${NISCOPE_LIBRARIES}" )
    pbuilder_add_ext_libraries( ${NISCOPE_LIBRARIES} )

    set( CMAKE_REQUIRED_INCLUDES ${NISCOPE_INCLUDE_DIR} )

	add_definitions( -DNISCOPELINUX )

endif( WIN32 )


#############################
# Add header and source files
#############################

add_digitizer_header_files(
    mt_digitizer_pxie5122.hh
)

add_digitizer_source_files(
    mt_digitizer_pxie5122.cc
)


######
# Done
######

message( STATUS "Including the PXIe-5122 digitizer" )
