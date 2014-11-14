##########################################
# Find libraries, include directories, etc
##########################################

find_path( PX14400_INCLUDE_DIR px14.h PATHS ${PX14400_PREFIX} )
if( NOT PX14400_INCLUDE_DIR )
    message( FATAL_ERROR "Did not find px14400 header; set PX14400_PREFIX to suggest a path" )
endif( NOT PX14400_INCLUDE_DIR )
include_directories( ${PX14400_INCLUDE_DIR} )
link_directories( ${PX14400_PREFIX} )
    
find_library( PX14400_LIBRARIES sig_px14400 PATHS ${PX14400_PREFIX} )
if( NOT PX14400_LIBRARIES )
    message( FATAL_ERROR "Did not find px1500 library; set PX14400_PREFIX to suggest a path" )
endif( NOT PX14400_LIBRARIES )
message( STATUS "Found px14400 library: ${PX14400_LIBRARIES}" )
pbuilder_add_ext_libraries( ${PX14400_LIBRARIES} )


#############################
# Add header and source files
#############################

add_digitizer_header_files(
    mt_digitizer_px14400.hh
)

add_digitizer_source_files(
    mt_digitizer_px14400.cc
)


######
# Done
######

message( STATUS "Including the PX14400 digitizer" )
