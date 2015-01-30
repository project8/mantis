cmake_minimum_required( VERSION 2.8 )

#########
# setup #
#########

project( Mantis )
cmake_policy (SET CMP0011 NEW)

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Scarab/cmake )

include( GetVersion )
include( PackageBuilder )

pbuilder_prepare_project( ${PROJECT_VERSION_MAJOR} ${PROJECT_VERSION_MINOR} ${PROJECT_REVISION} )

include_directories( BEFORE 
    ${PROJECT_SOURCE_DIR}/Common
    ${PROJECT_SOURCE_DIR}/Client
    ${PROJECT_SOURCE_DIR}/Server
    ${PROJECT_BINARY_DIR}/Common
)

# build shared libraries
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (BUILD_SHARED_LIBS ON)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (BUILD_SHARED_LIBS OFF)    
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")


##################
# mantis options #
##################

# Option to build the client library and executables
set( Mantis_BUILD_CLIENT TRUE CACHE BOOL "Flag to enable building of the client library and applications" )

# Option to build the server library and executables, and options to build different servers
set( Mantis_BUILD_SERVER FALSE CACHE BOOL "Flag to enable building of the server library and applications" )

# add an option to perform iterator time profiling
set( Mantis_ENABLE_ITERATOR_TIMING FALSE CACHE BOOL "Flag to enable iterator time profiling" )
if( Mantis_ENABLE_ITERATOR_TIMING )
    add_definitions( -DENABLE_ITERATOR_TIMING )
else( Mantis_ENABLE_ITERATOR_TIMING )
    remove_definitions( -DENABLE_ITERATOR_TIMING )
endif( Mantis_ENABLE_ITERATOR_TIMING )

# add an option to build the validation tests
option (Mantis_ENABLE_TESTING "Build the validation tests and enable CMake testing" OFF)
if (Mantis_ENABLE_TESTING)
    enable_testing ()
endif (Mantis_ENABLE_TESTING)


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


############
# RapidJSON (as included with Mantis)
############

include_directories( BEFORE ${PROJECT_SOURCE_DIR}/RapidJSON )


##########
# Monarch
##########

add_subdirectory( Monarch )
pbuilder_add_ext_libraries( ${Monarch_LIBRARIES} )
# add to the RPATH to be used when installing, but only if it's not a system directory
list( FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${Monarch_LIBRARY_DIR}" isSystemDir )
if( "${isSystemDir}" STREQUAL "-1" )
   list( APPEND CMAKE_INSTALL_RPATH ${Monarch_LIBRARY_DIR} )
endif( "${isSystemDir}" STREQUAL "-1" )
include_directories( BEFORE 
    ${PROJECT_SOURCE_DIR}/Monarch/Monarch3
    ${PROJECT_SOURCE_DIR}/Monarch/libthorax/cpp
    ${PROJECT_SOURCE_DIR}/Monarch/libthorax
    ${PROJECT_BINARY_DIR}/Monarch
)


###########
# Protobuf
###########

find_package( Protobuf )
include_directories( ${PROTOBUF_INCLUDE_DIR} )
pbuilder_add_ext_libraries( ${PROTOBUF_LIBRARIES} )
include_directories( BEFORE ${CMAKE_CURRENT_BINARY_DIR}/Protobuf )
add_subdirectory( Protobuf )


##########
# Threads
##########

find_package( Threads )
pbuilder_add_ext_libraries( ${CMAKE_THREAD_LIBS_INIT} )


########
# Boost as included with mantis
########

include_directories ( BEFORE 
    ${PROJECT_SOURCE_DIR}
)

add_subdirectory (boost)

pbuilder_add_ext_libraries( boost_atomic )


################
# Build Mantis #
################

add_subdirectory( Common )
add_subdirectory( Client )
add_subdirectory( Server )
add_subdirectory( Applications )
