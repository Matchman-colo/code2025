// SettingWindow.cpp

#include "settingwindow.h"
#include "ui_settingwindow.h"
#include <QDebug>

SettingWindow::SettingWindow(MainWindow *mainWindow, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingWindow),
    m(mainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(QStringLiteral("清扫模式设置"));

    connect(ui->apply_btn, &QPushButton::clicked, this, &SettingWindow::on_apply_btn_clicked);
    connect(this, &SettingWindow::parsedDataReady, m, &MainWindow::onDataReceived);
}

SettingWindow::~SettingWindow()
{
    delete ui;
}



void SettingWindow::on_apply_btn_clicked()
{
    int temp_min = ui->temp_min->text().toInt();
    int temp_max = ui->temp_max->text().toInt();
    int humi_min = ui->humi_min->text().toInt();
    int humi_max = ui->humi_max->text().toInt();
    int illu_min = ui->illu_min->text().toInt();
    int illu_max = ui->illu_max->text().toInt();
    QDateTime start_t = ui->startTimeEdit->dateTime();
    QDateTime end_t = ui->endTimeEdit->dateTime();

    emit parsedDataReady(temp_min, temp_max, humi_min, humi_max, illu_min, illu_max, start_t, end_t);

    this->close();
}


void SettingWindow::on_back_btn_clicked()
{
    this->hide();
}

