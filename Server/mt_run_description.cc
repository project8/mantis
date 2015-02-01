/*
 * mt_run_description.cc
 *
 *  Created on: Mar 20, 2014
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_run_description.hh"
#include "mt_version.hh"

#include<string>
using std::string;

#include "mt_logger.hh"

namespace mantis
{
    MTLOGGER(mtlog, "run_description");

    run_description::run_description()
    {
        // default description
        add( "id", param_value( 0 ) );
        add( "status", param_value( 0 ) );
        add( "client", new param_node() );
        add( "mantis", new param_node() );
        add( "monarch", new param_node() );
        add( "mantis-config", new param_node() );
        add( "response", new param_node() );
    }

    run_description::~run_description()
    {
    }

    void run_description::set_id( unsigned a_id )
    {
        param_value t_value;
        this->replace( "id", t_value << a_id );
        return;
    }

    unsigned run_description::get_id() const
    {
        return this->get_value< unsigned >( "id" );
    }

    void run_description::set_status( status a_status )
    {
        param_value t_value;
        this->replace( "status", t_value << (unsigned)a_status );
        return;
    }

    run_description::status run_description::get_status() const
    {
        return (status)(this->get_value< unsigned >( "status" ));
    }

    void run_description::set_client_exe( const std::string& a_exe )
    {
        param_value t_value;
        this->node_at( "client" )->replace( "exe", t_value << a_exe );
        return;
    }

    void run_description::set_client_version( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "client" )->replace( "version", t_value << a_ver );
        return;
    }

    void run_description::set_client_commit( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "client" )->replace( "commit", t_value << a_ver );
        return;
    }

    void run_description::set_mantis_server_exe( const std::string& a_exe )
    {
        param_value t_value;
        this->node_at( "mantis" )->replace( "exe", t_value << a_exe );
        return;
    }

    void run_description::set_mantis_server_version( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "mantis" )->replace( "version", t_value << a_ver );
        return;
    }

    void run_description::set_mantis_server_commit( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "mantis" )->replace( "commit", t_value << a_ver );
        return;
    }

    void run_description::set_monarch_version( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "monarch" )->replace( "version", t_value << a_ver );
        return;
    }

    void run_description::set_monarch_commit( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "monarch" )->replace( "commit", t_value << a_ver );
        return;
    }

    void run_description::set_file_config( const param_node& a_config )
    {
        this->replace( "file", a_config );
        return;
    }

    void run_description::set_mantis_config( const param_node& a_config )
    {
        this->replace( "mantis-config", a_config );
        return;
    }

    void run_description::set_response( const param_node& a_response )
    {
        this->replace( "response", a_response );
        return;
    }

} /* namespace Katydid */
