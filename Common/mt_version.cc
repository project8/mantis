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

namespace mantis
{
    MTLOGGER( mtlog, "version" );

    char version::s_delimeter = '.';

    version::version() :
            f_major_ver( Mantis_VERSION_MAJOR ),
            f_minor_ver( Mantis_VERSION_MINOR ),
            f_patch_ver( Mantis_VERSION_PATCH ),
            f_version()
    {
    }

    version::version( const std::string& a_ver ) :
                    f_major_ver( 0 ),
                    f_minor_ver( 0 ),
                    f_patch_ver( 0 ),
                    f_version()
    {
        parse( a_ver );
    }

    version::~version()
    {
    }

    unsigned version::major_version() const
    {
        return f_major_ver;
    }
    unsigned version::minor_version() const
    {
        return f_minor_ver;
    }
    unsigned version::patch_version() const
    {
        return f_patch_ver;
    }

    const std::string& version::version_str() const
    {
        return f_version;
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

} /* MT_VERSION_HH_ */


