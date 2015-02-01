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
            f_device( NULL ),
            f_buffer_condition(),
            f_buffer( NULL ),
            f_buffer_size( 0 ),
            f_block_size( 0 )
    {
    }

    device_manager::~device_manager()
    {
        delete f_device;
    }

    bool device_manager::configure( run_description& a_run_desc )
    {
        const param_node* t_mantis_config = a_run_desc.node_at( "mantis-config" );
        if( t_mantis_config == NULL )
        {
            MTERROR( mtlog, "Mantis configuration is missing" );
            return false;
        }

        //TODO: for multi-device mode, this node will be called "devices", and each device will have its own node(?)
        const param_node* t_device_config = t_mantis_config->node_at( "device" );
        if( t_device_config == NULL )
        {
            MTERROR( mtlog, "Device configuration is missing" );
        }

        if( ! set_device( t_device_config->get_value( "name"), t_device_config->get_value< unsigned >( "buffer-size" ), t_device_config->get_value< unsigned >( "block-size" ) ) )
        {
            MTERROR( mtlog, "Unable to set device" );
            return false;
        }

        if( ! f_device->initialize( t_mantis_config ) )
        {
            MTERROR( mtlog, "Unable to configure device" );
            return false;
        }

        return true;
    }

    bool device_manager::set_device( const std::string& a_dev, unsigned a_buffer_size, unsigned a_block_size )
    {
        if( f_buffer != NULL && (a_dev != f_device_name || a_buffer_size != f_buffer_size || a_block_size != f_block_size) )
        {
            f_device->deallocate( f_buffer );
            f_buffer = NULL;
        }

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

        if( f_buffer == NULL )
        {
            f_buffer = new buffer( a_buffer_size, a_block_size );
            if(! f_device->allocate( f_buffer, &f_buffer_condition ) )
            {
                MTERROR( mtlog, "Device <" << a_dev << "> was not able to allocate the buffer" );
                return false;
            }
            f_buffer_size = a_buffer_size;
            f_block_size = a_block_size;
        }

        return true;
    }

    digitizer* device_manager::device()
    {
        return f_device;
    }

    buffer* device_manager::get_buffer()
    {
        return f_buffer;
    }

    condition* device_manager::buffer_condition()
    {
        return &f_buffer_condition;
    }



} /* namespace mantis */
