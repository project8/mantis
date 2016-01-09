/*
 * mt_concurrent_queue.hh
 *
 *  Created on: July 8, 2015
 *      Author: nsoblath
 *
 *      FIFO Queue
 *
 *      Based almost exactly on the class concurrent_queue from:
 *      http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
 *      Post author: Anthony Williams
 *      Copyright  2005-2013 Just Software Solutions Ltd. All rights reserved.
 */

#ifndef MT_CONCURRENT_QUEUE_HH_
#define MT_CONCURRENT_QUEUE_HH_

#include "logger.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#include <deque>

namespace mantis
{
    LOGGER( mtlog_cq, "concurrent_queue" );

    template< class XDataType >
    class concurrent_queue
    {
        public:
            typedef std::deque< XDataType > queue;

            struct queue_not_empty
            {
                queue& f_queue;
                queue_not_empty( queue& a_queue ) :
                    f_queue( a_queue )
                {}
                bool operator()() const
                {
                    return ! f_queue.empty();
                }
            };

            typedef boost::unique_lock< boost::mutex > scoped_lock;

        public:
            concurrent_queue() :
                f_queue(),
                f_interrupt( false ),
                f_timeout( boost::posix_time::milliseconds( 1000 ) ),
                f_mutex(),
                f_condition_var()
            {
            }

            virtual ~concurrent_queue()
            {
                f_queue.clear();
            }

        private:
            queue f_queue;
            bool f_interrupt;

            boost::posix_time::time_duration f_timeout; /// Timeout duration in milliseconds

            mutable boost::mutex f_mutex;
            boost::condition_variable f_condition_var;

        public:
            void push( XDataType const& a_data )
            {
                DEBUG( mtlog_cq, "Attempting to push to queue" );
                scoped_lock lock( f_mutex );
                DEBUG( mtlog_cq, "Pushing to concurrent queue; size: " << f_queue.size() );
                f_queue.push_back( a_data );
                lock.unlock();
                f_condition_var.notify_one();
                return;
            }

            bool empty() const
            {
                scoped_lock lock( f_mutex );
                return f_queue.empty();
            }

            bool size() const
            {
                scoped_lock lock( f_mutex );
                return f_queue.size();
            }

            bool try_pop( XDataType& a_popped_value )
            {
                scoped_lock lock( f_mutex );
                f_interrupt = false;
                if( f_queue.empty() )
                {
                    return false;
                }

                a_popped_value = f_queue.front();
                f_queue.pop_front();
                return true;
            }

            bool wait_and_pop( XDataType& a_popped_value )
            {
                scoped_lock lock( f_mutex );
                f_interrupt = false;
                f_condition_var.wait( lock, queue_not_empty( f_queue ) );
                if( f_interrupt )
                {
                    f_interrupt = false;
                    return false;
                }

                a_popped_value = f_queue.front();
                f_queue.pop_front();
                DEBUG( mtlog_cq, "Popping from concurrent queue; size: " << f_queue.size() );
                return true;
            }

            bool timed_wait_and_pop( XDataType& a_popped_value )
            {
                scoped_lock lock( f_mutex );
                f_interrupt = false;
                boost::system_time const waitUntil = boost::get_system_time() + f_timeout;
                if( ! f_condition_var.timed_wait( lock, waitUntil, queue_not_empty( f_queue ) ) )
                {
                    //DEBUG( mtlog_cq, "Queue wait has timed out" );
                    return false;
                }
                if( f_interrupt )
                {
                    f_interrupt = false;
                    return false;
                }

                a_popped_value = f_queue.front();
                f_queue.pop_front();
                DEBUG( mtlog_cq, "Popping from concurrent queue; size: " << f_queue.size() );
                return true;
            }

            void interrupt()
            {
                f_interrupt = true;
                f_condition_var.notify_one();
                return;
            }

            inline unsigned get_timeout() const
            {
                return f_timeout.total_milliseconds();
            }

            inline void set_timeout( unsigned a_duration )
            {
                f_timeout = boost::posix_time::milliseconds( a_duration );
                return;
            }
    };

} /* namespace mantis */

#endif /* MT_CONCURRENT_QUEUE_HH_ */
