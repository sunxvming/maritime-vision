#include "quidateselect.h"
#include "ui_quidateselect.h"
#include "quihelper.h"

SINGLETON_IMPL(QUIDateSelect)
QUIDateSelect::QUIDateSelect(QWidget *parent) : QDialog(parent), ui(new Ui::QUIDateSelect)
{
    ui->setupUi(this);
    this->initForm();
}

QUIDateSelect::~QUIDateSelect()
{
    delete ui;
}

void QUIDateSelect::showEvent(QShowEvent *)
{
    //设置按钮图标
    QUIHelper::setIconBtn(ui->btnOk, ":/image/btn_ok.png", 0xf00c);
    QUIHelper::setIconBtn(ui->btnCancel, ":/image/btn_close.png", 0xf00d);
    IconHelper::setIcon(ui->QUILabIco, QUIConfig::IconMain, QUIConfig::FontSize + 2);
    //居中显示
    QUIHelper::setFormInCenter(this);
    //激活窗体
    this->activateWindow();
}

void QUIDateSelect::initForm()
{    
    //设置阴影
    QUIHelper::setFormShadow(this, ui->verticalLayout, QUIConfig::HighColor, 5, 15);
    //设置无边框
    QUIHelper::setFramelessForm(this, ui->QUIWidgetTitle, ui->QUILabIco, ui->btnMenu_Close);
    //设置关闭按钮单击关闭窗体
    connect(ui->btnMenu_Close, SIGNAL(clicked()), this, SLOT(on_btnCancel_clicked()));

    //设置标题和窗体大小
    this->setWindowTitle(ui->QUILabTitle->text());
    this->setFixedSize(QUIDialogMinWidth + 50, QUIDialogMinHeight);

    //按钮设置最小尺寸和图标大小
    QList<QPushButton *> btns  = ui->frame->findChildren<QPushButton *>();
    foreach (QPushButton *btn, btns) {
        btn->setMinimumWidth(QUIBtnMinWidth);
        btn->setIconSize(QSize(QUIIconWidth, QUIIconHeight));
    }

    //设置默认日期当前日期
    ui->dateTimeStart->setDate(QDate::currentDate());
    ui->dateTimeEnd->setDate(QDate::currentDate().addDays(1));
    ui->dateTimeStart->calendarWidget()->setGridVisible(true);
    ui->dateTimeEnd->calendarWidget()->setGridVisible(true);
    ui->dateTimeStart->calendarWidget()->setLocale(QLocale::Chinese);
    ui->dateTimeEnd->calendarWidget()->setLocale(QLocale::Chinese);
    this->setFormat("yyyy-MM-dd");
}

void QUIDateSelect::on_btnOk_clicked()
{
    //过滤非法时间范围
    QDateTime dateStart = ui->dateTimeStart->dateTime();
    QDateTime dateEnd = ui->dateTimeEnd->dateTime();
    if (dateStart > dateEnd) {
        QUIHelper::showMessageBoxError("开始时间不能大于结束时间!", 3);
        return;
    }

    //返回对话框执行结果并关闭窗体
    this->done(QMessageBox::Ok);
    this->close();
}

void QUIDateSelect::on_btnCancel_clicked()
{
    //返回对话框执行结果并关闭窗体
    this->done(QMessageBox::Cancel);
    this->close();
}

QString QUIDateSelect::getDateTimeStart() const
{
    return ui->dateTimeStart->dateTime().toString(format);
}

QString QUIDateSelect::getDateTimeEnd() const
{
    return ui->dateTimeEnd->dateTime().toString(format);
}

void QUIDateSelect::setFormat(const QString &format)
{
    this->format = format;
    ui->dateTimeStart->setDisplayFormat(format);
    ui->dateTimeEnd->setDisplayFormat(format);
}
