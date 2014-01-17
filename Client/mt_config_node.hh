/*
 * mt_config_value.hh
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
    class config_value_data;
    class config_value_array;
    class config_value_object;

    class config_value
    {
        public:
            config_value();
            config_value(const config_value& orig);
            virtual ~config_value();

            virtual config_value* clone() const;

            virtual bool is_data() const;
            virtual bool is_array() const;
            virtual bool is_object() const;

            virtual const config_value& operator/(const std::string&) const;

/*
            config_value_data& as_data();
            config_value_array& as_array();
            config_value_object& as_object();

            const config_value_data& as_data() const;
            const config_value_array& as_array() const;
            const config_value_object& as_object() const;
*/

            virtual std::string to_string() const;

            static unsigned s_indent_level;
    };

    class config_value_data : public config_value
    {
        public:
            config_value_data();
            config_value_data(const config_value_data& orig);
            virtual ~config_value_data();

            virtual config_value* clone() const;

            virtual bool is_data() const;

            const std::string& value() const;
            template< typename XValType >
            XValType value();

            template< typename XStreamableType >
            config_value_data& operator<<( const XStreamableType& a_streamable );

            virtual std::string to_string() const;

        protected:
            std::stringstream f_data_str;
            mutable std::string f_data_buffer;

    };

    template< typename XValType >
    XValType config_value_data::value()
    {
        XValType t_return;
        f_data_str >> t_return;
        return t_return;
    }

    template< typename XStreamableType >
    config_value_data& config_value_data::operator<<( const XStreamableType& a_streamable )
    {
        f_data_str.str("");
        f_data_str << a_streamable;
        return *this;
    }


    class config_value_array : public config_value
    {
        public:
            config_value_array();
            config_value_array( const config_value_array& orig );
            virtual ~config_value_array();

            virtual config_value* clone() const;

            virtual bool is_array() const;

            virtual std::string to_string() const;

    };

    class config_value_object : public config_value
    {
        public:
            typedef std::map< std::string, config_value* > contents;
            typedef contents::iterator iterator;
            typedef contents::const_iterator const_iterator;
            typedef contents::value_type contents_type;

            config_value_object();
            config_value_object( const config_value_object& orig );
            virtual ~config_value_object();

            virtual config_value* clone() const;

            virtual bool is_object() const;

            virtual const config_value& operator/(const std::string& a_name) const;

            bool has( const std::string& a_name ) const;
            unsigned count( const std::string& a_name ) const;

            /// Returns a pointer to the config_value corresponding to a_name.
            /// Returns NULL if a_name is not present.
            const config_value* at( const std::string& a_name ) const;
            /// Returns a pointer to the config_value corresponding to a_name.
            /// Returns NULL if a_name is not present.
            config_value* at( const std::string& a_name );

            const config_value_data* data_at( const std::string& a_name ) const;
            config_value_data* data_at( const std::string& a_name );

            const config_value_array* array_at( const std::string& a_name ) const;
            config_value_array* array_at( const std::string& a_name );

            const config_value_object* object_at( const std::string& a_name ) const;
            config_value_object* object_at( const std::string& a_name );

            /// Returns a reference to the config_value corresponding to a_name.
            /// Throws an exception if a_name is not present.
            const config_value& operator[]( const std::string& a_name ) const;
            /// Returns a reference to the config_value corresponding to a_name.
            /// Adds a new value if a_name is not present.
            config_value& operator[]( const std::string& a_name );

            /// creates a copy of a_value
            bool add( const std::string& a_name, const config_value& a_value );
            /// directly adds (without copying) a_value_ptr
            bool add( const std::string& a_name, config_value* a_value_ptr );

            /// creates a copy of a_value
            void replace( const std::string& a_name, const config_value& a_value );
            /// directly adds (without copying) a_value_ptr
            void replace( const std::string& a_name, config_value* a_value_ptr );

            /// Merges the contents of a_object into this object.
            /// If names in the contents of a_object exist in this object,
            /// the values in this object corresponding to the matching names will be replaced.
            void merge( const config_value_object* a_object );

            void erase( const std::string& a_name );
            config_value* remove( const std::string& a_name );

            iterator begin();
            const_iterator begin() const;

            iterator end();
            const_iterator end() const;

            virtual std::string to_string() const;

        protected:
            contents f_contents;

    };



    std::ostream& operator<<(std::ostream& out, const config_value& value);
    std::ostream& operator<<(std::ostream& out, const config_value_data& value);
    std::ostream& operator<<(std::ostream& out, const config_value_array& value);
    std::ostream& operator<<(std::ostream& out, const config_value_object& value);



    //***************************************
    //************** READER *****************
    //***************************************

    class config_maker_json
    {
        public:
            config_maker_json();
            virtual ~config_maker_json();

            static config_value_object* read_file( const std::string& a_filename );
            static config_value_object* read_string( const std::string& a_json_str );
            static config_value_object* read_document( const rapidjson::Document& a_document );
            static config_value* read_value( const rapidjson::Value& a_value );
    };


    //***************************************
    //************** NODE *******************
    //***************************************

/*
    class config_node
    {
        public:
            config_node(const std::string& a_name);
            config_node(const config_value& orig);
            virtual ~config_node();

            const std::string& name() const;
            void set_name(const std::string& a_name);

            config_node* parent() const;

        protected:
            config_node();

            std::string f_name;

            config_node* f_parent;
    };

    class config_node_null : public config_value, public config_node
    {
        public:
            config_node_null(const std::string& a_name);
            config_node_null(const config_node_data& orig);
            virtual ~config_node_null();

        protected:
            config_node_null();
    };

    class config_node_data : public config_value_data, public config_node
    {
        public:
            config_node_data(const std::string& a_name);
            config_node_data(const config_node_data& orig);
            virtual ~config_node_data();

        protected:
            config_node_data();
    };

    class config_node_array : public config_value_array, public config_node
    {
        public:
            config_node_array(const std::string& a_name);
            config_node_array(const config_node_array& orig);
            virtual ~config_node_array();

        protected:
            config_node_array();
    };

    class config_node_object : public config_value_object, public config_node
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
