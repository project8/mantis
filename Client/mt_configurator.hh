/*
 * mt_configurator.hh
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#ifndef MT_CONFIGURATOR_HH_
#define MT_CONFIGURATOR_HH_

#include "mt_config_node.hh"

#include "mt_exception.hh"

#include <string>

namespace mantis
{

    class configurator
    {
        public:
            configurator( int an_argc, char** an_argv, config_value_object* a_default = NULL );
            virtual ~configurator();

            config_value_object& config();
            const config_value_object& config() const;

            template< typename XReturnType >
            XReturnType get( const std::string& a_name );

            template< typename XReturnType >
            XReturnType get( const std::string& a_name, XReturnType a_default );

        private:
            config_value_object f_master_config;

            mutable config_value* f_config_value_buffer;

            std::string f_string_buffer;
    };

    template< typename XReturnType >
    XReturnType configurator::get( const std::string& a_name )
    {
        f_config_value_buffer = f_master_config.at( a_name );
        if( f_config_value_buffer->is_data() )
        {
            return static_cast< config_value_data* >( f_config_value_buffer )->value< XReturnType >();
        }
        throw exception() << "configurator does not have a value for <" << a_name << ">";
    }

    template< typename XReturnType >
    XReturnType configurator::get( const std::string& a_name, XReturnType a_default )
    {
        f_config_value_buffer = f_master_config.at( a_name );
        if( f_config_value_buffer != NULL && f_config_value_buffer->is_data() )
        {
            return static_cast< config_value_data* >( f_config_value_buffer )->value< XReturnType >();
        }
        return a_default;

    }


} /* namespace mantis */
#endif /* MT_CONFIGURATOR_HH_ */
