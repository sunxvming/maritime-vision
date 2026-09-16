#ifndef VIDEOHELPER_H
#define VIDEOHELPER_H

#include "videohead.h"

class VideoHelper
{
public:
    //获取当前视频内核版本
    static QString getVersion();
    static QString getVersion(const VideoCore &videoCore);
    //设置解码内核(type可以指定内核/否则按照定义的优先级)
    static void initVideoCore(VideoPara &videoPara, int type = 0);

    //根据旧的范围值和值计算新的范围值对应的值
    static int getRangeValue(int oldMin, int oldMax, int oldValue, int newMin, int newMax);

    //校验网络地址是否可达
    static bool checkUrl(const QString &videoUrl, int timeout = 500);
    //检查地址是否正常(文件是否存在或者网络地址是否可达)
    static bool checkUrl(VideoThread *videoThread, const VideoType &videoType, const QString &videoUrl, int timeout = 500);
    //获取转义后的地址(有些视频流带了用户信息有特殊字符需要先转义)
    static QString getRightUrl(const VideoType &videoType, const QString &videoUrl);

    //根据分辨率计算yuv420数据大小
    static int getYuvSize(int width, int height);
    //重命名录制的文件(vlc内核专用)
    static void renameFile(const QString &fileName);
    //鼠标指针复位
    static void resetCursor();

    //加载解析内核到下拉框
    static void loadVideoCore(QComboBox *cbox, int &videoCore);
    //加载硬件加速名称到下拉框
    static void loadHardware(QComboBox *cbox, const VideoCore &videoCore, QString &hardware);
    //加载缓存时间到下拉框
    static void loadCaching(QComboBox *cbox);
    //加载打开间隔到下拉框
    static void loadOpenSleepTime(QComboBox *cbox);
    //加载读取超时到下拉框
    static void loadReadTimeout(QComboBox *cbox);
    //加载连接超时到下拉框
    static void loadConnectTimeout(QComboBox *cbox);

    //根据地址获取类型
    static VideoType getVideoType(const QString &videoUrl);
    //根据地址获取是否只是音频
    static bool getOnlyAudio(const QString &videoUrl);    

    //根据地址获取本地摄像头参数
    static void getCameraPara(const VideoCore &videoCore, QString &videoUrl, QString &bufferSize, int &frameRate);
    //对参数进行矫正
    static VideoType initPara(WidgetPara &widgetPara, VideoPara &videoPara);

    //创建视频采集类
    static VideoThread *newVideoThread(QWidget *parent, const VideoCore &videoCore);
    //对采集线程设置参数
    static void initVideoThread(VideoThread *videoThread, const VideoPara &videoPara);
};

#endif // VIDEOHELPER_H
