#include "quihelpercore.h"
#include "quihelper.h"

int QUIHelperCore::getScreenIndex()
{
    //需要对多个屏幕进行处理
    int screenIndex = 0;
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    int screenCount = qApp->screens().size();
#else
    int screenCount = qApp->desktop()->screenCount();
#endif

    if (screenCount > 1) {
        //找到当前鼠标所在屏幕
        QPoint pos = QCursor::pos();
        for (int i = 0; i < screenCount; ++i) {
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
            if (qApp->screens().at(i)->geometry().contains(pos)) {
#else
            if (qApp->desktop()->screenGeometry(i).contains(pos)) {
#endif
                screenIndex = i;
                break;
            }
        }
    }
    return screenIndex;
}

QRect QUIHelperCore::getScreenRect(bool available)
{
    QRect rect;
    int screenIndex = QUIHelperCore::getScreenIndex();
    if (available) {
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
        rect = qApp->screens().at(screenIndex)->availableGeometry();
#else
        rect = qApp->desktop()->availableGeometry(screenIndex);
#endif
    } else {
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
        rect = qApp->screens().at(screenIndex)->geometry();
#else
        rect = qApp->desktop()->screenGeometry(screenIndex);
#endif
    }
    return rect;
}

qreal QUIHelperCore::getScreenRatio(bool devicePixel)
{
    qreal ratio = 1.0;
    int screenIndex = getScreenIndex();
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    QScreen *screen = qApp->screens().at(screenIndex);
    if (devicePixel) {
        //需要开启 AA_EnableHighDpiScaling 属性才能正常获取
        ratio = screen->devicePixelRatio() * 100;
    } else {
        ratio = screen->logicalDotsPerInch();
    }
#else
    //Qt4不能动态识别缩放更改后的值
    ratio = qApp->desktop()->screen(screenIndex)->logicalDpiX();
#endif
    return ratio / 96;
}

QRect QUIHelperCore::checkCenterRect(QRect &rect, bool available)
{
    QRect deskRect = QUIHelper::getScreenRect(available);
    int formWidth = rect.width();
    int formHeight = rect.height();
    int deskWidth = deskRect.width();
    int deskHeight = deskRect.height();
    int formX = deskWidth / 2 - formWidth / 2 + deskRect.x();
    int formY = deskHeight / 2 - formHeight / 2;
    rect = QRect(formX, formY, formWidth, formHeight);
    return deskRect;
}

int QUIHelperCore::deskWidth()
{
    return getScreenRect().width();
}

int QUIHelperCore::deskHeight()
{
    return getScreenRect().height();
}

QSize QUIHelperCore::deskSize()
{
    return getScreenRect().size();
}

void QUIHelperCore::setFormInCenter(QWidget *form, QWidget *centerBaseForm)
{
    int formWidth = form->width();
    int formHeight = form->height();

    //如果=0表示采用系统桌面屏幕为参照
    QRect rect;
    if (centerBaseForm == 0) {
        rect = getScreenRect();
    } else {
        rect = centerBaseForm->geometry();
    }

    int deskWidth = rect.width();
    int deskHeight = rect.height();
    QPoint movePoint(deskWidth / 2 - formWidth / 2 + rect.x(), deskHeight / 2 - formHeight / 2 + rect.y());
    form->move(movePoint);
}

void QUIHelperCore::showForm(QWidget *form)
{
    setFormInCenter(form);

    //判断宽高是否超过了屏幕分辨率,超过了则最大化显示
    //qDebug() << TIMEMS << form->size() << deskSize();
    if (form->width() + 20 > deskWidth() || form->height() + 50 > deskHeight()) {
        form->showMaximized();
    } else {
        form->show();
    }
}

QString QUIHelperCore::appName()
{
    //没有必要每次都获取,只有当变量为空时才去获取一次
    static QString name;
    if (name.isEmpty()) {
        name = qApp->applicationFilePath();
        //下面的方法主要为了过滤安卓的路径 lib程序名_armeabi-v7a/lib程序名_arm64-v8a
        QStringList list = name.split("/");
        name = list.at(list.size() - 1).split(".").at(0);
        name.replace("_armeabi-v7a", "");
        name.replace("_arm64-v8a", "");
    }

    return name;
}

QString QUIHelperCore::appPath()
{
    static QString path;
    if (path.isEmpty()) {
#ifdef Q_OS_ANDROID
        //默认安卓根目录
        path = "/storage/emulated/0";
        //带上程序名称作为目录 前面加个0方便排序
        path = path + "/0" + appName();
#else
        path = qApp->applicationDirPath();
#endif
    }

    return path;
}

QString QUIHelperCore::getCompilerString()
{
    QString compilerString = "<unknown>";
#if defined(Q_CC_CLANG)
    QString isAppleString;
#if defined(__apple_build_version__)
    isAppleString = QLatin1String(" (Apple)");
#endif
    compilerString = QLatin1String("Clang ") + QString::number(__clang_major__) + QLatin1Char('.') + QString::number(__clang_minor__) + isAppleString;
#elif defined(Q_CC_GNU)
    compilerString = QLatin1String("GCC ") + QLatin1String(__VERSION__);
#elif defined(Q_CC_MSVC)
    if (_MSC_VER > 1999) {
        compilerString = QLatin1String("MSVC <unknown>");
    } else if (_MSC_VER >= 1920) {
        compilerString = QLatin1String("MSVC 2019");
    } else if (_MSC_VER >= 1910) {
        compilerString = QLatin1String("MSVC 2017");
    } else if (_MSC_VER >= 1900) {
        compilerString = QLatin1String("MSVC 2015");
    } else if (_MSC_VER >= 1800) {
        compilerString = QLatin1String("MSVC 2013");
    } else if (_MSC_VER >= 1700) {
        compilerString = QLatin1String("MSVC 2012");
    } else if (_MSC_VER >= 1600) {
        compilerString = QLatin1String("MSVC 2010");
    } else {
        compilerString = QLatin1String("MSVC <old>");
    }
#endif
    return compilerString;
}

QString QUIHelperCore::getUuid()
{
    QString uuid = QUuid::createUuid().toString();
    uuid.replace("{", "");
    uuid.replace("}", "");
    return uuid;
}

void QUIHelperCore::checkPath(const QString &dirName)
{
    //相对路径需要补全完整路径
    QString path = dirName;
    if (path.startsWith("./")) {
        path.replace(".", "");
        path = QUIHelper::appPath() + path;
    } else if (!path.startsWith("/") && !path.contains(":/")) {
        path = QUIHelper::appPath() + "/" + path;
    }

    //目录不存在则新建
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(path);
    }
}

void QUIHelperCore::sleep(int msec)
{
    if (msec <= 0) {
        return;
    }

#if 0
    //非阻塞方式延时,现在很多人推荐的方法
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
#else
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    //阻塞方式延时,如果在主线程会卡住主界面
    QThread::msleep(msec);
#else
    //非阻塞方式延时,不会卡住主界面,据说可能有问题
    QTime endTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < endTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
#endif
#endif
}

void QUIHelperCore::checkRun()
{
#ifdef Q_OS_WIN
    //延时1秒钟,等待程序释放完毕
    QUIHelper::sleep(1000);
    //创建共享内存,判断是否已经运行程序
    static QSharedMemory mem(QUIHelper::appName());
    if (!mem.create(1)) {
        QUIHelper::showMessageBoxError("程序已运行, 软件将自动关闭!", 5, true);
        exit(0);
    }
#endif
}

void QUIHelperCore::setCode(bool utf8)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    //如果想要控制台打印信息中文正常就注释掉这个设置
    if (utf8) {
        QTextCodec *codec = QTextCodec::codecForName("utf-8");
        QTextCodec::setCodecForLocale(codec);
    }
#else
#if _MSC_VER
    QTextCodec *codec = QTextCodec::codecForName("gbk");
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
#endif
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
#endif
}

QFont QUIHelperCore::addFont(const QString &fontFile, const QString &fontName)
{
    //判断图形字体是否存在,不存在则加入
    QFontDatabase fontDb;
    if (!fontDb.families().contains(fontName)) {
        int fontId = fontDb.addApplicationFont(fontFile);
        QStringList listName = fontDb.applicationFontFamilies(fontId);
        if (listName.size() == 0) {
            qDebug() << QString("load %1 error").arg(fontName);
        }
    }

    //再次判断是否包含字体名称防止加载失败
    QFont font;
    if (fontDb.families().contains(fontName)) {
        font = QFont(fontName);
#if (QT_VERSION >= QT_VERSION_CHECK(4,8,0))
        font.setHintingPreference(QFont::PreferNoHinting);
#endif
    }

    return font;
}

void QUIHelperCore::setFont(const QString &fontFile, const QString &fontName, int fontSize)
{
    QFont font;
    font.setFamily(fontName);
    font.setPixelSize(fontSize);

    //如果存在字体文件则选择字体文件中的字体
    //安卓版本和网页版本需要字体文件一起打包单独设置字体
    if (!fontFile.isEmpty()) {
        QFontDatabase fontDb;
        int fontId = fontDb.addApplicationFont(fontFile);
        if (fontId != -1) {
            QStringList fontNames = fontDb.applicationFontFamilies(fontId);
            if (fontNames.size() > 0) {
                font.setFamily(fontNames.at(0));
                font.setPixelSize(fontSize);
            }
        }
    }

    qApp->setFont(font);
}

void QUIHelperCore::setTranslator()
{
    //以后还有其他的自行加上去就行
    QUIHelperCore::setTranslator(":/qm/widgets.qm");
    QUIHelperCore::setTranslator(":/qm/qt_zh_CN.qm");
    QUIHelperCore::setTranslator(":/qm/designer_zh_CN.qm");
}

void QUIHelperCore::setTranslator(const QString &qmFile)
{
    //过滤下不存在的就不用设置了
    if (!QFile(qmFile).exists()) {
        return;
    }

    QTranslator *translator = new QTranslator(qApp);
    if (translator->load(qmFile)) {
        qApp->installTranslator(translator);
    }
}

#ifdef Q_OS_ANDROID
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QtAndroidExtras>
#else
//Qt6中将相关类移到了core模块而且名字变了
#include <QtCore/private/qandroidextras_p.h>
#endif
#endif

bool QUIHelperCore::checkPermission(const QString &permission)
{
#ifdef Q_OS_ANDROID
#if (QT_VERSION >= QT_VERSION_CHECK(5,10,0) && QT_VERSION < QT_VERSION_CHECK(6,0,0))
    QtAndroid::PermissionResult result = QtAndroid::checkPermission(permission);
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::requestPermissionsSync(QStringList() << permission);
        result = QtAndroid::checkPermission(permission);
        if (result == QtAndroid::PermissionResult::Denied) {
            return false;
        }
    }
#else
    QFuture<QtAndroidPrivate::PermissionResult> result = QtAndroidPrivate::requestPermission(permission);
    if (result.resultAt(0) == QtAndroidPrivate::PermissionResult::Denied) {
        return false;
    }
#endif
#endif
    return true;
}

void QUIHelperCore::initAndroidPermission()
{
    //可以把所有要动态申请的权限都写在这里
    checkPermission("android.permission.CALL_PHONE");
    checkPermission("android.permission.SEND_SMS");
    checkPermission("android.permission.READ_EXTERNAL_STORAGE");
    checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");

    checkPermission("android.permission.ACCESS_COARSE_LOCATION");
    checkPermission("android.permission.BLUETOOTH");
    checkPermission("android.permission.BLUETOOTH_SCAN");
    checkPermission("android.permission.BLUETOOTH_CONNECT");
    checkPermission("android.permission.BLUETOOTH_ADVERTISE");
}

void QUIHelperCore::initAll(bool utf8)
{    
    //初始化随机数种子
    QUIHelper::initRand();
    //初始化安卓权限
    QUIHelperCore::initAndroidPermission();
    //设置编码
    QUIHelperCore::setCode(utf8);
    //设置字体
    QUIHelperCore::setFont();
    //设置翻译文件支持多个
    QUIHelperCore::setTranslator();
    //设置不使用本地系统环境代理配置
    QNetworkProxyFactory::setUseSystemConfiguration(false);
}

void QUIHelperCore::initMain(bool desktopSettingsAware, bool useOpenGLES)
{
    //设置是否应用操作系统设置比如字体
    QApplication::setDesktopSettingsAware(desktopSettingsAware);

#ifdef Q_OS_ANDROID
#if (QT_VERSION >= QT_VERSION_CHECK(5,6,0))
    //开启高分屏缩放支持
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#else
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    //不应用任何缩放
    QApplication::setAttribute(Qt::AA_Use96Dpi);
#endif
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    //高分屏缩放策略
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5,4,0))
    //设置opengl模式 AA_UseDesktopOpenGL(默认) AA_UseOpenGLES AA_UseSoftwareOpenGL
    //在一些很旧的设备上或者对opengl支持很低的设备上需要使用AA_UseOpenGLES表示禁用硬件加速
    //如果开启的是AA_UseOpenGLES则无法使用硬件加速比如ffmpeg的dxva2
    if (useOpenGLES) {
        QApplication::setAttribute(Qt::AA_UseOpenGLES);
    }

    //设置opengl共享上下文
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#endif
}
