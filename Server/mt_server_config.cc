/*
 * mt_client_config.cc
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS
#define M3_API_EXPORTS

#include "mt_server_config.hh"

#include "M3Types.hh"

#include<string>
using std::string;

namespace mantis
{

    server_config::server_config()
    {
        // default server configuration

        param_value t_value;

        param_node* t_broker_node = new param_node();
        t_broker_node->add( "port", t_value << 5672 );
        t_broker_node->add( "addr", t_value << "localhost" );
        add( "broker", t_broker_node );

        param_node* t_run_node = new param_node();
        t_run_node->add( "duration", t_value << 500 );
        add( "run", t_run_node );

        param_node* t_dev_node = new param_node();

        param_node* t_test_node = new param_node();
        t_test_node->add( "name", t_value << "test" );
        t_test_node->add( "enabled", t_value << true );
        t_test_node->add( "rate", t_value << 250 );
        t_test_node->add( "n-channels", t_value << 1 );
        t_test_node->add( "data-mode", t_value << monarch3::sDigitized );
        t_test_node->add( "channel-mode", t_value << monarch3::sInterleaved );
        t_test_node->add( "sample-size", t_value << 1 );
        t_test_node->add( "buffer-size", t_value << 512 );
        t_test_node->add( "record-size", t_value << 4194304 );
        t_test_node->add( "data-chunk-size", t_value << 1024 );
        t_dev_node->add( "test", t_test_node );

        param_node* t_test16_node = new param_node();
        t_test16_node->add( "name", t_value << "test16" );
        t_test16_node->add( "enabled", t_value << false );
        t_test16_node->add( "rate", t_value << 250 );
        t_test16_node->add( "n-channels", t_value << 1 );
        t_test16_node->add( "data-mode", t_value << monarch3::sDigitized );
        t_test16_node->add( "channel-mode", t_value << monarch3::sInterleaved );
        t_test16_node->add( "sample-size", t_value << 1 );
        t_test16_node->add( "buffer-size", t_value << 512 );
        t_test16_node->add( "record-size", t_value << 4194304 );
        t_test16_node->add( "data-chunk-size", t_value << 1024 );
        t_dev_node->add( "test16", t_test16_node );

        param_node* t_pxie5122_node = new param_node();
        t_pxie5122_node->add( "name", t_value << "pxie5122" );
        t_pxie5122_node->add( "enabled", t_value << false );
        t_pxie5122_node->add( "resource-name", t_value << "PXI1Slot2" );
        t_pxie5122_node->add( "rate-req", t_value << 100 );
        t_pxie5122_node->add( "n-channels", t_value << 1 );
        t_pxie5122_node->add( "data-mode", t_value << monarch3::sDigitized );
        t_pxie5122_node->add( "channel-mode", t_value << monarch3::sSeparate );
        t_pxie5122_node->add( "sample-size", t_value << 1 );
        t_pxie5122_node->add( "buffer-size", t_value << 512 );
        t_pxie5122_node->add( "record-size-req", t_value << 524288 );// 1048576 );
        t_pxie5122_node->add( "data-chunk-size", t_value << 1024 );
        t_pxie5122_node->add( "input-impedance", t_value << 50 );
        t_pxie5122_node->add( "voltage-range", t_value << 0.5 );
        t_pxie5122_node->add( "voltage-offset", t_value << -0.25 );
        t_pxie5122_node->add( "input-coupling", t_value << 1 ); // DC coupling
        t_pxie5122_node->add( "probe-attenuation", t_value << 1.0 );
        t_pxie5122_node->add( "acq-timeout", t_value << 10.0 );
        t_dev_node->add( "pxie5122", t_pxie5122_node );

        add( "devices", t_dev_node );

    }

    server_config::~server_config()
    {
    }

} /* namespace Katydid */
