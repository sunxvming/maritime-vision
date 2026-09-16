#ifndef FRMALGORITHMLIST_H
#define FRMALGORITHMLIST_H

#include <QWidget>
#include <QModelIndex>
#include <QJsonObject>

class QStandardItemModel;

namespace Ui {
class frmAlgorithmList;
}

class frmAlgorithmList : public QWidget
{
    Q_OBJECT

public:
    explicit frmAlgorithmList(QWidget *parent = 0);
    ~frmAlgorithmList();

    void refreshList();

signals:
    void showForm(const QString &mode, int id);

private:
    Ui::frmAlgorithmList *ui;

    QStandardItemModel *model;

    // All data loaded from server (client-side pagination)
    QList<QJsonObject> m_allData;

    int m_currentPage;
    int m_totalPages;

    static const int PAGE_SIZE = 20;

    enum Column {
        COL_ID       = 0,
        COL_NAME_CN  = 1,
        COL_NAME_EN  = 2,
        COL_MODEL    = 3,
        COL_BMODEL   = 4,
        COL_CONF     = 5,
        COL_RISK     = 6,
        COL_ENABLED  = 7,
        COL_OP       = 8,
        COL_COUNT    = 9
    };

private slots:
    void initForm();
    void initTable();

    void loadAllData();
    void showPage(int page);
    void onAlgorithmsLoaded(const QJsonObject &respData);

    void on_btnAdd_clicked();
    void on_btnRefresh_clicked();
    void on_btnPrevPage_clicked();
    void on_btnNextPage_clicked();

    void onViewClicked(int row);
    void onEditClicked(int row);
    void onDeleteClicked(int row);
};

#endif // FRMALGORITHMLIST_H
