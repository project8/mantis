#ifndef MTQ_CLIENT_EXE_GUI_HH
#define MTQ_CLIENT_EXE_GUI_HH

#include <QString>
#include <QWidget>

namespace mantis
{
    class param_node;
}

namespace Ui {
    class client_exe_gui;
}

class q_client_exe_gui : public QWidget
{
    Q_OBJECT

public:
    explicit q_client_exe_gui( QWidget *parent = 0 );
    ~q_client_exe_gui();

private slots:
    void on_buttonNew_clicked();
    void on_buttonOpen_clicked();
    void on_buttonSave_clicked();
    void on_buttonSaveAs_clicked();

    void on_EggFilenameDialogButton_clicked();
    void open_simple_egg_filename_dialog();
    void open_full_egg_filename_dialog();

    void on_DescriptionButton_toggled(bool checked);

    void on_runEnabledButton_stateChanged(int state);

    void on_runButton_clicked();

    void set_file_edited();
    void set_file_not_edited();

private:
    bool toConfigFile( const QString& filename );
    bool fromConfigFile( const QString& filename );

    mantis::param_node* toParam();
    bool fromParam( const mantis::param_node* a_node );

    Ui::client_exe_gui *ui;

    QString f_default_filename_text;
    bool f_config_edited;

    bool f_use_simple_egg_filename_dialog;
};

#endif // MTQ_CLIENT_EXE_GUI_HH
