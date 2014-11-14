##########################################
# Find libraries, include directories, etc
##########################################

find_path( U1084A_INCLUDE_DIR AgMD1Fundamental.h PATHS ${U1084A_PREFIX} )
if( NOT U1084A_INCLUDE_DIR )
    message( FATAL_ERROR "Did not find u1084a header (AgMD1Fundamental.h); set U1084A_PREFIX to suggest a path" )
endif( NOT U1084A_INCLUDE_DIR )
include_directories( ${U1084A_INCLUDE_DIR} )
link_directories( ${U1084A_PREFIX} )
    
find_library( U1084A_LIBRARIES AgMD1 PATHS ${U1084A_PREFIX} )
if( NOT U1084A_LIBRARIES )
    message( FATAL_ERROR "Did not find u1084a library; set U1084A_PREFIX to suggest a path" )
endif( NOT U1084A_LIBRARIES )
message( STATUS "Found u1084a library: ${U1084A_LIBRARIES}" )
pbuilder_add_ext_libraries( ${U1084A_LIBRARIES} )


#############################
# Add header and source files
#############################

add_digitizer_header_files(
    mt_digitizer_u1084a.hh
)

add_digitizer_source_files(
    mt_digitizer_u1084a.cc
)


######
# Done
######

message( STATUS "Including the U1084A digitizer" )
