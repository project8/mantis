/*
 * mt_param.cc
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

    unsigned param::s_indent_level = 0;

    param::param()
    {
    }

    param::param( const param& )
    {
    }

    param::~param()
    {
    }

    param* param::clone() const
    {
        return new param( *this );
    }

    bool param::is_null() const
    {
        return true;
    }

    bool param::is_data() const
    {
        return false;
    }

    bool param::is_array() const
    {
        return false;
    }

    bool param::is_node() const
    {
        return false;
    }

    param_data& param::as_data()
    {
        param_data* t_cast_ptr = static_cast< param_data* >( this );
        return *t_cast_ptr;
    }

    param_array& param::as_array()
    {
        param_array* t_cast_ptr = static_cast< param_array* >( this );
        return *t_cast_ptr;
    }

    param_node& param::as_node()
    {
        param_node* t_cast_ptr = static_cast< param_node* >( this );
        return *t_cast_ptr;
    }

    const param_data& param::as_data() const
    {
        const param_data* t_cast_ptr = static_cast< const param_data* >( this );
        return *t_cast_ptr;
    }

    const param_array& param::as_array() const
    {
        const param_array* t_cast_ptr = static_cast< const param_array* >( this );
        return *t_cast_ptr;
    }

    const param_node& param::as_node() const
    {
        const param_node* t_cast_ptr = static_cast< const param_node* >( this );
        return *t_cast_ptr;
    }

    std::string param::to_string() const
    {
        return string();
    }

    //************************************
    //***********  DATA  *****************
    //************************************

    param_data::param_data() :
            param(),
            f_data_str()
    {
    }

    param_data::param_data( const param_data& orig ) :
            param( orig ),
            f_data_str()
    {
        f_data_str << orig.f_data_str.str();
    }

    param_data::~param_data()
    {
    }

    param* param_data::clone() const
    {
        return new param_data( *this );
    }

    bool param_data::is_data() const
    {
        return true;
    }

    const string& param_data::get() const
    {
        f_data_buffer = f_data_str.str();
        return f_data_buffer;
    }

    std::string param_data::to_string() const
    {
        return string(f_data_str.str());
    }

    //************************************
    //***********  ARRAY  ****************
    //************************************

    param_array::param_array() :
            param()
    {
    }

    param_array::param_array( const param_array& orig ) :
            param( orig )
    {
    }

    param_array::~param_array()
    {
    }

    param* param_array::clone() const
    {
        return new param_array( *this );
    }

    bool param_array::is_array() const
    {
        return true;
    }

    std::string param_array::to_string() const
    {
        return string("array printing is not yet implemented");
    }

    //************************************
    //***********  OBJECT  ***************
    //************************************

    param_node::param_node() :
            param(),
            f_contents()
    {
    }

    param_node::param_node( const param_node& orig ) :
            param( orig ),
            f_contents()
    {
        for( const_iterator it = orig.f_contents.begin(); it != orig.f_contents.end(); ++it )
        {
            this->replace( it->first, *it->second );
        }
    }

    param_node::~param_node()
    {
        for( iterator it = f_contents.begin(); it != f_contents.end(); ++it )
        {
            delete it->second;
        }
    }

    param* param_node::clone() const
    {
        return new param_node( *this );
    }

    bool param_node::is_node() const
    {
        return true;
    }

    bool param_node::has( const std::string& a_name ) const
    {
        return f_contents.count( a_name ) > 0;
    }

    unsigned param_node::count( const std::string& a_name ) const
    {
        return f_contents.count( a_name );
    }

    const param* param_node::at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return it->second;
    }

    param* param_node::at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return it->second;
    }

    const param_data* param_node::data_at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return it->second->as_data();
    }

    param_data* param_node::data_at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return it->second->as_data();
    }

    const param_array* param_node::array_at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return it->second->as_array();
    }

    param_array* param_node::array_at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return static_cast< param_array* >(it->second);
    }

    const param_node* param_node::node_at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return it->second->as_node();
    }

    param_node* param_node::node_at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return it->second->as_node();
    }

    const param& param_node::operator[]( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            throw exception() << "No value present corresponding to name <" << a_name << ">\n";
        }
        return *(it->second);
    }

    param& param_node::operator[]( const std::string& a_name )
    {
        return *f_contents[ a_name ];
    }

    bool param_node::add( const std::string& a_name, const param& a_value )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            f_contents.insert( contents_type( a_name, a_value.clone() ) );
            return true;
        }
        return false;
    }

    bool param_node::add( const std::string& a_name, param* a_value )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            f_contents.insert( contents_type( a_name, a_value ) );
            return true;
        }
        return false;
    }

    void param_node::replace( const std::string& a_name, const param& a_value )
    {
        erase( a_name );
        f_contents[ a_name ] = a_value.clone();
        return;
    }

    void param_node::replace( const std::string& a_name, param* a_value )
    {
        erase( a_name );
        f_contents[ a_name ] = a_value;
        return;
    }

    void param_node::merge( const param_node* a_object )
    {
        for( const_iterator it = a_object->f_contents.begin(); it != a_object->f_contents.end(); ++it )
        {
            this->replace( it->first, *it->second );
        }
    }

    void param_node::erase( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it != f_contents.end() )
        {
            delete it->second;
            f_contents.erase( it );
        }
        return;
    }

    param* param_node::remove( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it != f_contents.end() )
        {
            param* removed = it->second;
            f_contents.erase( it );
            return removed;
        }
        return NULL;
    }

    param_node::iterator param_node::begin()
    {
        return f_contents.begin();
    }

    param_node::const_iterator param_node::begin() const
    {
        return f_contents.begin();
    }

    param_node::iterator param_node::end()
    {
        return f_contents.end();
    }

    param_node::const_iterator param_node::end() const
    {
        return f_contents.end();
    }

    std::string param_node::to_string() const
    {
        stringstream out;
        string indentation;
        for ( unsigned i=0; i<param::s_indent_level; ++i )
            indentation += "    ";
        out << '\n' << indentation << "{\n";
        param::s_indent_level++;
        for( const_iterator it = begin(); it != end(); ++it )
        {
            out << indentation << "    " << it->first << " : " << *(it->second) << '\n';
        }
        param::s_indent_level--;
        out << indentation << "}\n";
        return out.str();
    }




    std::ostream& operator<<(std::ostream& out, const param& a_value)
    {
        return out << a_value.to_string();
    }


    std::ostream& operator<<(std::ostream& out, const param_data& a_value)
    {
        return out << a_value.to_string();
    }


    std::ostream& operator<<(std::ostream& out, const param_array& a_value)
    {
        return out << a_value.to_string();
    }


    std::ostream& operator<<(std::ostream& out, const param_node& a_value)
    {
        return out << a_value.to_string();
        /*
        string indentation;
        for (unsigned i=0; i<param::s_indent_level; ++i)
            indentation += "    ";
        out << '\n' << indentation << "{\n";
        param::s_indent_level++;
        for( param_node::const_iterator it = a_value.begin(); it != a_value.end(); ++it )
        {
            out << indentation << '\t' << it->first << " : " << it->second << '\n';
        }
        param::s_indent_level--;
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

    param_node* config_maker_json::read_file( const std::string& a_filename )
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

    param_node* config_maker_json::read_string( const std::string& a_json_string )
    {
        rapidjson::Document t_config_doc;
        if( t_config_doc.Parse<0>( a_json_string.c_str() ).HasParseError() )
        {
            std::cerr << "error parsing string:\n" << t_config_doc.GetParseError() << std::endl;
            return NULL;
        }
        return config_maker_json::read_document( t_config_doc );
    }

    param_node* config_maker_json::read_document( const rapidjson::Document& a_doc )
    {
        param_node* t_config = new param_node();
        for( rapidjson::Value::ConstMemberIterator jsonIt = a_doc.MemberBegin();
                jsonIt != a_doc.MemberEnd();
                ++jsonIt)
        {
            t_config->replace( jsonIt->name.GetString(), config_maker_json::read_value( jsonIt->value ) );
        }
        return t_config;
    }

    param* config_maker_json::read_value( const rapidjson::Value& a_value )
    {
        if( a_value.IsNull() )
        {
            return new param();
        }
        if( a_value.IsObject() )
        {
            param_node* t_config_object = new param_node();
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
            param_array* t_config_array = new param_array();
            // TODO: implement array reading!
            return t_config_array;
        }
        if( a_value.IsString() )
        {
            param_data* t_config_data = new param_data();
            (*t_config_data) << a_value.GetString();
            return t_config_data;
        }
        if( a_value.IsBool() )
        {
            param_data* t_config_data = new param_data();
            (*t_config_data) << a_value.GetBool();
            return t_config_data;
        }
        if( a_value.IsInt() )
        {
            param_data* t_config_data = new param_data();
            (*t_config_data) << a_value.GetInt();
            return t_config_data;
        }
        if( a_value.IsUint() )
        {
            param_data* t_config_data = new param_data();
            (*t_config_data) << a_value.GetUint();
            return t_config_data;
        }
        if( a_value.IsInt64() )
        {
            param_data* t_config_data = new param_data();
            (*t_config_data) << a_value.GetInt64();
            return t_config_data;
        }
        if( a_value.IsUint64() )
        {
            param_data* t_config_data = new param_data();
            (*t_config_data) << a_value.GetUint64();
            return t_config_data;
        }
        if( a_value.IsDouble() )
        {
            param_data* t_config_data = new param_data();
            (*t_config_data) << a_value.GetDouble();
            return t_config_data;
        }
        std::cout << "(config_reader_json) unknown type; returning null value" << std::endl;
        return new param();
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
            param(),
            config_node()
    {
    }

    config_node_null::config_node_null(const std::string& a_name) :
            param(),
            config_node( a_name )
    {
    }

    config_node_null::config_node_null(const config_node_data& orig) :
            param( orig ),
            config_node( orig )
    {
    }

    config_node_null::~config_node_null()
    {
    }





    config_node_data::config_node_data() :
            param_data(),
            config_node()
    {
    }

    config_node_data::config_node_data(const std::string& a_name) :
            param_data(),
            config_node( a_name )
    {
    }

    config_node_data::config_node_data(const config_node_data& orig) :
            param_data( orig ),
            config_node( orig )
    {
    }

    config_node_data::~config_node_data()
    {
    }




    config_node_array::config_node_array() :
            param_array(),
            config_node()
    {
    }

    config_node_array::config_node_array(const std::string& a_name) :
            param_array(),
            config_node( a_name )
    {
    }

    config_node_array::config_node_array(const config_node_array& orig) :
            param_array( orig ),
            config_node( orig )
    {
    }

    config_node_array::~config_node_array()
    {
    }




    config_node_object::config_node_object() :
            param_node(),
            config_node()
    {
    }

    config_node_object::config_node_object(const std::string& a_name) :
            param_node(),
            config_node( a_name )
    {
    }

    config_node_object::config_node_object(const config_node_object& orig) :
            param_node( orig ),
            config_node( orig )
    {
    }

    config_node_object::~config_node_object()
    {
    }

*/

} /* namespace Katydid */
