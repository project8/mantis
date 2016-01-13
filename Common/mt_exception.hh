#ifndef MT_EXCEPTION_HH_
#define MT_EXCEPTION_HH_

#include "mt_api.hh"

#include <sstream>
#include <exception>

//#ifdef _WIN32
//MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::basic_stringstream< char, std::char_traits< char >, std::allocator< char > >;
//#endif

namespace mantis
{

    class MANTIS_API exception : public std::exception
    {
        public:
            exception();
            exception( const exception& );
            ~exception() throw ();

            template< class x_streamable >
            exception& operator<<( const x_streamable& a_fragment )
            {
                f_exception << a_fragment;
                return *this;
            }

            virtual const char* what() const throw();

        private:
            std::stringstream f_exception;
    };

}

#endif
