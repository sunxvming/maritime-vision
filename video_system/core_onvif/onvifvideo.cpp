#include "onvifvideo.h"

OnvifVideo::OnvifVideo(QObject *parent) : QObject(parent)
{
    device = (OnvifDevice *)parent;
}

QList<OnvifVideoSource> OnvifVideo::getVideoSources()
{
    QList<OnvifVideoSource> videoSources;
    if (device->mediaUrl.isEmpty()) {
        return videoSources;
    }

    //读取文件传入带用户认证的通用头部数据和其他参数构建要发送的数据
    QString file = OnvifHelper::getFile(":/onvifsend/GetVideoSources.xml");
    file = file.arg(device->getHeadData());

    //发送请求数据
    QByteArray dataSend = file.toUtf8();
    QNetworkReply *reply = device->request->post(device->mediaUrl, dataSend);

    //拿到请求结果并处理数据
    QByteArray dataReceive;
    bool ok = device->checkData(reply, dataReceive, "获取视频配置");
    if (ok) {
        //解析视频配置参数
        OnvifQuery query;
        if (query.setData(dataReceive)) {
            videoSources = query.getVideoSources();
        }
    }

    return videoSources;
}

void OnvifVideo::getImageOption(const QString &videoSource, OnvifImageSetting &imageSetting)
{
    if (device->imageUrl.isEmpty()) {
        return;
    }

    //读取文件传入带用户认证的通用头部数据和其他参数构建要发送的数据
    QString file = OnvifHelper::getFile(":/onvifsend/GetOptions.xml");
    file = file.arg(device->getHeadData()).arg(videoSource);

    //发送请求数据
    QByteArray dataSend = file.toUtf8();
    QNetworkReply *reply = device->request->post(device->imageUrl, dataSend);

    //拿到请求结果并处理数据
    QByteArray dataReceive;
    bool ok = device->checkData(reply, dataReceive, "获取参数范围");
    if (ok) {
        //解析图片参数范围
        OnvifQuery query;
        if (query.setData(dataReceive)) {
            query.getImageOption(imageSetting);
        }
    }
}

void OnvifVideo::getImageSetting(const QString &videoSource, OnvifImageSetting &imageSetting)
{
    if (device->imageUrl.isEmpty()) {
        return;
    }

    //读取文件传入带用户认证的通用头部数据和其他参数构建要发送的数据
    QString file = OnvifHelper::getFile(":/onvifsend/GetImagingSettings.xml");
    file = file.arg(device->getHeadData()).arg(videoSource);

    //发送请求数据
    QByteArray dataSend = file.toUtf8();
    QNetworkReply *reply = device->request->post(device->imageUrl, dataSend);

    //拿到请求结果并处理数据
    QByteArray dataReceive;
    bool ok = device->checkData(reply, dataReceive, "获取图片参数");
    if (ok) {
        //解析图片配置参数
        OnvifQuery query;
        if (query.setData(dataReceive)) {
            query.getImageSetting(imageSetting);
        }
    }
}

bool OnvifVideo::setImageSetting(const QString &videoSource, const OnvifImageSetting &imageSetting)
{
    return false;
}
