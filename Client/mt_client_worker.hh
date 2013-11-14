#ifndef MT_CLIENT_WORKER_HH_
#define MT_CLIENT_WORKER_HH_

#include "mt_callable.hh"

#include "mt_writer.hh"
#include "mt_condition.hh"
#include "request.pb.h"
#include "mt_server.hh"

namespace mantis
{

    class client_worker :
        public callable
    {
        public:
            client_worker( request* a_request, writer* a_writer, condition* a_buffer_condition );
            virtual ~client_worker();

            void execute();

        private:
            server* f_server;
            writer* f_writer;
            condition* f_buffer_condition;
    };

}

#endif
