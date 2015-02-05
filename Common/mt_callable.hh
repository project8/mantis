#ifndef MT_CALLABLE_HH_
#define MT_CALLABLE_HH_

#include "mt_constants.hh"

namespace mantis
{

    class MANTIS_API callable
    {
        public:
            callable();
            virtual ~callable();

            virtual void execute() = 0;

            virtual void cancel() = 0;
    };

}

#endif
