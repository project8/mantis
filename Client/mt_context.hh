#ifndef MT_CONTEXT_HH_
#define MT_CONTEXT_HH_

#include "mt_connection.hh"
#include "request.pb.h"
#include "status.pb.h"
#include "response.pb.h"

namespace mantis
{

    class context
    {
        public:
            context();
            virtual ~context();

            void set_connection( connection* a_connection );
            connection* get_connection();

            request* get_request();
            void push_request();
            void pull_request();

            status* get_status();
            void push_status();
            void pull_status();

            response* get_response();
            void push_response();
            void pull_response();

        private:
            connection* f_connection;
            request f_request;
            status f_status;
            response f_response;

            static const unsigned long int f_buffer_length = 1024;
            char f_buffer[ f_buffer_length ];
    };

}

#endif
