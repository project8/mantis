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
        std::list< run* >::iterator t_it;
        for( t_it = f_runs.begin(); t_it != f_runs.end(); t_it++ )
        {
            delete *t_it;
        }
        f_mutex.unlock();
        return;
    }

    bool queue::is_empty()
    {
        bool t_empty = false;
        f_mutex.lock();
        t_empty = f_runs.empty();
        f_mutex.unlock();
        return t_empty;
    }

    void queue::to_front( run* a_run )
    {
        f_mutex.lock();
        f_runs.push_front( a_run );
        f_mutex.unlock();
        return;
    }

    void queue::to_back( run* a_run )
    {
        f_mutex.lock();
        f_runs.push_back( a_run );
        f_mutex.unlock();
        return;
    }

    void queue::push_response()
    {
        f_mutex.lock();
        std::list< run* >::iterator t_it;
        for( t_it = f_runs.begin(); t_it != f_runs.end(); t_it++ )
        {
            (*t_it)->push_response();
        }
        f_mutex.unlock();
        return;
    }

}
