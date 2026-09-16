#include "filterhelper.h"

//官方文档: https://ffmpeg.org/ffmpeg-filters.html
//视频滤镜: https://blog.csdn.net/yang1fei2/article/details/127939788
//音频滤镜: https://blog.csdn.net/yang1fei2/article/details/127777958
//播放示例: ffplay -i f:/mp4/1.mp4 -vf eq=brightness=0.5:contrast=100.0:saturation=3.0
//转换示例: ffmpeg -y -i f:/mp4/1.mp4 -vf drawtext=text='%{pts\:hms}':fontsize=30:fontcolor=#ff0000:x=10:y=10 f:/mp4/out.mp4

QString FilterHelper::getFilter()
{
    //图片模糊: boxblur=2:2
    //图片黑白: lutyuv=u=128:v=128
    //格式转换: format=pix_fmts=yuvj422p
    //网格线条: drawgrid=w=100:h=100:t=2:c=red@0.5
    //图片参数: eq=contrast=1.0:brightness=0.8:saturation=1.0:gamma=1.0
    //尖锐程度: unsharp=13:13
    //图片参数: 对比度contrast=1.0[-1000.0,1000.0]/明亮度brightness=0.0[-1.0,1.0]/饱和度saturation=1.0[0.0,3.0]/伽马gamma=1.0[0.1,10.0]

    QString filter;
    //filter = "boxblur=2:2";
    return filter;
}

QString FilterHelper::getFilter(int rotate, bool hardware)
{
    //0=逆时针旋转90度并垂直翻转
    //1=顺时针旋转90度
    //2=逆时针旋转90度
    //3=顺时针旋转90度并水平翻转
    //hflip=水平翻转
    //vflip=垂直翻转

    QString filter;
    if (rotate == 90) {
        filter = "transpose=1";
    } else if (rotate == 180) {
        filter = "vflip";
    } else if (rotate == 270) {
        filter = "transpose=2";
    } else if (rotate > 0) {
        filter = QString("rotate=%1*PI/180").arg(rotate);
    }

    return filter;
}

QString FilterHelper::getFilter(const OsdInfo &osd, bool hardware)
{
    QString filter;
    if (!osd.visible) {
        return filter;
    }

    //图片滤镜没搞定
    if (osd.format == OsdFormat_Image) {
        return filter;
    }

    //硬解码不支持日期时间(很奇怪不知道什么原因)
    if (hardware) {
        if (osd.format == OsdFormat_Date || osd.format == OsdFormat_Time || osd.format == OsdFormat_DateTime) {
            return filter;
        }
    }

    //drawtext=text='hello':fontsize=30:fontcolor=#ffffff:x=10:y=10
    //linux上需要对应的ffmpeg库编译的时候指定freetype否则不能用drawtext

    //计算对应的坐标位置
    QString offset = "10";
    QString x = QString::number(osd.point.x());
    QString y = QString::number(osd.point.y());
    if (osd.position == OsdPosition_LeftTop) {
        x = offset;
        y = offset;
    } else if (osd.position == OsdPosition_LeftBottom) {
        x = offset;
        y = "h-th-" + offset;
    } else if (osd.position == OsdPosition_RightTop) {
        x = "w-tw-" + offset;
        y = offset;
    } else if (osd.position == OsdPosition_RightBottom) {
        x = "w-tw-" + offset;
        y = "h-th-" + offset;
    } else if (osd.position == OsdPosition_Center) {
        x = "(w-tw)/2";
        y = "(h-th)/2";
    }

    //时间格式: https://baike.baidu.com/item/strftime/9569073 pts时间 %{pts\\:hms}
    QString text = osd.text;
    if (osd.format == OsdFormat_Date) {
        text = "%{localtime:%Y-%m-%d}";
    } else if (osd.format == OsdFormat_Time) {
        text = "%{localtime:%H\\\\:%M\\\\:%S}";
    } else if (osd.format == OsdFormat_DateTime) {
        text = "%{localtime}";
        //text = "%{localtime:%Y-%m-%d %H\\\\:%M\\\\:%S}";
    }

    QStringList list;
    //如果有:需要转义
    list << QString("text='%1'").arg(text.replace(":", "\\:"));
    list << QString("fontsize=%1").arg(osd.fontSize);
    QColor color = osd.color;
    list << QString("fontcolor=%1@%2").arg(color.name()).arg(color.alphaF());
    list << QString("fontfile=%1").arg(".//wenquanyi.ttf");
    list << QString("x=%1").arg(x);
    list << QString("y=%1").arg(y);
    //背景颜色
    //list << QString("box=1:boxcolor=%1").arg("#158863");
    filter = QString("drawtext=%1").arg(list.join(":"));
    return filter;
}

QString FilterHelper::getFilter(const GraphInfo &graph, bool hardware)
{
    //drawbox=x=10:y=10:w=100:h=100:c=#ffffff@1:t=2
    QString filter;
    //硬解码不支持
    if (hardware) {
        return filter;
    }

    //暂时只实现了矩形区域
    QRect rect = graph.rect;
    if (rect.isEmpty()) {
        return filter;
    }

    QStringList list;
    list << QString("x=%1").arg(rect.x());
    list << QString("y=%1").arg(rect.y());
    list << QString("w=%1").arg(rect.width());
    list << QString("h=%1").arg(rect.height());

    QColor color = graph.borderColor;
    list << QString("c=%1@%2").arg(color.name()).arg(color.alphaF());

    //背景颜色不透明则填充背景颜色
    if (graph.bgColor == Qt::transparent) {
        list << QString("t=%1").arg(graph.borderWidth);
    } else {
        list << QString("t=%1").arg("fill");
    }

    filter = QString("drawbox=%1").arg(list.join(":"));
    return filter;
}
