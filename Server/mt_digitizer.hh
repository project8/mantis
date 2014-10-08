#ifndef MT_DIGITIZER_HH_
#define MT_DIGITIZER_HH_

#include "mt_callable.hh"

#include "request.pb.h"

#include "thorax.hh"

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

            virtual unsigned data_type_size() = 0;

            const dig_calib_params& params() const;
            dig_calib_params& params();

        public:
            virtual bool run_basic_test() = 0;
            bool run_insitu_test();

        protected:
            struct dig_calib_params f_params;
    };

#define MT_REGISTER_DIGITIZER(dig_class, dig_name) \
        static registrar< digitizer, dig_class > s_##dig_class##_digitizer_registrar( dig_name );

}

#endif
