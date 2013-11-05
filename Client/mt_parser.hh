#ifndef MT_PARSER_HH_
#define MT_PARSER_HH_

#include "mt_exception.hh"

#include <map>
#include <string>
#include <sstream>

namespace mantis
{

    class parser
    {
        public:
            typedef std::map< std::string, std::string > item_map;
            typedef item_map::const_iterator imap_cit;

        public:
            parser( int an_argc, char** an_argv );
            virtual ~parser();

            void parse( int an_argc, char** an_argv );
            void clear();

            imap_cit begin() const;
            imap_cit end() const;

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
            item_map f_map;
            static const char f_separator = '=';
            static const size_t f_npos = std::string::npos;
    };

}

#endif
