/*
 * mt_run_client.hh
 *
 *  Created on: Mar 7, 2014
 *      Author: nsoblath
 */

#ifndef MT_RUN_CLIENT_HH_
#define MT_RUN_CLIENT_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_param.hh"


#define RETURN_SUCCESS 1
#define RETURN_ERROR -1
#define RETURN_CANCELED -2
#define RETURN_REVOKED -3


namespace mantis
{
    class client_file_writing;
    class run_context_dist;

    class run_client : public callable
    {
        private:
            class setup_loop : public callable
            {
                public:
                    setup_loop( run_context_dist* a_run_context );
                    virtual ~setup_loop();

                    void execute();
                    void cancel();

                    int get_return();

                private:
                    run_context_dist* f_run_context;
                    atomic_bool f_canceled;
                    int f_return;
            };

            class run_loop : public callable
            {
                public:
                    run_loop( run_context_dist* a_run_context, client_file_writing* a_file_writing = NULL );
                    virtual ~run_loop();

                    void execute();
                    void cancel();

                    int get_return();

                private:
                    run_context_dist* f_run_context;
                    client_file_writing* f_file_writing;
                    atomic_bool f_canceled;
                    int f_return;
            };

        public:
            run_client( const param_node* a_node, const std::string& a_exe_name = "unknown" );
            virtual ~run_client();

            void execute();
            void cancel();

            int get_return();

        private:
            param_node f_config;
            std::string f_exe_name;
            atomic_bool f_canceled;
            int f_return;
    };

} /* namespace mantis */

#endif /* MT_RUN_CLIENT_HH_ */
