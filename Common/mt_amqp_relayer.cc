#define MANTIS_API_EXPORTS

#include "mt_amqp_relayer.hh"

#include "mt_broker.hh"
#include "mt_exception.hh"

#include "logger.hh"
#include "param.hh"

//#include <boost/uuid/uuid_io.hpp>

using std::string;

using scarab::param_node;

namespace mantis
{
    LOGGER( mtlog, "amqp_relayer" );

    amqp_relayer::amqp_relayer( const broker* a_broker ) :
            f_channel( a_broker->open_channel() ),
            f_request_exchange( "requests" ),
            f_alert_exchange( "alerts" ),
            //f_uuid_gen(),
            f_queue(),
            f_canceled( false )
    {
    }

    amqp_relayer::~amqp_relayer()
    {
    }

    bool amqp_relayer::initialize( const param_node* a_amqp_config )
    {
        if( ! f_channel )
        {
            ERROR( mtlog, "AMQP channel is not open" );
            return false;
        }

        try
        {
            f_request_exchange = a_amqp_config->get_value( "exchange", f_request_exchange );
            f_channel->DeclareExchange( f_request_exchange, AmqpClient::Channel::EXCHANGE_TYPE_TOPIC, true );
        }
        catch( std::exception& e )
        {
            ERROR( mtlog, "Unable to declare exchange <" << f_request_exchange << ">; aborting.\n(" << e.what() << ")" );
            return false;
        }

        try
        {
            f_alert_exchange = a_amqp_config->get_value( "alert-exchange", f_alert_exchange );
            f_channel->DeclareExchange( f_alert_exchange, AmqpClient::Channel::EXCHANGE_TYPE_TOPIC, true );
        }
        catch( std::exception& e )
        {
            ERROR( mtlog, "Unable to declare exchange <" << f_alert_exchange << ">; aborting.\n(" << e.what() << ")" );
            return false;
        }

        return true;
    }

    void amqp_relayer::execute()
    {
        DEBUG( mtlog, "AMQP relayer starting" );
        while( ! f_canceled.load() )
        {
            message* t_message = NULL;
            bool t_have_message = f_queue.timed_wait_and_pop( t_message ); // blocking call for next message to send; timed so that cancellation can be rechecked
            if( ! t_have_message ) continue;

            if( t_message->get_message_type() == T_REQUEST || t_message->get_message_type() == T_REPLY )
            {
                relay_request( t_message );
            }
            else if( t_message->get_message_type() == T_ALERT )
            {
                relay_alert( t_message );
            }
            else
            {
                WARN( mtlog, "Cannot currently handle message type <" << t_message->get_message_type() << ">; will be ignored");
            }

            delete t_message;
        }

        DEBUG( mtlog, "Exiting the AMQP relayer" );

        return;
    }


    /*
    Asyncronous cancellation:
    */
    void amqp_relayer::cancel()
    {
        DEBUG( mtlog, "Canceling amqp_relayer" );
        f_canceled.store( true );
        f_queue.interrupt();

        //close_connection();

        return;
    }

    bool amqp_relayer::relay_request( message* a_message )
    {
        DEBUG( mtlog, "Relaying message to the requests exchange" );

        string t_consumer_tag;
        return a_message->do_publish( f_channel, f_request_exchange, t_consumer_tag );
    }

    bool amqp_relayer::relay_alert( message* a_message )
    {
        DEBUG( mtlog, "Relaying message to the requests exchange" );

        string t_consumer_tag;
        return a_message->do_publish( f_channel, f_alert_exchange, t_consumer_tag );
    }

    /*
    bool amqp_relayer::encode_message( const message_data* a_data, std::string& a_message ) const
    {
        switch( a_data->f_encoding )
        {
            case k_json:
                if( ! param_output_json::write_string( *(a_data->f_message), a_message, param_output_json::k_compact ) )
                {
                    ERROR( mtlog, "Could not convert message to string" );
                    return false;
                }
                return true;
                break;
            default:
                ERROR( mtlog, "Invalid encoding: " << a_data->f_encoding );
                return false;
                break;
        }
        // should not get here
        return false;
    }
    */

    bool amqp_relayer::send_message( message* a_message )
    {
        DEBUG( mtlog, "Received send-message request addressed to <" << a_message->get_routing_key() << ">" );
        f_queue.push( a_message );
        return true;
    }

    /*
    bool amqp_relayer::send_alert( const param_node* a_message, std::string a_routing_key, encoding a_encoding )
    {
        message_data* t_data = new message_data();
        t_data->f_message = new param_node( *a_message );
        t_data->f_message_type = k_alert;
        t_data->f_routing_key = a_routing_key;
        t_data->f_encoding = a_encoding;
        DEBUG( mtlog, "Received send-alert request addressed to <" << a_routing_key << ">" );
        f_queue.push( t_data );
        return true;
    }

    string amqp_relayer::interpret_encoding( amqp_relayer::encoding a_encoding) const
    {
        switch( a_encoding )
        {
            case k_json:
                return std::string( "application/json" );
                break;
            default:
                return std::string( "Unknown" );
        }
    }
    */


}
