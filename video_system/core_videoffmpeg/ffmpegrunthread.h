#ifndef FFMPEGRUNTHREAD_H
#define FFMPEGRUNTHREAD_H

#include "ffmpeginclude.h"
#include <QThread>
#include <QStringList>

class FFmpegRunThread : public QThread
{
    Q_OBJECT
public:
    //执行完成是否删除转换前的文件
    static bool deleteSrcFile;
    //可执行文件路径
    static QString ffmpegBin;

    //自动查找可执行文件路径
    static void findFFmpegBin();
    //删除转换前的文件
    static QString deleteFile(const QStringList &args);
    //立即执行转换
    static bool execute(const QStringList &args);

    explicit FFmpegRunThread(QObject *parent = 0);

protected:
    void run();

private:
    //执行的转换命令
    QStringList args;

public slots:
    //立即执行命令
    void startExecute(const QStringList &args);
};

#endif // FFMPEGRUNTHREAD_H
