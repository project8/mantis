#include "mt_parser.hh"

namespace mantis
{

    parser::parser( int an_argc, char** an_argv ) :
            f_map()
    {
        size_t t_pos;
        std::string t_argument;
        std::string t_name;
        std::string t_value;

        for( int t_index = 1; t_index < an_argc; t_index++ )
        {
            t_argument.assign( an_argv[ t_index ] );
            t_pos = t_argument.find_last_of( f_separator );
            if( t_pos != std::string::npos )
            {
                t_name = t_argument.substr( 0, t_pos );
                t_value = t_argument.substr( t_pos + 1 );
                f_map[ t_name ] = t_value;
                continue;
            }

            throw exception() << "argument <" << t_argument << "> does not match <name>=<value> pattern";
        }
    }

    parser::~parser()
    {
    }

}
