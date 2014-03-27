cmake_minimum_required( VERSION 2.8 )

#########
# setup #
#########

project( Mantis )
cmake_policy (SET CMP0011 NEW)

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} )

include( PackageBuilder.cmake )
<<<<<<< HEAD
pbuilder_prepare_project( 2 5 1 )
include_directories( ${PROJECT_SOURCE_DIR}/Client )
include_directories( ${PROJECT_SOURCE_DIR}/Server )
=======
pbuilder_prepare_project( 2 5 0 )
include_directories( 
    ${PROJECT_SOURCE_DIR}/Client
    ${PROJECT_SOURCE_DIR}/Server
    ${PROJECT_BINARY_DIR}/Client
)
>>>>>>> refs/heads/templatedMantis

##################
# mantis options #
##################

# Option for a non-px1500 build, which includes only the client application
set( Mantis_BUILD_SERVER FALSE CACHE BOOL "Flag to enable building of the server library and applications" )
set( Mantis_INCLUDE_PX1500_SERVER TRUE CACHE BOOL "Flag to include the px1500 server; ignored unless the build-server flag is enabled" )
set( Mantis_INCLUDE_PX14400_SERVER FALSE CACHE BOOL "Flag to include the px14400 server; ignored unless the build-server flag is enabled" )
set( Mantis_INCLUDE_ROACH_SERVER FALSE CACHE BOOL "Flag to include the roach server; ignored unless the build-server flag is enabled" )

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
    execute_process( COMMAND ${GIT_EXECUTABLE} rev-parse -q HEAD  OUTPUT_VARIABLE Mantis_GIT_COMMIT  OUTPUT_STRIP_TRAILING_WHITESPACE )
    execute_process( COMMAND ${GIT_EXECUTABLE} describe --tags --long  OUTPUT_VARIABLE Mantis_GIT_DESCRIBE  OUTPUT_STRIP_TRAILING_WHITESPACE )
endif( GIT_FOUND )


############
# RapidJSON (as included with Mantis)
############

include_directories ("${PROJECT_SOURCE_DIR}/RapidJSON")


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
include_directories( 
    ${PROJECT_SOURCE_DIR}/Monarch/Source
    ${PROJECT_SOURCE_DIR}/Monarch/libthorax/cpp
    ${PROJECT_SOURCE_DIR}/Monarch/libthorax
    ${PROJECT_BINARY_DIR}/Monarch
    ${PROJECT_BINARY_DIR}/Monarch/Protobuf 
)


#############
# Digitizers
#############

if( Mantis_BUILD_SERVER )
    if( Mantis_INCLUDE_PX1500_SERVER )
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
        message( STATUS "Will include the PX1500 server" )
    endif (Mantis_INCLUDE_PX1500_SERVER)

    if( Mantis_INCLUDE_PX14400_SERVER )
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
        message( STATUS "Will include the PX14400 server" )
    endif (Mantis_INCLUDE_PX14400_SERVER)
    
    if( Mantis_INCLUDE_ROACH_SERVER )
        add_subdirectory( katcp_devel )
        pbuilder_add_ext_libraries( ${katcp_devel_LIBRARIES} )
        # add to the RPATH to be used when installing, but only if it's not a system directory
        list( FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${katcp_devel_LIBRARIES}" isSystemDir )
        if( "${isSystemDir}" STREQUAL "-1" )
            list( APPEND CMAKE_INSTALL_RPATH ${katcp_devel_LIBRARIES} )
        endif( "${isSystemDir}" STREQUAL "-1" )
        message( Status "Katcp:${katcp_devel_LIBRARIES}")
        include_directories( ${PROJECT_SOURCE_DIR}/katcp_devel/katcp )
    endif( Mantis_INCLUDE_ROACH_SERVER )
else( Mantis_BUILD_SERVER )
    message( STATUS "Will perform a client-only build")
endif( Mantis_BUILD_SERVER )


###########
# Protobuf
###########

find_package( Protobuf )
include_directories( ${PROTOBUF_INCLUDE_DIR} )
pbuilder_add_ext_libraries( ${PROTOBUF_LIBRARIES} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/Protobuf )
add_subdirectory( Protobuf )


##########
# Threads
##########

find_package( Threads )
pbuilder_add_ext_libraries( ${CMAKE_THREAD_LIBS_INIT} )


########
# Boost as included with mantis
########

include_directories (
    ${PROJECT_SOURCE_DIR}
)

add_subdirectory (boost)

pbuilder_add_ext_libraries( boost_atomic )


################
# Build Mantis #
################

add_subdirectory( Client )
add_subdirectory( Server )
add_subdirectory( Applications )
