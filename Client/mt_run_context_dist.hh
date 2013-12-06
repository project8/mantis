#ifndef MT_RUN_CONTEXT_DIST_HH_
#define MT_RUN_CONTEXT_DIST_HH_

#include "mt_distribution.hh"
#include "request.pb.h"
#include "status.pb.h"
#include "client_status.pb.h"
#include "response.pb.h"

namespace mantis
{

    class run_context_dist : public distribution
    {
        private:
            typedef uint32_t message_id_type;

        public:
            run_context_dist();
            virtual ~run_context_dist();

            bool pull_next_message( int flags = 0 );

            request* get_request();
            bool push_request( int flags = 0 );
            bool pull_request( int flags = 0 );

            status* get_status();
            bool push_status( int flags = 0 );
            bool pull_status( int flags = 0 );

            client_status* get_client_status();
            bool push_client_status( int flags = 0 );
            bool pull_client_status( int flags = 0 );

            response* get_response();
            bool push_response( int flags = 0 );
            bool pull_response( int flags = 0 );

        private:
            bool verify_message_type( message_id_type, int flags = 0 );

            request f_request;
            status f_status;
            client_status f_client_status;
            response f_response;

            static const message_id_type f_request_id;
            static const message_id_type f_status_id;
            static const message_id_type f_client_status_id;
            static const message_id_type f_response_id;
    };

}

#endif
