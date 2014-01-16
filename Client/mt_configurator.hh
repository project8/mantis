/*
 * mt_configurator.hh
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#ifndef MT_CONFIGURATOR_HH_
#define MT_CONFIGURATOR_HH_

#include "mt_config_node.hh"

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
            XReturnType get_required_value( const std::string& a_name );

            template< typename XReturnType >
            XReturnType get_optional_value( const std::string& a_name, XReturnType a_default );

            bool get_bool_required( const std::string& a_name );
            bool get_bool_optional( const std::string& a_name, bool a_default );

            int get_int_required( const std::string& a_name );
            int get_int_optional( const std::string& a_name, int a_default );

            unsigned get_uint_required( const std::string& a_name );
            unsigned get_uint_optional( const std::string& a_name, unsigned a_default );

            double get_double_required( const std::string& a_name );
            double get_double_optional( const std::string& a_name, double a_default );

            const std::string& get_string_required( const std::string& a_name );
            const std::string& get_string_optional( const std::string& a_name, const std::string& a_default );

            void show();

        private:
            config_value_object f_master_config;

            mutable config_value* f_config_value_buffer;

            std::string f_string_buffer;
    };

    template< typename XReturnType >
    XReturnType configurator::get_required_value( const std::string& a_name )
    {
        f_config_value_buffer = f_master_config.at( a_name );
        if( f_config_value_buffer->is_data() )
        {
            return static_cast< config_value_data* >( f_config_value_buffer )->value< XReturnType >();
        }
        throw exception() << "configurator does not have a value for <" << a_name << ">";
    }

    template< typename XReturnType >
    XReturnType configurator::get_optional_value( const std::string& a_name, XReturnType a_default )
    {
        f_config_value_buffer = f_master_config.at( a_name );
        if( f_config_value_buffer->is_data() )
        {
            return static_cast< config_value_data* >( f_config_value_buffer )->value< XReturnType >();
        }
        return a_default;

    }


} /* namespace mantis */
#endif /* MT_CONFIGURATOR_HH_ */
