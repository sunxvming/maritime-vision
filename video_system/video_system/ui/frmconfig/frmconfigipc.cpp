#include "frmconfigipc.h"
#include "ui_frmconfigipc.h"
#include "quihelper.h"
#include "devicehelper.h"
#include "urlhelper.h"
#include "dbquery.h"
#include "dbdelegate.h"
#include "frmconfigplus.h"
#include "dlgipcalgoconfig.h"
#include "ApiClient.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItem>
#include <QPushButton>
#include <QHBoxLayout>
#include <memory>

// Column indices
static const int COL_ID      = 0;
static const int COL_NAME    = 1;
static const int COL_NVR     = 2;
static const int COL_TYPE    = 3;
static const int COL_ONVIF   = 4;
static const int COL_PROFILE = 5;
static const int COL_SOURCE  = 6;
static const int COL_RTSP    = 7;
static const int COL_RTSPSUB = 8;
static const int COL_POS     = 9;
static const int COL_IMAGE   = 10;
static const int COL_X       = 11;
static const int COL_Y       = 12;
static const int COL_USER    = 13;
static const int COL_PWD     = 14;
static const int COL_ENABLE  = 15;
static const int COL_SCENE   = 16;
static const int COL_ALGO    = 17;
static const int COL_MARK    = 18;
static const int COL_COUNT   = 19;

frmConfigIpc::frmConfigIpc(QWidget *parent) : QWidget(parent), ui(new Ui::frmConfigIpc)
{
    ui->setupUi(this);
    this->initForm();
    this->initData();
    this->initIcon();
}

frmConfigIpc::~frmConfigIpc()
{
    delete ui;
}

void frmConfigIpc::showEvent(QShowEvent *)
{
    loadDataFromApi();
}

void frmConfigIpc::initForm()
{
    ui->widgetTop->setProperty("flag", "navbtn");
    ui->labTip->setText("提示 → 改动后立即应用");

    QUIHelper::initTableView(ui->tableView, AppData::RowHeight, false, true);
    connect(AppEvent::Instance(), SIGNAL(changeStyle()), this, SLOT(initIcon()));

    if (AppConfig::TableDataPolicy == 1) {
        ui->tableView->setSelectionMode(QAbstractItemView::MultiSelection);
        ui->tableView->setEditTriggers(QAbstractItemView::DoubleClicked);
    }

    ui->widgetIpcSearch->setVisible(AppConfig::VisibleIpcSearch);
    connect(ui->widgetIpcSearch, SIGNAL(addDevices(QList<QStringList>)), this, SLOT(addDevices(QList<QStringList>)));
    connect(frmConfigPlus::Instance(), SIGNAL(addPlus(QStringList, QStringList)), this, SLOT(addPlus(QStringList, QStringList)));
}

void frmConfigIpc::initIcon()
{
    CommonNav::setIconBtn(ui->widgetTop);
}

void frmConfigIpc::initData()
{
    model = new QStandardItemModel(this);
    ui->tableView->setModel(model);

    columnNames << "编号" << "名称" << "录像机" << "厂家" << "设备地址" << "配置文件" << "视频文件"
                << "主码流地址" << "子码流地址" << "经纬度" << "地图"
                << "X坐标" << "Y坐标" << "用户姓名" << "用户密码" << "启用" << "场景" << "算法配置" << "备注";
    columnWidths << 40 << 90 << 90 << 80 << 250 << 100 << 100 << 130 << 130
                 << 150 << 90 << 45 << 45 << 80 << 80 << 40 << 100 << 110 << 60;

    int size = columnNames.size();
    if (QUIHelper::deskWidth() >= 1920) {
        for (int i = 0; i < size - 2; ++i) columnWidths[i] += 30;
        columnWidths[COL_RTSP] = 350;
        columnWidths[COL_RTSPSUB] = 350;
    }

    model->setHorizontalHeaderLabels(columnNames);
    for (int i = 0; i < size; ++i)
        ui->tableView->setColumnWidth(i, columnWidths.at(i));

    ui->tableView->setColumnHidden(COL_IMAGE,      true);
    ui->tableView->setColumnHidden(COL_POS,      true);
    ui->tableView->setColumnHidden(COL_ID,      true);
    ui->tableView->setColumnHidden(COL_ONVIF,   true);
    ui->tableView->setColumnHidden(COL_PROFILE, true);
    ui->tableView->setColumnHidden(COL_SOURCE,  true);
    ui->tableView->setColumnHidden(COL_X,       true);
    ui->tableView->setColumnHidden(COL_Y,       true);

    d_cbox_nvrName = new DbDelegate(this);
    d_cbox_nvrName->setDelegateType("QComboBox");
    ui->tableView->setItemDelegateForColumn(COL_NVR, d_cbox_nvrName);
    nvrNameChanged();

    d_cbox_ipcImage = new DbDelegate(this);
    d_cbox_ipcImage->setDelegateType("QComboBox");
    ui->tableView->setItemDelegateForColumn(COL_IMAGE, d_cbox_ipcImage);
    ipcImageChanged();

    DbDelegate *d_txt_userPwd = new DbDelegate(this);
    d_txt_userPwd->setDelegateType("QLineEdit");
    d_txt_userPwd->setDelegatePwd(true);
    d_txt_userPwd->setDelegateColumn(COL_PWD);
    ui->tableView->setItemDelegateForColumn(COL_PWD, d_txt_userPwd);

    DbDelegate *d_ckbox_ipcEnable = new DbDelegate(this);
    d_ckbox_ipcEnable->setDelegateColumn(COL_ENABLE);
    d_ckbox_ipcEnable->setDelegateType("QCheckBox");
    d_ckbox_ipcEnable->setCheckBoxText("启用", "禁用");
    ui->tableView->setItemDelegateForColumn(COL_ENABLE, d_ckbox_ipcEnable);
}

void frmConfigIpc::nvrNameChanged()
{
    QStringList nvrNames;
    foreach (QString nvrName, DbData::NvrInfo_NvrName) {
        if (!nvrNames.contains(nvrName))
            nvrNames << nvrName;
    }
    d_cbox_nvrName->setDelegateValue(nvrNames);
}

void frmConfigIpc::ipcImageChanged()
{
    QStringList mapNames;
    mapNames << " -- 无 -- " << AppData::MapNames;
    d_cbox_ipcImage->setDelegateValue(mapNames);
}

void frmConfigIpc::loadScenes()
{
    ApiClient::instance()->get("/api/v1/alarm_scene",
        [this](const QJsonObject &data) {
            onScenesLoaded(data);
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "loadScenes error:" << err;
        }
    );
}

void frmConfigIpc::onScenesLoaded(const QJsonObject &respData)
{
    m_sceneMap.clear();
    QJsonArray list = respData.value("list").toArray();
    for (const QJsonValue &val : list) {
        QJsonObject obj = val.toObject();
        m_sceneMap[obj["scene_id"].toInt()] = obj["scene_name"].toString();
    }
    loadIpcList();
}

void frmConfigIpc::loadIpcList()
{
    ApiClient::instance()->getAllIpc(
        [this](const QJsonArray &list) {
            onIpcDataLoaded(list);
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "loadDataFromApi error:" << err;
        }
    );
}

void frmConfigIpc::onIpcDataLoaded(const QJsonArray &list)
{
    model->setRowCount(0);
    for (const QJsonValue &val : list) {
        QJsonObject obj = val.toObject();
        int row = model->rowCount();
        QList<QStandardItem *> items;

        auto *idItem = new QStandardItem(QString::number(obj["ipc_id"].toInt()));
        idItem->setData(false, Qt::UserRole);
        items << idItem;
        items << new QStandardItem(obj["ipc_name"].toString());
        items << new QStandardItem(obj["nvr_name"].toString());
        items << new QStandardItem(obj["ipc_type"].toString());
        items << new QStandardItem(obj["onvif_addr"].toString());
        items << new QStandardItem(obj["profile_token"].toString());
        items << new QStandardItem(obj["video_source"].toString());
        items << new QStandardItem(obj["rtsp_main"].toString());
        items << new QStandardItem(obj["rtsp_sub"].toString());
        items << new QStandardItem(obj["ipc_position"].toString());
        items << new QStandardItem(obj["ipc_image"].toString());
        items << new QStandardItem(QString::number(obj["ipc_x"].toInt()));
        items << new QStandardItem(QString::number(obj["ipc_y"].toInt()));
        items << new QStandardItem(obj["user_name"].toString());
        items << new QStandardItem(obj["user_pwd"].toString());
        items << new QStandardItem(obj["ipc_enable"].toString());

        int sceneId = obj["scene_id"].toInt();
        QString sceneName = m_sceneMap.value(sceneId, "未设置");
        auto *sceneItem = new QStandardItem(sceneName);
        sceneItem->setData(sceneId, Qt::UserRole);
        sceneItem->setEditable(false);
        items << sceneItem;

        auto *algoItem = new QStandardItem("");
        algoItem->setData(obj["algorithm_ids"].toString(), Qt::UserRole);
        items << algoItem;

        items << new QStandardItem(obj["ipc_mark"].toString());
        model->appendRow(items);

        QWidget *opWidget = new QWidget;
        QHBoxLayout *opLayout = new QHBoxLayout(opWidget);
        opLayout->setContentsMargins(20, 2, 20, 2);
        opLayout->setSpacing(20);

        QPushButton *btnConfig = new QPushButton("配置");
        btnConfig->setMaximumWidth(60);
        opLayout->addWidget(btnConfig);
        opLayout->addStretch();

        ui->tableView->setIndexWidget(model->index(row, COL_ALGO), opWidget);

        int capturedRow = row;
        connect(btnConfig, &QPushButton::clicked, this, [this, capturedRow]() {
            onAlgorithmConfigClicked(capturedRow);
        });
    }
}

void frmConfigIpc::loadDataFromApi()
{
    loadScenes();
}

void frmConfigIpc::refreshTable()
{
    loadDataFromApi();
}

void frmConfigIpc::addDevice(const QStringList &deviceInfo)
{
    int count = model->rowCount();

    // 从上一行继承默认值（如果有）
    int ipcID    = 0;  // 0 表示新记录，保存时由服务端自动分配
    QString ipcName   = count > 0 ? model->item(count - 1, COL_NAME)->text()    : "";
    QString nvrName   = count > 0 ? model->item(count - 1, COL_NVR)->text()     : "";
    QString ipcType   = count > 0 ? model->item(count - 1, COL_TYPE)->text()    : "other";
    QString onvifAddr = count > 0 ? model->item(count - 1, COL_ONVIF)->text()   : "";
    QString profileToken = count > 0 ? model->item(count - 1, COL_PROFILE)->text() : "";
    QString videoSource  = count > 0 ? model->item(count - 1, COL_SOURCE)->text()  : "";
    QString rtspMain  = count > 0 ? model->item(count - 1, COL_RTSP)->text()    : "";
    QString rtspSub   = count > 0 ? model->item(count - 1, COL_RTSPSUB)->text() : "";
    QString ipcPosition = count > 0 ? model->item(count - 1, COL_POS)->text()   : "";
    QString ipcImage  = count > 0 ? model->item(count - 1, COL_IMAGE)->text()   : "";
    int ipcX = count > 0 ? model->item(count - 1, COL_X)->text().toInt() : 5;
    int ipcY = count > 0 ? model->item(count - 1, COL_Y)->text().toInt() : 5;
    QString userName  = count > 0 ? model->item(count - 1, COL_USER)->text()    : "admin";
    QString userPwd   = count > 0 ? model->item(count - 1, COL_PWD)->text()     : "admin";
    QString ipcEnable = "启用";

    // 名称自动递增 #后面跟序号
    int idx = ipcName.indexOf("#");
    if (idx >= 0) {
        int number = ipcName.mid(idx + 1).toInt();
        ipcName = ipcName.mid(0, idx) + "#" + QString::number(number + 1);
    }

    // 本地码流地址递增
    idx = rtspMain.indexOf("/mp4/");
    if (idx >= 0) {
        QString end = rtspMain.mid(idx + 5);
        QStringList list = end.split(".");
        int number = list.first().toInt();
        rtspMain = rtspMain.mid(0, idx) + "/mp4/" + QString::number(number + 1) + "." + list.last();
        rtspSub  = rtspMain;
    }

    FormHelper::checkPosition(ipcX, ipcY, AppData::DeviceWidth, AppData::DeviceHeight);

    // 第一行默认值
    if (count == 0) {
        ipcName  = "摄像机#1";
        nvrName  = DbData::NvrInfo_NvrName.isEmpty() ? "录像机#1" : DbData::NvrInfo_NvrName.first();
        ipcType  = "other";
        rtspMain = "rtsp://192.168.1.128:554/0";
        rtspSub  = "rtsp://192.168.1.128:554/1";
        ipcPosition = AppConfig::MapCenter;
        ipcPosition.replace(",", "|");
        ipcImage = AppData::MapNames.size() > 0 ? AppData::MapNames.first() : " -- 无 -- ";
        ipcX = 5; ipcY = 5;
    }

    // 批量添加时覆盖字段
    if (deviceInfo.size() > 7) {
        userName     = deviceInfo.at(0);
        userPwd      = deviceInfo.at(1);
        ipcType      = deviceInfo.at(2);
        onvifAddr    = deviceInfo.at(3);
        profileToken = deviceInfo.at(4);
        videoSource  = deviceInfo.at(5);
        rtspMain     = deviceInfo.at(6);
        rtspSub      = deviceInfo.at(7);

        QString ip = UrlHelper::getUrlIP(onvifAddr);
        QStringList ips = ip.split(".");
        QString flag = ips.last();
        ipcName = flag.isEmpty() ? QString("视频文件#%1").arg(count + 1) : QString("摄像机#%1").arg(flag);

        if (ipcType.startsWith("NVR_Ch")) {
            QString ch = ipcType.split("_").at(1);
            QString channel = QString("%1").arg(ch.mid(2, 3).toInt(), 3, 10, QChar('0'));
            int addr = (ips.at(3).toInt() | ips.at(2).toInt() << 8);
            ipcID = QString("%1%2").arg(addr).arg(channel).toInt();
            ipcName = QString("通道%1").arg(channel);

            int nvrIdx = DbData::NvrInfo_NvrIP.indexOf(ip);
            if (nvrIdx >= 0) {
                nvrName = DbData::NvrInfo_NvrName.at(nvrIdx);
            } else {
                nvrName = QString("录像机#%1").arg(flag);
                QString nvrType = ipcType.endsWith("HIKVISION") ? "海康" :
                                  ipcType.endsWith("Dahua")     ? "大华" : "其他";
                DbQuery::addNvrInfo(ip, nvrName, nvrType);
                DbQuery::loadNvrInfo();
            }
        }
    }

    // 新增一行到 model
    QList<QStandardItem *> items;
    auto *idItem = new QStandardItem(ipcID > 0 ? QString::number(ipcID) : "0");
    // Qt::UserRole = true 标记为新行，保存时走 POST；false 为已有行，走 PUT
    idItem->setData(ipcID == 0, Qt::UserRole);
    items << idItem;
    items << new QStandardItem(ipcName);
    items << new QStandardItem(nvrName);
    items << new QStandardItem(ipcType);
    items << new QStandardItem(onvifAddr);
    items << new QStandardItem(profileToken);
    items << new QStandardItem(videoSource);
    items << new QStandardItem(rtspMain);
    items << new QStandardItem(rtspSub);
    items << new QStandardItem(ipcPosition);
    items << new QStandardItem(ipcImage);
    items << new QStandardItem(QString::number(ipcX));
    items << new QStandardItem(QString::number(ipcY));
    items << new QStandardItem(userName);
    items << new QStandardItem(userPwd);
    items << new QStandardItem(ipcEnable);
    items << new QStandardItem("未设置");
    items << new QStandardItem("");
    items << new QStandardItem("");
    model->appendRow(items);
}

void frmConfigIpc::addDevices(const QList<QStringList> &deviceInfos)
{
    int size = model->rowCount() + deviceInfos.size();

    for (const QStringList &deviceInfo : deviceInfos) {
        QString onvifAddr = deviceInfo.at(3);
        QString rtspMain  = deviceInfo.at(6);

        bool exist = false;
        for (int j = 0; j < DbData::IpcInfo_Count; ++j) {
            if (DbData::IpcInfo_OnvifAddr.at(j) == onvifAddr &&
                DbData::IpcInfo_RtspMain.at(j) == rtspMain) {
                exist = true;
                break;
            }
        }
        if (!exist) addDevice(deviceInfo);
    }
    on_btnSave_clicked();
}

void frmConfigIpc::addPlus(const QStringList &rtspMains, const QStringList &rtspSubs)
{
    int size = rtspMains.size();
    if (size == 0) return;

    for (int i = 0; i < size; ++i) {
        QString rtspMain = rtspMains.at(i);
        QString rtspSub  = rtspSubs.at(i);
        QString userName = "admin", userPwd = "12345";

        int idx = rtspMain.lastIndexOf("@");
        if (idx > 0) {
            QString userInfo = rtspMain.mid(0, idx);
            userInfo.replace("rtsp://", "");
            QStringList list = userInfo.split(":");
            userName = list.at(0);
            userPwd  = list.at(1);
        }

        QString ipcType = "other";
        QString ip = UrlHelper::getUrlIP(rtspMain);
        QString onvifAddr, profileToken, videoSource;
        if (AppConfig::PlusType == 1) {
            ipcType = "HIKVISION";
            onvifAddr = QString("http://%1/onvif/device_service").arg(ip);
            profileToken = "Profile_1"; videoSource = "VideoSource_1";
        } else if (AppConfig::PlusType == 2) {
            ipcType = "Dahua";
            onvifAddr = QString("http://%1/onvif/device_service").arg(ip);
            profileToken = "Profile000"; videoSource = "VideoSource000";
        }

        if (AppConfig::PlusNvr) {
            ipcType = QString("NVR_Ch%1_%2").arg(i + 1).arg(ipcType);
            if (AppConfig::PlusType == 1) {
                QString flag = QString("%1").arg(i + 1, 3, 10, QChar('0'));
                profileToken = "ProfileToken" + flag;
                videoSource  = "VideoSourceToken" + flag;
            } else if (AppConfig::PlusType == 2) {
                QString flag = QString("%100").arg(i, 3, 10, QChar('0'));
                profileToken = "MediaProfile" + flag;
                videoSource  = flag;
            }
        }

        QStringList deviceInfo;
        deviceInfo << userName << userPwd << ipcType << onvifAddr << profileToken << videoSource << rtspMain << rtspSub;
        addDevice(deviceInfo);
    }
    on_btnSave_clicked();
}

void frmConfigIpc::on_btnAdd_clicked()
{
    int count = model->rowCount();
    addDevice(QStringList());
    ui->tableView->setCurrentIndex(model->index(count, 0));
}

void frmConfigIpc::on_btnSave_clicked()
{
    ui->tableView->setFocus();

    struct SaveState {
        int total;
        int completed;
        bool hasError;
        frmConfigIpc *self;
    };

    auto state = std::make_shared<SaveState>();
    state->total = model->rowCount();
    state->completed = 0;
    state->hasError = false;
    state->self = this;

    if (state->total == 0) {
        loadDataFromApi();
        DbQuery::loadIpcInfo();
        AppEvent::Instance()->slot_saveIpcInfo(true);
        return;
    }

    for (int row = 0; row < model->rowCount(); ++row) {
        int ipcId = model->item(row, COL_ID)->text().toInt();

        QJsonObject body;
        body["ipc_name"]      = model->item(row, COL_NAME)->text();
        body["nvr_name"]      = model->item(row, COL_NVR)->text();
        body["ipc_type"]      = model->item(row, COL_TYPE)->text();
        body["onvif_addr"]    = model->item(row, COL_ONVIF)->text();
        body["profile_token"] = model->item(row, COL_PROFILE)->text();
        body["video_source"]  = model->item(row, COL_SOURCE)->text();
        body["rtsp_main"]     = model->item(row, COL_RTSP)->text();
        body["rtsp_sub"]      = model->item(row, COL_RTSPSUB)->text();
        body["ipc_position"]  = model->item(row, COL_POS)->text();
        body["ipc_image"]     = model->item(row, COL_IMAGE)->text();
        body["ipc_x"]         = model->item(row, COL_X)->text().toInt();
        body["ipc_y"]         = model->item(row, COL_Y)->text().toInt();
        body["user_name"]     = model->item(row, COL_USER)->text();
        body["user_pwd"]      = model->item(row, COL_PWD)->text();
        body["ipc_enable"]    = model->item(row, COL_ENABLE)->text();
        body["ipc_mark"]      = model->item(row, COL_MARK)->text();

        int sceneId = model->item(row, COL_SCENE)->data(Qt::UserRole).toInt();
        body["scene_id"] = sceneId > 0 ? sceneId : QJsonValue();

        QString algorithmIds = model->item(row, COL_ALGO)->data(Qt::UserRole).toString();
        body["algorithm_ids"] = algorithmIds.isEmpty() ? QJsonValue() : algorithmIds;

        bool isNewRow = model->item(row, COL_ID)->data(Qt::UserRole).toBool();

        if (isNewRow) {
            ApiClient::instance()->addIpc(body,
                [state](const QJsonObject &) {
                    state->completed++;
                    if (state->completed >= state->total) {
                        state->self->onAllSavesCompleted(state->hasError);
                    }
                },
                [state](const QString &err) {
                    qDebug() << TIMEMS << "addIpc error:" << err;
                    state->hasError = true;
                    state->completed++;
                    if (state->completed >= state->total) {
                        state->self->onAllSavesCompleted(state->hasError);
                    }
                });
        } else {
            ApiClient::instance()->updateIpc(ipcId, body,
                [state](const QJsonObject &) {
                    state->completed++;
                    if (state->completed >= state->total) {
                        state->self->onAllSavesCompleted(state->hasError);
                    }
                },
                [state](const QString &err) {
                    qDebug() << TIMEMS << "updateIpc error:" << err;
                    state->hasError = true;
                    state->completed++;
                    if (state->completed >= state->total) {
                        state->self->onAllSavesCompleted(state->hasError);
                    }
                });
        }
    }
}

void frmConfigIpc::onAllSavesCompleted(bool hasError)
{
    if (hasError)
        QUIHelper::showMessageBoxError("保存信息失败, 请检查服务器连接!");

    loadDataFromApi();
    DbQuery::loadIpcInfo();
    AppEvent::Instance()->slot_saveIpcInfo(true);
}

void frmConfigIpc::on_btnDelete_clicked()
{
    int row = ui->tableView->currentIndex().row();
    if (row < 0) {
        QUIHelper::showMessageBoxError("请选择要删除的行!");
        return;
    }

    if (QUIHelper::showMessageBoxQuestion("确定要删除选中的摄像机吗?\n摄像机对应的轮询信息都会被删除!") != QMessageBox::Yes)
        return;

    QStringList ids, addrs;
    QItemSelectionModel *selections = ui->tableView->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();
    foreach (QModelIndex index, selected) {
        if (index.column() == COL_ID)
            ids << index.data().toString();
        else if (index.column() == COL_RTSP)
            addrs << index.data().toString();
    }

    DbQuery::deleteIpcInfos(ids.join(","));
    DbQuery::deletePollInfos(addrs.join(","));
    AppEvent::Instance()->slot_saveIpcInfo(true);
    loadDataFromApi();
}

void frmConfigIpc::on_btnReturn_clicked()
{
    // 撤销未保存的更改：重新从服务端加载
    loadDataFromApi();
}

void frmConfigIpc::on_btnClear_clicked()
{
    if (model->rowCount() <= 0) return;

    if (QUIHelper::showMessageBoxQuestion("确定要清空所有信息吗? 清空后不能恢复!") != QMessageBox::Yes)
        return;

    // 逐条删除所有摄像头
    QStringList ids;
    for (int i = 0; i < model->rowCount(); ++i) {
        QString id = model->item(i, COL_ID)->text();
        if (!id.isEmpty() && id != "0") ids << id;
    }
    if (!ids.isEmpty())
        DbQuery::deleteIpcInfos(ids.join(","));

    DbQuery::loadIpcInfo();
    loadDataFromApi();
    DeviceHelper::initDeviceTree();
    DeviceHelper::initVideoIcon();
    AppEvent::Instance()->slot_saveIpcInfo();
}

void frmConfigIpc::on_btnInput_clicked()
{
    // 导入暂不支持 HTTP 模式，提示用户
    QUIHelper::showMessageBoxInfo("请通过服务端管理界面导入摄像机信息。");
}

void frmConfigIpc::on_btnOutput_clicked()
{
    FormHelper::outputData("IpcID asc", columnNames, "IpcInfo", "摄像机信息");
}

void frmConfigIpc::dataout(quint8 type)
{
    QList<QString> cols;
    QList<int> widths;
    cols   << "编号" << "名称" << "录像机" << "厂家" << "启用" << "备注";
    widths << 50    << 120   << 120     << 150   << 60   << 100;
    QString columns = "IpcID,IpcName,NvrName,IpcType,IpcEnable,IpcMark";
    QString where   = "order by IpcID asc";
    FormHelper::dataout(type, cols, widths, "摄像机信息", "IpcInfo", columns, where);
}

void frmConfigIpc::on_btnPrint_clicked() { dataout(0); }
void frmConfigIpc::on_btnXls_clicked()   { dataout(2); }

void frmConfigIpc::on_btnPlus_clicked()
{
    frmConfigPlus::Instance()->show();
}

void frmConfigIpc::on_btnSearch_clicked()
{
    ui->widgetIpcSearch->setVisible(!ui->widgetIpcSearch->isVisible());
    AppConfig::VisibleIpcSearch = ui->widgetIpcSearch->isVisible();
    AppConfig::writeConfig();
}

void frmConfigIpc::onAlgorithmConfigClicked(int row)
{
    if (row < 0 || row >= model->rowCount()) return;

    int ipcId = model->item(row, COL_ID)->text().toInt();
    int sceneId = model->item(row, COL_SCENE)->data(Qt::UserRole).toInt();
    QString algorithmIds = model->item(row, COL_ALGO)->data(Qt::UserRole).toString();

    DlgIpcAlgoConfig dlg(this);
    dlg.loadIpcConfig(ipcId, sceneId, algorithmIds);

    if (dlg.exec() == QDialog::Accepted) {
        int newSceneId = dlg.getSelectedSceneId();
        QString newAlgoIds = dlg.getSelectedAlgorithmIds();

        QJsonObject body;
        body["scene_id"] = newSceneId > 0 ? newSceneId : QJsonValue();
        body["algorithm_ids"] = newAlgoIds.isEmpty() ? QJsonValue() : newAlgoIds;

        ApiClient::instance()->updateIpc(ipcId, body,
            [this](const QJsonObject &) {
                QUIHelper::showMessageBoxInfo("算法配置保存成功！", 1);
                loadDataFromApi();
            },
            [](const QString &err) {
                qDebug() << TIMEMS << "updateIpc error:" << err;
                QUIHelper::showMessageBoxError("算法配置保存失败！");
            }
        );
    }
}


