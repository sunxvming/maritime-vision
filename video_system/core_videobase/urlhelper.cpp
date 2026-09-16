#include "urlhelper.h"
#include "qregexp.h"

QString UrlHelper::getRtspUrl(const DeviceType &deviceType, int channel, int streamType)
{
    QString url;
    if (deviceType == DeviceType_HaiKang) {
        url = QString("rtsp://admin:12345@[Addr]:554/Streaming/Channels/%1%2?transportmode=unicast").arg(channel + 1).arg(streamType + 1, 2, 10, QChar('0'));
    } else if (deviceType == DeviceType_DaHua) {
        url = QString("rtsp://admin:12345@[Addr]:554/cam/realmonitor?channel=%1&subtype=%2&unicast=true&proto=Onvif").arg(channel + 1).arg(streamType);
    } else if (deviceType == DeviceType_YuShi) {
        url = QString("rtsp://admin:12345@[Addr]/media/video%1").arg(streamType + 1);
    } else if (deviceType == DeviceType_ShenGuang) {
        url = QString("rtsp://admin:12345@[Addr]/live?channel=%1&stream=%2").arg(channel + 1).arg(streamType);
    } else {
        url = QString("rtsp://admin:12345@[Addr]:554/%1").arg(streamType);
    }
    return url;
}

QString UrlHelper::getUrlIP(const QString &url)
{
    QRegExp regExp("((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))");
    int start = regExp.indexIn(url);
    int length = regExp.matchedLength();
    return url.mid(start, length);
}

int UrlHelper::getUrlPort(const QString &url)
{
    //必须是最后一个:符号(用户可能地址中带了用户信息用:作为分隔符)
    int index = url.lastIndexOf(":");
    QString temp = url.mid(index + 1, 6);
    QStringList list = temp.split("/");
    int port = list.at(0).toInt();
    return (port == 0 ? 554 : port);
}

void UrlHelper::getUserInfo(const QString &url, QString &userName, QString &userPwd)
{
    userName = "admin";
    userPwd = "12345";
    //必须是最后一个@符号(因为用户名称和密码也可能是这个字符)
    int index = url.lastIndexOf("@");
    if (index > 0) {
        QString userInfo = url.mid(0, index);
        userInfo.replace("rtsp://", "");
        QStringList list = userInfo.split(":");
        userName = list.at(0);
        userPwd = list.at(1);
    }
}

QString UrlHelper::getCompany(const QString &url)
{
    QString company;
    if (url.contains("/Streaming/Channels/")) {
        company = "HIKVISION";
    } else if (url.contains("/cam/realmonitor?channel=")) {
        company = "Dahua";
    } else if (url.contains("/media/video")) {
        company = "UNIVIEW";
    } else if (url.contains("/live?channel=")) {
        company = "General";
    }
    return company;
}

DeviceType UrlHelper::getDeviceType(const QString &url)
{
    DeviceType deviceType;
    if (url.contains("/Streaming/Channels/")) {
        deviceType = DeviceType_HaiKang;
    } else if (url.contains("/cam/realmonitor?channel=")) {
        deviceType = DeviceType_DaHua;
    } else if (url.contains("/media/video")) {
        deviceType = DeviceType_YuShi;
    } else if (url.contains("/live?channel=")) {
        deviceType = DeviceType_ShenGuang;
    }
    return deviceType;
}

void UrlHelper::getOtherInfo(const QString &url, int &channel, int &streamType)
{
    DeviceType deviceType = getDeviceType(url);
    QString temp = url.split("/").last();
    if (deviceType == DeviceType_HaiKang) {
        temp = temp.split("?").first();
        //101=通道1码流01 1602=通道16码流02
        channel = temp.mid(0, temp.length() - 2).toInt();
        streamType = temp.right(2).toInt();
    } else if (deviceType == DeviceType_DaHua) {
        temp = temp.split("?").last();
        QStringList list = temp.split("&");
        foreach (QString text, list) {
            int value = text.split("=").last().toInt();
            if (text.startsWith("channel")) {
                channel = value;
            } else if (text.startsWith("subtype")) {
                streamType = value;
            }
        }
    } else if (deviceType == DeviceType_YuShi) {
        streamType = temp.mid(5, 2).toInt();
    } else if (deviceType == DeviceType_ShenGuang) {
        temp = temp.split("?").last();
        QStringList list = temp.split("&");
        foreach (QString text, list) {
            int value = text.split("=").last().toInt();
            if (text.startsWith("channel")) {
                channel = value;
            } else if (text.startsWith("stream")) {
                streamType = value;
            }
        }
    } else {
        streamType = temp.toInt();
    }
}

void UrlHelper::getUrlInfo(const QString &url, UrlInfo &urlInfo)
{
    urlInfo.ip = getUrlIP(url);
    urlInfo.port = getUrlPort(url);
    getUserInfo(url, urlInfo.userName, urlInfo.userPwd);
    urlInfo.company = getCompany(url);
    urlInfo.deviceType = getDeviceType(url);
    getOtherInfo(url, urlInfo.channel, urlInfo.streamType);
}
