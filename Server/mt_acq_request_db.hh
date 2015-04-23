#ifndef MT_ACQ_REQUEST_DB_HH_
#define MT_ACQ_REQUEST_DB_HH_

#include "mt_mutex.hh"

#include <list>
#include <map>

namespace mantis
{
    class acq_request;

//#ifdef _WIN32
//    MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::list< acq_request* >;
//    MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::map< unsigned, acq_request* >;
//#endif
    
    class MANTIS_API acq_request_db
    {
        public:
            acq_request_db();
            virtual ~acq_request_db();

            //***************
            // DB Commands
            //***************

            bool empty();

            acq_request* get_acq_request( unsigned a_id );
            const acq_request* get_acq_request( unsigned a_id ) const;

            unsigned add( acq_request* a_acq_request ); /// adds acq_request to the database (but not the queue); returns the assigned acq_request ID number
            acq_request* remove( unsigned a_id ); /// removes acq_request with id a_id, and returns the pointer to it

            void flush(); /// remove completed & failed acq_requests; removed acq_requests are deleted
            void clear(); /// remove all acq_requests; acq_requests are deleted

            //******************
            // Queue Commands
            //******************

            bool queue_empty();

            unsigned enqueue( acq_request* a_acq_request ); /// adds acq_request to the queue and database; returns the assigned acq_request ID number
            acq_request* pop();

            void clear_queue(); /// remove all requests in the queue; removed acq_requests are deleted

        private:
            typedef std::map< unsigned, acq_request* > acq_request_db_data;

            mutable mutex f_mutex;
            std::list< acq_request* > f_acq_request_queue;
            acq_request_db_data f_acq_request_db;

            unsigned f_id_count;
    };

}

#endif /* MT_ACQ_REQUEST_DB_HH_ */
