#ifndef MT_ACQ_REQUEST_DB_HH_
#define MT_ACQ_REQUEST_DB_HH_

#include "mt_mutex.hh"

#include <boost/uuid/uuid.hpp>

#include <list>
#include <map>
#include <string>

namespace mantis
{
    class acq_request;
    class condition;
    class config_manager;
    class param_node;

    struct request_reply_package;

//#ifdef _WIN32
//    MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::list< acq_request* >;
//    MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::map< unsigned, acq_request* >;
//#endif
    
    class MANTIS_API acq_request_db
    {
        public:
            acq_request_db( config_manager* a_conf_mgr, condition* a_condition, const std::string& a_exe_name = "unknown" );
            virtual ~acq_request_db();

            //***************
            // DB Commands
            //***************

            bool empty();

            acq_request* get_acq_request( boost::uuids::uuid a_id );
            const acq_request* get_acq_request( boost::uuids::uuid a_id ) const;

            boost::uuids::uuid add( acq_request* a_acq_request ); /// adds acq_request to the database (but not the queue); returns the assigned acq_request ID number
            acq_request* remove( boost::uuids::uuid a_id ); /// removes acq_request with id a_id, and returns the pointer to it

            void flush(); /// remove completed & failed acq_requests; removed acq_requests are deleted
            void clear(); /// remove all acq_requests; acq_requests are deleted

            //******************
            // Queue Commands
            //******************

            bool queue_empty();
            size_t queue_size();

            boost::uuids::uuid enqueue( acq_request* a_acq_request ); /// adds acq_request to the queue and database; returns the assigned acq_request ID number
            bool cancel( boost::uuids::uuid a_id ); /// removes acq_request with id a_id from the queue
            acq_request* pop();

            void clear_queue(); /// remove all requests in the queue; removed acq_requests are deleted

            //********************
            // Request handlers
            //********************

            bool handle_new_acq_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );

            bool handle_get_acq_status_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );
            bool handle_queue_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg  );
            bool handle_queue_size_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg  );

            bool handle_cancel_acq_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg  );
            bool handle_clear_queue_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg  );

        private:
            typedef std::map< boost::uuids::uuid, acq_request* > acq_request_db_data;
            typedef std::list< acq_request* > acq_request_queue;

            mutable mutex f_mutex;
            acq_request_queue f_acq_request_queue;
            acq_request_db_data f_acq_request_db;

            condition* f_queue_condition;

            config_manager* f_config_mgr;

            std::string f_exe_name;
    };

}

#endif /* MT_ACQ_REQUEST_DB_HH_ */
