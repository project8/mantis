#include "mtq_full_file_dialog.hh"

#include <QGridLayout>

full_file_dialog::full_file_dialog(QWidget *parent, Qt::WindowFlags flags) :
    QFileDialog( parent, flags )
{
    AddDialogSwitchButton();
}

full_file_dialog::full_file_dialog( QWidget * parent, const QString & caption, const QString & directory, const QString & filter ) :
        QFileDialog( parent, caption, directory, filter )
{
    AddDialogSwitchButton();
}

void full_file_dialog::AddDialogSwitchButton()
{
    QGridLayout* mainLayout = dynamic_cast< QGridLayout* >( layout() );
    Q_ASSERT( mainLayout );

    f_switch_dialog_button = new QPushButton( "Simple File Dialog ...", this );
    QObject::connect( f_switch_dialog_button, SIGNAL(clicked()), this, SLOT(on_fileDialogButton_clicked()) );

    mainLayout->addWidget( f_switch_dialog_button, mainLayout->rowCount(), 0, 1, -1, Qt::AlignRight );

    return;
}

void full_file_dialog::on_fileDialogButton_clicked()
{
    reject();
    emit switch_file_dialog();
}

