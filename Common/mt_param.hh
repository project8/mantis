/*
 * mt_param.hh
 *
 *  Created on: Jan 14, 2014
 *      Author: nsoblath
 */

#ifndef MT_PARAM_HH_
#define MT_PARAM_HH_

#include "mt_exception.hh"

#include "mt_logger.hh"

#include "document.h"
#include "filestream.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include <deque>
#include <map>
#include <sstream>
#include <string>

//#ifdef _WIN32
//MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::basic_string< char, std::char_traits< char >, std::allocator< char > >;
//#endif

namespace mantis
{
    MTLOGGER(mtlog_p, "param");
    class param_value;
    class param_array;
    class param_node;

    class MANTIS_API param_exception : public exception
    {
        public:
            param_exception();
            param_exception( const param_exception& );
            ~param_exception() throw ();
    };


    class MANTIS_API param
    {
        public:
            param();
            param(const param& orig);
            virtual ~param();

            virtual param* clone() const;

            virtual bool is_null() const;
            virtual bool is_value() const;
            virtual bool is_array() const;
            virtual bool is_node() const;

            param_value& as_value();
            param_array& as_array();
            param_node& as_node();

            const param_value& as_value() const;
            const param_array& as_array() const;
            const param_node& as_node() const;

            virtual std::string to_string() const;

            static unsigned s_indent_level;
    };

    class MANTIS_API param_value : public param
    {
        public:
            param_value();
            template< typename XStreamableType >
            param_value( XStreamableType a_streamable );
            param_value(const param_value& orig);
            virtual ~param_value();

            param_value& operator=( const param_value& rhs );

            virtual param* clone() const;

            bool empty() const;

            virtual bool is_null() const;
            virtual bool is_value() const;

            const std::string& get() const;
            template< typename XValType >
            XValType get() const;

            template< typename XStreamableType >
            param_value& operator<<( const XStreamableType& a_streamable );

            virtual std::string to_string() const;

            void clear();

        protected:
            std::string f_value;

    };

    template< typename XStreamableType >
    param_value::param_value( XStreamableType a_streamable ) :
            param(),
            f_value()
    {
        (*this) << a_streamable;
    }

    template< typename XValType >
    XValType param_value::get() const
    {
        XValType t_return;
        std::stringstream t_buffer;
        t_buffer << f_value;
        t_buffer >> t_return;
        return t_return;
    }

    template< typename XStreamableType >
    param_value& param_value::operator<<( const XStreamableType& a_streamable )
    {
        std::stringstream t_buffer;
        t_buffer << a_streamable;
        f_value = t_buffer.str();
        return *this;
    }


//#ifdef _WIN32
//    MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::deque< param* >;
//#endif

    class MANTIS_API param_array : public param
    {
        public:
            typedef std::deque< param* > contents;
            typedef contents::iterator iterator;
            typedef contents::const_iterator const_iterator;
            typedef contents::reverse_iterator reverse_iterator;
            typedef contents::const_reverse_iterator const_reverse_iterator;
            typedef contents::value_type contents_type;

        public:
            param_array();
            param_array( const param_array& orig );
            virtual ~param_array();

            param_array& operator=( const param_array& rhs );

            virtual param* clone() const;

            virtual bool is_null() const;
            virtual bool is_array() const;

            unsigned size() const;
            bool empty() const;

            /// sets the size of the array
            /// if smaller than the current size, extra elements are deleted
            void resize( unsigned a_size );

            /// Returns the result of ParamValue::get if a_name is present and is of type ParamValue
            /// Throws an exception if a_name is not present or is not of type ParamValue
            std::string get_value( unsigned a_index ) const;
            /// Returns the result of ParamValue::get if a_name is present and is of type ParamValue
            /// Throws an exception if a_name is not present or is not of type ParamValue
            template< typename XValType >
            XValType get_value( unsigned a_index ) const;

            /// Returns the result of ParamValue::get if a_name is present and is of type ParamValue
            /// Returns a_default if a_name is not present or is not of type ParamValue
            std::string get_value( unsigned a_index, const std::string& a_default ) const;
            std::string get_value( unsigned a_index, const char* a_default ) const;
            /// Returns the result of ParamValue::get if a_name is present and is of type ParamValue
            /// Returns a_default if a_name is not present or is not of type ParamValue
            template< typename XValType >
            XValType get_value( unsigned a_index, XValType a_default ) const;

            /// Returns a pointer to the param corresponding to a_name.
            /// Returns NULL if a_name is not present.
            const param* at( unsigned a_index ) const;
            /// Returns a pointer to the param corresponding to a_name.
            /// Returns NULL if a_name is not present.
            param* at( unsigned a_index );

            /// Returns a pointer to the param_value (static-ly cast) corresponding to a_name.
            /// Returns NULL if a_name is not present.
            const param_value* value_at( unsigned a_index ) const;
            /// Returns a pointer to the param_value (static-ly cast) corresponding to a_name.
            /// Returns NULL if a_name is not present.
            param_value* value_at( unsigned a_index );

            /// Returns a pointer to the param_array (static-ly cast) corresponding to a_name.
            /// Returns NULL if a_name is not present.
            const param_array* array_at( unsigned a_index ) const;
            /// Returns a pointer to the param_array (static-ly cast) corresponding to a_name.
            /// Returns NULL if a_name is not present.
            param_array* array_at( unsigned a_index );

            /// Returns a pointer to the param_node (static-ly cast) corresponding to a_name.
            /// Returns NULL if a_name is not present.
            const param_node* node_at( unsigned a_index ) const;
            /// Returns a pointer to the param_node (static-ly cast) corresponding to a_name.
            /// Returns NULL if a_name is not present.
            param_node* node_at( unsigned a_index );

            /// Returns a reference to the param at a_index.
            /// Behavior is undefined if a_index is out-of-range.
            const param& operator[]( unsigned a_index ) const;
            /// Returns a reference to the param at a_index.
            /// Behavior is undefined if a_index is out-of-range.
            param& operator[]( unsigned a_index );

            const param* front() const;
            param* front();

            const param* back() const;
            param* back();

            // assign a copy of a_value to the array at a_index
            void assign( unsigned a_index, const param& a_value );
            // directly assign a_value_ptr to the array at a_index
            void assign( unsigned a_index, param* a_value_ptr );

            void push_back( const param& a_value );
            void push_back( param* a_value_ptr );

            void push_front( const param& a_value );
            void push_front( param* a_value_ptr );

            void append( const param_array& an_array );

            void erase( unsigned a_index );
            param* remove( unsigned a_index );
            void clear();

            iterator begin();
            const_iterator begin() const;

            iterator end();
            const_iterator end() const;

            reverse_iterator rbegin();
            const_reverse_iterator rbegin() const;

            reverse_iterator rend();
            const_reverse_iterator rend() const;

            virtual std::string to_string() const;

        protected:
            contents f_contents;
    };

    template< typename XValType >
    XValType param_array::get_value( unsigned a_index ) const
    {
        const param_value* value = value_at( a_index );
        if( value == NULL ) throw exception() << "No value is present at index <" << a_index << ">";
        return value->get< XValType >();
    }

    template< typename XValType >
    XValType param_array::get_value( unsigned a_index, XValType a_default ) const
    {
        const param_value* value = value_at( a_index );
        if( value == NULL ) return a_default;
        return value->get< XValType >();
    }



//#ifdef _WIN32
//    MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::map< std::string, param* >;
//#endif

    class MANTIS_API param_node : public param
    {
        public:
            typedef std::map< std::string, param* > contents;
            typedef contents::iterator iterator;
            typedef contents::const_iterator const_iterator;
            typedef contents::value_type contents_type;

            param_node();
            param_node( const param_node& orig );
            virtual ~param_node();

            param_node& operator=( const param_node& rhs );

            virtual param* clone() const;

            virtual bool is_null() const;
            virtual bool is_node() const;

            unsigned size() const;

            bool has( const std::string& a_name ) const;
            unsigned count( const std::string& a_name ) const;

            /// Returns the result of ParamValue::get if a_name is present and is of type ParamValue
            /// Throws an exception if a_name is not present or is not of type ParamValue
            std::string get_value( const std::string& a_name ) const;
            /// Returns the result of ParamValue::get if a_name is present and is of type ParamValue
            /// Throws an exception if a_name is not present or is not of type ParamValue
            template< typename XValType >
            XValType get_value( const std::string& a_name ) const;

            /// Returns the result of ParamValue::get if a_name is present and is of type ParamValue
            /// Returns a_default if a_name is not present or is not of type ParamValue
            std::string get_value( const std::string& a_name, const std::string& a_default ) const;
            std::string get_value( const std::string& a_name, const char* a_default ) const;
            /// Returns the result of ParamValue::get if a_name is present and is of type ParamValue
            /// Returns a_default if a_name is not present or is not of type ParamValue
            template< typename XValType >
            XValType get_value( const std::string& a_name, XValType a_default ) const;

            /// Returns a pointer to the param corresponding to a_name.
            /// Returns NULL if a_name is not present.
            const param* at( const std::string& a_name ) const;
            /// Returns a pointer to the param corresponding to a_name.
            /// Returns NULL if a_name is not present.
            param* at( const std::string& a_name );

            const param_value* value_at( const std::string& a_name ) const;
            param_value* value_at( const std::string& a_name );

            const param_array* array_at( const std::string& a_name ) const;
            param_array* array_at( const std::string& a_name );

            const param_node* node_at( const std::string& a_name ) const;
            param_node* node_at( const std::string& a_name );

            /// Returns a reference to the param corresponding to a_name.
            /// Throws an exception if a_name is not present.
            const param& operator[]( const std::string& a_name ) const;
            /// Returns a reference to the param corresponding to a_name.
            /// Adds a new value if a_name is not present.
            param& operator[]( const std::string& a_name );

            /// creates a copy of a_value
            bool add( const std::string& a_name, const param& a_value );
            /// directly adds (without copying) a_value_ptr
            bool add( const std::string& a_name, param* a_value_ptr );

            /// creates a copy of a_value
            void replace( const std::string& a_name, const param& a_value );
            /// directly adds (without copying) a_value_ptr
            void replace( const std::string& a_name, param* a_value_ptr );

            /// Merges the contents of a_object into this object.
            /// If names in the contents of a_object exist in this object,
            /// the values in this object corresponding to the matching names will be replaced.
            void merge( const param_node& a_object );

            void erase( const std::string& a_name );
            param* remove( const std::string& a_name );
            void clear();

            iterator begin();
            const_iterator begin() const;

            iterator end();
            const_iterator end() const;

            virtual std::string to_string() const;

        protected:
            contents f_contents;

    };

    template< typename XValType >
    XValType param_node::get_value( const std::string& a_name ) const
    {
        const param_value* value = value_at( a_name );
        if( value == NULL ) throw exception() << "No value with name <" << a_name << "> is present at this node";
        return value->get< XValType >();
    }

    template< typename XValType >
    XValType param_node::get_value( const std::string& a_name, XValType a_default ) const
    {
        const param_value* value = value_at( a_name );
        if( value == NULL ) return a_default;
        return value->get< XValType >();
    }



    MANTIS_API std::ostream& operator<<(std::ostream& out, const param& value);
    MANTIS_API std::ostream& operator<<(std::ostream& out, const param_value& value);
    MANTIS_API std::ostream& operator<<(std::ostream& out, const param_array& value);
    MANTIS_API std::ostream& operator<<(std::ostream& out, const param_node& value);



    //***************************************
    //************** INPUT ******************
    //***************************************

    class MANTIS_API param_input_json
    {
        public:
            param_input_json();
            virtual ~param_input_json();

            static param_node* read_file( const std::string& a_filename );
            static param_node* read_string( const std::string& a_json_str );
            static param_node* read_document( const rapidjson::Document& a_document );
            static param* read_value( const rapidjson::Value& a_value );
    };

    //***************************************
    //************** OUTPUT *****************
    //***************************************

    class MANTIS_API param_output_json
    {
        public:
            typedef rapidjson::Writer< rapidjson::FileStream, rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<> > rj_file_writer;
            typedef rapidjson::PrettyWriter< rapidjson::FileStream, rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<> > rj_pretty_file_writer;
            typedef rapidjson::Writer< rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<> > rj_string_writer;
            typedef rapidjson::PrettyWriter< rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<> > rj_pretty_string_writer;

            enum json_writing_style
            {
                k_compact,
                k_pretty
            };

        public:
            param_output_json();
            virtual ~param_output_json();

            static bool write_file( const param& a_to_write, const std::string& a_filename, json_writing_style a_style );
            static bool write_string( const param& a_to_write, std::string& a_string, json_writing_style a_style );
            template< class XWriter >
            static bool write_param( const param& a_to_write, XWriter* a_writer );
            template< class XWriter >
            static bool write_param_null( const param& a_to_write, XWriter* a_writer );
            template< class XWriter >
            static bool write_param_value( const param_value& a_to_write, XWriter* a_writer );
            template< class XWriter >
            static bool write_param_array( const param_array& a_to_write, XWriter* a_writer );
            template< class XWriter >
            static bool write_param_node( const param_node& a_to_write, XWriter* a_writer );

    };

    template< class XWriter >
    bool param_output_json::write_param( const param& a_to_write, XWriter* a_writer )
    {
        if( a_to_write.is_null() )
        {
            return param_output_json::write_param_null( a_to_write, a_writer );
        }
        if( a_to_write.is_value() )
        {
            return param_output_json::write_param_value( a_to_write.as_value(), a_writer );
        }
        if( a_to_write.is_array() )
        {
            return param_output_json::write_param_array( a_to_write.as_array(), a_writer );
        }
        if( a_to_write.is_node() )
        {
            return param_output_json::write_param_node( a_to_write.as_node(), a_writer );
        }
        MTWARN( mtlog_p, "parameter not written: <" << a_to_write << ">" );
        return false;
    }
    template< class XWriter >
    bool param_output_json::write_param_null( const param& /*a_to_write*/, XWriter* a_writer )
    {
        //MTWARN( mtlog_p, "writing null" );
        a_writer->Null();
        return true;
    }
    template< class XWriter >
    bool param_output_json::write_param_value( const param_value& a_to_write, XWriter* a_writer )
    {
        //MTWARN( mtlog_p, "writing value" );
        a_writer->String(a_to_write.to_string().c_str());
        return true;
    }
    template< class XWriter >
    bool param_output_json::write_param_array( const param_array& a_to_write, XWriter* a_writer )
    {
        //MTWARN( mtlog_p, "writing array" );
        a_writer->StartArray();
        for( param_array::const_iterator it = a_to_write.begin(); it != a_to_write.end(); ++it )
        {
            if( ! param_output_json::write_param( *(*it), a_writer ) )
            {
                MTERROR( mtlog_p, "Error while writing parameter array" );
                return false;
            }
        }
        a_writer->EndArray();
        return true;
    }
    template< class XWriter >
    bool param_output_json::write_param_node( const param_node& a_to_write, XWriter* a_writer )
    {
        //MTWARN( mtlog_p, "writing node" );
        a_writer->StartObject();
        for( param_node::const_iterator it = a_to_write.begin(); it != a_to_write.end(); ++it )
        {
            a_writer->String( it->first.c_str() );
            if( ! param_output_json::write_param( *(it->second), a_writer ) )
            {
                MTERROR( mtlog_p, "Error while writing parameter node" );
                return false;
            }
        }
        a_writer->EndObject();
        return true;
    }


} /* namespace mantis */

#endif /* MT_PARAM_HH_ */
