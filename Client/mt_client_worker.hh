#ifndef MT_CLIENT_WORKER_HH_
#define MT_CLIENT_WORKER_HH_

#include "mt_callable.hh"

#include "mt_condition.hh"
#include "request.pb.h"
#include "mt_record_receiver.hh"
#include "mt_writer.hh"

namespace mantis
{

    class client_worker :
        public callable
    {
        public:
            client_worker( request* a_request, record_receiver* a_receiver, writer* a_writer, condition* a_buffer_condition );
            virtual ~client_worker();

            void execute();
            void cancel();

        private:
            record_receiver* f_receiver;
            writer* f_writer;
            condition* f_buffer_condition;

            enum thread_state
            {
                k_inactive,
                k_running
            } f_receiver_state, f_writer_state;
    };

}

#endif
