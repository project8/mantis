#ifndef MT_DIGITIZER_HH_
#define MT_DIGITIZER_HH_

#include "mt_callable.hh"

#include "thorax.hh"

#include <cstddef>

namespace mantis
{
    class buffer;
    class condition;
    class param_node;

    class MANTIS_API digitizer : public callable
    {
        public:
            digitizer();
            virtual ~digitizer();

            virtual bool allocate() = 0;
            virtual bool deallocate() = 0;

            virtual bool initialize( param_node* a_global_config, param_node* a_dev_config ) = 0;
            virtual void finalize( param_node* a_response ) = 0;

            virtual unsigned data_type_size() = 0;

            const dig_calib_params& params() const;
            dig_calib_params& params();

            buffer* get_buffer();
            condition* get_buffer_condition();

        public:
            virtual bool run_basic_test() = 0;
            bool run_insitu_test();

        protected:
            struct dig_calib_params f_params;

            buffer* f_buffer;
            condition* f_buffer_condition;
    };

#define MT_REGISTER_DIGITIZER(dig_class, dig_name) \
        static registrar< digitizer, dig_class > s_##dig_class##_digitizer_registrar( dig_name );

}

#endif
