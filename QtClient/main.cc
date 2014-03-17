#include <QtGui/QApplication>
#include "mtq_client_exe_gui.hh"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    q_client_exe_gui w;
    w.show();

    return a.exec();
}
