/*
 * mt_message.cc
 *
 *  Created on: Jul 9, 2015
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_message.hh"

#include "mt_broker.hh"
#include "mt_constants.hh"
#include "mt_logger.hh"
#include "mt_param_json.hh"
#include "mt_version.hh"
#include "thorax.hh"

#include <boost/uuid/uuid_io.hpp>

using std::string;

namespace mantis
{
    MTLOGGER( mtlog, "message" );

    //***********
    // Message
    //***********

    boost::uuids::random_generator message::f_uuid_gen;

    message::message() :
            f_routing_key(),
            f_correlation_id(),
            f_encoding( k_json ),
            f_timestamp(),
            f_sender_info(),
            f_payload( new param_node() )
    {
        version_global* t_version = version_global::get_instance();
        f_sender_info.f_commit = t_version->commit();
        f_sender_info.f_version = t_version->version_str();
        f_sender_info.f_package = t_version->package();
        f_sender_info.f_exe = t_version->exe_name();
        f_sender_info.f_hostname = t_version->hostname();
        f_sender_info.f_username = t_version->username();
    }

    message::~message()
    {
        delete f_payload;
    }

    amqp_message_ptr message::create_amqp_message() const
    {
        string t_body;
        if( ! encode_message_body( t_body ) )
        {
            MTERROR( mtlog, "Unable to encode message body" );
            return amqp_message_ptr();
        }

        amqp_message_ptr t_message = AmqpClient::BasicMessage::Create( t_body );
        t_message->ContentEncoding( interpret_encoding() );
        t_message->CorrelationId( f_correlation_id );
        this->derived_modify_amqp_message( t_message );

        return t_message;
    }

    bool message::encode_message_body( std::string& a_body ) const
    {
        param_node* t_sender_node = new param_node();
        t_sender_node->add( "commit", param_value( f_sender_info.f_commit ) );
        t_sender_node->add( "exe", param_value( f_sender_info.f_exe ) );
        t_sender_node->add( "version", param_value( f_sender_info.f_version ) );
        t_sender_node->add( "package", param_value( f_sender_info.f_package ) );
        t_sender_node->add( "hostname", param_value( f_sender_info.f_hostname ) );
        t_sender_node->add( "username", param_value( f_sender_info.f_username ) );

        param_node t_body_node;
        t_body_node.add( "msgtype", param_value( T_REQUEST ) );
        t_body_node.add( "timestamp", param_value( get_absolute_time_string() ) );
        t_body_node.add( "sender_info", t_sender_node );
        t_body_node.add( "payload", f_payload->clone() ); // use a clone of f_payload

        if( ! this->derived_modify_message_body( t_body_node ) )
        {
            MTERROR( mtlog, "Something went wrong in the derived-class modify_body_message function" );
            return false;
        }

        switch( f_encoding )
        {
            case k_json:
                if( ! param_output_json::write_string( t_body_node, a_body, param_output_json::k_compact ) )
                {
                    MTERROR( mtlog, "Could not convert message body to string" );
                    return false;
                }
                return true;
                break;
            default:
                MTERROR( mtlog, "Cannot encode using <" << interpret_encoding() << "> (" << f_encoding << ")" );
                return false;
                break;
        }
        // should not get here
        return false;
    }

    string message::interpret_encoding() const
    {
        switch( f_encoding )
        {
            case k_json:
                return std::string( "application/json" );
                break;
            case k_msgpack:
                return std::string( "application/msgpack" );
                break;
            default:
                return std::string( "Unknown" );
        }
    }



    //***********
    // Request
    //***********

    msg_request::msg_request() :
            message(),
            f_reply_to(),
            f_message_op( OP_UNKNOWN )
    {
        f_correlation_id = boost::uuids::to_string( f_uuid_gen() );
    }

    msg_request::~msg_request()
    {

    }

    unsigned msg_request::f_message_type = T_REQUEST;
    unsigned msg_request::message_type()
    {
        return msg_request::f_message_type;
    }
    unsigned msg_request::get_message_type() const
    {
        return msg_request::f_message_type;
    }

    msg_request* msg_request::create( param_node* a_payload, unsigned a_msg_op, const std::string& a_routing_key, message::encoding a_encoding )
    {
        msg_request* t_request = new msg_request();
        t_request->set_payload( a_payload );
        t_request->set_message_op( a_msg_op );
        t_request->set_routing_key( a_routing_key );
        t_request->set_encoding( a_encoding );
        return t_request;
    }

    bool msg_request::do_publish( amqp_channel_ptr a_channel, const std::string& a_exchange, std::string& a_reply_consumer_tag )
    {
        // create the reply-to queue
        string t_reply_to = a_channel->DeclareQueue( "" );
        set_reply_to( t_reply_to );

        // begin consuming on the reply-to queue
        // TODO: is this where this should be done?
        a_reply_consumer_tag = a_channel->BasicConsume( t_reply_to );
        MTDEBUG( mtlog, "Consumer tag for reply: " << a_reply_consumer_tag );

        MTINFO( mtlog, "Sending request with routing key <" << get_routing_key() << ">" );

        amqp_message_ptr t_message = create_amqp_message();

        try
        {
            a_channel->BasicPublish( a_exchange, f_routing_key, t_message, true, false );
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


    //*********
    // Reply
    //*********

    msg_reply::msg_reply() :
            message(),
            f_return_code( R_SUCCESS ),
            f_return_msg(),
            f_return_buffer()
    {
    }

    msg_reply::~msg_reply()
    {

    }

    unsigned msg_reply::f_message_type = T_REPLY;
    unsigned msg_reply::message_type()
    {
        return msg_reply::f_message_type;
    }
    unsigned msg_reply::get_message_type() const
    {
        return msg_reply::f_message_type;
    }

    msg_reply* msg_reply::create( unsigned a_retcode, const std::string& a_ret_msg, param_node* a_payload, const std::string& a_routing_key, message::encoding a_encoding )
    {
        msg_reply* t_reply = new msg_reply();
        t_reply->set_return_code( a_retcode );
        t_reply->set_return_message( a_ret_msg );
        t_reply->set_payload( a_payload );
        t_reply->set_routing_key( a_routing_key );
        t_reply->set_encoding( a_encoding );
        return t_reply;
    }

    bool msg_reply::do_publish( amqp_channel_ptr a_channel, const std::string& a_exchange, std::string& a_reply_consumer_tag )
    {
        MTINFO( mtlog, "Sending reply with routing key <" << get_routing_key() << ">" );

        a_reply_consumer_tag.clear(); // no reply expected

        amqp_message_ptr t_message = create_amqp_message();

        try
        {
            a_channel->BasicPublish( a_exchange, f_routing_key, t_message, true, false );
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


    //*********
    // Alert
    //*********

    msg_alert* msg_alert::create( param_node* a_payload, const std::string& a_routing_key, message::encoding a_encoding )
    {
        msg_alert* t_alert = new msg_alert();
        t_alert->set_payload( a_payload );
        t_alert->set_routing_key( a_routing_key );
        t_alert->set_encoding( a_encoding );
        return t_alert;
    }

    msg_alert::msg_alert() :
            message()
    {
        f_correlation_id = boost::uuids::to_string( f_uuid_gen() );
    }

    msg_alert::~msg_alert()
    {

    }

    unsigned msg_alert::f_message_type = T_ALERT;
    unsigned msg_alert::message_type()
    {
        return msg_alert::f_message_type;
    }
    unsigned msg_alert::get_message_type() const
    {
        return msg_alert::f_message_type;
    }

    bool msg_alert::do_publish( amqp_channel_ptr a_channel, const std::string& a_exchange, std::string& a_reply_consumer_tag )
    {
        MTINFO( mtlog, "Sending alert with routing key <" << get_routing_key() << ">" );

        a_reply_consumer_tag.clear(); // no reply expected

        amqp_message_ptr t_message = create_amqp_message();

        try
        {
            a_channel->BasicPublish( a_exchange, f_routing_key, t_message, true, false );
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


    //********
    // Info
    //********

    msg_info::msg_info() :
            message()
    {
        f_correlation_id = boost::uuids::to_string( f_uuid_gen() );
    }

    msg_info::~msg_info()
    {

    }

    unsigned msg_info::f_message_type = T_INFO;
    unsigned msg_info::message_type()
    {
        return msg_info::f_message_type;
    }
    unsigned msg_info::get_message_type() const
    {
        return msg_info::f_message_type;
    }

    bool msg_info::do_publish( amqp_channel_ptr a_channel, const std::string& a_exchange, std::string& a_reply_consumer_tag )
    {
        MTINFO( mtlog, "Sending info with routing key <" << get_routing_key() << ">" );

        a_reply_consumer_tag.clear(); // no reply expected

        amqp_message_ptr t_message = create_amqp_message();

        try
        {
            a_channel->BasicPublish( a_exchange, f_routing_key, t_message, true, false );
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

} /* namespace mantis */
