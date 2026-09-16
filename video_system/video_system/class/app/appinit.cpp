#include "appinit.h"
#include "appstyle.h"
#include "quihelper.h"
#include "dbquery.h"
#include "devicemap.h"
#include "dbconnthread.h"
#include "frmconfigdb.h"
#include "ApiClient.h"
#include "ai/AiTcpClient.h"

SINGLETON_IMPL(AppInit)
AppInit::AppInit(QObject *parent) : QObject(parent)
{
}

bool AppInit::eventFilter(QObject *watched, QEvent *event)
{
    //负责将按键字符串存入队列
    if (event->type() == QEvent::KeyPress) {
        //记住程序最后的活动时间-包括键盘+鼠标活动
        AppData::LastLiveTime = QDateTime::currentDateTime();
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int keyValue = keyEvent->key();
        if (keyValue == Qt::Key_Escape) {
            emit keyPressed("esc");
            return true;
        } else if (keyEvent->modifiers() & Qt::AltModifier) {
            if (keyValue == Qt::Key_Enter || keyValue == Qt::Key_Return) {
                emit keyPressed("alt+enter");
                return true;
            }
        }
    }

    if (event->type() == QEvent::MouseMove) {
        //记住程序最后的活动时间-包括键盘+鼠标活动
        AppData::LastLiveTime = QDateTime::currentDateTime();
        if (hideCursor) {
            hideCursor = false;
            qApp->restoreOverrideCursor();
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        //识别鼠标松开
        AppEvent::Instance()->slot_mouseButtonRelease();
    } else if (event->type() == QEvent::UngrabMouse) {
        //qchart控件居然鼠标按下是UngrabMouse事件而不是MouseButtonRelease
        AppEvent::Instance()->slot_mouseButtonRelease();
    }

    //以下代码处理无边框窗体可拖动
    QWidget *w = (QWidget *)watched;
    if (!w->property("canMove").toBool()) {
        return QObject::eventFilter(watched, event);
    }

    static QPoint mousePoint;
    static bool mousePressed = false;

    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (event->type() == QEvent::MouseButtonPress) {
        if (mouseEvent->button() == Qt::LeftButton) {
            mousePressed = true;
            mousePoint = mouseEvent->globalPos() - w->pos();
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        mousePressed = false;
    } else if (event->type() == QEvent::MouseMove) {
        if (mousePressed) {
            w->move(mouseEvent->globalPos() - mousePoint);
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}

void AppInit::checkCursor()
{
    //全屏模式下超过规定时间鼠标未动则隐藏光标
    if (AppConfig::FormFull && !hideCursor && AppConfig::TimeHideCursor > 0) {
        QDateTime now = QDateTime::currentDateTime();
        if (AppData::LastLiveTime.secsTo(now) > AppConfig::TimeHideCursor) {
            hideCursor = true;
            qApp->setOverrideCursor(Qt::BlankCursor);
        }
    }
}

void AppInit::start()
{
    //安装全局事件过滤器
    qApp->installEventFilter(this);

    //一次性设置所有包括编码字体翻译等
    QUIHelper::initAll();

    //新建目录
    QUIHelper::checkPath("db");
    QUIHelper::checkPath("log");
    QUIHelper::checkPath("logo");
    QUIHelper::checkPath("snap");
    QUIHelper::checkPath("config");

    //载入配置文件
    AppConfig::ConfigFile = QString("%1/config/%2.ini").arg(QUIHelper::appPath()).arg("video_system");
    AppConfig::readConfig();
    AppStyle::initStyle();

    //设置左侧+最大化+最小化+关闭 图标
    QUIConfig::IconMain = 0xea2a;
    QUIConfig::IconMax = 0xf2d2;
    QUIConfig::IconNormal = 0xf2d0;
    if (AppConfig::LogoImage.startsWith("icon_")) {
        QString icon = AppConfig::LogoImage.split("_").last();
        QUIConfig::IconMain = icon.toInt(NULL, 16);
    }

    //初始化杂七杂八
    this->initOther();
    //初始化数据库
    this->initDb();
    //初始化不同工作模式的全局设置
    this->initWorkMode();
    //校验自动登录
    this->autoLogin();
}

void AppInit::initOther()
{
    //计算地图宽高
#if 1
    AppData::MapWidth = QUIHelper::deskWidth() - AppData::LeftWidth - AppData::RightWidth - 25;
    AppData::MapHeight = QUIHelper::deskHeight() - AppData::TopHeight - AppData::BottomHeight - 15;
#else
    AppData::MapWidth = AppConfig::FormGeometry.width() - AppData::LeftWidth - AppData::RightWidth - 25;
    AppData::MapHeight = AppConfig::FormGeometry.height() - AppData::TopHeight - AppData::BottomHeight - 15;
#endif

    AppData::NvrTypes << "海康" << "大华" << "宇视" << "深广" << "其他";
    AppData::IpcTypes << "海康" << "大华" << "宇视" << "深广" << "其他";

    //目录不存在则新建
    QUIHelper::checkPath(AppData::VideoNormalPath);
    QUIHelper::checkPath(AppData::ImageNormalPath);

    //重新设置完整的目录路径
    AppData::VideoNormalPath = QUIHelper::appPath() + "/" + AppData::VideoNormalPath;
    AppData::VideoAlarmPath = QUIHelper::appPath() + "/" + AppData::VideoAlarmPath;
    AppData::ImageNormalPath = QUIHelper::appPath() + "/" + AppData::ImageNormalPath;
    AppData::ImageAlarmPath = QUIHelper::appPath() + "/" + AppData::ImageAlarmPath;

    //载入地图文件名称集合
    AppData::MapPath = QUIHelper::appPath() + "/map";
    QStringList filter;
    filter << "*.jpg" << "*.bmp" << "*.png";
    QDir mapPath(AppData::MapPath);
    AppData::MapNames << mapPath.entryList(filter);
    DeviceMap::Instance()->loadMap();

    //载入声音文件名称集合
    AppData::SoundPath = QUIHelper::appPath() + "/sound";
    filter.clear();
    filter << "*.wav" << "*.mp3" << "*.mdi";
    QDir soundPath(AppData::SoundPath);
    AppData::SoundNames << soundPath.entryList(filter);

    //设置开机启动
    QString strPath = QApplication::applicationFilePath();
    strPath = QDir::toNativeSeparators(strPath);
    QUIHelper::runWithSystem(QUIHelper::appName(), strPath, AppConfig::AutoRun);

    //启动定时器计算多久用户没有操作过鼠标
    hideCursor = false;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkCursor()));
    timer->start(1000);
}

void AppInit::initDb()
{
    //初始化数据库连接信息结构体数据
    DbInfo dbInfo;
    frmConfigDb::initDbInfo(dbInfo, "qt_sql_default_connection");

    bool ok = true;
    QString dbType = AppConfig::LocalDbType.toUpper();
    if (dbType == "SQLITE") {
        dbInfo.dbName = DbHelper::getDbDefaultFile("video_system");
        if (QFile(dbInfo.dbName).size() <= 4) {
            ok = false;
        }
    }

    //如果打开失败则弹出数据库配置界面进行设置
    DbData::DbLocal = new DbConnThread;
    DbData::DbLocal->setConnInfo(DbHelper::getDbType(dbType), dbInfo);
    if (!ok || !DbData::DbLocal->openDb()) {
        QUIWidget qui;
        QUIWidgetData widgetData;
        widgetData.title = "数据库配置";
        widgetData.visibleMin = false;
        widgetData.visibleMax = false;
        qui.setWidgetData(widgetData);

        frmConfigDb *configDb = new frmConfigDb;
        configDb->setConnFlag("video_system");
        qui.setMainWidget(configDb);
        QUIHelper::setFormInCenter(&qui);
        qui.exec();
        exit(0);
    } else {
        DbData::DbLocal->start();
    }

    DbQuery::loadNvrInfo();
    ApiClient::init(AppConfig::WebServerUrl);

    // Start AI TCP client — connects to ai_service on port 9002, auto-reconnects on failure
    {
        QUrl url(AppConfig::AIServerUrl);
        AiTcpClient::init(url.host(), url.port());
    }
    DbQuery::loadPollInfo();
    DbQuery::loadRecordInfo();

    //查询最大记录数
    DbData::UserLogID = DbHelper::getMaxID("LogInfo", "LogID");
    //DbQuery::addUserLog(100, "测试数据");
}

void AppInit::initWorkMode()
{

    AppData::GpsDeviceNames << "测试设备";
    AppData::GpsDevicePoints << AppConfig::MapCenter;
    AppData::GpsDeviceColors << "#22A3A9";

    AppData::GpsDeviceCount = AppData::GpsDeviceNames.size();

    //设置权限对应的文字描述
    UserHelper::PermissionName.clear();
    UserHelper::PermissionName << "系统设置" << "删除记录" << "调整布局" << "视频回放" << "电子地图" << "日志查询" << "用户管理";
}

void AppInit::autoLogin()
{
    UserHelper::loadUserInfo();
    if (AppConfig::AutoLogin) {
        UserHelper::CurrentUserName = AppConfig::LastLoginer;
        UserHelper::getUserInfo();
        DbQuery::addUserLog("用户自动登录");
    }
}
