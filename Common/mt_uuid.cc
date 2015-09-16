/*
 * mt_uuid.cc
 *
 *  Created on: Sep 16, 2015
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_uuid.hh"

#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>

#include <sstream>


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
        static std::stringstream t_conv;
        static uuid_t t_id;

        if( a_id_str.empty() ) return generate_nil_uuid();
        t_conv.str(a_id_str);
        t_conv >> t_id;
        return t_id;
    }

    std::string MANTIS_API string_from_uuid( const uuid_t& a_id )
    {
        return boost::uuids::to_string( a_id );
    }

    //uuid_factory::uuid_factory();
    //uuid_factory::~uuid_factory();



} /* namespace mantis */
