#define MANTIS_API_EXPORTS

#include "mt_acq_request_db.hh"

#include "mt_logger.hh"
#include "mt_acq_request.hh"


namespace mantis
{
    MTLOGGER( mtlog, "acq_request_db" );

    acq_request_db::acq_request_db() :
            f_mutex(),
            f_acq_request_queue(),
            f_acq_request_db(),
            f_id_count( 0 )
    {
    }

    acq_request_db::~acq_request_db()
    {
        clear();
        return;
    }

    bool acq_request_db::empty()
    {
        bool t_empty;
        f_mutex.lock();
        t_empty = f_acq_request_db.empty();
        f_mutex.unlock();
        return t_empty;
    }

    acq_request* acq_request_db::get_acq_request( unsigned a_id )
    {
        acq_request* t_desc = NULL;
        f_mutex.lock();
        acq_request_db_data::iterator t_acq_request_it;
        t_acq_request_it = f_acq_request_db.find( a_id );
        if( t_acq_request_it != f_acq_request_db.end()) t_desc = t_acq_request_it->second;
        f_mutex.unlock();
        return t_desc;
    }

    const acq_request* acq_request_db::get_acq_request( unsigned a_id ) const
    {
        const acq_request* t_desc = NULL;
        f_mutex.lock();
        acq_request_db_data::const_iterator t_acq_request_it;
        t_acq_request_it = f_acq_request_db.find( a_id );
        if( t_acq_request_it != f_acq_request_db.end()) t_desc = t_acq_request_it->second;
        f_mutex.unlock();
        return t_desc;
    }

    unsigned acq_request_db::add( acq_request* a_acq_request )
    {
        unsigned t_id = 0;
        f_mutex.lock();
        t_id = f_id_count++;
        f_acq_request_db[ t_id ] = a_acq_request;
        f_mutex.unlock();
        return t_id;
    }

    acq_request* acq_request_db::remove( unsigned a_id )
    {
        acq_request* t_desc = NULL;
        f_mutex.lock();
        acq_request_db_data::iterator t_acq_request_it;
        t_acq_request_it = f_acq_request_db.find( a_id );
        if( t_acq_request_it != f_acq_request_db.end())
        {
            t_desc = t_acq_request_it->second;
            f_acq_request_db.erase( t_acq_request_it );
        }
        f_mutex.unlock();
        return t_desc;
    }


    void acq_request_db::flush()
    {
        f_mutex.lock();
        acq_request_db_data::iterator t_it_counter = f_acq_request_db.begin();
        acq_request_db_data::iterator t_it_deletable;
        while( t_it_counter != f_acq_request_db.end() )
        {
            t_it_deletable = t_it_counter++;
            if( t_it_deletable->second->get_status() > acq_request::running )
            {
                delete t_it_deletable->second;
            }
        }
        f_mutex.unlock();
        return;
    }

    void acq_request_db::clear()
    {
        f_mutex.lock();
        for( acq_request_db_data::iterator t_acq_request_it = f_acq_request_db.begin(); t_acq_request_it != f_acq_request_db.end(); ++t_acq_request_it )
        {
            delete t_acq_request_it->second;
        }
        f_acq_request_db.clear();
        f_acq_request_queue.clear();
        f_mutex.unlock();
        return;
    }

    bool acq_request_db::queue_empty()
    {
        bool t_empty;
        f_mutex.lock();
        t_empty = f_acq_request_queue.empty();
        f_mutex.unlock();
        return t_empty;
    }

    unsigned acq_request_db::enqueue( acq_request* a_acq_request )
    {
        unsigned t_id = 0;
        f_mutex.lock();
        t_id = f_id_count++;
        f_acq_request_db[ t_id ] = a_acq_request;
        f_acq_request_queue.push_back( a_acq_request );
        a_acq_request->set_status( acq_request::waiting );
        f_mutex.unlock();
        return t_id;
    }

    acq_request* acq_request_db::pop()
    {
        acq_request* t_acq_request = NULL;
        f_mutex.lock();
        if( ! f_acq_request_queue.empty() )
        {
            t_acq_request = f_acq_request_queue.front();
            f_acq_request_queue.pop_front();
        }
        f_mutex.unlock();
        return t_acq_request;
    }

    void acq_request_db::clear_queue()
    {
        f_mutex.lock();
        while( ! f_acq_request_queue.empty() )
        {
            acq_request_db_data::iterator t_acq_request_it = f_acq_request_db.find( f_acq_request_queue.front()->get_id() );
            delete t_acq_request_it->second;
            f_acq_request_db.erase( t_acq_request_it );
            f_acq_request_queue.pop_front();
        }
        f_mutex.unlock();
        return;
    }

} /* namespace mantis */
