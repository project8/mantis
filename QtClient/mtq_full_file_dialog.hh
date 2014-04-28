#ifndef MTQ_FULL_FILE_DIALOG_HH
#define MTQ_FULL_FILE_DIALOG_HH

#include <QFileDialog>
#include <QPushButton>

class full_file_dialog : public QFileDialog
{
    Q_OBJECT
public:
    explicit full_file_dialog( QWidget *parent, Qt::WindowFlags flags );
    explicit full_file_dialog( QWidget * parent = 0, const QString & caption = QString(), const QString & directory = QString(), const QString & filter = QString() );

private slots:
    void on_fileDialogButton_clicked();

signals:
    void switch_file_dialog();

private:
    void AddDialogSwitchButton();

    QPushButton* f_switch_dialog_button;

};

#endif // MTQ_FULL_FILE_DIALOG_HH
