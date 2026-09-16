#include "abstractsavethread.h"

bool AbstractSaveThread::debugInfo = true;
bool AbstractSaveThread::directSave = true;
bool AbstractSaveThread::convertMerge = false;
bool AbstractSaveThread::isConvertMerge = false;
AbstractSaveThread::AbstractSaveThread(QObject *parent) : QThread(parent)
{
    stopped = false;
    isOk = false;
    isPause = false;

    saveVideoType = SaveVideoType_None;
    saveAudioType = SaveAudioType_None;

    videoWidth = 640;
    videoHeight = 480;
    frameRate = 25;

    sampleRate = 8000;
    channelCount = 1;
    profile = 1;
}

AbstractSaveThread::~AbstractSaveThread()
{
    this->stop();
}

void AbstractSaveThread::run()
{
    while (!stopped) {
        this->save();
        msleep(1);
    }

    stopped = false;
    isOk = false;
    isPause = false;
}

bool AbstractSaveThread::getIsOk()
{
    return this->isOk;
}

bool AbstractSaveThread::getIsPause()
{
    return this->isPause;
}

QString AbstractSaveThread::getFileName()
{
    return this->fileName;
}

void AbstractSaveThread::deleteFile(const QString &fileName)
{
    //如果文件大小0则删除
    QFile file(fileName);
    if (file.exists() && file.size() == 0) {
        file.remove();
        debug("失败删除", "");
    }
}

void AbstractSaveThread::setFlag(const QString &flag)
{
    this->flag = flag;
}

void AbstractSaveThread::debug(const QString &head, const QString &msg)
{
    if (!debugInfo) {
        return;
    }

    //如果设置了唯一标识则放在打印前面
    QString text = head;
    if (!flag.isEmpty()) {
        text = QString("标识[%1] %2").arg(flag).arg(text);
    }

    if (msg.isEmpty()) {
        qDebug() << TIMEMS << QString("%1 -> 文件: %2").arg(text).arg(fileName);
    } else {
        qDebug() << TIMEMS << QString("%1 -> %2 文件: %3").arg(text).arg(msg).arg(fileName);
    }
}

bool AbstractSaveThread::init()
{
    return true;
}

void AbstractSaveThread::save()
{

}

void AbstractSaveThread::close()
{

}

void AbstractSaveThread::open(const QString &fileName)
{
    //已经打开不用继续防止重复打开
    if (isOk) {
        return;
    }

    //先初始化
    this->fileName = fileName;
    if (!this->init()) {
        return;
    }

    //调整视频文件拓展名
    if (saveVideoType == SaveVideoType_Yuv) {
        this->fileName.replace("mp4", "yuv");
    } else if (saveVideoType == SaveVideoType_H264) {
        this->fileName.replace("mp4", "h264");
    }

    //调整音频文件拓展名
    if (saveAudioType == SaveAudioType_Pcm || saveAudioType == SaveAudioType_Wav) {
        this->fileName.replace("mp4", "pcm");
    } else if (saveAudioType == SaveAudioType_Aac) {
        this->fileName.replace("mp4", "aac");
    }

    //视频保存到MP4用的是内部接口打开文件
    file.setFileName(this->fileName);
    if (saveVideoType != SaveVideoType_Mp4) {
        isOk = file.open(QFile::WriteOnly);
    } else {
        isOk = true;
    }

    //启动线程
    this->start();
    debug("开始录制", "");
}

void AbstractSaveThread::pause()
{
    if (!isOk) {
        return;
    }

    isPause = !isPause;
    debug(isPause ? "暂停录制" : "继续录制", "");
}

void AbstractSaveThread::stop()
{
    if (!isOk) {
        return;
    }

    //处于运行状态才可以停止
    if (this->isRunning()) {
        stopped = true;
        isPause = false;
        this->wait();
    }

    //关闭文件
    isOk = false;
    debug("结束录制", "");
    if (file.isOpen()) {
        file.close();
    }

    //关闭释放并清理文件
    this->close();
    this->deleteFile(fileName);
}
