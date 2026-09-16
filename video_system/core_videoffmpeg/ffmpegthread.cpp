#include "ffmpegthread.h"
#include "ffmpeghelper.h"
#include "ffmpegfilter.h"
#include "ffmpegsync.h"
#include "ffmpegsave.h"
#include "videohelper.h"

#ifdef audiox
#include "../core_audio/audioplayer.h"
#else
#include "../core_videobase/audioplayer.h"
#endif

FFmpegThread::FFmpegThread(QObject *parent) : VideoThread(parent)
{
    speed = 1.0;
    videoIndex = -1;
    audioIndex = -1;
    changePosition = false;

    realPacketSize = 0;
    realPacketCount = 0;

    videoFirstPts = 0;
    audioFirstPts = 0;
    lastVideoFrameTime = 0;

    packet = NULL;
    videoFrame = NULL;
    audioFrame = NULL;

    yuvFrame = NULL;
    imageFrame = NULL;
    hardFrame = NULL;

    yuvData = NULL;
    imageData = NULL;
    pcmData = NULL;

    yuvSwsCtx = NULL;
    imageSwsCtx = NULL;
    pcmSwrCtx = NULL;

    options = NULL;
    formatCtx = NULL;

    videoCodec = NULL;
    audioCodec = NULL;

    videoCodecCtx = NULL;
    audioCodecCtx = NULL;

    saveVideoType = SaveVideoType_Mp4;
    saveAudioType = SaveAudioType_Aac;
    FFmpegHelper::initLib();

    //初始化音频解码同步线程(本线程释放的时候也会自动释放)
    useSync = true;
    audioSync = new FFmpegSync(0, this);
    connect(audioSync, SIGNAL(receivePosition(qint64)), this, SIGNAL(receivePosition(qint64)));

    //初始化视频解码同步线程(本线程释放的时候也会自动释放)
    videoSync = new FFmpegSync(1, this);
    connect(videoSync, SIGNAL(receivePosition(qint64)), this, SIGNAL(receivePosition(qint64)));

    //实例化恢复暂停定时器(单次定时器)
    timerPause = new QTimer(this);
    connect(timerPause, SIGNAL(timeout()), this, SLOT(pause()));
    timerPause->setInterval(100);
    timerPause->setSingleShot(true);

    //实例化切换进度定时器(单次定时器)
    timerPosition = new QTimer(this);
    connect(timerPosition, SIGNAL(timeout()), this, SLOT(setPosition()));
    timerPosition->setInterval(200);
    timerPosition->setSingleShot(true);

    //实例化音频播放类
    audioPlayer = new AudioPlayer;
    connect(audioPlayer, SIGNAL(receiveVolume(int)), this, SIGNAL(receiveVolume(int)));
    connect(audioPlayer, SIGNAL(receiveMuted(bool)), this, SIGNAL(receiveMuted(bool)));
    connect(audioPlayer, SIGNAL(receiveLevel(qreal, qreal)), this, SIGNAL(receiveLevel(qreal, qreal)));
}

FFmpegThread::~FFmpegThread()
{
    //释放音频播放类
    audioPlayer->deleteLater();
}

void FFmpegThread::run()
{
    while (!stopped) {
        if (!isOk) {
            //记住开始解码的时间用于用音视频同步等
            startTime = av_gettime();
            this->closeVideo();
            this->openVideo();
            this->checkOpen();
            continue;
        }

        //暂停及切换进度期间继续更新时间防止超时
        if (isPause || changePosition) {
            lastTime = QDateTime::currentDateTime();
            //视频流要继续读取不然会一直累积
            if (!getIsFile()) {
                av_read_frame(formatCtx, packet);
                av_packet_unref(packet);
            }
            msleep(5);
            continue;
        }

        //解码队列中帧数过多暂停读取（降低阈值以减少延迟）
        if (videoSync->getPacketCount() >= 50 || audioSync->getPacketCount() >= 50) {
            QThread::usleep(100);
            continue;
        }

        //读取一帧(通过标志位控制回调那边做超时判断)
        tryRead = true;
        int result = av_read_frame(formatCtx, packet);
        tryRead = false;
        if (result >= 0) {
            //错误计数清零以及更新最后的解码时间
            errorCount = 0;
            lastTime = QDateTime::currentDateTime();

            //判断当前包是视频还是音频
            int index = packet->stream_index;
            if (index == videoIndex) {
                //checkRealPacketSize(packet, frameRate);
                decodeVideo0(packet);
                //QString msg = QString("time: %1 pts: %2 dts: %3").arg(qint64(FFmpegHelper::getPtsTime(formatCtx, packet) / 1000)).arg(packet->pts).arg(packet->dts);
                //debug("视频数据", msg);
            } else if (index == audioIndex) {
                decodeAudio0(packet);
                //QString msg = QString("time: %1 pts: %2 dts: %3").arg(qint64(FFmpegHelper::getPtsTime(formatCtx, packet) / 1000)).arg(packet->pts).arg(packet->dts);
                //debug("音频数据", msg);
            }
        } else if (getIsFile()) {
            //如果是视频文件则判断是否文件结束
            errorCount++;
            if (result == AVERROR_EOF || result == AVERROR_EXIT) {
                //当同步队列中的数量为0(表示解码处理完成)才需要跳出
                if (videoSync->getPacketCount() == 0 && audioSync->getPacketCount() == 0) {
                    if (playRepeat) {
                        this->replay();
                    } else {
                        this->stop2();
                        msleep(10);
                        break;
                    }
                }
            }
        } else {
            //下面这种情况在摄像机掉线后出现(一般3秒钟来一次)
            errorCount++;
            debug("错误计数", QString("数量: %1 原因: %2").arg(errorCount).arg(FFmpegHelper::getError(result)));
            if (errorCount >= 3) {
                break;
            }
        }

        av_packet_unref(packet);
        //不要固定延时，让线程尽快处理下一帧
        //队列满时会自动等待，不需要在这里延时
    }

    //非文件需要调用所在窗体关闭(文件已经在上面处理了)
    if (!getIsFile()) {
        this->stop2();
        msleep(10);
    }

    //标志位判断是否需要重连
    bool needRestart = (errorCount > 1 && readTimeout > 0);
    //关闭视频
    this->closeVideo();

    if (needRestart) {
        debug("超时重连", "");
        QMetaObject::invokeMethod(this, "start");
        return;
    }

    debug("线程结束", "");
}

void FFmpegThread::replay()
{
    //加个时间限制防止频繁执行
    QDateTime now = QDateTime::currentDateTime();
    if (qAbs(now.msecsTo(lastTime)) < 300) {
        return;
    }

    lastTime = now;
    videoSync->reset();
    audioSync->reset();
    this->position = 0;
    this->setPosition();
    debug("循环播放", "");
}

void FFmpegThread::checkRealPacketSize(AVPacket *packet, int maxCount)
{
    realPacketCount++;
    realPacketSize += packet->size;
    if (realPacketCount == maxCount) {
        QString mb = QString::number((qreal)realPacketSize / 100000, 'f', 2);
        debug("实时码率", QString("大小: %1 mb").arg(mb));
        realPacketSize = 0;
        realPacketCount = 0;
    }
}

bool FFmpegThread::scaleAndSaveVideo(bool &needScale, AVPacket *packet)
{
    //有些视频流会动态改变分辨率需要重新打开
    if (videoType == VideoType_Rtsp && hardware == "none") {
        int width = videoFrame->width;
        int height = videoFrame->height;
        //必须要从帧这里获取分辨率/流那边获取到的还是旧的
        //FFmpegHelper::getResolution(formatCtx->streams[videoIndex], width, height);
        if (videoWidth != width || videoHeight != height) {
            debug("尺寸变化", QString("变化: %1x%2 -> %3x%4").arg(videoWidth).arg(videoHeight).arg(width).arg(height));
            isOk = false;
            return false;
        }
    }

    //不是yuv420则先要转换(本地摄像头格式yuyv422/还有些视频文件是各种各样的格式)
    //很多摄像头视频流是yuvj420也不需要转换可以直接用
    AVPixelFormat format = videoCodecCtx->pix_fmt;
    needScale = (format != AV_PIX_FMT_YUV420P && format != AV_PIX_FMT_YUVJ420P);

    //硬解码模式下格式是NV12所以needScale永远为真
    //硬解码模式下没有在录像阶段不需要转换
    if (hardware != "none" && !isRecord) {
        needScale = false;
    }

    //截图和绘制模式下没有在录像阶段不需要转换
    if ((isSnap || videoMode == VideoMode_Painter) && !isRecord) {
        needScale = false;
    }

    //rtsp视频流本身收到的就是h264裸流永远不用转换
    if (videoType == VideoType_Rtsp) {
        needScale = false;
    }

    //将非yuv420格式转换到yuv420格式
    if (needScale) {
        int result = sws_scale(yuvSwsCtx, (const quint8 * const *)videoFrame->data, videoFrame->linesize, 0, videoHeight, yuvFrame->data, yuvFrame->linesize);
        if (result < 0) {
            debug("转换失败", QString("格式: %1 原因: %2").arg(format).arg(FFmpegHelper::getError(result)));
            return false;
        }
    }

#ifdef videosave
    if (saveVideoType > 1) {
        if (videoType == VideoType_Rtsp) {
            //直接写入原数据包可以节省CPU(部分厂家比如安迅士的需要重新编码)
            if (videoUrl.contains("/axis-media/")) {
                saveFile->writeVideo(videoFrame);
            } else {
                saveFile->writeVideo(packet);
            }
        } else if (needScale) {
            saveFile->writeVideo(yuvFrame);
        } else {
            saveFile->writeVideo(videoFrame);
        }
    }
#endif

    return true;
}

void FFmpegThread::decodeVideo0(AVPacket *packet)
{
    // 有些监控视频保存的MP4文件首帧开始的时间不是0所以需要减去
    if (videoFirstPts > AV_TIME_BASE) {
        packet->pts -= videoFirstPts;
        packet->dts = packet->pts;
    }

    if (useSync) {
        // 加入到队列交给解码同步线程处理
        videoSync->append(FFmpegHelper::creatPacket(packet));
    } else {
        // 直接当前线程解码
        decodeVideo1(packet);
        if (decodeType != DecodeType_Fastest) {
            if (!getIsFile() && frameRate > 0) {
                // RTSP流：使用PTS时间延迟并自动校准
                qint64 ptsTime = FFmpegHelper::getPtsTime(formatCtx, packet); // 微秒
                // 如果是第一帧，记录基准
                if (!firstVideoPtsSet) {
                    videoPtsBase = ptsTime;
                    videoTimeBase = av_gettime();
                    firstVideoPtsSet = true;
                }
                // 计算理想显示时间
                qint64 idealTime = videoTimeBase + (ptsTime - videoPtsBase);
                qint64 now = av_gettime();
                qint64 sleepTime = idealTime - now;

                // 自动校准：若睡眠时间过长（>1秒），通常是因为PTS跳变或长时间卡顿，重新对齐
                if (sleepTime > 1000000) {
                    // 将基准时间调整为当前系统时间，使当前帧立即显示，后续帧基于新基准
                    videoTimeBase = now - (ptsTime - videoPtsBase);
                    sleepTime = 0;
                }
                // 若落后太多（sleepTime < -500ms），可选择性校准，但一般无需额外操作
                // 因为此时sleepTime为负，不会等待，会自然追赶；但为避免追赶过快，可限制最大加速度
                if (sleepTime > 0) {
                    av_usleep(sleepTime);
                }
                // 若sleepTime <= 0，直接显示，不等待
            } else {
                // 文件播放：保留原有PTS绝对时间法
                FFmpegHelper::delayTime(formatCtx, packet, startTime);
            }
        }
    }
}

void FFmpegThread::decodeVideo1(AVPacket *packet)
{
    if (hardware == "none") {
        FFmpegHelper::decode(this, videoCodecCtx, packet, videoFrame, true);
    } else {
        FFmpegHelper::decode(this, videoCodecCtx, packet, hardFrame, videoFrame);
    }
}

void FFmpegThread::decodeVideo2(AVPacket *packet)
{
    //如果需要重新初始化则先初始化滤镜
    if (!filterData.init) {
        this->initFilter();
    }

    //先处理滤镜(这里就直接在源数据上处理/也可以是不同的帧数据)
    if (filterData.isOk) {
        if (av_buffersrc_add_frame(filterData.filterSrcCtx, videoFrame) >= 0) {
            av_buffersink_get_frame(filterData.filterSinkCtx, videoFrame);
        }
    }

    bool needScale = false;
    if (!scaleAndSaveVideo(needScale, packet)) {
        return;
    }

    //截图和绘制都转成图片
    if (isSnap || videoMode == VideoMode_Painter) {
        //启动计时
        timer.restart();
        //将数据转成图片
        int result = sws_scale(imageSwsCtx, (const quint8 * const *)videoFrame->data, videoFrame->linesize, 0, videoHeight, imageFrame->data, imageFrame->linesize);
        if (result < 0) {
            return;
        }

        QImage image((quint8 *)imageData, videoWidth, videoHeight, QImage::Format_RGB888);
        if (image.isNull()) {
            return;
        }

        //如果有旋转角度先要旋转
        if (rotate > 0) {
            QTransform matrix;
            matrix.rotate(rotate);
            image = image.transformed(matrix, Qt::SmoothTransformation);
        }

        if (isSnap) {
            isSnap = false;
            image.save(this->snapName, "jpg");
            QMetaObject::invokeMethod(this, "snapFinsh");
        } else {
            emit receiveImage(image, timer.elapsed());
        }
    } else {
        //qDebug() << TIMEMS << videoWidth << videoHeight << videoFrame->width << videoFrame->height << videoFrame->linesize[0] << videoFrame->linesize[1] << videoFrame->linesize[2];
        if (hardware == "none") {
            if (needScale) {
                emit receiveFrame(videoWidth, videoHeight, yuvFrame->data[0], yuvFrame->data[1], yuvFrame->data[2], videoWidth, videoWidth / 2, videoWidth / 2);
                this->writeVideoData(videoWidth, videoHeight, yuvFrame->data[0], yuvFrame->data[1], yuvFrame->data[2]);
            } else {
                emit receiveFrame(videoFrame->width, videoFrame->height, videoFrame->data[0], videoFrame->data[1], videoFrame->data[2], videoFrame->linesize[0], videoFrame->linesize[1], videoFrame->linesize[2]);
                this->writeVideoData(videoFrame->linesize[0], videoFrame->height, videoFrame->data[0], videoFrame->data[1], videoFrame->data[2]);
            }
        } else {
            emit receiveFrame(videoWidth, videoHeight, videoFrame->data[0], videoFrame->data[1], videoFrame->linesize[0], videoFrame->linesize[1]);
        }
    }
}

void FFmpegThread::decodeAudio0(AVPacket *packet)
{
    //如果没有开启则不用继续
    if (!decodeAudio) {
        return;
    }

    if (audioFirstPts > AV_TIME_BASE) {
        packet->pts -= audioFirstPts;
        packet->dts = packet->pts;
    }

    if (useSync) {
        //加入到队列交给解码同步线程处理
        audioSync->append(FFmpegHelper::creatPacket(packet));
    } else {
        //直接当前线程解码
        decodeAudio1(packet);
        if (decodeType != DecodeType_Fastest) {
            FFmpegHelper::delayTime(formatCtx, packet, startTime);
        }
    }
}

void FFmpegThread::decodeAudio1(AVPacket *packet)
{
    FFmpegHelper::decode(this, audioCodecCtx, packet, audioFrame, false);
}

void FFmpegThread::decodeAudio2(AVPacket *packet)
{
    int channel = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    int len = av_samples_get_buffer_size(NULL, channel, audioFrame->nb_samples, AV_SAMPLE_FMT_S16, 1);
    int result = swr_convert(pcmSwrCtx, &pcmData, len, (const quint8 **)audioFrame->data, audioFrame->nb_samples);
    if (result >= 0) {
        //播放声音数据
        char *data = (char *)pcmData;
        if (playAudio && !timerPause->isActive()) {
            //audioPlayer->playAudioData(data, len);
            QMetaObject::invokeMethod(audioPlayer, "playAudioData", Q_ARG(const char *, data), Q_ARG(qint64, len));
        }

        //保存音频文件
        if (saveAudioType == SaveAudioType_Pcm || saveAudioType == SaveAudioType_Wav) {
            this->writeAudioData(data, len);
        } else if (saveAudioType == SaveAudioType_Aac) {
            this->writeAudioData((char *)packet->data, packet->size);
        }
    }
}

void FFmpegThread::initOption()
{
    //在打开码流前指定各种参数比如: 探测时间/超时时间/最大延时等
    //设置缓存大小,1080p可将值调大,现在很多摄像机是2k可能需要调大,一般2k是1080p的四倍
    av_dict_set(&options, "buffer_size", "8192000", 0);
    //通信协议采用tcp还是udp,通过参数传入,不设置默认是udp
    //udp优点是无连接,在网线拔掉以后十几秒钟重新插上还能继续接收,缺点是网络不好的情况下会丢包花屏
    QByteArray data = transport.toUtf8();
    const char *rtsp_transport = data.constData();
    av_dict_set(&options, "rtsp_transport", rtsp_transport, 0);
    //设置超时断开连接时间,单位微秒,3000000表示3秒
    av_dict_set(&options, "stimeout", "3000000", 0);
    //设置最大时延,单位微秒,1000000表示1秒
    av_dict_set(&options, "max_delay", "1000000", 0);
    //自动开启线程数
    av_dict_set(&options, "threads", "auto", 0);
    //开启无缓存,rtmp等视频流不建议开启
    //av_dict_set(&options, "fflags", "nobuffer", 0);

    //增加rtp/sdp支持,后面发现不要加
    if (videoUrl.endsWith(".sdp")) {
        //av_dict_set(&options, "protocol_whitelist", "file,rtp,udp", 0);
    }

    //本地摄像头设备单独设置参数
    if (videoType == VideoType_Camera) {
        //设置分辨率
        if (bufferSize != "0x0") {
            av_dict_set(&options, "video_size", bufferSize.toUtf8().constData(), 0);
        }
        //设置帧率
        if (frameRate > 0) {
            av_dict_set(&options, "framerate", QString::number(frameRate).toUtf8().constData(), 0);
        }

        //设置输入格式(具体要看设置是否支持)
        //后面改成了统一转成标准yuv420所以不用设置也没有任何影响
        //av_dict_set(&options, "input_format", "mjpeg", 0);
    }
}

bool FFmpegThread::initInput()
{
    //实例化格式处理上下文
    formatCtx = avformat_alloc_context();
    //设置超时回调(有些不存在的地址或者网络不好的情况下要卡很久)
    formatCtx->interrupt_callback.callback = FFmpegHelper::avinterruptCallBackFun;
    formatCtx->interrupt_callback.opaque = this;

    //先判断是否是本地摄像头
    AVInputFormatx *ifmt = NULL;
    if (videoType == VideoType_Camera) {
#if defined(Q_OS_WIN)
        //ifmt = av_find_input_format("vfwcap");
        ifmt = av_find_input_format("dshow");
#elif defined(Q_OS_LINUX)
        //可以打开cheese程序查看本地摄像头(如果是在虚拟机中需要设置usb选项3.1)
        //ifmt = av_find_input_format("v4l2");
        ifmt = av_find_input_format("video4linux2");
#elif defined(Q_OS_MAC)
        ifmt = av_find_input_format("avfoundation");
#endif
    }

    //打开输入(通过标志位控制回调那边做超时判断)
    //其他地方调用 formatCtx->url formatCtx->filename 可以拿到设置的地址(两个变量值一样)
    tryOpen = true;
    QByteArray urlData = VideoHelper::getRightUrl(videoType, videoUrl).toUtf8();
    int result = avformat_open_input(&formatCtx, urlData.data(), ifmt, &options);
    tryOpen = false;
    if (result < 0) {
        debug("打开出错", "错误: " + FFmpegHelper::getError(result));
        return false;
    }

    //根据自己项目需要开启下面部分代码加快视频流打开速度
    //开启后由于值太小可能会出现部分视频流获取不到分辨率
    if (decodeType == DecodeType_Fastest && videoType == VideoType_Rtsp) {
        //接口内部读取的最大数据量(从源文件中读取的最大字节数)
        //默认值5000000导致这里卡很久最耗时(可以调小来加快打开速度)
        formatCtx->probesize = 50000;
        //从文件中读取的最大时长(单位为 AV_TIME_BASE units)
        formatCtx->max_analyze_duration = 5 * AV_TIME_BASE;
        //内部读取的数据包不放入缓冲区
        //formatCtx->flags |= AVFMT_FLAG_NOBUFFER;
        //设置解码错误验证过滤花屏
        //formatCtx->error_recognition |= AV_EF_EXPLODE;
    }

    //获取流信息
    result = avformat_find_stream_info(formatCtx, NULL);
    if (result < 0) {
        debug("找流失败", "错误: " + FFmpegHelper::getError(result));
        return false;
    }

    //解码格式
    formatName = formatCtx->iformat->name;
    //某些格式比如视频流不做音视频同步(响应速度快)
    if (formatName == "rtsp" || videoUrl.endsWith(".sdp")) {
        useSync = false;
    }

    //设置了最快速度则不启用音视频同步
    if (decodeType == DecodeType_Fastest) {
        useSync = false;
    }

    //有些格式不支持硬解码
    if (formatName.contains("rm") || formatName.contains("avi") || formatName.contains("webm")) {
        hardware = "none";
    }

    //本地摄像头设备解码出来的直接就是yuv显示不需要硬解码
    if (videoType == VideoType_Camera) {
        useSync = false;
        hardware = "none";
    }

    //过低版本不支持硬解码
#if (FFMPEG_VERSION_MAJOR < 3)
    hardware = "none";
#endif

    //发送文件时长信号(这里获取到的是秒)
    duration = formatCtx->duration / AV_TIME_BASE;
    duration = duration * 1000;
    if (getIsFile()) {
        //文件必须要音视频同步
        useSync = true;
        emit receiveDuration(duration);
    }

    QString msg = QString("格式: %1 时长: %2 秒 加速: %3").arg(formatName).arg(duration / 1000).arg(hardware);
    debug("文件信息", msg);
    return true;
}

bool FFmpegThread::initVideo()
{
    //找到视频流索引
    videoIndex = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0) {
        //有些没有视频流所以这里不用返回
        debug("无视频流", "");
    } else {
        //先获取旋转角度(如果有旋转角度则不能用硬件加速)
        this->getRotate();
        if (rotate != 0) {
            hardware = "none";
        }

        //获取视频流
        int result = -1;
        AVStream *videoStream = formatCtx->streams[videoIndex];

        //查找视频解码器(如果上面av_find_best_stream第五个参数传了则这里不需要)
        AVCodecID codecID = FFmpegHelper::getCodecID(videoStream);
        videoCodec = avcodec_find_decoder(codecID);
        //videoCodec = avcodec_find_decoder_by_name("h264");
        if (!videoCodec) {
            debug("无解码器", "错误: 查找视频解码器失败");
            return false;
        }

        //创建视频解码器上下文
        videoCodecCtx = avcodec_alloc_context3(NULL);
        if (!videoCodecCtx) {
            debug("无解码器", "错误: 创建视频解码器上下文失败");
            return false;
        }

        result = FFmpegHelper::copyContext(videoCodecCtx, videoStream, false);
        if (result < 0) {
            debug("无解码器", "错误: 设置视频解码器参数失败 " + FFmpegHelper::getError(result));
            return false;
        }

        //初始化硬件加速(硬解码)
        if (!initHardware()) {
            return false;
        }

        //设置加速解码(设置 lowres = max_lowres 的话很可能画面采用最小的分辨率)
        //videoCodecCtx->lowres = videoCodec->max_lowres;
        videoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        videoCodecCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;
        videoCodecCtx->flags2 |= AV_CODEC_FLAG2_FAST;

        //打开视频解码器
        result = avcodec_open2(videoCodecCtx, videoCodec, NULL);
        if (result < 0) {
            debug("打开解码", "错误: 打开视频解码器失败 " + FFmpegHelper::getError(result));
            return false;
        }

        //获取分辨率大小
        FFmpegHelper::getResolution(videoStream, videoWidth, videoHeight);
        //如果没有获取到宽高则返回
        if (videoWidth == 0 || videoHeight == 0) {
            debug("无分辨率", "");
            return false;
        }

        //记录首帧开始时间
        videoFirstPts = videoStream->start_time;
        //帧率优先取平均帧率
        double fps = av_q2d(videoStream->avg_frame_rate);
        if (fps <= 0 || fps > 30 || qIsNaN(fps)) {
            fps = av_q2d(videoStream->r_frame_rate);
        }

        frameRate = fps;
        QString msg = QString("索引: %1 解码: %2 帧率: %3 宽高: %4x%5 角度: %6").arg(videoIndex).arg(videoCodec->name).arg(fps).arg(videoWidth).arg(videoHeight).arg(rotate);
        debug("视频信息", msg);
    }

    return true;
}

bool FFmpegThread::initHardware()
{
    if (hardware == "none") {
        return true;
    }
#if (FFMPEG_VERSION_MAJOR > 2)
    //根据名称自动寻找硬解码
    enum AVHWDeviceType type;
    //发现嵌入式上低版本的库没有av_hwdevice_find_type_by_name函数
#ifdef __arm__
#if (FFMPEG_VERSION_MAJOR < 4)
    return false;
#else
    type = av_hwdevice_find_type_by_name(hardware.toUtf8().data());
#endif
#else
    type = av_hwdevice_find_type_by_name(hardware.toUtf8().data());
#endif
    debug("硬件加速", QString("名称: %1 数值: %2").arg(hardware).arg(type));

    //找到对应的硬解码格式
    FFmpegHelper::hw_pix_fmt = FFmpegHelper::find_fmt_by_hw_type(type);
    if (FFmpegHelper::hw_pix_fmt == -1) {
        debug("加速失败", "错误: 未找到对应加速类型");
        return false;
    }

    int result = -1;
    //解码器格式赋值为硬解码
    videoCodecCtx->get_format = FFmpegHelper::get_hw_format;
    //av_opt_set_int(videoCodecCtx, "refcounted_frames", 1, 0);

    //创建硬解码设备
    AVBufferRef *hw_device_ref;
    result = av_hwdevice_ctx_create(&hw_device_ref, type, NULL, NULL, 0);
    if (result < 0) {
        debug("加速失败", "错误: 创建视频解码器失败 " + FFmpegHelper::getError(result));
        return false;
    }

    videoCodecCtx->hw_device_ctx = av_buffer_ref(hw_device_ref);
    av_buffer_unref(&hw_device_ref);
    return true;
#else
    return false;
#endif
}

bool FFmpegThread::initAudio()
{
    //找到音频流索引
    audioIndex = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audioIndex < 0) {
        //有些没有音频流所以这里不用返回
        debug("无音频流", "");
    } else {
        //获取音频流
        int result = -1;
        AVStream *audioStream = formatCtx->streams[audioIndex];

        //查找音频解码器(如果上面av_find_best_stream第五个参数传了则这里不需要)
        AVCodecID codecID = FFmpegHelper::getCodecID(audioStream);
        audioCodec = avcodec_find_decoder(codecID);
        //audioCodec = avcodec_find_decoder_by_name("aac");
        if (!audioCodec) {
            debug("无解码器", "错误: 查找音频解码器失败");
            return false;
        }

        //创建音频解码器上下文
        audioCodecCtx = avcodec_alloc_context3(audioCodec);
        if (!audioCodecCtx) {
            debug("无解码器", "错误: 创建音频解码器上下文失败");
            return false;
        }

        result = FFmpegHelper::copyContext(audioCodecCtx, audioStream, false);
        if (result < 0) {
            debug("无解码器", "错误: 设置音频解码器参数失败 " + FFmpegHelper::getError(result));
            return false;
        }

        //打开音频解码器
        result = avcodec_open2(audioCodecCtx, audioCodec, NULL);
        if (result < 0) {
            debug("打开解码", "错误: 打开音频解码器失败 " + FFmpegHelper::getError(result));
            return false;
        }

        //记录首帧开始时间
        audioFirstPts = audioStream->start_time;

        //获取音频参数
        profile = audioCodecCtx->profile;
        sampleRate = audioCodecCtx->sample_rate;
        channelCount = 2;//audioCodecCtx->channels;//发现有些地址居然有6个声道
        int sampleSize = 2;//av_get_bytes_per_sample(audioCodecCtx->sample_fmt) / 2;
        QString codecName = audioCodec->name;//long_name

        //如果解码不是aac且设置了保存为aac则强制改成保存为wav
        //有些文件就算音频解码是aac然后用aac保存输出可能也有错(只需要改成wav格式100%正确)
        if (codecName != "aac" && saveAudioType == SaveAudioType_Aac) {
            saveAudioType = SaveAudioType_Wav;
        }

        //音频采样转换
        qint64 channelOut = AV_CH_LAYOUT_STEREO;
        qint64 channelIn = av_get_default_channel_layout(audioCodecCtx->channels);
        pcmSwrCtx = swr_alloc();
        pcmSwrCtx = swr_alloc_set_opts(NULL, channelOut, AV_SAMPLE_FMT_S16, sampleRate, channelIn, audioCodecCtx->sample_fmt, sampleRate, 0, 0);

        //分配音频数据内存 192000 这个值是看ffplay代码中的
        if (swr_init(pcmSwrCtx) >= 0) {
            quint64 byte = (192000 * 3) / 2;
            pcmData = (quint8 *)av_malloc(byte * sizeof(quint8));
            if (!pcmData) {
                av_free(pcmData);
                return false;
            }
        } else {
            return false;
        }

        QString msg = QString("索引: %1 解码: %2 比特: %3 声道: %4 采样: %5 品质: %6").arg(audioIndex).arg(codecName).arg(formatCtx->bit_rate).arg(channelCount).arg(sampleRate).arg(profile);
        debug("音频信息", msg);
    }

    return true;
}

void FFmpegThread::initOther()
{
    //分配数据包内存
    packet = FFmpegHelper::creatPacket(NULL);
    //音频流索引存在则分配音频帧内存
    if (audioIndex >= 0) {
        audioFrame = av_frame_alloc();
    }

    //视频流索引存在才需要分配视频帧内存以及其他相关设置
    if (videoIndex >= 0) {
        videoFrame = av_frame_alloc();
        yuvFrame = av_frame_alloc();
        imageFrame = av_frame_alloc();
        hardFrame = av_frame_alloc();

        //设置属性以便该帧对象正常
        yuvFrame->format = AV_PIX_FMT_YUV420P;
        yuvFrame->width = videoWidth;
        yuvFrame->height = videoHeight;

        //定义及获取像素格式
        AVPixelFormat srcFormat = AV_PIX_FMT_YUV420P;
        //重新设置源图片格式
        if (hardware == "none") {
            //通过解码器获取解码格式
            srcFormat = videoCodecCtx->pix_fmt;
        } else {
            //硬件加速需要指定格式
            srcFormat = AV_PIX_FMT_NV12;
        }

        //默认最快速度的解码采用的SWS_FAST_BILINEAR参数,可能会丢失部分图片数据,可以自行更改成其他参数
        int flags = SWS_FAST_BILINEAR;
        if (decodeType == DecodeType_Fast) {
            flags = SWS_FAST_BILINEAR;
        } else if (decodeType == DecodeType_Full) {
            flags = SWS_BICUBIC;
        } else if (decodeType == DecodeType_Even) {
            flags = SWS_BILINEAR;
        }

        //硬解码模式下需要对齐(32 64 128等)
        int align = 1;
        if (hardware != "none") {
            align = 64;
        }

        //分配视频帧数据(转yuv420)
        int yuvSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, videoWidth, videoHeight, align);
        yuvData = (quint8 *)av_malloc(yuvSize * sizeof(quint8));
        av_image_fill_arrays(yuvFrame->data, yuvFrame->linesize, yuvData, AV_PIX_FMT_YUV420P, videoWidth, videoHeight, align);
        //视频图像转换(转yuv420)
        yuvSwsCtx = sws_getContext(videoWidth, videoHeight, srcFormat, videoWidth, videoHeight, AV_PIX_FMT_YUV420P, flags, NULL, NULL, NULL);

        //分配视频帧数据(转rgb)
        int imageSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, videoWidth, videoHeight, 1);
        imageData = (quint8 *)av_malloc(imageSize * sizeof(quint8));
        av_image_fill_arrays(imageFrame->data, imageFrame->linesize, imageData, AV_PIX_FMT_RGB24, videoWidth, videoHeight, 1);
        //视频图像转换(转rgb)
        imageSwsCtx = sws_getContext(videoWidth, videoHeight, srcFormat, videoWidth, videoHeight, AV_PIX_FMT_RGB24, flags, NULL, NULL, NULL);

        QString srcFormatString = FFmpegHelper::getFormatString(srcFormat);
        QString dstFormatString = FFmpegHelper::getFormatString(videoMode == VideoMode_Painter ? AV_PIX_FMT_RGB24 : AV_PIX_FMT_YUV420P);
        QString msg = QString("源头: %1 目标: %2").arg(srcFormatString).arg(dstFormatString);
        debug("格式信息", msg);
    }

    //还有一种情况获取到了专辑封面也在这里
    if (onlyAudio) {
        videoIndex = -1;
        this->initMetadata();
    }

    //测试发现部分厂家摄像机(比如海康)不支持48000采样率解码音频
    if (videoType == VideoType_Rtsp && sampleRate >= 48000) {
        decodeAudio = false;
        audioIndex = -1;
    }

    //检查保存参数
    this->checkSave();
    //初始化音频播放
    this->initAudioPlayer();
    //初始化滤镜
    this->initFilter();
}

void FFmpegThread::initMetadata()
{
    //读取音频文件信息
    AVDictionaryEntry *tag = NULL;
    while ((tag = av_dict_get(formatCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        debug("专辑信息", QString("名称: %1 键值: %2").arg(tag->key).arg(tag->value));
    }

    //读取封面
    emit receivePlayStart(timer.elapsed());
    if (formatCtx->iformat->read_header(formatCtx) >= 0) {
        for (int i = 0; i < formatCtx->nb_streams; ++i) {
            if (formatCtx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                AVPacket pkt = formatCtx->streams[i]->attached_pic;
                QImage image = QImage::fromData((quint8 *)pkt.data, pkt.size);
                emit receiveImage(image, 0);
                break;
            }
        }
    }
}

void FFmpegThread::checkSave()
{
    //没有找到视频则不用录制视频
    if (videoIndex < 0) {
        saveVideoType = SaveVideoType_None;
    }

    //大分辨率在硬解码状态下需要64位的版本才能保存(可能提示 Generic error in an external library)
    if ((videoWidth >= 3840 || videoHeight >= 2160) && videoType != VideoType_Rtsp && saveVideoType > 0) {
        if (hardware != "none" && QSysInfo::WordSize == 32) {
            saveVideoType = SaveVideoType_None;
        }
    }

    //非本地摄像头设备没有音频则不用录制音频
    if (audioIndex < 0 && videoType != VideoType_Camera) {
        saveAudioType = SaveAudioType_None;
    }

    //本地摄像头强制采用wav格式
    if (videoType == VideoType_Camera) {
        if (saveAudioType > 1) {
            saveAudioType = SaveAudioType_Wav;
        }
#ifndef Q_OS_WIN
        saveAudioType = SaveAudioType_None;
#endif
    }

    //设置了保存声音并且比特=0则强制改成wav格式
    if (saveAudioType == SaveAudioType_Aac && formatCtx->bit_rate == 0) {
        saveAudioType = SaveAudioType_Wav;
    }
}

void FFmpegThread::initAudioPlayer()
{
    //设置是否需要计算音频振幅
    audioPlayer->setAudioLevel(audioLevel);

    //本地摄像头需要打开本地音频输入
    if (videoType == VideoType_Camera) {
        QMetaObject::invokeMethod(audioPlayer, "openAudioInput", Q_ARG(int, 8000), Q_ARG(int, 1), Q_ARG(int, 16));
        QMetaObject::invokeMethod(audioPlayer, "openAudioOutput", Q_ARG(int, 8000), Q_ARG(int, 1), Q_ARG(int, 16));
        //如果设置过需要保存音频数据则关联音频数据输出到保存音频槽函数
        if (saveAudioType > 0) {
            connect(audioPlayer, SIGNAL(receiveInputData(QByteArray)), this, SLOT(writeAudioData(QByteArray)));
        }
    } else if (audioIndex >= 0) {
        QMetaObject::invokeMethod(audioPlayer, "openAudioOutput", Q_ARG(int, sampleRate), Q_ARG(int, channelCount), Q_ARG(int, 16));
    }
}

void FFmpegThread::initFilter()
{
    //摄像头的视频流一般摄像机那边可以设置好OSD/低分辨率的本地摄像头也不支持
    QString codecName = videoCodecCtx->codec->name;
    if (videoIndex < 0 || codecName == "rawvideo" || videoType == VideoType_Rtsp) {
        filterData.enable = false;
    }

    //初始化视频滤镜用于标签和图形绘制
    filterData.init = true;
    if (filterData.enable) {
        filterData.rotate = rotate;
        filterData.listOsd = listOsd;
        filterData.listGraph = listGraph;
        filterData.formatIn = (hardware == "none" ? AV_PIX_FMT_YUV420P : AV_PIX_FMT_NV12);
        filterData.formatOut = (hardware == "none" ? AV_PIX_FMT_YUV420P : AV_PIX_FMT_NV12);
        FFmpegFilter::initFilter(this, formatCtx->streams[videoIndex], videoCodecCtx, filterData);
    }
}

bool FFmpegThread::openVideo()
{
    //先检查地址是否正常(文件是否存在或者网络地址是否可达)
    if (!VideoHelper::checkUrl(this, videoType, videoUrl, connectTimeout)) {
        return false;
    }

    //启动计时
    timer.start();

    //初始化参数
    this->initOption();
    //初始化输入
    if (!initInput()) {
        return false;
    }
    //初始化视频
    if (!initVideo()) {
        return false;
    }
    //初始化音频
    if (!initAudio()) {
        return false;
    }
    //初始化其他
    this->initOther();

    //启动音视频同步线程
    audioSync->start();
    videoSync->start();

    isOk = true;
    emit recorderStateChanged(RecorderState_Stopped, fileName);
    lastTime = QDateTime::currentDateTime();
    int time = timer.elapsed();
    debug("打开成功", QString("用时: %1 毫秒").arg(time));
    //这里过滤了只有视频的才需要(音频的在下面处理)
    if (videoIndex >= 0) {
        emit receivePlayStart(time);
    }
    return isOk;
}

void FFmpegThread::closeVideo()
{
    speed = 1.0;
    videoIndex = -1;
    audioIndex = -1;
    changePosition = false;

    realPacketSize = 0;
    realPacketCount = 0;

    videoFirstPts = 0;
    audioFirstPts = 0;

    //停止解码同步线程
    audioSync->stop();
    videoSync->stop();

    //先停止录制
    recordStop();
    //搞个标志位判断是否需要调用父类的释放(可以防止重复调用)
    bool needClose = (onlyAudio ? pcmData : yuvData);

    if (packet) {
        FFmpegHelper::freePacket(packet);
        packet = NULL;
    }

    if (yuvSwsCtx) {
        sws_freeContext(yuvSwsCtx);
        yuvSwsCtx = NULL;
    }

    if (imageSwsCtx) {
        sws_freeContext(imageSwsCtx);
        imageSwsCtx = NULL;
    }

    if (pcmSwrCtx) {
        swr_free(&pcmSwrCtx);
        pcmSwrCtx = NULL;
    }

    if (videoFrame) {
        FFmpegHelper::freeFrame(videoFrame);
        videoFrame = NULL;
    }

    if (audioFrame) {
        FFmpegHelper::freeFrame(audioFrame);
        audioFrame = NULL;
    }

    if (yuvFrame) {
        FFmpegHelper::freeFrame(yuvFrame);
        yuvFrame = NULL;
    }

    if (imageFrame) {
        FFmpegHelper::freeFrame(imageFrame);
        imageFrame = NULL;
    }

    if (hardFrame) {
        FFmpegHelper::freeFrame(hardFrame);
        hardFrame = NULL;
    }

    if (yuvData) {
        av_free(yuvData);
        yuvData = NULL;
    }

    if (imageData) {
        av_free(imageData);
        imageData = NULL;
    }

    if (pcmData) {
        av_free(pcmData);
        pcmData = NULL;
    }

    //videoCodec会跟着自动释放
    if (videoCodecCtx) {
        avcodec_free_context(&videoCodecCtx);
        videoCodec = NULL;
        videoCodecCtx = NULL;
    }

    //audioCodec会跟着自动释放
    if (audioCodecCtx) {
        avcodec_free_context(&audioCodecCtx);
        audioCodec = NULL;
        audioCodecCtx = NULL;
    }

    if (options) {
        av_dict_free(&options);
        options = NULL;
    }

    if (formatCtx) {
        avformat_close_input(&formatCtx);
        avformat_free_context(formatCtx);
        formatCtx = NULL;
    }

    //释放滤镜相关
    FFmpegFilter::freeFilter(filterData);

    if (needClose) {
        VideoThread::closeVideo();
    }
}

qint64 FFmpegThread::getStartTime()
{
    return this->startTime;
}

QDateTime FFmpegThread::getLastTime()
{
    return this->lastTime;
}

bool FFmpegThread::getTryOpen()
{
    return this->tryOpen;
}

bool FFmpegThread::getTryRead()
{
    return this->tryRead;
}

bool FFmpegThread::getTryStop()
{
    return this->stopped;
}

int FFmpegThread::getRotate()
{
    //不是默认值说明已经获取过旋转角度不用再去获取
    if (rotate != -1) {
        return rotate;
    }

    rotate = 0;
    if (videoIndex >= 0) {
        AVDictionaryEntry *tag = NULL;
        AVStream *stream = formatCtx->streams[videoIndex];
        tag = av_dict_get(stream->metadata, "rotate", tag, 0);
        if (tag != NULL) {
            rotate = atoi(tag->value);
            emit receiveSizeChanged();
        }
    }

    return rotate;
}

qint64 FFmpegThread::getDuration()
{
    return duration;
}

qint64 FFmpegThread::getPosition()
{
    return position;
}

void FFmpegThread::setPosition(qint64 position)
{
    if (isOk && getIsFile()) {
        //设置切换进度标志位以便暂停解析
        changePosition = true;
        //用定时器设置有个好处避免重复频繁设置进度(频繁设置可能会崩溃)
        this->position = position;
        timerPosition->stop();
        timerPosition->start();
    }
}

void FFmpegThread::setPosition()
{
    if (!isOk) {
        return;
    }

    //清空同步线程缓存
    audioSync->clear();
    videoSync->clear();

    //发过来的是毫秒而参数需要微秒(有些文件开始时间不是0所以需要加上该时间)
    //asf文件开始时间是一个很大的负数这里不需要加上
    qint64 timestamp = position * 1000;
    qint64 timestart = formatCtx->start_time;
    if (timestart > 0) {
        timestamp += timestart;
    }

    //AVSEEK_FLAG_BACKWARD=找到附近画面清晰的帧/AVSEEK_FLAG_ANY=直接定位到指定帧不管画面是否清晰
    av_seek_frame(formatCtx, -1, timestamp, AVSEEK_FLAG_BACKWARD);

    //如果处于暂停阶段在等待新进度切换好以后需要再次暂停
    if (isPause) {
        timerPause->stop();
        timerPause->start();
    }

    //继续播放以便切换到当前新进度的位置
    this->next();
}

double FFmpegThread::getSpeed()
{
    return this->speed;
}

void FFmpegThread::setSpeed(double speed)
{
    if (isOk && getIsFile()) {
        this->pause();
        this->speed = speed;
        this->next();
    }
}

int FFmpegThread::getVolume()
{
    return audioPlayer->getVolume();
}

void FFmpegThread::setVolume(int volume)
{
    audioPlayer->setVolume(volume);
}

bool FFmpegThread::getMuted()
{
    return audioPlayer->getMuted();
}

void FFmpegThread::setMuted(bool muted)
{
    audioPlayer->setMuted(muted);
}

void FFmpegThread::pause()
{
    if (this->isRunning()) {
        isPause = true;
    }
}

void FFmpegThread::next()
{
    if (this->isRunning()) {
        isPause = false;
        changePosition = false;
        //复位同步线程(不复位继续播放后会瞬间快速跳帧)
        audioSync->reset();
        videoSync->reset();
    }
}

void FFmpegThread::recordStart(const QString &fileName)
{
#ifdef videosave
    AbstractVideoThread::recordStart(fileName);
    if ((saveVideoType > 1) && !onlyAudio) {
        this->setFileName(fileName);
        //处于暂停阶段则切换暂停标志位(暂停后再次恢复说明又重新开始录制)
        if (saveFile->getIsPause()) {
            isRecord = true;
            saveFile->pause();
            emit recorderStateChanged(RecorderState_Recording, fileName);
        } else {
            saveFile->setPara(saveVideoType, videoWidth, videoHeight, frameRate, formatCtx->streams[videoIndex], (videoType == VideoType_Camera));
            saveFile->setDirectCopy(videoType == VideoType_Rtsp);
            saveFile->open(fileName);
            if (saveFile->getIsOk()) {
                isRecord = true;
                emit recorderStateChanged(RecorderState_Recording, fileName);
            }
        }
    }
#endif
}

void FFmpegThread::recordPause()
{
#ifdef videosave
    AbstractVideoThread::recordPause();
    if ((saveVideoType > 1) && !onlyAudio) {
        if (saveFile->getIsOk()) {
            isRecord = false;
            saveFile->pause();
            emit recorderStateChanged(RecorderState_Paused, fileName);
        }
    }
#endif
}

void FFmpegThread::recordStop()
{
#ifdef videosave
    AbstractVideoThread::recordStop();
    if ((saveVideoType > 1) && !onlyAudio) {
        if (saveFile->getIsOk()) {
            isRecord = false;
            saveFile->stop();
            //执行过转换合并的不用再发信号
            if (!saveFile->isConvertMerge) {
                emit recorderStateChanged(RecorderState_Stopped, fileName);
            }
        }
    }
#endif
}

void FFmpegThread::setOsdInfo(const QList<OsdInfo> &listOsd)
{
    this->listOsd = listOsd;
    if (filterData.enable && isOk) {
        filterData.init = false;
    }
}

void FFmpegThread::setGraphInfo(const QList<GraphInfo> &listGraph)
{
    this->listGraph = listGraph;
    if (filterData.enable && isOk) {
        filterData.init = false;
    }
}
