#include "frmvideopanel.h"
#include "ui_frmvideopanel.h"
#include "quihelper.h"
#include "AiTcpClient.h"
#include "dbquery.h"
#include "devicehelper.h"
#include "deviceonvif.h"
#include "videobox.h"
#include "videomanage.h"
#include "frmvideopaneltool.h"
#include "frmvideowidget.h"

frmVideoPanel::frmVideoPanel(QWidget *parent) : QWidget(parent), ui(new Ui::frmVideoPanel)
{
    ui->setupUi(this);
    this->initForm();
    this->initIcon();
    this->initVideo();
    this->initVideoManage();
    this->initOther();
    this->initMenu();
    play_video_all();
}

frmVideoPanel::~frmVideoPanel()
{
    delete ui;
}

void frmVideoPanel::mousePressEvent(QMouseEvent *e)
{
    //没有启用视频拖动则不处理
    if (!AppConfig::VideoDrag || AppConfig::WorkMode == 3) {
        return;
    }

    //过滤下如果按下的是底部工具栏则不处理
    if (!videoPanelTool->geometry().contains(e->pos())) {
        videoPressed = true;
        videoMove = false;
    }
}

void frmVideoPanel::mouseReleaseEvent(QMouseEvent *)
{
    if (!videoMove) {
        videoPressed = false;
        videoMove = false;
        videoSwap = 0;
        this->setCursor(Qt::ArrowCursor);
        return;
    }

    //如果从所有视频画面中没有找到则说明移出到窗体外
    if (!videoExist) {
        videoSelect->stop();
        int index = videoWidgets.indexOf(videoSelect);
        AppData::VideoUrls[index] = "";
        DeviceHelper::saveUrls();
        DeviceHelper::addMsg(QString("%1 删除视频").arg(videoSelect->getBgText()), 0, true);
    } else {
        //立即交换两个窗体信息
        if (videoSwap && videoSwap != videoSelect) {
            int channel1 = videoWidgets.indexOf(videoSelect);
            int channel2 = videoWidgets.indexOf(videoSwap);
            QMetaObject::invokeMethod(videoPanelTool, "changeVideo", Q_ARG(int, channel1), Q_ARG(int, channel2));
        }
    }

    videoPressed = false;
    videoMove = false;
    videoSwap = 0;

    //改变鼠标形状
    this->setCursor(Qt::ArrowCursor);
}

void frmVideoPanel::mouseMoveEvent(QMouseEvent *e)
{
    if (videoPressed) {
        videoMove = true;
        this->setCursor(QCursor(pixCursor));
        //先将窗体的坐标转换为视频窗体的坐标
        QPoint globalPos = e->globalPos();

        //逐个从所有的窗体比较判断当前是否是属于某个播放窗体
        videoExist = false;
        foreach (QWidget *widget, videoWidgets) {
            //如果控件不可见则继续,必须是可见的控件,比如8通道有1-8和9-16两种,通道1和通道9位置一样
            if (!widget->isVisible()) {
                continue;
            }

            QPoint widgetPos = this->mapToGlobal(widget->pos());
            QRect rect(widgetPos.x(), widgetPos.y(), widget->width(), widget->height());
            if (rect.contains(globalPos)) {
                videoSwap = (VideoWidget *)widget;
                videoExist = true;
                break;
            }
        }
    }
}

bool frmVideoPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        //选中通道后通知节点通道选中
        VideoWidget *videoWidget = (VideoWidget *) watched;
        AppEvent::Instance()->slot_selectVideo(videoWidgets.indexOf(videoWidget));
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        if (watched->inherits("QWidget")) {
            VideoWidget *videoWidget = (VideoWidget *) watched;
            QWidget *widget = videoWidget;
            if (videoWidget->parent()->inherits("frmVideoWidget")) {
                widget = (QWidget *)videoWidget->parent();
            }

            if (!AppConfig::VideoMax) {
                AppConfig::VideoMax = true;
                videoBox->hide_video_all();
                ui->gridLayout->addWidget(widget, 0, 0);
                widget->setVisible(true);
            } else {
                AppConfig::VideoMax = false;
                videoBox->show_video_all();
            }

            AppConfig::writeConfig();
            videoWidget->setFocus();
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        videoSelect = (VideoWidget *) watched;
        AppData::CurrentUrl = videoSelect->getVideoPara().videoUrl;
        //在视频分割窗体弹出右键菜单
        if (qApp->mouseButtons() == Qt::RightButton) {
            videoMenu->exec(QCursor::pos());
        }
    }

    return QWidget::eventFilter(watched, event);
}

void frmVideoPanel::initForm()
{
    timerRestart = new QTimer(this);
    connect(timerRestart, SIGNAL(timeout()), this, SLOT(restartVideo2()));

    //初始化底部快捷工具栏
    videoPanelTool = new frmVideoPanelTool;
    videoPanelTool->setFixedHeight(AppData::ToolHeight);
    ui->widget->layout()->addWidget(videoPanelTool);

    //启动定时器处理鼠标多久未操作自动全屏
    QTimer *timerAutoFull = new QTimer(this);
    connect(timerAutoFull, SIGNAL(timeout()), this, SLOT(autoFull()));
    timerAutoFull->start(1000);

    //关联样式改变信号自动重新设置图标
    connect(AppEvent::Instance(), SIGNAL(changeStyle()), this, SLOT(initIcon()));
    //绑定重新加载所有通道
    connect(AppEvent::Instance(), SIGNAL(restartVideo()), this, SLOT(restartVideo()));
    //应用程序关闭前先关闭所有视频
    connect(AppEvent::Instance(), SIGNAL(exitAll()), this, SLOT(close_video_all()));
}

void frmVideoPanel::initIcon()
{
    //从图形字体获取鼠标按下拖动形状
    pixCursor = IconHelper::getPixmap(QUIConfig::TextColor, 0xea3e, 20, 25, 25);
}

void frmVideoPanel::initVideo()
{
    videoExist = false;
    videoPressed = false;
    videoSelect = 0;
    videoSwap = 0;

    //视频控件参数配置更改立即执行
    connect(AppEvent::Instance(), SIGNAL(changeVideoConfig()), this, SLOT(changeVideoConfig()));
    //视频管理参数配置更改立即执行
    connect(AppEvent::Instance(), SIGNAL(changeVideoManage()), this, SLOT(initVideoManage()));

    for (int i = 0; i < AppConfig::VideoCount; ++i) {
        VideoWidget *videoWidget = new VideoWidget;
        QString objName = QString("ch%1").arg(i + 1, 2, 10, QChar('0'));
        videoWidget->setObjectName(objName);
        //设置背景文字
        videoWidget->setBgText(QString("通道 %1").arg(i + 1, 2, 10, QChar('0')));
        videoWidget->installEventFilter(this);

        //设置音量和静音状态
        videoWidget->setVolume(AppConfig::SoundValue);
        videoWidget->setMuted(AppConfig::SoundMuted);

        //绑定播放开始信号
        connect(videoWidget, SIGNAL(sig_receivePlayStart(int)), DeviceOnvif::Instance(), SLOT(receivePlayStart(int)));
        //绑定播放结束信号
        connect(videoWidget, SIGNAL(sig_receivePlayFinsh()), DeviceOnvif::Instance(), SLOT(receivePlayFinsh()));
        //绑定拖放信号
        connect(videoWidget, SIGNAL(sig_fileDrag(QString)), this, SLOT(fileDrag(QString)));
        //绑定悬浮条按钮单击信号
        connect(videoWidget, SIGNAL(sig_btnClicked(QString)), this, SLOT(btnClicked(QString)));

        AppData::VideoUrls << "";
        videoWidgets << videoWidget;
    }

    //初始化上一次打开的地址列表
    int count = DeviceHelper::initUrls() - 1;


    //设置当前控件为第一个控件
    videoSelect = videoWidgets.first();
    //初始化视频控件参数
    this->changeVideoConfig();
}

void frmVideoPanel::changeVideoConfig()
{
    for (int i = 0; i < AppConfig::VideoCount; ++i) {
        DeviceHelper::initVideoWidget(videoWidgets.at(i), AppConfig::SoundValue, AppConfig::SoundMuted, true, AppConfig::ReadTimeout);
    }
}

void frmVideoPanel::initVideoManage()
{   
    // print AppData::VideoUrls for debugging
    qDebug() << "Initializing VideoManage with VideoUrls:" << AppData::VideoUrls;

    VideoManage::Instance()->setVideoUrls(AppData::VideoUrls);
    VideoManage::Instance()->setVideoWidgets(videoWidgets);
    VideoManage::Instance()->setRecordTimes(DbQuery::getRecordTimes());
    VideoManage::Instance()->setSavePath(AppData::VideoNormalPath);
}

void frmVideoPanel::initOther()
{
    // 连接 AI TCP 客户端的检测结果信号
    connect(AiTcpClient::instance(), &AiTcpClient::detectionReceived,
            this, &frmVideoPanel::onDetectionReceived);

    // connect(AiTcpClient::instance(), &AiTcpClient::alarmReceived,
    //         this, &frmVideoPanel::onAlarmEventReceived);


    // 初始化检测覆盖层清理定时器，每秒检查一次
    timerClearDetection = new QTimer(this);
    connect(timerClearDetection, &QTimer::timeout, this, [this]() {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        const qint64 timeout = 1000; // 3秒超时

        for (auto it = lastDetectionTime.begin(); it != lastDetectionTime.end(); ++it) {
            if (now - it.value() > timeout) {
                it.key()->clearGraph();
                it.key()->clearOsd();
            }
        }
    });
    timerClearDetection->start(1000);
}

void frmVideoPanel::onAlarmEventReceived(const AlarmEvent &result)
{
    // 1. 通过 cameraId 在 IpcInfo 表中查找对应的主/子码流地址
    int ipcIndex = DbData::IpcInfo_IpcID.indexOf(result.cameraId.toInt());
    if (ipcIndex < 0) return;
    const QString rtspMain = DbData::IpcInfo_RtspMain.at(ipcIndex);
    const QString rtspSub  = DbData::IpcInfo_RtspSub.at(ipcIndex);

    // 2. 遍历所有 VideoWidget，找到 videoUrl 与主/子码流匹配的控件
    VideoWidget *target = nullptr;
    for (VideoWidget *w : videoWidgets) {
        const QString url = w->getVideoPara().videoUrl;
        if (url == rtspMain || url == rtspSub) {
            target = w;
            break;
        }
    }
    if (!target) return;

    // 更新最后接收时间
    lastDetectionTime[target] = QDateTime::currentMSecsSinceEpoch();

    // 清空上一帧的检测结果
    target->clearGraph();
    target->clearOsd();

    QRect imageRect = target->getImageRect();
    if (imageRect.isEmpty()) return;

    // 绘制报警事件的覆盖层
    QString key = QString("alarm_%1").arg(result.id);

    // 绘制报警框（bbox 为归一化坐标，需映射到实际显示区域）
    BoundingBox bbox;
    bbox.x      = result.bbox_x;
    bbox.y      = result.bbox_y;
    bbox.width  = result.bbox_w;
    bbox.height = result.bbox_h;

    GraphInfo box;
    box.name        = key;
    box.borderWidth = 2;
    box.borderColor = QColor("#ff0000");
    box.bgColor     = Qt::transparent;
    box.rect        = bbox.toRect(imageRect);
    target->appendGraph(box);

    // 绘制标签：显示在框的左上角
    OsdInfo label;
    label.name     = key;
    label.visible  = true;
    label.fontSize = 14;
    label.text     = QString("%1 [%2]").arg(result.alarmType).arg(result.trackId);
    label.color    = QColor("#ff0000");
    label.format   = OsdFormat_Text;
    label.position = OsdPosition_Custom;
    label.point    = box.rect.topLeft() + QPoint(-1, -23);
    label.invertedColors = true;
    target->appendOsd(label);
}


void frmVideoPanel::onDetectionReceived(const DetectionResult &result)
{
    // 1. 通过 cameraId 在 IpcInfo 表中查找对应的主/子码流地址
    int ipcIndex = DbData::IpcInfo_IpcID.indexOf(result.cameraId.toInt());
    if (ipcIndex < 0) return;
    const QString rtspMain = DbData::IpcInfo_RtspMain.at(ipcIndex);
    const QString rtspSub  = DbData::IpcInfo_RtspSub.at(ipcIndex);

    // 2. 遍历所有 VideoWidget，找到 videoUrl 与主/子码流匹配的控件
    VideoWidget *target = nullptr;
    for (VideoWidget *w : videoWidgets) {
        const QString url = w->getVideoPara().videoUrl;
        if (url == rtspMain || url == rtspSub) {
            target = w;
            break;
        }
    }
    if (!target) return;

    // 更新最后接收时间
    lastDetectionTime[target] = QDateTime::currentMSecsSinceEpoch();

    // 清空上一帧的检测结果
    target->clearGraph();
    target->clearOsd();

    if (result.detections.isEmpty()) return;

    QRect imageRect = target->getImageRect();
    if (imageRect.isEmpty()) return;

    for (const Detection &det : result.detections) {
        QString key = QString("det_%1").arg(det.trackId);

        // 绘制检测框
        GraphInfo box;
        box.name        = key;
        box.borderWidth = 2;
        box.borderColor = det.alarmType.isEmpty() ? QColor("#00FF00") : QColor("#FF0000");
        box.bgColor     = Qt::transparent;
        box.rect        = det.bbox.toRect(imageRect);
        target->appendGraph(box);

        // 绘制标签：显示在框的左上角
        OsdInfo label;
        label.name     = key;
        label.visible  = true;
        label.fontSize = 14;
        label.text     = QString("%1 [%2] %3%").arg(det.label).arg(det.trackId).arg(qRound(det.confidence * 100));
        label.color    = det.alarmType.isEmpty() ? QColor("#00FF00") : QColor("#FF0000");
        label.format   = OsdFormat_Text;
        label.position = OsdPosition_Custom;
        label.point    = box.rect.topLeft() + QPoint(-1, -23);
        label.invertedColors = true;  // 启用反色效果
        label.bold     = true;        // 启用粗体
        target->appendOsd(label);
    }
}

void frmVideoPanel::restartVideo()
{
    //初始化视频配置参数到视频控件
    this->initVideoManage();
    //关闭所有视频通道
    this->close_video_all();
    //定时器延时执行打开,用定时器的好处是防止重复执行可以立即取消前一次
    timerRestart->stop();
    timerRestart->start(1000);
}

void frmVideoPanel::restartVideo2()
{
    timerRestart->stop();
    this->changeVideoConfig();
    VideoManage::Instance()->start();
}

void frmVideoPanel::initMenu()
{
    videoMenu = new QMenu(this);

    actionFull = new QAction("切换全屏模式", videoMenu);
    connect(actionFull, SIGNAL(triggered(bool)), this, SLOT(full()));

    videoMenu->addAction(actionFull);
    videoMenu->addSeparator();

    videoMenu->addAction("删除当前视频", this, SLOT(delete_video_one()));
    videoMenu->addAction("删除所有视频", this, SLOT(delete_video_all()));
    videoMenu->addSeparator();

    videoMenu->addAction("截图当前视频", this, SLOT(snapshot_video_one()));
    videoMenu->addAction("截图所有视频", this, SLOT(snapshot_video_all()));
    videoMenu->addSeparator();


    
    videoMenu->addAction("显示通道配置", this, SLOT(show_video_config()));
    


    QList<bool> enable;
    enable << AppConfig::EnableChannel4 << AppConfig::EnableChannel6 << AppConfig::EnableChannel8;
    enable << AppConfig::EnableChannel9 << AppConfig::EnableChannel13 << AppConfig::EnableChannel16;
    enable << AppConfig::EnableChannel25 << AppConfig::EnableChannel36 << AppConfig::EnableChannel36;

    //实例化通道布局类
    videoBox = new VideoBox(this);
    connect(videoBox, SIGNAL(changeVideo(int, QString, bool)), this, SLOT(changeVideo(int, QString, bool)));
    videoBox->initMenu(videoMenu, enable);
    videoBox->setVideoType(AppConfig::VideoType);
    videoBox->setLayout(ui->gridLayout);
    this->setWidgets();

    //控制底部面板是否可见
    videoPanelTool->setVisible(true);
}

void frmVideoPanel::setWidgets()
{
    //重新转换类型
    QWidgetList widgets;
    foreach (VideoWidget *videoWidget, this->videoWidgets) {
        widgets << videoWidget;
    }

    videoBox->setWidgets(widgets);
    videoBox->show_video_all();
}

void frmVideoPanel::full()
{
    doFull(actionFull->text() == "切换全屏模式");
}

void frmVideoPanel::autoFull()
{
    //已经全屏或者设置的时间为0表示不启用
    if (AppConfig::FormFull || AppConfig::TimeAutoFull == 0 || !isVisible()) {
        return;
    }

    //计算鼠标未操作到现在的时间
    QDateTime now = QDateTime::currentDateTime();
    if (AppData::LastLiveTime.secsTo(now) > AppConfig::TimeAutoFull) {
        doFull(true);
    }
}

void frmVideoPanel::doFull(bool full)
{
    if (full) {
        actionFull->setText("切换正常模式");
        videoPanelTool->setVisible(false);
    } else {
        actionFull->setText("切换全屏模式");
        videoPanelTool->setVisible(true);
    }

    AppConfig::FormFull = full;
    AppConfig::writeConfig();
    AppEvent::Instance()->slot_fullScreen(full);
}

void frmVideoPanel::play(const QStringList &urls)
{
    VideoManage::Instance()->setVideoUrls(urls);
    VideoManage::Instance()->start();
}

void frmVideoPanel::play_video_all()
{
    play(AppData::VideoUrls);
}

void frmVideoPanel::close_video_all()
{
    //轮询状态不提示
    if (AppConfig::Polling) {
        VideoManage::Instance()->stop();
        return;
    }

    //弹出提示信息窗体等待加载完成
    QUIHelper::showSplashInfo("正在关闭所有通道, 请勿关闭程序...", 50, 15);
    qApp->processEvents();
    qApp->setOverrideCursor(Qt::WaitCursor);
    VideoManage::Instance()->stop();
    qApp->restoreOverrideCursor();
    QUIHelper::hideSplashInfo();
}

void frmVideoPanel::delete_video_one()
{
    //清空当前url以后云台控制失效(意味着必须打开的时候才能云台控制)
    AppData::CurrentUrl = "";
    if (videoSelect) {
        int index = videoWidgets.indexOf(videoSelect);
        QString url = AppData::VideoUrls.at(index);
        if (url.isEmpty()) {
            return;
        }

        //重新替换当前url并保存
        AppData::VideoUrls[index] = "";
        DeviceHelper::saveUrls();
        DeviceHelper::addMsg(QString("通道 %1 删除视频").arg(index + 1), 0, true);
        videoWidgets.at(index)->stop();
    }
}

void frmVideoPanel::delete_video_all()
{
    close_video_all();

    //替换所有url为空并保存
    for (int i = 0; i < AppConfig::VideoCount; ++i) {
        AppData::VideoUrls[i] = "";
    }

    DeviceHelper::saveUrls();
    DeviceHelper::addMsg(QString("所有通道 删除视频"), 0, true);
    //清空当前url以后云台控制失效,意味着必须打开的时候才能云台控制
    AppData::CurrentUrl = "";
}

void frmVideoPanel::snapshot_video_one()
{
    if (videoSelect) {
        QString objName = videoSelect->objectName();
        QString fileName = QString("%1/%2_%3.jpg").arg(VideoWidget::snapPath).arg(objName).arg(STRDATETIMEMS);
        videoSelect->snap(fileName);
        QUIHelper::showTipBox("提示", QString("截图已保存到文件\n%1").arg(fileName), AppConfig::FullScreen, false, 5);
    }
}

void frmVideoPanel::snapshot_video_all()
{
    for (int index = 0; index < AppConfig::VideoCount; index++) {
        QString objName = videoWidgets.at(index)->objectName();
        QString fileName = QString("%1/%2_%3.jpg").arg(VideoWidget::snapPath).arg(objName).arg(STRDATETIMEMS);
        videoWidgets.at(index)->snap(fileName);
    }

    QUIHelper::showTipBox("提示", QString("截图已保存到文件夹\n%1").arg(VideoWidget::snapPath), AppConfig::FullScreen, false, 5);
}

void frmVideoPanel::get_video_gps()
{
    if (videoSelect) {
        QString rtsp = videoSelect->getVideoPara().videoUrl;
        QString ipcPosition;
        DbQuery::getIpcInfo(rtsp, ipcPosition);
        if (!ipcPosition.isEmpty()) {
            //打印经纬度值
            DeviceHelper::addMsg(ipcPosition);
            //弹框提示
            if (QUIHelper::showMessageBoxQuestion("是否发送经纬度值?") == QMessageBox::Yes) {
                //执行对应动作
            }
        }
    }
}

void frmVideoPanel::show_video_config()
{
    //与现有的通道拖曳交换功能雷同,后期再考虑是否其他的需要做在这里
    //其实就是弹出个配置界面,具体要操作什么丢里面就行
}

void frmVideoPanel::btnClicked(const QString &btnName)
{
    //识别是哪个通道上的哪个按钮
    VideoWidget *videoWidget = (VideoWidget *)sender();
    DeviceHelper::addMsg(QString("按下按钮 %1").arg(btnName));
    if (btnName.endsWith("btnClose")) {
        //关闭以后立即更新url.txt
        int index = videoWidgets.indexOf(videoWidget);
        if (index >= 0) {
            videoWidgets.at(index)->setFocus();
            AppData::VideoUrls[index] = "";
            DeviceHelper::saveUrls();
            DeviceHelper::addMsg(QString("通道 %1 关闭视频").arg(index + 1), 0, true);
        }
    }
}

void frmVideoPanel::fileDrag(const QString &url)
{
    //接收到的文件立即更新url.txt
    VideoWidget *videoWidget = (VideoWidget *)sender();
    int index = videoWidgets.indexOf(videoWidget);
    if (index >= 0) {        
        videoWidget->open(url);
        videoWidget->setFocus();
        AppData::VideoUrls[index] = url;
        DeviceHelper::saveUrls();
        DeviceHelper::addMsg(QString("通道 %1 拖曳视频").arg(index + 1), 0, true);
        QTimer::singleShot(10, AppEvent::Instance(), SIGNAL(fileDragOpen()));
    }
}

void frmVideoPanel::changeVideo(int type, const QString &videoType, bool videoMax)
{
    AppConfig::VideoType = videoType;
    AppConfig::VideoMax = videoMax;
    AppConfig::writeConfig();
    //底部工具栏通道切换按钮选中
    QMetaObject::invokeMethod(videoPanelTool, "changeVideo", Q_ARG(int, type));
}
