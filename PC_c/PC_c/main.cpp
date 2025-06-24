#include "mainwindow.h"
#include "mymqttclient.h"
#include "publishheader.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    mymqttclient * m = new mymqttclient;
    MainWindow w;
    w.show();
//    m->resolveHostname(HOSTNAME);
//    m->mymqttInit(m->IoTDAIPAddress,1883);
//    qDebug() << "Qt SSL 支持：" << QSslSocket::supportsSsl();

    return a.exec();
}
