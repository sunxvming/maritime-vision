#include "frmsceneform.h"
#include "ui_frmsceneform.h"
#include "quihelper.h"
#include "ApiClient.h"
#include <QJsonArray>
#include <QMessageBox>
#include <QDebug>
#include <QStandardItemModel>
#include <QHeaderView>

#define TIMEMS qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))

frmSceneForm::frmSceneForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmSceneForm),
    m_currentId(0)
{
    ui->setupUi(this);
    this->initForm();
}

frmSceneForm::~frmSceneForm()
{
    delete ui;
}

void frmSceneForm::initForm()
{
    // Setup algorithm table
    algoModel = new QStandardItemModel(this);
    QStringList headers;
    headers << "选择" << "算法名称";
    algoModel->setHorizontalHeaderLabels(headers);

    QUIHelper::initTableView(ui->tableAlgorithms, AppData::RowHeight, false, false);
    ui->tableAlgorithms->setModel(algoModel);
    ui->tableAlgorithms->horizontalHeader()->setStretchLastSection(true);

    ui->tableAlgorithms->setColumnWidth(0, 80);
    ui->tableAlgorithms->setColumnWidth(1, 400);

    connect(ui->btnBack, SIGNAL(clicked()), this, SLOT(on_btnBack_clicked()));
    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(on_btnSave_clicked()));
}

void frmSceneForm::loadScene(const QString &mode, int id)
{
    m_mode = mode;
    m_currentId = id;
    m_selectedAlgoIds.clear();

    ui->txtSceneName->clear();
    ui->txtDescription->clear();

    if (mode == "add") {
        ui->labTitle->setText("场景管理 - 新增");
        setReadOnly(false);
        ui->btnSave->setVisible(true);
        loadAlgorithms();
        return;
    }

    ApiClient::instance()->get(
        QString("/api/v1/alarm_scene/%1").arg(id),
        [this, mode](const QJsonObject &data) {
            fillForm(data);
            if (mode == "view") {
                ui->labTitle->setText("场景管理 - 查看");
                setReadOnly(true);
                ui->btnSave->setVisible(false);
            } else if (mode == "edit") {
                ui->labTitle->setText("场景管理 - 编辑");
                setReadOnly(false);
                ui->btnSave->setVisible(true);
            }
            loadAlgorithms();
        },
        [this](const QString &err) {
            qDebug() << TIMEMS << "loadScene error:" << err;
            QUIHelper::showMessageBoxError("加载场景数据失败！", 3);
            emit backToList();
        }
    );
}

void frmSceneForm::fillForm(const QJsonObject &obj)
{
    ui->txtSceneName->setText(obj["scene_name"].toString());
    ui->txtDescription->setPlainText(obj["scene_description"].toString());

    QString algorithmIds = obj["algorithm_ids"].toString();
    if (!algorithmIds.isEmpty()) {
        QStringList ids = algorithmIds.split(",", Qt::SkipEmptyParts);
        for (const QString &idStr : ids) {
            m_selectedAlgoIds.append(idStr.trimmed().toInt());
        }
    }
}

void frmSceneForm::setReadOnly(bool readOnly)
{
    ui->txtSceneName->setReadOnly(readOnly);
    ui->txtDescription->setReadOnly(readOnly);
    ui->tableAlgorithms->setEnabled(!readOnly);
}

void frmSceneForm::loadAlgorithms()
{
    ApiClient::instance()->get("/api/v1/algorithms",
        [this](const QJsonObject &data) {
            QJsonArray algoList;
            if (data.contains("data") && data.value("data").isArray()) {
                algoList = data.value("data").toArray();
            }
            onAlgorithmsLoaded(algoList);
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "loadAlgorithms error:" << err;
            QUIHelper::showMessageBoxError("加载算法列表失败！", 3);
        }
    );
}

void frmSceneForm::onAlgorithmsLoaded(const QJsonArray &algoList)
{
    algoModel->setRowCount(0);
    for (const QJsonValue &val : algoList) {
        QJsonObject obj = val.toObject();
        int row = algoModel->rowCount();
        algoModel->insertRow(row);

        int algoId = obj["id"].toInt();
        QString nameCn = obj["name_cn"].toString();

        QStandardItem *checkItem = new QStandardItem();
        checkItem->setCheckable(true);
        checkItem->setCheckState(
            m_selectedAlgoIds.contains(algoId) ? Qt::Checked : Qt::Unchecked
        );
        checkItem->setData(algoId, Qt::UserRole);
        algoModel->setItem(row, 0, checkItem);

        QStandardItem *nameItem = new QStandardItem(nameCn);
        nameItem->setEditable(false);
        algoModel->setItem(row, 1, nameItem);
    }
}

void frmSceneForm::on_btnBack_clicked()
{
    emit backToList();
}

void frmSceneForm::on_btnSave_clicked()
{
    QString sceneName = ui->txtSceneName->text().trimmed();
    if (sceneName.isEmpty()) {
        QUIHelper::showMessageBoxError("请输入场景名称！", 3);
        return;
    }

    QString sceneDescription = ui->txtDescription->toPlainText().trimmed();

    QStringList selectedIds;
    for (int row = 0; row < algoModel->rowCount(); ++row) {
        QStandardItem *item = algoModel->item(row, 0);
        if (item && item->checkState() == Qt::Checked) {
            int algoId = item->data(Qt::UserRole).toInt();
            selectedIds << QString::number(algoId);
        }
    }

    QJsonObject body;
    body["scene_name"] = sceneName;
    body["scene_description"] = sceneDescription;
    body["algorithm_ids"] = selectedIds.join(",");

    auto onSuccess = [this](const QJsonObject &) {
        QUIHelper::showMessageBoxInfo(m_mode == "add" ? "新增成功！" : "更新成功！", 1);
        emit backToList();
    };

    auto onFailed = [this](const QString &err) {
        qDebug() << TIMEMS << "saveScene error:" << err;
        QUIHelper::showMessageBoxError(m_mode == "add" ? "新增失败！" : "更新失败！", 3);
    };

    if (m_mode == "add") {
        ApiClient::instance()->post("/api/v1/alarm_scene", body, onSuccess, onFailed);
    } else if (m_mode == "edit") {
        ApiClient::instance()->put(
            QString("/api/v1/alarm_scene/%1").arg(m_currentId),
            body, onSuccess, onFailed
        );
    }
}
