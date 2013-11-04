#ifndef MT_DIGITIZER_HH_
#define MT_DIGITIZER_HH_

#include "mt_callable.hh"

namespace mantis
{
    class request;
    class response;

    class digitizer :
        public callable
    {
        public:
            digitizer();
            virtual ~digitizer();

            void initialize( request* a_request ) = 0;
            void finalize( response* a_response ) = 0;

    };

}

#endif
