#ifndef FRMALARMLIST_H
#define FRMALARMLIST_H

#include <QWidget>
#include <QModelIndex>

class QStandardItemModel;

namespace Ui {
class frmAlarmList;
}

class frmAlarmList : public QWidget
{
    Q_OBJECT

public:
    explicit frmAlarmList(QWidget *parent = 0);
    ~frmAlarmList();

signals:
    void showDetail(int alarmId);

private:
    Ui::frmAlarmList *ui;

    QStandardItemModel *model;

    int currentPage;
    int totalPages;
    int totalCount;

    // column indices
    enum Column {
        COL_CHECK   = 0,
        COL_ID      = 1,
        COL_CAMERA  = 2,
        COL_ALGO    = 3,
        COL_TIME    = 4,
        COL_RISK    = 5,
        COL_STATUS  = 6,
        COL_OP      = 7,
        COL_COUNT   = 8
    };

private slots:
    void initForm();
    void initTable();

    void loadData();
    void loadPage(int page);
    void loadCameraList();
    void loadAlgorithmList();

    void onPageDataLoaded(const QJsonObject &respData);
    void onCameraListLoaded(const QJsonObject &respData);
    void onAlgorithmListLoaded(const QJsonObject &respData);
    void onBatchStatusCompleted(int failCount, int totalCount);

    void on_btnSearch_clicked();
    void on_btnReset_clicked();
    void on_btnBatchStatus_clicked();
    void on_btnPrevPage_clicked();
    void on_btnNextPage_clicked();
    void on_btnFirstPage_clicked();
    void on_btnLastPage_clicked();

    void on_btnSelectAll_clicked();
    void on_btnSelectNone_clicked();

    void onDetailClicked(int row);
    void onDeleteClicked(int row);
    void onStatusClicked(int row);

    void on_tableView_doubleClicked(const QModelIndex &index);
};

#endif // FRMALARMLIST_H
