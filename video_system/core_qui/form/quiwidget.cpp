#include "quiwidget.h"
#include "ui_quiwidget.h"
#include "quihelper.h"

QUIWidget::QUIWidget(QWidget *parent) : QDialog(parent), ui(new Ui::QUIWidget)
{
    ui->setupUi(this);
    this->initForm();
}

QUIWidget::~QUIWidget()
{
    delete ui;
}

bool QUIWidget::eventFilter(QObject *watched, QEvent *event)
{
    //处理无边框窗体可移动
    static QPoint mousePoint;
    static bool mousePressed = false;

    if (watched == this) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->type() == QEvent::MouseButtonPress) {
            if (mouseEvent->button() == Qt::LeftButton) {
                mousePressed = true;
                mousePoint = mouseEvent->globalPos() - this->pos();
                return true;
            }
        } else if (mouseEvent->type() == QEvent::MouseButtonRelease) {
            mousePressed = false;
            return true;
        } else if (mouseEvent->type() == QEvent::MouseMove) {
            if (mousePressed) {
                this->move(mouseEvent->globalPos() - mousePoint);
                return true;
            }
        }
    } else if (watched == ui->QUIWidgetTitle) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            //只有当最大化按钮可见才说明可以最大化
            if (ui->btnMenu_Max->isVisible()) {
                this->on_btnMenu_Max_clicked();
            }
        }
    } else if (watched == mainWidget) {
        if (event->type() == QEvent::Hide) {
            this->hide();
        } else if (event->type() == QEvent::Close) {
            this->close();
        }
    }

    return QDialog::eventFilter(watched, event);
}

QSize QUIWidget::sizeHint() const
{
    return QSize(600, 450);
}

QSize QUIWidget::minimumSizeHint() const
{
    return QSize(200, 150);
}

void QUIWidget::initForm()
{
    //绑定事件过滤器监听鼠标移动
    this->installEventFilter(this);
    //设置阴影
    //QUIHelper::setFormShadow(this, ui->verticalLayout, QUIConfig::HighColor, 5, 15);
    //设置无边框
    QUIHelper::setFramelessForm(this, ui->QUIWidgetTitle, ui->QUILabIco, ui->btnMenu_Close, false, false, true);

    max = false;
    location = this->geometry();
    mainWidget = 0;

    //双击标题栏最大化切换
    ui->QUIWidgetTitle->installEventFilter(this);

    //添加换肤菜单
    QStringList styleNames, styleFiles;
    QUIStyle::getStyle(styleNames, styleFiles);

    //添加到动作分组中形成互斥效果
    actionGroup = new QActionGroup(this);
    int size = styleNames.size();
    for (int i = 0; i < size; ++i) {
        QAction *action = new QAction(this);
        //设置可选中前面有个勾勾
        action->setCheckable(true);
        action->setText(styleNames.at(i));
        action->setData(styleFiles.at(i));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(changeStyle()));
        ui->btnMenu->addAction(action);
        actionGroup->addAction(action);
    }

    //默认选择一种样式
    setQssChecked(":/qss/lightblue.css");
    //设置个默认值
    setWidgetData(QUIWidgetData());
}

void QUIWidget::changeStyle()
{
    QAction *action = (QAction *)sender();
    QString qssFile = action->data().toString();

    //有些应用可能只需要发送个换肤的信号给他就行
    if (widgetData.changedStyle) {
        QUIStyle::setStyleFile(qssFile);
    }

    emit changeStyle(qssFile);
}

void QUIWidget::on_btnMenu_Min_clicked()
{
    if (widgetData.minHide) {
        this->hide();
    } else {
        this->showMinimized();
    }
}

void QUIWidget::on_btnMenu_Max_clicked()
{
    if (max) {
        this->setGeometry(location);
        IconHelper::setIcon(ui->btnMenu_Max, QUIConfig::IconNormal, widgetData.iconSize);
    } else {
        location = this->geometry();
        this->setGeometry(QUIHelper::getScreenRect());
        IconHelper::setIcon(ui->btnMenu_Max, QUIConfig::IconMax, widgetData.iconSize);
    }

    max = !max;
    this->setProperty("canMove", !max);
}

void QUIWidget::on_btnMenu_Close_clicked()
{
    //先发送关闭信号
    emit closing();
    if (widgetData.exitAll) {
        mainWidget->close();
        this->close();
    }
}

void QUIWidget::setMainWidget(QWidget *mainWidget)
{
    //一个QUI窗体对象只能设置一个主窗体
    if (this->mainWidget == 0) {
        //将子窗体添加到布局
        ui->QUIWidgetMain->layout()->addWidget(mainWidget);
        //自动设置大小
        this->resize(mainWidget->width(), mainWidget->height() + ui->QUIWidgetTitle->height());
        this->mainWidget = mainWidget;
        //安装事件过滤器识别关闭隐藏等
        //mainWidget->installEventFilter(this);
        QUIHelper::setFormInCenter(this);
    }
}

void QUIWidget::setWidgetData(const QUIWidgetData &widgetData)
{
    this->widgetData = widgetData;

    ui->QUILabTitle->setText(widgetData.title);
    this->setWindowTitle(ui->QUILabTitle->text());
    ui->QUIWidgetTitle->setFixedHeight(widgetData.titleHeight);
    ui->QUILabTitle->setAlignment(widgetData.alignment);

    //设置置顶显示
    if (widgetData.onTop) {
        this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    } else {
        this->setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
    }

    //设置窗口阻塞模式
    if (widgetData.windowModal) {
        this->setWindowModality(Qt::WindowModal);
    } else {
        this->setWindowModality(Qt::NonModal);
    }

    //设置右下角可拉伸
    this->setSizeGripEnabled(widgetData.sizeGripEnabled);

    //设置按钮宽度
    int btnWidth = widgetData.btnWidth;
    ui->QUILabIco->setFixedWidth(btnWidth + 5);
    ui->btnMenu->setFixedWidth(btnWidth);
    ui->btnMenu_Min->setFixedWidth(btnWidth);
    ui->btnMenu_Max->setFixedWidth(btnWidth);
    ui->btnMenu_Close->setFixedWidth(btnWidth);

    //设置按钮图标
    int iconSize = widgetData.iconSize;
    IconHelper::setIcon(ui->QUILabIco, QUIConfig::IconMain, iconSize + 2);
    IconHelper::setIcon(ui->btnMenu, QUIConfig::IconMenu, iconSize);
    IconHelper::setIcon(ui->btnMenu_Min, QUIConfig::IconMin, iconSize);
    IconHelper::setIcon(ui->btnMenu_Max, QUIConfig::IconNormal, iconSize);
    IconHelper::setIcon(ui->btnMenu_Close, QUIConfig::IconClose, iconSize);

    //设置对应按钮是否可见
    ui->btnMenu->setVisible(widgetData.visibleMenu);
    ui->btnMenu_Min->setVisible(widgetData.visibleMin);
    ui->btnMenu_Max->setVisible(widgetData.visibleMax);
    ui->btnMenu_Close->setVisible(widgetData.visibleClose);
}

void QUIWidget::setQssChecked(const QString &qssFile)
{
    //选中默认的样式
    QList<QAction *> actions = actionGroup->actions();
    foreach (QAction *action, actions) {
        if (action->data().toString() == qssFile) {
            action->setChecked(true);
            break;
        }
    }
}
