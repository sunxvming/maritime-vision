#include "ffmpegrun.h"
#include "ffmpegrunthread.h"
#include "qfileinfo.h"

bool FFmpegRun::run(const QStringList &args, bool exec)
{
    if (exec) {
        return FFmpegRunThread::execute(args);
    } else {
        //每个指令都开启独立的线程执行(执行完成会立即释放线程)
        FFmpegRunThread *runThread = new FFmpegRunThread;
        runThread->startExecute(args);
        return true;
    }
}

QString FFmpegRun::replaceSuffix(const QString &fileSrc, const QString &suffix)
{
    QString fileDst = fileSrc;
    fileDst.replace(QFileInfo(fileSrc).suffix(), suffix);
    return fileDst;
}

bool FFmpegRun::yuv420pToMp4(const QString &fileSrc)
{
    //根据文件名自动提取宽度高度帧率
    //要求名字格式 640x408x30.xxx 640_480_30.xxx 1920x1080_aaaa.xxx
    int frameRate = 25;
    int width = 640;
    int height = 480;

    QString name = QFileInfo(fileSrc).baseName();
    QStringList list;
    if (name.contains("_")) {
        list = name.split("_");
    } else if (name.contains("x")) {
        list = name.split("x");
    }

    if (list.size() >= 2) {
        width = list.at(0).toInt();
        height = list.at(1).toInt();
    }
    if (list.size() >= 3) {
        frameRate = list.at(2).toInt();
    }

    return yuv420pToMp4(fileSrc, frameRate, width, height);
}

bool FFmpegRun::yuv420pToMp4(const QString &fileSrc, int frameRate, int width, int height)
{
    QString fileDst = replaceSuffix(fileSrc, "mp4");
    return yuv420pToMp4(fileSrc, fileDst, frameRate, width, height);
}

//录制视频 ffmpeg -rtsp_transport tcp -y -re -i rtsp://admin:Admin123456@192.168.0.64:554/Streaming/Channels/102 -vcodec copy -t 00:00:10.00 -f mp4 f:/output.mp4
//播放音频 ffplay -ar 48000 -ac 2 -f s16le -i d:/out.pcm
//倍速播放 ffplay f:/mp4/1.mp4 -af atempo=2 -vf setpts=PTS/2

//-c:v libx264 -preset fast -crf 18  framerate=r  video_size=s  vcodec=c:v  acodec=c:a
//ffmpeg.exe -threads auto -f rawvideo -framerate 30 -video_size 640x480 -pix_fmt yuv420p -i e:/1.yuv e:/1.mp4
//ffmpeg -threads auto -f rawvideo -r 30 -s 640x480 -pix_fmt yuv420p -i /home/liu/1.yuv /home/liu/1.mp4
bool FFmpegRun::yuv420pToMp4(const QString &fileSrc, const QString &fileDst, int frameRate, int width, int height)
{
    QStringList args;
    args << "-threads" << "auto";
    args << "-f" << "rawvideo";
#if 0
    args << "-framerate" << QString::number(frameRate);
    args << "-video_size" << QString("%1x%2").arg(width).arg(height);
#else
    args << "-r" << QString::number(frameRate);
    args << "-s" << QString("%1x%2").arg(width).arg(height);
#endif
    args << "-pix_fmt" << "yuv420p";
    args << "-i" << fileSrc;
    //下面这些参数非必须(linux上不支持)
#if 0
    args << "-vcodec" << "libx264";
    args << "-preset" << "fast";
    args << "-crf" << QString::number(18);
#endif
    args << fileDst;
    return run(args);
}

bool FFmpegRun::mp4ToYuv420p(const QString &fileSrc)
{
    QString fileDst = replaceSuffix(fileSrc, "yuv");
    return mp4ToYuv420p(fileSrc, fileDst, 0, 0);
}

//ffmpeg.exe -i e:/1.mp4 -s 640x480 -pix_fmt yuv420p e:/1.yuv
bool FFmpegRun::mp4ToYuv420p(const QString &fileSrc, const QString &fileDst, int width, int height)
{
    QStringList args;
    //尺寸为空则自动采用默认的
    if (width > 0 && height > 0) {
        args << "-s" << QString("%1x%2").arg(width).arg(height);
    }

    args << "-pix_fmt" << "yuv420p";
    args << "-i" << fileSrc;
    args << "-y" << fileDst;
    return run(args);
}

bool FFmpegRun::wavToAac(const QString &fileSrc)
{
    QString fileDst = replaceSuffix(fileSrc, "aac");
    return wavToAac(fileSrc, fileDst);
}

bool FFmpegRun::wavToAac(const QString &fileSrc, const QString &fileDst)
{
    QStringList args;
    args << "-i" << fileSrc;
    args << fileDst;
    return run(args, true);
}

bool FFmpegRun::aacAndH264ToMp4(const QString &fileSrc)
{
    QString aacFile = replaceSuffix(fileSrc, "aac");
    QString fileDst = replaceSuffix(fileSrc, "mp4");
    return mergeToMp4(aacFile, fileSrc, fileDst);
}

bool FFmpegRun::aacAndMp4ToMp4(const QString &fileSrc)
{
    QString aacFile = replaceSuffix(fileSrc, "aac");
    QString mp4File = replaceSuffix(fileSrc, "mp4");
    QString baseName = fileSrc;
    baseName.replace(".mp4", "");
    //搞个临时的名称转换好以后再重命名
    QString fileDst = baseName + "-tmp.mp4";
    return mergeToMp4(aacFile, mp4File, fileDst);
}

bool FFmpegRun::mergeToMp4(const QString &fileSrc1, const QString &fileSrc2, const QString &fileDst)
{
    //文件不存在不用继续
    if (QFile(fileSrc1).size() == 0 || QFile(fileSrc2).size() == 0) {
        return false;
    }

    //ffmpeg -i d:/1.aac -i d:/1.mp4 -y d:/out.mp4
    QStringList args;
    //QString arg = "-vcodec copy -acodec copy";
    //QString arg = "-c:v copy -c:a aac -strict experimental";
    //args << arg.split(" ");

    args << "-i" << fileSrc1;
    args << "-i" << fileSrc2;
    args << "-y" << fileDst;
    return run(args);
}

bool FFmpegRun::convertMp4(const QString &fileSrc)
{
    QString baseName = fileSrc;
    baseName.replace(".mp4", "");
    //搞个临时的名称转换好以后再重命名
    QString fileDst = baseName + "-tmp.mp4";
    return convertMp4(fileSrc, fileDst);
}

bool FFmpegRun::convertMp4(const QString &fileSrc, const QString &fileDst)
{
    //文件不存在不用继续
    if (QFile(fileSrc).size() == 0) {
        return false;
    }

    //ffmpeg -i d:/1.mp4 -y d:/out.mp4
    QStringList args;
    args << "-i" << fileSrc;
    args << "-y" << fileDst;
    return run(args);
}
