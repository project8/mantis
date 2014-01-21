/*
 * mt_param.hh
 *
 *  Created on: Jan 14, 2014
 *      Author: nsoblath
 */

#ifndef MT_CONFIG_NODE_HH_
#define MT_CONFIG_NODE_HH_

#include "document.h"

#include <map>
#include <sstream>
#include <string>

#include <iostream>

namespace mantis
{
    class param_data;
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
            virtual bool is_data() const;
            virtual bool is_array() const;
            virtual bool is_object() const;

            virtual const param& operator/(const std::string&) const;

/*
            param_data& as_data();
            param_array& as_array();
            param_node& as_object();

            const param_data& as_data() const;
            const param_array& as_array() const;
            const param_node& as_object() const;
*/

            virtual std::string to_string() const;

            static unsigned s_indent_level;
    };

    class param_data : public param
    {
        public:
            param_data();
            param_data(const param_data& orig);
            virtual ~param_data();

            virtual param* clone() const;

            virtual bool is_data() const;

            const std::string& value() const;
            template< typename XValType >
            XValType value();

            template< typename XStreamableType >
            param_data& operator<<( const XStreamableType& a_streamable );

            virtual std::string to_string() const;

        protected:
            std::stringstream f_data_str;
            mutable std::string f_data_buffer;

    };

    template< typename XValType >
    XValType param_data::value()
    {
        XValType t_return;
        f_data_str >> t_return;
        return t_return;
    }

    template< typename XStreamableType >
    param_data& param_data::operator<<( const XStreamableType& a_streamable )
    {
        f_data_str.str("");
        f_data_str << a_streamable;
        return *this;
    }


    class param_array : public param
    {
        public:
            param_array();
            param_array( const param_array& orig );
            virtual ~param_array();

            virtual param* clone() const;

            virtual bool is_array() const;

            virtual std::string to_string() const;

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

            virtual bool is_object() const;

            virtual const param& operator/(const std::string& a_name) const;

            bool has( const std::string& a_name ) const;
            unsigned count( const std::string& a_name ) const;

            /// Returns a pointer to the param corresponding to a_name.
            /// Returns NULL if a_name is not present.
            const param* at( const std::string& a_name ) const;
            /// Returns a pointer to the param corresponding to a_name.
            /// Returns NULL if a_name is not present.
            param* at( const std::string& a_name );

            const param_data* data_at( const std::string& a_name ) const;
            param_data* data_at( const std::string& a_name );

            const param_array* array_at( const std::string& a_name ) const;
            param_array* array_at( const std::string& a_name );

            const param_node* object_at( const std::string& a_name ) const;
            param_node* object_at( const std::string& a_name );

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
    std::ostream& operator<<(std::ostream& out, const param_data& value);
    std::ostream& operator<<(std::ostream& out, const param_array& value);
    std::ostream& operator<<(std::ostream& out, const param_node& value);



    //***************************************
    //************** READER *****************
    //***************************************

    class config_maker_json
    {
        public:
            config_maker_json();
            virtual ~config_maker_json();

            static param_node* read_file( const std::string& a_filename );
            static param_node* read_string( const std::string& a_json_str );
            static param_node* read_document( const rapidjson::Document& a_document );
            static param* read_value( const rapidjson::Value& a_value );
    };


    //***************************************
    //************** NODE *******************
    //***************************************

/*
    class config_node
    {
        public:
            config_node(const std::string& a_name);
            config_node(const param& orig);
            virtual ~config_node();

            const std::string& name() const;
            void set_name(const std::string& a_name);

            config_node* parent() const;

        protected:
            config_node();

            std::string f_name;

            config_node* f_parent;
    };

    class config_node_null : public param, public config_node
    {
        public:
            config_node_null(const std::string& a_name);
            config_node_null(const config_node_data& orig);
            virtual ~config_node_null();

        protected:
            config_node_null();
    };

    class config_node_data : public param_data, public config_node
    {
        public:
            config_node_data(const std::string& a_name);
            config_node_data(const config_node_data& orig);
            virtual ~config_node_data();

        protected:
            config_node_data();
    };

    class config_node_array : public param_array, public config_node
    {
        public:
            config_node_array(const std::string& a_name);
            config_node_array(const config_node_array& orig);
            virtual ~config_node_array();

        protected:
            config_node_array();
    };

    class config_node_object : public param_node, public config_node
    {
        public:
            config_node_object(const std::string& a_name);
            config_node_object(const config_node_object& orig);
            virtual ~config_node_object();

        protected:
            config_node_object();
    };
    */

} /* namespace Katydid */

#endif /* MT_CONFIG_NODE_HH_ */
