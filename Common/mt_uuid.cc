/*
 * mt_uuid.cc
 *
 *  Created on: Sep 16, 2015
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_uuid.hh"

#include "mt_exception.hh"

#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>

//#include <sstream>


namespace mantis
{
    uuid_t MANTIS_API generate_random_uuid()
    {
        static boost::uuids::random_generator t_gen;

        return t_gen();
    }

    uuid_t MANTIS_API generate_nil_uuid()
    {
        return boost::uuids::nil_uuid();
    }

    uuid_t MANTIS_API uuid_from_string( const std::string& a_id_str )
    {
        static boost::uuids::string_generator t_gen;

        if( a_id_str.empty() ) return generate_nil_uuid();

        try
        {
            return t_gen( a_id_str );
        }
        catch(...)
        {
            throw;
        }
    }

    uuid_t MANTIS_API uuid_from_string( const char* a_id_str )
    {
        static boost::uuids::string_generator t_gen;

        if( strcmp( a_id_str, "" ) == 0 ) return generate_nil_uuid();

        try
        {
            return t_gen( a_id_str );
        }
        catch(...)
        {
            throw;
        }
    }

    uuid_t MANTIS_API uuid_from_string( const std::string& a_id_str, bool& a_valid_flag )
    {
        a_valid_flag = true;
        try
        {
            return uuid_from_string( a_id_str );
        }
        catch(...)
        {
            a_valid_flag = false;
            return generate_nil_uuid();
        }
    }

    uuid_t MANTIS_API uuid_from_string( const char* a_id_str, bool& a_valid_flag )
    {
        a_valid_flag = true;
        try
        {
            return uuid_from_string( a_id_str );
        }
        catch(...)
        {
            a_valid_flag = false;
            return generate_nil_uuid();
        }
    }


    std::string MANTIS_API string_from_uuid( const uuid_t& a_id )
    {
        return boost::uuids::to_string( a_id );
    }

    //uuid_factory::uuid_factory();
    //uuid_factory::~uuid_factory();



} /* namespace mantis */
