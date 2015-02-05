#define MANTIS_API_EXPORTS

#include "mt_exception.hh"

namespace mantis
{

    exception::exception() :
            std::exception(),
            f_exception( "" )
    {
    }
    exception::exception( const exception& an_exception ) :
            std::exception(),
            f_exception( an_exception.f_exception.str() )
    {
    }

    exception::~exception() throw ()
    {
    }

    const char* exception::what() const throw ()
    {
        return f_exception.str().c_str();
    }

}
