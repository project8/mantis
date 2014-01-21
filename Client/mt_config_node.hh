/*
 * mt_param.hh
 *
 *  Created on: Jan 14, 2014
 *      Author: nsoblath
 */

#ifndef MT_CONFIG_NODE_HH_
#define MT_CONFIG_NODE_HH_

#include "document.h"

#include <deque>
#include <map>
#include <sstream>
#include <string>

#include <iostream>

namespace mantis
{
    class param_value;
    class param_array;
    class param_node;

    class param
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

    class param_value : public param
    {
        public:
            param_value();
            param_value(const param_value& orig);
            virtual ~param_value();

            virtual param* clone() const;

            virtual bool is_value() const;

            const std::string& get() const;
            template< typename XValType >
            XValType get();

            template< typename XStreamableType >
            param_value& operator<<( const XStreamableType& a_streamable );

            virtual std::string to_string() const;

        protected:
            std::stringstream f_value_str;
            mutable std::string f_value_buffer;

    };

    template< typename XValType >
    XValType param_value::get()
    {
        XValType t_return;
        f_value_str >> t_return;
        return t_return;
    }

    template< typename XStreamableType >
    param_value& param_value::operator<<( const XStreamableType& a_streamable )
    {
        f_value_str.str("");
        f_value_str << a_streamable;
        return *this;
    }


    class param_array : public param
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

            virtual param* clone() const;

            virtual bool is_array() const;

            unsigned size() const;
            bool empty() const;

            /// sets the size of the array
            /// if smaller than the current size, extra elements are deleted
            void resize( unsigned a_size );

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

            void erase( unsigned a_index );
            param* remove( unsigned a_index );

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

    class param_node : public param
    {
        public:
            typedef std::map< std::string, param* > contents;
            typedef contents::iterator iterator;
            typedef contents::const_iterator const_iterator;
            typedef contents::value_type contents_type;

            param_node();
            param_node( const param_node& orig );
            virtual ~param_node();

            virtual param* clone() const;

            virtual bool is_node() const;

            bool has( const std::string& a_name ) const;
            unsigned count( const std::string& a_name ) const;

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
            void merge( const param_node* a_object );

            void erase( const std::string& a_name );
            param* remove( const std::string& a_name );

            iterator begin();
            const_iterator begin() const;

            iterator end();
            const_iterator end() const;

            virtual std::string to_string() const;

        protected:
            contents f_contents;

    };



    std::ostream& operator<<(std::ostream& out, const param& value);
    std::ostream& operator<<(std::ostream& out, const param_value& value);
    std::ostream& operator<<(std::ostream& out, const param_array& value);
    std::ostream& operator<<(std::ostream& out, const param_node& value);



    //***************************************
    //************** INPUT ******************
    //***************************************

    class param_input_json
    {
        public:
            param_input_json();
            virtual ~param_input_json();

            static param_node* read_file( const std::string& a_filename );
            static param_node* read_string( const std::string& a_json_str );
            static param_node* read_document( const rapidjson::Document& a_document );
            static param* read_value( const rapidjson::Value& a_value );
    };

} /* namespace Katydid */

#endif /* MT_CONFIG_NODE_HH_ */
