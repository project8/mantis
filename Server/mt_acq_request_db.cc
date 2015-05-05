#define MANTIS_API_EXPORTS

#include "mt_acq_request_db.hh"

#include "mt_acq_request.hh"
#include "mt_config_manager.hh"
#include "mt_condition.hh"
#include "mt_logger.hh"
#include "mt_param.hh"
#include "mt_request_receiver.hh"

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>


using std::string;

namespace mantis
{
    MTLOGGER( mtlog, "acq_request_db" );

    acq_request_db::acq_request_db( config_manager* a_conf_mgr, condition* a_condition, const std::string& a_exe_name ) :
            f_mutex(),
            f_acq_request_queue(),
            f_acq_request_db(),
            f_queue_condition( a_condition ),
            f_config_mgr( a_conf_mgr ),
            f_exe_name( a_exe_name )
    {
    }

    acq_request_db::~acq_request_db()
    {
        clear();
        return;
    }

    bool acq_request_db::empty()
    {
        bool t_empty;
        f_mutex.lock();
        t_empty = f_acq_request_db.empty();
        f_mutex.unlock();
        return t_empty;
    }

    acq_request* acq_request_db::get_acq_request( boost::uuids::uuid a_id )
    {
        acq_request* t_desc = NULL;
        f_mutex.lock();
        acq_request_db_data::iterator t_acq_request_it;
        t_acq_request_it = f_acq_request_db.find( a_id );
        if( t_acq_request_it != f_acq_request_db.end()) t_desc = t_acq_request_it->second;
        f_mutex.unlock();
        return t_desc;
    }

    const acq_request* acq_request_db::get_acq_request( boost::uuids::uuid a_id ) const
    {
        const acq_request* t_desc = NULL;
        f_mutex.lock();
        acq_request_db_data::const_iterator t_acq_request_it;
        t_acq_request_it = f_acq_request_db.find( a_id );
        if( t_acq_request_it != f_acq_request_db.end()) t_desc = t_acq_request_it->second;
        f_mutex.unlock();
        return t_desc;
    }

    boost::uuids::uuid acq_request_db::add( acq_request* a_acq_request )
    {
        boost::uuids::uuid t_id = boost::uuids::nil_uuid();
        f_mutex.lock();
        f_acq_request_db[ a_acq_request->get_id() ] = a_acq_request;
        t_id = a_acq_request->get_id();
        f_mutex.unlock();
        return t_id;
    }

    acq_request* acq_request_db::remove( boost::uuids::uuid a_id )
    {
        acq_request* t_desc = NULL;
        f_mutex.lock();
        acq_request_db_data::iterator t_acq_request_it;
        t_acq_request_it = f_acq_request_db.find( a_id );
        if( t_acq_request_it != f_acq_request_db.end())
        {
            t_desc = t_acq_request_it->second;
            f_acq_request_db.erase( t_acq_request_it );
        }
        f_mutex.unlock();
        return t_desc;
    }


    void acq_request_db::flush()
    {
        f_mutex.lock();
        acq_request_db_data::iterator t_it_counter = f_acq_request_db.begin();
        acq_request_db_data::iterator t_it_deletable;
        while( t_it_counter != f_acq_request_db.end() )
        {
            t_it_deletable = t_it_counter++;
            if( t_it_deletable->second->get_status() > acq_request::running )
            {
                delete t_it_deletable->second;
            }
        }
        f_mutex.unlock();
        return;
    }

    void acq_request_db::clear()
    {
        f_mutex.lock();
        for( acq_request_db_data::iterator t_acq_request_it = f_acq_request_db.begin(); t_acq_request_it != f_acq_request_db.end(); ++t_acq_request_it )
        {
            delete t_acq_request_it->second;
        }
        f_acq_request_db.clear();
        f_acq_request_queue.clear();
        f_mutex.unlock();
        return;
    }

    bool acq_request_db::queue_empty()
    {
        bool t_empty;
        f_mutex.lock();
        t_empty = f_acq_request_queue.empty();
        f_mutex.unlock();
        return t_empty;
    }

    boost::uuids::uuid acq_request_db::enqueue( acq_request* a_acq_request )
    {
        boost::uuids::uuid t_id = boost::uuids::nil_uuid();
        f_mutex.lock();
        f_acq_request_db[ a_acq_request->get_id() ] = a_acq_request;
        f_acq_request_queue.push_back( a_acq_request );
        a_acq_request->set_status( acq_request::waiting );
        t_id = a_acq_request->get_id();
        f_mutex.unlock();
        return t_id;
    }

    acq_request* acq_request_db::pop()
    {
        acq_request* t_acq_request = NULL;
        f_mutex.lock();
        if( ! f_acq_request_queue.empty() )
        {
            t_acq_request = f_acq_request_queue.front();
            f_acq_request_queue.pop_front();
            t_acq_request->set_status( acq_request::revoked );
        }
        f_mutex.unlock();
        return t_acq_request;
    }

    void acq_request_db::clear_queue()
    {
        f_mutex.lock();
        while( ! f_acq_request_queue.empty() )
        {
            acq_request_db_data::iterator t_acq_request_it = f_acq_request_db.find( f_acq_request_queue.front()->get_id() );
            delete t_acq_request_it->second;
            f_acq_request_db.erase( t_acq_request_it );
            f_acq_request_queue.pop_front();
        }
        f_mutex.unlock();
        return;
    }

    bool acq_request_db::handle_new_acq_request( const param_node& a_msg_payload, const std::string& /*a_mantis_routing_key*/, request_reply_package& a_pkg )
    {
        // required
        const param_value* t_file_node = a_msg_payload.value_at( "file" );
        if( t_file_node == NULL )
        {
            MTWARN( mtlog, "No or invalid file configuration present; aborting run request" );
            a_pkg.send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "No file configuration present; aborting request" );
            return false;
        }

        // optional
        const param_node* t_client_node = a_msg_payload.node_at( "client" );

        boost::uuids::random_generator t_gen;
        acq_request* t_acq_req = new acq_request( t_gen() );
        t_acq_req->set_status( acq_request::created );

        t_acq_req->set_file_config( *t_file_node );
        if( a_msg_payload.has( "description" ) ) t_acq_req->set_description_config( *(a_msg_payload.value_at( "description" ) ) );

        t_acq_req->set_mantis_server_commit( TOSTRING(Mantis_GIT_COMMIT) );
        t_acq_req->set_mantis_server_exe( f_exe_name );
        t_acq_req->set_mantis_server_version( TOSTRING(Mantis_VERSION) );
        t_acq_req->set_monarch_commit( TOSTRING(Monarch_GIT_COMMIT) );
        t_acq_req->set_monarch_version( TOSTRING(Monarch_VERSION ) );

        param_node* t_acq_config = f_config_mgr->copy_master_server_config( "acq" );
        t_acq_req->set_mantis_config( *t_acq_config );
        delete t_acq_config;
        // remove non-enabled devices from the devices node
        param_node* t_dev_node = t_acq_req->node_at( "mantis-config" )->node_at( "devices" );
        std::vector< std::string > t_devs_to_remove;
        for( param_node::iterator t_node_it = t_dev_node->begin(); t_node_it != t_dev_node->end(); ++t_node_it )
        {
            try
            {
                if( ! t_node_it->second->as_node().get_value< bool >( "enabled", false ) )
                {
                    t_devs_to_remove.push_back( t_node_it->first );
                }
            }
            catch( exception& )
            {
                MTWARN( mtlog, "Found non-node param object in \"devices\"" );
            }
        }
        for( std::vector< std::string >::const_iterator it = t_devs_to_remove.begin(); it != t_devs_to_remove.end(); ++it )
        {
            t_dev_node->remove( *it );
        }


        if( t_client_node != NULL )
        {
            t_acq_req->set_client_commit( t_client_node->get_value( "commit", "N/A" ) );
            t_acq_req->set_client_exe( t_client_node->get_value( "exe", "N/A" ) );
            t_acq_req->set_client_version( t_client_node->get_value( "version", "N/A" ) );
        }
        else
        {
            t_acq_req->set_client_commit( "N/A" );
            t_acq_req->set_client_exe( "N/A" );
            t_acq_req->set_client_version( "N/A" );
        }

        t_acq_req->set_status( acq_request::acknowledged );

        MTINFO( mtlog, "Queuing request" );
        enqueue( t_acq_req );

        a_pkg.f_reply_node.node_at( "content" )->merge( *t_acq_req );
        a_pkg.f_reply_node.node_at( "content" )->add( "status-meaning", new param_value( acq_request::interpret_status( t_acq_req->get_status() ) ) );
        if( ! a_pkg.send_reply( R_SUCCESS, "Run request succeeded" ) )
        {
            MTWARN( mtlog, "Failed to send reply regarding the run request" );
        }

       // if the queue condition is waiting, release it
        if( f_queue_condition->is_waiting() == true )
        {
            //MTINFO( mtlog, "releasing queue condition" );
            f_queue_condition->release();
        }

        return true;
    }

    bool acq_request_db::handle_get_acq_status_request( const param_node& a_msg_payload, const std::string& /*a_mantis_routing_key*/, request_reply_package& a_pkg )
    {
        if( ! a_msg_payload.has( "values" ) || ! a_msg_payload[ "values" ].is_array() )
        {
            a_pkg.send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Invalid payload: no <values> array present" );
            return false;
        }
        string t_acq_id_str = a_msg_payload.array_at( "values" )->get_value( 0 );

        if( t_acq_id_str.empty() )
        {
            a_pkg.send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Invalid/no acquisition id" );
            return false;
        }

        boost::uuids::uuid t_id;
        std::stringstream t_conv;
        t_conv << t_acq_id_str;
        t_conv >> t_id;
        MTDEBUG( mtlog, "Requesting status of acquisition <" << t_id << ">" );

        const acq_request* t_request = get_acq_request( t_id );
        if( t_request == NULL )
        {
            a_pkg.send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Did not find acquisition <" + to_string( t_id ) + ">" );
            return false;
        }

        a_pkg.f_reply_node.node_at( "content" )->merge( *t_request );
        a_pkg.f_reply_node.node_at( "content" )->add( "status-meaning", new param_value( acq_request::interpret_status( t_request->get_status() ) ) );

        return a_pkg.send_reply( R_SUCCESS, "Acquisition status request succeeded" );
    }


} /* namespace mantis */
