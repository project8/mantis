/*
 * mt_configuration.cc
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#include "mt_configuration.hh"

#include "filestream.h"
#include "prettywriter.h"

#include <cstring>
using std::string;

namespace mantis
{

    configuration::configuration() :
             rapidjson::Document()
    {
        SetObject();
    }

    configuration::~configuration()
    {
    }

    configuration& configuration::operator+=( configuration& rhs )
    {
        for( MemberIterator rhs_it = rhs.MemberBegin(); rhs_it != rhs.MemberEnd(); ++rhs_it )
        {
            //std::cout << "(configuration operator+=) adding: " << rhs_it->name.GetString() << std::endl;
            this->RemoveMember( rhs_it->name.GetString() );
            this->AddMember( rhs_it->name, rhs_it->value, GetAllocator() );
        }
        return *this;
    }

    void configuration::print() const
    {
        rapidjson::FileStream config_stream(stdout);
        rapidjson::PrettyWriter< rapidjson::FileStream > writer(config_stream);
        Accept(writer);
        std::cout << std::endl;
        return;
    }

} /* namespace mantis */
