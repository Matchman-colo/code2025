#ifndef MYMQTTCLIENT_H
#define MYMQTTCLIENT_H
#include <QObject>
#include <QtMqtt/qmqttclient.h>
#include <QtMqtt/qmqttmessage.h>
#include <QtMqtt/qmqtttopicname.h>
#include <QHostInfo>
#include <QHostAddress>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "publishheader.h"
#include <QSslConfiguration>
#include <QMqttClient>

class mymqttclient : public QObject
{
    Q_OBJECT
public:
    mymqttclient();
    void mymqttInit(QString domainName, quint16 Port);
    void connectTOHuaWeiIOT(QString domainName, quint16 Port);
    void subPushTOHuaWeiIOT(QString pushContent);
    void disConnectTOHuaWeiIOT();
    void resolveHostname(const QString &hostname);
    void queryDeviceProperty(const QString &serviceId);
    QString IoTDAIPAddress;
private:
    QMqttClient* mqttclient;
    QByteArray *msg;

};

#endif // MYMQTTCLIENT_H
