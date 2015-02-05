##########################################
# Find libraries, include directories, etc
##########################################

find_path( NISCOPE_INCLUDE_DIR niScope.h PATHS $ENV{NIIVIPATH}/Include )
message( STATUS "NI-Scope include dir: ${NISCOPE_INCLUDE_DIR}")
include_directories( ${NISCOPE_INCLUDE_DIR} )

find_path( NISCOPE_VXI_INCLUDE_DIR vpptype.h PATHS $ENV{VXIPNPPATH}/WinNT/Include )
message( STATUS "NI-Scope VXI include dir: ${NISCOPE_VXI_INCLUDE_DIR}")
include_directories( ${NISCOPE_VXI_INCLUDE_DIR} )

find_library( NISCOPE_LIBRARIES niScope PATHS $ENV{NIIVIPATH}/Lib_x64/msc )
if( NOT NISCOPE_LIBRARIES )
    message( FATAL_ERROR "Did not find NI-Scope library" )
endif( NOT NISCOPE_LIBRARIES )
message( STATUS "Found NI-Scope library: ${NISCOPE_LIBRARIES}" )
pbuilder_add_ext_libraries( ${NISCOPE_LIBRARIES} )


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
