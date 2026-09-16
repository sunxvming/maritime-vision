#include "frmmodule.h"
#include "ui_frmmodule.h"
#include "quihelper.h"
#include "customtitlebar.h"
#include "frmvideopanel.h"

#include "frmdevicetree.h"
#include "frmmsglist.h"
#include "frmmsgtable.h"

#include "frmipcptz.h"
#include "frmipccontrol.h"
#include "frmipcpreset.h"

frmModule::frmModule(QWidget *parent) : QMainWindow(parent), ui(new Ui::frmModule)
{
    ui->setupUi(this);
    this->initForm();
    this->initWidget();
    this->addWidget();
}

frmModule::~frmModule()
{
    delete ui;
}

void frmModule::showEvent(QShowEvent *)
{
    static bool isLoad = false;
    if (!isLoad) {
        isLoad = true;
        this->initSize();
        QTimer::singleShot(100, this, SLOT(initLayout()));
        QTimer::singleShot(1000, this, SLOT(changeWindowOpacity()));
        return;
    }

    foreach (QDockWidget *dockWidget, hideWidgets) {
        dockWidget->setVisible(true);
    }
}

void frmModule::hideEvent(QHideEvent *)
{
    hideWidgets.clear();
    foreach (QDockWidget *dockWidget, dockWidgetMap.values()) {
        if (dockWidget->isVisible() && dockWidget->isFloating()) {
            dockWidget->setVisible(false);
            hideWidgets << dockWidget;
        }
    }
}

void frmModule::initForm()
{
    connect(AppEvent::Instance(), SIGNAL(exitAll()), this, SLOT(saveLayout()));
    connect(AppEvent::Instance(), SIGNAL(changeWindowOpacity()), this, SLOT(changeWindowOpacity()));
    connect(AppEvent::Instance(), SIGNAL(fullScreen(bool)), this, SLOT(fullScreen(bool)));
}

void frmModule::initSize()
{
    if (dockOrder.isEmpty())
        return;

    QList<QDockWidget *> widgets;
    QList<int> widths, heights;
    foreach (const QString &key, dockOrder) {
        widgets << dockWidgetMap[key];
        widths << dockWidthMap[key];
        heights << dockHeightMap[key];
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5,6,0))
    this->resizeDocks(widgets, widths, Qt::Horizontal);
    this->resizeDocks(widgets, heights, Qt::Vertical);
#endif
}

void frmModule::initMenu()
{
    QList<QString> titles;
    QList<bool> visibles;
    foreach (const QString &key, dockOrder) {
        QDockWidget *dock = dockWidgetMap[key];
        titles << dock->windowTitle();
        visibles << dock->isVisible();
    }
    emit loadModuleFinshed(titles, visibles);
}

void frmModule::initWidget()
{
    frmVideoPanel *videoPanel = new frmVideoPanel;
    AppData::videoPanel = videoPanel;
    videoPanel->setObjectName("centralWidget_frmVideoPanel");
    this->setCentralWidget(videoPanel);

    // 清空容器
    dockWidgetMap.clear();
    dockOrder.clear();
    dockWidthMap.clear();
    dockHeightMap.clear();

    int width = 230;
    int height = 500;

    newWidget(new frmMsgList, "图文警情", width, height);
    newWidget(new frmMsgTable, "窗口信息", width, height);
    newWidget(new frmDeviceTree, "设备列表", width, height);
    // newWidget(new frmIpcPtz, "云台控制", width, height);
    // newWidget(new frmIpcControl, "设备控制", width, height);
    // newWidget(new frmIpcPreset, "预置巡航", width, height);
}

QDockWidget *frmModule::newWidget(QWidget *widget, const QString &title, int width, int height)
{
    QString objName = widget->objectName();
    QDockWidget *dockWidget = new QDockWidget;
    dockWidget->setObjectName("dockWidget_" + objName);
    dockWidget->setWindowTitle(title);
    dockWidget->setWidget(widget);

    CustomTitleBar *titleBar = new CustomTitleBar;
    titleBar->setObjectName("titleBar_" + objName);
    titleBar->setFull(false);
    titleBar->setTitle(title);
    dockWidget->setTitleBarWidget(titleBar);
    connect(dockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(visibilityChanged(bool)));

    if (!UserHelper::checkPermission("调整布局")) {
        dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    }
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    // 以 title 作为 key 存储
    dockWidgetMap[title] = dockWidget;
    dockOrder.append(title);
    dockWidthMap[title] = width;
    dockHeightMap[title] = height;

    return dockWidget;
}

void frmModule::addWidget()
{
    // 左侧停靠
    addWidget("设备列表", 0);
    addWidget("图文警情", 0);
    addWidget("窗口信息", 0);

    // 右侧停靠
    // addWidget("云台控制", 1);
    // addWidget("设备控制", 1);
    // addWidget("预置巡航", 1);

    // 合并选项卡
    // tabifyDockWidget(dockWidgetMap["图文警情"], dockWidgetMap["窗口信息"]);
    // tabifyDockWidget(dockWidgetMap["云台控制"], dockWidgetMap["设备控制"]);
    // tabifyDockWidget(dockWidgetMap["设备控制"], dockWidgetMap["预置巡航"]);

    // 激活默认选项卡
    dockWidgetMap["图文警情"]->raise();
    // dockWidgetMap["云台控制"]->raise();
}

void frmModule::addWidget(const QString &key, int position)
{
    if (!dockWidgetMap.contains(key))
        return;

    QDockWidget *widget = dockWidgetMap[key];
    Qt::DockWidgetArea area;
    switch (position) {
        case 0: area = Qt::LeftDockWidgetArea; break;
        case 1: area = Qt::RightDockWidgetArea; break;
        case 2: area = Qt::TopDockWidgetArea; break;
        case 3: area = Qt::BottomDockWidgetArea; break;
        case 4: area = Qt::AllDockWidgetAreas; break;
        default: area = Qt::NoDockWidgetArea; break;
    }
    this->addDockWidget(area, widget);
}


void frmModule::changeWindowOpacity()
{
    foreach (QDockWidget *dock, dockWidgetMap.values()) {
        dock->setWindowOpacity((qreal)AppConfig::WindowOpacity / 100);
    }
}

void frmModule::fullScreen(bool full)
{
    this->saveLayout(!AppConfig::FormFull);
    QTimer::singleShot(200, this, SLOT(fullScreen()));
}

void frmModule::fullScreen()
{
    this->initLayout(AppConfig::FormFull);
}

QString frmModule::getLayoutIni(bool full)
{
    QString flag = full ? "full" : "normal";
    QString file = QString("%1/layout/video_workmode%2_%3.ini").arg(QUIHelper::appPath()).arg(AppConfig::WorkMode).arg(flag);
    return file;
}

void frmModule::initLayout(bool full)
{
    QString file = getLayoutIni(full);
    QByteArray data = AppConfig::readLayout(file);
    this->restoreState(data);
    this->initMenu();
}

void frmModule::saveLayout(bool full)
{
    if (!this->isVisible())
        return;

    QString file = getLayoutIni(full);
    QByteArray data = this->saveState();
    AppConfig::writeLayout(file, data);
}

void frmModule::resetLayout(bool full)
{
    QFile(getLayoutIni(full)).remove();

    foreach (QDockWidget *dock, dockWidgetMap.values()) {
        dock->setVisible(true);
        dock->setFloating(false);
    }

    this->initSize();
    this->saveLayout(full);
}

void frmModule::visibilityChanged(bool visible)
{
    QDockWidget *dockWidget = (QDockWidget *)sender();
    emit visibilityChangedFromModule(dockWidget->windowTitle(), dockWidget->isVisible());
}

void frmModule::visibilityChangedFromMain(const QString &title, bool visible)
{
    if (!this->isVisible())
        return;

    if (title.endsWith("当前布局")) {
        this->saveLayout();
    } else if (title.endsWith("所有模块")) {
        foreach (QDockWidget *dock, dockWidgetMap.values()) {
            dock->setVisible(visible);
        }
    } else if (title.endsWith("普通布局")) {
        resetLayout(false);
    } else if (title.endsWith("全屏布局")) {
        resetLayout(true);
    } else {
        if (dockWidgetMap.contains(title)) {
            dockWidgetMap[title]->setVisible(visible);
        }
    }
}