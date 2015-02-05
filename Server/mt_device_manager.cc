/*
 * mt_device_manager.cc
 *
 *  Created on: Jan 25, 2015
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_device_manager.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_factory.hh"
#include "mt_logger.hh"
#include "mt_param.hh"
#include "mt_run_description.hh"

namespace mantis
{
    MTLOGGER( mtlog, "device_manager" );

    device_manager::device_manager() :
            f_device_name(),
            f_device( NULL )
    {
    }

    device_manager::~device_manager()
    {
        delete f_device;
    }

    bool device_manager::configure( run_description& a_run_desc )
    {
        param_node* t_mantis_config = a_run_desc.node_at( "mantis-config" );
        if( t_mantis_config == NULL )
        {
            MTERROR( mtlog, "Mantis configuration is missing" );
            return false;
        }

        param_node* t_device_config = t_mantis_config->node_at( "devices" );
        if( t_device_config == NULL )
        {
            MTERROR( mtlog, "Device configuration is missing" );
            return false;
        }

        // For now: find the first device that's enabled
        // TODO: for mutli-device usage, will use all enabled devices
        param_node* t_enabled_dev_config = NULL;
        for( param_node::iterator t_node_it = t_device_config->begin(); t_node_it != t_device_config->end(); ++t_node_it )
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
                MTWARN( mtlog, "Found non-node param object in \"devices\"" );
            }
        }
        if( t_enabled_dev_config == NULL )
        {
            MTERROR( mtlog, "Did not find an enabled device" );
            return false;
        }

        if( !set_device( t_enabled_dev_config->get_value( "name" ) ) )
        {
            MTERROR( mtlog, "Unable to set device" );
            return false;
        }

        if( ! f_device->initialize( t_mantis_config, t_enabled_dev_config ) )
        {
            MTERROR( mtlog, "Unable to configure device" );
            return false;
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
                    MTERROR( mtlog, "Could not create digitizer <" << a_dev << ">; aborting" );
                    return false;
                }
            }
            catch( exception& e )
            {
                MTERROR( mtlog, "Exception caught while creating device <" << a_dev << ">: " << e.what() );
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



} /* namespace mantis */
