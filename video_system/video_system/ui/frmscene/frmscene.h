#ifndef FRMSCENE_H
#define FRMSCENE_H

#include <QWidget>

namespace Ui {
class frmScene;
}

class frmSceneList;
class frmSceneForm;

class frmScene : public QWidget
{
    Q_OBJECT

public:
    explicit frmScene(QWidget *parent = 0);
    ~frmScene();

private:
    Ui::frmScene *ui;

    frmSceneList *pageList;
    frmSceneForm *pageForm;

private slots:
    void initForm();
    void initWidget();
    void showForm(const QString &mode, int id);
    void showList();
};

#endif // FRMSCENE_H
