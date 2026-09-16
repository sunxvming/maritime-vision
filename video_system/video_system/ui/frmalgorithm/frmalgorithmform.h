#ifndef FRMALGORITHMFORM_H
#define FRMALGORITHMFORM_H

#include <QWidget>

namespace Ui {
class frmAlgorithmForm;
}

class frmAlgorithmForm : public QWidget
{
    Q_OBJECT

public:
    explicit frmAlgorithmForm(QWidget *parent = 0);
    ~frmAlgorithmForm();

    // mode: "add" | "edit" | "view"
    void loadAlgorithm(const QString &mode, int id);

signals:
    void backToList();

private:
    Ui::frmAlgorithmForm *ui;

    QString m_mode;
    int     m_currentId;

    void fillForm(const QJsonObject &obj);
    void setReadOnly(bool readOnly);

private slots:
    void initForm();
    void on_btnBack_clicked();
    void on_btnSave_clicked();
};

#endif // FRMALGORITHMFORM_H
