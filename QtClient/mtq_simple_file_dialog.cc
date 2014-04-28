#include "mtq_simple_file_dialog.hh"
#include "ui_filename_dialog.h"

simple_file_dialog::simple_file_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::filename_dialog)
{
    ui->setupUi(this);

    this->setWindowTitle( "Egg File" );

    QObject::connect( ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()) );

    QObject::connect( ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()) );
}

simple_file_dialog::~simple_file_dialog()
{
    delete ui;
}

QString simple_file_dialog::get_filename() const
{
    return ui->filenameBox->toPlainText();
}

void simple_file_dialog::set_filename(const QString& file)
{
    ui->filenameBox->setPlainText(file);
}


void simple_file_dialog::on_fileDialogButton_clicked()
{
    reject();
    emit switch_file_dialog();
}
