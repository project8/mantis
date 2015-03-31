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

            /// Assumes that the parameter is a value, and returns a reference to itself.
            const param_value& operator()() const;
            /// Assumes that the parameter is a value, and returns a reference to itself.
            param_value& operator()();

            /// Assumes that the parameter is an array, and returns a reference to the KTParam at aIndex.
            /// Behavior is undefined if aIndex is out-of-range.
            const param& operator[]( unsigned a_index ) const;
            /// Assumes that the parameter is an array, and returns a reference to the KTParam at aIndex.
            /// Behavior is undefined if aIndex is out-of-range.
            param& operator[]( unsigned a_index );

            /// Assumes that the parameter is a node, and returns a reference to the KTParam corresponding to aName.
            /// Throws an KTException if aName is not present.
            const param& operator[]( const std::string& a_name ) const;
            /// Assumes that the parameter is a node, and returns a reference to the KTParam corresponding to aName.
            /// Adds a new Value if aName is not present.
            param& operator[]( const std::string& a_name );

            virtual std::string to_string() const;

            static unsigned s_indent_level;
    };

/*
    class param_value_string;
    template< typename XValueType >
    class param_value_typed;

    struct dispatcher
    {
            template< typename XReturnType >
            XReturnType dispatch( const param_value_string& a_param );
            template< typename XReturnType >
            XReturnType dispatch( const param_value_typed< bool >& a_param );
            template< typename XReturnType >
            XReturnType dispatch( const param_value_typed< uint32_t >& a_param );
            template< typename XReturnType >
            XReturnType dispatch( const param_value_typed< uint64_t >& a_param );
            template< typename XReturnType >
            XReturnType dispatch( const param_value_typed< int32_t >& a_param );
            template< typename XReturnType >
            XReturnType dispatch( const param_value_typed< int64_t >& a_param );
            template< typename XReturnType >
            XReturnType dispatch( const param_value_typed< double >& a_param );
    };
*/
    class MANTIS_API param_value : public param
    {
        public:
            //static param_value* create( const std::string& a_value );
            //static param_value* create( const char* a_value );
            //static param_value* create( bool a_value );
            //static param_value* create( uint32_t a_value );
            //static param_value* create( uint64_t a_value );
            //static param_value* create( int32_t a_value );
            //static param_value* create( int64_t a_value );
            //static param_value* create( double a_value );

        public:
            param_value();
            param_value( bool a_value );
            param_value( uint8_t a_value );
            param_value( uint16_t a_value );
            param_value( uint32_t a_value );
            param_value( uint64_t a_value );
            param_value( int8_t a_value );
            param_value( int16_t a_value );
            param_value( int32_t a_value );
            param_value( int64_t a_value );
            param_value( float a_value );
            param_value( double a_value );
            param_value( const std::string& a_value );
            param_value( const char* a_value );
            param_value( const param_value& orig );
            virtual ~param_value();

            param_value& operator=( const param_value& rhs );

            virtual param* clone() const;

            bool empty() const;

            virtual bool is_null() const;
            virtual bool is_value() const;

            bool is_bool() const;
            bool is_uint() const;
            bool is_int() const;
            bool is_double() const;
            bool is_string() const;

            bool as_bool() const;
            uint64_t as_uint() const;
            int64_t as_int() const;
            double as_double() const;
            std::string as_string() const;

            //virtual void visit( dispatcher& ) const;

            //const std::string& get() const;
            template< typename XValType >
            XValType get() const;

            void set( bool a_value );
            void set( uint8_t a_value );
            void set( uint16_t a_value );
            void set( uint32_t a_value );
            void set( uint64_t a_value );
            void set( int8_t a_value );
            void set( int16_t a_value );
            void set( int32_t a_value );
            void set( int64_t a_value );
            void set( float a_value );
            void set( double a_value );
            void set( const std::string& a_value );
            void set( const char* a_value );
            //template< typename XValType >
            //void set( XValType a_value );

            //template< typename XStreamableType >
            //param_value& operator<<( const XStreamableType& a_streamable );

            virtual std::string to_string() const;

            void clear();

        private:
            union Values
            {
                bool f_bool;
                uint64_t f_uint;
                int64_t f_int;
                double f_double;
                std::string* f_string;
            } f_value;

            enum ValueTypes
            {
                k_bool,
                k_uint,
                k_int,
                k_double,
                k_string,
                k_invalid
            } f_value_type;

        protected:
            //virtual void do_get( std::string& a_return ) const;
            //virtual void do_set( const std::string& a_value );

            //virtual void do_set( const char* a_value );

            //virtual void do_get( bool& a_return ) const;
            //virtual void do_set( bool a_value );

            //virtual void do_get( uint32_t& a_return ) const;
            //virtual void do_set( uint32_t a_value );

            //virtual void do_get( uint64_t& a_return ) const;
            //virtual void do_set( uint64_t a_value );

            //virtual void do_get( int32_t& a_return ) const;
            //virtual void do_set( int32_t a_value );

            //virtual void do_get( int64_t& a_return ) const;
            //virtual void do_set( int64_t a_value );

            //virtual void do_get( double& a_return ) const;
            //virtual void do_set( double a_value );
/*
            template< typename XReturnType >
            XReturnType  do_get_as( const param_value_typed< bool >& a_param ) const
            {
                return a_param.get_as< XReturnType >();
            }
            template< typename XReturnType >
            XReturnType  do_get_as( const param_value_typed< uint32_t >& a_param ) const
            {
                return a_param.get_as< XReturnType >();
            }
            template< typename XReturnType >
            XReturnType  do_get_as( const param_value_typed< uint64_t >& a_param ) const
            {
                return a_param.get_as< XReturnType >();
            }
            template< typename XReturnType >
            XReturnType  do_get_as( const param_value_typed< int32_t >& a_param ) const
            {
                return a_param.get_as< XReturnType >();
            }
            template< typename XReturnType >
            XReturnType  do_get_as( const param_value_typed< int64_t >& a_param ) const
            {
                return a_param.get_as< XReturnType >();
            }
            template< typename XReturnType >
            XReturnType do_get_as( const param_value_typed< double >& a_param ) const
            {
                return a_param.get_as< XReturnType >();
            }
*/

            //template< typename XValType >
            //void do_get( XValType& a_return ) const;

            //template< typename XValType >
            //void do_set( XValType& a_value );
    };
/*
    template< typename XStreamableType >
    param_value::param_value( XStreamableType a_streamable ) :
            param(),
            f_value()
    {
        (*this) << a_streamable;
    }
*//*
    template< typename XValType >
    XValType param_value::get() const
    {
        XValType t_return;
        std::stringstream t_buffer;
        t_buffer << f_value;
        t_buffer >> t_return;
        return t_return;
    }
*//*
    template< typename XStreamableType >
    param_value& param_value::operator<<( const XStreamableType& a_streamable )
    {
        //std::stringstream t_buffer;
        //t_buffer << a_streamable;
        //f_value = t_buffer.str();
        return *this;
    }
*/

    template< typename XValType >
    XValType param_value::get() const
    {
        if( f_value_type == k_bool ) return static_cast< XValType >( as_bool() );
        else if( f_value_type == k_uint ) return static_cast< XValType >( as_uint() );
        else if( f_value_type == k_int ) return static_cast< XValType >( as_int() );
        else if( f_value_type == k_double ) return static_cast< XValType >( as_double() );
        else if( f_value_type == k_string )
        {
            std::stringstream t_conv;
            t_conv << k_string;
            XValType t_return;
            t_conv >> t_return;
            return t_return;
        }
        return XValType();
    }

    template<>
    inline std::string param_value::get< std::string >() const
    {
        return as_string();
    }
/*
    template< typename XValType >
    void param_value::set( XValType a_value )
    {
        do_set( a_value );
        return;
    }
*//*
    template< typename XValType >
    void param_value::do_get( XValType& a_return ) const
    {
        XValType t_return;
        do_get( t_return );
        return;
    }
*/
/*
    class MANTIS_API param_value_string : public param_value
    {
        public:
            param_value_string();
            param_value_string( const std::string& a_value );
            param_value_string( const char* a_value );
            template< typename XStreamableType >
            param_value_string( XStreamableType a_streamable );
            param_value_string(const param_value_string& orig);
            virtual ~param_value_string();

            param_value_string& operator=( const param_value_string& rhs );

            virtual param* clone() const;

            virtual bool empty() const;

            virtual void visit( dispatcher& a_disp ) const
            {
                a_disp.dispatch< std::string >( *this );
            }
            void do_something() const
            {
                std::cout << "here i am!" << std::endl;
            }

            //template< typename XStreamableType >
            //param_value& operator<<( const XStreamableType& a_streamable );

            virtual std::string to_string() const;

            virtual void clear();

        protected:
            virtual void do_get( std::string& a_return ) const;
            virtual void do_set( const std::string& a_value );

            virtual void do_set( const char* a_value );

            std::string f_value;
    };

    template< typename XStreamableType >
    param_value_string::param_value_string( XStreamableType a_streamable ) :
            param_value(),
            f_value()
    {
        (*this) << a_streamable;
    }

    template< typename XValueType >
    class MANTIS_API param_value_typed : public param_value
    {
        public:
            param_value_typed();
            param_value_typed( XValueType a_double );
            param_value_typed(const param_value_typed& orig);
            virtual ~param_value_typed();

            param_value_typed& operator=( const param_value_typed& rhs );

            virtual param* clone() const;

            bool empty() const;

            virtual void visit( dispatcher& a_disp ) const
            {
                a_disp.dispatch( *this );
            }
            void do_something() const
            {
                std::cout << "here i am!" << std::endl;
            }

            //template< typename XStreamableType >
            //param_value& operator<<( const XStreamableType& a_streamable );

            template< typename XReturnType >
            XReturnType get_as() const
            {
                return static_cast< XReturnType >( f_value );
            }

            virtual std::string to_string() const;

            void clear();

        protected:
            virtual void do_get( XValueType& a_return ) const;
            virtual void do_set( XValueType a_value );

            XValueType f_value;
    };

    template< typename XValueType >
    param_value_typed< XValueType >::param_value_typed() :
            param_value(),
            f_value()
    {
    }

    template< typename XValueType >
    param_value_typed< XValueType >::param_value_typed( XValueType a_value ) :
            param_value(),
            f_value( a_value )
    {
    }

    template< typename XValueType >
    param_value_typed< XValueType >::param_value_typed( const param_value_typed< XValueType >& orig ) :
            param_value( orig ),
            f_value( orig.f_value )
    {
    }

    template< typename XValueType >
    param_value_typed< XValueType >::~param_value_typed()
    {
    }

    template< typename XValueType >
    param_value_typed< XValueType >& param_value_typed< XValueType >::operator=( const param_value_typed< XValueType >& rhs )
    {
        f_value = rhs.f_value;
        return *this;
    }

    template< typename XValueType >
    param* param_value_typed< XValueType >::clone() const
    {
        return new param_value_typed< XValueType >( *this );
    }

    template< typename XValueType >
    bool param_value_typed< XValueType >::empty() const
    {
        return f_value == false;
    }

    template< typename XValueType >
    void param_value_typed< XValueType >::clear()
    {
        f_value = false;
        return;
    }

    template< typename XValueType >
    std::string param_value_typed< XValueType >::to_string() const
    {
        std::stringstream t_conv;
        t_conv << f_value;
        return std::string( t_conv.str() );
    }

    template< typename XValueType >
    void param_value_typed< XValueType >::do_get( XValueType& a_return ) const
    {
        a_return = f_value;
        return;
    }

    template< typename XValueType >
    void param_value_typed< XValueType >::do_set( XValueType a_value )
    {
        f_value = a_value;
        return;
    }



    //struct dispatcher_imp : dispatcher
    //{
        //virtual ~dispatcher_imp() {}
    template< typename XReturnType >
    XReturnType dispatcher::dispatch( const param_value_string& a_param )
    {
        std::cout << "string!" << std::endl;
        a_param.do_something();
    }
    template< typename XReturnType >
    XReturnType dispatcher::dispatch( const param_value_typed< bool >& a_param )
    {
        std::cout << "bool!" << std::endl;
        a_param.do_something();
    }
    template< typename XReturnType >
    XReturnType dispatcher::dispatch( const param_value_typed< uint32_t >& a_param )
    {
        std::cout << "uint32_t!" << std::endl;
        a_param.do_something();
    }
    template< typename XReturnType >
    XReturnType dispatcher::dispatch( const param_value_typed< uint64_t >& a_param )
    {
        std::cout << "uint64_t!" << std::endl;
        a_param.do_something();
    }
    template< typename XReturnType >
    XReturnType dispatcher::dispatch( const param_value_typed< int32_t >& a_param )
    {
        std::cout << "int32_t!" << std::endl;
        a_param.do_something();
    }
    template< typename XReturnType >
    XReturnType dispatcher::dispatch( const param_value_typed< int64_t >& a_param )
    {
        std::cout << "int64_t!" << std::endl;
        a_param.do_something();
    }
    template< typename XReturnType >
    XReturnType dispatcher::dispatch( const param_value_typed< double >& a_param )
    {
        std::cout << "double!" << std::endl;
        a_param.do_something();
    }
*/


/*
    class MANTIS_API param_value_double : public param_value
    {
        public:
            param_value_double();
            param_value_double( double a_double );
            param_value_double(const param_value_double& orig);
            virtual ~param_value_double();

            param_value_double& operator=( const param_value_double& rhs );

            virtual param* clone() const;

            bool empty() const;

            //template< typename XStreamableType >
            //param_value& operator<<( const XStreamableType& a_streamable );

            virtual std::string to_string() const;

            void clear();

        protected:
            virtual void do_get( double& a_return ) const;
            virtual void do_set( double a_value );

            double f_value;
    };
*/




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
            bool empty() const;

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


} /* namespace mantis */

#endif /* MT_PARAM_HH_ */
