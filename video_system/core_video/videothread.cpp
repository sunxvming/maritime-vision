#include "videothread.h"
#include "videohelper.h"
#include "videotask.h"

QList<VideoThread *> VideoThread::videoThreads;
VideoThread *VideoThread::getVideoThread(const WidgetPara &widgetPara, const VideoPara &videoPara)
{
    //句柄模式不能共用
    if (!widgetPara.sharedData || widgetPara.videoMode == VideoMode_Hwnd) {
        return NULL;
    }

    VideoThread *thread = NULL;
    foreach (VideoThread *videoThread, videoThreads) {
        //解析内核和视频地址一致才能唯一决定一个采集线程
        if (videoThread->getVideoCore() == videoPara.videoCore && videoThread->getVideoUrl() == videoPara.videoUrl) {
            //限定纯音频不用共享
            if (!videoThread->getOnlyAudio()) {
                videoThread->refCount++;
                thread = videoThread;
            }
            break;
        }
    }

    return thread;
}

VideoThread::VideoThread(QObject *parent) : AbstractVideoThread(parent)
{
    refCount = 0;
    duration = 0;
    position = 0;
    rotate = -1;

    videoCore = VideoCore_None;
    videoUrl = "";
    videoType = VideoType_Other;

    videoWidth = 0;
    videoHeight = 0;
    bufferSize = "0x0";

    decodeType = DecodeType_Fast;
    hardware = "none";
    transport = "tcp";
    caching = 500;

    audioLevel = false;
    decodeAudio = true;
    playAudio = true;
    playRepeat = false;

    openSleepTime = 3000;
    readTimeout = 0;
    connectTimeout = 500;

    //启动任务处理线程做一些额外的处理
    VideoTask::Instance()->start();
}

void VideoThread::checkOpen()
{
    //特意每次做个小延时每次都去判断标志位等可以大大加快关闭速度
    int count = 0;
    int maxCount = openSleepTime / 100;
    while (!stopped) {
        msleep(100);
        count++;
        //测试下来正常情况下基本上等待一次后 isOk=true
        if (count == maxCount || isOk) {
            break;
        }
    }
}

void VideoThread::run()
{
    while (!stopped) {
        if (!isOk) {
            this->closeVideo();
            if (videoMode == VideoMode_Hwnd) {
                QMetaObject::invokeMethod(this, "openVideo");
            } else {
                this->openVideo();
            }

            this->checkOpen();
            continue;
        }

        if (videoCore == VideoCore_Vlc) {
            if (videoWidth <= 0 && !getOnlyAudio()) {
                //视频文件需要尝试读取媒体信息多次保证能够读取到(一般视频流需要多次才能读取到)
                this->readMediaInfo();
            }
        } else if (videoCore == VideoCore_HaiKang) {
            if (videoType == VideoType_FileLocal) {
                //本地文件需要这里实时读取播放进度
                this->getPosition();
            } else if (isOk && videoMode == VideoMode_Hwnd) {
                //句柄模式下视频流如果打开正常了则sdk内部处理重连
                lastTime = QDateTime::currentDateTime();
            }
        } else if (videoCore == VideoCore_EasyPlayer) {
            if (videoWidth <= 0) {
                this->readMediaInfo();
            }

            this->getPosition();
        }

        //启用了自动重连则通过判断最后的消息时间(超时则重新打开)
        if (readTimeout > 0) {
            qint64 offset = lastTime.msecsTo(QDateTime::currentDateTime());
            if (offset >= readTimeout) {
                isOk = false;
                debug("超时重连", "");
                continue;
            }
        }

        msleep(100);
    }

    //关闭视频
    this->closeVideo();

    //文件名为空才说明真正处理完可以彻底结束线程(否则一直等因为有可能文件还没保存完成)
    while (!fileName.isEmpty()) {
        debug("等待完成", "");
        msleep(5);
    }

    debug("线程结束", "");
}

bool VideoThread::openVideo()
{
    return false;
}

void VideoThread::closeVideo()
{
    duration = 0;
    position = 0;

    videoWidth = 0;
    videoHeight = 0;
    rotate = -1;

    errorCount = 0;
    snapName = "";
    fileName = "";

    stopped = false;
    isOk = false;
    isPause = false;
    isSnap = false;
    isRecord = false;
    debug("关闭线程", "");
    emit receivePlayFinsh();
}

void VideoThread::debug(const QString &head, const QString &msg)
{
    AbstractVideoThread::debug(head, msg, videoUrl);
}

VideoCore VideoThread::getVideoCore() const
{
    return this->videoCore;
}

void VideoThread::setVideoCore(const VideoCore &videoCore)
{
    this->videoCore = videoCore;
}

QString VideoThread::getVideoUrl() const
{
    return this->videoUrl;
}

void VideoThread::setVideoUrl(const QString &videoUrl)
{
    //海康大华等厂家sdk只支持rtsp和mp4
    if (videoCore == VideoCore_HaiKang || videoCore == VideoCore_DaHua) {
        if (!videoUrl.startsWith("rtsp") && !videoUrl.endsWith(".mp4")) {
            this->videoUrl = "";
            return;
        }
    }

    this->videoUrl = videoUrl;
    this->videoType = VideoHelper::getVideoType(videoUrl);
    this->onlyAudio = VideoHelper::getOnlyAudio(videoUrl);
}

bool VideoThread::getIsFile() const
{
    return (videoType == VideoType_FileLocal || videoType == VideoType_FileHttp);
}

bool VideoThread::getOnlyAudio() const
{
    return this->onlyAudio;
}

VideoType VideoThread::getVideoType() const
{
    return this->videoType;
}

QString VideoThread::getBufferSize() const
{
    return this->bufferSize;
}

void VideoThread::setBufferSize(const QString &bufferSize)
{
    this->bufferSize = bufferSize;
}

DecodeType VideoThread::getDecodeType() const
{
    return this->decodeType;
}

void VideoThread::setDecodeType(const DecodeType &decodeType)
{
    this->decodeType = decodeType;
}

QString VideoThread::getHardware() const
{
    return this->hardware;
}

void VideoThread::setHardware(const QString &hardware)
{
    this->hardware = hardware;
}

QString VideoThread::getTransport() const
{
    return this->transport;
}

void VideoThread::setTransport(const QString &transport)
{
    this->transport = transport;
}

int VideoThread::getCaching() const
{
    return this->caching;
}

void VideoThread::setCaching(int caching)
{
    this->caching = caching;
}

bool VideoThread::getAudioLevel() const
{
    return this->audioLevel;
}

void VideoThread::setAudioLevel(bool audioLevel)
{
    this->audioLevel = audioLevel;
}

bool VideoThread::getDecodeAudio() const
{
    return this->decodeAudio;
}

void VideoThread::setDecodeAudio(bool decodeAudio)
{
    this->decodeAudio = decodeAudio;
}

bool VideoThread::getPlayAudio() const
{
    return this->playAudio;
}

void VideoThread::setPlayAudio(bool playAudio)
{
    this->playAudio = playAudio;
}

bool VideoThread::getPlayRepeat() const
{
    return this->playRepeat;
}

void VideoThread::setPlayRepeat(bool playRepeat)
{
    this->playRepeat = playRepeat;
}

int VideoThread::getOpenSleepTime() const
{
    return this->openSleepTime;
}

void VideoThread::setOpenSleepTime(int openSleepTime)
{
    this->openSleepTime = openSleepTime;
}

int VideoThread::getReadTimeout() const
{
    return this->readTimeout;
}

void VideoThread::setReadTimeout(int readTimeout)
{
    this->readTimeout = readTimeout;
}

int VideoThread::getConnectTimeout() const
{
    return this->connectTimeout;
}

void VideoThread::setConnectTimeout(int connectTimeout)
{
    this->connectTimeout = connectTimeout;
}

void VideoThread::readMediaInfo()
{

}

void VideoThread::setAspect(double width, double height)
{

}

qint64 VideoThread::getDuration()
{
    return duration;
}

qint64 VideoThread::getPosition()
{
    return position;
}

void VideoThread::setPosition(qint64 position)
{
    emit receivePosition(position);
}

double VideoThread::getSpeed()
{
    return 1;
}

void VideoThread::setSpeed(double speed)
{

}

int VideoThread::getVolume()
{
    return 100;
}

void VideoThread::setVolume(int volume)
{
    emit receiveVolume(volume);
}

bool VideoThread::getMuted()
{
    return false;
}

void VideoThread::setMuted(bool muted)
{
    emit receiveMuted(muted);
}

void VideoThread::stop2()
{
    if (readTimeout > 0 && isOk && isRunning()) {
        debug("重新打开", "");
        isOk = false;
        return;
    }

    //调用父类的停止更彻底
    QWidget *w = (QWidget *)this->parent();
    while (!w->inherits("VideoWidget")) {
        w = (QWidget *)w->parent();
        if (!w) {
            break;
        }
    }

    if (w) {
        QMetaObject::invokeMethod(w, "stop");
    }
}

void VideoThread::snapFinsh()
{
    //文件已经存在不用重新保存(png格式重新保存为jpg以减少体积)
    QImage image(snapName);
    if (snapName.endsWith(".png")) {
        QFile(snapName).remove();
        snapName = snapName.replace(".png", ".jpg");
        image.save(snapName, "jpg");
    }

    isSnap = false;
    emit snapImage(image, snapName);
}
