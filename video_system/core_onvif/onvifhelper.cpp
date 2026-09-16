#include "onvifhelper.h"
#include "qregexp.h"

QString OnvifHelper::getUuid()
{
    QUuid uid = QUuid::createUuid();
    QString str = uid.toString();
    QRegExp reg("\\{|\\}");
    int start = reg.indexIn(str);
    int length = reg.matchedLength();
    QString temp = str.mid(start, length);
    str.replace(temp, "");
    return str;
}

void OnvifHelper::sleep(int msec)
{
    if (msec > 0) {
#if (QT_VERSION < QT_VERSION_CHECK(5,7,0))
        QTime endTime = QTime::currentTime().addMSecs(msec);
        while (QTime::currentTime() < endTime) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
#else
        QThread::msleep(msec);
#endif
    }
}

bool OnvifHelper::isIP(const QString &ip)
{
    QRegExp RegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    return RegExp.exactMatch(ip);
}

QString OnvifHelper::getUrlIP(const QString &url)
{
    QRegExp regExp("((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))");
    int start = regExp.indexIn(url);
    int length = regExp.matchedLength();
    return url.mid(start, length);
}

QByteArray OnvifHelper::getFile(const QString &fileName)
{
    QByteArray data;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        data = file.readAll();
        //加上回车换行方便打印查看
        data += "\r\n";
        file.close();
    }

    return data;
}

QString OnvifHelper::getFirstUrl(const QString &url, int index)
{
    QString result;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    QStringList list = url.split(' ', Qt::SkipEmptyParts);
#else
    QStringList list = url.split(' ', QString::SkipEmptyParts);
#endif
    if (list.length() > index) {
        result = list.at(index).trimmed();
    }

    return result;
}

QString OnvifHelper::ipv4IntToString(quint32 ip)
{
    QString result = QString("%1.%2.%3.%4").arg((ip >> 24) & 0xFF).arg((ip >> 16) & 0xFF).arg((ip >> 8) & 0xFF).arg(ip & 0xFF);
    return result;
}

quint32 OnvifHelper::ipv4StringToInt(const QString &ip)
{
    int result = 0;
    if (isIP(ip)) {
        QStringList list = ip.split(".");
        int ip0 = list.at(0).toInt();
        int ip1 = list.at(1).toInt();
        int ip2 = list.at(2).toInt();
        int ip3 = list.at(3).toInt();
        result = ip3 | ip2 << 8 | ip1 << 16 | ip0 << 24;
    }
    return result;
}

quint32 OnvifHelper::ipv4ToPrefixLength(const QString &ip)
{
    QStringList list = ip.split(".");
    quint8 c1 = list.at(0).toInt();
    quint8 c2 = list.at(1).toInt();
    quint8 c3 = list.at(2).toInt();
    quint8 c4 = list.at(3).toInt();
    quint32 sum = c1 << 24 | c2 << 16 | c3 << 8 | c4;

    //绝大部分的情况快速判断即可
    if (sum >= 0xffffffff) {
        return 32;
    } else if (sum == 0xffffff00) {
        return 24;
    } else if (sum == 0xffff0000) {
        return 16;
    } else if (sum == 0xff000000) {
        return 8;
    }

    //挨个计算
    quint32 result = 0;
    for (int i = 0; i < 32; ++i) {
        if ((sum << i) & 0x80000000) {
            result++;
        } else {
            break;
        }
    }

    return result;
}

QString OnvifHelper::prefixLengthToIpv4(int prefixlen)
{
    //绝大部分的情况快速判断即可
    if (prefixlen == 8) {
        return "255.0.0.0";
    } else if (prefixlen == 16) {
        return "255.255.0.0";
    } else if (prefixlen == 24) {
        return "255.255.255.0";
    } else if (prefixlen >= 32) {
        return "255.255.255.255";
    }

    int sum = 0;
    for (int i = prefixlen, j = 31; i > 0; i--, j--) {
        sum += (1 << j);
    }

    return QString("%1.%2.%3.%4").arg((sum >> 24) & 0xff).arg((sum >> 16) & 0xff).arg((sum >> 8) & 0xff).arg(sum & 0xff);
}
