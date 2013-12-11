#ifndef MT_RUN_CONTEXT_DIST_HH_
#define MT_RUN_CONTEXT_DIST_HH_

#include "mt_atomic.hh"
#include "mt_callable.hh"
#include "mt_condition.hh"
#include "mt_distribution.hh"
#include "mt_mutex.hh"
#include "request.pb.h"
#include "status.pb.h"
#include "client_status.pb.h"
#include "response.pb.h"

namespace mantis
{

    class run_context_dist : public distribution, public callable
    {
        private:
            typedef uint32_t message_id_type;

        public:
            run_context_dist();
            virtual ~run_context_dist();

            void execute();

            void cancel();

            bool is_active();
            void wait_until_active();

            bool wait_for_request();
            bool wait_for_status();
            bool wait_for_client_status();
            bool wait_for_response();

            /// return values:
            ///  1 = success
            ///  0 = closed connection
            /// -1 = error
            int pull_next_message( int flags = 0 );

            request* lock_request_out();
            bool push_request( int flags = 0 );
            bool push_request_no_mutex( int flags = 0 );
            request* lock_request_in();
            bool pull_request( int flags = 0 );
            bool pull_request_no_mutex( int flags = 0 );

            status* lock_status_out();
            bool push_status( int flags = 0 );
            bool push_status_no_mutex( int flags = 0 );
            status* lock_status_in();
            bool pull_status( int flags = 0 );
            bool pull_status_no_mutex( int flags = 0 );

            client_status* lock_client_status_out();
            bool push_client_status( int flags = 0 );
            bool push_client_status_no_mutex( int flags = 0 );
            client_status* lock_client_status_in();
            bool pull_client_status( int flags = 0 );
            bool pull_client_status_no_mutex( int flags = 0 );

            response* lock_response_out();
            bool push_response( int flags = 0 );
            bool push_response_no_mutex( int flags = 0 );
            response* lock_response_in();
            bool pull_response( int flags = 0 );
            bool pull_response_no_mutex( int flags = 0 );

            void unlock_outbound();
            void unlock_inbound();

        private:
            bool verify_message_type( message_id_type a_type_wanted, message_id_type& a_type_found, int flags = 0 );

            request f_request_out;
            request f_request_in;
            status f_status_out;
            status f_status_in;
            client_status f_client_status_out;
            client_status f_client_status_in;
            response f_response_out;
            response f_response_in;

            static const message_id_type f_unknown_id;
            static const message_id_type f_request_id;
            static const message_id_type f_status_id;
            static const message_id_type f_client_status_id;
            static const message_id_type f_response_id;

            mutex f_outbound_mutex;
            mutex f_inbound_mutex_read;
            mutex f_inbound_mutex_write;

            atomic_bool f_is_active;
            atomic_bool f_is_canceled;
            condition f_is_active_condition;
            mutex f_is_active_mutex;

            condition f_request_condition;
            condition f_status_condition;
            condition f_client_status_condition;
            condition f_response_condition;
    };

}

#endif
