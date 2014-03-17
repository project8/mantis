#include "mtq_client_exe_gui.hh"
#include "ui_client_exe_gui.h"

#include "mtq_single_client_window.hh"

#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QTextStream>

#include "mt_param.hh"

q_client_exe_gui::q_client_exe_gui( QWidget *parent ) :
    QWidget( parent ),
    ui( new Ui::client_exe_gui ),
    f_default_filename_text( "[filename]" ),
    f_config_edited( false )
{
    ui->setupUi( this );
    ui->ConfigFilename->setText( f_default_filename_text );

    QObject::connect( ui->EggFilename, SIGNAL(textChanged(const QString&)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->AcquisitionRate, SIGNAL(valueChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->Duration, SIGNAL(valueChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->Mode, SIGNAL(valueChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->FileWriter, SIGNAL(currentIndexChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ServerAddress, SIGNAL(textChanged(const QString&)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ServerPort, SIGNAL(valueChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ClientAddress, SIGNAL(textChanged(const QString&)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ClientPort, SIGNAL(valueChanged(int)), this, SLOT(set_file_edited()) );
}

q_client_exe_gui::~q_client_exe_gui()
{
    delete ui;
}

void q_client_exe_gui::on_buttonNew_clicked()
{
    ui->ConfigFilename->setText( f_default_filename_text );
    this->set_file_not_edited();
    return;
}

void q_client_exe_gui::on_buttonOpen_clicked()
{
    QFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setViewMode( QFileDialog::Detail );
    dialog.setAcceptMode( QFileDialog::AcceptOpen );

    if( ui->ConfigFilename->text() != f_default_filename_text && ! ui->ConfigFilename->text().isEmpty() )
    {
        dialog.selectFile( ui->ConfigFilename->text() );
    }

    if( dialog.exec() )
    {
        QStringList fileList = dialog.selectedFiles();
        if( this->fromConfigFile( fileList.at( 0 ) ) )
        {
            ui->ConfigFilename->setText( fileList.at( 0 ) );
            this->set_file_not_edited();
        }
        else
        {
            QMessageBox t_msg_box;
            t_msg_box.setText( "Unable to open file: " + fileList.at( 0 ) );
            t_msg_box.exec();
        }
    }

    return;
}

void q_client_exe_gui::on_buttonSave_clicked()
{
    if( ui->ConfigFilename->text() == f_default_filename_text || ui->ConfigFilename->text().isEmpty() )
    {
        this->on_buttonSaveAs_clicked();
        return;
    }

    if( ! this->toConfigFile( ui->ConfigFilename->text() ) )
    {
        QMessageBox t_msg_box;
        t_msg_box.setText( "Unable to save file: " + ui->ConfigFilename->text() );
        t_msg_box.exec();
        return;
    }

    this->set_file_not_edited();

    return;
}

void q_client_exe_gui::on_buttonSaveAs_clicked()
{
    QFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::AnyFile );
    dialog.setViewMode( QFileDialog::Detail );
    dialog.setAcceptMode( QFileDialog::AcceptSave );

    if( ui->ConfigFilename->text() != f_default_filename_text && ! ui->ConfigFilename->text().isEmpty() )
    {
        dialog.selectFile( ui->ConfigFilename->text() );
    }

    if( dialog.exec() )
    {
        QStringList fileList = dialog.selectedFiles();
        if( this->toConfigFile( fileList.at( 0 ) ) )
        {
            ui->ConfigFilename->setText( fileList.at( 0 ) );
            this->set_file_not_edited();
        }
        else
        {
            QMessageBox t_msg_box;
            t_msg_box.setText( "Unable to save file: " + fileList.at( 0 ) );
            t_msg_box.exec();
        }
    }

    return;
}


void q_client_exe_gui::on_runButton_clicked()
{
    if( ui->EggFilename->text().isEmpty() )
    {
        QMessageBox t_msg_box;
        t_msg_box.setText( "Please provide an egg filename" );
        t_msg_box.exec();
        return;
    }

    if( ui->ServerAddress->text().isEmpty() )
    {
        QMessageBox t_msg_box;
        t_msg_box.setText( "Please provide an server host" );
        t_msg_box.exec();
        return;
    }

    if( ui->FileWriter->currentText().toLower() == QString( "client" ) && ui->ClientAddress->text().isEmpty() )
    {
        QMessageBox t_msg_box;
        t_msg_box.setText( "Please provide an client host" );
        t_msg_box.exec();
        return;
    }

    mantis::param_node* t_config = toParam();

    q_single_client_window* newClient = new q_single_client_window();
    newClient->show();

    newClient->start_client( t_config );

    delete t_config;
    return;
}

bool q_client_exe_gui::toConfigFile( const QString& filename )
{
    mantis::param_node* t_config = toParam();

    mantis::param_output_json::write_file( *t_config, filename.toStdString(), mantis::param_output_json::k_pretty );

    delete t_config;
    return true;
}

bool q_client_exe_gui::fromConfigFile( const QString& filename )
{
    mantis::param_node* t_config = mantis::param_input_json::read_file( filename.toStdString() );

    bool t_return = fromParam( t_config );

    delete t_config;
    return t_return;
}

mantis::param_node* q_client_exe_gui::toParam()
{
    mantis::param_value t_value;
    mantis::param_node* t_config = new mantis::param_node();

    // filename
    t_config->add( "file", t_value << ui->EggFilename->text().toStdString() );

    // acquisition rate
    t_config->add( "rate", t_value << ui->AcquisitionRate->value() );

    // duration
    t_config->add( "duration", t_value << ui->Duration->value() );

    // run mode
    t_config->add( "mode", t_value << ui->Mode->value() );

    // file writer
    t_config->add( "file-writer", t_value << ui->FileWriter->currentText().toLower().toStdString() );

    // server host
    t_config->add( "host", t_value << ui->ServerAddress->text().toStdString() );

    // server port
    t_config->add( "port", t_value << ui->ServerPort->value() );

    // client host
    t_config->add( "client-host", t_value << ui->ClientAddress->text().toStdString() );

    // client port
    t_config->add( "client-port", t_value << ui->ClientPort->value() );

    //TODO: impelement OtherConfig

    return t_config;
}

bool q_client_exe_gui::fromParam( const mantis::param_node* a_node )
{
    // filename
    ui->EggFilename->setText( a_node->get_value( "file", ui->EggFilename->text().toStdString() ).c_str() );

    // acquisition rate
    ui->AcquisitionRate->setValue( a_node->get_value< unsigned >( "rate", ui->AcquisitionRate->value() ) );

    // duration
    ui->Duration->setValue( a_node->get_value< unsigned >( "duration", ui->Duration->value() ) );

    // run mode
    ui->Mode->setValue( a_node->get_value< unsigned >( "mode", ui->Mode->value() ) );

    // file writer
    if( a_node->has( "file-writer" ) )
    {
        QString t_writer( a_node->get_value( "file-writer" ).c_str() );
        int t_count = ui->FileWriter->count();
        bool t_match_found = false;
        for( int t_index = 0; t_index < t_count; ++t_index )
        {
            if( t_writer == ui->FileWriter->itemText( t_index ).toLower() )
            {
                ui->FileWriter->setCurrentIndex( t_index );
                t_match_found = true;
                break;
            }
        }
        if( ! t_match_found )
        {
            QMessageBox t_msg_box;
            t_msg_box.setText( "Invalid file writer: " + t_writer );
            t_msg_box.exec();
            return false;
        }
    }

    // server host
    ui->ServerAddress->setText( a_node->get_value( "host", ui->ServerAddress->text().toStdString() ).c_str() );

    // server port
    ui->ServerPort->setValue( a_node->get_value< unsigned >( "port", ui->ServerPort->value() ) );

    // client host
    ui->ClientAddress->setText( a_node->get_value( "client-host", ui->ClientAddress->text().toStdString() ).c_str() );

    // client port
    ui->ClientPort->setValue( a_node->get_value< unsigned >( "client-port", ui->ClientPort->value() ) );

    //TODO: impelement OtherConfig

    return true;
}

void q_client_exe_gui::set_file_edited()
{
    if( ui->EggFilename->text() != this->f_default_filename_text && ! ui->EggFilename->text().isEmpty() )
    {
        f_config_edited = true;
        ui->EditedFlag->setText( "- edited" );
    }
    return;
}

void q_client_exe_gui::set_file_not_edited()
{
    f_config_edited = false;
    ui->EditedFlag->setText( " " );
    return;
}
