#ifndef MT_RECORD_RECEIVER_HH_
#define MT_RECORD_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_server.hh"
#include "mt_buffer.hh"
#include "mt_condition.hh"

namespace mantis
{

    class record_receiver :
        public callable
    {
        public:
            record_receiver( server* a_server, buffer* a_buffer, condition* a_condition );
            virtual ~record_receiver();

            void execute();

        private:
            server* f_server;
            buffer* f_buffer;
            condition* f_condition;
    };

}

#endif
