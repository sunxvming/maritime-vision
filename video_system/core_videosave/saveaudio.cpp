#include "saveaudio.h"
#include "savehelper.h"
#ifdef ffmpeg
#include "ffmpegrun.h"
#endif

SaveAudio::SaveAudio(QObject *parent) : AbstractSaveThread(parent)
{
}

void SaveAudio::save()
{
    //从队列中取出数据处理
    if (datas.size() > 0) {
        mutex.lock();
        QByteArray data = datas.takeFirst();
        mutex.unlock();
        this->save(data);
    }
}

void SaveAudio::save(const QByteArray &buffer)
{
    if (saveAudioType == SaveAudioType_Pcm || saveAudioType == SaveAudioType_Wav) {
        file.write(buffer);
    } else if (saveAudioType == SaveAudioType_Aac) {
        //每帧都要写入桢头再写入数据
        char header[7];
        SaveHelper::adtsHeader(header, buffer.length(), sampleRate, channelCount, profile);
        file.write(header, 7);
        file.write(buffer);
    }
}

void SaveAudio::close()
{
    //清空队列中的数据
    datas.clear();

    if (saveAudioType == SaveAudioType_Wav) {
        //执行pcm转wav文件
        QString wavFile = fileName;
        wavFile.replace("pcm", "wav");
        SaveHelper::pcmToWav(fileName, wavFile, sampleRate, channelCount);
#ifdef ffmpeg
        //转换成aac文件(不需要转换可以注释掉)
        isConvertMerge = false;
        if (convertMerge) {
            isConvertMerge = FFmpegRun::wavToAac(wavFile);
        }
#endif
    }
}

void SaveAudio::setPara(const SaveAudioType &saveAudioType, int sampleRate, int channelCount, int profile)
{
    this->saveAudioType = saveAudioType;
    this->sampleRate = sampleRate;
    this->channelCount = channelCount;
    this->profile = profile;
}

void SaveAudio::write(const char *data, qint64 len)
{
    //没打开或者暂停阶段不处理
    if (!isOk || isPause) {
        return;
    }

    QByteArray buffer = QByteArray(data, len);
    //可以直接写入到文件也可以排队处理
    if (directSave) {
        this->save(buffer);
    } else {
        mutex.lock();
        datas << buffer;
        mutex.unlock();
    }
}
