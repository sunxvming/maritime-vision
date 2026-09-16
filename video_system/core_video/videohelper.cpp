#include "videohelper.h"
#include "qtcpsocket.h"
#include "urlhelper.h"

#ifdef qmedia
#include "qmediahelper.h"
#endif

#ifdef ffmpeg
#include "ffmpeghelper.h"
#endif

#ifdef vlcx
#include "vlchelper.h"
#endif

#ifdef mpvx
#include "mpvhelper.h"
#endif

#ifdef qtavx
#include "qtavhelper.h"
#endif

#ifdef haikang
#include "haikanghelper.h"
#endif

#ifdef easyplayer
#include "easyplayerhelper.h"
#endif

QString VideoHelper::getVersion()
{
#ifdef qmedia
    return getVersion(VideoCore_QMedia);
#elif ffmpeg
    return getVersion(VideoCore_FFmpeg);
#elif vlcx
    return getVersion(VideoCore_Vlc);
#elif mpvx
    return getVersion(VideoCore_Mpv);
#elif qtavx
    return getVersion(VideoCore_Qtav);
#elif haikang
    return getVersion(VideoCore_HaiKang);
#elif easyplayer
    return getVersion(VideoCore_EasyPlayer);
#else
    return getVersion(VideoCore_Other);
#endif
}

QString VideoHelper::getVersion(const VideoCore &videoCore)
{
    QString version = "none v1.0";
    if (videoCore == VideoCore_QMedia) {
#ifdef qmedia
        version = "Qt v" + QMediaHelper::getVersion();
#endif
    } else if (videoCore == VideoCore_FFmpeg) {
#ifdef ffmpeg
        version = "ffmpeg v" + FFmpegHelper::getVersion();
#endif
    } else if (videoCore == VideoCore_Vlc) {
#ifdef vlcx
        version = "vlc v" + VlcHelper::getVersion();
#endif
    } else if (videoCore == VideoCore_Mpv) {
#ifdef mpvx
        version = "mpv v" + MpvHelper::getVersion();
#endif
    } else if (videoCore == VideoCore_Qtav) {
#ifdef qtavx
        version = "qtav v" + QtavHelper::getVersion();
#endif
    } else if (videoCore == VideoCore_HaiKang) {
#ifdef haikang
        version = "haikang v" + HaiKangHelper::getVersion();
#endif
    } else if (videoCore == VideoCore_EasyPlayer) {
#ifdef easyplayer
        version = "easyplayer v" + EasyPlayerHelper::getVersion();
#endif
    }

    return version;
}

void VideoHelper::initVideoCore(VideoPara &videoPara, int type)
{
    videoPara.videoCore = VideoCore_None;
    if (type == 0) {
#ifdef qmedia
        videoPara.videoCore = VideoCore_QMedia;
#elif ffmpeg
        videoPara.videoCore = VideoCore_FFmpeg;
#elif vlcx
        videoPara.videoCore = VideoCore_Vlc;
#elif mpvx
        videoPara.videoCore = VideoCore_Mpv;
#elif qtavx
        videoPara.videoCore = VideoCore_Qtav;
#elif haikang
        videoPara.videoCore = VideoCore_HaiKang;
#elif easyplayer
        videoPara.videoCore = VideoCore_EasyPlayer;
#endif
    } else if (type == 1) {
#ifdef qmedia
        videoPara.videoCore = VideoCore_QMedia;
#endif
    } else if (type == 2) {
#ifdef ffmpeg
        videoPara.videoCore = VideoCore_FFmpeg;
#endif
    } else if (type == 3) {
#ifdef vlcx
        videoPara.videoCore = VideoCore_Vlc;
#endif
    } else if (type == 4) {
#ifdef mpvx
        videoPara.videoCore = VideoCore_Mpv;
#endif
    } else if (type == 5) {
#ifdef qtavx
        videoPara.videoCore = VideoCore_Qtav;
#endif
    } else if (type == 10) {
#ifdef haikang
        videoPara.videoCore = VideoCore_HaiKang;
#endif
    } else if (type == 20) {
#ifdef easyplayer
        videoPara.videoCore = VideoCore_EasyPlayer;
#endif
    }
}

int VideoHelper::getRangeValue(int oldMin, int oldMax, int oldValue, int newMin, int newMax)
{
    return (((oldValue - oldMin) * (newMax - newMin)) / (oldMax - oldMin)) + newMin;
}

bool VideoHelper::checkUrl(const QString &videoUrl, int timeout)
{
    //没有超时时间则认为永远返回真
    if (timeout <= 0) {
        return true;
    }

    //找出IP地址和端口号
    QString ip = UrlHelper::getUrlIP(videoUrl);
    int port = UrlHelper::getUrlPort(videoUrl);

    //局部的事件循环不卡主界面
    QEventLoop eventLoop;

    //设置超时
    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
    timer.setSingleShot(true);
    timer.start(timeout);

    QTcpSocket tcpSocket;
    QObject::connect(&tcpSocket, SIGNAL(connected()), &eventLoop, SLOT(quit()));
    tcpSocket.connectToHost(ip, port);
    eventLoop.exec();

    //超时没有连接上则判断该设备不在线
    if (tcpSocket.state() != QAbstractSocket::ConnectedState) {
        //qDebug() << TIMEMS << QString("连接失败 -> 地址: %1").arg(videoUrl);;
        return false;
    }

    return true;
}

bool VideoHelper::checkUrl(VideoThread *videoThread, const VideoType &videoType, const QString &videoUrl, int timeout)
{
    QString error;
    if (videoUrl.isEmpty()) {
        error = "地址为空";
    } else if (videoType == VideoType_Rtsp) {
        if (!checkUrl(videoUrl, timeout)) {
            error = "网络地址不可达";
        }
    } else if (videoType == VideoType_FileLocal) {
        if (!QFile(videoUrl).exists()) {
            error = "文件不存在";
        }
    } else if (videoType == VideoType_Other) {

    }

    if (error.isEmpty()) {
        return true;
    } else {
        videoThread->debug("地址有误", "原因: " + error);
        return false;
    }
}

QString VideoHelper::getRightUrl(const VideoType &videoType, const QString &videoUrl)
{
    //视频流中的特殊字符比如rtsp中的用户密码部分需要转义
    QString url = videoUrl;
    int index = videoUrl.lastIndexOf("@");
    if (videoType == VideoType_Rtsp && index > 0) {
        QString userName, userPwd;
        UrlHelper::getUserInfo(url, userName, userPwd);
        QString otherInfo = videoUrl.mid(index + 1, videoUrl.length() - index);
        userName = userName.toUtf8().toPercentEncoding();
        userPwd = userPwd.toUtf8().toPercentEncoding();

        //不能用字符串拼接(又会把转义字符转回去)
        //url = QString("rtsp://%1:%2@%3").arg(userName).arg(userPwd).arg(otherInfo);
        url.clear();
        url.append("rtsp://");
        url.append(userName);
        url.append(":");
        url.append(userPwd);
        url.append("@");
        url.append(otherInfo);
    }

    return url;
}

int VideoHelper::getYuvSize(int width, int height)
{
    //一般 1080P=3110400 720P=1382400
    //int size = width * height;
    //size = size + size / 2;
    //下面对应y+u+v大小
    int size = (width * height) + (width / 2 * height / 2) + (width / 2 * height / 2);
    return size;
}

void VideoHelper::renameFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return;
    }

    QFileInfo file(fileName);
    QString path = file.path();
    QString name = file.fileName();

    //找到目录下的所有视频文件
    QStringList filter;
    filter << "*.mp3" << "*.mp4" << "*.avi" << "*.asf" << "*.mkv" ;
    QDir dir(path);
    QStringList files = dir.entryList(filter);
    if (files.size() == 0) {
        return;
    }

    //录制的视频文件是按照内部规则命名的(要在生成文件后对文件重命名)
    //为了产生规范的命名需要在 libvlc_new 的时候设置 meta-title
    //建议每个视频通道必须设置唯一标识不然同一个时刻保存的文件只能重命名一个
    //正确名称: video1_2021-09-26-16-15-41.mp4 ch1_2021-09-26-16-15-41.mp4
    //默认名称: vlc-record-2022-11-04-14h07m43s-2.mp4-.mp4
    //默认名称: vlc-record-2021-09-26-16h15m13s-dshow___-.avi
    //默认名称: vlc-record-2021-09-26-20h46m19s-rtsp___192.168.0.15_media_video2-.avi
    //默认名称: vlc-record-2021-09-26-20h46m16s-6e0b2be040a943489ef0b9bb344b96b8.hd.m3u8-.mp4
    //规范名称: vlc-record-2021-09-26-16h15m13s-Ch1-.avi vlc-record-2021-09-26-16h15m13s-Ch2-.mp4

    foreach (QString file, files) {
        //已经改名的不用处理(未改名的都是vlc-record-开头)
        if (!file.startsWith("vlc-record-")) {
            continue;
        }

        //取出文件名称的时间部分
        QStringList list = file.split("-");
        if (list.size() < 7) {
            continue;
        }

        //取出指定标识(如果有 -ch -video 字样说明是指定了名称)
        QString flag = list.at(6);
        QString flag2 = flag.toLower();
        QString suffix = file.split(".").last();
        if (!flag2.startsWith("ch") && !flag2.startsWith("video")) {
            //没有指定格式的就按照日期时间命名
            name = QString("%1-%2-%3-%4").arg(list.at(2)).arg(list.at(3)).arg(list.at(4)).arg(list.at(5));
            QDateTime dateTime = QDateTime::fromString(name, "yyyy-MM-dd-h'h'm'm's's'");
            name = dateTime.toString("yyyy-MM-dd-HH-mm-ss-zzz") + "." + suffix;
        }

        QString oldName = path + "/" + file;
        QString newName = path + "/" + name.replace("mp4", suffix);
        QFile::rename(oldName, newName);
        qDebug() << TIMEMS << QString("文件重名 -> 标识: %1 原名: %2 新名: %3").arg(flag).arg(file).arg(name);
        break;
    }
}

void VideoHelper::resetCursor()
{
    qApp->setOverrideCursor(Qt::ArrowCursor);
    qApp->restoreOverrideCursor();
}

void VideoHelper::loadVideoCore(QComboBox *cbox, int &videoCore)
{
    //冻结该控件所有信号
    cbox->blockSignals(true);
    cbox->clear();

#ifdef qmedia
    cbox->addItem("qmedia", 1);
#endif
#ifdef ffmpeg
    cbox->addItem("ffmpeg", 2);
#endif
#ifdef vlcx
    cbox->addItem("vlc", 3);
#endif
#ifdef mpvx
    cbox->addItem("mpv", 4);
#endif
#ifdef qtavx
    cbox->addItem("qtav", 5);
#endif
#ifdef haikang
    cbox->addItem("海康", 10);
#endif
#ifdef easyplayer
    cbox->addItem("easyplayer", 20);
#endif

    //设置默认值
    int index = cbox->findData(videoCore);
    cbox->setCurrentIndex(index < 0 ? 0 : index);
    videoCore = cbox->itemData(cbox->currentIndex()).toInt();

    //取消冻结信号
    cbox->blockSignals(false);
}

void VideoHelper::loadHardware(QComboBox *cbox, const VideoCore &videoCore, QString &hardware)
{
    //冻结该控件所有信号
    cbox->blockSignals(true);
    cbox->clear();

    QStringList list;
    list << "none";

    //ffmpeg建议用dxva2 vlc建议用any mpv建议用auto
    if (videoCore == VideoCore_FFmpeg) {
        //list << "qsv" << "cuvid";
    } else if (videoCore == VideoCore_Vlc) {
        list << "any";
    } else if (videoCore == VideoCore_Mpv) {
        list << "auto";
    }

    //因特尔显卡=vaapi 英伟达显卡=vdpau
#if defined(Q_OS_WIN)
    list << "dxva2" << "d3d11va";
#elif defined(Q_OS_LINUX)
    list << "vaapi" << "vdpau";
#elif defined(Q_OS_MAC)
    list << "videotoolbox";
#endif

    //安卓 mediacodec  树莓派 mmal  CUDA平台 nvdec cuda  瑞星微 rkmpp
    cbox->addItems(list);

    //设置默认值
    int index = list.indexOf(hardware);
    cbox->setCurrentIndex(index < 0 ? 0 : index);
    hardware = cbox->currentText();

    //取消冻结信号
    cbox->blockSignals(false);
}

void VideoHelper::loadCaching(QComboBox *cbox)
{
    cbox->clear();
    QStringList listText, listData;
    listText << "0.2 秒" << "0.3 秒" << "0.5 秒" << "1.0 秒" << "3.0 秒" << "5.0 秒" << "8.0 秒";
    listData << "200" << "300" << "500" << "1000" << "3000" << "5000" << "8000";

    int size = listText.size();
    for (int i = 0; i < size; ++i) {
        cbox->addItem(listText.at(i), listData.at(i));
    }
}

void VideoHelper::loadOpenSleepTime(QComboBox *cbox)
{
    cbox->addItem("1 秒", 1000);
    cbox->addItem("3 秒", 3000);
    cbox->addItem("5 秒", 5000);
}

void VideoHelper::loadReadTimeout(QComboBox *cbox)
{
    cbox->addItem("不处理", 0);
    cbox->addItem("5 秒", 5000);
    cbox->addItem("10 秒", 10000);
    cbox->addItem("15 秒", 15000);
    cbox->addItem("30 秒", 30000);
    cbox->addItem("60 秒", 60000);
}

void VideoHelper::loadConnectTimeout(QComboBox *cbox)
{
    cbox->addItem("不处理", 0);
    cbox->addItem("0.3 秒", 300);
    cbox->addItem("0.5 秒", 500);
    cbox->addItem("1.0 秒", 1000);
    cbox->addItem("3.0 秒", 3000);
    cbox->addItem("5.0 秒", 5000);
}

VideoType VideoHelper::getVideoType(const QString &videoUrl)
{
    VideoType videoType;
    if (videoUrl.startsWith("rtsp")) {
        videoType = VideoType_Rtsp;
    } else if (videoUrl.startsWith("rtmp")) {
        videoType = VideoType_Rtmp;
    } else if (videoUrl.startsWith("http")) {
        //有拓展名结尾的则认为是网络文件
        static QStringList suffixs;
        if (suffixs.size() == 0) {
            suffixs << ".mp3" << ".mp4" << ".flv";
        }

        int index = videoUrl.lastIndexOf(".");
        QString suffix = videoUrl.mid(index, 5).toLower();
        if (suffixs.contains(suffix)) {
            videoType = VideoType_FileHttp;
        } else {
            videoType = VideoType_Http;
        }
    } else if (videoUrl.startsWith("video=") || videoUrl.startsWith("/dev/") || videoUrl.startsWith("dshow") || videoUrl.startsWith("avdevice")) {
        videoType = VideoType_Camera;
    } else if (QFile(videoUrl).exists()) {
        videoType = VideoType_FileLocal;
    } else {
        videoType = VideoType_Other;
    }

    return videoType;
}

bool VideoHelper::getOnlyAudio(const QString &videoUrl)
{
    //如果还有其他可能的格式自行增加即可
    QStringList suffixs;
    suffixs << "mp3" << "wav" << "aac" << "wma" << "mid" << "ogg";

    bool onlyAudio = false;
    QString suffix = videoUrl.split(".").last();
    if (suffixs.contains(suffix)) {
        onlyAudio = true;
    }

    return onlyAudio;
}

void VideoHelper::getCameraPara(const VideoCore &videoCore, QString &videoUrl, QString &bufferSize, int &frameRate)
{
    //支持不同格式输入(根据不同内核自动设置)
    //video=USB2.0 PC CAMERA|1920x1080|30
    //dshow://:dshow-vdev=USB2.0 PC CAMERA|1920x1080|30

    QString url = videoUrl;
    url.replace("video=", "");
    url.replace("dshow://", "");
    url.replace(":dshow-vdev=", "");
    url.replace("avdevice:dshow:", "");
    AbstractVideoThread::checkCameraUrl(url, videoUrl, bufferSize, frameRate);

    if (videoCore == VideoCore_FFmpeg) {
#ifdef Q_OS_WIN
        //windows上前面要加上video=开头
        url = QString("video=%1").arg(videoUrl);
#else
        url = QString("%1").arg(videoUrl);
#endif
    } else if (videoCore == VideoCore_Vlc) {
        url = QString("dshow://:dshow-vdev=%1").arg(videoUrl);
    } else if (videoCore == VideoCore_Qtav) {
#ifdef Q_OS_WIN
        url = QString("avdevice:dshow:video=%1").arg(videoUrl);
#else
        url = QString("avdevice:dshow:%1").arg(videoUrl);
#endif
    } else if (videoCore == VideoCore_EasyPlayer) {
        url = QString("dshow://video=%1").arg(videoUrl);
    }

    videoUrl = url;
}

VideoType VideoHelper::initPara(WidgetPara &widgetPara, VideoPara &videoPara)
{
    //获取视频类型
    QString url = videoPara.videoUrl;
    VideoType videoType = VideoHelper::getVideoType(url);

    //视频地址不能为空
    if (url.isEmpty()) {
        return videoType;
    }

    //只有音频则只能绘制模式(需要绘制专辑封面)
    if (VideoHelper::getOnlyAudio(url)) {
        widgetPara.videoMode = VideoMode_Painter;
    }

    //udp://开头的表示必须用udp协议
    if (url.startsWith("udp://")) {
        videoPara.transport = "udp";
    }

    //下面的纠正仅仅是目前已经实现的(如果后面有增加对应处理需要调整)
    if (videoPara.videoCore == VideoCore_QMedia) {
        //qmedia内核下的GPU模式对应要用rgbwidget窗体
        if (widgetPara.videoMode == VideoMode_Opengl) {
            videoPara.hardware = "rgb";
        }

        //Qt4中的多媒体只有句柄模式(Qt5.6以下绘制模式用QAbstractVideoSurface获取不到图片也只能用句柄模式)
#if (QT_VERSION < QT_VERSION_CHECK(5,6,0))
        widgetPara.videoMode = VideoMode_Hwnd;
#endif
    } else if (videoPara.videoCore == VideoCore_FFmpeg) {
        //ffmpeg内核没有句柄模式
        if (widgetPara.videoMode == VideoMode_Hwnd) {
            widgetPara.videoMode = VideoMode_Opengl;
        }

        //标签和图形都绘制在源头
        widgetPara.osdDrawMode = DrawMode_Source;
        widgetPara.graphDrawMode = DrawMode_Source;
    } else if (videoPara.videoCore == VideoCore_Vlc) {
        //标签和图形都绘制在源头
        widgetPara.osdDrawMode = DrawMode_Source;
        widgetPara.graphDrawMode = DrawMode_Source;

        //vlc内核下的GPU模式对应要用rgbwidget窗体
        if (widgetPara.videoMode == VideoMode_Opengl) {
            videoPara.hardware = "rgb";
        }

        //非句柄模式下非本地文件必须指定分辨率
        if (videoPara.bufferSize == "0x0" && videoType != VideoType_FileLocal) {
            videoPara.bufferSize == "640x480";
        }
    } else if (videoPara.videoCore == VideoCore_Mpv) {
        //mpv内核只有句柄模式
        widgetPara.videoMode = VideoMode_Hwnd;
        //标签和图形都绘制在源头
        widgetPara.osdDrawMode = DrawMode_Source;
        widgetPara.graphDrawMode = DrawMode_Source;
    } else if (videoPara.videoCore == VideoCore_Qtav) {
        //qtav内核只有句柄模式
        widgetPara.videoMode = VideoMode_Hwnd;        
        //标签和图形都绘制在源头
        widgetPara.osdDrawMode = DrawMode_Source;
        widgetPara.graphDrawMode = DrawMode_Source;

        //本地摄像头目前不支持硬件加速
        if (videoType == VideoType_Camera) {
            videoPara.hardware = "none";
        }
    } else if (videoPara.videoCore == VideoCore_HaiKang) {
        //目前不支持硬件加速
        videoPara.hardware = "none";
    } else if (videoPara.videoCore == VideoCore_DaHua) {
        //目前不支持硬件加速
        videoPara.hardware = "none";
    } else if (videoPara.videoCore == VideoCore_EasyPlayer) {
        //easyplayer内核只有句柄模式
        widgetPara.videoMode = VideoMode_Hwnd;
    }

    //纠正缓存分辨率
    if (!videoPara.bufferSize.contains("x")) {
        videoPara.bufferSize = "0x0";
    }

    //如果地址带了摄像头参数则需要重新赋值
    if (videoType == VideoType_Camera) {
        VideoHelper::getCameraPara(videoPara.videoCore, videoPara.videoUrl, videoPara.bufferSize, videoPara.frameRate);
        //qDebug() << TIMEMS << QString("纠正地址 -> 地址: %1 -> %2").arg(url).arg(videoPara.videoUrl);
    }

    //本地文件以及本地摄像头不需要连接超时时间
    if (videoType == VideoType_FileLocal || videoType == VideoType_Camera) {
        videoPara.connectTimeout = 0;
    }

    //打开休息时间不宜过短(建议最低1秒)
    if (videoPara.openSleepTime < 1000) {
        videoPara.openSleepTime = 1000;
    }

    //读取超时时间不宜过短(建议最低5秒)
    if (videoPara.readTimeout > 0 && videoPara.readTimeout < 5000) {
        videoPara.readTimeout = 5000;
    }

    //连接超时时间=0表示对网络流不做提前连接测试处理
    if (videoPara.connectTimeout > 0) {
        //连接超时时间过长则自动调整打开休息时间(至少要2秒)
        if (videoPara.openSleepTime > 0 && (videoPara.openSleepTime - videoPara.connectTimeout) < 2000) {
            videoPara.openSleepTime = videoPara.connectTimeout + 2000;
        }

        //连接超时时间过长则自动调整读取超时时间(至少要3轮)
        if (videoPara.readTimeout > 0 && (videoPara.readTimeout / videoPara.connectTimeout) < 3) {
            videoPara.readTimeout = videoPara.connectTimeout * 3;
        }
    }

    //如果没有opengl则强制改成绘制模式
#ifndef openglx
    if (widgetPara.videoMode == VideoMode_Opengl) {
        widgetPara.videoMode = VideoMode_Painter;
    }
#endif

    //句柄模式不能共享解码线程
    if (widgetPara.videoMode == VideoMode_Hwnd) {
        widgetPara.sharedData = false;
    }

    return videoType;
}

VideoThread *VideoHelper::newVideoThread(QWidget *parent, const VideoCore &videoCore)
{
    VideoThread *videoThread = NULL;
    if (videoCore == VideoCore_QMedia) {
#ifdef qmedia
        videoThread = new QMediaThread(parent);
#endif
    } else if (videoCore == VideoCore_FFmpeg) {
#ifdef ffmpeg
        videoThread = new FFmpegThread(parent);
#endif
    } else if (videoCore == VideoCore_Vlc) {
#ifdef vlcx
        videoThread = new VlcThread(parent);
#endif
    } else if (videoCore == VideoCore_Mpv) {
#ifdef mpvx
        videoThread = new MpvThread(parent);
#endif
    } else if (videoCore == VideoCore_Qtav) {
#ifdef qtavx
        videoThread = new QtavThread(parent);
#endif
    } else if (videoCore == VideoCore_HaiKang) {
#ifdef haikang
        videoThread = new HaiKangThread(parent);
#endif
    } else if (videoCore == VideoCore_EasyPlayer) {
#ifdef easyplayer
        videoThread = new EasyPlayerThread(parent);
#endif
    }

    //如果都没有定义则实例化基类
    if (!videoThread) {
        videoThread = new VideoThread(parent);
    }

    return videoThread;
}

void VideoHelper::initVideoThread(VideoThread *videoThread, const VideoPara &videoPara)
{
    //设置一堆参数
    videoThread->setVideoCore(videoPara.videoCore);
    videoThread->setVideoUrl(videoPara.videoUrl);
    videoThread->setBufferSize(videoPara.bufferSize);
    videoThread->setFrameRate(videoPara.frameRate);

    videoThread->setDecodeType(videoPara.decodeType);
    videoThread->setHardware(videoPara.hardware);
    videoThread->setTransport(videoPara.transport);
    videoThread->setCaching(videoPara.caching);

    videoThread->setAudioLevel(videoPara.audioLevel);
    videoThread->setDecodeAudio(videoPara.decodeAudio);
    videoThread->setPlayAudio(videoPara.playAudio);
    videoThread->setPlayRepeat(videoPara.playRepeat);

    videoThread->setOpenSleepTime(videoPara.openSleepTime);
    videoThread->setReadTimeout(videoPara.readTimeout);
    videoThread->setConnectTimeout(videoPara.connectTimeout);
}
