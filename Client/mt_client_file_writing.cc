/*
 * mt_client_file_writing.cc
 *
 *  Created on: Nov 26, 2013
 *      Author: nsoblath
 */

#include "mt_client_file_writing.hh"

#include "mt_buffer.hh"
#include "mt_client_worker.hh"
#include "mt_condition.hh"
#include "mt_exception.hh"
#include "mt_file_writer.hh"
#include "mt_logger.hh"
#include "mt_record_receiver.hh"
#include "mt_run_context_dist.hh"
#include "mt_run_description.hh"
#include "mt_server_tcp.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"

namespace mantis
{
    MTLOGGER( mtlog, "client_file_writing" );

    client_file_writing::client_file_writing( const param_node* a_config, run_context_dist* a_run_context, int a_write_port ) :
            f_config( a_config ),
            f_server( NULL ),
            f_buffer_condition( NULL ),
            f_buffer( NULL ),
            f_receiver( NULL ),
            f_writer( NULL ),
            f_worker( NULL ),
            f_thread( NULL )
    {
        // objects for receiving and writing data
        try
        {
            f_server = new server_tcp( a_write_port );
        }
        catch( exception& e)
        {
            a_run_context->lock_client_status_out()->set_state( client_status_state_t_error );
            a_run_context->push_client_status_no_mutex();
            a_run_context->unlock_outbound();
            throw exception() << "unable to create record-receiver server: " << e.what();
        }

        status* t_status = a_run_context->lock_status_in();

        f_buffer_condition = new condition();
        f_buffer = new buffer( t_status->buffer_size(), t_status->record_size() );

        f_receiver = new record_receiver( f_server );
        f_receiver->set_data_chunk_size( t_status->data_chunk_size() );
        f_receiver->set_data_type_size( t_status->data_type_size() );
        f_receiver->allocate( f_buffer, f_buffer_condition );

        run_description* t_run_desc = new run_description();
        t_run_desc->set_mantis_server_exe( t_status->server_exe() );
        t_run_desc->set_mantis_server_version( t_status->server_version() );
        t_run_desc->set_mantis_server_commit( t_status->server_commit() );
        param_node* t_server_config = param_input_json::read_string( t_status->server_config() );
        t_run_desc->set_server_config( *t_server_config );

        a_run_context->unlock_inbound();

        f_writer = new file_writer();
        f_writer->set_buffer( f_buffer, f_buffer_condition );
        f_writer->set_run_description( t_run_desc );
        f_writer->configure( f_config );

        try
        {
            f_worker = new client_worker( a_run_context->lock_request_out(), f_receiver, f_writer, f_buffer_condition );
        }
        catch( exception& e )
        {
            a_run_context->unlock_outbound();
            a_run_context->lock_client_status_out()->set_state( client_status_state_t_error );
            a_run_context->push_client_status_no_mutex();
            a_run_context->unlock_outbound();
            throw exception() << "unable to create client's writer: " << e.what();
        }
        a_run_context->unlock_outbound();

        f_thread = new thread( f_worker );

        MTINFO( mtlog, "starting record receiver" );

        try
        {
            signal_handler t_sig_hand;
            t_sig_hand.push_thread( f_thread );

            f_thread->start();
        }
        catch( exception& e )
        {
            throw exception() << "unable to start record-receiving server";
        }
    }

    client_file_writing::~client_file_writing()
    {
        delete f_thread;
        delete f_worker;
        delete f_writer;
        delete f_receiver;
        delete f_buffer;
        delete f_buffer_condition;
        delete f_server;
    }

    void client_file_writing::wait_for_finish()
    {
        f_thread->join();
        signal_handler t_sig_hand;
        t_sig_hand.pop_thread();
        return;
    }

    void client_file_writing::cancel()
    {
        f_worker->cancel();
        return;
    }

    bool client_file_writing::is_done()
    {
        return f_worker->is_done();
    }

} /* namespace mantis */
