#include "quiinputbox.h"
#include "ui_quiinputbox.h"
#include "quihelper.h"

SINGLETON_IMPL(QUIInputBox)
QUIInputBox::QUIInputBox(QWidget *parent) : QDialog(parent), ui(new Ui::QUIInputBox)
{
    ui->setupUi(this);
    this->initForm();
}

QUIInputBox::~QUIInputBox()
{
    delete ui;
}

void QUIInputBox::showEvent(QShowEvent *)
{
    //设置按钮图标
    QUIHelper::setIconBtn(ui->btnOk, ":/image/btn_ok.png", 0xf00c);
    QUIHelper::setIconBtn(ui->btnCancel, ":/image/btn_close.png", 0xf00d);
    //居中显示
    QUIHelper::setFormInCenter(this);
    //激活窗体
    this->activateWindow();
    //焦点设置到输入框
    ui->QUITxtValue->setFocus();
    value.clear();
}

void QUIInputBox::closeEvent(QCloseEvent *)
{
    timeout = 0;
    currentSec = 0;
}

void QUIInputBox::initForm()
{    
    //设置阴影
    QUIHelper::setFormShadow(this, ui->verticalLayout, QUIConfig::HighColor, 5, 15);
    //设置无边框
    QUIHelper::setFramelessForm(this, ui->QUIWidgetTitle, ui->QUILabIco, ui->btnMenu_Close);
    //设置关闭按钮单击关闭窗体
    connect(ui->btnMenu_Close, SIGNAL(clicked()), this, SLOT(on_btnCancel_clicked()));

    //设置标题和窗体大小
    this->setWindowTitle(ui->QUILabTitle->text());
    this->setFixedSize(QUIDialogMinWidth, QUIDialogMinHeight + 10);

    //按钮设置最小尺寸和图标大小
    QList<QPushButton *> btns  = ui->frame->findChildren<QPushButton *>();
    foreach (QPushButton *btn, btns) {
        btn->setMinimumWidth(QUIBtnMinWidth);
        btn->setIconSize(QSize(QUIIconWidth, QUIIconHeight));
    }

    timeout = 0;
    currentSec = 0;

    //关闭倒计时定时器    
    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(countDown()));
    timer->start();
}

void QUIInputBox::countDown()
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

void QUIInputBox::on_btnOk_clicked()
{
    if (ui->QUITxtValue->isVisible()) {
        value = ui->QUITxtValue->text();
    } else if (ui->QUICboxValue->isVisible()) {
        value = ui->QUICboxValue->currentText();
    }

    //返回对话框执行结果并关闭窗体
    this->done(QMessageBox::Ok);
    this->close();
}

void QUIInputBox::on_btnCancel_clicked()
{
    //返回对话框执行结果并关闭窗体
    this->done(QMessageBox::Cancel);
    this->close();
}

QString QUIInputBox::getValue() const
{
    return this->value;
}

void QUIInputBox::setParameter(const QString &info, int type, int timeout,
                               QString placeholderText, bool pwd, const QString &defaultValue)
{

    //设置倒计时时间
    this->timeout = timeout;
    this->currentSec = 0;
    ui->QUILabCountDown->clear();
    countDown();

    ui->QUILabInfo->setText(info);
    IconHelper::setIcon(ui->QUILabIco, QUIConfig::IconMain, QUIConfig::FontSize + 2);

    if (type == 0) {
        ui->QUICboxValue->setVisible(false);
        ui->QUITxtValue->setPlaceholderText(placeholderText);
        ui->QUITxtValue->setText(defaultValue);
        //密文显示
        if (pwd) {
            ui->QUITxtValue->setEchoMode(QLineEdit::Password);
        }
    } else if (type == 1) {
        ui->QUITxtValue->setVisible(false);
        ui->QUICboxValue->addItems(defaultValue.split("|"));
        //回显字符串作为默认的下拉选项
        if (!placeholderText.isEmpty()) {
            ui->QUICboxValue->setCurrentIndex(ui->QUICboxValue->findText(placeholderText));
        }
    }
}
