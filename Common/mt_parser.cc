#define MANTIS_API_EXPORTS

#include "mt_parser.hh"

#include "mt_exception.hh"
#include "mt_logger.hh"

#include <sstream>

namespace mantis
{
    MTLOGGER( mtlog, "parser" );

    parsable::parsable( const std::string& a_addr, const std::string& a_value ) :
            param_node()
    {
        add_next( this, a_addr, a_value );
    }

    parsable::~parsable()
    {
    }

    void parsable::add_next( param_node* a_parent, const std::string& a_addr, const std::string& a_value )
    {
        static const char t_separator = '.';
        size_t t_div_pos = a_addr.find( t_separator );
        if( t_div_pos == a_addr.npos )
        {
            // we've found the value; now check if it's a number or a string
            if( a_value.empty() )
            {
                a_parent->add( a_addr, new param() );
                MTDEBUG( mtlog, "Parsed CL value as NULL" );
            }
            // if "true" or "false", then bool
            if( a_value == "true" )
            {
                a_parent->add( a_addr, new param_value( true ) );
                MTDEBUG( mtlog, "Parsed CL value (" << a_value << ") as bool(true)" << *this );
            }
            else if( a_value == "false" )
            {
                a_parent->add( a_addr, new param_value( false ) );
                MTDEBUG( mtlog, "Parsed CL value (" << a_value << ") as bool(false):" << *this );
            }
            else
            {
                // test streaming to double as the most general test of whether the string is some sort of number
                double t_double;
                std::stringstream t_conv_double( a_value );
                if( ! (t_conv_double >> t_double).fail() )
                {
                    // now we know the value is numeric
                    if( a_value.find( '.' ) != std::string::npos )
                    {
                        // value is a floating-point number, since it has a decimal point
                        a_parent->add( a_addr, new param_value( t_double ) );
                        MTDEBUG( mtlog, "Parsed CL value (" << a_value << ") as double(" << t_double << "):" << *this );
                    }
                    else if( a_value[ 0 ] == '-' )
                    {
                        // value is a signed integer if it's negative
                        a_parent->add( a_addr, new param_value( (int64_t)t_double ) );
                        MTDEBUG( mtlog, "Parsed CL value (" << a_value << ") as int(" << (int64_t)t_double << "):" << *this );
                    }
                    else
                    {
                        // value is assumed to be unsigned if it's positive
                        a_parent->add( a_addr, new param_value( (uint64_t)t_double ) );
                        MTDEBUG( mtlog, "Parsed CL value (" << a_value << ") as uint(" << (uint64_t)t_double << ");" << *this );
                    }
                }
                else
                {
                    // value is not numeric; treat as a string
                    a_parent->add( a_addr, new param_value( a_value ) );
                    MTDEBUG( mtlog, "Parsed CL value (" << a_value << ") as a string:" << *this );
                }
            }
            return;
        }
        param_node* t_new_node = new param_node();
        a_parent->add( a_addr.substr( 0, t_div_pos ), t_new_node );
        add_next( t_new_node, a_addr.substr( t_div_pos + 1 ), a_value );
        return;
    }


    parser::parser( int an_argc, char** an_argv ) :
            param_node()
    {
        parse(an_argc, an_argv);
    }

    parser::~parser()
    {
    }

    void parser::parse( int an_argc, char** an_argv )
    {
        if( an_argc > 0)
        {
            this->replace( "executable", param_value( an_argv[ 0 ] ) );
        }

        for( int t_index = 1; t_index < an_argc; t_index++ )
        {
            //t_argument.assign( an_argv[ t_index ] );
            std::string t_argument( an_argv[ t_index ] );
            size_t t_val_pos = t_argument.find_first_of( f_separator );
            if( t_val_pos != std::string::npos )
            {
                parsable t_arg( t_argument.substr( 0, t_val_pos ), t_argument.substr( t_val_pos + 1 ) );
                this->merge( t_arg );
                continue;
            }
            else
            {
                parsable t_arg( t_argument );
                this->merge( t_arg );
                continue;
            }
            // value-less CL args now allowed (Noah -- 4/3/15)
            //throw exception() << "argument <" << t_argument << "> does not match <name>=<value> pattern";
        }

        return;
    }


}
