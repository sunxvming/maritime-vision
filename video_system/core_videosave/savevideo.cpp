#include "savevideo.h"
#ifdef ffmpeg
#include "ffmpegrun.h"
#endif

SaveVideo::SaveVideo(QObject *parent) : AbstractSaveThread(parent)
{
}

void SaveVideo::save()
{
    //从队列中取出数据处理
    if (dataYs.size() > 0) {
        mutex.lock();
        QByteArray dataY = dataYs.takeFirst();
        QByteArray dataU = dataUs.takeFirst();
        QByteArray dataV = dataVs.takeFirst();
        mutex.unlock();
        this->save(dataY, dataU, dataV);
    }
}

void SaveVideo::save(const QByteArray &dataY, const QByteArray &dataU, const QByteArray &dataV)
{
    if (saveVideoType == SaveVideoType_Yuv) {
        file.write(dataY);
        file.write(dataU);
        file.write(dataV);
    }
}

void SaveVideo::close()
{
    //清空队列中的数据
    dataYs.clear();
    dataUs.clear();
    dataVs.clear();

    if (saveVideoType == SaveVideoType_Yuv) {
#ifdef ffmpeg
        //转换成mp4文件(不需要转换可以注释掉)
        isConvertMerge = false;
        if (convertMerge) {
            isConvertMerge = FFmpegRun::yuv420pToMp4(fileName, frameRate, videoWidth, videoHeight);
        }
#endif
    }
}

void SaveVideo::setPara(const SaveVideoType &saveVideoType, int videoWidth, int videoHeight, int frameRate)
{
    this->saveVideoType = saveVideoType;
    this->videoWidth = videoWidth;
    this->videoHeight = videoHeight;
    this->frameRate = frameRate;
}

void SaveVideo::write(quint8 *dataY, quint8 *dataU, quint8 *dataV)
{
    //没打开或者暂停阶段不处理
    if (!isOk || isPause) {
        return;
    }

    //yuv420p一帧录制为1.5倍的宽度x高度个字节
    int size = videoWidth * videoHeight;
    QByteArray bufferY = QByteArray((char *)dataY, size);
    QByteArray bufferU = QByteArray((char *)dataU, size / 4);
    QByteArray bufferV = QByteArray((char *)dataV, size / 4);

    //可以直接写入到文件也可以排队处理
    if (directSave) {
        this->save(bufferY, bufferU, bufferV);
    } else {
        mutex.lock();
        dataYs << bufferY;
        dataUs << bufferU;
        dataVs << bufferV;
        mutex.unlock();
    }
}
