#include "mymqttclient.h"
#include <QMessageBox>
mymqttclient::mymqttclient()
{
    mqttclient = new QMqttClient;
}
void mymqttclient::mymqttInit(QString domainName, quint16 Port)
{
    if(mqttclient->state() != QMqttClient::Connected)
    {
        mqttclient->setHostname(domainName);
        mqttclient->setPort(Port);
        mqttclient->setClientId(CLIENTID);
        mqttclient->setUsername(USERNAME);
        mqttclient->setPassword(PASSWORD);
        mqttclient->keepAlive();
        mqttclient->cleanSession();
//        mqttclient->setTransport(QMqttClient::SecureTransport);
        mqttclient->connectToHost();
        qDebug()<<mqttclient->state()<<":"<<"表示连接成功！！！";
        connect(mqttclient, &QMqttClient::errorChanged, [](QMqttClient::ClientError error) {
            qDebug() << "MQTT 连接错误：" << error;
        });



        qDebug()<<mqttclient->state()<<":"<<"表示连接成功！！！";
    }
    else
    {
        mqttclient->disconnectFromHost();
        qDebug()<<"连接失败";
    }
}
void mymqttclient::connectTOHuaWeiIOT(QString domainName, quint16 Port)
{
    mymqttInit(domainName,Port);
}
void mymqttclient::disConnectTOHuaWeiIOT()
{
    mqttclient->disconnectFromHost();
    qDebug()<<"已与华为云断连";
}
void mymqttclient::subPushTOHuaWeiIOT(QString pushContent)
{
    QByteArray qba;
    qba.append(pushContent.toUtf8());
    qDebug()<<pushContent;
    if(mqttclient->state() == QMqttClient::Connected)
    {
        if(mqttclient->publish(QMqttTopicName(TOPIC),qba))
        {
            qDebug()<<"发布失败";
        }
        else
        {
            qDebug()<<"发布成功";
        }
    }
    else
    {
        qDebug()<<mqttclient->state();
        qDebug()<<"mqtt未连接";
    }
}

void mymqttclient::resolveHostname(const QString &hostname)
{
    QHostInfo info = QHostInfo::fromName(hostname);
    foreach (const QHostAddress &address, info.addresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            IoTDAIPAddress = address.toString();
            qDebug() << "IPv4 address:" << IoTDAIPAddress;
        } else if (address.protocol() == QAbstractSocket::IPv6Protocol)
        {
            qDebug() << "IPv6 address:" << address.toString();
        }
    }

}

void mymqttclient::queryDeviceProperty(const QString &serviceId)
{

    // 生成 request_id
    QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // 构造 Topic
    QString topic = QString("$oc/devices/%1/sys/properties/get/request_id=%2").arg(USERNAME, requestId);

    // 构造 JSON 消息
    QJsonObject json;
    json["service_id"] = serviceId;
    QJsonDocument doc(json);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);

    // 发布消息
    auto pub = mqttclient->publish(topic, payload);
    if (!pub)
    {
        qWarning() << "获取属性请求失败";
    }
    else
    {
        qDebug() << "请求发送: " << payload;
    }
}

