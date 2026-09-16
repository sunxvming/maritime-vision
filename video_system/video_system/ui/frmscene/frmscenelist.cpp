#include "frmscenelist.h"
#include "ui_frmscenelist.h"
#include "quihelper.h"
#include "ApiClient.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDebug>
#include <QStandardItemModel>
#include <QHeaderView>

#define TIMEMS qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))

frmSceneList::frmSceneList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmSceneList),
    m_currentPage(1),
    m_totalPages(1)
{
    ui->setupUi(this);
    this->initForm();
    this->initTable();
    this->loadAllData();
}

frmSceneList::~frmSceneList()
{
    delete ui;
}

void frmSceneList::initForm()
{
    connect(ui->btnAdd,      SIGNAL(clicked()), this, SLOT(on_btnAdd_clicked()));
    connect(ui->btnRefresh,  SIGNAL(clicked()), this, SLOT(on_btnRefresh_clicked()));
    connect(ui->btnPrevPage, SIGNAL(clicked()), this, SLOT(on_btnPrevPage_clicked()));
    connect(ui->btnNextPage, SIGNAL(clicked()), this, SLOT(on_btnNextPage_clicked()));
}

void frmSceneList::initTable()
{
    model = new QStandardItemModel(this);
    QStringList headers;
    headers << "ID" << "场景名称" << "描述" << "算法数量" << "操作";
    model->setHorizontalHeaderLabels(headers);

    QUIHelper::initTableView(ui->tableView, AppData::RowHeight, false, false);
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    // Set column widths
    QList<int> colWidths;
    colWidths << 60 << 200 << 300 << 100 << 200;
    for (int i = 0; i < colWidths.size() && i < COL_COUNT; ++i) {
        ui->tableView->setColumnWidth(i, colWidths.at(i));
    }
}

void frmSceneList::refreshList()
{
    loadAllData();
}

void frmSceneList::loadAllData()
{
    qDebug() << TIMEMS << "loadAllData: Starting to load scenes...";

    ApiClient::instance()->get("/api/v1/alarm_scene",
        [this](const QJsonObject &data) {
            qDebug() << TIMEMS << "loadAllData: Success callback, data:" << data;
            m_allData.clear();
            QJsonArray items = data.value("list").toArray();
            qDebug() << TIMEMS << "loadAllData: Parsed" << items.size() << "items";

            for (const QJsonValue &val : items) {
                if (val.isObject()) {
                    m_allData.append(val.toObject());
                }
            }

            int totalCount = m_allData.size();
            m_totalPages = (totalCount + PAGE_SIZE - 1) / PAGE_SIZE;
            if (m_totalPages < 1) m_totalPages = 1;

            ui->labTotal->setText(QString("共 %1 条").arg(totalCount));
            qDebug() << TIMEMS << "loadAllData: Total" << totalCount << "scenes, showing page 1";
            showPage(1);
        },
        [this](const QString &err) {
            qDebug() << TIMEMS << "loadScenes error:" << err;
            m_allData.clear();
            QUIHelper::showMessageBoxError("加载场景数据失败，请检查服务器连接！", 3);
        }
    );
}

void frmSceneList::showPage(int page)
{
    if (page < 1) page = 1;
    if (page > m_totalPages) page = m_totalPages;
    m_currentPage = page;

    model->setRowCount(0);

    // Update pagination controls
    ui->labPageInfo->setText(QString("第 %1 / %2 页").arg(m_currentPage).arg(m_totalPages));
    ui->btnPrevPage->setEnabled(m_currentPage > 1);
    ui->btnNextPage->setEnabled(m_currentPage < m_totalPages);

    // Calculate range
    int startIdx = (m_currentPage - 1) * PAGE_SIZE;
    int endIdx = qMin(startIdx + PAGE_SIZE, m_allData.size());

    // Fill table
    for (int i = startIdx; i < endIdx; ++i) {
        QJsonObject obj = m_allData.at(i);
        int row = model->rowCount();
        model->insertRow(row);

        auto setCell = [&](int col, const QString &text) {
            QStandardItem *item = new QStandardItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setEditable(false);
            model->setItem(row, col, item);
        };

        setCell(COL_ID, QString::number(obj["scene_id"].toInt()));
        setCell(COL_NAME, obj["scene_name"].toString());
        setCell(COL_DESC, obj["scene_description"].toString());

        // COL_ALGOS - count algorithms
        QString algoIds = obj["algorithm_ids"].toString();
        int algoCount = 0;
        if (!algoIds.isEmpty()) {
            QStringList ids = algoIds.split(",", Qt::SkipEmptyParts);
            algoCount = ids.size();
        }
        setCell(COL_ALGOS, QString::number(algoCount));

        // COL_OP - operation buttons widget
        QWidget *opWidget = new QWidget;
        QHBoxLayout *opLayout = new QHBoxLayout(opWidget);
        opLayout->setContentsMargins(20, 2, 20, 2);
        opLayout->setSpacing(20);

        QPushButton *btnView   = new QPushButton("查看");
        QPushButton *btnEdit   = new QPushButton("编辑");
        QPushButton *btnDelete = new QPushButton("删除");

        btnView->setMaximumWidth(60);
        btnEdit->setMaximumWidth(60);
        btnDelete->setMaximumWidth(60);

        opLayout->addWidget(btnView);
        opLayout->addWidget(btnEdit);
        opLayout->addWidget(btnDelete);
        opLayout->addStretch();

        ui->tableView->setIndexWidget(model->index(row, COL_OP), opWidget);

        // Connect signals with row capture
        int capturedRow = row;
        connect(btnView, &QPushButton::clicked, this, [this, capturedRow]() {
            onViewClicked(capturedRow);
        });
        connect(btnEdit, &QPushButton::clicked, this, [this, capturedRow]() {
            onEditClicked(capturedRow);
        });
        connect(btnDelete, &QPushButton::clicked, this, [this, capturedRow]() {
            onDeleteClicked(capturedRow);
        });
    }
}

// PLACEHOLDER_SLOTS

void frmSceneList::on_btnAdd_clicked()
{
    emit showForm("add", 0);
}

void frmSceneList::on_btnRefresh_clicked()
{
    refreshList();
}

void frmSceneList::on_btnPrevPage_clicked()
{
    showPage(m_currentPage - 1);
}

void frmSceneList::on_btnNextPage_clicked()
{
    showPage(m_currentPage + 1);
}

void frmSceneList::onViewClicked(int row)
{
    QStandardItem *itemId = model->item(row, COL_ID);
    if (!itemId) return;
    int sceneId = itemId->text().toInt();
    emit showForm("view", sceneId);
}

void frmSceneList::onEditClicked(int row)
{
    QStandardItem *itemId = model->item(row, COL_ID);
    if (!itemId) return;
    int sceneId = itemId->text().toInt();
    emit showForm("edit", sceneId);
}

void frmSceneList::onDeleteClicked(int row)
{
    QStandardItem *itemId = model->item(row, COL_ID);
    if (!itemId) return;

    QStandardItem *itemName = model->item(row, COL_NAME);
    QString sceneName = itemName ? itemName->text() : "";

    int ret = QUIHelper::showMessageBoxQuestion(QString("确定要删除场景 \"%1\" 吗？").arg(sceneName));
    if (ret != QMessageBox::Yes) {
        return;
    }

    int sceneId = itemId->text().toInt();

    ApiClient::instance()->del(
        QString("/api/v1/alarm_scene/%1").arg(sceneId),
        [this](const QJsonObject &) {
            QUIHelper::showMessageBoxInfo("删除成功！", 1);
            refreshList();
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "deleteScene error:" << err;
            QUIHelper::showMessageBoxError("删除失败！", 3);
        }
    );
}

