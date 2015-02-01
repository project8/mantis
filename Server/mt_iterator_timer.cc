/*
 * mt_iterator_timer.cc
 *
 *  Created on: Mar 27, 2014
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_iterator_timer.hh"

#include "mt_logger.hh"

#include <sstream>

using std::list;

namespace mantis
{
    MTLOGGER( mtlog, "iterator_timer" );

    iterator_timer::iterator_timer() :
            f_it_name( "default " ),
            f_ignore_incr( false ),
            f_ignore_decr( false ),
            f_increment_counter( 0 ),
            f_events()
    {
    }

    iterator_timer::~iterator_timer()
    {
        list< event >::const_iterator t_list_it = f_events.begin();
        event t_prev_event;
        timespec t_temp_duration;
        double t_time_thread_blocked_by_block = 0;
        double t_time_waiting_after_fail = 0;
        double t_time_working = 0;
        double t_time_misc = 0;
        do
        {
            t_prev_event = *t_list_it;
            ++t_list_it;
            if( t_prev_event.f_type == k_incr_begin )
            {
                // then the current event type must be incr_locked, after possibly blocking the thread for some time
                if( t_list_it == f_events.end() )
                {
                    MTWARN( mtlog, "event list ended after increment-begin" );
                }
                else if( (*t_list_it).f_type == k_incr_locked )
                {
                    time_diff( t_prev_event.f_time, (*t_list_it).f_time, &t_temp_duration );
                    t_time_thread_blocked_by_block += time_to_sec( t_temp_duration );
                    //std::cout << "blocked by block: " << t_prev_event.f_time.tv_sec << " + " << t_prev_event.f_time.tv_nsec << " --- "
                    //        << (*t_list_it).f_time.tv_sec << " + " << (*t_list_it).f_time.tv_nsec << " --- "
                    //        << t_temp_duration.tv_sec << " + " << t_temp_duration.tv_nsec << " --- "
                    //        << time_to_sec( t_temp_duration ) << std::endl;
                }
                else
                {
                    MTERROR( mtlog, "event type <" << (*t_list_it).f_type << "> following increment-begin!" );
                }
            }
            else if( t_prev_event.f_type == k_incr_try_begin )
            {
                // then the current event type must be incr_locked or incr_try_fail
                if( t_list_it == f_events.end() )
                {
                    MTWARN( mtlog, "event list ended after increment-try-begin" );
                }
                else if( (*t_list_it).f_type == k_incr_locked || (*t_list_it).f_type == k_incr_try_fail )
                {
                    time_diff( t_prev_event.f_time, (*t_list_it).f_time, &t_temp_duration );
                    t_time_misc += time_to_sec( t_temp_duration );
                    //std::cout << "misc: " << t_prev_event.f_time.tv_sec << " + " << t_prev_event.f_time.tv_nsec << " --- "
                    //        << (*t_list_it).f_time.tv_sec << " + " << (*t_list_it).f_time.tv_nsec << " --- "
                    //        << t_temp_duration.tv_sec << " + " << t_temp_duration.tv_nsec << " --- "
                    //        << time_to_sec( t_temp_duration ) << std::endl;
                }
                else
                {
                    MTERROR( mtlog, "event type <" << (*t_list_it).f_type << "> following increment-try-begin!" );
                }
            }
            else if( t_prev_event.f_type == k_incr_locked )
            {
                // then the current event should be incr_begin, incr_try_begin or incr_other
                if( t_list_it == f_events.end() )
                {
                    MTDEBUG( mtlog, "event list ended after increment-locked" );
                }
                else if( (*t_list_it).f_type == k_incr_begin || (*t_list_it).f_type == k_incr_try_begin || (*t_list_it).f_type == k_other )
                {
                    time_diff( t_prev_event.f_time, (*t_list_it).f_time, &t_temp_duration );
                    t_time_working += time_to_sec( t_temp_duration );
                    //std::cout << "working: " << t_prev_event.f_time.tv_sec << " + " << t_prev_event.f_time.tv_nsec << " --- "
                    //        << (*t_list_it).f_time.tv_sec << " + " << (*t_list_it).f_time.tv_nsec << " --- "
                    //        << t_temp_duration.tv_sec << " + " << t_temp_duration.tv_nsec << " --- "
                    //        << time_to_sec( t_temp_duration ) << std::endl;
                }
                else
                {
                    MTWARN( mtlog, "event type <" << (*t_list_it).f_type << "> following increment-locked!");
                }
            }
            else if( t_prev_event.f_type == k_incr_try_fail )
            {
                // then it's expected that the thread owner will try again, producing either an incr_begin, an incr_try_begin
                // incr_other would also be acceptable
                if( t_list_it == f_events.end() )
                {
                    MTDEBUG( mtlog, "event list ended after increment-try-fail" );
                }
                else if( (*t_list_it).f_type == k_incr_begin || (*t_list_it).f_type == k_incr_try_begin || (*t_list_it).f_type == k_other )
                {
                    time_diff( t_prev_event.f_time, (*t_list_it).f_time, &t_temp_duration );
                    t_time_waiting_after_fail += time_to_sec( t_temp_duration );
                    //std::cout << "waiting after fail: " << t_prev_event.f_time.tv_sec << " + " << t_prev_event.f_time.tv_nsec << " --- "
                    //        << (*t_list_it).f_time.tv_sec << " + " << (*t_list_it).f_time.tv_nsec << " --- "
                    //        << t_temp_duration.tv_sec << " + " << t_temp_duration.tv_nsec << " --- "
                    //        << time_to_sec( t_temp_duration ) << std::endl;
                }
                else
                {
                    MTWARN( mtlog, "event type <" << (*t_list_it).f_type << "> following increment-fail!");
                }
            }
            else
            {
                // ignore time period if the previous event type is 'other'
            }
        } while( t_list_it != f_events.end() );
        std::stringstream t_timer_report;
        t_timer_report << "iterator timer report for iterator <" << f_it_name << ">\n";
        t_timer_report << "-----------------------------------------------------------------------------------------------\n";
        t_timer_report << " * iterator advanced " << f_increment_counter << " times\n";
        t_timer_report << " * time spent working: " << t_time_working << " s\n";
        t_timer_report << " * time waiting for block mutexes: " << t_time_thread_blocked_by_block << " s\n";
        t_timer_report << " * time waiting after increment fail: " << t_time_waiting_after_fail << " s\n";
        t_timer_report << " * other time: " << t_time_misc << " s\n";
        MTINFO( mtlog, t_timer_report.str() );
    }

    void iterator_timer::set_name( const std::string& a_name )
    {
        f_it_name = a_name;
        return;
    }

} /* namespace mantis */
