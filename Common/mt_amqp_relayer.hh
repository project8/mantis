#ifndef MT_AMQP_RELAYER_HH_
#define MT_AMQP_RELAYER_HH_

#include "mt_callable.hh"
#include "relayer.hh"

namespace scarab
{
    class param_node;
}

namespace mantis
{
    class MANTIS_API amqp_relayer : public callable, public dripline::relayer
    {
        public:
            amqp_relayer();
            virtual ~amqp_relayer();

            bool initialize( const scarab::param_node* a_amqp_config );

            void execute();

            void cancel(); /// cancels the relayer entirely

    };

}

#endif
