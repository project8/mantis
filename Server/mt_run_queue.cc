#include "mt_request_queue.hh"

namespace mantis
{

    request_queue::request_queue() :
            f_mutex(),
            f_runs()
    {
    }
    request_queue::~request_queue()
    {
        f_mutex.lock();
        std::list< run_context_dist* >::iterator t_it;
        for( t_it = f_runs.begin(); t_it != f_runs.end(); t_it++ )
        {
            delete *t_it;
        }
        f_mutex.unlock();
        return;
    }

    bool request_queue::empty()
    {
        bool t_empty;
        f_mutex.lock();
        t_empty = f_runs.empty();
        f_mutex.unlock();
        return t_empty;
    }

    void request_queue::to_front( run_context_dist* a_run )
    {
        f_mutex.lock();
        f_runs.push_front( a_run );
        f_mutex.unlock();
        return;
    }
    run_context_dist* request_queue::from_front()
    {
        run_context_dist* t_front = NULL;
        f_mutex.lock();
        if( f_runs.empty() == false )
        {
            t_front = f_runs.front();
            f_runs.pop_front();
        }
        f_mutex.unlock();
        return t_front;
    }

    void request_queue::to_back( run_context_dist* a_run )
    {
        f_mutex.lock();
        f_runs.push_back( a_run );
        f_mutex.unlock();
        return;
    }
    run_context_dist* request_queue::from_back()
    {
        run_context_dist* t_back = NULL;
        f_mutex.lock();
        if( f_runs.empty() == false )
        {
            t_back = f_runs.back();
            f_runs.pop_back();
        }
        f_mutex.unlock();
        return t_back;
    }

    void request_queue::execute()
    {
        while( true )
        {
            sleep( 1 );

            f_mutex.lock();
            std::list< run_context_dist* >::iterator t_it;
            for( t_it = f_runs.begin(); t_it != f_runs.end(); t_it++ )
            {
                (*t_it)->push_status();
            }
            f_mutex.unlock();

        }
    }

}
