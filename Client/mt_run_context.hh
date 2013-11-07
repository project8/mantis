#ifndef MT_CONTEXT_HH_
#define MT_CONTEXT_HH_

#include "mt_connection.hh"
#include "request.pb.h"
#include "status.pb.h"
#include "response.pb.h"

namespace mantis
{

    class run_context
    {
        public:
            run_context();
            virtual ~run_context();

            void set_connection( connection* a_connection );
            connection* get_connection();

            request* get_request();
            bool push_request();
            bool pull_request();

            status* get_status();
            bool push_status();
            bool pull_status();

            response* get_response();
            bool push_response();
            bool pull_response();

        private:
            size_t reset_buffer( size_t a_size );

            connection* f_connection;
            request f_request;
            status f_status;
            response f_response;

            size_t f_buffer_size;
            char* f_buffer;
    };

}

#endif
