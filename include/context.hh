#ifndef RUN_HH_
#define RUN_HH_

#include "connection.hh"
#include "request.pb.h"
#include "status.pb.h"
#include "response.pb.h"

namespace mantis
{

    class context
    {
        public:
            context( connection* a_connection );
            virtual ~context();

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
            ::mantis::request f_request;
            ::mantis::status f_status;
            ::mantis::response f_response;
    };

}

#endif
