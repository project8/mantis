# QtClient CMakeLists
# Author: N. Oblath
# Created: Mar 17, 2014
# Based largely on CMake's QtDialog (for building cmake-gui)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

find_package( Qt5Widgets QUIET )
if ( Qt5Widgets_FOUND )
    include_directories( ${Qt5Widgets_INCLUDE_DIRS} )
    add_definitions( ${Qt5Widgets_DEFINITONS} )
    macro( qt4_wrap_ui )
        qt5_wrap_ui( ${ARGN} )
    endmacro()
     macro( qt4_wrap_cpp )
        qt5_wrap_cpp( ${ARGN} )
    endmacro()
    macro( qt4_add_resources )
        qt5_add_resources( ${ARGN} )
    endmacro()
    set( CMake_QT_LIBRARIES ${Qt5Widgets_LIBRARIES} )
    set( QT_QTMAIN_LIBRARY Qt5::WinMain )

    # Remove this when the minimum version of Qt is 4.6.
    add_definitions( -DQT_DISABLE_DEPRECATED_BEFORE=0 )

    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}" )
    
else( Qt5Widgets_FOUND )
    set( QT_MIN_VERSION "4.2.0" )
    find_package( Qt4 REQUIRED )
    if( NOT QT4_FOUND )
        message( SEND_ERROR "Failed to find Qt 4.2 or greater." )
        return()
    endif( NOT QT4_FOUND )

    include( ${QT_USE_FILE} )

    set( CMake_QT_LIBRARIES ${QT_LIBRARIES} )
endif( Qt5Widgets_FOUND )

set( client_gui_SOURCES 
    main.cc  
    mtq_client_exe_gui.cc
    mtq_full_file_dialog.cc
    mtq_run_client.cc
    mtq_signaling_streambuf.cc
    mtq_simple_file_dialog.cc
    mtq_single_client_window.cc
)
set( client_gui_HEADERS
    mtq_client_exe_gui.hh
    mtq_full_file_dialog.hh
    mtq_run_client.hh
    mtq_signaling_streambuf.hh
    mtq_simple_File_dialog.hh
    mtq_single_client_window.hh
)
set( client_gui_FORMS
    client_exe_gui.ui
    filename_dialog.ui
    single_client_window.ui
)
set( client_RESOURCES 
    mantis-gui.qrc 
)

QT4_WRAP_CPP( client_gui_HEADERS_MOC ${client_gui_HEADERS} )
QT4_WRAP_UI( client_gui_FORMS_HEADERS ${client_gui_FORMS} )
QT4_ADD_RESOURCES( client_gui_RESOURCES_RCC ${client_gui_RESOURCES} )

set( client_gui_ALL_SOURCES 
    ${client_gui_SOURCES} 
    ${client_gui_HEADERS} 
    ${client_gui_HEADERS_MOC} 
    ${client_gui_FORMS_HEADERS} 
    ${client_gui_RESOURCES_RCC}
)
if( APPLE )
    set( client_gui_ALL_SOURCES ${client_gui_ALL_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/mantis_head.icns )
    set( MACOSX_BUNDLE_ICON_FILE mantis_head.icns )
    set_source_files_properties( ${CMAKE_CURRENT_SOURCE_DIR}/mantis_head.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
endif( APPLE )

add_executable( mantis_client_gui MACOSX_BUNDLE
    ${client_gui_ALL_SOURCES}
)
#    ${client_gui_HEADERS_MOC}
#    ${client_gui_FORMS_HEADERS}
#    ${client_gui_RESOURCES_RCC}
#)
target_link_libraries( mantis_client_gui MantisClient ${EXTERNAL_LIBRARIES} ${QT_LIBRARIES} )

if( APPLE )
    #file( STRINGS "${CMake_SOURCE_DIR}/Copyright.txt" copyright_line
        #LIMIT_COUNT 1 REGEX "^Copyright 2000-20[0-9][0-9] Kitware" )

    set_target_properties( mantis_client_gui PROPERTIES
        OUTPUT_NAME MTClient
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${Mantis_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${Mantis_FULL_VERSION}"
        #MACOSX_BUNDLE_COPYRIGHT "${copyright_line}"
    )

    # Create a symlink in the build tree to provide a "mantis_client_gui" next
    # to the "mantis_client" executable that refers to the application bundle.
    add_custom_command( TARGET mantis_client_gui POST_BUILD
        COMMAND ln -sf MTClient.app/Contents/MacOS/MTClient
                   $<TARGET_FILE_DIR:mantis_client>/mantis_client_gui
    )
endif( APPLE )

set( plugin_dest_dir ${BIN_INSTALL_SUBDIR} )
set( qtconf_dest_dir ${BIN_INSTALL_SUBDIR} )
set( APPS "\${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_SUBDIR}/mantis_client_gui" )
if( APPLE )
    set( plugin_dest_dir ${BIN_INSTALL_SUBDIR}/MTClient.app/Contents/MacOS )
    set( qtconf_dest_dir ${BIN_INSTALL_SUBDIR}/MTClient.app/Contents/Resources )
    set( APPS "\${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_SUBDIR}/MTClient.app" )
endif( APPLE )

install( TARGETS mantis_client_gui 
    RUNTIME DESTINATION ${BIN_INSTALL_DIR} COMPONENT Runtime
    BUNDLE DESTINATION ${BIN_INSTALL_DIR} COMPONENT Runtime
)

install( DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime )

install( CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
    " COMPONENT Runtime )
    
set( DIRS ${QT_LIBRARY_DIRS} )

install( CODE "
    file( GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\" )
    include( BundleUtilities )
    fixup_bundle( \"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\" )
    " COMPONENT Runtime )

if( UNIX )
    # install a desktop file so CMake appears in the application start menu
    # with an icon
    #install(FILES CMake.desktop DESTINATION share/applications )
    #install(FILES CMakeSetup32.png DESTINATION share/pixmaps )
    #install(FILES cmakecache.xml DESTINATION share/mime/packages )
endif( UNIX )

if( APPLE )
    #set(CMAKE_POSTFLIGHT_SCRIPT
    #    "${CMake_BINARY_DIR}/Source/QtDialog/postflight.sh")
    #set(CMAKE_POSTUPGRADE_SCRIPT
    #    "${CMake_BINARY_DIR}/Source/QtDialog/postupgrade.sh")
    #configure_file("${CMake_SOURCE_DIR}/Source/QtDialog/postflight.sh.in"
    #    "${CMake_BINARY_DIR}/Source/QtDialog/postflight.sh")
    #configure_file("${CMake_SOURCE_DIR}/Source/QtDialog/postupgrade.sh.in"
    #    "${CMake_BINARY_DIR}/Source/QtDialog/postupgrade.sh")
    #install(CODE "execute_process(COMMAND ln -s \"../MacOS/CMake\" cmake-gui
    #            WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin)")
endif( APPLE )

if( APPLE OR WIN32 )
    # install rules for including 3rd party libs such as Qt
    # if a system Qt is used (e.g. installed in /usr/lib/), it will not be included in the installation
    #set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/mantis_client_gui${CMAKE_EXECUTABLE_SUFFIX}")
    #if(APPLE)
    #  set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/MacOS/MTClient")
    #endif()
    #install(CODE "
    #    include(\"${CMake_SOURCE_DIR}/Modules/BundleUtilities.cmake\")
    #    set(BU_CHMOD_BUNDLE_ITEMS ON)
    #    fixup_bundle(\"${fixup_exe}\" \"\" \"${QT_LIBRARY_DIR};${QT_BINARY_DIR}\")
    #")
endif( APPLE OR WIN32 )

set( CMAKE_PACKAGE_QTGUI TRUE )
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/QtDialogCPack.cmake.in"
    "${QtDialog_BINARY_DIR}/QtDialogCPack.cmake" @ONLY
)