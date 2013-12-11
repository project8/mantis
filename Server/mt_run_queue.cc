#include "mt_run_queue.hh"

#include "mt_run_context_dist.hh"

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

namespace mantis
{

    run_queue::run_queue() :
            f_mutex(),
            f_runs()
    {
    }
    run_queue::~run_queue()
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

    bool run_queue::empty()
    {
        bool t_empty;
        f_mutex.lock();
        t_empty = f_runs.empty();
        f_mutex.unlock();
        return t_empty;
    }

    void run_queue::to_front( run_context_dist* a_run )
    {
        f_mutex.lock();
        f_runs.push_front( a_run );
        f_mutex.unlock();
        return;
    }
    run_context_dist* run_queue::from_front()
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

    void run_queue::to_back( run_context_dist* a_run )
    {
        f_mutex.lock();
        f_runs.push_back( a_run );
        f_mutex.unlock();
        return;
    }
    run_context_dist* run_queue::from_back()
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

    void run_queue::execute()
    {
        while( true )
        {
            pthread_testcancel();

            // timing delay for updating status
            sleep( 1 );

            f_mutex.lock();
            std::list< run_context_dist* >::iterator t_it = f_runs.begin();
            while( t_it != f_runs.end() )
            {
                try
                {
                    if( (*t_it)->push_status() )
                    {
                        // we can still communicate with the client!
                        // continue with the next run in the queue
                        ++t_it;
                        continue;
                    }
                }
                catch( closed_connection& cc )
                {
                    cout << "[request_receiver] connection closed; detected in <" << cc.what() << ">" << endl;
                }
                // push was unsuccessful, indicating that the communication with the client is broken for some reason
                cerr << "[run_queue] communication with client failed; aborting run request" << endl;
                t_it = f_runs.erase( t_it );
            }
            f_mutex.unlock();

        }
        return;
    }

    void run_queue::cancel()
    {
        //std::cout << "CANCELLING RUN QUEUE" << std::endl;
        f_mutex.lock();
        while( ! f_runs.empty() )
        {
            run_context_dist* t_run_context = f_runs.front();
            status* t_status = t_run_context->lock_status_out();
            t_status->set_state( status_state_t_revoked );
            t_run_context->push_status_no_mutex();
            t_run_context->unlock_outbound();
            delete t_run_context->get_connection();
            delete t_run_context;
            f_runs.pop_front();
        }
        f_mutex.unlock();
        return;
    }

}
