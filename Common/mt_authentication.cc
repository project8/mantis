/*
 * mt_authentication.cc
 *
 *  Created on: Jun 15, 2015
 *      Author: nsoblath
 */

#include "mt_authentication.hh"

#include "logger.hh"
#include "param_json.hh"

#include <boost/filesystem.hpp>

#ifndef _WIN32
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h> // for getlogin_r, sysconf
#else
#include <Windows.h>
#endif

using scarab::param_input_json;

using namespace boost::filesystem;
using std::string;

namespace mantis
{

    LOGGER( mtlog, "authentication" );

    authentication::authentication() :
                    param_node(),
                    f_auth_filename( ".project8_authentications.json" ),
                    f_is_loaded( false )
    {
        // get the username
        string t_username;
        const size_t t_uname_bufsize = 1024;
        char t_username_buf[ t_uname_bufsize ];
#ifndef _WIN32
        if( getlogin_r( t_username_buf, t_uname_bufsize ) == 0 )
#else
        DWORD t_bufsize_win = t_uname_bufsize;
        if( GetUserName( t_username_buf, &t_bufsize_win ) )
#endif
        {
            t_username = string( t_username_buf );
        }
        else
        {
            WARN( mtlog, "Unable to get the username; authentications not loaded" );
        }

        // get the user home directory
        path t_auth_file_path;
        if( ! t_username.empty() )
        {
            string t_username = string( t_username_buf );

#ifndef _WIN32
            long t_pwd_bufsize = sysconf( _SC_GETPW_R_SIZE_MAX );
            if ( t_pwd_bufsize == -1 )  t_pwd_bufsize = 16384; // Should be more than enough

            char* t_pwd_buf = ( char* )malloc( (size_t)t_pwd_bufsize );
            if( t_pwd_buf == NULL )
            {
                WARN( mtlog, "Unable to allocate the pwd buffer; authentications not loaded")
            }
            else
            {
                struct passwd t_pwd;
                struct passwd* t_pwd_result;
                getpwnam_r( t_username.c_str(), &t_pwd, t_pwd_buf, t_pwd_bufsize, &t_pwd_result );
                if( t_pwd_result == NULL )
                {
                    WARN( mtlog, "Unable to get the pwd data; authentications not loaded" );
                }
                else
                {
                    t_auth_file_path = string( t_pwd.pw_dir ) / f_auth_filename;
                }
            }
#else
            t_auth_file_path = path( getenv( "HOMEDRIVE" ) ) / path( getenv( "HOMEPATH" ) ) / f_auth_filename;
#endif
        }

        // is the file there?
        bool t_auth_file_present = false;
        if( ! t_auth_file_path.empty() )
        {
            DEBUG( mtlog, "Looking for authentication file: <" << t_auth_file_path << ">" );
            try
            {
                t_auth_file_present = exists( t_auth_file_path ) && is_regular_file( t_auth_file_path );
                if( ! t_auth_file_present )
                {
                    WARN( mtlog, "File either doesn't exist (" << exists( t_auth_file_path ) << ") or isn't a regular file (" << is_regular_file( t_auth_file_path ) << ")" );
                }
            }
            catch( filesystem_error& e )
            {
                WARN( mtlog, "Unable to determine if the authentication file is a regular file: " << e.what() );
            }
        }

        // if so, load it
        if( t_auth_file_present )
        {
            param_node* t_read_file = param_input_json::read_file( t_auth_file_path.string() );
            if( t_read_file == NULL )
            {
                WARN( mtlog, "Unable to parse authentication file" );
            }
            else
            {
                this->param_node::operator=( *t_read_file );
                delete t_read_file;
                DEBUG( mtlog, "Authentications:\n" << *this );
                f_is_loaded = true;
            }
        }
        else
        {
            WARN( mtlog, "Authentications not loaded" );
        }
    }

    authentication::~authentication()
    {
    }

} /* namespace mantis */
