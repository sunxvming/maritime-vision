#ifndef FRMALGORITHM_H
#define FRMALGORITHM_H

#include <QWidget>

namespace Ui {
class frmAlgorithm;
}

class frmAlgorithmList;
class frmAlgorithmForm;

class frmAlgorithm : public QWidget
{
    Q_OBJECT

public:
    explicit frmAlgorithm(QWidget *parent = 0);
    ~frmAlgorithm();

private:
    Ui::frmAlgorithm *ui;

    frmAlgorithmList *pageList;
    frmAlgorithmForm *pageForm;

private slots:
    void initForm();
    void initWidget();
    void showForm(const QString &mode, int id);
    void showList();
};

#endif // FRMALGORITHM_H
