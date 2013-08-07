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
    }

    void queue::to_front( run* a_run )
    {
        f_mutex.lock();
        f_runs.push_front( a_run );
        f_mutex.unlock();
        return;
    }
    run* queue::from_front()
    {
        run* t_run = NULL;
        f_mutex.lock();
        t_run = f_runs.front();
        f_runs.pop_front();
        f_mutex.unlock();
        return t_run;
    }

    void queue::to_back( run* a_run )
    {
        f_mutex.lock();
        f_runs.push_back( a_run );
        f_mutex.unlock();
        return;
    }
    run* queue::from_back()
    {
        run* t_run = NULL;
        f_mutex.lock();
        t_run = f_runs.back();
        f_runs.pop_back();
        f_mutex.unlock();
        return t_run;
    }

    void queue::for_each( void (run::* a_command)() )
    {
        f_mutex.lock();
        std::list< run* >::iterator t_it;
        for( t_it = f_runs.begin(); t_it != f_runs.end(); t_it++ )
        {
            ((*t_it)->*(a_command))();
        }
        f_mutex.unlock();
        return;
    }

}
