# CMakeLists for mantis/boost
# Author: N. Oblath

set (BOOST_ATOMIC_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/lockpool.cpp
)


##################################################

add_library (boost_atomic ${BOOST_ATOMIC_SOURCEFILES})
#target_link_libraries (KatydidAnalysis ${KATYDID_LIBS} ${EXTERNAL_LIBRARIES})

pbuilder_install_libraries (boost_atomic)
#pbuilder_install_header_dirs (FILE_PATTERN "*.hpp" ${CMAKE_CURRENT_SOURCE_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${INCLUDE_INSTALL_DIR} FILES_MATCHING PATTERN "*.hpp")