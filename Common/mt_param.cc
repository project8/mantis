/*
 * mt_param.cc
 *
 *  Created on: Jan 14, 2014
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_param.hh"

#include "mt_logger.hh"

#include <sstream>
using std::string;
using std::stringstream;


#include <cstdio>
using std::string;



namespace mantis
{
    MTLOGGER( mtlog, "param" );

    param_exception::param_exception() :
            exception()
    {
    }
    param_exception::param_exception( const param_exception& an_exception ) :
            exception( an_exception )
    {
    }

    param_exception::~param_exception() throw ()
    {
    }


    MANTIS_API unsigned param::s_indent_level = 0;

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

    bool param::is_value() const
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

    param_value& param::as_value()
    {
        param_value* t_cast_ptr = dynamic_cast< param_value* >( this );
        return *t_cast_ptr;
    }

    param_array& param::as_array()
    {
        param_array* t_cast_ptr = dynamic_cast< param_array* >( this );
        return *t_cast_ptr;
    }

    param_node& param::as_node()
    {
        param_node* t_cast_ptr = dynamic_cast< param_node* >( this );
        return *t_cast_ptr;
    }

    const param_value& param::as_value() const
    {
        const param_value* t_cast_ptr = dynamic_cast< const param_value* >( this );
        return *t_cast_ptr;
    }

    const param_array& param::as_array() const
    {
        const param_array* t_cast_ptr = dynamic_cast< const param_array* >( this );
        return *t_cast_ptr;
    }

    const param_node& param::as_node() const
    {
        const param_node* t_cast_ptr = dynamic_cast< const param_node* >( this );
        return *t_cast_ptr;
    }

    std::string param::to_string() const
    {
        return string();
    }

    //************************************
    //***********  DATA  *****************
    //************************************

    param_value::param_value() :
            param(),
            f_value()
    {
    }

    param_value::param_value( const param_value& orig ) :
            param( orig ),
            f_value( orig.f_value )
    {
    }

    param_value::~param_value()
    {
    }

    param_value& param_value::operator=( const param_value& rhs )
    {
        f_value = rhs.f_value;
        return *this;
    }

    param* param_value::clone() const
    {
        return new param_value( *this );
    }

    bool param_value::is_null() const
    {
        return false;
    }

    bool param_value::empty() const
    {
        return f_value.empty();
    }

    bool param_value::is_value() const
    {
        return true;
    }

    const string& param_value::get() const
    {
         return f_value;
    }

    void param_value::clear()
    {
        f_value.clear();
        return;
    }

    std::string param_value::to_string() const
    {
        return string( f_value );
    }

    //************************************
    //***********  ARRAY  ****************
    //************************************

    param_array::param_array() :
            param(),
            f_contents()
    {
    }

    param_array::param_array( const param_array& orig ) :
            param( orig ),
            f_contents( orig.f_contents.size() )
    {
        for( unsigned ind = 0; ind < f_contents.size(); ++ind )
        {
            this->assign( ind, orig[ ind ].clone() );
        }
    }

    param_array::~param_array()
    {
        clear();
    }

    param_array& param_array::operator=( const param_array& rhs )
    {
        clear();
        for( unsigned ind = 0; ind < f_contents.size(); ++ind )
        {
            this->assign( ind, rhs[ ind ].clone() );
        }
        return *this;
    }

    param* param_array::clone() const
    {
        return new param_array( *this );
    }

    bool param_array::is_null() const
    {
        return false;
    }

    bool param_array::is_array() const
    {
        return true;
    }

    unsigned param_array::size() const
    {
        return f_contents.size();
    }
    bool param_array::empty() const
    {
        return f_contents.empty();
    }

    void param_array::resize( unsigned a_size )
    {
        unsigned curr_size = f_contents.size();
        for( unsigned ind = a_size; ind < curr_size; ++ind )
        {
            delete f_contents[ ind ];
        }
        f_contents.resize( a_size );
        return;
    }

    std::string param_array::get_value( unsigned a_index ) const
    {
        const param_value* value = value_at( a_index );
        if( value == NULL ) throw param_exception() << "No value at <" << a_index << "> is present at this node";
        return value->get();
    }

    std::string param_array::get_value( unsigned a_index, const std::string& a_default ) const
    {
        const param_value* value = value_at( a_index );
        if( value == NULL ) return a_default;
        return value->get();
    }

    std::string param_array::get_value( unsigned a_index, const char* a_default ) const
    {
        return get_value( a_index, string( a_default ) );
    }

    const param* param_array::at( unsigned a_index ) const
    {
        if( a_index >= f_contents.size() ) return NULL;
        return f_contents[ a_index ];
    }
    param* param_array::at( unsigned a_index )
    {
        if( a_index >= f_contents.size() ) return NULL;
        return f_contents[ a_index ];
    }

    const param_value* param_array::value_at( unsigned a_index ) const
    {
        if( a_index >= f_contents.size() ) return NULL;
        return &f_contents[ a_index ]->as_value();
    }
    param_value* param_array::value_at( unsigned a_index )
    {
        if( a_index >= f_contents.size() ) return NULL;
        return &f_contents[ a_index ]->as_value();
    }

    const param_array* param_array::array_at( unsigned a_index ) const
    {
        if( a_index >= f_contents.size() ) return NULL;
        return &f_contents[ a_index ]->as_array();
    }
    param_array* param_array::array_at( unsigned a_index )
    {
        if( a_index >= f_contents.size() ) return NULL;
        return &f_contents[ a_index ]->as_array();
    }

    const param_node* param_array::node_at( unsigned a_index ) const
    {
        if( a_index >= f_contents.size() ) return NULL;
        return &f_contents[ a_index ]->as_node();
    }
    param_node* param_array::node_at( unsigned a_index )
    {
        if( a_index >= f_contents.size() ) return NULL;
        return &f_contents[ a_index ]->as_node();
    }

    const param& param_array::operator[]( unsigned a_index ) const
    {
        return *f_contents[ a_index ];
    }
    param& param_array::operator[]( unsigned a_index )
    {
        return *f_contents[ a_index ];
    }

    const param* param_array::front() const
    {
        return f_contents.front();
    }
    param* param_array::front()
    {
        return f_contents.front();
    }

    const param* param_array::back() const
    {
        return f_contents.back();
    }
    param* param_array::back()
    {
        return f_contents.back();
    }

    // assign a copy of a_value to the array at a_index
    void param_array::assign( unsigned a_index, const param& a_value )
    {
        erase( a_index );
        f_contents[ a_index ] = a_value.clone();
        return;
    }
    // directly assign a_value_ptr to the array at a_index
    void param_array::assign( unsigned a_index, param* a_value_ptr )
    {
        erase( a_index );
        f_contents[ a_index ] = a_value_ptr;
        return;
    }

    void param_array::push_back( const param& a_value )
    {
        f_contents.push_back( a_value.clone() );
        return;
    }
    void param_array::push_back( param* a_value_ptr )
    {
        f_contents.push_back( a_value_ptr );
        return;
    }

    void param_array::push_front( const param& a_value )
    {
        f_contents.push_front( a_value.clone() );
        return;
    }
    void param_array::push_front( param* a_value_ptr )
    {
        f_contents.push_front( a_value_ptr );
        return;
    }

    void param_array::append( const param_array& an_array )
    {
        for( param_array::const_iterator it = an_array.begin(); it != an_array.end(); ++it )
        {
            push_back( *(*it) );
        }
        return;
    }

    void param_array::erase( unsigned a_index )
    {
        delete f_contents[ a_index ];
        return;
    }
    param* param_array::remove( unsigned a_index )
    {
        param* t_current = f_contents[ a_index ];
        f_contents[ a_index ] = NULL;
        return t_current;
    }
    void param_array::clear()
    {
        for( unsigned ind = 0; ind < f_contents.size(); ++ind )
        {
            delete f_contents[ ind ];
        }
        return;
    }

    param_array::iterator param_array::begin()
    {
        return f_contents.begin();
    }
    param_array::const_iterator param_array::begin() const
    {
        return f_contents.begin();
    }

    param_array::iterator param_array::end()
    {
        return f_contents.end();
    }
    param_array::const_iterator param_array::end() const
    {
        return f_contents.end();
    }

    param_array::reverse_iterator param_array::rbegin()
    {
        return f_contents.rbegin();
    }
    param_array::const_reverse_iterator param_array::rbegin() const
    {
        return f_contents.rbegin();
    }

    param_array::reverse_iterator param_array::rend()
    {
        return f_contents.rend();
    }
    param_array::const_reverse_iterator param_array::rend() const
    {
        return f_contents.rend();
    }

    std::string param_array::to_string() const
    {
        stringstream out;
        string indentation;
        for ( unsigned i=0; i<param::s_indent_level; ++i )
            indentation += "    ";
        out << '\n' << indentation << "[\n";
        param::s_indent_level++;
        for( const_iterator it = begin(); it != end(); ++it )
        {
            out << indentation << "    " << **it << '\n';
        }
        param::s_indent_level--;
        out << indentation << "]\n";
        return out.str();
    }


    //************************************
    //***********  NODE  *****************
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
        clear();
    }

    param_node& param_node::operator=( const param_node& rhs )
    {
        clear();
        for( const_iterator it = rhs.f_contents.begin(); it != rhs.f_contents.end(); ++it )
        {
            this->replace( it->first, *it->second );
        }
        return *this;
    }

    param* param_node::clone() const
    {
        return new param_node( *this );
    }

    bool param_node::is_null() const
    {
        return false;
    }

    bool param_node::is_node() const
    {
        return true;
    }

    unsigned param_node::size() const
    {
        return f_contents.size();
    }

    bool param_node::has( const std::string& a_name ) const
    {
        return f_contents.count( a_name ) > 0;
    }

    unsigned param_node::count( const std::string& a_name ) const
    {
        return f_contents.count( a_name );
    }

    std::string param_node::get_value( const std::string& a_name ) const
    {
        const param_value* value = value_at( a_name );
        if( value == NULL ) throw param_exception() << "No value with name <" << a_name << "> is present at this node:\n" << *this;
        return value->get();
    }

    std::string param_node::get_value( const std::string& a_name, const std::string& a_default ) const
    {
        const param_value* value = value_at( a_name );
        if( value == NULL ) return a_default;
        return value->get();
    }

    std::string param_node::get_value( const std::string& a_name, const char* a_default ) const
    {
        return get_value( a_name, std::string( a_default ) );
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

    const param_value* param_node::value_at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return &it->second->as_value();
    }

    param_value* param_node::value_at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return &it->second->as_value();
    }

    const param_array* param_node::array_at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return &it->second->as_array();
    }

    param_array* param_node::array_at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return &it->second->as_array();
    }

    const param_node* param_node::node_at( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return &it->second->as_node();
    }

    param_node* param_node::node_at( const std::string& a_name )
    {
        iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            return NULL;
        }
        return &it->second->as_node();
    }

    const param& param_node::operator[]( const std::string& a_name ) const
    {
        const_iterator it = f_contents.find( a_name );
        if( it == f_contents.end() )
        {
            throw param_exception() << "No value present corresponding to name <" << a_name << ">\n";
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

    void param_node::merge( const param_node& a_object )
    {
        //MTDEBUG( mtlog, "merging object with " << a_object.size() << " items:\n" << a_object );
        for( const_iterator it = a_object.f_contents.begin(); it != a_object.f_contents.end(); ++it )
        {
            if( ! has( it->first ) )
            {
                //MTDEBUG( mtlog, "do not have object <" << it->first << "> = <" << *it->second << ">" );
                add( it->first, *it->second );
                continue;
            }
            param& t_param = (*this)[ it->first ];
            if( t_param.is_value() )
            {
                //MTDEBUG( mtlog, "replacing the value <" << it->first << "> with <" << *it->second << ">" );
                replace( it->first, *it->second );
                continue;
            }
            if( t_param.is_node() && it->second->is_node() )
            {
                //MTDEBUG( mtlog, "merging nodes")
                t_param.as_node().merge( it->second->as_node() );
                continue;
            }
            if( t_param.is_array() && it->second->is_array() )
            {
                //MTDEBUG( mtlog, "appending array" );
                t_param.as_array().append( it->second->as_array() );
                continue;
            }
            //MTDEBUG( mtlog, "generic replace" );
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

    void param_node::clear()
    {
        for( iterator it = f_contents.begin(); it != f_contents.end(); ++it )
        {
            delete it->second;
        }
        return;
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




    MANTIS_API std::ostream& operator<<(std::ostream& out, const param& a_value)
    {
        return out << a_value.to_string();
    }


    MANTIS_API std::ostream& operator<<(std::ostream& out, const param_value& a_value)
    {
        return out << a_value.to_string();
    }


    MANTIS_API std::ostream& operator<<(std::ostream& out, const param_array& a_value)
    {
        return out << a_value.to_string();
    }


    MANTIS_API std::ostream& operator<<(std::ostream& out, const param_node& a_value)
    {
        return out << a_value.to_string();
    }





    param_input_json::param_input_json()
    {
    }
    param_input_json::~param_input_json()
    {
    }

    param_node* param_input_json::read_file( const std::string& a_filename )
    {
        FILE* t_config_file = fopen( a_filename.c_str(), "r" );
        if( t_config_file == NULL )
        {
            MTERROR( mtlog, "file <" << a_filename << "> did not open" );
            return NULL;
        }
        rapidjson::FileStream t_file_stream( t_config_file );

        rapidjson::Document t_config_doc;
        if( t_config_doc.ParseStream<0>( t_file_stream ).HasParseError() )
        {
            unsigned errorPos = t_config_doc.GetErrorOffset();
            rewind( t_config_file );
            unsigned iChar, newlineCount = 1, lastNewlinePos = 0;
            int thisChar;
            for( iChar = 0; iChar != errorPos; ++iChar )
            {
                thisChar = fgetc( t_config_file );
                if( thisChar == EOF )
                {
                    break;
                }
                if( thisChar == '\n' || thisChar == '\r' )
                {
                    newlineCount++;
                    lastNewlinePos = iChar + 1;
                }
            }
            if( iChar == errorPos )
            {
                MTERROR( mtlog, "error parsing config file :\n" <<
                        '\t' << t_config_doc.GetParseError() << '\n' <<
                        "\tThe error was reported at line " << newlineCount << ", character " << errorPos - lastNewlinePos );
            }
            else
            {
                MTERROR( mtlog, "error parsing config file :\n" <<
                        '\t' << t_config_doc.GetParseError() <<
                        "\tend of file reached before error location was found" );
            }
            fclose( t_config_file );
            return NULL;
        }
        fclose( t_config_file );

        return param_input_json::read_document( t_config_doc );
    }

    param_node* param_input_json::read_string( const std::string& a_json_string )
    {
        rapidjson::Document t_config_doc;
        if( t_config_doc.Parse<0>( a_json_string.c_str() ).HasParseError() )
        {
            MTERROR( mtlog, "error parsing string:\n" << t_config_doc.GetParseError() );
            return NULL;
        }
        return param_input_json::read_document( t_config_doc );
    }

    param_node* param_input_json::read_document( const rapidjson::Document& a_doc )
    {
        param_node* t_config = new param_node();
        for( rapidjson::Value::ConstMemberIterator jsonIt = a_doc.MemberBegin();
                jsonIt != a_doc.MemberEnd();
                ++jsonIt)
        {
            t_config->replace( jsonIt->name.GetString(), param_input_json::read_value( jsonIt->value ) );
        }
        return t_config;
    }

    param* param_input_json::read_value( const rapidjson::Value& a_value )
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
                t_config_object->replace( jsonIt->name.GetString(), param_input_json::read_value( jsonIt->value ) );
            }
            return t_config_object;
        }
        if( a_value.IsArray() )
        {
            param_array* t_config_array = new param_array();
            for( rapidjson::Value::ConstValueIterator jsonIt = a_value.Begin();
                    jsonIt != a_value.End();
                    ++jsonIt)
            {
                t_config_array->push_back( param_input_json::read_value( *jsonIt ) );
            }
            return t_config_array;
        }
        if( a_value.IsString() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetString();
            return t_config_value;
        }
        if( a_value.IsBool() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetBool();
            return t_config_value;
        }
        if( a_value.IsInt() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetInt();
            return t_config_value;
        }
        if( a_value.IsUint() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetUint();
            return t_config_value;
        }
        if( a_value.IsInt64() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetInt64();
            return t_config_value;
        }
        if( a_value.IsUint64() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetUint64();
            return t_config_value;
        }
        if( a_value.IsDouble() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetDouble();
            return t_config_value;
        }
        MTWARN( mtlog, "(config_reader_json) unknown type; returning null value" );
        return new param();
    }



    param_output_json::param_output_json()
    {}

    param_output_json::~param_output_json()
    {}

    bool param_output_json::write_file( const param& a_to_write, const std::string& a_filename, json_writing_style a_style )
    {
        if( a_filename.empty() )
        {
            MTERROR( mtlog, "Filename cannot be an empty string" );
            return false;
        }

        FILE* file = fopen( a_filename.c_str(), "w" );
        if( file == NULL )
        {
            MTERROR( mtlog, "Unable to open file: " << a_filename );
            return false;
        }

        rapidjson::FileStream t_filestream( file );

        bool t_result = false;
        if( a_style == k_compact )
        {
            rj_file_writer t_writer( t_filestream );
            t_result = param_output_json::write_param( a_to_write, &t_writer );
        }
        else
        {
            rj_pretty_file_writer t_writer( t_filestream );
            t_result = param_output_json::write_param( a_to_write, &t_writer );
        }

        if (! t_result )
        {
            MTERROR( mtlog, "Error while writing file" );
            return false;
        }

        return true;
    }
    bool param_output_json::write_string( const param& a_to_write, std::string& a_string, json_writing_style a_style )
    {
        rapidjson::StringBuffer t_str_buff;

        bool t_result = false;
        if( a_style == k_compact )
        {
            rj_string_writer t_writer( t_str_buff );
            t_result = param_output_json::write_param( a_to_write, &t_writer );
        }
        else
        {
            rj_pretty_string_writer t_writer( t_str_buff );
            t_result = param_output_json::write_param( a_to_write, &t_writer );
        }

        if (! t_result )
        {
            MTERROR( mtlog, "Error while writing string" );
            return false;
        }

        a_string.assign( t_str_buff.GetString() );

        return true;
    }

} /* namespace mantis */
