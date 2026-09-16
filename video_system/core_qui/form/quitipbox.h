#ifndef QUITIPBOX_H
#define QUITIPBOX_H

#include <QDialog>
#include "quisingleton.h"
class QPropertyAnimation;

namespace Ui {
class QUITipBox;
}

class QUITipBox : public QDialog
{
    Q_OBJECT SINGLETON_DECL(QUITipBox)

public:
    explicit QUITipBox(QWidget *parent = 0);
    ~QUITipBox();

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::QUITipBox *ui;

    //超时关闭时间
    int timeout;
    //当前已显示过的时间
    int currentSec;
    //记住窗体的尺寸
    QSize formSize;

    //是否全屏
    bool fullScreen;
    //窗体切换动画
    QPropertyAnimation *animation;

private slots:
    //初始化窗体数据
    void initForm();
    //倒计时关闭
    void countDown();

public Q_SLOTS:
    //设置提示信息
    void setTip(const QString &title, const QString &tip, bool fullScreen = false, bool center = true, int timeout = 0);
    //隐藏界面
    void hide();
};

#endif // QUITIPBOX_H
