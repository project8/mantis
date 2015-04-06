#ifndef MT_PARSER_HH_
#define MT_PARSER_HH_

#include "mt_param.hh"
#include "mt_param_json.hh"

#include <string>

namespace mantis
{
    class MANTIS_API parsable : public param_node
    {
        public:
            parsable( const std::string& a_addr_with_value );
            parsable( const std::string& a_addr, const std::string& a_value );
            ~parsable();

        private:
            void add_next( param_node* a_parent, const std::string& a_addr, const std::string& a_value );

        public:
            static const char f_value_separator = '=';
            static const char f_node_separator = '.';


    };


    class MANTIS_API parser : public param_node
    {
        public:
            parser( int an_argc, char** an_argv );
            virtual ~parser();

            void parse( int an_argc, char** an_argv );
    };

}

#endif
