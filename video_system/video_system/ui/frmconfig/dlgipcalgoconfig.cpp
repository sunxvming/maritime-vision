#include "dlgipcalgoconfig.h"
#include "ui_dlgipcalgoconfig.h"
#include "api/ApiClient.h"
#include <QJsonArray>
#include <QMessageBox>
#include <QDebug>

#define TIMEMS qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))

DlgIpcAlgoConfig::DlgIpcAlgoConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgIpcAlgoConfig),
    m_ipcId(0),
    m_selectedSceneId(0),
    m_pendingAlgorithmIds(""),
    m_settingSceneProgrammatically(false)
{
    ui->setupUi(this);
    this->initForm();
}

DlgIpcAlgoConfig::~DlgIpcAlgoConfig()
{
    delete ui;
}

void DlgIpcAlgoConfig::initForm()
{
    setWindowTitle("算法配置");

    algoModel = new QStandardItemModel(this);
    QStringList headers;
    headers << "选择" << "算法名称";
    algoModel->setHorizontalHeaderLabels(headers);

    ui->tableAlgorithms->setModel(algoModel);
    ui->tableAlgorithms->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableAlgorithms->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableAlgorithms->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableAlgorithms->horizontalHeader()->setStretchLastSection(true);
    ui->tableAlgorithms->verticalHeader()->setVisible(false);
    ui->tableAlgorithms->setAlternatingRowColors(true);

    ui->tableAlgorithms->setColumnWidth(0, 80);
    ui->tableAlgorithms->setColumnWidth(1, 400);

    loadScenes();
}

void DlgIpcAlgoConfig::loadIpcConfig(int ipcId, int sceneId, const QString &algorithmIds)
{
    // Only store the values. The combo and checkboxes are set once loadScenes()
    // completes, so that we don't race against the async scene fetch.
    m_ipcId = ipcId;
    m_selectedSceneId = sceneId;
    m_pendingAlgorithmIds = algorithmIds;
}

void DlgIpcAlgoConfig::loadScenes()
{
    ui->cmbScene->clear();
    ui->cmbScene->addItem("未设置", 0);

    ApiClient::instance()->get("/api/v1/alarm_scene",
        [this](const QJsonObject &data) {
            QJsonArray sceneList = data.value("list").toArray();
            for (const QJsonValue &val : sceneList) {
                QJsonObject obj = val.toObject();
                int sceneId = obj["scene_id"].toInt();
                QString sceneName = obj["scene_name"].toString();
                ui->cmbScene->addItem(sceneName, sceneId);
            }

            // Now that the combo is populated, select the stored scene.
            // Use the flag so on_cmbScene_currentIndexChanged knows this is
            // a programmatic change and should not clear m_pendingAlgorithmIds.
            if (m_selectedSceneId > 0) {
                for (int i = 0; i < ui->cmbScene->count(); ++i) {
                    if (ui->cmbScene->itemData(i).toInt() == m_selectedSceneId) {
                        m_settingSceneProgrammatically = true;
                        ui->cmbScene->setCurrentIndex(i);
                        m_settingSceneProgrammatically = false;
                        break;
                    }
                }
            }
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "loadScenes error:" << err;
        }
    );
}

void DlgIpcAlgoConfig::loadAlgorithms()
{
    algoModel->setRowCount(0);
    if (m_selectedSceneId == 0) return;

    ApiClient::instance()->get(
        QString("/api/v1/alarm_scene/%1").arg(m_selectedSceneId),
        [this](const QJsonObject &data) {
            onSceneLoadedForAlgorithms(data);
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "loadScene error:" << err;
        }
    );
}

void DlgIpcAlgoConfig::onSceneLoadedForAlgorithms(const QJsonObject &sceneData)
{
    QString algorithmIds = sceneData["algorithm_ids"].toString();
    if (algorithmIds.isEmpty()) return;

    QStringList ids = algorithmIds.split(",", Qt::SkipEmptyParts);

    ApiClient::instance()->get("/api/v1/algorithms",
        [this, ids](const QJsonObject &data) {
            QJsonArray allAlgos = data.value("data").toArray();
            fillAlgorithmTable(ids, allAlgos);
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "loadAlgorithms error:" << err;
        }
    );
}

void DlgIpcAlgoConfig::fillAlgorithmTable(const QStringList &ids, const QJsonArray &allAlgos)
{
    QMap<int, QString> algoMap;
    for (const QJsonValue &val : allAlgos) {
        QJsonObject obj = val.toObject();
        algoMap[obj["id"].toInt()] = obj["name_cn"].toString();
    }

    for (const QString &idStr : ids) {
        int algoId = idStr.trimmed().toInt();
        if (!algoMap.contains(algoId)) continue;

        int row = algoModel->rowCount();
        algoModel->insertRow(row);

        QStandardItem *checkItem = new QStandardItem();
        checkItem->setCheckable(true);
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setData(algoId, Qt::UserRole);
        algoModel->setItem(row, 0, checkItem);

        QStandardItem *nameItem = new QStandardItem(algoMap[algoId]);
        nameItem->setEditable(false);
        algoModel->setItem(row, 1, nameItem);
    }

    // Apply stored selection. If there is a pending selection, check only those
    // IDs; otherwise check everything by default (new scene assignment).
    if (!m_pendingAlgorithmIds.isEmpty()) {
        QStringList selectedIds = m_pendingAlgorithmIds.split(",", Qt::SkipEmptyParts);
        QList<int> idList;
        for (const QString &s : selectedIds)
            idList.append(s.trimmed().toInt());

        for (int row = 0; row < algoModel->rowCount(); ++row) {
            QStandardItem *item = algoModel->item(row, 0);
            if (item) {
                int algoId = item->data(Qt::UserRole).toInt();
                item->setCheckState(idList.contains(algoId) ? Qt::Checked : Qt::Unchecked);
            }
        }
    } else {
        for (int row = 0; row < algoModel->rowCount(); ++row) {
            QStandardItem *item = algoModel->item(row, 0);
            if (item) item->setCheckState(Qt::Checked);
        }
    }
}

void DlgIpcAlgoConfig::on_cmbScene_currentIndexChanged(int index)
{
    if (index < 0) return;

    int sceneId = ui->cmbScene->itemData(index).toInt();
    m_selectedSceneId = sceneId;

    // When the user manually picks a different scene, discard the stored
    // algorithm selection so fillAlgorithmTable checks all by default.
    if (!m_settingSceneProgrammatically) {
        m_pendingAlgorithmIds = "";
    }

    loadAlgorithms();
}

QString DlgIpcAlgoConfig::getSelectedAlgorithmIds() const
{
    QStringList selectedIds;
    for (int row = 0; row < algoModel->rowCount(); ++row) {
        QStandardItem *item = algoModel->item(row, 0);
        if (item && item->checkState() == Qt::Checked) {
            int algoId = item->data(Qt::UserRole).toInt();
            selectedIds << QString::number(algoId);
        }
    }
    return selectedIds.join(",");
}

int DlgIpcAlgoConfig::getSelectedSceneId() const
{
    return m_selectedSceneId;
}

void DlgIpcAlgoConfig::on_btnOk_clicked()
{
    accept();
}

void DlgIpcAlgoConfig::on_btnCancel_clicked()
{
    reject();
}
