# Minimum cmake verison 3.0 required by the way CMake handles rpath information on Mac OSX.
cmake_minimum_required (VERSION 3.0)

#########
# setup #
#########

cmake_policy( SET CMP0048 NEW ) # version in project()
project( Mantis VERSION 3.3.1 )

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Scarab/cmake )

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

# Option to build the client library and executables
set( Mantis_BUILD_CLIENT TRUE CACHE BOOL "Flag to enable building of the client library and applications" )

# Option to build the server library and executables, and options to build different servers
set( Mantis_BUILD_SERVER TRUE CACHE BOOL "Flag to enable building of the server library and applications" )

# add an option to perform iterator time profiling
set( Mantis_ENABLE_ITERATOR_TIMING FALSE CACHE BOOL "Flag to enable iterator time profiling" )
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
set( Mantis_BUILD_DOCS FALSE CACHE BOOL "Enable make targets for both doxygen and sphinx (requires Doxygen, Sphinx, and Breathe)" )


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
find_package( Boost 1.46.0 REQUIRED COMPONENTS atomic filesystem thread date_time )
# make sure dynamic linking is assumed for all boost libraries
add_definitions( -DBOOST_ALL_DYN_LINK )
include_directories( ${Boost_INCLUDE_DIRS} )
pbuilder_add_ext_libraries( ${Boost_LIBRARIES} )


########
# Scarab
########

pbuilder_add_submodule( Scarab Scarab/library )
set( Scarab_BUILD_CODEC_JSON TRUE CACHE BOOL "" FORCE )
set( Scarab_BUILD_CODEC_MSGPACK TRUE CACHE BOOL "" FORCE )


##########
# Monarch
##########

pbuilder_add_submodule( Monarch Monarch )


###########
# libthorax
###########

pbuilder_add_submodule( libthorax libthorax )


###################
# SimpleAmqpClient
###################

pbuilder_add_submodule( SimpleAmqpClient SimpleAmqpClient )


##########
# Threads
##########

find_package( Threads )
pbuilder_add_ext_libraries( ${CMAKE_THREAD_LIBS_INIT} )


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
