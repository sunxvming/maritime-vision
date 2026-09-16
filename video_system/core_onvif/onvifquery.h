#ifndef ONVIFQUERY_H
#define ONVIFQUERY_H

#include "onvifhead.h"

class OnvifQuery : public QObject
{
    Q_OBJECT
public:
    explicit OnvifQuery(QObject *parent = 0);

private:
    //处理的数据
    QByteArray buffer;
    //节点地址
    OnvifWsdlAddr wsdlAddr;
    //xml数据解析对象
    QDomDocument doc;

public:
    //通用获取数据节点方法
    QString getValue2(const QString &key);
    QString getValue3(const QString &key);
    QString getValue4(const QString &key);

    //支持直接指定本地的文件进行处理
    bool setData(const QByteArray &data, const QString &fileName = QString());    

    //获取搜索设备 能够拿到部分设备信息
    void getSearchInfo(OnvifDeviceInfo &deviceInfo, const QString &ip);
    //获取设备信息 拿到剩余部分设备信息
    void getDeviceInfo(OnvifDeviceInfo &deviceInfo);
    //获取设备信息 拿到剩余部分设备信息
    void getScopes(OnvifDeviceInfo &deviceInfo);

    //重新校验地址
    void checkAddr(const QString &ipport, QString &addr, bool checkOnvif, bool replacePort);

    //获取服务地址(早期onvif协议要用getCapabilities)
    OnvifHttpAddr getServices(const QString &ipport);
    OnvifHttpAddr getCapabilities(const QString &ipport);

    //获取服务文件
    QList<OnvifProfileInfo> getProfiles();
    //获取预置位集合
    QList<OnvifPresetInfo> getPresets();

    //通用获取地址
    QString getValueByTagName(const QString &name, const QString &wsdl);
    QString getValueByTagName(QDomElement element, const QString &name);

    //获取视频流地址
    QString getStreamUri(const QString &ipport);
    //获取截图地址
    QString getSnapshotUri(const QString &ipport);

    //获取订阅事件请求地址
    QString getEventAddr(const QString &ipport);
    //获取事件内容
    OnvifEventInfo getEventInfo();

    //获取视频配置集合
    QList<OnvifVideoSource> getVideoSources();

    //获取图片参数范围及参数值
    void getImageOption(const QString &tagName, qreal &min, qreal &max);
    void getImageOption(OnvifImageSetting &imageSetting);
    void getImageSetting(OnvifImageSetting &imageSetting);
};

#endif // ONVIFQUERY_H
