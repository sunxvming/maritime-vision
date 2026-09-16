#ifndef SAVEVIDEO_H
#define SAVEVIDEO_H

#include "abstractsavethread.h"

class SaveVideo : public AbstractSaveThread
{
    Q_OBJECT
public:
    explicit SaveVideo(QObject *parent = 0);

private:
    //数据队列
    QList<QByteArray> dataYs;
    QList<QByteArray> dataUs;
    QList<QByteArray> dataVs;

private slots:
    //保存数据
    void save();
    void save(const QByteArray &dataY, const QByteArray &dataU, const QByteArray &dataV);

    //关闭释放
    void close();

public slots:
    //设置参数
    void setPara(const SaveVideoType &saveVideoType, int videoWidth, int videoHeight, int frameRate);
    //写入数据
    void write(quint8 *dataY, quint8 *dataU, quint8 *dataV);
};

#endif // SAVEVIDEO_H
