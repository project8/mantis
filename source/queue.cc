#include "queue.hh"

namespace mantis
{

    queue::queue() :
            f_mutex(),
            f_runs()
    {
    }
    queue::~queue()
    {
        f_mutex.lock();
        std::list< context* >::iterator t_it;
        for( t_it = f_runs.begin(); t_it != f_runs.end(); t_it++ )
        {
            delete *t_it;
        }
        f_mutex.unlock();
        return;
    }

    bool queue::empty()
    {
        bool t_empty;
        f_mutex.lock();
        t_empty = f_runs.empty();
        f_mutex.unlock();
        return t_empty;
    }

    void queue::to_front( context* a_run )
    {
        f_mutex.lock();
        f_runs.push_front( a_run );
        f_mutex.unlock();
        return;
    }
    context* queue::from_front()
    {
        context* t_front = NULL;
        f_mutex.lock();
        if( f_runs.empty() == false )
        {
            t_front = f_runs.front();
            f_runs.pop_front();
        }
        f_mutex.unlock();
        return t_front;
    }

    void queue::to_back( context* a_run )
    {
        f_mutex.lock();
        f_runs.push_back( a_run );
        f_mutex.unlock();
        return;
    }
    context* queue::from_back()
    {
        context* t_back = NULL;
        f_mutex.lock();
        if( f_runs.empty() == false )
        {
            t_back = f_runs.back();
            f_runs.pop_back();
        }
        f_mutex.unlock();
        return t_back;
    }

    void queue::execute()
    {
        while( true )
        {
            sleep( 1 );

            f_mutex.lock();
            std::list< context* >::iterator t_it;
            for( t_it = f_runs.begin(); t_it != f_runs.end(); t_it++ )
            {
                (*t_it)->push_status();
            }
            f_mutex.unlock();

        }
    }

}
