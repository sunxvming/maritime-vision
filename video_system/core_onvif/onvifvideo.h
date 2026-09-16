#ifndef ONVIFVIDEO_H
#define ONVIFVIDEO_H

#include "onvifhead.h"

class OnvifVideo : public QObject
{
    Q_OBJECT
public:
    explicit OnvifVideo(QObject *parent = 0);

private:
    //onvif设备对象
    OnvifDevice *device;

public slots:
    //获取视频配置文件
    QList<OnvifVideoSource> getVideoSources();

    //图片参数 Brightness(亮度) ColorSaturation(色彩饱和度) Contrast(饱和度) Sharpness(锐度)
    //特别注意 这几个参数范围值有多种 0-100(海康) 0-128(锐度) 0-255(大部分)
    void getImageOption(const QString &videoSource, OnvifImageSetting &imageSetting);
    void getImageSetting(const QString &videoSource, OnvifImageSetting &imageSetting);
    //下面设置图片参数暂时在onvifother类中实现的
    bool setImageSetting(const QString &videoSource, const OnvifImageSetting &imageSetting);
};

#endif // ONVIFVIDEO_H
