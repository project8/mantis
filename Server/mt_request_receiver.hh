#ifndef MT_REQUEST_RECEIVER_HH_
#define MT_REQUEST_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_mutex.hh"
#include "mt_param.hh"

namespace mantis
{
    class buffer;
    class condition;
    class broker;
    class run_database;
    class server_tcp;

    class request_receiver : public callable
    {
        public:
            request_receiver( const param_node* a_config, broker* a_broker, run_database* a_run_database, condition* a_condition, const std::string& a_exe_name = "unknown" );
            virtual ~request_receiver();

            void execute();
            void cancel();

        private:
            void apply_config( const std::string& a_config_addr, const param_value& a_value );

            mutex f_msc_mutex;
            param_node f_master_server_config;

            broker* f_broker;
            run_database* f_run_database;
            condition* f_queue_condition;
            std::string f_exe_name;
    };

}

#endif
