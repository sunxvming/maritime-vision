#include "quihelperform.h"
#include "iconhelper.h"
#include "quihelper.h"
#include "quimessagebox.h"
#include "quitipbox.h"
#include "quidateselect.h"
#include "quiinputbox.h"
#include "quiabout.h"
#include "quisplash.h"

bool QUIHelperForm::formShadow = true;
void QUIHelperForm::setFormShadow(QWidget *widget, QLayout *layout, const QString &color, int margin, int radius)
{
    //在部分linux系统设置了背景透明是黑色的所以限定只在win
#ifndef Q_OS_WIN
    return;
#endif
    if (margin <= 0 || radius <= 0 || !formShadow) {
        return;
    }

    //先判断是否已经存在阴影效果
    QGraphicsDropShadowEffect *shadowEffect = (QGraphicsDropShadowEffect *)widget->graphicsEffect();
    if (shadowEffect == 0) {
        shadowEffect = new QGraphicsDropShadowEffect(widget);
    }

    //采用系统自带的函数设置阴影
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(color);
    shadowEffect->setBlurRadius(radius);
    widget->setGraphicsEffect(shadowEffect);

    //必须设置背景透明
    widget->setAttribute(Qt::WA_TranslucentBackground, true);
    //设置布局边距留出空间给边框阴影
    layout->setContentsMargins(margin, margin, margin, margin);
}

void QUIHelperForm::setFormShadow(const QString &color)
{
    if (!formShadow) {
        return;
    }

    //重新应用边框阴影颜色等
    QGraphicsDropShadowEffect *shadowEffect = 0;

    //消息框
    shadowEffect = (QGraphicsDropShadowEffect *) QUIMessageBox::Instance()->graphicsEffect();
    if (shadowEffect) {
        shadowEffect->setColor(color);
    }

    //输入框
    shadowEffect = (QGraphicsDropShadowEffect *) QUIInputBox::Instance()->graphicsEffect();
    if (shadowEffect) {
        shadowEffect->setColor(color);
    }

    //右下角提示框
    shadowEffect = (QGraphicsDropShadowEffect *) QUITipBox::Instance()->graphicsEffect();
    if (shadowEffect) {
        shadowEffect->setColor(color);
    }

    //日期选择框
    shadowEffect = (QGraphicsDropShadowEffect *) QUIDateSelect::Instance()->graphicsEffect();
    if (shadowEffect) {
        shadowEffect->setColor(color);
    }

    //关于对话框
    shadowEffect = (QGraphicsDropShadowEffect *) QUIAbout::Instance()->graphicsEffect();
    if (shadowEffect) {
        shadowEffect->setColor(color);
    }

    //中间提示信息框
    shadowEffect = (QGraphicsDropShadowEffect *) QUISplash::Instance()->graphicsEffect();
    if (shadowEffect) {
        shadowEffect->setColor(color);
    }
}

void QUIHelperForm::setFramelessForm(QWidget *widgetMain, bool tool, bool top, bool menu, bool x11)
{
    //设置弱属性 form 会产生边框
    widgetMain->setProperty("form", true);
    //设置弱属性 camMove 表示当前窗体可以移动
    widgetMain->setProperty("canMove", true);

    //根据设定逐个追加属性
    widgetMain->setWindowFlags(Qt::FramelessWindowHint);
    if (tool) {
        widgetMain->setWindowFlags(widgetMain->windowFlags() | Qt::Tool);
    }
    if (top) {
        widgetMain->setWindowFlags(widgetMain->windowFlags() | Qt::WindowStaysOnTopHint);
    }
    if (menu) {
        //如果是其他系统比如neokylin会产生系统边框
#ifdef Q_OS_WIN
        widgetMain->setWindowFlags(widgetMain->windowFlags() | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
#endif
    }
    if (x11) {
        //开启后忽略系统的窗口管理,窗体可以拖动到屏幕外
#ifdef __arm__
        widgetMain->setWindowFlags(widgetMain->windowFlags() | Qt::X11BypassWindowManagerHint);
#endif
    }
}

void QUIHelperForm::setFramelessForm(QWidget *widgetMain, QWidget *widgetTitle,
                                     QLabel *labIco, QPushButton *btnClose,
                                     bool tool, bool top, bool menu, bool x11)
{
    //设置固定尺寸
    labIco->setFixedWidth(QUITitleMinSize);
    btnClose->setFixedWidth(QUITitleMinSize);
    widgetTitle->setFixedHeight(QUITitleMinSize);
    widgetTitle->setProperty("form", "title");

    //设置无边框属性
    setFramelessForm(widgetMain, tool, top, menu, x11);

    //设置图标
    IconHelper::setIcon(labIco, QUIConfig::IconMain, QUIConfig::FontSize + 2);
    IconHelper::setIcon(btnClose, QUIConfig::IconClose, QUIConfig::FontSize);
}

bool QUIHelperForm::isCustomUI = false;
int QUIHelperForm::showMessageBox(const QString &info, int type, int timeout, bool exec)
{
    int result = 0;
    if (type == 0) {
        showMessageBoxInfo(info, timeout, exec);
    } else if (type == 1) {
        showMessageBoxError(info, timeout, exec);
    } else if (type == 2) {
        result = showMessageBoxQuestion(info);
    }

    return result;
}

void QUIHelperForm::showMessageBoxInfo(const QString &info, int timeout, bool exec)
{
    if (isCustomUI) {
        if (exec) {
            QUIMessageBox msg;
            msg.setMessage(info, 0, timeout);
            msg.exec();
        } else {
            QUIMessageBox::Instance()->setMessage(info, 0, timeout);
            QUIMessageBox::Instance()->show();
        }
    } else {
        QMessageBox box(QMessageBox::Information, "提示", info);
        box.setStandardButtons(QMessageBox::Yes);
        box.setButtonText(QMessageBox::Yes, QString("确 定"));
        box.exec();
        //QMessageBox::information(0, "提示", info, QMessageBox::Yes);
    }
}

void QUIHelperForm::showMessageBoxError(const QString &info, int timeout, bool exec)
{
    if (isCustomUI) {
        if (exec) {
            QUIMessageBox msg;
            msg.setMessage(info, 2, timeout);
            msg.exec();
        } else {
            QUIMessageBox::Instance()->setMessage(info, 2, timeout);
            QUIMessageBox::Instance()->show();
        }
    } else {
        QMessageBox box(QMessageBox::Critical, "错误", info);
        box.setStandardButtons(QMessageBox::Yes);
        box.setButtonText(QMessageBox::Yes, QString("确 定"));
        box.exec();
        //QMessageBox::critical(0, "错误", info, QMessageBox::Yes);
    }
}

int QUIHelperForm::showMessageBoxQuestion(const QString &info)
{
    if (isCustomUI) {
        QUIMessageBox msg;
        msg.setMessage(info, 1);
        msg.update();
        return msg.exec();
    } else {
        QMessageBox box(QMessageBox::Question, "询问", info);
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setButtonText(QMessageBox::Yes, QString("确 定"));
        box.setButtonText(QMessageBox::No, QString("取 消"));
        return box.exec();
        //return QMessageBox::question(0, "询问", info, QMessageBox::Yes | QMessageBox::No);
    }
}

void QUIHelperForm::showTipBox(const QString &title, const QString &tip, bool fullScreen, bool center, int timeout)
{
    QUITipBox::Instance()->setTip(title, tip, fullScreen, center, timeout);
    QUITipBox::Instance()->show();
}

void QUIHelperForm::hideTipBox()
{
    QUITipBox::Instance()->hide();
}

QString QUIHelperForm::showInputBox(const QString &title, int type, int timeout,
                                    const QString &placeholderText, bool pwd,
                                    const QString &defaultValue)
{
    QString result;
    if (isCustomUI) {
        QUIInputBox input;
        input.setParameter(title, type, timeout, placeholderText, pwd, defaultValue);
        if (input.exec() == QMessageBox::Ok) {
            result = input.getValue();
        }
    } else {
        result = QInputDialog::getText(0, "输入框", title);
    }

    return result;
}

int QUIHelperForm::showDateSelect(QString &dateStart, QString &dateEnd, const QString &format)
{
    QUIDateSelect dateSelect;
    dateSelect.setFormat(format);
    int result = dateSelect.exec();
    dateStart = dateSelect.getDateTimeStart();
    dateEnd = dateSelect.getDateTimeEnd();
    return result;
}

void QUIHelperForm::showAboutInfo(const AboutInfo &info, int timeout, bool exec)
{
    if (exec) {
        QUIAbout about;
        about.setInfo(info, timeout);
        about.exec();
    } else {
        QUIAbout::Instance()->setInfo(info, timeout);
        QUIAbout::Instance()->show();
    }
}

void QUIHelperForm::showSplashInfo(const QString &info, int fontSizeMain, int fontSizeSub, int timeout, bool exec)
{
    if (exec) {
        //阻塞模式还有个小问题,尺寸不会自适应
        QUISplash splash;
        splash.setInfo(info, fontSizeMain, fontSizeSub, timeout);
        splash.exec();
    } else {
        QUISplash::Instance()->setInfo(info, fontSizeMain, fontSizeSub,timeout);
        QUISplash::Instance()->show();
    }
}

void QUIHelperForm::hideSplashInfo()
{
    QUISplash::Instance()->hide();
}
