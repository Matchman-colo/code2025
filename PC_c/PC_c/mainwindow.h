#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QButtonGroup>
#include <QTimeZone>
#include <QDateTime>
#include <QTimer>





QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int function_select;
    QNetworkAccessManager *manager;
    QByteArray Token;
    QButtonGroup *pbuttonGroup;
    int c_state=0;
    QString c_time="";
    int temp_max;
    int temp_min;
    int humi_max;
    int humi_min;
    int illu_max;
    int illu_min;
    QDateTime start_t;
    QDateTime end_t;
    QTimer *timer;


private slots:
    void on_logcheck_btn_clicked();

    void on_automode_btn_clicked();


    void on_get_btn_clicked();
    void GetToken();
    void Get_device_properties();
    void replyFinished(QNetworkReply *reply);
    void SaveDataToFile(QByteArray token);

    void on_token_btn_clicked();

    void onRadioClickSlot();
    void appendToFile(const QString &fileName, const QString &text);
    bool conditionMet();
    void checkTimeAndCondition();



    void on_startauto_btn_clicked();

public slots:
    void onDataReceived(int temp_min, int temp_max, int humi_min,int humi_max,int illu_min,int illu_max,QDateTime start_t,QDateTime end_t);

private:
    Ui::MainWindow *ui;
    bool messageSent;
};


#endif // MAINWINDOW_H
