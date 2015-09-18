/*
 * mt_uuid.hh
 *
 *  Created on: Sep 16, 2015
 *      Author: nsoblath
 */

#ifndef MT_UUID_HH_
#define MT_UUID_HH_

//#include "mt_singleton.hh"
//#include "mt_destroyer.hh"

#include "mt_constants.hh"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp> // to allow streaming of uuid_t

#include <string>

namespace mantis
{
    typedef boost::uuids::uuid uuid_t;

    uuid_t MANTIS_API generate_random_uuid();
    uuid_t MANTIS_API generate_nil_uuid();

    uuid_t MANTIS_API uuid_from_string( const std::string& a_id_str );
    uuid_t MANTIS_API uuid_from_string( const char* a_id_str );

    uuid_t MANTIS_API uuid_from_string( const std::string& a_id_str, bool& a_valid_flag );
    uuid_t MANTIS_API uuid_from_string( const char* a_id_str, bool& a_valid_flag );

    std::string MANTIS_API string_from_uuid( const uuid_t& a_id );

/*
    class uuid_factory : public singleton< uuid_factory >
    {
        public:
            uuid_t generate_random();
            uuid_t generate_nil();

        private:


        public:
            uuid_t from_string( const std::string& a_id_str );

            std::string to_string( const uuid_t& a_id );


        protected:
            friend class singleton< uuid_factory >;
            friend class destroyer< uuid_factory >;
            uuid_factory();
            ~uuid_factory();
    };
*/



} /* namespace mantis */

#endif /* MT_UUID_HH_ */
