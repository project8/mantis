# Minimum cmake verison 3.1 required for the variable CMAKE_CXX_STANDARD.
cmake_minimum_required (VERSION 3.1)

#########
# setup #
#########

cmake_policy( SET CMP0048 NEW ) # version in project()
project( Mantis VERSION 3.3.1 )

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/dripline/cpp/scarab/cmake )

include( PackageBuilder )

pbuilder_prepare_project()

include_directories( BEFORE 
    ${PROJECT_SOURCE_DIR}/Common
    ${PROJECT_SOURCE_DIR}/Client
    ${PROJECT_SOURCE_DIR}/Server
    ${PROJECT_BINARY_DIR}/Common
)


##################
# mantis options #
##################

# require C++11
set_option( USE_CPP11 TRUE )


# Option to build the client library and executables
option( Mantis_BUILD_CLIENT "Enable building of the client library and applications" TRUE  )

# Option to build the server library and executables, and options to build different servers
option( Mantis_BUILD_SERVER "Enable building of the server library and applications" TRUE )

# add an option to perform iterator time profiling
option( Mantis_ENABLE_ITERATOR_TIMING "Flag to enable iterator time profiling" FALSE )
if( Mantis_ENABLE_ITERATOR_TIMING )
    add_definitions( -DENABLE_ITERATOR_TIMING )
else( Mantis_ENABLE_ITERATOR_TIMING )
    remove_definitions( -DENABLE_ITERATOR_TIMING )
endif( Mantis_ENABLE_ITERATOR_TIMING )

# add an option to build the validation tests
if (Mantis_ENABLE_TESTING)
    enable_testing ()
endif (Mantis_ENABLE_TESTING)

# add an option to build docs
option( Mantis_BUILD_DOCS "Enable make targets for both doxygen and sphinx (requires Doxygen, Sphinx, and Breathe)" FALSE )


#######################
# mantis dependencies #
#######################

######
# Git
######

# if git is used, get the commit SHA1
find_package( Git )
if( GIT_FOUND )
    execute_process( COMMAND ${GIT_EXECUTABLE} rev-parse -q HEAD  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE Mantis_GIT_COMMIT  OUTPUT_STRIP_TRAILING_WHITESPACE )
    execute_process( COMMAND ${GIT_EXECUTABLE} describe --tags --long  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE Mantis_GIT_DESCRIBE  OUTPUT_STRIP_TRAILING_WHITESPACE )
endif( GIT_FOUND )


########
# Boost
########

# Boost (1.46 required for filesystem version 3)
#find_package( Boost 1.46.0 REQUIRED COMPONENTS atomic filesystem date_time )
# make sure dynamic linking is assumed for all boost libraries
#add_definitions( -DBOOST_ALL_DYN_LINK )
#include_directories( ${Boost_INCLUDE_DIRS} )
#pbuilder_add_ext_libraries( ${Boost_LIBRARIES} )


##########
# Monarch
##########

pbuilder_add_submodule( Monarch Monarch )


##########
# dripline
##########

pbuilder_add_submodule( Dripline dripline/cpp )


#########################
# Winsock (Windows only)
#########################

if( WIN32 )
    pbuilder_add_ext_libraries( "Ws2_32" )
endif( WIN32 )


################
# Build Mantis #
################

add_subdirectory( Common )
add_subdirectory( Client )
add_subdirectory( Server )
if( Mantis_ENABLE_EXECUTABLES )
    add_subdirectory( Applications )
endif( Mantis_ENABLE_EXECUTABLES )
if( Mantis_BUILD_DOCS )
    add_subdirectory( Documentation )
endif( Mantis_BUILD_DOCS )

pbuilder_variables_for_parent()
