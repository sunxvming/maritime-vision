#include "quiabout.h"
#include "ui_quiabout.h"
#include "quihelper.h"

SINGLETON_IMPL(QUIAbout)
QUIAbout::QUIAbout(QWidget *parent) : QDialog(parent), ui(new Ui::QUIAbout)
{
    ui->setupUi(this);
    this->initForm();
}

QUIAbout::~QUIAbout()
{
    delete ui;
}

void QUIAbout::showEvent(QShowEvent *)
{
    QUIHelper::setFormInCenter(this);
}

void QUIAbout::initForm()
{
    //设置阴影
    QUIHelper::setFormShadow(this, ui->verticalLayout, QUIConfig::HighColor, 5, 15);
    //设置无边框置顶显示不显示在任务栏
    QUIHelper::setFramelessForm(this, true, true, false);
    //设置关闭自动隐藏
    connect(ui->btnMenu_Close, SIGNAL(clicked()), this, SLOT(hide()));
    //重新设置不支持拖动
    this->setProperty("canMove", false);

    //设置不同的图标
    IconHelper::setIcon(ui->QUILabIco, QUIConfig::IconMain, QUIConfig::FontSize + 5);
    IconHelper::setIcon(ui->btnMenu_Close, QUIConfig::IconClose, QUIConfig::FontSize + 3);

    //启动定时器关闭界面
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(hide()));

    //设置一个默认值的信息
    this->setInfo(AboutInfo());
}

void QUIAbout::setInfo(const AboutInfo &info, int timeout)
{
    //设置标题
    ui->QUILabName->setText(info.title);
    ui->QUILabTitle->setText(QString("关于%1").arg(info.title));
    this->setWindowTitle(ui->QUILabTitle->text());
    IconHelper::setIcon(ui->QUILabIco, QUIConfig::IconMain, QUIConfig::FontSize + 2);

    //设置html内容
    QStringList list;
    list << QString("<html><body>");
    list << QString("<p>版本: %1</p>").arg(info.version);
    list << QString("<p>版权: %1</p>").arg(info.copyright);
    list << QString("<p>电话: %1</p>").arg(info.tel);
    list << QString("<p>网址: <a href=\"%1\"><span style=\"text-decoration:none;color:#ffffff;\">%1</a></p>").arg(info.http);
    list << QString("</body></html>");
    ui->QUILabInfo->setText(list.join("\n"));
    //设置可以打开超链接
    ui->QUILabInfo->setOpenExternalLinks(true);

    //自适应内容以便留出更多的空间拖动
    //ui->QUILabInfo->adjustSize();
    //下面的方法更合适,只需要宽度留出空间就行
    QSize size = ui->QUILabInfo->sizeHint();
    ui->QUILabInfo->resize(size.width(), ui->QUILabInfo->height());

    //设置logo图片
    QUIHelper::setPixmap(ui->QUILabImage, info.logo);

    //启动定时器关闭窗体
    timer->stop();
    if (timeout > 0) {
        timer->start(timeout * 1000);
    }

    //重新设置下尺寸才能真正刷新改变后的颜色,尤其是Qt6
    if (this->width() == 720) {
        this->setFixedSize(721, 300);
    } else {
        this->setFixedSize(720, 300);
    }
}
