#ifndef MTQ_FILENAME_DIALOG_H
#define MTQ_FILENAME_DIALOG_H

#include <QDialog>

namespace Ui {
    class filename_dialog;
}

class simple_file_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit simple_file_dialog(QWidget *parent = 0);
    ~simple_file_dialog();

    QString get_filename() const;

public slots:
    void set_filename(const QString& file);

private slots:
    void on_fileDialogButton_clicked();

signals:
    void switch_file_dialog();

private:
    Ui::filename_dialog *ui;
};

#endif // MTQ_FILENAME_DIALOG_H
