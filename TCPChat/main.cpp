#include "tcp.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TCP tcp;
    tcp.show();
    return a.exec();
}
