#ifndef WIDGETHELPER_H
#define WIDGETHELPER_H

#include "widgethead.h"
#include "abstractvideowidget.h"

class WidgetHelper
{
public:
    //传入图片尺寸和窗体区域及边框大小返回居中区域(scaleMode: 0-自动调整 1-等比缩放 2-拉伸填充)
    static QRect getCenterRect(const QSize &imageSize, const QRect &widgetRect, int borderWidth = 2, const ScaleMode &scaleMode = ScaleMode_Auto);
    //传入图片尺寸和窗体尺寸及缩放策略返回合适尺寸(scaleMode: 0-自动调整 1-等比缩放 2-拉伸填充)
    static void getScaledImage(QImage &image, const QSize &widgetSize, const ScaleMode &scaleMode = ScaleMode_Auto, bool fast = true);

    //绘制标签信息
    static void drawOsd(QPainter *painter, const OsdInfo &osd, const QRect &rect);
    //绘制矩形区域比如人脸框    
    static void drawRect(QPainter *painter, const QRect &rect, int borderWidth = 3, QColor borderColor = Qt::red, bool angle = false);
    //绘制点集合多边形路径比如三角形
    static void drawPoints(QPainter *painter, const QList<QPoint> &pts, int borderWidth = 3, QColor borderColor = Qt::red);
    //绘制路径集合
    static void drawPath(QPainter *painter, QPainterPath path, int borderWidth = 3, QColor borderColor = Qt::red);

    //显示截图预览
    static QLabel *showImage(QLabel *label, QWidget *widget, const QImage &image);
    //演示添加标签
    static void addOsd(AbstractVideoWidget *widget, int &index, bool supportImage);
    static void addOsd2(AbstractVideoWidget *widget, int count);
    //演示添加图形
    static void addGraph(AbstractVideoWidget *widget, int &index);
    static void addGraph2(AbstractVideoWidget *widget, int count);
};

#endif // WIDGETHELPER_H
