#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QWidget>
#include <QDateTime>
#include "mainwindow.h"

class MainWindow;

namespace Ui {
class SettingWindow;
}

class SettingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SettingWindow(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~SettingWindow();

signals:
    void parsedDataReady(int temp_min, int temp_max, int humi_min, int humi_max,
                         int illu_min, int illu_max, QDateTime start_t, QDateTime end_t);

private slots:


    void on_apply_btn_clicked();

    void on_back_btn_clicked();

private:
    Ui::SettingWindow *ui;
    MainWindow *m;
};

#endif // SETTINGWINDOW_H
