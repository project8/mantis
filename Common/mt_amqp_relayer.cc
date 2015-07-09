#define MANTIS_API_EXPORTS

#include "mt_amqp_relayer.hh"

#include "mt_broker.hh"
#include "mt_connection.hh"
#include "mt_exception.hh"
#include "mt_logger.hh"
#include "mt_param_json.hh"

#include <boost/uuid/uuid_io.hpp>

using std::string;

namespace mantis
{
    MTLOGGER( mtlog, "amqp_relayer" );

    amqp_relayer::amqp_relayer() :
            f_broker( broker::get_instance() ),
            f_request_exchange( "requests" ),
            f_alert_exchange( "alerts" ),
            f_uuid_gen(),
            f_queue(),
            f_canceled( false )
    {
    }

    amqp_relayer::~amqp_relayer()
    {
    }

    bool amqp_relayer::initialize( const param_node* a_amqp_config )
    {
        if( a_amqp_config == NULL ) return false;

        if( ! f_broker->is_connected() )
        {
            if(! f_broker->connect( a_amqp_config->get_value( "broker", "" ),
                    a_amqp_config->get_value< unsigned >( "broker-port", 0 ) ) )
            {
                MTERROR( mtlog, "Cannot create connection to the AMQP broker" );
                return false;
            }
        }
        else
        {
            if( f_broker->get_address() != a_amqp_config->get_value( "broker" ) ||
                    f_broker->get_port() != a_amqp_config->get_value< unsigned >( "broker-port" ) )
            {
                MTERROR( mtlog, "Already connected to a different AMQP broker: " << f_broker->get_address() << ":" << f_broker->get_port() );
                return false;
            }
        }

        try
        {
            f_request_exchange = a_amqp_config->get_value( "exchange", f_request_exchange );
            f_broker->get_connection().amqp()->DeclareExchange( f_request_exchange, AmqpClient::Channel::EXCHANGE_TYPE_TOPIC, true );
        }
        catch( std::exception& e )
        {
            f_broker->disconnect();
            MTERROR( mtlog, "Unable to declare exchange <" << f_request_exchange << ">; aborting.\n(" << e.what() << ")" );
            return false;
        }

        try
        {
            f_alert_exchange = a_amqp_config->get_value( "alert-exchange", f_alert_exchange );
            f_broker->get_connection().amqp()->DeclareExchange( f_alert_exchange, AmqpClient::Channel::EXCHANGE_TYPE_TOPIC, true );
        }
        catch( std::exception& e )
        {
            f_broker->disconnect();
            MTERROR( mtlog, "Unable to declare exchange <" << f_alert_exchange << ">; aborting.\n(" << e.what() << ")" );
            return false;
        }

        return true;
    }

    void amqp_relayer::execute()
    {
        MTDEBUG( mtlog, "AMQP relayer starting" );
        while( ! f_canceled.load() )
        {
            message_data* t_data = NULL;
            bool t_have_message = f_queue.timed_wait_and_pop( t_data ); // blocking call for next message to send; timed so that cancellation can be rechecked
            if( ! t_have_message ) continue;

            switch( t_data->f_message_type ) {
                case k_request:
                    relay_request( t_data );
                    break;
                case k_alert:
                    relay_alert( t_data );
                    break;
                default:
                    MTWARN( mtlog, "Unknown message type <" << t_data->f_message_type << ">; will be ignored");
                    break;
            }

            delete t_data;
        }

        MTDEBUG( mtlog, "Exiting the AMQP relayer" );

        return;
    }


    /*
    Asyncronous cancellation:
    */
    void amqp_relayer::cancel()
    {
        MTDEBUG( mtlog, "Canceling amqp_relayer" );
        f_canceled.store( true );

        //close_connection();

        return;
    }

    bool amqp_relayer::relay_request( message_data* a_data )
    {
        MTDEBUG( mtlog, "Relaying request" );
        string t_request_str;
        if( ! encode_message( a_data, t_request_str ) )
        {
            return false;
        }

        // strings for passing to the various do_[type]_request functions
        string t_reply_to = broker::get_instance()->get_connection().amqp()->DeclareQueue( "" );
        string t_consumer_tag = broker::get_instance()->get_connection().amqp()->BasicConsume( t_reply_to );
        MTDEBUG( mtlog, "Consumer tag for reply: " << t_consumer_tag );

        MTINFO( mtlog, "Sending request with routing key <" << a_data->f_routing_key << ">" );

        AmqpClient::BasicMessage::ptr_t t_message = AmqpClient::BasicMessage::Create( t_request_str );
        t_message->ContentEncoding( interpret_encoding( a_data->f_encoding ) );
        t_message->CorrelationId( boost::uuids::to_string( f_uuid_gen() ) );
        t_message->ReplyTo( t_reply_to );

        try
        {
            f_broker->get_connection().amqp()->BasicPublish( f_request_exchange, a_data->f_routing_key, t_message, true, false );
        }
        catch( AmqpClient::MessageReturnedException& e )
        {
            MTERROR( mtlog, "Request message could not be sent: " << e.what() );
            return false;
        }
        catch( std::exception& e )
        {
            MTERROR( mtlog, "Error publishing request to queue: " << e.what() );
            return false;
        }
        return true;
    }

    bool amqp_relayer::relay_alert( message_data* a_data )
    {
        MTDEBUG( mtlog, "Relaying alert" );
        string t_alert_str;
        if( ! encode_message( a_data, t_alert_str ) )
        {
            return false;
        }

        MTINFO( mtlog, "Sending alert with routing key <" << a_data->f_routing_key << ">" );

        AmqpClient::BasicMessage::ptr_t t_message = AmqpClient::BasicMessage::Create( t_alert_str );
        t_message->ContentEncoding( interpret_encoding( a_data->f_encoding ) );
        t_message->CorrelationId( boost::uuids::to_string( f_uuid_gen() ) );

        try
        {
            f_broker->get_connection().amqp()->BasicPublish( f_alert_exchange, a_data->f_routing_key, t_message, true, false );
        }
        catch( AmqpClient::MessageReturnedException& e )
        {
            MTERROR( mtlog, "Alert message could not be sent: " << e.what() );
            return false;
        }
        catch( std::exception& e )
        {
            MTERROR( mtlog, "Error publishing alert to queue: " << e.what() );
            return false;
        }
        return true;
    }

    bool amqp_relayer::encode_message( const message_data* a_data, std::string& a_message ) const
    {
        switch( a_data->f_encoding )
        {
            case k_json:
                if( ! param_output_json::write_string( *(a_data->f_message), a_message, param_output_json::k_compact ) )
                {
                    MTERROR( mtlog, "Could not convert message to string" );
                    return false;
                }
                return true;
                break;
            default:
                MTERROR( mtlog, "Invalid encoding: " << a_data->f_encoding );
                return false;
                break;
        }
        // should not get here
        return false;
    }

    bool amqp_relayer::send_request( const param_node* a_message, std::string a_routing_key, encoding a_encoding )
    {
        message_data* t_data = new message_data();
        t_data->f_message = new param_node( *a_message );
        t_data->f_message_type = k_request;
        t_data->f_routing_key = a_routing_key;
        t_data->f_encoding = a_encoding;
        MTDEBUG( mtlog, "Received send-request request addressed to <" << a_routing_key << ">" );
        f_queue.push( t_data );
        return true;
    }

    bool amqp_relayer::send_alert( const param_node* a_message, std::string a_routing_key, encoding a_encoding )
    {
        message_data* t_data = new message_data();
        t_data->f_message = new param_node( *a_message );
        t_data->f_message_type = k_alert;
        t_data->f_routing_key = a_routing_key;
        t_data->f_encoding = a_encoding;
        MTDEBUG( mtlog, "Received send-alert request addressed to <" << a_routing_key << ">" );
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



}
