#include "mt_parser.hh"

#include <iostream>

namespace mantis
{

    parser::parser( int an_argc, char** an_argv )
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
                std::string t_value(t_argument.substr( t_val_pos + 1 ));
                size_t t_type_pos = t_name.find_first_of( f_type_sep );
                char t_type = 's';
                if( t_type_pos != std::string::npos )
                {
                    t_type = t_name[ t_type_pos + 1 ];
                    t_name.assign( t_name.substr( 0, t_type_pos ) );
                }
                //std::cout << "(parser) adding < " << t_name << "<" << t_type << "> > = <" << t_value << ">" << std::endl;

                rapidjson::Value t_new_value;
                std::stringstream t_converter;
                t_converter << t_value;
                switch( t_type )
                {
                    case f_type_bool:
                        bool b_val;
                        t_converter >> b_val;
                        t_new_value.SetBool( b_val );
                        break;
                    case f_type_int:
                        int i_val;
                        t_converter >> i_val;
                        t_new_value.SetInt( i_val );
                        break;
                    case f_type_uint:
                        unsigned u_val;
                        t_converter >> u_val;
                        t_new_value.SetUint( u_val );
                        break;
                    case f_type_double:
                        double d_val;
                        t_converter >> d_val;
                        t_new_value.SetDouble( d_val );
                        break;
                    default:
                        t_new_value.SetString( t_value.c_str(), t_value.size(), GetAllocator() );
                        break;
                }
                RemoveMember( t_name.c_str() );
                AddMember( t_name.c_str(), GetAllocator(), t_new_value, GetAllocator() );
                //print();

                continue;
            }

            throw exception() << "argument <" << t_argument << "> does not match <name>=<value> pattern";
        }

        return;
    }


}
