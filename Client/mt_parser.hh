#ifndef MT_PARSER_HH_
#define MT_PARSER_HH_

#include "mt_param.hh"

#include "mt_exception.hh"

#include <map>
#include <string>
#include <sstream>

namespace mantis
{

    class parser : public param_node
    {
        public:
            parser( int an_argc, char** an_argv );
            virtual ~parser();

            void parse( int an_argc, char** an_argv );

        private:
            static const char f_separator = '=';
            static const size_t f_npos = std::string::npos;

    };

}

#endif
