#ifndef CLEANLOGWINDOW_H
#define CLEANLOGWINDOW_H

#include <QWidget>
#include <QStringListModel>


namespace Ui {
class CleanLogWindow;
}

class CleanLogWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CleanLogWindow(QWidget *parent = nullptr);
    ~CleanLogWindow();

private slots:
    void on_back_btn_clicked();

    void on_pushButton_clicked();

private:
    Ui::CleanLogWindow *ui;
};

#endif // CLEANLOGWINDOW_H
