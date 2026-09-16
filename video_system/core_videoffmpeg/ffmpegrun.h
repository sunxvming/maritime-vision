#ifndef FFMPEGRUN_H
#define FFMPEGRUN_H

#include <QStringList>

class FFmpegRun
{
public:
    //执行处理
    static bool run(const QStringList &args, bool exec = true);

    //拓展名替换
    static QString replaceSuffix(const QString &fileSrc, const QString &suffix);

    //yuv420p文件转mp4文件
    static bool yuv420pToMp4(const QString &fileSrc);
    static bool yuv420pToMp4(const QString &fileSrc, int frameRate, int width, int height);
    static bool yuv420pToMp4(const QString &fileSrc, const QString &fileDst, int frameRate, int width, int height);

    //mp4文件转yuv420p文件
    static bool mp4ToYuv420p(const QString &fileSrc);
    static bool mp4ToYuv420p(const QString &fileSrc, const QString &fileDst, int width, int height);

    //音频转换 https://blog.csdn.net/Daibvly/article/details/121487204
    //wav文件转aac文件
    static bool wavToAac(const QString &fileSrc);
    static bool wavToAac(const QString &fileSrc, const QString &fileDst);

    //视频转换 https://www.cnblogs.com/renhui/p/9223969.html
    //合并aac以及h264文件或者mp4文件到带声音的mp4文件
    static bool aacAndH264ToMp4(const QString &fileSrc);
    static bool aacAndMp4ToMp4(const QString &fileSrc);
    static bool mergeToMp4(const QString &fileSrc1, const QString &fileSrc2, const QString &fileDst);

    //转换视频文件到mp4文件
    static bool convertMp4(const QString &fileSrc);
    static bool convertMp4(const QString &fileSrc, const QString &fileDst);
};

#endif // FFMPEGRUN_H
