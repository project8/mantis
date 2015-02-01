#ifndef MT_RUN_DATABASE_HH_
#define MT_RUN_DATABASE_HH_

#include "mt_mutex.hh"

#include <list>
#include <map>

namespace mantis
{
    class run_description;

    class MANTIS_API run_database
    {
        public:
            run_database();
            virtual ~run_database();

            //***************
            // DB Commands
            //***************

            bool empty();

            run_description* get_run( unsigned a_id );
            const run_description* get_run( unsigned a_id ) const;

            unsigned add( run_description* a_run ); /// adds run to the database (but not the queue); returns the assigned run ID number
            run_description* remove( unsigned a_id ); /// removes run with id a_id, and returns the pointer to it

            void flush(); /// remove completed & failed runs; removed run descriptions are deleted
            void clear(); /// remove all runs; run descriptions are deleted

            //******************
            // Queue Commands
            //******************

            bool queue_empty();

            unsigned enqueue( run_description* a_run ); /// adds run to the queue and database; returns the assigned run ID number
            run_description* pop();

            void clear_queue(); /// remove all runs in the queue; removed run descriptions are deleted

        private:
            typedef std::map< unsigned, run_description* > run_db;

            mutable mutex f_mutex;
            std::list< run_description* > f_run_queue;
            run_db f_run_db;

            unsigned f_id_count;
    };

}

#endif /* MT_RUN_DATABASE_HH_ */
