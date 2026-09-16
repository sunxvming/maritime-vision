#ifndef URLHELPER_H
#define URLHELPER_H

#include "widgethead.h"

//设备类型
enum DeviceType {
    DeviceType_Normal = 0,  //通用厂家
    DeviceType_HaiKang = 1, //海康威视
    DeviceType_DaHua = 2,   //大华股份
    DeviceType_YuShi = 3,   //杭州宇视
    DeviceType_ShenGuang = 4//上海深广
};

//地址信息结构体
struct UrlInfo {
    QString ip;             //通信地址
    int port;               //通信端口
    QString userName;       //用户名称
    QString userPwd;        //用户密码

    int channel;            //通道编号
    int streamType;         //码流类型
    QString company;        //厂家标识
    DeviceType deviceType;  //设备类型

    UrlInfo() {
        port = 0;
        channel = 0;
        streamType = 0;
    }

    //重载打印输出格式
    friend QDebug operator << (QDebug debug, const UrlInfo &urlInfo) {
        QStringList list;
        list << QString("通信地址: %1").arg(urlInfo.ip);
        list << QString("通信端口: %1").arg(urlInfo.port);
        list << QString("用户名称: %1").arg(urlInfo.userName);
        list << QString("用户密码: %1").arg(urlInfo.userPwd);

        list << QString("通道编号: %1").arg(urlInfo.channel);
        list << QString("码流类型: %1").arg(urlInfo.streamType);
        list << QString("厂家标识: %1").arg(urlInfo.company);
        list << QString("设备类型: %1").arg(urlInfo.deviceType);

#if (QT_VERSION >= QT_VERSION_CHECK(5,4,0))
        debug.noquote() << list.join("\n");
#else
        debug << list.join("\n");
#endif
        return debug;
    }
};

class UrlHelper
{
public:
    //各个厂家的实时视频流字符串(通道和码流默认约定从0开始)
    static QString getRtspUrl(const DeviceType &deviceType, int channel = 0, int streamType = 0);

    //根据地址获取地址对应的各种信息
    static QString getUrlIP(const QString &url);
    static int getUrlPort(const QString &url);
    static void getUserInfo(const QString &url, QString &userName, QString &userPwd);
    static QString getCompany(const QString &url);
    static DeviceType getDeviceType(const QString &url);
    static void getOtherInfo(const QString &url, int &channel, int &streamType);
    static void getUrlInfo(const QString &url, UrlInfo &urlInfo);
};

#endif // URLHELPER_H
