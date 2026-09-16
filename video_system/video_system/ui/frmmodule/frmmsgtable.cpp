#include "frmmsgtable.h"
#include "ui_frmmsgtable.h"
#include "devicehelper.h"

frmMsgTable::frmMsgTable(QWidget *parent) : QWidget(parent), ui(new Ui::frmMsgTable)
{
    ui->setupUi(this);
    this->initForm();
    this->initAction();
    resizeEvent(NULL);
}

frmMsgTable::~frmMsgTable()
{
    delete ui;
}

void frmMsgTable::showEvent(QShowEvent *)
{
    static bool isLoad = false;
    if (!isLoad) {
        isLoad = true;
        resizeEvent(NULL);
    }
}

void frmMsgTable::resizeEvent(QResizeEvent *)
{
    //设置的0行表示自动计算行数直到不产生滚动条
    if (AppConfig::MsgTableCount == 0 && isVisible()) {
        //根据高度变化自动设置行数
        int height = ui->tableWidgetMsg->height() - 30;
        //行数=高度/每行高度
        int rowCount = height / AppData::RowHeight;
        ui->tableWidgetMsg->setRowCount(rowCount);
    }
}

void frmMsgTable::initForm()
{
    DeviceHelper::setTableWidget(ui->tableWidgetMsg);
    DeviceHelper::initDeviceTable();
}

void frmMsgTable::initAction()
{
    //增加右键菜单操作用于演示各种动作
    this->setContextMenuPolicy(Qt::ActionsContextMenu);
    QStringList listTexts;
    listTexts << "清空消息";
    foreach (QString text, listTexts) {
        QAction *action = new QAction(text, this);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(doAction()));
        this->addAction(action);
    }
}

void frmMsgTable::doAction()
{
    QAction *action = (QAction *)sender();
    QString text = action->text();
    if (text == "清空消息") {
        DeviceHelper::clearMsg();
    }
}
