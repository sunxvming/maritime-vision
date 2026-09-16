#include "ffmpegsave.h"
#include "ffmpegrun.h"
#include "ffmpeghelper.h"

FFmpegSave::FFmpegSave(QObject *parent) : AbstractSaveThread(parent)
{
    packetCount = 0;
    directCopyMode = false;
    firstKeyFrameReceived = false;

    videoPacket = NULL;
    videoCodec = NULL;
    videoCodecCtx = NULL;
    formatCtx = NULL;

    videoStreamIn = NULL;
    videoStreamOut = NULL;
}

FFmpegSave::~FFmpegSave()
{

}

bool FFmpegSave::initVideoH264()
{
    //查找视频编码器(如果源头是H265则采用HEVC作为编码器)
    AVCodecID codecID = FFmpegHelper::getCodecID(videoStreamIn);
    if (codecID == AV_CODEC_ID_HEVC) {
        videoCodec = avcodec_find_encoder(AV_CODEC_ID_HEVC);
    } else {
        videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    }

    if (!videoCodec) {
        debug("编码失败", QString("错误: 查找视频编码器失败"));
        return false;
    }

    //创建视频编码器上下文
    videoCodecCtx = avcodec_alloc_context3(videoCodec);
    if (!videoCodecCtx) {
        debug("编码失败", QString("错误: 创建视频编码器上下文失败"));
        return false;
    }

    //参数说明 https://blog.csdn.net/qq_40179458/article/details/110449653
    //为了兼容低版本的编译器推荐选择第一种方式
#if 1
    //放大系数是为了小数位能够正确放大到整型
    int ratio = 10000;
    videoCodecCtx->time_base.num = 1 * ratio;
    videoCodecCtx->time_base.den = frameRate * ratio;
    videoCodecCtx->framerate.num = frameRate * ratio;
    videoCodecCtx->framerate.den = 1 * ratio;
#elif 0
    videoCodecCtx->time_base = {1, frameRate};
    videoCodecCtx->framerate = {frameRate, 1};
#else
    videoCodecCtx->time_base = videoStreamIn->codec->time_base;
    videoCodecCtx->framerate = videoStreamIn->codec->framerate;
#endif

    //大分辨率需要加上下面几个参数设置(否则在32位的库不能正常编码提示 Generic error in an external library)
    if ((videoWidth >= 3840 || videoHeight >= 2160)) {
        videoCodecCtx->qmin = 10;
        videoCodecCtx->qmax = 51;
        videoCodecCtx->me_range = 16;
        videoCodecCtx->max_qdiff = 4;
        videoCodecCtx->qcompress = 0.6;
    }

    //初始化视频编码器参数(如果要文件体积小一些画质差一些可以降低码率)
    videoCodecCtx->bit_rate = FFmpegHelper::getBitRate(videoWidth, videoHeight);
    videoCodecCtx->width = videoWidth;
    videoCodecCtx->height = videoHeight;
    videoCodecCtx->level = 50;
    videoCodecCtx->gop_size = 10;
    videoCodecCtx->max_b_frames = 3;
    videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    videoCodecCtx->profile = FF_PROFILE_H264_MAIN;

    //加上下面这个才能在文件属性中看到分辨率等信息 https://www.cnblogs.com/lidabo/p/15754031.html
    if (saveVideoType == SaveVideoType_Mp4) {
        videoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    //加速选项 https://www.jianshu.com/p/034f5b3e7f94
    //加载预设 https://blog.csdn.net/JineD/article/details/125304570
    //速度选项 ultrafast/superfast/veryfast/faster/fast/medium/slow/slower/veryslow/placebo
    //视觉优化 film/animation/grain/stillimage/psnr/ssim/fastdecode/zerolatency
    if (videoCodecCtx->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(videoCodecCtx->priv_data, "preset", "ultrafast", 0);
        //设置零延迟(本地摄像头视频流保存如果不设置则播放的时候会越来越模糊)
        av_opt_set(videoCodecCtx->priv_data, "tune", "zerolatency", 0);
    } else if (videoCodecCtx->codec_id == AV_CODEC_ID_HEVC) {
        av_opt_set(videoCodecCtx->priv_data, "preset", "ultrafast", 0);
        av_opt_set(videoCodecCtx->priv_data, "tune", "zero-latency", 0);
        //av_opt_set(videoCodecCtx->priv_data, "x265-params", "qp=20", 0);
    }

    //打开视频编码器
    int result = avcodec_open2(videoCodecCtx, videoCodec, NULL);
    if (result < 0) {
        debug("打开编码", QString("错误: 打开视频编码器失败 %1").arg(FFmpegHelper::getError(result)));
        return false;
    }

    videoPacket = FFmpegHelper::creatPacket(NULL);
    return true;
}

bool FFmpegSave::initVideoMp4()
{
    //必须先设置过输入视频流
    if (!videoStreamIn || fileName.isEmpty()) {
        return false;
    }

    QByteArray fileData = fileName.toUtf8();
    const char *filename = fileData.data();

    //开辟一个格式上下文用来处理视频流输出
    int result = avformat_alloc_output_context2(&formatCtx, NULL, "mp4", filename);
    if (result < 0) {
        debug("创建格式", QString("错误: %1").arg(FFmpegHelper::getError(result)));
        return false;
    }

    //创建视频流用来输出视频数据到文件
    videoStreamOut = avformat_new_stream(formatCtx, NULL);
    //直接写包模式：从输入流复制参数（保留摄像头真实的SPS/PPS extradata）
    //编码模式：从编码器复制参数（使用本地编码器生成的SPS/PPS）
    if (directCopyMode) {
        result = avcodec_parameters_copy(videoStreamOut->codecpar, videoStreamIn->codecpar);
        videoStreamOut->time_base = videoStreamIn->time_base;
    } else {
        result = FFmpegHelper::copyContext(videoCodecCtx, videoStreamOut, true);
    }
    if (result < 0) {
        debug("创建视频", QString("错误: %1").arg(FFmpegHelper::getError(result)));
        goto end;
    }

    //打开输出文件
    result = avio_open(&formatCtx->pb, filename, AVIO_FLAG_WRITE);
    if (result < 0) {
        debug("打开输出", QString("错误: %1").arg(FFmpegHelper::getError(result)));
        goto end;
    }

    //写入文件开始符
    result = avformat_write_header(formatCtx, NULL);
    if (result < 0) {
        debug("写入失败", QString("错误: %1").arg(FFmpegHelper::getError(result)));
        goto end;
    }

    return true;

end:
    //关闭释放并清理文件
    this->close();
    this->deleteFile(fileName);
    return false;
}

void FFmpegSave::writePacket(AVPacket *packet)
{
    packetCount++;
    if (saveVideoType == SaveVideoType_H264) {
        file.write((char *)packet->data, packet->size);
    } else if (saveVideoType == SaveVideoType_Mp4) {
        AVRational timeBaseIn = videoStreamIn->time_base;
        AVRational timeBaseOut = videoStreamOut->time_base;

        //没有下面这段判断在遇到不连续的帧的时候就会错位(相当于每次重新计算时间基准保证时间正确)
        //不连续帧的情况有暂停录制以及切换播放进度导致中间有些帧不需要录制
        double fps = frameRate;//av_q2d(videoStreamIn->r_frame_rate);
        double duration = AV_TIME_BASE / fps;
        packet->pts = (packetCount * duration) / (av_q2d(timeBaseIn) * AV_TIME_BASE);
        packet->dts = packet->pts;
        packet->duration = duration / (av_q2d(timeBaseIn) * AV_TIME_BASE);

        //重新调整时间基准
        packet->pts = av_rescale_q_rnd(packet->pts, timeBaseIn, timeBaseOut, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        packet->dts = packet->pts;
        //packet->dts = av_rescale_q_rnd(packet->dts, timeBaseIn, timeBaseOut, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        packet->duration = av_rescale_q(packet->duration, timeBaseIn, timeBaseOut);
        packet->pos = -1;

        //写入一帧数据
        //qDebug() << TIMEMS << frameRate << packet->pts << packet->duration;
        int result = av_interleaved_write_frame(formatCtx, packet);
        if (result < 0) {
            debug("写入失败", QString("错误: %1").arg(FFmpegHelper::getError(result)));
        }
    }

    av_packet_unref(packet);
}

bool FFmpegSave::init()
{
    bool ok = false;
    if (saveVideoType == SaveVideoType_H264) {
        ok = this->initVideoH264();
    } else if (saveVideoType == SaveVideoType_Mp4) {
        //直接写包模式不需要本地编码器，直接初始化容器即可
        if (directCopyMode) {
            ok = this->initVideoMp4();
        } else if (this->initVideoH264()) {
            ok = this->initVideoMp4();
        }
    }

    return ok;
}

void FFmpegSave::save()
{
    //从队列中取出数据处理
    //qDebug() << TIMEMS << videoFrames.size() << videoPackets.size();
    if (videoFrames.size() > 0) {
        mutex.lock();
        AVFrame *frame = videoFrames.takeFirst();
        mutex.unlock();
        FFmpegHelper::encode(this, videoCodecCtx, videoPacket, frame, true);
        FFmpegHelper::freeFrame(frame);
    }

    if (videoPackets.size() > 0) {
        mutex.lock();
        AVPacket *packet = videoPackets.takeFirst();
        mutex.unlock();
        this->writePacket(packet);
        FFmpegHelper::freePacket(packet);
    }
}

void FFmpegSave::close()
{
    //写入过开始符才能写入文件结束符(没有这个判断会报错)
    if (packetCount > 0 && saveVideoType == SaveVideoType_Mp4) {
        av_write_trailer(formatCtx);
    }

    //清空队列中的数据
    foreach (AVFrame *frame, videoFrames) {
        FFmpegHelper::freeFrame(frame);
    }

    foreach (AVPacket *packet, videoPackets) {
        FFmpegHelper::freePacket(packet);
    }

    packetCount = 0;
    firstKeyFrameReceived = false;
    videoFrames.clear();
    videoPackets.clear();

    //释放临时数据包
    if (videoPacket) {
        FFmpegHelper::freePacket(videoPacket);
        videoPacket = NULL;
    }

    //关闭编码器上下文并释放对象
    if (videoCodecCtx) {
        avcodec_free_context(&videoCodecCtx);
        videoCodec = NULL;
        videoCodecCtx = NULL;
    }

    //关闭文件流并释放对象
    if (formatCtx) {
        avio_close(formatCtx->pb);
        avformat_free_context(formatCtx);
        formatCtx = NULL;
        videoStreamOut = NULL;
        videoStreamIn = NULL;
    }

    //执行转换合并音频文件到一个文件
    isConvertMerge = false;
    if (convertMerge) {
        if (saveVideoType == SaveVideoType_H264) {
            isConvertMerge = FFmpegRun::aacAndH264ToMp4(fileName);
        } else if (saveVideoType == SaveVideoType_Mp4) {
            isConvertMerge = FFmpegRun::aacAndMp4ToMp4(fileName);
        }
    }
}

void FFmpegSave::setDirectCopy(bool direct)
{
    this->directCopyMode = direct;
}

void FFmpegSave::setPara(const SaveVideoType &saveVideoType, int videoWidth, int videoHeight, int frameRate, AVStream *videoStreamIn, bool camera)
{
    this->saveVideoType = saveVideoType;
    this->videoWidth = videoWidth;
    this->videoHeight = videoHeight;
    if (camera) {
        this->frameRate = frameRate > 15 ? 16.621 : frameRate;
    } else {
        this->frameRate = frameRate;
    }
    this->videoStreamIn = videoStreamIn;
}

void FFmpegSave::writeVideo(AVFrame *frame)
{
    //没打开或者暂停阶段不处理
    if (!isOk || isPause) {
        return;
    }

    //可以直接写入到文件也可以排队处理
    if (directSave) {
        FFmpegHelper::encode(this, videoCodecCtx, videoPacket, frame, true);
    } else {
        mutex.lock();
        videoFrames << av_frame_clone(frame);
        mutex.unlock();
    }
}

void FFmpegSave::writeVideo(AVPacket *packet)
{
    //没打开或者暂停阶段不处理
    if (!isOk || isPause) {
        return;
    }

    //等到第一个关键帧才开始写入，避免开头灰屏（P/B帧无参考帧则解码失败）
    if (!firstKeyFrameReceived) {
        if (!(packet->flags & AV_PKT_FLAG_KEY)) {
            return;
        }
        firstKeyFrameReceived = true;
    }

    //可以直接写入到文件也可以排队处理
    if (directSave) {
        this->writePacket(packet);
    } else {
        mutex.lock();
        videoPackets << FFmpegHelper::creatPacket(packet);
        mutex.unlock();
    }
}
