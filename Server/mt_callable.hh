#ifndef MT_CALLABLE_HH_
#define MT_CALLABLE_HH_

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
