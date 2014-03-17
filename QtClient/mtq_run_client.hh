#ifndef MTQ_RUN_CLIENT_HH
#define MTQ_RUN_CLIENT_HH

#include <QObject>

#include <QThread>

namespace mantis
{
    class param_node;
}

class q_single_client_window;

class q_run_client : public QObject
{
    Q_OBJECT

public:
    explicit q_run_client( QObject *parent = 0 );

    virtual ~q_run_client();

    void copy_config( const mantis::param_node* a_config );

    void set_window( q_single_client_window* a_window );

private:
    QThread f_run_client_thread;

    mantis::param_node* f_config;

    q_single_client_window* f_window;

signals:
    void run_complete( int a_return );

public slots:
    void do_run();

};

#endif // MTQ_RUN_CLIENT_HH
