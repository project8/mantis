/*
 * mt_configuration.cc
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#include "mt_configuration.hh"

#include <string>
using std::string;

namespace mantis
{

    configuration::configuration() :
             rapidjson::Document()
    {
    }

    configuration::~configuration()
    {
    }

    configuration& configuration::operator+=( configuration& rhs )
    {
        for( MemberIterator rhs_it = rhs.MemberBegin(); rhs_it != rhs.MemberEnd(); ++rhs_it )
        {
            MemberIterator this_it = MemberBegin();
            for( ; this_it != MemberEnd(); ++this_it )
            {
                if( string(this_it->name.GetString()) == string(rhs_it->name.GetString()) )
                {
                    this_it->value = rhs_it->value;
                    break;
                }
            }
            if( this_it == MemberEnd() )
            {
                AddMember( rhs_it->name, rhs_it->value, GetAllocator() );
            }
        }
        return *this;
    }

} /* namespace mantis */
