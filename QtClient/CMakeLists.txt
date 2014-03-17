# QtClient CMakeLists
# Author: N. Oblath
# Created: Mar 17, 2014

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set( client_gui_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cc  
    ${CMAKE_CURRENT_SOURCE_DIR}/mtq_client_exe_gui.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mtq_run_client.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mtq_signaling_streambuf.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mtq_single_client_window.cc
)
set( client_gui_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/mtq_client_exe_gui.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mtq_run_client.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mtq_signaling_streambuf.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/mtq_single_client_window.hh
)
set( client_gui_FORMS
    ${CMAKE_CURRENT_SOURCE_DIR}/client_exe_gui.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/single_client_window.ui
)
set( client_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mantis-gui.qrc )

QT4_WRAP_CPP( client_gui_HEADERS_MOC ${client_gui_HEADERS} )
QT4_WRAP_UI( client_gui_FORMS_HEADERS ${client_gui_FORMS} )
QT4_ADD_RESOURCES( client_gui_RESOURCES_RCC ${client_gui_RESOURCES} )

add_executable( mantis_client_gui
    ${client_gui_SOURCES}
    ${client_gui_HEADERS_MOC}
    ${client_gui_FORMS_HEADERS}
    ${client_gui_RESOURCES_RCC}
)
target_link_libraries( mantis_client_gui MantisClient ${EXTERNAL_LIBRARIES} ${QT_LIBRARIES} )
pbuilder_install_executables( mantis_client_gui )