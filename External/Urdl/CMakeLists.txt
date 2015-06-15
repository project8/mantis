# Minimum cmake verison 3.0 required by the way CMake handles rpath information on Mac OSX.
cmake_minimum_required (VERSION 3.0)

#########
# setup #
#########

cmake_policy( SET CMP0048 NEW ) # version in project()
project( Urdl VERSION 3.0.0 )

list( APPEND CMAKE_MODULE_PATH ${Mantis_SOURCE_DIR}/Scarab/cmake )

include( PackageBuilder )

pbuilder_prepare_project()

include_directories( BEFORE 
    ${PROJECT_SOURCE_DIR}/include
)

set( CMAKE_MACOSX_RPATH TRUE )

set ( BUILD_SHARED_LIBS ON )

# TODO: on windows, may need to add libraries ws2_32, and mswsock

# turn off SSL to simplify
add_definitions( -DURDL_DISABLE_SSL=1 )

########
# Boost
########

# Boost (1.46 required for filesystem version 3)
find_package( Boost 1.46.0 REQUIRED COMPONENTS system date_time )
# make sure dynamic linking is assumed for all boost libraries
add_definitions( -DBOOST_ALL_DYN_LINK )
include_directories( ${Boost_INCLUDE_DIRS} )
pbuilder_add_ext_libraries( ${Boost_LIBRARIES} )


########
# Build
########

set( URDL_HEADERFILES
    include/urdl/http.hpp
    include/urdl/istream.hpp
    include/urdl/istreambuf.hpp
    include/urdl/option_set.hpp
    include/urdl/read_stream.hpp
    include/urdl/url.hpp
    include/urdl/detail/abi_prefix.hpp
    include/urdl/detail/abi_suffix.hpp
    include/urdl/detail/config.hpp
    include/urdl/detail/connect.hpp
    include/urdl/detail/coroutine.hpp
    include/urdl/detail/file_read_stream.hpp
    include/urdl/detail/handshake.hpp
    include/urdl/detail/http_read_stream.hpp
    include/urdl/detail/parsers.hpp
    include/urdl/detail/scoped_ptr.hpp
    include/urdl/impl/http.ipp
    include/urdl/impl/istreambuf.ipp
    include/urdl/impl/option_set.ipp
    include/urdl/impl/url.ipp
)

set( URDL_SOURCEFILES
    src/urdl.cpp
)

add_library( Urdl ${URDL_SOURCEFILES} )
target_link_libraries( Urdl ${EXTERNAL_LIBRARIES} )

pbuilder_install_headers( ${URDL_HEADERFILES} )
pbuilder_install_libraries( Urdl )
