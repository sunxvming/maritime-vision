#include "ffmpeghelper.h"
#include "qmutex.h"
#include "qstringlist.h"

QString FFmpegHelper::getVersion()
{
    return QString("%1").arg(FFMPEG_VERSION);
}

QString FFmpegHelper::getError(int errnum)
{
    //常见错误码 https://blog.csdn.net/sz76211822/article/details/52371966
    char errbuf[256] = { 0 };
    av_strerror(errnum, errbuf, sizeof(errbuf));
    return errbuf;
}

//http://ruiy.leanote.com/post/c-%E4%BD%BF%E7%94%A8ffmpeg
void FFmpegHelper::debugInfo()
{
    //输出所有支持的解码器名称
    QStringList listEncoderVideo, listEncoderAudio, listEncoderOther;
    QStringList listDecoderVideo, listDecoderAudio, listDecoderOther;
#if (FFMPEG_VERSION_MAJOR < 4)
    AVCodec *coder = av_codec_next(NULL);
    while (coder != NULL) {
#else
    void *opaque = NULL;
    const AVCodec *coder;
    while ((coder = av_codec_iterate(&opaque)) != NULL) {
#endif
        QString name = QString("%1").arg(coder->name);
        if (av_codec_is_encoder(coder)) {
            if (coder->type == AVMEDIA_TYPE_VIDEO) {
                listEncoderVideo << name;
            } else if (coder->type == AVMEDIA_TYPE_AUDIO) {
                listEncoderAudio << name;
            } else {
                listEncoderOther << name;
            }
        } else if (av_codec_is_decoder(coder)) {
            if (coder->type == AVMEDIA_TYPE_VIDEO) {
                listDecoderVideo << name;
            } else if (coder->type == AVMEDIA_TYPE_AUDIO) {
                listDecoderAudio << name;
            } else {
                listDecoderOther << name;
            }
        }
#if (FFMPEG_VERSION_MAJOR < 4)
        coder = coder->next;
#endif
    }

    qDebug() << TIMEMS << QString("视频编码 -> %1").arg(listEncoderVideo.join("/"));
    qDebug() << TIMEMS << QString("音频编码 -> %1").arg(listEncoderAudio.join("/"));
    qDebug() << TIMEMS << QString("其他编码 -> %1").arg(listEncoderOther.join("/"));
    qDebug() << TIMEMS << QString("视频解码 -> %1").arg(listDecoderVideo.join("/"));
    qDebug() << TIMEMS << QString("音频解码 -> %1").arg(listDecoderAudio.join("/"));
    qDebug() << TIMEMS << QString("其他解码 -> %1").arg(listDecoderOther.join("/"));

    //输出支持的协议
    QStringList listProtocolsIn, listProtocolsOut;
    struct URLProtocol *protocol = NULL;
    struct URLProtocol **protocol2 = &protocol;

    avio_enum_protocols((void **)protocol2, 0);
    while ((*protocol2)) {
        listProtocolsIn << avio_enum_protocols((void **)protocol2, 0);
    }

    protocol = NULL;
    avio_enum_protocols((void **)protocol2, 1);
    while ((*protocol2)) {
        listProtocolsOut << avio_enum_protocols((void **)protocol2, 1);
    }

    qDebug() << TIMEMS << QString("输入协议 -> %1").arg(listProtocolsIn.join("/"));
    qDebug() << TIMEMS << QString("输出协议 -> %1").arg(listProtocolsOut.join("/"));
}

void FFmpegHelper::initLib()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    static bool isInit = false;
    if (!isInit) {
        //注册库中所有可用的文件格式和解码器
#if (FFMPEG_VERSION_MAJOR < 4)
        av_register_all();
#endif
        //注册滤镜特效库相关(色调/模糊/水平翻转/裁剪/加方框/叠加文字等功能)
#if (FFMPEG_VERSION_MAJOR < 5)
        avfilter_register_all();
#endif
        //注册所有设备相关支持
#ifdef ffmpegdevice
        avdevice_register_all();
#endif
        //初始化网络流格相关(使用网络流时必须先执行)
        avformat_network_init();

        //设置日志级别
        //如果不想看到烦人的打印信息可以设置成 AV_LOG_QUIET 表示不打印日志
        //有时候发现使用不正常比如打开了没法播放视频则需要打开日志看下报错提示
        av_log_set_level(AV_LOG_QUIET);

        isInit = true;
        qDebug() << TIMEMS << QString("初始化库 -> 类型: %1 版本: %2").arg("ffmpeg").arg(getVersion());
        //debugInfo();
    }
}

void FFmpegHelper::showDevice()
{
#if defined(Q_OS_WIN)
    showDevice("dshow");
#elif defined(Q_OS_LINUX)
    showDevice("v4l2");
#elif defined(Q_OS_MAC)
    showDevice("avfoundation");
#endif
}

//ffmpeg -list_devices true -f dshow -i dummy
void FFmpegHelper::showDevice(const QString &type)
{
    AVFormatContext *ctx = avformat_alloc_context();
    AVInputFormatx *ifmt = av_find_input_format(type.toUtf8().data());

    AVDictionary *opts = NULL;
    av_dict_set(&opts, "list_devices", "true", 0);

    if (type == "vfwcap") {
        avformat_open_input(&ctx, "list", ifmt, NULL);
    } else if (type == "dshow") {
        avformat_open_input(&ctx, "video=dummy", ifmt, &opts);
    } else {
        avformat_open_input(&ctx, "", ifmt, &opts);
    }

    //释放资源
    av_dict_free(&opts);
    avformat_close_input(&ctx);
}

void FFmpegHelper::showOption(const QString &device)
{
#if defined(Q_OS_WIN)
    showOption("dshow", device);
#elif defined(Q_OS_LINUX)
    showOption("v4l2", device);
#elif defined(Q_OS_MAC)
    showOption("avfoundation", device);
#endif
}

void FFmpegHelper::showOption(const QString &type, const QString &device)
{
    AVFormatContext *ctx = avformat_alloc_context();
    AVInputFormatx *ifmt = av_find_input_format(type.toUtf8().data());

    AVDictionary *opts = NULL;
    av_dict_set(&opts, "list_options", "true", 0);

    QByteArray url = device.toUtf8();
    if (type == "dshow") {
        url = QString("video=%1").arg(device).toUtf8();
    }

    avformat_open_input(&ctx, url.data(), ifmt, &opts);

    //释放资源
    av_dict_free(&opts);
    avformat_close_input(&ctx);
}

QString FFmpegHelper::getFormatString(int format)
{
    return getFormatString(AVPixelFormat(format));
}

QString FFmpegHelper::getFormatString(AVPixelFormat format)
{
    //后面才发现内置了转换函数(不用傻哔哔的一个个写)
    if (format == AV_PIX_FMT_NONE) {
        return "none";
    } else {
        return av_pix_fmt_desc_get(format)->name;
    }

    QString text;
    if (format == AV_PIX_FMT_NONE) {
        text = "AV_PIX_FMT_NONE";
    } else if (format == AV_PIX_FMT_YUV420P) {
        text = "AV_PIX_FMT_YUV420P";
    } else if (format == AV_PIX_FMT_YUYV422) {
        text = "AV_PIX_FMT_YUYV422";
    } else if (format == AV_PIX_FMT_RGB24) {
        text = "AV_PIX_FMT_RGB24";
    } else if (format == AV_PIX_FMT_BGR24) {
        text = "AV_PIX_FMT_BGR24";
    } else if (format == AV_PIX_FMT_RGBA) {
        text = "AV_PIX_FMT_RGBA";
    } else if (format == AV_PIX_FMT_BGRA) {
        text = "AV_PIX_FMT_BGRA";
    } else if (format == AV_PIX_FMT_YUV422P) {
        text = "AV_PIX_FMT_YUV422P";
    } else if (format == AV_PIX_FMT_YUVJ420P) {
        text = "AV_PIX_FMT_YUVJ420P";
    } else if (format == AV_PIX_FMT_YUVJ422P) {
        text = "AV_PIX_FMT_YUVJ422P";
    } else if (format == AV_PIX_FMT_UYVY422) {
        text = "AV_PIX_FMT_UYVY422";
    } else if (format == AV_PIX_FMT_NV12) {
        text = "AV_PIX_FMT_NV12";
    } else if (format == AV_PIX_FMT_NV21) {
        text = "AV_PIX_FMT_NV21";
    } else if (format == AV_PIX_FMT_VAAPI) {
        text = "AV_PIX_FMT_VAAPI";
    } else if (format == AV_PIX_FMT_VDPAU) {
        text = "AV_PIX_FMT_VDPAU";
    } else if (format == AV_PIX_FMT_YVYU422) {
        text = "AV_PIX_FMT_YVYU422";
    } else if (format == AV_PIX_FMT_QSV) {
        text = "AV_PIX_FMT_QSV";
#if (FFMPEG_VERSION_MAJOR > 2)
    } else if (format == AV_PIX_FMT_CUDA) {
        text = "AV_PIX_FMT_CUDA";
    } else if (format == AV_PIX_FMT_D3D11) {
        text = "AV_PIX_FMT_D3D11";
#endif
    } else if (format == AV_PIX_FMT_YUV420P10LE) {
        text = "AV_PIX_FMT_YUV420P10LE";
    } else {
        text = QString::number(format);
    }

    return text;
}

qint64 FFmpegHelper::getBitRate(int width, int height)
{
    qint64 bitRate = 400;
    int size = width * height;
    if (size <= (640 * 360)) {
        bitRate = 400;
    } else if (size <= (960 * 540)) {
        bitRate = 900;
    } else if (size <= (1280 * 720)) {
        bitRate = 1500;
    } else if (size <= (1920 * 1080)) {
        bitRate = 3000;
    } else if (size <= (2560 * 1440)) {
        bitRate = 3500;
    } else if (size <= (3840 * 2160)) {
        bitRate = 6000;
    }

    return bitRate * 1000;
}

void FFmpegHelper::rotateFrame(int rotate, AVFrame *frameSrc, AVFrame *frameDst)
{
    int n = 0;
    int pos = 0;
    int w = frameSrc->width;
    int h = frameSrc->height;
    int hw = w / 2;
    int hh = h / 2;

    //根据不同的旋转角度拷贝yuv分量
    if (rotate == 90) {
        n = 0;
        int size = w * h;
        for (int j = 0; j < w; j++) {
            pos = size;
            for (int i = h - 1; i >= 0; i--) {
                pos -= w;
                frameDst->data[0][n++] = frameSrc->data[0][pos + j];
            }
        }

        n = 0;
        int hsize = size / 4;
        for (int j = 0; j < hw; j++) {
            pos = hsize;
            for (int i = hh - 1; i >= 0; i--) {
                pos -= hw;
                frameDst->data[1][n] = frameSrc->data[1][ pos + j];
                frameDst->data[2][n] = frameSrc->data[2][ pos + j];
                n++;
            }
        }
    } else if (rotate == 180) {
        n = 0;
        pos = w * h;
        for (int i = 0; i < h; i++) {
            pos -= w;
            for (int j = 0; j < w; j++) {
                frameDst->data[0][n++] = frameSrc->data[0][pos + j];
            }
        }

        n = 0;
        pos = w * h / 4;
        for (int i = 0; i < hh; i++) {
            pos -= hw;
            for (int j = 0; j < hw; j++) {
                frameDst->data[1][n] = frameSrc->data[1][ pos + j];
                frameDst->data[2][n] = frameSrc->data[2][ pos + j];
                n++;
            }
        }
    } else if (rotate == 270) {
        n = 0;
        for (int i = w - 1; i >= 0; i--) {
            pos = 0;
            for (int j = 0; j < h; j++) {
                frameDst->data[0][n++] = frameSrc->data[0][pos + i];
                pos += w;
            }
        }

        n = 0;
        for (int i = hw - 1; i >= 0; i--) {
            pos = 0;
            for (int j = 0; j < hh; j++) {
                frameDst->data[1][n] = frameSrc->data[1][pos + i];
                frameDst->data[2][n] = frameSrc->data[2][pos + i];
                pos += hw;
                n++;
            }
        }
    }

    //设置尺寸
    if (rotate == 90 || rotate == 270) {
        frameDst->linesize[0] = h;
        frameDst->linesize[1] = h / 2;
        frameDst->linesize[2] = h / 2;
        frameDst->width = h;
        frameDst->height = w;
    } else {
        frameDst->linesize[0] = w;
        frameDst->linesize[1] = w / 2;
        frameDst->linesize[2] = w / 2;
        frameDst->width = w;
        frameDst->height = h;
    }

    //设置其他参数
    frameDst->pts = frameSrc->pts;
    frameDst->pkt_dts = frameSrc->pkt_dts;
    frameDst->format = frameSrc->format;
    frameDst->key_frame = frameSrc->key_frame;
}

qint64 FFmpegHelper::getPts(AVPacket *packet)
{
    //有些文件(比如asf文件)取不到pts需要矫正
    qint64 pts = 0;
    if (packet->dts == AV_NOPTS_VALUE && packet->pts && packet->pts != AV_NOPTS_VALUE) {
        pts = packet->pts;
    } else if (packet->dts != AV_NOPTS_VALUE) {
        pts = packet->dts;
    }
    return pts;
}

double FFmpegHelper::getPtsTime(AVFormatContext *formatCtx, AVPacket *packet)
{
    AVStream *stream = formatCtx->streams[packet->stream_index];
    qint64 pts = getPts(packet);
    //qDebug() << TIMEMS << pts << packet->pos << packet->duration;
    //double time = pts * av_q2d(stream->time_base) * 1000;
    double time = pts * 1.0 * av_q2d(stream->time_base) * AV_TIME_BASE;
    //double time = pts * 1.0 * stream->time_base.num / stream->time_base.den * AV_TIME_BASE;
    return time;
}

double FFmpegHelper::getDurationTime(AVFormatContext *formatCtx, AVPacket *packet)
{
    AVStream *stream = formatCtx->streams[packet->stream_index];
    double time = packet->duration * av_q2d(stream->time_base);
    return time;
}

qint64 FFmpegHelper::getDelayTime(AVFormatContext *formatCtx, AVPacket *packet, qint64 startTime)
{
    AVRational time_base = formatCtx->streams[packet->stream_index]->time_base;
    AVRational time_base_q = {1, AV_TIME_BASE};//AV_TIME_BASE_Q
    qint64 pts = getPts(packet);
    qint64 pts_time = av_rescale_q(pts, time_base, time_base_q);
    qint64 now_time = av_gettime() - startTime;
    qint64 offset_time = pts_time - now_time;
    return offset_time;
}

void FFmpegHelper::delayTime(AVFormatContext *formatCtx, AVPacket *packet, qint64 startTime)
{
    qint64 offset_time = getDelayTime(formatCtx, packet, startTime);
    //qDebug() << TIMEMS << offset_time << packet->pts << packet->dts;
    if (offset_time > 0 && offset_time < 1 * 1000 * 1000) {
        av_usleep(offset_time);
    }
}

AVPixelFormat FFmpegHelper::find_fmt_by_hw_type(const AVHWDeviceType type)
{
    enum AVPixelFormat fmt;
    switch (type) {
        case AV_HWDEVICE_TYPE_VAAPI:
            fmt = AV_PIX_FMT_VAAPI;
            break;
        case AV_HWDEVICE_TYPE_DXVA2:
            fmt = AV_PIX_FMT_DXVA2_VLD;
            break;
#if (FFMPEG_VERSION_MAJOR > 2)
        case AV_HWDEVICE_TYPE_D3D11VA:
            fmt = AV_PIX_FMT_D3D11;
            break;
#endif
        case AV_HWDEVICE_TYPE_VDPAU:
            fmt = AV_PIX_FMT_VDPAU;
            break;
        case AV_HWDEVICE_TYPE_VIDEOTOOLBOX:
            fmt = AV_PIX_FMT_VIDEOTOOLBOX;
            break;
        default:
            fmt = AV_PIX_FMT_NONE;
            break;
    }
    return fmt;
}

AVPixelFormat FFmpegHelper::hw_pix_fmt = AV_PIX_FMT_NONE;
AVPixelFormat FFmpegHelper::get_hw_format(AVCodecContext *ctx, const AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;
    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt) {
            return *p;
        }
    }
    return AV_PIX_FMT_NONE;
}

int FFmpegHelper::decode(FFmpegThread *thread, AVCodecContext *avctx, AVPacket *packet, AVFrame *frameSrc, AVFrame *frameDst)
{
    int result = -1;
#ifdef videoffmpeg
    QString flag = "硬解出错";
#if (FFMPEG_VERSION_MAJOR > 2)
    result = avcodec_send_packet(avctx, packet);
    if (result < 0) {
        thread->debug(flag, QString("步骤: %1 原因: %2").arg("avcodec_send_packet").arg(getError(result)));
        return result;
    }

    while (result >= 0) {
        result = avcodec_receive_frame(avctx, frameSrc);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            break;
        } else if (result < 0) {
            thread->debug(flag, QString("步骤: %1 原因: %2").arg("avcodec_receive_frame").arg(getError(result)));
            break;
        }

        //将数据从GPU拷贝到CPU
        result = av_hwframe_transfer_data(frameDst, frameSrc, 0);
        if (result < 0) {
            av_frame_unref(frameDst);
            av_frame_unref(frameSrc);
            thread->debug(flag, QString("步骤: %1 原因: %2").arg("av_hwframe_transfer_data").arg(getError(result)));
            return result;
        }
        goto end;
    }
#endif
    return result;

end:
    //调用线程处理解码后的数据
    thread->decodeVideo2(packet);
#endif
    return result;
}

int FFmpegHelper::decode(FFmpegThread *thread, AVCodecContext *avctx, AVPacket *packet, AVFrame *frame, bool video)
{
    int result = -1;
#ifdef videoffmpeg
    QString flag = video ? "视频解码" : "音频解码";
#if (FFMPEG_VERSION_MAJOR < 3)
    if (video) {
        avcodec_decode_video2(avctx, frame, &result, packet);
        if (result < 0) {
            thread->debug(flag, QString("步骤: %1 原因: %2").arg("avcodec_decode_video2").arg(getError(result)));
            return result;
        }
    } else {
        avcodec_decode_audio4(avctx, frame, &result, packet);
        if (result < 0) {
            thread->debug(flag, QString("步骤: %1 原因: %2").arg("avcodec_decode_audio4").arg(getError(result)));
            return result;
        }
    }
    goto end;
#else
    result = avcodec_send_packet(avctx, packet);
    if (result < 0 && (result != AVERROR(EAGAIN)) && (result != AVERROR_EOF)) {
        //if (result < 0) {
        thread->debug(flag, QString("步骤: %1 原因: %2").arg("avcodec_send_packet").arg(getError(result)));
        return result;
    }

    while (result >= 0) {
        result = avcodec_receive_frame(avctx, frame);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            break;
        } else if (result < 0) {
            thread->debug(flag, QString("步骤: %1 原因: %2").arg("avcodec_receive_frame").arg(getError(result)));
            break;
        }
        goto end;
    }
#endif
    return result;

end:
    //调用线程处理解码后的数据
    if (video) {
        thread->decodeVideo2(packet);
    } else {
        thread->decodeAudio2(packet);
    }
#endif
    return result;
}

int FFmpegHelper::encode(FFmpegSave *thread, AVCodecContext *avctx, AVPacket *packet, AVFrame *frame, bool video)
{
    int result = -1;
#ifdef videosave
    QString flag = video ? "视频编码" : "音频编码";
#if (FFMPEG_VERSION_MAJOR < 3)
    if (video) {
        avcodec_encode_video2(avctx, packet, frame, &result);
        if (result < 0) {
            thread->debug(flag, QString("步骤: %1 原因: %2").arg("avcodec_encode_video2").arg(getError(result)));
            return result;
        }
    } else {
        avcodec_encode_audio2(avctx, packet, frame, &result);
        if (result < 0) {
            thread->debug(flag, QString("步骤: %1 原因: %2").arg("avcodec_encode_audio2").arg(getError(result)));
            return result;
        }
    }
    goto end;
#else
    result = avcodec_send_frame(avctx, frame);
    if (result < 0) {
        thread->debug(flag, QString("步骤: %1 原因: %2").arg("avcodec_send_frame").arg(getError(result)));
        return result;
    }

    while (result >= 0) {
        result = avcodec_receive_packet(avctx, packet);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            break;
        } else if (result < 0) {
            thread->debug(flag, QString("步骤: %1 原因: %2").arg("avcodec_receive_packet").arg(getError(result)));
            break;
        }
        goto end;
    }
#endif
    return result;

end:
    thread->writePacket(packet);
#endif
    return result;
}

void FFmpegHelper::getResolution(AVStream *stream, int &width, int &height)
{
#if (FFMPEG_VERSION_MAJOR < 3)
    width = stream->codec->width;
    height = stream->codec->height;
#else
    width = stream->codecpar->width;
    height = stream->codecpar->height;
#endif
}

AVCodecID FFmpegHelper::getCodecID(AVStream *stream)
{
#if (FFMPEG_VERSION_MAJOR < 3)
    return stream->codec->codec_id;
#else
    return stream->codecpar->codec_id;
#endif
}

int FFmpegHelper::copyContext(AVCodecContext *avctx, AVStream *stream, bool from)
{
    int result = -1;
#if (FFMPEG_VERSION_MAJOR < 3)
    if (from) {
        result = avcodec_copy_context(stream->codec, avctx);
    } else {
        result = avcodec_copy_context(avctx, stream->codec);
    }
#else
    if (from) {
        result = avcodec_parameters_from_context(stream->codecpar, avctx);
    } else {
        result = avcodec_parameters_to_context(avctx, stream->codecpar);
    }
#endif
    return result;
}

AVPacket *FFmpegHelper::creatPacket(AVPacket *packet)
{
    AVPacket *pkt;
#if (FFMPEG_VERSION_MAJOR < 3)
    //旧方法(废弃使用)
    if (packet) {
        pkt = new AVPacket;
        av_init_packet(pkt);
        av_copy_packet(pkt, packet);
    } else {
        pkt = new AVPacket;
        av_new_packet(pkt, sizeof(AVPacket));
    }
#else
    //新方法(推荐使用)
    if (packet) {
        pkt = av_packet_clone(packet);
    } else {
        pkt = av_packet_alloc();
    }
#endif
    return pkt;
}

void FFmpegHelper::freeFrame(AVFrame *frame)
{
    if (frame) {
        av_frame_free(&frame);
        av_free(frame->data);
    }
}

void FFmpegHelper::freePacket(AVPacket *packet)
{
    if (packet) {
        av_packet_unref(packet);
        //测试发现linux上再次释放指针会出错
#ifndef Q_OS_WIN
        return;
#endif
#if (FFMPEG_VERSION_MAJOR < 3)
        av_freep(packet);
#else
        av_packet_free(&packet);
#endif
    }
}

int FFmpegHelper::avinterruptCallBackFun(void *ctx)
{
#ifdef videoffmpeg
    FFmpegThread *thread = (FFmpegThread *)ctx;
    //2021-9-29 增加先判断是否尝试停止线程,有时候不存在的地址反复打开关闭会卡主导致崩溃
    //多了这个判断可以立即停止
    if (thread->getTryStop()) {
        thread->debug("主动停止", "");
        return 1;
    }

    //打开超时判定和读取超时判定
    if (thread->getTryOpen()) {
        //时间差值=当前时间-开始解码的时间(单位微秒)
        qint64 offset = av_gettime() - thread->getStartTime();
        int timeout = thread->getConnectTimeout() * 1000;
        //没有设定对应值的话限定最小值3秒
        timeout = (timeout <= 0 ? 3000000 : timeout);
        if (offset > timeout) {
            //thread->debug("打开超时", QString("设置: %1 当前: %2").arg(timeout).arg(offset));
            return 1;
        }
    } else if (thread->getTryRead()) {
        //时间差值=当前时间-最后一次读取的时间(单位毫秒)
        QDateTime now = QDateTime::currentDateTime();
        qint64 offset = thread->getLastTime().msecsTo(now);
        int timeout = thread->getReadTimeout();
        //没有设定对应值的话限定最小值3秒
        timeout = (timeout <= 0 ? 3000 : timeout);
        if (offset > timeout) {
            //thread->debug("读取超时", QString("设置: %1 当前: %2").arg(timeout).arg(offset));
            return 1;
        }
    }
#endif
    return 0;
}
