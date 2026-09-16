#ifndef SAVEAUDIO_H
#define SAVEAUDIO_H

#include "abstractsavethread.h"

class SaveAudio : public AbstractSaveThread
{
    Q_OBJECT
public:
    explicit SaveAudio(QObject *parent = 0);

private:
    //数据队列
    QList<QByteArray> datas;

private slots:
    //保存数据
    void save();
    void save(const QByteArray &buffer);

    //关闭释放
    void close();

public slots:
    //设置参数
    void setPara(const SaveAudioType &saveAudioType, int sampleRate, int channelCount, int profile);
    //写入数据
    void write(const char *data, qint64 len);
};

#endif // SAVEAUDIO_H
