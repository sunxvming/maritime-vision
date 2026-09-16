#include "quisplash.h"
#include "ui_quisplash.h"
#include "quihelper.h"

SINGLETON_IMPL(QUISplash)
QUISplash::QUISplash(QWidget *parent) : QDialog(parent), ui(new Ui::QUISplash)
{
    ui->setupUi(this);
    this->initForm();
}

QUISplash::~QUISplash()
{
    delete ui;
}

void QUISplash::showEvent(QShowEvent *)
{
    //居中显示
    QUIHelper::setFormInCenter(this);
}

void QUISplash::closeEvent(QCloseEvent *)
{
    timeout = 0;
    currentSec = 0;
}

void QUISplash::initForm()
{
    //设置阴影
    QUIHelper::setFormShadow(this, ui->verticalLayout, QUIConfig::HighColor, 5, 15);
    //设置无边框置顶显示不显示在任务栏
    QUIHelper::setFramelessForm(this, true, true, false);

    timeout = 0;
    currentSec = 0;
    formSize = QSize(520, 120);

    //关闭倒计时定时器
    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(countDown()));
    timer->start();

    //设置显示的提示文字
    this->setInfo("请稍等, 正在疯狂努力加载中...", QUIConfig::FontSize + 18, QUIConfig::FontSize + 5);
}

void QUISplash::countDown()
{
    if (timeout == 0) {
        return;
    }

    if (currentSec < timeout) {
        currentSec++;
    } else {
        this->close();
    }

    QString text = QString("关闭倒计时 %1 s").arg(timeout - currentSec + 1);
    ui->QUILabCountDown->setText(text);
}

void QUISplash::setInfo(const QString &info, int fontSizeMain, int fontSizeSub, int timeout)
{
    //设置倒计时时间
    this->timeout = timeout;
    this->currentSec = 0;
    ui->QUILabCountDown->clear();
    ui->QUILabCountDown->setVisible(timeout > 0);
    countDown();

    //设置下字体
    QFont font;
    font.setPixelSize(fontSizeMain);
    ui->QUILabInfo->setFont(font);
    font.setPixelSize(fontSizeSub);
    ui->QUILabCountDown->setFont(font);
    //设置提示信息文本
    ui->QUILabInfo->setText(info);

    //合适尺寸再加点会显得更大气
    QSize newSize = sizeHint() + QSize(50, 20);
    //矫正最小尺寸
    if (newSize.width() < 520) {
        newSize.setWidth(520);
    }
    if (newSize.height() < 120) {
        newSize.setHeight(120);
    }

    //重新设置下尺寸才能真正刷新改变后的颜色,尤其是Qt6
    if (newSize == formSize) {
        formSize = newSize + QSize(1, 0);
    } else {
        formSize = newSize;
    }

    this->setFixedSize(formSize);
}
