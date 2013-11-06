#ifndef MT_PARSER_HH_
#define MT_PARSER_HH_

#include "mt_configuration.hh"

#include "mt_exception.hh"

#include <map>
#include <string>
#include <sstream>

namespace mantis
{

    class parser : public configuration
    {
        public:
            parser( int an_argc, char** an_argv );
            virtual ~parser();

            void parse( int an_argc, char** an_argv );

        private:
            static const char f_separator = '=';
            static const char f_type_sep = '/';
            static const char f_type_bool = 'b';
            static const char f_type_int = 'i';
            static const char f_type_uint = 'u';
            static const char f_type_double = 'd';
            static const size_t f_npos = std::string::npos;

        public:

            //TODO: remove get_required and get_optional, and f_map
            /**/ // temporarily still here
            template< class x_type >
            x_type get_required( const std::string& a_name )
            {
                std::stringstream t_converter;
                x_type t_value;

                std::map< std::string, std::string >::iterator t_it = f_map.find( a_name );
                if( t_it != f_map.end() )
                {
                    t_converter << t_it->second;
                    t_converter >> t_value;
                }
                else
                {
                    throw exception() << "argument name <" << a_name << "> not found";
                }

                return t_value;
            }

            template< class x_type >
            x_type get_optional( const std::string& a_name, const x_type& a_default )
            {
                std::stringstream t_converter;
                x_type t_value = a_default;

                std::map< std::string, std::string >::iterator t_it = f_map.find( a_name );
                if( t_it != f_map.end() )
                {
                    t_converter << t_it->second;
                    t_converter >> t_value;
                }

                return t_value;
            }

        private:
            std::map< std::string, std::string > f_map;
            /**/
    };

}

#endif
