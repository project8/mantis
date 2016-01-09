#ifndef MT_PARSER_HH_
#define MT_PARSER_HH_

#include "mt_constants.hh"

#include "param.hh"

#include <string>

namespace mantis
{
    using scarab::param;
    using scarab::param_node;

    class MANTIS_API parser : public param_node
    {
        public:
            parser( int an_argc, char** an_argv );
            virtual ~parser();

            void parse( int an_argc, char** an_argv );
    };

}

#endif
