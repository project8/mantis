#ifndef RUN_HH_
#define RUN_HH_

#include "connection.hh"
#include "request.pb.h"
#include "response.pb.h"

namespace mantis
{

    class run
    {
        public:
            run( connection* a_connection );
            virtual ~run();

            void push_request();
            void pull_request();
            request& get_request();

            void push_response();
            void pull_response();
            response& get_response();

        private:
            connection* f_connection;
            request f_request;
            response f_response;
    };

}

#endif
