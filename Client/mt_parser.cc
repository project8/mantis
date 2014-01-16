#include "mt_parser.hh"

#include <iostream>

namespace mantis
{

    parser::parser( int an_argc, char** an_argv ) :
            config_value_object()
    {
        parse(an_argc, an_argv);
    }

    parser::~parser()
    {
    }

    void parser::parse( int an_argc, char** an_argv )
    {
        for( int t_index = 1; t_index < an_argc; t_index++ )
        {
            //t_argument.assign( an_argv[ t_index ] );
            std::string t_argument( an_argv[ t_index ] );
            size_t t_val_pos = t_argument.find_first_of( f_separator );
            if( t_val_pos != std::string::npos )
            {
                std::string t_name(t_argument.substr( 0, t_val_pos ));

                config_value_data* new_data = new config_value_data();
                *new_data << t_argument.substr( t_val_pos + 1 );

                //std::cout << "(parser) adding < " << t_name << "<" << t_type << "> > = <" << new_data.value() << ">" << std::endl;

                this->replace( t_name, new_data );

                continue;
            }

            throw exception() << "argument <" << t_argument << "> does not match <name>=<value> pattern";
        }

        return;
    }


}
