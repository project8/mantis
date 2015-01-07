/*
 * mt_client_file_writing.hh
 *
 *  Created on: Nov 26, 2013
 *      Author: nsoblath
 */

#ifndef MT_CLIENT_FILE_WRITING_HH_
#define MT_CLIENT_FILE_WRITING_HH_

namespace mantis
{
    class buffer;
    class client_worker;
    class condition;
    class file_writer;
    class param_node;
    class record_receiver;
    class run_context_dist;
    class server_tcp;
    class thread;

    class client_file_writing
    {
        public:
            client_file_writing( const param_node* a_config, run_context_dist* a_run_context_dist, int a_write_port );
            virtual ~client_file_writing();

            void wait_for_finish();

            void cancel();

            bool is_done();

        private:
            const param_node* f_config;
            server_tcp* f_server;
            condition *f_buffer_condition;
            buffer *f_buffer;
            record_receiver* f_receiver;
            file_writer *f_writer;
            client_worker* f_worker;
            thread* f_thread;
    };

} /* namespace mantis */
#endif /* MT_CLIENT_FILE_WRITING_HH_ */
