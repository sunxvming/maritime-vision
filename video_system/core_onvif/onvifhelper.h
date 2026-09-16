#ifndef ONVIFHELPER_H
#define ONVIFHELPER_H

#include "onvifdevice.h"
class OnvifDevice;

class OnvifHelper
{
public:
    //获取uuid
    static QString getUuid();
    //设置延时
    static void sleep(int msec);

    //判断是否是合法的IP
    static bool isIP(const QString &ip);
    //取出url地址中的地址
    static QString getUrlIP(const QString &url);

    //读取本地文件内容
    static QByteArray getFile(const QString &fileName);
    //取出第一个url
    static QString getFirstUrl(const QString &url, int index = 0);

    //IP地址字符串与整型转换
    static QString ipv4IntToString(quint32 ip);
    static quint32 ipv4StringToInt(const QString &ip);

    //子网掩码和前缀长度转换 24=255.255.255.0
    static quint32 ipv4ToPrefixLength(const QString &ip);
    static QString prefixLengthToIpv4(int prefixlen);
};

#endif // ONVIFHELPER_H
