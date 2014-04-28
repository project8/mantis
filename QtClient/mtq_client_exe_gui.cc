#include "mtq_client_exe_gui.hh"
#include "ui_client_exe_gui.h"

#include "mtq_simple_file_dialog.hh"
#include "mtq_full_file_dialog.hh"
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
    f_config_edited( false ),
    f_use_simple_egg_filename_dialog( true )
{
    ui->setupUi( this );
    ui->ConfigFilename->setText( f_default_filename_text );

    // close the description
    on_DescriptionButton_toggled( false );

    // connect all config changes to set_file_edited()
    // Run
    QObject::connect( ui->EggFilename, SIGNAL(textChanged(const QString&)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->AcquisitionRate, SIGNAL(valueChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->Duration, SIGNAL(valueChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->Mode, SIGNAL(valueChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->FileWriter, SIGNAL(currentIndexChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->Description, SIGNAL(textChanged()), this, SLOT(set_file_edited()) );
    QObject::connect( ui->EggFilenameEnable, SIGNAL(clicked()), this, SLOT(set_file_edited()) );
    QObject::connect( ui->AcquisitionRateEnable, SIGNAL(clicked()), this, SLOT(set_file_edited()) );
    QObject::connect( ui->DurationEnable, SIGNAL(clicked()), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ModeEnable, SIGNAL(clicked()), this, SLOT(set_file_edited()) );
    QObject::connect( ui->FileWriterEnable, SIGNAL(clicked()), this, SLOT(set_file_edited()) );
    QObject::connect( ui->DescriptionEnable, SIGNAL(clicked()), this, SLOT(set_file_edited()) );
    // Network
    QObject::connect( ui->ServerAddress, SIGNAL(textChanged(const QString&)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ServerPort, SIGNAL(valueChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ClientAddress, SIGNAL(textChanged(const QString&)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ClientPort, SIGNAL(valueChanged(int)), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ServerAddressEnable, SIGNAL(clicked()), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ServerPortEnable, SIGNAL(clicked()), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ClientAddressEnable, SIGNAL(clicked()), this, SLOT(set_file_edited()) );
    QObject::connect( ui->ClientPortEnable, SIGNAL(clicked()), this, SLOT(set_file_edited()) );

    QObject::connect( ui->ClearDescriptionButton, SIGNAL(clicked()), ui->Description, SLOT(clear()) );
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

void q_client_exe_gui::on_EggFilenameDialogButton_clicked()
{
    if( f_use_simple_egg_filename_dialog )
    {
        open_simple_egg_filename_dialog();
    }
    else
    {
        open_full_egg_filename_dialog();
    }
    return;
}

void q_client_exe_gui::open_simple_egg_filename_dialog()
{
    f_use_simple_egg_filename_dialog = true;

    simple_file_dialog dialog( this );
    QObject::connect( &dialog, SIGNAL(switch_file_dialog()), this, SLOT(open_full_egg_filename_dialog()) );
    dialog.set_filename( ui->EggFilename->text() );

    if( dialog.exec() )
    {
        ui->EggFilename->setText( dialog.get_filename() );
    }

    return;
}

void q_client_exe_gui::open_full_egg_filename_dialog()
{
    f_use_simple_egg_filename_dialog = false;

    full_file_dialog dialog( this );
    QObject::connect( &dialog, SIGNAL(switch_file_dialog()), this, SLOT(open_simple_egg_filename_dialog()) );
    dialog.setFileMode( QFileDialog::AnyFile );
    dialog.setViewMode( QFileDialog::Detail );
    dialog.setAcceptMode( QFileDialog::AcceptOpen );
    dialog.setNameFilter(tr("Egg (*.egg)"));
    dialog.selectFile( ui->EggFilename->text() );

    if( dialog.exec() )
    {
        QStringList fileList = dialog.selectedFiles();
        ui->EggFilename->setText( fileList.at( 0 ) );
    }

    return;
}

void q_client_exe_gui::on_DescriptionButton_toggled(bool checked)
{
    if( checked )
    {
        ui->Description->show();
        ui->DescriptionButton->setText( "Close" );
    }
    else
    {
        ui->Description->hide();
        ui->DescriptionButton->setText( "Open" );
    }

    return;
}

void q_client_exe_gui::on_runEnabledButton_stateChanged( int state )
{
    if( state == Qt::Checked )
    {
        ui->runButton->setEnabled( true );
    }
    else if( state == Qt::Unchecked )
    {
        ui->runButton->setEnabled( false );
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
    if( ui->EggFilenameEnable->isChecked() )
        t_config->add( "file", t_value << ui->EggFilename->text().toStdString() );

    // acquisition rate
    if( ui->AcquisitionRateEnable->isChecked() )
        t_config->add( "rate", t_value << ui->AcquisitionRate->value() );

    // duration
    if( ui->DurationEnable->isChecked() )
        t_config->add( "duration", t_value << ui->Duration->value() );

    // run mode
    if( ui->ModeEnable->isChecked() )
        t_config->add( "mode", t_value << ui->Mode->value() );

    // file writer
    if( ui->FileWriterEnable->isChecked() )
        t_config->add( "file-writer", t_value << ui->FileWriter->currentText().toLower().toStdString() );

    // description
    if( ui->DescriptionEnable->isChecked() )
        t_config->add( "description", t_value << ui->Description->toPlainText().toStdString() );

    // server host
    if( ui->ServerAddressEnable->isChecked() )
        t_config->add( "host", t_value << ui->ServerAddress->text().toStdString() );

    // server port
    if( ui->ServerPortEnable->isChecked() )
        t_config->add( "port", t_value << ui->ServerPort->value() );

    // client host
    if( ui->ClientAddressEnable->isChecked() )
        t_config->add( "client-host", t_value << ui->ClientAddress->text().toStdString() );

    // client port
    if( ui->ClientPortEnable->isChecked() )
        t_config->add( "client-port", t_value << ui->ClientPort->value() );

    //TODO: impelement OtherConfig

    return t_config;
}

bool q_client_exe_gui::fromParam( const mantis::param_node* a_node )
{
    // filename
    if( a_node->has( "file" ) )
    {
        ui->EggFilename->setText( a_node->get_value( "file" ).c_str() );
        ui->EggFilenameEnable->setChecked( true );
    }
    else
    {
        ui->EggFilenameEnable->setChecked( false );
    }

    // acquisition rate
    if( a_node->has( "rate" ) )
    {
        ui->AcquisitionRate->setValue( a_node->get_value< unsigned >( "rate" ) );
        ui->AcquisitionRateEnable->setChecked( true );
    }
    else
    {
        ui->AcquisitionRateEnable->setChecked( false );
    }

    // duration
    if( a_node->has( "duration" ) )
    {
        ui->Duration->setValue( a_node->get_value< unsigned >( "duration" ) );
        ui->DurationEnable->setChecked( true );
    }
    else
    {
        ui->DurationEnable->setChecked( false );
    }

    // run mode
    if( a_node->has( "mode" ) )
    {
        ui->Mode->setValue( a_node->get_value< unsigned >( "mode" ) );
        ui->ModeEnable->setChecked( true );
    }
    else
    {
        ui->ModeEnable->setChecked( false );
    }

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
                ui->FileWriterEnable->setChecked( true );
                t_match_found = true;
                break;
            }
        }
        if( ! t_match_found )
        {
            ui->FileWriterEnable->setChecked( false );
            QMessageBox t_msg_box;
            t_msg_box.setText( "Invalid file writer: " + t_writer );
            t_msg_box.exec();
            return false;
        }
    }
    else
    {
        ui->FileWriterEnable->setChecked( false );
    }

    // run description
    if( a_node->has( "description" ) )
    {
        ui->Description->setPlainText( a_node->get_value( "description" ).c_str() );
        ui->DescriptionEnable->setChecked( true );
    }
    else
    {
        ui->DescriptionEnable->setChecked( false );
    }

    // server host
    if( a_node->has( "host" ) )
    {
        ui->ServerAddress->setText( a_node->get_value( "host" ).c_str() );
        ui->ServerAddressEnable->setChecked( true );
    }
    else
    {
        ui->ServerAddressEnable->setChecked( false );
    }

    // server port
    if( a_node->has( "port" ) )
    {
        ui->ServerPort->setValue( a_node->get_value< unsigned >( "port" ) );
        ui->ServerPortEnable->setChecked( true );
    }
    else
    {
        ui->ServerPortEnable->setChecked( false );
    }

    // client host
    if( a_node->has( "client-host" ) )
    {
        ui->ClientAddress->setText( a_node->get_value( "client-host" ).c_str() );
        ui->ClientAddressEnable->setChecked( true );
    }
    else
    {
        ui->ClientAddressEnable->setChecked( false );
    }

    // client port
    if( a_node->has( "client-port" ) )
    {
        ui->ClientPort->setValue( a_node->get_value< unsigned >( "client-port" ) );
        ui->ClientPortEnable->setChecked( true );
    }
    else
    {
        ui->ClientPortEnable->setChecked( false );
    }

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
