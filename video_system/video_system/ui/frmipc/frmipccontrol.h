#ifndef FRMIPCCONTROL_H
#define FRMIPCCONTROL_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class frmIpcControl;
}

class frmIpcControl : public QWidget
{
    Q_OBJECT

public:
    explicit frmIpcControl(QWidget *parent = 0);
    ~frmIpcControl();

private:
    Ui::frmIpcControl *ui;

private slots:
    void initForm();

private slots:
    //滑块值变化设置文本框
    void valueChanged(int value);
    //执行结果返回
    void receiveResult(const QString &url, const QString &cmd, const QVariant &data);

private slots:
    void on_btnGetImageSetting_clicked();
    void on_btnSetImageSetting_clicked();

    void on_btnSystemReboot_clicked();
    void on_btnSetDateTime_clicked();

    void on_btnSnapImage_clicked();
    void on_btnLoadVideo_clicked();
};

#endif // FRMIPCCONTROL_H
