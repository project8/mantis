#ifndef MT_EXCEPTION_HH_
#define MT_EXCEPTION_HH_

#include <sstream>
#include <exception>

namespace mantis
{

    class exception :
        public std::exception
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
