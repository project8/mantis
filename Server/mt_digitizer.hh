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

            virtual bool allocate( buffer* a_buffer, condition* a_condition ) = 0;
            virtual bool initialize( request* a_request ) = 0;
            virtual void finalize( response* a_response ) = 0;

            virtual bool write_mode_check( request_file_write_mode_t mode ) = 0;

            virtual unsigned bit_depth() = 0;
            virtual unsigned data_type_size() = 0;

    };

    class test_digitizer
    {
        public:
            test_digitizer() {}
            virtual ~test_digitizer() {}

            virtual bool run_test() = 0;
    };

}

#endif
