/*
 * mt_config_value.cc
 *
 *  Created on: Jan 14, 2014
 *      Author: nsoblath
 */

#include "mt_config_node.hh"

#include "mt_exception.hh"

#include <iostream>
#include <sstream>
using std::string;
using std::stringstream;


// for the config maker
#include "filestream.h"

#include <cstdio>
using std::string;



namespace mantis
{

    unsigned config_value::s_indent_level = 0;

    config_value::config_value()
    {
    }

    config_value::config_value( const config_value& )
    {
    }

    config_value::~config_value()
    {
    }

    config_value* config_value::clone() const
    {
        return new config_value( *this );
    }

    bool config_value::is_data() const
    {
        return false;
    }

    bool config_value::is_array() const
    {
        return false;
    }

    bool config_value::is_object() const
    {
        return false;
    }

    const config_value& config_value::operator/(const std::string&) const
    {
        return *this;
    }

/*
    config_value_data& config_value::as_data()
    {
        return static_cast< config_value_data >( *this );
    }

    config_value_array& config_value::as_array()
    {
        return static_cast< config_value_array >( *this );
    }

    config_value_object& config_value::as_object()
    {
        return static_cast< config_value_object >( *this );
    }

    const config_value_data& config_value::as_data() const
    {
        return static_cast< config_value_data >( *this );
    }

    const config_value_array& config_value::as_array() const
    {
        return static_cast< config_value_array >( *this );
    }

    const config_value_object& config_value::as_object() const
    {
        return static_cast< config_value_object >( *this );
    }
*/

    std::string config_value::to_string() const
    {
        return string();
    }

    //************************************
    //***********  DATA  *****************
    //************************************

    config_value_data::config_value_data() :
            config_value(),
            f_data_str()
    {
    }

    config_value_data::config_value_data( const config_value_data& orig ) :
            config_value( orig ),
            f_data_str()
    {
        f_data_str << orig.f_data_str.str();
    }

    config_value_data::~config_value_data()
    {
    }

    config_value* config_value_data::clone() const
    {
        return new config_value_data( *this );
    }

    bool config_value_data::is_data() const
    {
        return true;
    }

    const string& config_value_data::value() const
    {
        f_data_buffer = f_data_str.str();
        return f_data_buffer;
    }

    std::string config_value_data::to_string() const
    {
        return string(f_data_str.str());
    }

    //************************************
    //***********  ARRAY  ****************
    //************************************

    config_value_array::config_value_array() :
            config_value()
    {
    }

    config_value_array::config_value_array( const config_value_array& orig ) :
            config_value( orig )
    {
    }

    config_value_array::~config_value_array()
    {
    }

    config_value* config_value_array::clone() const
    {
        return new config_value_array( *this );
    }

    bool config_value_array::is_array() const
    {
        return true;
    }

    std::string config_value_array::to_string() const
    {
        return string("array printing is not yet implemented");
    }

    //************************************
    //***********  OBJECT  ***************
    //************************************

    config_value_object::config_value_object() :
            config_value(),
            f_contents()
    {
    }

    config_value_object::config_value_object( const config_value_object& orig ) :
            config_value( orig ),
            f_contents()
    {
        for( const_iterator it = orig.f_contents.begin(); it != orig.f_contents.end(); ++it )
        {
            this->replace( it->first, *it->second );
        }
    }

    config_value_object::~config_value_object()
    {
        for( iterator it = f_contents.begin(); it != f_contents.end(); ++it )
        {
            delete it->second;
        }
    }

    config_value* config_value_object::clone() const
    {
        return new config_value_object( *this );
    }

    bool config_value_object::is_object() const
    {
        return true;
    }

    const config_value& config_value_object::operator/(const std::string& a_name) const
    {
        return (*this)[a_name];
    }

    bool config_value_object::has( const std::string& a_name ) const
    {
        return f_contents.count( a_name ) > 0;
    }

    unsigned config_value_object::count( const std::string& a_name ) const
    {
        return f_contents.count( a_name );
    }

    const config_value* config_value_object::at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return it->second;
    }

    config_value* config_value_object::at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return it->second;
    }

    const config_value_data* config_value_object::data_at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return static_cast< const config_value_data* >(it->second);
    }

    config_value_data* config_value_object::data_at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return static_cast< config_value_data* >(it->second);
    }

    const config_value_array* config_value_object::array_at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return static_cast< const config_value_array* >(it->second);
    }

    config_value_array* config_value_object::array_at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return static_cast< config_value_array* >(it->second);
    }

    const config_value_object* config_value_object::object_at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return static_cast< const config_value_object* >(it->second);
    }

    config_value_object* config_value_object::object_at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return static_cast< config_value_object* >(it->second);
    }

    const config_value& config_value_object::operator[]( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            throw exception() << "No value present corresponding to name <" << a_name << ">\n";
        }
        return *(it->second);
    }

    config_value& config_value_object::operator[]( const std::string& a_name )
    {
        return *f_contents[ a_name ];
    }

    bool config_value_object::add( const std::string& a_name, const config_value& a_value )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            f_contents.insert( contents_type( a_name, a_value.clone() ) );
            return true;
        }
        return false;
    }

    bool config_value_object::add( const std::string& a_name, config_value* a_value )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            f_contents.insert( contents_type( a_name, a_value ) );
            return true;
        }
        return false;
    }

    void config_value_object::replace( const std::string& a_name, const config_value& a_value )
    {
        erase( a_name );
        f_contents[ a_name ] = a_value.clone();
        return;
    }

    void config_value_object::replace( const std::string& a_name, config_value* a_value )
    {
        erase( a_name );
        f_contents[ a_name ] = a_value;
        return;
    }

    void config_value_object::merge( const config_value_object* a_object )
    {
        for( const_iterator it = a_object->f_contents.begin(); it != a_object->f_contents.end(); ++it )
        {
            this->replace( it->first, *it->second );
        }
    }

    void config_value_object::erase( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it != f_contents.end() )
        {
            delete it->second;
            f_contents.erase( it );
        }
        return;
    }

    config_value* config_value_object::remove( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it != f_contents.end() )
        {
            config_value* removed = it->second;
            f_contents.erase( it );
            return removed;
        }
        return NULL;
    }

    config_value_object::iterator config_value_object::begin()
    {
        return f_contents.begin();
    }

    config_value_object::const_iterator config_value_object::begin() const
    {
        return f_contents.begin();
    }

    config_value_object::iterator config_value_object::end()
    {
        return f_contents.end();
    }

    config_value_object::const_iterator config_value_object::end() const
    {
        return f_contents.end();
    }

    std::string config_value_object::to_string() const
    {
        stringstream out;
        string indentation;
        for ( unsigned i=0; i<config_value::s_indent_level; ++i )
            indentation += "    ";
        out << '\n' << indentation << "{\n";
        config_value::s_indent_level++;
        for( const_iterator it = begin(); it != end(); ++it )
        {
            out << indentation << "    " << it->first << " : " << *(it->second) << '\n';
        }
        config_value::s_indent_level--;
        out << indentation << "}\n";
        return out.str();
    }




    std::ostream& operator<<(std::ostream& out, const config_value& a_value)
    {
        return out << a_value.to_string();
    }


    std::ostream& operator<<(std::ostream& out, const config_value_data& a_value)
    {
        return out << a_value.to_string();
    }


    std::ostream& operator<<(std::ostream& out, const config_value_array& a_value)
    {
        return out << a_value.to_string();
    }


    std::ostream& operator<<(std::ostream& out, const config_value_object& a_value)
    {
        return out << a_value.to_string();
        /*
        string indentation;
        for (unsigned i=0; i<config_value::s_indent_level; ++i)
            indentation += "    ";
        out << '\n' << indentation << "{\n";
        config_value::s_indent_level++;
        for( config_value_object::const_iterator it = a_value.begin(); it != a_value.end(); ++it )
        {
            out << indentation << '\t' << it->first << " : " << it->second << '\n';
        }
        config_value::s_indent_level--;
        out << '\n' << indentation << "}\n";
        return out;
        */
    }





    config_maker_json::config_maker_json()
    {
    }
    config_maker_json::~config_maker_json()
    {
    }

    config_value_object* config_maker_json::read_file( const std::string& a_filename )
    {
        FILE* t_config_file = fopen( a_filename.c_str(), "r" );
        if( t_config_file == NULL )
        {
            std::cerr << "file <" << a_filename << "> did not open" << std::endl;
            return NULL;
        }
        rapidjson::FileStream t_file_stream( t_config_file );

        rapidjson::Document t_config_doc;
        if( t_config_doc.ParseStream<0>( t_file_stream ).HasParseError() )
        {
            std::cerr << "error parsing config file:\n" << t_config_doc.GetParseError() << std::endl;
            fclose( t_config_file );
            return NULL;
        }
        fclose( t_config_file );

        return config_maker_json::read_document( t_config_doc );
    }

    config_value_object* config_maker_json::read_string( const std::string& a_json_string )
    {
        rapidjson::Document t_config_doc;
        if( t_config_doc.Parse<0>( a_json_string.c_str() ).HasParseError() )
        {
            std::cerr << "error parsing string:\n" << t_config_doc.GetParseError() << std::endl;
            return NULL;
        }
        return config_maker_json::read_document( t_config_doc );
    }

    config_value_object* config_maker_json::read_document( const rapidjson::Document& a_doc )
    {
        config_value_object* t_config = new config_value_object();
        for( rapidjson::Value::ConstMemberIterator jsonIt = a_doc.MemberBegin();
                jsonIt != a_doc.MemberEnd();
                ++jsonIt)
        {
            t_config->replace( jsonIt->name.GetString(), config_maker_json::read_value( jsonIt->value ) );
        }
        return t_config;
    }

    config_value* config_maker_json::read_value( const rapidjson::Value& a_value )
    {
        if( a_value.IsNull() )
        {
            return new config_value();
        }
        if( a_value.IsObject() )
        {
            config_value_object* t_config_object = new config_value_object();
            for( rapidjson::Value::ConstMemberIterator jsonIt = a_value.MemberBegin();
                    jsonIt != a_value.MemberEnd();
                    ++jsonIt)
            {
                t_config_object->replace( jsonIt->name.GetString(), config_maker_json::read_value( jsonIt->value ) );
            }
            return t_config_object;
        }
        if( a_value.IsArray() )
        {
            config_value_array* t_config_array = new config_value_array();
            // TODO: implement array reading!
            return t_config_array;
        }
        if( a_value.IsString() )
        {
            config_value_data* t_config_data = new config_value_data();
            (*t_config_data) << a_value.GetString();
            return t_config_data;
        }
        if( a_value.IsBool() )
        {
            config_value_data* t_config_data = new config_value_data();
            (*t_config_data) << a_value.GetBool();
            return t_config_data;
        }
        if( a_value.IsInt() )
        {
            config_value_data* t_config_data = new config_value_data();
            (*t_config_data) << a_value.GetInt();
            return t_config_data;
        }
        if( a_value.IsUint() )
        {
            config_value_data* t_config_data = new config_value_data();
            (*t_config_data) << a_value.GetUint();
            return t_config_data;
        }
        if( a_value.IsInt64() )
        {
            config_value_data* t_config_data = new config_value_data();
            (*t_config_data) << a_value.GetInt64();
            return t_config_data;
        }
        if( a_value.IsUint64() )
        {
            config_value_data* t_config_data = new config_value_data();
            (*t_config_data) << a_value.GetUint64();
            return t_config_data;
        }
        if( a_value.IsDouble() )
        {
            config_value_data* t_config_data = new config_value_data();
            (*t_config_data) << a_value.GetDouble();
            return t_config_data;
        }
        std::cout << "(config_reader_json) unknown type; returning null value" << std::endl;
        return new config_value();
    }

/*



    // private constructor
    config_node::config_node() :
            f_name(),
            f_parent( NULL )
    {
    }

    config_node::config_node(const std::string& a_name) :
            f_name( a_name ),
            f_parent( NULL )
    {
    }

    config_node::config_node(const config_node& orig) :
            f_name( orig.f_name ),
            f_parent( NULL )
    {
    }

    config_node::~config_node()
    {
    }

    const std::string& config_node::name() const
    {
        return f_name;
    }

    void config_node::set_name(const std::string& a_name)
    {
        f_name = a_name;
        return;
    }

    config_node* config_node::parent() const
    {
        return f_parent;
    }



    config_node_null::config_node_null() :
            config_value(),
            config_node()
    {
    }

    config_node_null::config_node_null(const std::string& a_name) :
            config_value(),
            config_node( a_name )
    {
    }

    config_node_null::config_node_null(const config_node_data& orig) :
            config_value( orig ),
            config_node( orig )
    {
    }

    config_node_null::~config_node_null()
    {
    }





    config_node_data::config_node_data() :
            config_value_data(),
            config_node()
    {
    }

    config_node_data::config_node_data(const std::string& a_name) :
            config_value_data(),
            config_node( a_name )
    {
    }

    config_node_data::config_node_data(const config_node_data& orig) :
            config_value_data( orig ),
            config_node( orig )
    {
    }

    config_node_data::~config_node_data()
    {
    }




    config_node_array::config_node_array() :
            config_value_array(),
            config_node()
    {
    }

    config_node_array::config_node_array(const std::string& a_name) :
            config_value_array(),
            config_node( a_name )
    {
    }

    config_node_array::config_node_array(const config_node_array& orig) :
            config_value_array( orig ),
            config_node( orig )
    {
    }

    config_node_array::~config_node_array()
    {
    }




    config_node_object::config_node_object() :
            config_value_object(),
            config_node()
    {
    }

    config_node_object::config_node_object(const std::string& a_name) :
            config_value_object(),
            config_node( a_name )
    {
    }

    config_node_object::config_node_object(const config_node_object& orig) :
            config_value_object( orig ),
            config_node( orig )
    {
    }

    config_node_object::~config_node_object()
    {
    }

*/

} /* namespace Katydid */
