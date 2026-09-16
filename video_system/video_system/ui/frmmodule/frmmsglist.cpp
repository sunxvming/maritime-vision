#include "frmmsglist.h"
#include "ui_frmmsglist.h"
#include "frmmsglistitem.h"
#include "frmalarmdetail.h"
#include "quihelper.h"
#include "devicehelper.h"
#include "AiTcpClient.h"
#include "ttsmanager.h"
#include "dbdata.h"
#include "ApiClient.h"

frmMsgList::frmMsgList(QWidget *parent) : QWidget(parent), ui(new Ui::frmMsgList)
{
    ui->setupUi(this);
    this->initForm();
    this->initPanel();
    this->initAction();

    if (AiTcpClient::instance()) {
        connect(AiTcpClient::instance(), &AiTcpClient::alarmReceived,
                this, &frmMsgList::onAlarmReceived);
    }
}

frmMsgList::~frmMsgList()
{
    delete ui;
}

void frmMsgList::showEvent(QShowEvent *)
{
    static bool isLoad = false;
    if (!isLoad) {
        isLoad = true;
        resizeEvent(NULL);
    }

    if (AppConfig::MsgListCount != 0) {
        msgListCount = AppConfig::MsgListCount;
    }
}

void frmMsgList::resizeEvent(QResizeEvent *)
{
    //设置的0行表示自动计算行数直到不产生滚动条
    if (AppConfig::MsgListCount == 0 && isVisible()) {
        //根据高度变化自动设置行数
        int height = ui->panelWidget->height() - 10;
        //行数=高度/每行高度
        msgListCount = height / 80;
        checkCount();
    }
}

void frmMsgList::initForm()
{
    msgListCount = 1;
}

void frmMsgList::initPanel()
{
    //设置边距
    ui->panelWidget->setMargin(6);
    //设置间距
    ui->panelWidget->setSpace(6);
    //设置自动横向拉伸
    ui->panelWidget->setAutoWidth(true);
    //设置列数
    ui->panelWidget->setColumnCount(1);



}

void frmMsgList::initAction()
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

void frmMsgList::doAction()
{
    QAction *action = (QAction *)sender();
    QString text = action->text();
    if (text == "清空消息") {
        clearMsg();
    }
}

void frmMsgList::checkCount()
{
    //判断数量是否超过最大值超过则移除多余的
    while (ui->panelWidget->getWidgets().size() >= msgListCount) {
        QWidget *widget = ui->panelWidget->getWidgets().last();
        ui->panelWidget->removeWidget(widget);
        widget->deleteLater();
    }
}

void frmMsgList::clearMsg()
{
    ui->panelWidget->clearWidgets();
}

void frmMsgList::onAlarmReceived(const AlarmEvent &alarm)
{
    checkCount();

    frmMsgListItem *item = new frmMsgListItem;
    item->setAlarmData(alarm);

    connect(item, &frmMsgListItem::itemClicked,
            this, &frmMsgList::onAlarmItemClicked);

    ui->panelWidget->insertWidget(0, item);

    // Resolve camera name from in-memory cache (reflects latest name without server restart).
    QString cameraName = alarm.cameraName;
    int cameraId = alarm.cameraId.toInt();
    for (int i = 0; i < DbData::IpcInfo_Count; ++i) {
        if (DbData::IpcInfo_IpcID.at(i) == cameraId) {
            cameraName = DbData::IpcInfo_IpcName.at(i);
            break;
        }
    }

    speakAlarm(cameraName, alarm.algorithmId, alarm.voiceText);
}

void frmMsgList::speakAlarm(const QString &cameraName, int algorithmId, const QString &fallback)
{
    auto doSpeak = [cameraName](const QString &voiceText) {
        QString text = QString("警告 %1 %2").arg(cameraName, voiceText);
        TtsManager::instance().speak(text);
    };

    if (algorithmId > 0) {
        if (m_algoCacheLoaded) {
            doSpeak(m_algoVoiceCache.value(algorithmId, fallback));
        } else {
            loadAlgoCacheThenSpeak(cameraName, algorithmId, fallback);
        }
    } else {
        doSpeak(fallback);
    }
}

void frmMsgList::loadAlgoCacheThenSpeak(const QString &cameraName, int algorithmId, const QString &fallback)
{
    ApiClient::instance()->get("/api/v1/algorithms",
        [this, cameraName, algorithmId, fallback](const QJsonObject &data) {
            m_algoCacheLoaded = true;
            m_algoVoiceCache.clear();
            QJsonArray items = data.value("data").toArray();
            for (const QJsonValue &val : items) {
                QJsonObject obj = val.toObject();
                int id = obj["id"].toInt();
                QString vt = obj["voice_text"].toString();
                if (id > 0) m_algoVoiceCache.insert(id, vt);
            }
            QString voiceText = m_algoVoiceCache.value(algorithmId, fallback);
            QString text = QString("警告 %1 %2").arg(cameraName, voiceText);
            TtsManager::instance().speak(text);
        },
        [cameraName, fallback](const QString &err) {
            qDebug() << "loadAlgoCacheThenSpeak error:" << err;
            QString text = QString("警告 %1 %2").arg(cameraName, fallback);
            TtsManager::instance().speak(text);
        }
    );
}

void frmMsgList::addMsg(const QString &msg, const QString &result, const QImage &image, const QString &time)
{
    // Legacy method for compatibility with devicehelper
    // This is not used for alarm display, which uses onAlarmReceived instead
    checkCount();

    frmMsgListItem *item = new frmMsgListItem;
    // Note: This is the old interface, alarm display uses setAlarmData instead
    ui->panelWidget->insertWidget(0, item);
}

void frmMsgList::onAlarmItemClicked(const QString &alarmId)
{
    qDebug() << "Alarm item clicked, UUID:" << alarmId;

    // Find the alarm data from the item
    AlarmEvent alarmData;
    bool found = false;

    // Search through the panel widgets to find the clicked item
    QList<QWidget*> widgets = ui->panelWidget->getWidgets();
    for (QWidget *widget : widgets) {
        frmMsgListItem *item = qobject_cast<frmMsgListItem*>(widget);
        if (item && item->getAlarmId() == alarmId) {
            alarmData = item->getAlarmData();
            found = true;
            break;
        }
    }

    if (!found) {
        qDebug() << "Alarm data not found for ID:" << alarmId;
        return;
    }

    // Create and show detail dialog
    frmAlarmDetail *detailDialog = new frmAlarmDetail;
    detailDialog->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    detailDialog->setAttribute(Qt::WA_DeleteOnClose);
    detailDialog->setWindowModality(Qt::ApplicationModal);
    detailDialog->resize(1024, 700);

    // Hide back and refresh buttons for popup from alarm list
    detailDialog->findChild<QPushButton*>("btnBack")->setVisible(false);
    detailDialog->findChild<QPushButton*>("btnRefresh")->setVisible(false);

    connect(detailDialog, &frmAlarmDetail::backToList, detailDialog, &QWidget::close);

    QUIHelper::setFormInCenter(detailDialog);
    detailDialog->show();

    // Load alarm data from the cached AlarmEvent
    detailDialog->loadAlarmFromEvent(alarmData);
}
