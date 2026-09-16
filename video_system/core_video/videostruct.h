#ifndef VIDEOSTRUCT_H
#define VIDEOSTRUCT_H

#include <QString>
#include <QList>
#include <QDebug>

//解析内核
enum VideoCore {
    VideoCore_None = 0,         //不解析处理(默认值)

    VideoCore_QMedia = 1,       //采用qmedia解析(qt自带且依赖本地解码器且部分平台支持)
    VideoCore_FFmpeg = 2,       //采用ffmpeg解析(通用性最好)
    VideoCore_Vlc = 3,          //采用vlc解析(支持本地文件最好)
    VideoCore_Mpv = 4,          //采用mpv解析(支持本地文件最好且跨平台最多)
    VideoCore_Qtav = 5,         //采用qtav解析(框架结构最好/基于ffmpeg)

    VideoCore_HaiKang = 10,     //采用海康sdk解析
    VideoCore_DaHua = 11,       //采用大华sdk解析
    VideoCore_YuShi = 12,       //采用宇视sdk解析

    VideoCore_EasyPlayer = 20,  //采用easyplayer解析    
};

//视频类型
enum VideoType {
    VideoType_FileLocal = 0,    //本地文件
    VideoType_FileHttp = 1,     //网络文件
    VideoType_Camera = 2,       //本地摄像头
    VideoType_Rtsp = 3,         //视频流rtsp
    VideoType_Rtmp = 4,         //视频流rtmp
    VideoType_Http = 5,         //视频流http
    VideoType_Other = 255       //其他未知
};

//解码速度策略
enum DecodeType {
    DecodeType_Fast = 0,        //速度优先
    DecodeType_Full = 1,        //质量优先
    DecodeType_Even = 2,        //均衡处理
    DecodeType_Fastest = 3      //最快速度(速度优先且不做任何音视频同步)
};

//视频采集参数
struct VideoPara {
    VideoCore videoCore;        //解析内核
    QString videoUrl;           //视频地址
    QString bufferSize;         //缓存分辨率
    int frameRate;              //视频帧率

    DecodeType decodeType;      //解码速度策略
    QString hardware;           //硬件加速名称
    QString transport;          //通信协议(tcp udp)
    int caching;                //缓存时间(默认500毫秒)

    bool audioLevel;            //开启音频振幅
    bool decodeAudio;           //解码音频数据
    bool playAudio;             //解码播放声音
    bool playRepeat;            //重复循环播放

    int openSleepTime;          //打开休息时间(最低1000 单位毫秒)
    int readTimeout;            //采集超时时间(0=不处理 单位毫秒)
    int connectTimeout;         //连接超时时间(0=不处理 单位毫秒)

    VideoPara() {
        videoCore = VideoCore_None;
        videoUrl = "";
        bufferSize = "0x0";
        frameRate = 0;

        decodeType = DecodeType_Fast;
        hardware = "none";
        transport = "tcp";
        caching = 500;

        audioLevel = false;
        decodeAudio = true;
        playAudio = true;
        playRepeat = false;

        openSleepTime = 3000;
        readTimeout = 0;
        connectTimeout = 500;
    }

    void reset() {
        videoUrl = "";
        bufferSize = "0x0";
        frameRate = 0;
    }
};

#endif // VIDEOSTRUCT_H
