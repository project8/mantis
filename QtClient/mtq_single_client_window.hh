#ifndef MTQ_SINGLE_CLIENT_WINDOW_HH
#define MTQ_SINGLE_CLIENT_WINDOW_HH

#include <QWidget>

#include <QProcess>
#include <QString>
#include <QTextCharFormat>
#include <QThread>

#include "mt_atomic.hh"
#include "mt_mutex.hh"
#include "mt_param.hh"

namespace Ui {
    class single_client_window;
}

class q_single_client_window : public QWidget
{
    Q_OBJECT

public:
    explicit q_single_client_window( QWidget *parent = 0 );
    ~q_single_client_window();

    void start_client( const mantis::param_node* a_node );

private:
    QTextCharFormat fStdFormat;
    QTextCharFormat fErrFormat;

private slots:
    void on_cancelButton_clicked();
    void on_closeButton_clicked();

public slots:
    void print_std_message( const QString& a_msg );
    void print_err_message( const QString& a_msg );

    void run_finished( int a_exit_code );

private:
    Ui::single_client_window *ui;

    QThread f_client_thread;

    mantis::atomic_bool f_running;
    mantis::atomic_bool f_in_use;

    mantis::mutex f_mutex;
};

#endif // MTQ_SINGLE_CLIENT_WINDOW_HH
