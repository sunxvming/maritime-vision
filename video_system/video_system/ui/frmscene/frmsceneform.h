#ifndef FRMSCENEFORM_H
#define FRMSCENEFORM_H

#include <QWidget>
#include <QJsonObject>
#include <QStandardItemModel>

namespace Ui {
class frmSceneForm;
}

class frmSceneForm : public QWidget
{
    Q_OBJECT

public:
    explicit frmSceneForm(QWidget *parent = 0);
    ~frmSceneForm();

    void loadScene(const QString &mode, int id);

signals:
    void backToList();

private:
    Ui::frmSceneForm *ui;

    QString m_mode;              // "add", "edit", "view"
    int m_currentId;

    QStandardItemModel *algoModel;
    QList<int> m_selectedAlgoIds;

    void fillForm(const QJsonObject &obj);
    void setReadOnly(bool readOnly);
    void loadAlgorithms();
    void setAlgorithmSelection(const QString &algorithmIds);
    void onAlgorithmsLoaded(const QJsonArray &algoList);

private slots:
    void initForm();
    void on_btnBack_clicked();
    void on_btnSave_clicked();
};

#endif // FRMSCENEFORM_H
