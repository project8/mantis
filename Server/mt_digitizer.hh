#ifndef MT_DIGITIZER_HH_
#define MT_DIGITIZER_HH_

#include "mt_callable.hh"

#include "request.pb.h"

#include <cstddef>

namespace mantis
{
    class buffer;
    class condition;
    class response;

    class digitizer :
        public callable
    {
        public:
            digitizer();
            virtual ~digitizer();

            virtual void allocate( buffer* a_buffer, condition* a_condition ) = 0;
            virtual void initialize( request* a_request ) = 0;
            virtual void finalize( response* a_response ) = 0;

            virtual bool write_mode_check( request_file_write_mode_t mode ) = 0;

    };
}

#endif
