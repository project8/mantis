#ifndef CALLABLE_HH_
#define CALLABLE_HH_

namespace mantis
{

    class callable
    {
        public:
            callable();
            virtual ~callable();

            virtual void execute() = 0;
    };

}

#endif
