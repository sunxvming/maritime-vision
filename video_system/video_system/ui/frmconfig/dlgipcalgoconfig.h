#ifndef DLGIPCALGOCONFIG_H
#define DLGIPCALGOCONFIG_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class DlgIpcAlgoConfig;
}

class DlgIpcAlgoConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DlgIpcAlgoConfig(QWidget *parent = 0);
    ~DlgIpcAlgoConfig();

    void loadIpcConfig(int ipcId, int sceneId, const QString &algorithmIds);
    QString getSelectedAlgorithmIds() const;
    int getSelectedSceneId() const;

private:
    Ui::DlgIpcAlgoConfig *ui;

    int m_ipcId;
    int m_selectedSceneId;
    QString m_pendingAlgorithmIds;
    bool m_settingSceneProgrammatically;
    QStandardItemModel *algoModel;
    QMap<int, QString> m_allAlgorithms;

    void fillAlgorithmTable(const QStringList &ids, const QJsonArray &allAlgos);

private slots:
    void initForm();
    void loadScenes();
    void loadAlgorithms();
    void onSceneLoadedForAlgorithms(const QJsonObject &sceneData);
    void on_cmbScene_currentIndexChanged(int index);
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
};

#endif // DLGIPCALGOCONFIG_H
