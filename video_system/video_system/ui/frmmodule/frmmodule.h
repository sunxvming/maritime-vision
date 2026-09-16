#ifndef FRMMODULE_H
#define FRMMODULE_H

#include <QMainWindow>
#include <QMap>
#include <QList>

namespace Ui {
class frmModule;
}

class frmModule : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmModule(QWidget *parent = 0);
    ~frmModule();

protected:
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

private:
    Ui::frmModule *ui;

    // 停靠窗体容器：key -> QDockWidget*
    QMap<QString, QDockWidget *> dockWidgetMap;
    // 记录添加顺序，用于遍历时保持原有顺序
    QList<QString> dockOrder;
    // 每个停靠窗体的默认宽高
    QMap<QString, int> dockWidthMap;
    QMap<QString, int> dockHeightMap;

    // 切换到其他窗体时暂时隐藏的浮动窗体
    QList<QDockWidget *> hideWidgets;

private slots:
    void initForm();
    void initSize();
    void initMenu();

    void initWidget();
    QDockWidget *newWidget(QWidget *widget, const QString &title, int width, int height);

    void addWidget();
    void addWidget(const QString &key, int position);

    void changeWindowOpacity();
    void fullScreen(bool full);
    void fullScreen();

    QString getLayoutIni(bool full);
    void initLayout(bool full = false);
    void saveLayout(bool full = false);
    void resetLayout(bool full = false);    

    void visibilityChanged(bool visible);
    void visibilityChangedFromMain(const QString &title, bool visible);

signals:
    void visibilityChangedFromModule(const QString &title, bool visible);
    void loadModuleFinshed(const QList<QString> &titles, const QList<bool> &visibles);
};

#endif // FRMMODULE_H