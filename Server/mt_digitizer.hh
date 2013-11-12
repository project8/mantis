#ifndef MT_DIGITIZER_HH_
#define MT_DIGITIZER_HH_

#include "mt_callable.hh"

namespace mantis
{
    class buffer;
    class condition;
    class request;
    class response;

    class digitizer :
        public callable
    {
        public:
            digitizer();
            virtual ~digitizer();

            virtual void allocate( buffer* a_buffer, condition* a_condition );
            virtual void initialize( request* a_request ) = 0;
            virtual void finalize( response* a_response ) = 0;

        private:
            buffer* f_buffer;
            condition* f_condition;

    };

}

#endif
