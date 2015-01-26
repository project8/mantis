/*
 * mt_configurator.hh
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#ifndef MT_CONFIGURATOR_HH_
#define MT_CONFIGURATOR_HH_

#include "mt_param.hh"

#include "mt_exception.hh"

#include <string>

namespace mantis
{

    class configurator
    {
        public:
            configurator( int an_argc, char** an_argv, param_node* a_default = NULL );
            virtual ~configurator();

            const std::string& exe_name() const;

            param_node& config();
            const param_node& config() const;

            template< typename XReturnType >
            XReturnType get( const std::string& a_name ) const;

            template< typename XReturnType >
            XReturnType get( const std::string& a_name, XReturnType a_default ) const;

        private:
            std::string f_exe_name; // the name of the executable being used

            param_node* f_master_config;

            mutable param* f_param_buffer;

            std::string f_string_buffer;
    };

    template< typename XReturnType >
    XReturnType configurator::get( const std::string& a_name ) const
    {
        f_param_buffer = const_cast< param* >( f_master_config->at( a_name ) );
        if( f_param_buffer != NULL && f_param_buffer->is_value() )
        {
            return f_param_buffer->as_value().get< XReturnType >();
        }
        throw exception() << "configurator does not have a value for <" << a_name << ">";
    }

    template< typename XReturnType >
    XReturnType configurator::get( const std::string& a_name, XReturnType a_default ) const
    {
        f_param_buffer = const_cast< param* >( f_master_config->at( a_name ) );
        if( f_param_buffer != NULL && f_param_buffer->is_value() )
        {
            return f_param_buffer->as_value().get< XReturnType >();
        }
        return a_default;

    }


} /* namespace mantis */
#endif /* MT_CONFIGURATOR_HH_ */
