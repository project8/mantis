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

    struct MANTIS_API digitizer_config_template
    {
        virtual ~digitizer_config_template() {};
        virtual void add( param_node* a_node, const std::string& a_type ) = 0;
    };

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

            const dig_calib_params& params( unsigned i_chan ) const;
            dig_calib_params& params( unsigned i_chan );

            buffer* get_buffer();
            condition* get_buffer_condition();

            enum status {
                k_ok = 0,
                k_warning = 1,
                k_error = 2
            };
            status get_status() const;
            const std::string& get_status_message() const;
            void set_status( status a_status, const std::string& a_message );

        public:
            virtual bool run_basic_test() = 0;
            bool run_insitu_test();

        protected:
            struct dig_calib_params* f_params;

            buffer* f_buffer;
            condition* f_buffer_condition;

            status f_status;
            std::string f_status_message;
    };

    inline const dig_calib_params& digitizer::params( unsigned i_chan ) const
    {
        return f_params[ i_chan ];
    }

    inline dig_calib_params& digitizer::params( unsigned i_chan )
    {
        return f_params[ i_chan ];
    }

    inline buffer* digitizer::get_buffer()
    {
        return f_buffer;
    }

    inline condition* digitizer::get_buffer_condition()
    {
        return f_buffer_condition;
    }


    inline digitizer::status digitizer::get_status() const
    {
        return f_status;
    }

    inline const std::string& digitizer::get_status_message() const
    {
        return f_status_message;
    }

    inline void digitizer::set_status( digitizer::status a_status, const std::string& a_message )
    {
        f_status = a_status;
        f_status_message = a_message;
        return;
    }

#define MT_REGISTER_DIGITIZER(dig_class, dig_name) \
        static registrar< digitizer, dig_class > s_##dig_class##_digitizer_registrar( dig_name ); \
        static registrar< digitizer_config_template, dig_class##_config_template > s_##dig_class##_config_template_registrar( dig_name );

}

#endif
