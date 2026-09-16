#ifndef ABSTRACTSAVETHREAD_H
#define ABSTRACTSAVETHREAD_H

#include "widgethead.h"

class AbstractSaveThread : public QThread
{
    Q_OBJECT
public:
    //是否打印线程消息
    static bool debugInfo;
    //直接写入还是排队写入
    static bool directSave;
    //文件生成后是否调用转换合并
    static bool convertMerge;
    //是否执行了转换合并
    static bool isConvertMerge;

    explicit AbstractSaveThread(QObject *parent = 0);
    ~AbstractSaveThread();

protected:
    void run();

protected:
    //数据锁
    QMutex mutex;
    //停止线程标志位
    volatile bool stopped;
    //打开是否成功
    volatile bool isOk;
    //暂停写入标志位
    volatile bool isPause;    

    //唯一标识
    QString flag;
    //文件对象
    QFile file;
    //文件名称
    QString fileName;

    //保存视频文件类型
    SaveVideoType saveVideoType;
    //保存音频文件类型
    SaveAudioType saveAudioType;

    //视频宽度
    int videoWidth;
    //视频高度
    int videoHeight;
    //帧率
    double frameRate;

    //音频采样率
    int sampleRate;
    //音频通道数
    int channelCount;
    //音频品质
    int profile;

public:
    //是否已经打开
    bool getIsOk();
    //是否处于暂停
    bool getIsPause();
    //获取保存文件
    QString getFileName();
    //删除文件
    void deleteFile(const QString &fileName);

    //设置唯一标识(用来打印输出方便区分)
    void setFlag(const QString &flag);
    //统一格式打印信息
    void debug(const QString &head, const QString &msg);

private slots:
    //初始化
    virtual bool init();
    //保存数据
    virtual void save();
    //关闭释放
    virtual void close();

public slots:
    //开始保存
    virtual void open(const QString &fileName);
    //暂停保存
    virtual void pause();
    //停止保存
    virtual void stop();
};

#endif // ABSTRACTSAVETHREAD_H
