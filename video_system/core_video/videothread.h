#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include "videohead.h"
#include "abstractvideothread.h"

class VideoThread : public AbstractVideoThread
{
    Q_OBJECT
public:
    //保存整个应用程序所有的解码线程集合
    //引用计数(当前解码内核被几个窗体共用)
    int refCount;
    static QList<VideoThread *> videoThreads;
    static VideoThread *getVideoThread(const WidgetPara &widgetPara, const VideoPara &videoPara);

    explicit VideoThread(QObject *parent = 0);

protected:
    //打开后休息一下
    virtual void checkOpen();
    //线程执行内容
    virtual void run();

protected slots:
    //打开视频
    virtual bool openVideo();
    //关闭视频
    virtual void closeVideo();

protected:
    //文件总时长
    qint64 duration;
    //当前播放位置
    qint64 position;

    //解析内核
    VideoCore videoCore;
    //视频地址
    QString videoUrl;
    //视频类型
    VideoType videoType;
    //缓存分辨率
    QString bufferSize;

    //解码速度策略
    DecodeType decodeType;
    //硬件加速名称
    QString hardware;
    //通信协议(tcp udp)
    QString transport;
    //缓存时间(默认500毫秒)
    int caching;

    //开启音频振幅
    bool audioLevel;
    //是否解码音频
    bool decodeAudio;
    //是否播放声音
    bool playAudio;
    //重复循环播放
    bool playRepeat;

    //打开休息时间(最低1000 单位毫秒)
    int openSleepTime;
    //采集超时时间(0=不处理 单位毫秒)
    int readTimeout;
    //连接超时时间(0=不处理 单位毫秒)
    int connectTimeout;

public:
    //统一格式打印信息
    void debug(const QString &head, const QString &msg);

    //获取和设置解析内核
    VideoCore getVideoCore() const;
    void setVideoCore(const VideoCore &videoCore);

    //获取和设置视频地址
    QString getVideoUrl() const;
    void setVideoUrl(const QString &videoUrl);

    //获取视频是否是文件
    bool getIsFile() const;
    //获取是否只有音频
    bool getOnlyAudio() const;

    //获取视频类型
    VideoType getVideoType() const;

    //获取和设置缓存分辨率
    QString getBufferSize() const;
    void setBufferSize(const QString &bufferSize);

    //获取和设置解码策略
    DecodeType getDecodeType() const;
    void setDecodeType(const DecodeType &decodeType);

    //获取和设置硬件加速名称
    QString getHardware() const;
    void setHardware(const QString &hardware);

    //获取和设置通信协议
    QString getTransport() const;
    void setTransport(const QString &transport);

    //获取和设置缓存时间
    int getCaching() const;
    void setCaching(int caching);

    //获取和设置音频振幅
    bool getAudioLevel() const;
    void setAudioLevel(bool audioLevel);

    //获取和设置解码音频
    bool getDecodeAudio() const;
    void setDecodeAudio(bool decodeAudio);

    //获取和设置播放声音
    bool getPlayAudio() const;
    void setPlayAudio(bool playAudio);

    //获取和设置循环播放
    bool getPlayRepeat() const;
    void setPlayRepeat(bool playRepeat);

    //获取和设置打开休息时间
    int getOpenSleepTime() const;
    void setOpenSleepTime(int openSleepTime);

    //获取和设置采集超时时间
    int getReadTimeout() const;
    void setReadTimeout(int readTimeout);

    //获取和设置连接超时时间
    int getConnectTimeout() const;
    void setConnectTimeout(int connectTimeout);

public slots:
    //获取媒体信息
    virtual void readMediaInfo();

    //设置视频宽高比例
    virtual void setAspect(double width, double height);
    //获取文件时长
    virtual qint64 getDuration();

    //获取和设置播放位置
    virtual qint64 getPosition();
    virtual void setPosition(qint64 position);

    //获取和设置播放速度
    virtual double getSpeed();
    virtual void setSpeed(double speed);

    //获取和设置音量大小
    virtual int getVolume();
    virtual void setVolume(int volume);

    //获取和设置静音状态
    virtual bool getMuted();
    virtual void setMuted(bool muted);

public slots:
    //调用线程对应窗体停止
    virtual void stop2();
    //截图完成
    virtual void snapFinsh();

signals:
    //文件时长
    void receiveDuration(qint64 duration);
    //播放时长
    void receivePosition(qint64 position);
};

#endif // VIDEOTHREAD_H
