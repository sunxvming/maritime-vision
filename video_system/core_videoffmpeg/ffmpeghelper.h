#ifndef FFMPEGHELPER_H
#define FFMPEGHELPER_H

#ifdef videoffmpeg
#include "ffmpegthread.h"
#else
class FFmpegThread;
#include "ffmpeginclude.h"
#endif
#include "ffmpegsave.h"

class FFmpegHelper
{
public:
    //获取版本
    static QString getVersion();
    //将枚举值错误代号转成字符串
    static QString getError(int errnum);

    //打印输出各种信息 https://blog.csdn.net/xu13879531489/article/details/80703465
    static void debugInfo();
    //初始化库(一个软件中只需要初始化一次就行)
    static void initLib();

    //打印设备列表和参数 type: vfwcap dshow v4l2 avfoundation
    static void showDevice();
    static void showDevice(const QString &type);
    static void showOption(const QString &device);
    static void showOption(const QString &type, const QString &device);

    //格式枚举值转字符串
    static QString getFormatString(int format);
    static QString getFormatString(AVPixelFormat format);
    //根据分辨率获取码率
    static qint64 getBitRate(int width, int height);

    //视频帧旋转
    static void rotateFrame(int rotate, AVFrame *frameSrc, AVFrame *frameDst);

    //获取pts值(带矫正)
    static qint64 getPts(AVPacket *packet);
    //播放时刻值(单位秒)
    static double getPtsTime(AVFormatContext *formatCtx, AVPacket *packet);
    //播放时长值(单位秒)
    static double getDurationTime(AVFormatContext *formatCtx, AVPacket *packet);
    //延时时间值(单位微秒)
    static qint64 getDelayTime(AVFormatContext *formatCtx, AVPacket *packet, qint64 startTime);
    //根据时间差延时
    static void delayTime(AVFormatContext *formatCtx, AVPacket *packet, qint64 startTime);

    //根据硬解码类型找到对应的硬解码格式
    static enum AVPixelFormat find_fmt_by_hw_type(const enum AVHWDeviceType type);
    //硬解码格式
    static enum AVPixelFormat hw_pix_fmt;
    //获取硬解码格式回调函数
    static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);

    //通用硬解码(音频没有硬解码)
    static int decode(FFmpegThread *thread, AVCodecContext *avctx, AVPacket *packet, AVFrame *frameSrc, AVFrame *frameDst);
    //通用软解码(支持音频视频)
    static int decode(FFmpegThread *thread, AVCodecContext *avctx, AVPacket *packet, AVFrame *frame, bool video);
    //通用软编码(支持音频视频)
    static int encode(FFmpegSave *thread, AVCodecContext *avctx, AVPacket *packet, AVFrame *frame, bool video);

    //获取分辨率
    static void getResolution(AVStream *stream, int &width, int &height);
    //获取解码器名称
    static enum AVCodecID getCodecID(AVStream *stream);
    //拷贝上下文参数
    static int copyContext(AVCodecContext *avctx, AVStream *stream, bool from);
    //生成一个数据包对象
    static AVPacket *creatPacket(AVPacket *packet);

    //释放数据帧数据包
    static void freeFrame(AVFrame *frame);
    static void freePacket(AVPacket *packet);

    //超时回调(包括打开超时和读取超时)
    static int avinterruptCallBackFun(void *ctx);    
};

#endif // FFMPEGHELPER_H
