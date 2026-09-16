#ifndef FRMSCENELIST_H
#define FRMSCENELIST_H

#include <QWidget>
#include <QJsonObject>
#include <QStandardItemModel>

namespace Ui {
class frmSceneList;
}

class frmSceneList : public QWidget
{
    Q_OBJECT

public:
    explicit frmSceneList(QWidget *parent = 0);
    ~frmSceneList();

    void refreshList();

signals:
    void showForm(const QString &mode, int id);

private:
    Ui::frmSceneList *ui;
    QStandardItemModel *model;

    QList<QJsonObject> m_allData;
    int m_currentPage;
    int m_totalPages;

    static const int PAGE_SIZE = 20;

    enum Column {
        COL_ID      = 0,
        COL_NAME    = 1,
        COL_DESC    = 2,
        COL_ALGOS   = 3,
        COL_OP      = 4,
        COL_COUNT   = 5
    };

private slots:
    void initForm();
    void initTable();
    void loadAllData();
    void showPage(int page);

    void on_btnAdd_clicked();
    void on_btnRefresh_clicked();
    void on_btnPrevPage_clicked();
    void on_btnNextPage_clicked();

    void onViewClicked(int row);
    void onEditClicked(int row);
    void onDeleteClicked(int row);
};

#endif // FRMSCENELIST_H
