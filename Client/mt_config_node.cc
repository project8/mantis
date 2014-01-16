/*
 * mt_config_value.cc
 *
 *  Created on: Jan 14, 2014
 *      Author: nsoblath
 */

#include "mt_config_node.hh"

#include "mt_exception.hh"

using std::string;

namespace mantis
{

    config_value::config_value()
    {
    }

    config_value::config_value( const config_value& )
    {
    }

    config_value::~config_value()
    {
    }

    config_value* config_value::clone()
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
        f_data_str << orig.f_data_str;
    }

    config_value_data::~config_value_data()
    {
    }

    config_value* config_value_data::clone()
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

    config_value* config_value_array::clone()
    {
        return new config_value_array( *this );
    }

    bool config_value_array::is_array() const
    {
        return true;
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
            this->replace( it->first, it->second->clone() );
        }
    }

    config_value_object::~config_value_object()
    {
        for( iterator it = f_contents.begin(); it != f_contents.end(); ++it )
        {
            delete it->second;
        }
    }

    config_value* config_value_object::clone()
    {
        return new config_value_object( *this );
    }

    bool config_value_object::is_object() const
    {
        return true;
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
            this->replace( it->first, it->second->clone() );
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
