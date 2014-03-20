/*
 * mt_run_description.cc
 *
 *  Created on: Mar 20, 2014
 *      Author: nsoblath
 */

#include "mt_run_description.hh"

#include<string>
using std::string;

namespace mantis
{

    run_description::run_description()
    {
        // default description
        param_node t_empty_node;
        add( "mantis", t_empty_node );
        add( "monarch", t_empty_node );
        add( "client-config", t_empty_node );
        add( "server-config", t_empty_node );
        add( "description", param_value() );
    }

    run_description::~run_description()
    {
    }


    void run_description::set_mantis_client_exe( const std::string& a_exe )
    {
        param_value t_value;
        this->node_at( "mantis" )->replace( "client-exe", t_value << a_exe );
        return;
    }

    void run_description::set_mantis_server_exe( const std::string& a_exe )
    {
        param_value t_value;
        this->node_at( "mantis" )->replace( "server-exe", t_value << a_exe );
        return;
    }

    void run_description::set_mantis_client_version( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "mantis" )->replace( "client-version", t_value << a_ver );
        return;
    }

    void run_description::set_mantis_server_version( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "mantis" )->replace( "server-version", t_value << a_ver );
        return;
    }

    void run_description::set_monarch_version( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "monarch" )->replace( "version", t_value << a_ver );
        return;
    }

    void run_description::set_mantis_client_commit( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "mantis" )->replace( "client-commit", t_value << a_ver );
        return;
    }

    void run_description::set_mantis_server_commit( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "mantis" )->replace( "server-commit", t_value << a_ver );
        return;
    }

    void run_description::set_monarch_commit( const std::string& a_ver )
    {
        param_value t_value;
        this->node_at( "monarch" )->replace( "commit", t_value << a_ver );
        return;
    }

    void run_description::set_description( const std::string& a_desc )
    {
        param_value t_value;
        this->replace( "description", t_value << a_desc );
        return;
    }

    void run_description::set_client_config( const param_node& a_config )
    {
        param_value t_value;
        this->replace( "client-config", a_config );
        return;
    }

    void run_description::set_server_config( const param_node& a_config )
    {
        param_value t_value;
        this->replace( "server-config", a_config );
        return;
    }


} /* namespace Katydid */
