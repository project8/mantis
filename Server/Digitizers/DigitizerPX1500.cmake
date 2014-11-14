##########################################
# Find libraries, include directories, etc
##########################################

find_path( PX1500_INCLUDE_DIR px1500.h PATHS ${PX1500_PREFIX} )
if( NOT PX1500_INCLUDE_DIR )
    message( FATAL_ERROR "Did not find px1500 header; set PX1500_PREFIX to suggest a path" )
endif( NOT PX1500_INCLUDE_DIR )
include_directories( ${PX1500_INCLUDE_DIR} )
link_directories( ${PX1500_PREFIX} )
    
find_library( PX1500_LIBRARIES sig_px1500 PATHS ${PX1500_PREFIX} )
if( NOT PX1500_LIBRARIES )
    message( FATAL_ERROR "Did not find px1500 library; set PX1500_PREFIX to suggest a path" )
endif( NOT PX1500_LIBRARIES )
message( STATUS "Found px1500 library: ${PX1500_LIBRARIES}" )
pbuilder_add_ext_libraries( ${PX1500_LIBRARIES} )


#############################
# Add header and source files
#############################

add_digitizer_header_files(
    mt_digitizer_px1500.hh
)

add_digitizer_source_files(
    mt_digitizer_px1500.cc
)


######
# Done
######

message( STATUS "Including the PX1500 digitizer" )
