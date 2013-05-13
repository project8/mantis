####################
# get that gravy #
####################

cmake_minimum_required( VERSION 2.8 )
include( $ENV{HOME}/.dmake/dmake.cmake )

#########
# setup #
#########

dmake_project_begin( Mantis 0 1 0 )

#######################
# mantis dependencies #
#######################

dmake_dependency_declare( Monarch 0 1 0 )
dmake_dependency_foreign_declare( px1500 0 1 0 )

##################
# mantis library #
##################

dmake_library_declare( MantisCore . )
dmake_library_headers( MantisCore
    MantisActor.hpp
    MantisBufferBlock.hpp
    MantisBuffer.hpp
    MantisBufferIterator.hpp
    MantisBufferRecord.hpp
    MantisBufferState.hpp
    MantisCallable.hpp
    MantisCondition.hpp
    MantisEnv.hpp
    MantisExceptions.hpp
    MantisFileWriter.hpp
    MantisHandlers.hpp
    MantisLogger.hpp
    MantisMutex.hpp
    MantisPX1500.hpp
    MantisThread.hpp
    MantisTime.hpp
)
dmake_library_sources( MantisCore
    MantisActor.cpp
    MantisBufferBlock.cpp
    MantisBuffer.cpp
    MantisBufferIterator.cpp
    MantisBufferRecord.cpp
    MantisBufferState.cpp
    MantisCallable.cpp
    MantisCondition.cpp
    MantisEnv.cpp
    MantisExceptions.cpp
    MantisFileWriter.cpp
    MantisLogger.cpp
    MantisMutex.cpp
    MantisPX1500.cpp
    MantisThread.cpp
)

######################
# mantis executables #
######################

dmake_executable_declare( Mantis . )
dmake_executable_sources( Mantis
    Mantis.cpp
)

#######################
# mantis installation #
#######################

dmake_project_end()
