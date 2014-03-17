#include "mtq_run_client.hh"
#include "mtq_signaling_streambuf.hh"
#include "mtq_single_client_window.hh"

#include "mt_logger.hh"
#include "mt_param.hh"
#include "mt_run_client.hh"

#include <iostream>

MTLOGGER( mtlog, "q_run_client" );

q_run_client::q_run_client( QObject *parent ) :
    QObject( parent ),
    f_config( NULL ),
    f_window( NULL )
{
}

q_run_client::~q_run_client()
{
    mantis::logger::SetOutStream( &std::cout );
    mantis::logger::SetErrStream( &std::cerr );
}

void q_run_client::copy_config( const mantis::param_node* a_config )
{
    f_config = new mantis::param_node( *a_config );
    return;
}

void q_run_client::set_window( q_single_client_window* a_window )
{
    f_window = a_window;
    return;
}

void q_run_client::do_run()
{
    try
    {
        if( f_window != NULL )
        {
            q_signaling_streambuf* t_sb_cout = new q_signaling_streambuf( this );
            q_signaling_streambuf* t_sb_cerr = new q_signaling_streambuf( this );

            QObject::connect( t_sb_cout, SIGNAL( print_message(const QString&) ),
                              f_window, SLOT( print_std_message(const QString&) ) );
            QObject::connect( t_sb_cerr, SIGNAL( print_message(const QString&) ),
                              f_window, SLOT( print_err_message(const QString&) ) );

            mantis::logger::SetOutStream( new std::ostream( t_sb_cout ) );
            mantis::logger::SetErrStream( new std::ostream( t_sb_cerr ) );

            mantis::logger::SetColored( false );
        }

        MTINFO( mtlog, "Final configuration:\n" << *f_config );

        mantis::run_client the_client( f_config );

        the_client.execute();

        emit run_complete( the_client.get_return() );
    }
    catch( mantis::exception& e )
    {
        MTERROR( mtlog, "mantis::exception caught: " << e.what() );
    }
    catch( std::exception& e )
    {
        MTERROR( mtlog, "std::exception caught: " << e.what() );
    }

    emit run_complete( RETURN_ERROR );
}
