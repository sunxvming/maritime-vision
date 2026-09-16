#include "frmalgorithmlist.h"
#include "ui_frmalgorithmlist.h"
#include "quihelper.h"
#include "ApiClient.h"

#include <QStandardItemModel>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

frmAlgorithmList::frmAlgorithmList(QWidget *parent) : QWidget(parent), ui(new Ui::frmAlgorithmList)
{
    ui->setupUi(this);
    m_currentPage = 1;
    m_totalPages  = 1;
    this->initForm();
    this->initTable();
    this->loadAllData();
}

frmAlgorithmList::~frmAlgorithmList()
{
    delete ui;
}

void frmAlgorithmList::initForm()
{
    connect(ui->btnAdd,      SIGNAL(clicked()), this, SLOT(on_btnAdd_clicked()));
    connect(ui->btnRefresh,  SIGNAL(clicked()), this, SLOT(on_btnRefresh_clicked()));
    connect(ui->btnPrevPage, SIGNAL(clicked()), this, SLOT(on_btnPrevPage_clicked()));
    connect(ui->btnNextPage, SIGNAL(clicked()), this, SLOT(on_btnNextPage_clicked()));
}

void frmAlgorithmList::initTable()
{
    model = new QStandardItemModel(this);

    QStringList headers;
    headers << "ID" << "中文名称" << "英文名称" << "模型路径" << "BModel路径" << "置信度" << "风险等级" << "启用状态" << "操作";
    model->setHorizontalHeaderLabels(headers);

    QUIHelper::initTableView(ui->tableView, AppData::RowHeight, false, false);
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    QList<int> colWidths;
    colWidths << 60 << 200 << 180 << 300 << 450 << 80 << 80 << 80 << 180;
    for (int i = 0; i < colWidths.size(); ++i) {
        ui->tableView->setColumnWidth(i, colWidths.at(i));
    }
}

void frmAlgorithmList::loadAllData()
{
    ApiClient::instance()->get("/api/v1/algorithms",
        [this](const QJsonObject &data) {
            onAlgorithmsLoaded(data);
        },
        [this](const QString &err) {
            qDebug() << TIMEMS << "loadAlgorithms error:" << err;
            m_allData.clear();
            QUIHelper::showMessageBoxError("加载算法数据失败，请检查服务器连接！", 3);
        }
    );
}

void frmAlgorithmList::onAlgorithmsLoaded(const QJsonObject &respData)
{
    m_allData.clear();

    QJsonArray items;
    if (respData.contains("data") && respData.value("data").isArray()) {
        items = respData.value("data").toArray();
    } else {
        qDebug() << TIMEMS << "Unexpected response format:" << respData;
    }

    for (const QJsonValue &val : items) {
        if (val.isObject()) {
            m_allData.append(val.toObject());
        }
    }

    int totalCount = m_allData.size();
    m_totalPages = (totalCount + PAGE_SIZE - 1) / PAGE_SIZE;
    if (m_totalPages < 1) m_totalPages = 1;

    ui->labTotal->setText(QString("共 %1 条").arg(totalCount));
    showPage(1);
}

void frmAlgorithmList::showPage(int page)
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

        setCell(COL_ID,      QString::number(obj["id"].toInt()));
        setCell(COL_NAME_CN, obj["name_cn"].toString());
        setCell(COL_NAME_EN, obj["name_en"].toString());
        setCell(COL_MODEL,   obj["model_path"].toString());
        setCell(COL_BMODEL,  obj["bmodel_path"].toString());
        setCell(COL_CONF,    QString::number(obj["confidence_threshold"].toDouble(), 'f', 2));
        setCell(COL_RISK,    obj["risk_level"].toString());
        setCell(COL_ENABLED, obj["enabled"].toInt() ? "是" : "否");

        // COL_OP — action buttons widget
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

        // Capture row index by value via lambda
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

void frmAlgorithmList::refreshList()
{
    loadAllData();
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void frmAlgorithmList::on_btnAdd_clicked()
{
    emit showForm("add", 0);
}

void frmAlgorithmList::on_btnRefresh_clicked()
{
    loadAllData();
}

void frmAlgorithmList::on_btnPrevPage_clicked()
{
    if (m_currentPage > 1) {
        showPage(m_currentPage - 1);
    }
}

void frmAlgorithmList::on_btnNextPage_clicked()
{
    if (m_currentPage < m_totalPages) {
        showPage(m_currentPage + 1);
    }
}

void frmAlgorithmList::onViewClicked(int row)
{
    if (row < 0 || row >= model->rowCount()) return;
    int id = model->item(row, COL_ID)->text().toInt();
    emit showForm("view", id);
}

void frmAlgorithmList::onEditClicked(int row)
{
    if (row < 0 || row >= model->rowCount()) return;
    int id = model->item(row, COL_ID)->text().toInt();
    emit showForm("edit", id);
}

void frmAlgorithmList::onDeleteClicked(int row)
{
    if (row < 0 || row >= model->rowCount()) return;
    int id = model->item(row, COL_ID)->text().toInt();

    if (QUIHelper::showMessageBoxQuestion("确定要删除该算法吗？") != QMessageBox::Yes)
        return;

    ApiClient::instance()->del(
        QString("/api/v1/algorithms/%1").arg(id),
        [this](const QJsonObject &) {
            QUIHelper::showMessageBoxInfo("删除成功！", 2);
            loadAllData();
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "deleteAlgorithm error:" << err;
            QUIHelper::showMessageBoxError("删除失败！", 3);
        }
    );
}
