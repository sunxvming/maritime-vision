#include "frmalarmlist.h"
#include "ui_frmalarmlist.h"
#include "quihelper.h"
#include "ApiClient.h"

#include <QStandardItemModel>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <memory>

static const int PAGE_SIZE = 20;

frmAlarmList::frmAlarmList(QWidget *parent) : QWidget(parent), ui(new Ui::frmAlarmList)
{
    ui->setupUi(this);
    currentPage = 1;
    totalPages  = 1;
    totalCount  = 0;
    this->initForm();
    this->initTable();
    this->loadPage(1);
}

frmAlarmList::~frmAlarmList()
{
    delete ui;
}

void frmAlarmList::initForm()
{
    // Default time range: last 7 days
    ui->dtEnd->setDateTime(QDateTime::currentDateTime());
    ui->dtStart->setDateTime(QDateTime::currentDateTime().addDays(-7));

    // Load camera and algorithm lists for dropdowns
    loadCameraList();
    loadAlgorithmList();

    // Note: Qt auto-connects slots named on_<objectName>_<signalName>
    // So we don't need manual connect() for btnSearch, btnReset, etc.

    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(on_tableView_doubleClicked(QModelIndex)));
}

void frmAlarmList::initTable()
{
    model = new QStandardItemModel(this);

    QStringList headers;
    headers << "选择" << "ID" << "摄像头" << "算法名称" << "报警时间" << "风险等级" << "状态" << "操作";
    model->setHorizontalHeaderLabels(headers);

    QUIHelper::initTableView(ui->tableView, AppData::RowHeight, false, false);
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    QList<int> colWidths;
    colWidths << 50 << 60 << 120 << 120 << 270 << 80 << 80 << 160;
    for (int i = 0; i < colWidths.size(); ++i) {
        ui->tableView->setColumnWidth(i, colWidths.at(i));
    }
}

void frmAlarmList::loadData()
{
    loadPage(currentPage);
}

void frmAlarmList::loadPage(int page)
{
    currentPage = page;

    QString path = QString("/api/v1/alarm-events?page=%1&page_size=%2").arg(page).arg(PAGE_SIZE);

    int cameraId = ui->cboxCamera->currentData().toInt();
    int algoId   = ui->cboxAlgo->currentData().toInt();
    QString status = ui->cboxStatus->currentText();

    if (cameraId > 0) path += QString("&camera_id=%1").arg(cameraId);
    if (algoId > 0)   path += QString("&algorithm_id=%1").arg(algoId);
    if (status != "全部") path += "&status=" + status;

    QString startTime = ui->dtStart->dateTime().toString("yyyy-MM-ddTHH:mm:ss");
    QString endTime   = ui->dtEnd->dateTime().toString("yyyy-MM-ddTHH:mm:ss");
    path += "&start_time=" + startTime + "&end_time=" + endTime;

    ApiClient::instance()->get(path,
        [this](const QJsonObject &data) {
            onPageDataLoaded(data);
        },
        [this](const QString &err) {
            qDebug() << TIMEMS << "loadAlarmEvents error:" << err;
            model->setRowCount(0);
            QUIHelper::showMessageBoxError("加载报警数据失败，请检查服务器连接！", 3);
        }
    );
}

void frmAlarmList::onPageDataLoaded(const QJsonObject &respData)
{
    model->setRowCount(0);

    totalCount = respData.value("total").toInt(0);
    totalPages = respData.value("total_pages").toInt(1);
    if (totalPages < 1) totalPages = 1;

    ui->labPageInfo->setText(QString("第 %1 / %2 页").arg(currentPage).arg(totalPages));
    ui->labTotal->setText(QString("共 %1 条").arg(totalCount));
    ui->btnPrevPage->setEnabled(currentPage > 1);
    ui->btnNextPage->setEnabled(currentPage < totalPages);
    ui->btnFirstPage->setEnabled(currentPage > 1);
    ui->btnLastPage->setEnabled(currentPage < totalPages);

    QJsonArray items = respData.value("items").toArray();
    for (const QJsonValue &val : items) {
        QJsonObject obj = val.toObject();
        int row = model->rowCount();
        model->insertRow(row);

        QStandardItem *checkItem = new QStandardItem();
        checkItem->setCheckable(true);
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setTextAlignment(Qt::AlignCenter);
        checkItem->setEditable(false);
        model->setItem(row, COL_CHECK, checkItem);

        auto setCell = [&](int col, const QString &text) {
            QStandardItem *item = new QStandardItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setEditable(false);
            model->setItem(row, col, item);
        };

        setCell(COL_ID,     QString::number(obj["id"].toInt()));
        setCell(COL_CAMERA, obj["camera_name"].toString());
        setCell(COL_ALGO,   obj["algorithm_name"].toString());

        QString alarmTime = obj["alarm_time"].toString();
        if (alarmTime.contains('T')) {
            QDateTime dt = QDateTime::fromString(alarmTime, Qt::ISODate);
            if (dt.isValid()) {
                alarmTime = dt.toString("yyyy-MM-dd HH:mm:ss");
            }
        } else if (alarmTime.contains('.')) {
            alarmTime = alarmTime.left(19);
        }
        setCell(COL_TIME,   alarmTime);
        setCell(COL_RISK,   obj["risk_level"].toString());
        setCell(COL_STATUS, obj["status"].toString());

        QWidget *opWidget = new QWidget;
        QHBoxLayout *opLayout = new QHBoxLayout(opWidget);
        opLayout->setContentsMargins(20, 2, 20, 2);
        opLayout->setSpacing(20);

        QPushButton *btnDetail = new QPushButton("详情");
        QPushButton *btnStatus = new QPushButton("标记已处理");
        QPushButton *btnDel    = new QPushButton("删除");

        btnDetail->setMaximumWidth(70);
        btnStatus->setMaximumWidth(120);
        btnDel->setMaximumWidth(60);

        opLayout->addWidget(btnDetail);
        opLayout->addWidget(btnStatus);
        opLayout->addWidget(btnDel);
        opLayout->addStretch();

        ui->tableView->setIndexWidget(model->index(row, COL_OP), opWidget);

        int capturedRow = row;
        connect(btnDetail, &QPushButton::clicked, this, [this, capturedRow]() {
            onDetailClicked(capturedRow);
        });
        connect(btnStatus, &QPushButton::clicked, this, [this, capturedRow]() {
            onStatusClicked(capturedRow);
        });
        connect(btnDel, &QPushButton::clicked, this, [this, capturedRow]() {
            onDeleteClicked(capturedRow);
        });
    }
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void frmAlarmList::on_btnSearch_clicked()
{
    loadPage(1);
}

void frmAlarmList::on_btnReset_clicked()
{
    ui->cboxCamera->setCurrentIndex(0);
    ui->cboxAlgo->setCurrentIndex(0);
    ui->cboxStatus->setCurrentIndex(0);
    ui->dtEnd->setDateTime(QDateTime::currentDateTime());
    ui->dtStart->setDateTime(QDateTime::currentDateTime().addDays(-7));
    loadPage(1);
}

void frmAlarmList::on_btnBatchStatus_clicked()
{
    QString targetStatus = ui->cboxBatchStatus->currentText();

    QList<int> selectedIds;
    for (int row = 0; row < model->rowCount(); ++row) {
        QStandardItem *checkItem = model->item(row, COL_CHECK);
        if (checkItem && checkItem->checkState() == Qt::Checked) {
            int id = model->item(row, COL_ID)->text().toInt();
            selectedIds << id;
        }
    }

    if (selectedIds.isEmpty()) {
        QUIHelper::showMessageBoxError("请先勾选要修改的记录！", 3);
        return;
    }

    if (QUIHelper::showMessageBoxQuestion(
            QString("确定将选中的 %1 条记录状态修改为「%2」吗？")
            .arg(selectedIds.size()).arg(targetStatus)) != QMessageBox::Yes) {
        return;
    }

    struct BatchState {
        int total;
        int completed;
        int failCount;
        frmAlarmList *self;
    };

    auto state = std::make_shared<BatchState>();
    state->total = selectedIds.size();
    state->completed = 0;
    state->failCount = 0;
    state->self = this;

    QJsonObject body;
    body["status"] = targetStatus;

    for (int id : selectedIds) {
        ApiClient::instance()->put(
            QString("/api/v1/alarm-events/%1/status").arg(id),
            body,
            [state](const QJsonObject &) {
                state->completed++;
                if (state->completed >= state->total) {
                    state->self->onBatchStatusCompleted(state->failCount, state->total);
                }
            },
            [state, id](const QString &err) {
                qDebug() << TIMEMS << "batchStatus error for id" << id << ":" << err;
                state->failCount++;
                state->completed++;
                if (state->completed >= state->total) {
                    state->self->onBatchStatusCompleted(state->failCount, state->total);
                }
            }
        );
    }
}

void frmAlarmList::onBatchStatusCompleted(int failCount, int totalCount)
{
    if (failCount > 0) {
        QUIHelper::showMessageBoxError(
            QString("批量修改完成，但有 %1 条记录修改失败！").arg(failCount), 3);
    } else {
        QUIHelper::showMessageBoxInfo(
            QString("批量修改成功，共更新 %1 条记录。").arg(totalCount), 3);
    }
    loadPage(currentPage);
}

void frmAlarmList::on_btnFirstPage_clicked()  { loadPage(1); }
void frmAlarmList::on_btnLastPage_clicked()   { loadPage(totalPages); }
void frmAlarmList::on_btnPrevPage_clicked()   { if (currentPage > 1) loadPage(currentPage - 1); }
void frmAlarmList::on_btnNextPage_clicked()   { if (currentPage < totalPages) loadPage(currentPage + 1); }

void frmAlarmList::on_btnSelectAll_clicked()
{
    for (int row = 0; row < model->rowCount(); ++row) {
        QStandardItem *item = model->item(row, COL_CHECK);
        if (item) item->setCheckState(Qt::Checked);
    }
}

void frmAlarmList::on_btnSelectNone_clicked()
{
    for (int row = 0; row < model->rowCount(); ++row) {
        QStandardItem *item = model->item(row, COL_CHECK);
        if (item) item->setCheckState(Qt::Unchecked);
    }
}

void frmAlarmList::onDetailClicked(int row)
{
    if (row < 0 || row >= model->rowCount()) return;
    int id = model->item(row, COL_ID)->text().toInt();
    emit showDetail(id);
}

void frmAlarmList::onStatusClicked(int row)
{
    if (row < 0 || row >= model->rowCount()) return;
    int id = model->item(row, COL_ID)->text().toInt();

    QJsonObject body;
    body["status"] = QString("已处理");

    ApiClient::instance()->put(
        QString("/api/v1/alarm-events/%1/status").arg(id),
        body,
        [this, row](const QJsonObject &) {
            model->item(row, COL_STATUS)->setText("已处理");
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "updateStatus error:" << err;
            QUIHelper::showMessageBoxError("修改状态失败！", 3);
        }
    );
}

void frmAlarmList::onDeleteClicked(int row)
{
    if (row < 0 || row >= model->rowCount()) return;
    int id = model->item(row, COL_ID)->text().toInt();

    if (QUIHelper::showMessageBoxQuestion("确定要删除该报警记录吗？") != QMessageBox::Yes)
        return;

    ApiClient::instance()->del(
        QString("/api/v1/alarm-events/%1").arg(id),
        [this, row](const QJsonObject &) {
            model->removeRow(row);
            --totalCount;
            ui->labTotal->setText(QString("共 %1 条").arg(totalCount));
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "deleteAlarm error:" << err;
            QUIHelper::showMessageBoxError("删除记录失败！", 3);
        }
    );
}

void frmAlarmList::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    onDetailClicked(index.row());
}

void frmAlarmList::loadCameraList()
{
    ApiClient::instance()->get("/api/v1/ipc",
        [this](const QJsonObject &data) {
            onCameraListLoaded(data);
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "loadCameraList error:" << err;
        }
    );
}

void frmAlarmList::onCameraListLoaded(const QJsonObject &respData)
{
    while (ui->cboxCamera->count() > 1) {
        ui->cboxCamera->removeItem(1);
    }

    QJsonArray items = respData.value("data").toArray();
    for (const QJsonValue &val : items) {
        QJsonObject obj = val.toObject();
        int id = obj["id"].toInt();
        QString name = obj["name"].toString();
        ui->cboxCamera->addItem(name, id);
    }
}

void frmAlarmList::loadAlgorithmList()
{
    ApiClient::instance()->get("/api/v1/algorithms",
        [this](const QJsonObject &data) {
            onAlgorithmListLoaded(data);
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "loadAlgorithmList error:" << err;
        }
    );
}

void frmAlarmList::onAlgorithmListLoaded(const QJsonObject &respData)
{
    while (ui->cboxAlgo->count() > 1) {
        ui->cboxAlgo->removeItem(1);
    }

    QJsonArray items = respData.value("data").toArray();
    for (const QJsonValue &val : items) {
        QJsonObject obj = val.toObject();
        int id = obj["id"].toInt();
        QString nameCn = obj["name_cn"].toString();
        ui->cboxAlgo->addItem(nameCn, id);
    }
}
