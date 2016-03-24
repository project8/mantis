/*
 * mt_device_manager.cc
 *
 *  Created on: Jan 25, 2015
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS
#define DRIPLINE_API_EXPORTS

#include "mt_device_manager.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_acq_request.hh"

#include "logger.hh"
#include "param.hh"

namespace mantis
{
    LOGGER( mtlog, "device_manager" );

    device_manager::device_manager() :
            f_device_name(),
            f_device( NULL ),
            f_device_config_templates()
    {
        factory< digitizer_config_template >* t_dev_ct_factory = factory< digitizer_config_template >::get_instance();
        for( factory< digitizer_config_template >::FactoryCIt it = t_dev_ct_factory->begin();
                it != t_dev_ct_factory->end(); ++it )
        {
            digitizer_config_template* t_dev_ct = t_dev_ct_factory->create( it );
            if( t_dev_ct == NULL )
            {
                LWARN( mtlog, "Unable to add device config template for device type <" << it->first << ">" );
                continue;
            }
            LDEBUG( mtlog, "Adding device config template for <" << it->first << ">" );
            t_dev_ct->add( &f_device_config_templates, it->first );
            delete t_dev_ct;
        }
    }

    device_manager::~device_manager()
    {
        delete f_device;
    }

    bool device_manager::configure( acq_request& a_acq_request )
    {
        try
        {
            param_node* t_acq_config = a_acq_request.node_at( "acquisition" );
            if( t_acq_config == NULL )
            {
                LERROR( mtlog, "Acquisition configuration is missing" );
                return false;
            }

            param_node* t_device_config = t_acq_config->node_at( "devices" );
            if( t_device_config == NULL )
            {
                LERROR( mtlog, "Device configuration is missing" );
                return false;
            }

            // For now: find the first device that's enabled
            // TODO: for mutli-device usage, will use all enabled devices
            param_node* t_enabled_dev_config = NULL;
            param_node::iterator t_node_it;
            for( t_node_it = t_device_config->begin(); t_node_it != t_device_config->end(); ++t_node_it )
            {
                try
                {
                    if( t_node_it->second->as_node().get_value< bool >( "enabled", false ) )
                    {
                        t_enabled_dev_config = &( t_node_it->second->as_node() );
                        break;
                    }
                }
                catch( exception& e )
                {
                    LWARN( mtlog, "Found non-node param object in \"devices\"" );
                }
            }
            if( t_enabled_dev_config == NULL )
            {
                LERROR( mtlog, "Did not find an enabled device" );
                return false;
            }

            if( ! set_device( t_node_it->second->as_node().get_value( "type" ) ) )
            {
                LERROR( mtlog, "Unable to set device" );
                return false;
            }

            if( ! f_device->initialize( t_acq_config, t_enabled_dev_config ) )
            {
                LERROR( mtlog, "Unable to configure device" );
                return false;
            }
        }
        catch( exception& e )
        {
            LERROR( mtlog, "An exception was thrown while configuring the device manager:\n\t" << e.what() );
        }

        return true;
    }

    bool device_manager::set_device( const std::string& a_dev )
    {
        if( a_dev != f_device_name )
        {
            delete f_device;
            f_device = NULL;

            try
            {
                factory< digitizer >* t_dig_factory = factory< digitizer >::get_instance();
                f_device = t_dig_factory->create( a_dev );
                if( f_device == NULL )
                {
                    LERROR( mtlog, "Could not create digitizer <" << a_dev << ">; aborting" );
                    return false;
                }
            }
            catch( exception& e )
            {
                LERROR( mtlog, "Exception caught while creating device <" << a_dev << ">: " << e.what() );
                return false;
            }

            f_device_name = a_dev;
        }

        return true;
    }

    digitizer* device_manager::device()
    {
        return f_device;
    }

    buffer* device_manager::get_buffer()
    {
        return f_device->get_buffer();
    }

    condition* device_manager::buffer_condition()
    {
        return f_device->get_buffer_condition();
    }

    param_node* device_manager::get_device_config( const std::string& a_device_type )
    {
        const param_node* t_device_config = f_device_config_templates.node_at( a_device_type );
        if( t_device_config == NULL ) return NULL;
        return new param_node( *t_device_config );
    }

} /* namespace mantis */
