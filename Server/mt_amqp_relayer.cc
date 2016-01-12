#define MANTIS_API_EXPORTS

#include "mt_amqp_relayer.hh"

#include "param.hh"

namespace mantis
{
    using scarab::param_node;

    amqp_relayer::amqp_relayer() :
            callable(),
            relayer()
    {
    }

    amqp_relayer::~amqp_relayer()
    {
    }

    bool amqp_relayer::initialize( const param_node* a_amqp_config )
    {
        request_exchange() = a_amqp_config->get_value( "exchange", request_exchange() );
        alert_exchange() = a_amqp_config->get_value( "alert-exchange", alert_exchange() );
        info_exchange() = a_amqp_config->get_value( "info-exchange", info_exchange() );

        return true;
    }

    void amqp_relayer::execute()
    {
        execute_relayer();
        return;
    }

}
