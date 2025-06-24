#include "cleanlogwindow.h"
#include "ui_cleanlogwindow.h"
#include "mainwindow.h"

CleanLogWindow::CleanLogWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CleanLogWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(QStringLiteral("清扫日志"));
}

CleanLogWindow::~CleanLogWindow()
{
    delete ui;
}

void CleanLogWindow::on_back_btn_clicked()
{  
    hide();
}


void CleanLogWindow::on_pushButton_clicked()
{
    QFile file_1("D:/app_ui/PC_c/cleanlog_1.txt");
    QFile file_2("D:/app_ui/PC_c/cleanlog_2.txt");
    if (file_1.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file_1);
        QStringList fileContent;
        while (!in.atEnd()) {
            fileContent.append(in.readLine());  // 每一行文件内容放入 QStringList
        }
        ui->listView_1->setModel(new QStringListModel(fileContent));  // 设置模型
        file_1.close();
    } else {
        qDebug() << "文件打开失败";
    }
    if (file_2.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file_2);
        QStringList fileContent;
        while (!in.atEnd()) {
            fileContent.append(in.readLine());  // 每一行文件内容放入 QStringList
        }
        ui->listView_2->setModel(new QStringListModel(fileContent));  // 设置模型
        file_2.close();
    } else {
        qDebug() << "文件打开失败";
    }

}

