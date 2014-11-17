set( DIGITIZER_HEADERFILES )
set( DIGITIZER_SOURCEFILES )
set( DIGITIZER_DIR ${PROJECT_SOURCE_DIR}/Server/Devices )

macro( add_digitizer DIGITIZER_NAME )
	# Macro to add a digitizer to the Mantis server.
	# The CMake configuration will expect a file in Server/Devices called Digitizer[DIGITIZER_NAME].cmake .
	# A cached CMake variable called Mantis_INCLUDE_[DIGITIZER_NAME]_DIGITIZER will be created with the default value FALSE.
	# If you want to change the default value of that variable, give "TRUE" as a second argument to this macro (not including the quotes).

	set( DIGITIZER_FLAG Mantis_INCLUDE_${DIGITIZER_NAME}_DIGITIZER )
	set( DEFAULT_FLAG_VALUE FALSE )
	if( ${ARGC} GREATER 1 )
		set( DEFAULT_FLAG_VALUE ${ARGV1} )
	endif()

    set( ${DIGITIZER_FLAG} ${DEFAULT_FLAG_VALUE} CACHE BOOL "Flag to build the ${DIGITIZER_NAME} server; only used if the Mantis_BUILD_SERVER flag is enabled" )

    if( Mantis_BUILD_SERVER )
        if( ${DIGITIZER_FLAG} )
            include( Digitizer${DIGITIZER_NAME} )
        endif( ${DIGITIZER_FLAG} )
    endif( Mantis_BUILD_SERVER )
endmacro()

macro( add_digitizer_header_files )
	foreach( HEADERFILE IN ITEMS ${ARGN} )
		#message( STATUS "appending ${DIGITIZER_DIR}/${HEADERFILE}" )
		list( APPEND DIGITIZER_HEADERFILES ${DIGITIZER_DIR}/${HEADERFILE} )
	endforeach()
endmacro()

macro( add_digitizer_source_files )
	foreach( SOURCEFILE IN ITEMS ${ARGN} )
		#message( STATUS "appending ${DIGITIZER_DIR}/${SOURCEFILE}" )
		list( APPEND DIGITIZER_SOURCEFILES ${DIGITIZER_DIR}/${SOURCEFILE} )
	endforeach()
endmacro()