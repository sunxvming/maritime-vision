#ifndef FFMPEGSAVE_H
#define FFMPEGSAVE_H

#include "ffmpeginclude.h"
#include "abstractsavethread.h"

//编码保存的大致流程
//01. 查找编码器 avcodec_find_encoder
//02. 创建编码器 avcodec_alloc_context3
//03. 设置编码器 pix_fmt/time_base/framerate/width/height
//04. 打开编码器 avcodec_open2
//06. 创建上下文 avformat_alloc_output_context2
//07. 创建输出流 avformat_new_stream
//08. 设置流参数 avcodec_parameters_from_context

//09. 写入开始符 avformat_write_header
//10. 发送数据帧 avcodec_send_frame
//11. 打包数据帧 avcodec_receive_packet
//12. 写入数据帧 av_interleaved_write_frame
//13. 写入结尾符 av_write_trailer

//14. 释放各资源 avcodec_free_context/avio_close/avformat_free_context

//特别提示
//1. 由于rtsp视频流收到的数据包packet就已经是标准的h264裸流带了各种pps啥的
//2. 所以可以直接录制打包成文件而不需要通过 avcodec_send_frame avcodec_receive_packet 编码
//3. 这样可以极大的降低CPU占用
//4. 本类其实好多种录制和编码方式集合看起来有点晕需要静下心来看

class FFmpegSave : public AbstractSaveThread
{
    Q_OBJECT
public:
    explicit FFmpegSave(QObject *parent = 0);
    ~FFmpegSave();

private:
    //统计包数量
    qint64 packetCount;
    //是否直接写入原始包（RTSP路径，不经过本地编码器）
    bool directCopyMode;
    //是否已收到第一个关键帧（等到关键帧才开始写入，避免开头灰屏）
    bool firstKeyFrameReceived;

    //视频帧
    AVPacket *videoPacket;
    //视频编码器
    AVCodecx *videoCodec;

    //视频编码器上下文
    AVCodecContext *videoCodecCtx;
    //输出文件格式上下文
    AVFormatContext *formatCtx;

    //输入文件视频流(参数传入)
    AVStream *videoStreamIn;
    //输出文件视频流
    AVStream *videoStreamOut;

    //视频帧队列
    QList<AVFrame *> videoFrames;
    //视频包队列
    QList<AVPacket *> videoPackets;

private:
    //初始化视频编码
    bool initVideoH264();
    bool initVideoMp4();

public:
    //编码写入数据包
    void writePacket(AVPacket *packet);

private slots:
    //初始化
    bool init();
    //保存数据
    void save();
    //关闭释放
    void close();

public slots:
    //设置参数
    void setPara(const SaveVideoType &saveVideoType, int videoWidth, int videoHeight, int frameRate, AVStream *videoStreamIn, bool camera);
    //设置是否直接写包（RTSP直接写原始包，不经过编码器）
    void setDirectCopy(bool direct);
    //写入视频帧
    void writeVideo(AVFrame *frame);
    //写入视频包
    void writeVideo(AVPacket *packet);
};

#endif // FFMPEGSAVE_H
