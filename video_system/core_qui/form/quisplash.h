#ifndef QUISPLASH_H
#define QUISPLASH_H

#include <QDialog>
#include "quisingleton.h"

namespace Ui {
class QUISplash;
}

class QUISplash : public QDialog
{
    Q_OBJECT SINGLETON_DECL(QUISplash)

public:
    explicit QUISplash(QWidget *parent = 0);
    ~QUISplash();

protected:
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *);

private:
    Ui::QUISplash *ui;

    //超时关闭时间
    int timeout;
    //当前已显示过的时间
    int currentSec;
    //记住窗体的尺寸
    QSize formSize;

private slots:
    //初始化窗体数据
    void initForm();
    //倒计时关闭
    void countDown();

public Q_SLOTS:
    //设置提示信息
    void setInfo(const QString &info, int fontSizeMain, int fontSizeSub, int timeout = 0);
};

#endif // QUISPLASH_H
