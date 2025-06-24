#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cleanlogwindow.h"
#include "settingwindow.h"
#include "mymqttclient.h"
#include <QNetworkRequest>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,messageSent(false)
    ,timer(nullptr)
    ,manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    this->setWindowTitle(QStringLiteral("“尘卫士”——智能光伏板清扫系统"));
    connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::replyFinished);

    pbuttonGroup = new QButtonGroup(this);
    pbuttonGroup->addButton(ui->btn_0,0);
    pbuttonGroup->addButton(ui->btn_2,2);
    pbuttonGroup->addButton(ui->btn_3,3);

    connect(ui->btn_0, SIGNAL(clicked()), this, SLOT(onRadioClickSlot()));
    connect(ui->btn_2, SIGNAL(clicked()), this, SLOT(onRadioClickSlot()));
    connect(ui->btn_3, SIGNAL(clicked()), this, SLOT(onRadioClickSlot()));

    temp_min = 0;
    temp_max = 100;
    humi_min = 0;
    humi_max = 100;
    illu_min = 0;
    illu_max = 100;
    start_t = QDateTime::currentDateTime();
    end_t = QDateTime::currentDateTime().addDays(1);


//    temp_max=0;
//    temp_min=0;
//    humi_max=0;
//    humi_min=0;
//    illu_max=0;
//    illu_min=0;

//    start_t = QDateTime::fromString("2025-02-20 7:00:00", "yyyy-MM-dd HH:mm:ss");
//    end_t = QDateTime::fromString("2025-02-20 17:00:00", "yyyy-MM-dd HH:mm:ss");

}

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
}


void MainWindow::on_logcheck_btn_clicked()
{
    CleanLogWindow *c = new CleanLogWindow();
    c->show();

}


void MainWindow::on_automode_btn_clicked()
{
    SettingWindow *s = new SettingWindow(this);
    s->show();
}

void MainWindow::on_token_btn_clicked()
{
    GetToken();
}


void MainWindow::on_get_btn_clicked()
{
    Get_device_properties();
}




void MainWindow::on_startauto_btn_clicked()
{
    qDebug()<<1;
    if (!timer) {
        timer = new QTimer(this);
    }

    // 确保每次都连接信号
    qDebug()<<2;
    if (!connect(timer, &QTimer::timeout, this, &MainWindow::checkTimeAndCondition)) {
        qDebug() << "连接失败！";  // 如果连接失败，输出错误信息
    }

    // 启动定时器
    qDebug()<<3;
    timer->start(5000);  // 每秒检查一次
    qDebug()<<4;

    // 手动调用一次检查函数（立即执行）
    checkTimeAndCondition();
}


void MainWindow::onRadioClickSlot()
{


    QString requestUrl;
    QNetworkRequest request;

    //设置请求地址
    QUrl url;

    //修改属性的地址
    requestUrl=URL_C;


    //设置数据提交格式
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

    //设置token
    request.setRawHeader("X-Auth-Token",Token); //Token是接口获取返回的的一串字符串

    //构造请求
    url.setUrl(requestUrl);

    request.setUrl(url);

    //打包请求参数赋值
    QString post_param_1=QString("{\"paras\":{\"clean_mode\":0},\"service_id\":\"ESP32C3\",\"command_name\":\"clean_mode\"}");
    QString post_param_3=QString("{\"paras\":{\"clean_mode\":2},\"service_id\":\"ESP32C3\",\"command_name\":\"clean_mode\"}");
    QString post_param_4=QString("{\"paras\":{\"clean_mode\":3},\"service_id\":\"ESP32C3\",\"command_name\":\"clean_mode\"}");


    switch(pbuttonGroup->checkedId())
    {
    case 0:
        manager->post(request, post_param_1.toUtf8());
        break;

    case 2:
        manager->post(request, post_param_3.toUtf8());
        break;
    case 3:
        manager->post(request, post_param_4.toUtf8());
        break;
    }
}






void MainWindow::GetToken()
{

    function_select = 3;

    if (!manager) {
        qDebug() << "Error: manager is NULL!";
        return;
    }

    QString requestUrl = QString("https://iam.%1.myhuaweicloud.com/v3/auth/tokens").arg(SERVERID);
    QUrl url(requestUrl);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 构造 JSON 请求
    QJsonObject authObj{
                        {"identity", QJsonObject{
                                                 {"methods", QJsonArray{"password"}},
                                                 {"password", QJsonObject{
                                                                          {"user", QJsonObject{
                                                                                               {"domain", QJsonObject{{"name", MAINUSER}}},
                                                                                               {"name", IAMUSER},
                                                                                               {"password", IAMPASSWORD}}}}}}},
                        {"scope", QJsonObject{
                                              {"project", QJsonObject{{"name", SERVERID}}}}}};

    QJsonObject rootObj{{"auth", authObj}};
    QByteArray requestData = QJsonDocument(rootObj).toJson();

    qDebug() << "请求URL：" << requestUrl;
        qDebug() << "请求数据：" << requestData;

                        // 发送 POST 请求
                        QNetworkReply *reply = manager->post(request, requestData);

    // 监听请求完成
    connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "请求失败：" << reply->errorString();
        } else {
            QByteArray token = reply->rawHeader("X-Subject-Token");
            qDebug() << "Token 获取成功：" << token;
        }
        reply->deleteLater();
    });

}


//查询设备属性
void MainWindow::Get_device_properties()
{
    //label_time
    QDateTime current_date_time =QDateTime::currentDateTime();

    QString current_date =current_date_time.toString("yyyy/MM/dd hh:mm:ss");
//    ui->label_time->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
//    ui->label_time->setText(current_date);

    function_select=0;

    QString requestUrl;
    QNetworkRequest request;

    //设置请求地址
    QUrl url;

    //获取token请求地址
    requestUrl = URL_S;

    //设置数据提交格式
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

    //设置token
    request.setRawHeader("X-Auth-Token",Token);

    //构造请求
    url.setUrl(requestUrl);

    request.setUrl(url);

    //发送请求
    manager->get(request);
    qDebug()<<"1";
}

//解析反馈结果
void MainWindow::replyFinished(QNetworkReply *reply)
{
    QString displayInfo;

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    //读取所有数据
    QByteArray replyData = reply->readAll();

    qDebug()<<"状态码:"<<statusCode;
    qDebug()<<"反馈的数据:"<<QString(replyData);

    //更新token
    if(function_select==3)
    {
        displayInfo="token 更新失败.";
        //读取HTTP响应头的数据
        QList<QNetworkReply::RawHeaderPair> RawHeader=reply->rawHeaderPairs();
        qDebug()<<"HTTP响应头数量:"<<RawHeader.size();
        qDebug()<<"RawHeader:"<<RawHeader;

        for(int i=0;i<RawHeader.size();i++)
        {
            QString first=RawHeader.at(i).first;
            QString second=RawHeader.at(i).second;
            if(first=="X-Subject-Token")
            {
                Token=second.toUtf8();
                displayInfo="token 更新成功.";

                //保存到文件
//                SaveDataToFile(Token);
                break;
            }
        }
        QMessageBox::information(this,"提示",displayInfo,QMessageBox::Ok,QMessageBox::Ok);
        return;
    }

    //判断状态码
    if(200 != statusCode)
    {
        //解析数据
        QJsonParseError json_error;
        QJsonDocument document = QJsonDocument::fromJson(replyData, &json_error);
        if(json_error.error == QJsonParseError::NoError)
        {
            //判断是否是对象,然后开始解析数据
            if(document.isObject())
            {
                QString error_str="";
                QJsonObject obj = document.object();
                QString error_code;
                //解析错误代码
                if(obj.contains("error_code"))
                {
                    error_code=obj.take("error_code").toString();

                    //身份验证失败
                    if(error_code=="IOTDA.000002")
                    {
                        //显示错误代码
                        QMessageBox::information(this,"提示","身份验证失败.\n先点击<更新Token>按钮.",QMessageBox::Ok,QMessageBox::Ok);
                        return;
                    }
                    //设备不在线
                    else if(error_code=="IOTDA.014016")
                    {
                        //显示错误代码
                        QMessageBox::information(this,"提示","设备未在线!\n无法下发命令.",QMessageBox::Ok,QMessageBox::Ok);
                        return;
                    }
                    else
                    {
                        error_str+="错误代码:";
                        error_str+=error_code;
                        error_str+="\n";
                    }
                }
                if(obj.contains("error_msg"))
                {
                    error_str+="错误消息:";
                    error_str+=obj.take("error_msg").toString();
                    error_str+="\n";
                }

                //显示错误代码
                QMessageBox::information(this,"提示",error_str,QMessageBox::Ok,QMessageBox::Ok);
            }
         }
        return;
    }

    //设置属性
    if(function_select==12 || function_select==13)
    {
        //解析数据
        QJsonParseError json_error;
        QJsonDocument document = QJsonDocument::fromJson(replyData, &json_error);
        if(json_error.error == QJsonParseError::NoError)
        {
            //判断是否是对象,然后开始解析数据
            if(document.isObject())
            {
                QJsonObject obj = document.object();
                if(obj.contains("response"))
                {
                    QJsonObject obj1=obj.take("response").toObject();
                    int val=0;
                    QString success;
                    if(obj1.contains("result_code"))
                    {
                         val=obj1.take("result_code").toInt();
                    }
                    if(obj1.contains("result_desc"))
                    {
                         success=obj1.take("result_desc").toString();
                    }

                    if(val==0 && success =="success")
                    {
                        //显示状态
                        QMessageBox::information(this,"提示","远程命令操作完成.",QMessageBox::Ok,QMessageBox::Ok);
                        return;
                    }
                    else
                    {
                        //显示状态
                        QMessageBox::information(this,"提示","设备未正确回应.请检查设备网络.",QMessageBox::Ok,QMessageBox::Ok);
                        return;
                    }
                }
            }
         }
    }

    //查询设备属性
    if(function_select==0)
    {
        //解析数据
        QJsonParseError json_error;
        QJsonDocument document = QJsonDocument::fromJson(replyData, &json_error);
        if(json_error.error == QJsonParseError::NoError)
        {
            //判断是否是对象,然后开始解析数据
            if(document.isObject())
            {
                QJsonObject obj = document.object();
                if(obj.contains("shadow"))
                {
                    QJsonArray array=obj.take("shadow").toArray();
                    for(int i=0;i<array.size();i++)
                    {
                        QJsonObject obj2=array.at(i).toObject();
                        if(obj2.contains("reported"))
                        {
                            QJsonObject obj3=obj2.take("reported").toObject();


                            if(obj3.contains("properties"))
                            {
                                //提取时间
                                QString event_time=obj3.take("event_time").toString();
                                qDebug()<<"event_time:"<<event_time;


                                // 解析时间字段为 QDateTime 对象
                                QDateTime dateTime = QDateTime::fromString(event_time, "yyyyMMddTHHmmssZ");

                                // 转换时区为北京时间（假设服务器下发的时间是UTC时间）
                                dateTime.setTimeSpec(Qt::UTC);
                                dateTime = dateTime.toLocalTime();

                                // 输出标准的时间格式
                                QString update_time=dateTime.toString("yyyy-MM-dd HH:mm:ss");

                                QJsonObject properties=obj3.take("properties").toObject();



                                qDebug()<<"开始解析数据....";


                                int temperature;
                                int humidity;
                                int illumination;
                                int clean_log;

                                //提取数据
                                temperature=properties.take("temperature").toInt();
                                humidity=properties.take("humidity").toInt();
                                illumination=properties.take("illumination").toInt();
                                clean_log=properties.take("cleaning_log").toInt();



                                ui->temp_label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                                ui->temp_label->setText(QString("%1").arg(temperature));

                                ui->illu_label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                                ui->illu_label->setText(QString("%1").arg(illumination));

                                ui->humi_label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                                ui->humi_label->setText(QString("%1").arg(humidity));

                                QDateTime dTime = QDateTime::fromString(event_time, "yyyyMMdd'T'HHmmss'Z'");
                                dTime = dTime.addSecs(8 * 3600);


                                // 转换为可读字符串
                                QString formattedTime = dTime.toString("yyyy-MM-dd HH:mm:ss");

//                                qDebug() << "本地时间:" << formattedTime;

                                if(c_time.isEmpty() || c_time!=formattedTime)
                                {
                                    QString file_1="./cleanlog_1.txt";
                                    QString file_2="./cleanlog_2.txt";
                                    c_time=formattedTime;
                                    QString log=c_time;

                                    if(clean_log==11)
                                    {
                                        appendToFile(file_1,log+"->检测到沙尘，已清扫");
                                    }
                                    else if(clean_log==10)
                                    {
                                        appendToFile(file_1,log+"->检测到干净，未清扫");
                                    }
                                    else if(clean_log==21)
                                    {
                                        appendToFile(file_2,log+"->检测到沙尘，已清扫");
                                    }
                                    else if(clean_log==20)
                                    {
                                        appendToFile(file_2,log+"->检测到干净，未清扫");
                                    }
                                    else
                                    {

                                    }

                                }


                            }
                        }
                    }
                }
            }
         }
        return;
    }
}

void MainWindow::SaveDataToFile(QByteArray token)
{
    QString filePath = "C:/Users/SkylarLiu/Desktop/token.txt"; // 目标文件

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "无法打开文件：" << filePath << "错误：" << file.errorString();
            return;
    }

    file.write(token); // 覆盖写入数据
    file.close();

    qDebug() << "Token 已成功写入：" << filePath;
}
void MainWindow::appendToFile(const QString &fileName, const QString &text)
{
    QFile file(fileName);

    // 打开文件，模式为追加（Append）
    if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);

            // 在文件中写入文本并换行
            out << text << "\n";  // "\n" 确保换行

            // 关闭文件
            file.close();
    } else {
            qDebug() << "无法打开文件";
    }
}

void MainWindow::onDataReceived(int temp_min1, int temp_max1, int humi_min1,int humi_max1,int illu_min1,int illu_max1,QDateTime start_t1,QDateTime end_t1)
{
    qDebug() << "接收参数: " << temp_min1<<temp_max1<<humi_min1<<humi_max1<<illu_min1<<illu_max1<<start_t1<<end_t1;


    temp_max=temp_max1;
    temp_min=temp_min1;
    humi_max=humi_max1;
    humi_min=humi_min1;
    illu_max=illu_max1;
    illu_min=illu_min1;
    start_t=start_t1;
    end_t=end_t1;

    qDebug() << "接收参数: " << temp_min<<temp_max<<humi_min<<humi_max<<illu_min<<illu_max<<start_t<<end_t;


}

bool MainWindow::conditionMet()
{
    int temp=ui->temp_label->text().toInt();
    int humi=ui->humi_label->text().toInt();
    int illu=ui->illu_label->text().toInt();

    bool flag_s=(temp<=temp_max && temp>=temp_min && humi<=humi_max && humi>=humi_min && illu<=illu_max && illu>=illu_min);


    return flag_s;
}

void MainWindow::checkTimeAndCondition()
{
    qDebug()<<"开始";
    QDateTime currentTime = QDateTime::currentDateTime();
    QString requestUrl;
    QNetworkRequest request;

    //设置请求地址
    QUrl url;

    //修改属性的地址
    requestUrl=URL_C;


    //设置数据提交格式
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

    //设置token
    request.setRawHeader("X-Auth-Token",Token); //Token是接口获取返回的的一串字符串

    //构造请求
    url.setUrl(requestUrl);

    request.setUrl(url);
    QString post_param_3=QString("{\"paras\":{\"clean_mode\":2},\"service_id\":\"ESP32C3\",\"command_name\":\"clean_mode\"}");
    QString post_param_4=QString("{\"paras\":{\"clean_mode\":3},\"service_id\":\"ESP32C3\",\"command_name\":\"clean_mode\"}");

    qDebug()<<"过程中";
    qDebug()<<currentTime;
    qDebug()<<temp_max;
    qDebug()<<start_t;
    qDebug()<<end_t;
    if (currentTime >= start_t && currentTime <= end_t)
    {
            qDebug()<<"时间条件满足";
            // 当前时间在开始时间和结束时间之间
            if (!messageSent && conditionMet())
            {
            // 满足条件并且“3”还没有发送
            manager->post(request, post_param_3.toUtf8());
            messageSent = true;  // 标记已发送“3”
            qDebug()<<"发送3";
            }
    } else if (currentTime > end_t && messageSent)
    {
            // 当前时间超过结束时间并且“3”已发送，发送“4”
            manager->post(request, post_param_4.toUtf8());
            messageSent = false; // 重置标志，准备下一次使用
            qDebug()<<"发送4";
    }
}




