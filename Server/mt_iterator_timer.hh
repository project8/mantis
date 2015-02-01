/*
 * mt_iterator_timer.hh
 *
 *  Created on: Mar 27, 2014
 *      Author: nsoblath
 */

#ifndef MT_ITERATOR_TIMER_HH_
#define MT_ITERATOR_TIMER_HH_

//#include "mt_singleton.hh"

#include "mt_logger.hh"

#include "thorax.hh"

#include <iostream>
#include <list>
#include <string>

namespace mantis
{
    MTLOGGER( mtlog_it, "iterator_timer" );

    class MANTIS_API iterator_timer
    {
        public:
            enum event_type
            {
                k_incr_begin,
                k_incr_try_begin,
                k_incr_locked,
                k_incr_try_fail,
                k_other
            };

            struct event
            {
                timespec f_time;
                event_type f_type;
                event( event_type a_type = k_other )
                {
                    f_type = a_type;
                }
            };

        public:
            iterator_timer();
            virtual ~iterator_timer();

            void set_name( const std::string& a_name );

            void set_ignore_incr();
            void unset_ignore_incr();

            void set_ignore_decr();
            void unset_ignore_decr();

            void increment_begin();
            void increment_try_begin();
            void increment_locked();
            void increment_try_fail();

            void other();

        private:
            std::string f_it_name;

            bool f_ignore_incr;
            bool f_ignore_decr;
            unsigned f_increment_counter;

            std::list< event > f_events;


    };

    inline void iterator_timer::set_ignore_incr()
    {
        f_ignore_incr = true;
        return;
    }

    inline void iterator_timer::unset_ignore_incr()
    {
        f_ignore_incr = false;
        return;
    }

    inline void iterator_timer::set_ignore_decr()
    {
        f_ignore_decr = true;
        return;
    }

    inline void iterator_timer::unset_ignore_decr()
    {
        f_ignore_decr = false;
        return;
    }

    inline void iterator_timer::increment_begin()
    {
        if( f_ignore_incr ) return;
        f_events.push_back( event( k_incr_begin ) );
        get_time_monotonic( &f_events.back().f_time );
        return;
    }

    inline void iterator_timer::increment_try_begin()
    {
        if( f_ignore_incr ) return;
        f_events.push_back( event( k_incr_try_begin ) );
        get_time_monotonic( &f_events.back().f_time );
        return;
    }

    inline void iterator_timer::increment_locked()
    {
        if( f_ignore_incr ) return;
        f_events.push_back( event( k_incr_locked ) );
        get_time_monotonic( &f_events.back().f_time );
        ++f_increment_counter;
        return;
    }

    inline void iterator_timer::increment_try_fail()
    {
        if( f_ignore_incr ) return;
        f_events.push_back( event( k_incr_try_fail ) );
        get_time_monotonic( &f_events.back().f_time );
        return;
    }

    inline void iterator_timer::other()
    {
        if( f_ignore_incr || f_ignore_decr ) return;
        f_events.push_back( event( k_other ) );
        get_time_monotonic( &f_events.back().f_time );
        return;
    }


#ifndef IT_TIMER_SET_IGNORE_INCR
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_SET_IGNORE_INCR( IT ) IT.timer().set_ignore_incr();
#else
#define IT_TIMER_SET_IGNORE_INCR( IT )
#endif
#endif

#ifndef IT_TIMER_UNSET_IGNORE_INCR
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_UNSET_IGNORE_INCR( IT ) IT.timer().unset_ignore_incr();
#else
#define IT_TIMER_UNSET_IGNORE_INCR( IT )
#endif
#endif

#ifndef IT_TIMER_SET_IGNORE_DECR
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_SET_IGNORE_DECR( IT ) IT.timer().set_ignore_decr();
#else
#define IT_TIMER_SET_IGNORE_DECR( IT )
#endif
#endif

#ifndef IT_TIMER_UNSET_IGNORE_DECR
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_UNSET_IGNORE_DECR( IT ) IT.timer().unset_ignore_decr();
#else
#define IT_TIMER_UNSET_IGNORE_DECR( IT )
#endif
#endif

#ifndef IT_TIMER_DECLARE
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_DECLARE \
    public: \
        iterator_timer& timer() {return f_timer;} \
    protected: \
        iterator_timer f_timer;
#else
#define IT_TIMER_DECLARE
#endif
#endif

#ifndef IT_TIMER_INITIALIZE
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_INITIALIZE \
{ \
    MTWARN( mtlog_it, "iterator timer initialized: " << name() ) \
    f_timer.set_name( name() ); \
}
#else
#define IT_TIMER_INITIALIZE
#endif
#endif

#ifndef IT_TIMER_INCR_BEGIN
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_INCR_BEGIN \
{ \
    f_timer.increment_begin(); \
}
#else
#define IT_TIMER_INCR_BEGIN
#endif
#endif

#ifndef IT_TIMER_INCR_TRY_BEGIN
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_INCR_TRY_BEGIN \
{ \
    f_timer.increment_try_begin(); \
}
#else
#define IT_TIMER_INCR_TRY_BEGIN
#endif
#endif

#ifndef IT_TIMER_INCR_LOCKED
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_INCR_LOCKED \
{ \
    f_timer.increment_locked(); \
    f_buffer->print_states(); \
}
#else
#define IT_TIMER_INCR_LOCKED
#endif
#endif

#ifndef IT_TIMER_INCR_TRY_FAIL
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_INCR_TRY_FAIL \
{ \
    f_timer.increment_try_fail(); \
}
#else
#define IT_TIMER_INCR_TRY_FAIL
#endif
#endif

#ifndef IT_TIMER_OTHER
#ifdef ENABLE_ITERATOR_TIMING
#define IT_TIMER_OTHER \
{ \
    f_timer.other(); \
}
#else
#define IT_TIMER_OTHER
#endif
#endif

} /* namespace mantis */

#endif /* MT_ITERATOR_TIMER_HH_ */
