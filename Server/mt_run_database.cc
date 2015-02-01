#define MANTIS_API_EXPORTS

#include "mt_run_database.hh"

#include "mt_logger.hh"
#include "mt_run_description.hh"


namespace mantis
{
    MTLOGGER( mtlog, "run_database" );

    run_database::run_database() :
            f_mutex(),
            f_run_queue(),
            f_run_db(),
            f_id_count( 0 )
    {
    }

    run_database::~run_database()
    {
        clear();
        return;
    }

    bool run_database::empty()
    {
        bool t_empty;
        f_mutex.lock();
        t_empty = f_run_db.empty();
        f_mutex.unlock();
        return t_empty;
    }

    run_description* run_database::get_run( unsigned a_id )
    {
        run_description* t_desc = NULL;
        f_mutex.lock();
        run_db::iterator t_run_it;
        t_run_it = f_run_db.find( a_id );
        if( t_run_it != f_run_db.end()) t_desc = t_run_it->second;
        f_mutex.unlock();
        return t_desc;
    }

    const run_description* run_database::get_run( unsigned a_id ) const
    {
        const run_description* t_desc = NULL;
        f_mutex.lock();
        run_db::const_iterator t_run_it;
        t_run_it = f_run_db.find( a_id );
        if( t_run_it != f_run_db.end()) t_desc = t_run_it->second;
        f_mutex.unlock();
        return t_desc;
    }

    unsigned run_database::add( run_description* a_run )
    {
        unsigned t_id = 0;
        f_mutex.lock();
        t_id = f_id_count++;
        f_run_db[ t_id ] = a_run;
        f_mutex.unlock();
        return t_id;
    }

    run_description* run_database::remove( unsigned a_id )
    {
        run_description* t_desc = NULL;
        f_mutex.lock();
        run_db::iterator t_run_it;
        t_run_it = f_run_db.find( a_id );
        if( t_run_it != f_run_db.end())
        {
            t_desc = t_run_it->second;
            f_run_db.erase( t_run_it );
        }
        f_mutex.unlock();
        return t_desc;
    }


    void run_database::flush()
    {
        f_mutex.lock();
        run_db::iterator t_it_counter = f_run_db.begin();
        run_db::iterator t_it_deletable;
        while( t_it_counter != f_run_db.end() )
        {
            t_it_deletable = t_it_counter++;
            if( t_it_deletable->second->get_status() > run_description::running )
            {
                delete t_it_deletable->second;
            }
        }
        f_mutex.unlock();
        return;
    }

    void run_database::clear()
    {
        f_mutex.lock();
        for( run_db::iterator t_run_it = f_run_db.begin(); t_run_it != f_run_db.end(); ++t_run_it )
        {
            delete t_run_it->second;
        }
        f_run_db.clear();
        f_run_queue.clear();
        f_mutex.unlock();
        return;
    }

    bool run_database::queue_empty()
    {
        bool t_empty;
        f_mutex.lock();
        t_empty = f_run_queue.empty();
        f_mutex.unlock();
        return t_empty;
    }

    unsigned run_database::enqueue( run_description* a_run )
    {
        unsigned t_id = 0;
        f_mutex.lock();
        t_id = f_id_count++;
        f_run_db[ t_id ] = a_run;
        f_run_queue.push_back( a_run );
        a_run->set_status( run_description::waiting );
        f_mutex.unlock();
        return t_id;
    }

    run_description* run_database::pop()
    {
        run_description* t_run = NULL;
        f_mutex.lock();
        if( ! f_run_queue.empty() )
        {
            t_run = f_run_queue.front();
            f_run_queue.pop_front();
        }
        f_mutex.unlock();
        return t_run;
    }

    void run_database::clear_queue()
    {
        f_mutex.lock();
        while( ! f_run_queue.empty() )
        {
            run_db::iterator t_run_it = f_run_db.find( f_run_queue.front()->get_id() );
            delete t_run_it->second;
            f_run_db.erase( t_run_it );
            f_run_queue.pop_front();
        }
        f_mutex.unlock();
        return;
    }

} /* namespace mantis */
