/*
 * mt_version.cc
 *
 *  Created on: Mar 21, 2014
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_version.hh"

#include "mt_logger.hh"

#include <sstream>

#ifdef _WIN32
#include <Windows.h> // for extracting the exe name, gethostname and GetUserName
#elif __APPLE__
#include <mach-o/dyld.h> // for extracting the exe name
#include <unistd.h> // for gethostname and getlogin_r
#elif __linux
#include <unistd.h> // for readlink, gethostname and getlogin_r
#endif

using std::string;

namespace mantis
{
    MTLOGGER( mtlog, "version" );

    char version::s_delimeter = '.';

    version::version() :
            f_major_ver( Mantis_VERSION_MAJOR ),
            f_minor_ver( Mantis_VERSION_MINOR ),
            f_patch_ver( Mantis_VERSION_PATCH ),
            f_version( TOSTRING(Mantis_VERSION) ),
            f_package( TOSTRING(Mantis_PACKAGE_NAME) ),
            f_commit( TOSTRING(Mantis_GIT_COMMIT) ),
            f_exe_name(),
            f_hostname(),
            f_username()
    {
        update_exe_host_user();
    }

    version::version( const std::string& a_ver ) :
            f_major_ver( 0 ),
            f_minor_ver( 0 ),
            f_patch_ver( 0 ),
            f_version( a_ver ),
            f_package( TOSTRING(Mantis_PACKAGE_NAME) ),
            f_commit( TOSTRING(Mantis_GIT_COMMIT) ),
            f_exe_name(),
            f_hostname(),
            f_username()
    {
        parse( a_ver );
        update_exe_host_user();
    }

    version::~version()
    {
    }

    bool version::parse( const std::string& a_ver )
    {
        if( a_ver == "unknown" )
        {
            f_major_ver = 0;
            f_minor_ver = 0;
            f_patch_ver = 0;
            f_version = a_ver;
            return true;
        }

        size_t t_delim_pos_1 = a_ver.find( s_delimeter, 0 );
        if( t_delim_pos_1 == std::string::npos )
        {
            MTERROR( mtlog, "version string <" << a_ver << "> is not in the right format (did not find first delimeter)" );
            return false;
        }
        std::stringstream t_maj_ver_str;
        t_maj_ver_str << a_ver.substr( 0, t_delim_pos_1 );

        size_t t_delim_pos_2 = a_ver.find( s_delimeter, t_delim_pos_1 + 1 );
        if( t_delim_pos_2 == std::string::npos )
        {
            MTERROR( mtlog, "version string <" << a_ver << "> is not in the right format (did not find second delimeter)" );
            return false;
        }
        std::stringstream t_min_ver_str;
        t_min_ver_str << a_ver.substr(t_delim_pos_1 + 1, t_delim_pos_2 );

        std::stringstream t_patch_ver;
        t_patch_ver << a_ver.substr( t_delim_pos_2 + 1 );

        t_maj_ver_str >> f_major_ver;
        t_min_ver_str >> f_minor_ver;
        t_patch_ver >> f_patch_ver;
        f_version = a_ver;

        return true;
    }

    bool version::combine( unsigned a_maj_ver, unsigned a_min_ver, unsigned a_patch_ver )
    {
        std::stringstream t_ver_str;
        t_ver_str << a_maj_ver << s_delimeter << a_min_ver << s_delimeter << a_patch_ver;
        f_version = t_ver_str.str();
        return true;
    }

    bool version::update_exe_host_user()
    {
        const size_t t_bufsize = 1024;

        // Username
        char t_username_buf[ t_bufsize ];
#ifdef _WIN32
        DWORD t_bufsize_win = t_bufsize;
        if( GetUserName( t_username_buf, &t_bufsize_win ) )
#else
        if( getlogin_r( t_username_buf, t_bufsize ) == 0 )
#endif
        {
            f_username = string( t_username_buf );
        }
        else
        {
            MTWARN( mtlog, "Unable to get the username" );
        }

        // Hostname
        char t_hostname_buf[ t_bufsize ];
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
#endif
        // gethostname is the same on posix and windows
        if( gethostname( t_hostname_buf, t_bufsize ) == 0 )
        {
            f_hostname = string( t_hostname_buf );
        }
        else
        {
            MTWARN( mtlog, "Unable to get the hostname" );
        }
#ifdef _WIN32
        WSACleanup();
#endif

        // name of executable
        //f_exe_name = t_parser.get_value( t_name_exe, f_exe_name );
#ifdef _WIN32
        TCHAR t_exe_buf[ MAX_PATH ];
        if( ! GetModuleFileName( NULL, t_exe_buf, MAX_PATH ) )
#elif __APPLE__
        char t_exe_buf[ 2048 ];
        uint32_t t_exe_bufsize = sizeof( t_exe_buf );
        if( _NSGetExecutablePath( t_exe_buf, &t_exe_bufsize ) != 0 )
#elif __linux
        const size_t t_exe_bufsize = 2048;
        char t_exe_buf[ t_exe_bufsize ];
        if( readlink( "/proc/self/exe", t_exe_buf, t_exe_bufsize ) < 0 )
#endif
        {
            MTWARN( mtlog, "Could not retrieve executable file name" );
#ifdef __APPLE__
            MTWARN( mtlog, "Executable name buffer is too small; needs size %u\n" << t_bufsize );
#endif
        }
        f_exe_name = string( t_exe_buf );

        return true;
    }


    version_global::version_global() :
        version()
    {
    }

    version_global::~version_global()
    {
    }

} /* MT_VERSION_HH_ */


