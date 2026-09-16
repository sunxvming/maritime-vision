#include "videourl.h"
#include "qfile.h"
#include "qdebug.h"
#ifdef Q_CC_MSVC
#pragma execution_character_set("utf-8")
#endif

void VideoUrl::readUrls(const QString &fileName, QStringList &urls)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return;
    }

    QTextStream in(&file);
    int size = urls.size();
    while (!in.atEnd()) {
        //每次读取一行
        QString line = in.readLine();
        //去除空格回车换行
        line = line.trimmed();
        line.replace("\r", "");
        line.replace("\n", "");
        //空行或者注释行不用处理
        if (line.isEmpty() || line.startsWith("#")) {
            continue;
        }

        //格式: 0,rtsp://192.168.1.200
        QStringList list = line.split(",");
        if (list.size() != 2) {
            continue;
        }

        int channel = list.at(0).toInt();
        //替换成新的url
        if (channel >= 0 && channel < size) {
            urls[channel] = list.at(1);
        }
    }

    file.close();
}

void VideoUrl::writeUrls(const QString &fileName, const QStringList &urls)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        return;
    }

    QTextStream out(&file);
    int size = urls.size();
    for (int i = 0; i < size; ++i) {
        QString url = urls.at(i);
        if (!url.isEmpty()) {
            out << QString("%1,%2\n").arg(i).arg(url);
        }
    }

    file.close();
}

QStringList VideoUrl::getUrls(int type)
{
    QStringList urls;

    //安卓上暂时就放几个测试就好
#ifdef Q_OS_ANDROID
    appendUrl(urls, "/dev/video0");
    appendUrl(urls, "rtmp://livetv.dhtv.cn:1935/live/news");
    appendUrl(urls, "http://39.135.138.60:18890/TVOD/88888910/224/3221225619/index.m3u8");
    appendUrl(urls, "http://vfx.mtime.cn/Video/2019/03/18/mp4/190318231014076505.mp4");
    appendUrl(urls, "http://vfx.mtime.cn/Video/2019/03/19/mp4/190319212559089721.mp4");
    appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.64:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_2");
    appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.106:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif");
    return urls;
#endif

    if (type & 0x01) {
        appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.15:554/media/video1");
        appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.15:554/media/video2");
        appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.64:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_2");
        appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.64:554/Streaming/Channels/102?transportmode=unicast&profile=Profile_2");
        appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.106:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif");
        appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.106:554/cam/realmonitor?channel=1&subtype=1&unicast=true&proto=Onvif");
        appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.160:554/stream0?username=admin&password=E10ADC3949BA59ABBE56E057F20F883E");
        appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.160:554/stream1?username=admin&password=E10ADC3949BA59ABBE56E057F20F883E");

        appendUrl(urls, "http://vts.simba-cn.com:280/gb28181/21100000001320000002.m3u8");
        appendUrl(urls, "https://hls01open.ys7.com/openlive/6e0b2be040a943489ef0b9bb344b96b8.hd.m3u8");
    }

    if (type & 0x02) {
        appendUrl(urls, "http://vfx.mtime.cn/Video/2019/02/04/mp4/190204084208765161.mp4");
        appendUrl(urls, "http://vfx.mtime.cn/Video/2019/03/18/mp4/190318231014076505.mp4");
        appendUrl(urls, "http://vfx.mtime.cn/Video/2019/03/19/mp4/190319212559089721.mp4");
        appendUrl(urls, "http://vfx.mtime.cn/Video/2019/03/17/mp4/190317150237409904.mp4");
        appendUrl(urls, "http://vfx.mtime.cn/Video/2019/03/14/mp4/190314223540373995.mp4");
        appendUrl(urls, "http://vjs.zencdn.net/v/oceans.mp4");
        appendUrl(urls, "https://media.w3.org/2010/05/sintel/trailer.mp4");
        appendUrl(urls, "https://sf1-hscdn-tos.pstatp.com/obj/media-fe/xgplayer_doc_video/flv/xgplayer-demo-360p.flv");
    }

    if (type & 0x04) {
        //格式 搜索到的 https://music.163.com/#/song?id=179768
        appendUrl(urls, "http://music.163.com/song/media/outer/url?id=179768.mp3");
        appendUrl(urls, "http://music.163.com/song/media/outer/url?id=5238772.mp3");
        appendUrl(urls, "http://music.163.com/song/media/outer/url?id=447925558.mp3");
    }

    if (type & 0x08) {
#ifdef Q_OS_WIN
        appendUrl(urls, "f:/mp4/1.mp4");
        appendUrl(urls, "f:/mp4/1000.mkv");
        appendUrl(urls, "f:/mp4/1001.wmv");
        appendUrl(urls, "f:/mp4/1002.mov");
        appendUrl(urls, "f:/mp4/1003.mp4");
        appendUrl(urls, "f:/mp4/1080.mp4");

        appendUrl(urls, "f:/mp5/1.avi");
        appendUrl(urls, "f:/mp5/1.ts");
        appendUrl(urls, "f:/mp5/1.asf");
        appendUrl(urls, "f:/mp5/1.mp4");
        appendUrl(urls, "f:/mp5/5.mp4");
        appendUrl(urls, "f:/mp5/1.rmvb");
        appendUrl(urls, "f:/mp5/4k.mp4");
        appendUrl(urls, "f:/mp5/h264.mp4");
        appendUrl(urls, "f:/mp5/h265.mp4");
        appendUrl(urls, "f:/mp5/haikang.mp4");
        appendUrl(urls, "f:/mp4/新建文件夹/1.mp4");
#endif
    }

    if (type & 0x10) {
#ifdef Q_OS_WIN
        appendUrl(urls, "f:/mp3/1.mp3");
        appendUrl(urls, "f:/mp3/1.wav");
        appendUrl(urls, "f:/mp3/1.wma");
        appendUrl(urls, "f:/mp3/1.mid");
#endif
    }

    if (type & 0x20) {
#ifdef Q_OS_WIN
        appendUrl(urls, "video=USB Video Device|1280x720|30");
        appendUrl(urls, "dshow://:dshow-vdev=USB PC CAMERA01|640*480|25");
#else
        appendUrl(urls, "/dev/video0");
        appendUrl(urls, "/dev/video1");
#endif
    }

    if (type & 0x40) {
#ifdef Q_OS_WIN
        appendUrl(urls, "f:/mp3/1.mp3");
        appendUrl(urls, "f:/mp4/1001.wmv");
        appendUrl(urls, "f:/mp4/1003.mp4");
        appendUrl(urls, "f:/mp5/1.mp4");
        appendUrl(urls, "f:/mp5/4k.mp4");
        appendUrl(urls, "f:/mp5/h265.mp4");
        appendUrl(urls, "f:/mp5/haikang.mp4");
        appendUrl(urls, "f:/mp6/1.mkv");
        appendUrl(urls, "f:/mp4/新建文件夹/1.mp4");
#else
        appendUrl(urls, "/home/liu/1.mp4");
        appendUrl(urls, "/dev/video0");
#endif
        appendUrl(urls, "video=USB Video Device|1280x720|30");
        appendUrl(urls, "rtmp://livetv.dhtv.cn:1935/live/news");
        appendUrl(urls, "http://39.135.138.60:18890/TVOD/88888910/224/3221225619/index.m3u8");
        appendUrl(urls, "http://music.163.com/song/media/outer/url?id=5238772.mp3");
        appendUrl(urls, "http://vfx.mtime.cn/Video/2019/03/18/mp4/190318231014076505.mp4");
        appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.64:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_2");
        appendUrl(urls, "rtsp://admin:Admin123456@192.168.0.106:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif");
    }

    if (type & 0x80) {
        appendUrl(urls, "rtmp://livetv.dhtv.cn:1935/live/news");
        appendUrl(urls, "rtmp://livetv.dhtv.cn:1935/live/peoples");
        appendUrl(urls, "rtmp://livetv.dhtv.cn:1935/live/citylife");
        appendUrl(urls, "rtmp://livetv.dhtv.cn:1935/live/financial");
    }

    return urls;
}

void VideoUrl::appendUrl(QStringList &urls, const QString &url)
{
    if (!urls.contains(url)) {
        urls << url;
    }
}

QStringList VideoUrl::getUrls2(int index)
{
    QStringList urls;
    if (index == 1) {
        for (int i = 1; i <= 4; ++i) {
            urls << QString("f:/mp4/%1.mp4").arg(i);
        }
    } else if (index == 2) {
        urls << "f:/mp5/1.mp4";
        urls << "f:/mp5/2.mp4";
        urls << "f:/mp5/3.mp4";
        urls << "f:/mp5/4k.mp4";
    } else if (index == 3) {
        for (int i = 1; i <= 16; ++i) {
            urls << QString("f:/mp4/%1.mp4").arg(i);
        }
    } else if (index == 4) {
        urls << "http://vfx.mtime.cn/Video/2019/03/18/mp4/190318231014076505.mp4";
        urls << "http://vfx.mtime.cn/Video/2019/03/19/mp4/190319212559089721.mp4";
        urls << "http://vfx.mtime.cn/Video/2019/03/17/mp4/190317150237409904.mp4";
        urls << "http://vfx.mtime.cn/Video/2019/03/14/mp4/190314223540373995.mp4";
    } else if (index == 5) {
        for (int i = 1; i <= 16; ++i) {
            urls << "http://vfx.mtime.cn/Video/2019/03/17/mp4/190317150237409904.mp4";
        }
    } else if (index == 6) {
        urls << "rtmp://livetv.dhtv.cn:1935/live/news";
        urls << "rtmp://livetv.dhtv.cn:1935/live/peoples";
        urls << "rtmp://livetv.dhtv.cn:1935/live/citylife";
        urls << "rtmp://livetv.dhtv.cn:1935/live/financial";
    } else if (index == 7) {
        for (int i = 1; i <= 16; ++i) {
            urls << "rtmp://livetv.dhtv.cn:1935/live/news";
        }
    } else if (index == 8) {
        urls << "rtsp://admin:Admin123456@192.168.0.15:554/media/video1";
        urls << "rtsp://admin:Admin123456@192.168.0.64:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_2";
        urls << "rtsp://admin:Admin123456@192.168.0.106:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif";
        urls << "rtsp://admin:Admin123456@192.168.0.107:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif";
    } else if (index == 9) {
        for (int i = 1; i <= 16; ++i) {
            urls << "rtsp://admin:Admin123456@192.168.0.106:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif";
        }
    } else if (index == 10) {
        for (int i = 1; i <= 4; ++i) {
            urls << "rtsp://admin:Admin123456@192.168.0.15:554/media/video1";
        }
        for (int i = 1; i <= 4; ++i) {
            urls << "rtsp://admin:Admin123456@192.168.0.64:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_2";
        }
        for (int i = 1; i <= 4; ++i) {
            urls << "rtsp://admin:Admin123456@192.168.0.106:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif";
        }
        for (int i = 1; i <= 4; ++i) {
            urls << "rtsp://admin:Admin123456@192.168.0.107:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif";
        }
    } else if (index == 11) {
        for (int i = 1; i <= 4; ++i) {
            urls << "rtsp://admin:Admin123456@192.168.0.15:554/media/video2";
        }
        for (int i = 1; i <= 4; ++i) {
            urls << "rtsp://admin:Admin123456@192.168.0.64:554/Streaming/Channels/102?transportmode=unicast&profile=Profile_2";
        }
        for (int i = 1; i <= 4; ++i) {
            urls << "rtsp://admin:Admin123456@192.168.0.106:554/cam/realmonitor?channel=1&subtype=1&unicast=true&proto=Onvif";
        }
        for (int i = 1; i <= 4; ++i) {
            urls << "rtsp://admin:Admin123456@192.168.0.107:554/cam/realmonitor?channel=1&subtype=1&unicast=true&proto=Onvif";
        }
    }

    return urls;
}
