/*
 * mt_configurator.hh
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#ifndef MT_CONFIGURATOR_HH_
#define MT_CONFIGURATOR_HH_

#include "mt_configuration.hh"

#include <string>

namespace mantis
{

    class configurator
    {
        public:
            configurator( int an_argc, char** an_argv, configuration* a_default = NULL );
            virtual ~configurator();

            configuration& config();
            const configuration& config() const;

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
            configuration f_master_config;

            std::string f_string_buffer;
    };

} /* namespace mantis */
#endif /* MT_CONFIGURATOR_HH_ */
