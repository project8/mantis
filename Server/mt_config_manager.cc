/*
 * mt_config_manager.cc
 *
 *  Created on: May 4, 2015
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_config_manager.hh"

#include "mt_exception.hh"
#include "mt_device_manager.hh"
#include "mt_message.hh"
#include "mt_parser.hh"
#include "mt_request_receiver.hh"

#include "logger.hh"
#include "parsable.hh"

namespace mantis
{
    using std::string;

    using scarab::parsable;

    LOGGER( mtlog, "config_manager" );

    config_manager::config_manager( const param_node& a_config, device_manager* a_dev_mgr ) :
            f_master_server_config( a_config ),
            f_dev_mgr( a_dev_mgr )
    {
    }

    config_manager::~config_manager()
    {
    }

    param_node* config_manager::copy_master_server_config( const std::string& a_node_name ) const
    {
        param_node* t_copy = NULL;
        f_msc_mutex.lock();
        if( a_node_name.empty() ) t_copy = new param_node( f_master_server_config );
        else t_copy = new param_node( *( f_master_server_config.node_at( a_node_name ) ) );
        f_msc_mutex.unlock();
        return t_copy;
    }

    bool config_manager::handle_get_acq_config_request( const msg_request* /*a_request*/, request_reply_package& a_pkg )
    {
        f_msc_mutex.lock();
        a_pkg.f_payload.merge( *f_master_server_config.node_at( "acq" ) );
        f_msc_mutex.unlock();
        return a_pkg.send_reply( R_SUCCESS, "Get request succeeded" );
    }

    bool config_manager::handle_get_server_config_request( const msg_request* /*a_request*/, request_reply_package& a_pkg )
    {
        f_msc_mutex.lock();
        a_pkg.f_payload.merge( f_master_server_config );
        f_msc_mutex.unlock();
        return a_pkg.send_reply( R_SUCCESS, "Get request succeeded" );
    }

    bool config_manager::handle_set_request( const msg_request* a_request, request_reply_package& a_pkg)
    {
        string t_routing_key = a_request->get_mantis_routing_key();
        if( t_routing_key.empty() )
        {
            a_pkg.send_reply( R_AMQP_ERROR_ROUTINGKEY_NOTFOUND, "No routing key was provided" );
            return false;
        }

        DEBUG( mtlog, "Applying a setting" );
        // apply a configuration setting
        // the destination node should specify the configuration to set
        try
        {
            f_msc_mutex.lock();
            t_routing_key += string( "=" ) + a_request->get_payload().array_at( "values" )->get_value( 0 );
            parsable t_routing_key_node_with_value( t_routing_key );
            DEBUG( mtlog, "Parsed routing key and added value:\n" << t_routing_key_node_with_value );
            if( ! f_master_server_config.node_at( "acq" )->has_subset( t_routing_key_node_with_value ) )
            {
                f_msc_mutex.unlock();
                a_pkg.send_reply( R_DEVICE_ERROR, "Value not found: " + t_routing_key );
                return false;
            }
            f_master_server_config.node_at( "acq" )->merge( t_routing_key_node_with_value );
            f_msc_mutex.unlock();
        }
        catch( exception& e )
        {
            f_msc_mutex.unlock();
            a_pkg.send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, string( "Invalid payload: " ) + e.what() );
            return false;
        }

        f_msc_mutex.lock();
        a_pkg.f_payload.add( "master-config", *f_master_server_config.node_at( "acq" ) );
        f_msc_mutex.unlock();

        return a_pkg.send_reply( R_SUCCESS, "Request succeeded" );
    }


    bool config_manager::handle_add_request( const msg_request* a_request, request_reply_package& a_pkg )
    {
        // add something to the master config

        parsable t_dest_node( a_request->get_mantis_routing_key() );

        if( ! t_dest_node[ "add" ].is_node() )
        {
            a_pkg.send_reply( R_DEVICE_ERROR, "<add> instruction was not properly formatted" );
            return false;
        }
        if( t_dest_node.node_at( "add" )->has( "device" ) )
        {
            DEBUG( mtlog, "Attempting to add a device" );
            // it's expected that any values in the payload are digitizers to be added
            try
            {
                param_node* t_devices_node = f_master_server_config.node_at( "acq" )->node_at( "devices" );
                const param_node& t_req_payload = a_request->get_payload();
                for( param_node::const_iterator t_dev_it = t_req_payload.begin(); t_dev_it != t_req_payload.end(); ++t_dev_it )
                {
                    string t_device_type = t_dev_it->first;
                    if( ! t_dev_it->second->is_value() )
                    {
                        DEBUG( mtlog, "Skipping <" << t_device_type << "> because it's not a value" );
                        continue;
                    }
                    string t_device_name = t_dev_it->second->as_value().as_string();

                    // check if we have a device of this name
                    if( t_devices_node->has( t_device_name ) )
                    {
                        a_pkg.send_reply( R_DEVICE_ERROR, "The master config already has device <" + t_device_name + ">" );
                        return false;
                    }

                    // get the config template from the device manager
                    param_node* t_device_config = f_dev_mgr->get_device_config( t_device_type );
                    if( t_device_config == NULL )
                    {
                        a_pkg.send_reply( R_DEVICE_ERROR, "Did not find device of type <" + t_device_type + ">" );
                        return false;
                    }
                    t_device_config->add( "type", param_value( t_device_type ) );
                    t_device_config->add( "enabled", param_value( 0 ) );

                    // add the configuration to the master config
                    t_devices_node->add( t_device_name, t_device_config );
                }
            }
            catch( exception& e )
            {
                a_pkg.send_reply( R_AMQP_ERROR_ROUTINGKEY_NOTFOUND, string( "add.device instruction was not formatted properly: " ) + e.what() );
                return false;
            }
        }
        else
        {
            a_pkg.send_reply( R_AMQP_ERROR_ROUTINGKEY_NOTFOUND, "Invalid add instruction" );
            return false;
        }

        f_msc_mutex.lock();
        a_pkg.f_payload.add( "master-config", f_master_server_config );
        f_msc_mutex.unlock();

        return a_pkg.send_reply( R_SUCCESS, "Add request succeeded" );
    }

    bool config_manager::handle_remove_request( const msg_request* a_request, request_reply_package& a_pkg )
    {
        // remove something from the master config

        parsable t_dest_node( a_request->get_mantis_routing_key() );

        if( ! t_dest_node[ "remove" ].is_node() )
        {
            a_pkg.send_reply( R_DEVICE_ERROR, "<remove> instruction was not properly formatted" );
            return false;
        }
        if( t_dest_node.node_at( "remove" )->has( "device" ) )
        {
            DEBUG( mtlog, "Attempting to remove a device" );
            try
             {
                 string t_device_name = t_dest_node.node_at( "remove" )->node_at( "device" )->begin()->first;

                 // check if we have a device of this name
                 if( ! f_master_server_config.node_at( "acq" )->node_at( "devices" )->has( t_device_name ) )
                 {
                     a_pkg.send_reply( R_DEVICE_ERROR, "The master config does not have device <" + t_device_name + ">" );
                     return false;
                 }

                 // add the configuration to the master config
                 f_master_server_config.node_at( "acq" )->node_at( "devices" )->erase( t_device_name );
             }
             catch( exception& e )
             {
                a_pkg.send_reply( R_MESSAGE_ERROR_INVALID_VALUE, string( "remove.device instruction was not formatted properly: " ) + e.what() );
                return false;
             }
        }
        else
        {
            a_pkg.send_reply( R_SUCCESS, "Invalid remove instruction" );
            return false;
        }

        f_msc_mutex.lock();
        a_pkg.f_payload.add( "master-config", f_master_server_config );
        f_msc_mutex.unlock();

        return a_pkg.send_reply( R_AMQP_ERROR_ROUTINGKEY_NOTFOUND, "Remove instruction succeeded" );
    }


    bool config_manager::handle_replace_acq_config( const msg_request* a_request, request_reply_package& a_pkg )
    {
        // payload contents should replace the acquisition config
        DEBUG( mtlog, "Loading a full configuration" );
        f_msc_mutex.lock();
        (*f_master_server_config.node_at( "acq" )) = a_request->get_payload();

        a_pkg.f_payload.add( "master-config", f_master_server_config );
        f_msc_mutex.unlock();

        return a_pkg.send_reply( R_SUCCESS, "Request_succeeded" );
    }




} /* namespace mantis */
