/*
 * mt_acq_request.cc
 *
 *  Created on: Mar 20, 2014
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_acq_request.hh"
#include "mt_version.hh"

#include<string>
using std::string;

#include "mt_logger.hh"

namespace mantis
{
    MTLOGGER(mtlog, "acq_request");

    acq_request::acq_request()
    {
        // default description
        add( "id", param_value( 0 ) );
        add( "status", param_value( 0 ) );
        add( "client", new param_node() );
        add( "mantis", new param_node() );
        add( "monarch", new param_node() );
        add( "mantis-config", new param_node() );
        add( "response", new param_node() );
        add( "file", new param_value( "scrambled.egg" ) );
        add( "description", new param_value( "" ) );
    }

    acq_request::~acq_request()
    {
    }

    void acq_request::set_id( unsigned a_id )
    {
        this->replace( "id", param_value( a_id ) );
        return;
    }

    unsigned acq_request::get_id() const
    {
        return this->get_value< unsigned >( "id" );
    }

    void acq_request::set_status( status a_status )
    {
        this->replace( "status", param_value( (unsigned)a_status ) );
        return;
    }

    acq_request::status acq_request::get_status() const
    {
        return (status)(this->get_value< unsigned >( "status" ));
    }

    void acq_request::set_client_exe( const std::string& a_exe )
    {
        this->node_at( "client" )->replace( "exe", param_value( a_exe ) );
        return;
    }

    void acq_request::set_client_version( const std::string& a_ver )
    {
        this->node_at( "client" )->replace( "version", param_value( a_ver ) );
        return;
    }

    void acq_request::set_client_commit( const std::string& a_ver )
    {
        this->node_at( "client" )->replace( "commit", param_value( a_ver ) );
        return;
    }

    void acq_request::set_mantis_server_exe( const std::string& a_exe )
    {
        this->node_at( "mantis" )->replace( "exe", param_value( a_exe ) );
        return;
    }

    void acq_request::set_mantis_server_version( const std::string& a_ver )
    {
        this->node_at( "mantis" )->replace( "version", param_value( a_ver ) );
        return;
    }

    void acq_request::set_mantis_server_commit( const std::string& a_ver )
    {
        this->node_at( "mantis" )->replace( "commit", param_value( a_ver ) );
        return;
    }

    void acq_request::set_monarch_version( const std::string& a_ver )
    {
        this->node_at( "monarch" )->replace( "version", param_value( a_ver ) );
        return;
    }

    void acq_request::set_monarch_commit( const std::string& a_ver )
    {
        this->node_at( "monarch" )->replace( "commit", param_value( a_ver ) );
        return;
    }

    void acq_request::set_file_config( const param_value& a_config )
    {
        this->replace( "file", a_config );
        return;
    }

    void acq_request::set_description_config( const param_value& a_config )
    {
        this->replace( "description", a_config );
        return;
    }

    void acq_request::set_mantis_config( const param_node& a_config )
    {
        this->replace( "mantis-config", a_config );
        return;
    }

    void acq_request::set_response( const param_node& a_response )
    {
        this->replace( "response", a_response );
        return;
    }

} /* namespace mantis */
