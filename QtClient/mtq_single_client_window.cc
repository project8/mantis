#include "mtq_single_client_window.hh"
#include "ui_single_client_window.h"

#include "mtq_run_client.hh"

#include <signal.h>
#include <sstream>

q_single_client_window::q_single_client_window( QWidget *parent ) :
    QWidget( parent ),
    ui( new Ui::single_client_window ),
    f_client_thread(),
    f_running( false ),
    f_in_use( false ),
    f_mutex()
{
    ui->setupUi( this );

    QFont outputFont( "Courier" );
    ui->stdoutBox->setFont( outputFont );
    fErrFormat.setForeground( QBrush( Qt::red ) );
}

q_single_client_window::~q_single_client_window()
{
    delete ui;
}

void q_single_client_window::on_cancelButton_clicked()
{
    f_mutex.lock();
    if( f_running.load() )
    {
        // terminate
        ::raise( SIGINT );
        f_running.store( false );
    }
    f_mutex.unlock();
    return;
}

void q_single_client_window::on_closeButton_clicked()
{
    if( f_running.load() )
    {
        on_cancelButton_clicked();
    }
    this->close();
    return;
}

void q_single_client_window::start_client( const mantis::param_node* a_node )
{
    if( f_in_use.load() )
    {
        return;
    }
    f_in_use.store( true );

    // prepare the thread
    q_run_client* t_run_client = new q_run_client();
    t_run_client->set_window( this );
    t_run_client->copy_config( a_node );
    t_run_client->moveToThread( &f_client_thread );

    // connect signal/slots
    QObject::connect( &f_client_thread, SIGNAL( started() ),
                      t_run_client, SLOT( do_run() ) );
//    QObject::connect( &f_client_thread, SIGNAL( finished() ),
//                      t_run_client, SLOT( delete_later() ) );
    QObject::connect( t_run_client, SIGNAL( run_complete(int) ),
                      this, SLOT( run_finished(int) ) );

    f_mutex.lock();
    f_running.store( true );

    f_client_thread.start();

    f_mutex.unlock();

/*
    QTemporaryFile t_buffer_file;
    if( ! t_buffer_file.open() )
    {
        QMessageBox t_msg_box;
        t_msg_box.setText( "Unable to buffer config" );
        t_msg_box.exec();
        return;
    }
    QTextStream t_buffer_str( &t_buffer_file );
    t_buffer_str << a_node;
    t_buffer_file.flush();

    ui->stdoutBox->appendPlainText( QString( t_buffer_file.fileName() ) );

    QString t_program = "/Users/nsoblath/My_Documents/Project_8/mantis/build-templatedMantis/bin/mantis_client";
    QStringList t_arguments;
    t_arguments << "config=" + t_buffer_file.fileName();

    f_client_proc = new QProcess(this);

    ui->cancelButton->setText( "Cancel" );
    QObject::connect(ui->cancelButton, SIGNAL( clicked(void) ) ,
                         f_client_proc, SLOT( terminate(void) ) );
    QObject::connect(f_client_proc, SIGNAL( finished(int) ),
                     this, SLOT( process_finished(int) ) );

    f_client_proc->start(t_program, t_arguments);
    //ofstream t_buffer_file("client_config_buffer.json");
    //ui->stdoutBox->appendPlainText( QString( a_node->to_string().c_str() ) );
*/
    return;
}

void q_single_client_window::print_std_message( const QString& a_msg )
{
    ui->stdoutBox->setCurrentCharFormat( fStdFormat );
    ui->stdoutBox->append( a_msg );
    return;
}

void q_single_client_window::print_err_message( const QString& a_msg )
{
    ui->stdoutBox->setCurrentCharFormat( fErrFormat );
    //QTextEdit will terminate the msg with a ParagraphSeparator, but it also replaces
    //all newlines with ParagraphSeparators. By replacing the newlines by ourself, one
    //error msg will be one paragraph.
    QString paragraph( a_msg );
    paragraph.replace( QLatin1Char('\n'), QChar::LineSeparator );
    ui->stdoutBox->append( paragraph );
    return;
}

void q_single_client_window::run_finished( int a_exit_code )
{
    // not using the mutex here because f_running is an atomic bool, and no other operations are used
    f_running.store( false );
    std::stringstream t_exit_msg;
    t_exit_msg << "mantis_client finished with return code " << a_exit_code;
    print_std_message( QString( t_exit_msg.str().c_str() ) );
    return;
}

