#include "ffmpegrunthread.h"
#include "qapplication.h"
#include "qelapsedtimer.h"
#include "qprocess.h"
#include "qfile.h"

bool FFmpegRunThread::deleteSrcFile = true;
#ifdef Q_OS_WIN
QString FFmpegRunThread::ffmpegBin = "ffmpeg.exe";
#else
QString FFmpegRunThread::ffmpegBin = "ffmpeg";
#endif

void FFmpegRunThread::findFFmpegBin()
{
    static bool isFind = false;
    if (!isFind) {
        isFind = true;
        //先从当前目录或者lib目录查找
#if defined(Q_OS_WIN)
        QString file = qApp->applicationDirPath() + "/ffmpeg.exe";
#elif defined(Q_OS_LINUX)
        QString file = qApp->applicationDirPath() + "/lib/ffmpeg";
#elif defined(Q_OS_MAC)
        QString file = qApp->applicationDirPath() + "ffmpeg";
#endif
        if (QFile(file).exists()) {
            ffmpegBin = file;
        }
    }
}

QString FFmpegRunThread::deleteFile(const QStringList &args)
{
    //找到转换前后的文件名称
    QStringList fileSrcs;
    int size = args.size();
    QString fileDst = args.at(size - 1);
    for (int i = 0; i < size; ++i) {
        //紧跟 -i 参数后面的肯定是原文件
        if (args.at(i) == "-i") {
            fileSrcs << args.at(i + 1);
            i++;
        }
    }

    //删除转换前的文件(不需要可以注释)
    if (deleteSrcFile) {
        foreach (QString fileSrc, fileSrcs) {
            QFile(fileSrc).remove();
            qDebug() << TIMEMS << QString("删除文件 -> 文件: %1").arg(fileSrc);
        }
    }

    //如果带有-tmp表示临时文件需要重命名真正的名称
    if (fileDst.contains("-tmp")) {
        QString newName = fileDst;
        newName.replace("-tmp", "");
        QFile(fileDst).rename(newName);
    }

    return fileDst;
}

bool FFmpegRunThread::execute(const QStringList &args)
{
    //启动计时
    QElapsedTimer timer;
    timer.start();

    //先查找ffmpeg文件没找到则不用继续
    findFFmpegBin();
    if (!QFile(ffmpegBin).exists()) {
        //qDebug() << TIMEMS << QString("转换出错 -> 原因: %1").arg("未找到转换程序");
        //return false;
    }

    qDebug() << TIMEMS << QString("执行转换 -> 路径: %1").arg(ffmpegBin);
    qDebug() << TIMEMS << QString("执行转换 -> 命令: %1").arg(args.join(", "));

    //异步执行命令(Qt5.8版本以下会弹出运行黑框)
    //QProcess::startDetached(ffmpegBin, args);

    //换成不弹框的方式(结束的时候自动释放)
    //QProcess *process = new QProcess;
    //QObject::connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    //process->start(ffmpegBin, args);

    //再次换成异步阻塞等待完成方式
    QEventLoop eventLoop;
    QProcess process;
    QObject::connect(&process, SIGNAL(finished(int)), &eventLoop, SLOT(quit()));
    //QObject::connect(this, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    process.start(ffmpegBin, args);
    eventLoop.exec();

    QString fileDst = deleteFile(args);
    qDebug() << TIMEMS << QString("转换完成 -> 用时: %1 毫秒 文件: %2").arg(timer.elapsed()).arg(fileDst);
    return true;
}

FFmpegRunThread::FFmpegRunThread(QObject *parent) : QThread(parent)
{
    //线程结束自动释放
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void FFmpegRunThread::run()
{
    this->execute(args);
}

void FFmpegRunThread::startExecute(const QStringList &args)
{
    this->args = args;
    this->start();
}
