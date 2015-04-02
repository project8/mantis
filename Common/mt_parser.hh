#ifndef MT_PARSER_HH_
#define MT_PARSER_HH_

#include "mt_param.hh"
#include "mt_param_json.hh"

#include <string>

namespace mantis
{
    class MANTIS_API cl_arg : public param_node
    {
        public:
            cl_arg( const std::string& a_addr, const std::string& a_value );
            ~cl_arg();

        private:
            void add_next( param_node* a_parent, const std::string& a_addr, const std::string& a_value );
    };


    class MANTIS_API parser : public param_node
    {
        public:
            parser( int an_argc, char** an_argv );
            virtual ~parser();

            void parse( int an_argc, char** an_argv );

        private:
            static const char f_separator = '=';

    };

}

#endif
