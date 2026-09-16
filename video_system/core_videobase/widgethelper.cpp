#include "widgethelper.h"

QRect WidgetHelper::getCenterRect(const QSize &imageSize, const QRect &widgetRect, int borderWidth, const ScaleMode &scaleMode)
{
    QSize newSize = imageSize;
    QSize widgetSize = widgetRect.size() - QSize(borderWidth * 1, borderWidth * 1);

    if (scaleMode == ScaleMode_Auto) {
        if (newSize.width() > widgetSize.width() || newSize.height() > widgetSize.height()) {
            newSize.scale(widgetSize, Qt::KeepAspectRatio);
        }
    } else if (scaleMode == ScaleMode_Aspect) {
        newSize.scale(widgetSize, Qt::KeepAspectRatio);
    } else {
        newSize = widgetSize;
    }

    int x = widgetRect.center().x() - newSize.width() / 2;
    int y = widgetRect.center().y() - newSize.height() / 2;
    //不是2的倍数需要偏移1像素
    x += (x % 2 == 0 ? 1 : 0);
    y += (y % 2 == 0 ? 1 : 0);
    return QRect(x, y, newSize.width(), newSize.height());
}

void WidgetHelper::getScaledImage(QImage &image, const QSize &widgetSize, const ScaleMode &scaleMode, bool fast)
{
    Qt::TransformationMode mode = fast ? Qt::FastTransformation : Qt::SmoothTransformation;
    if (scaleMode == ScaleMode_Auto) {
        if (image.width() > widgetSize.width() || image.height() > widgetSize.height()) {
            image = image.scaled(widgetSize, Qt::KeepAspectRatio, mode);
        }
    } else if (scaleMode == ScaleMode_Aspect) {
        image = image.scaled(widgetSize, Qt::KeepAspectRatio, mode);
    } else {
        image = image.scaled(widgetSize, Qt::IgnoreAspectRatio, mode);
    }
}

void WidgetHelper::drawOsd(QPainter *painter, const OsdInfo &osd, const QRect &rect)
{
    //默认位置在左上角
    int flag = Qt::AlignLeft | Qt::AlignTop;
    QPoint point = QPoint(rect.x(), rect.y());

    //绘制下区域看下效果
    //painter->setPen(Qt::red);
    //painter->drawRect(rect);

    if (osd.position == OsdPosition_LeftTop) {
        flag = Qt::AlignLeft | Qt::AlignTop;
        point = QPoint(rect.x(), rect.y());
    } else if (osd.position == OsdPosition_LeftBottom) {
        flag = Qt::AlignLeft | Qt::AlignBottom;
        point = QPoint(rect.x(), rect.height() - osd.image.height());
    } else if (osd.position == OsdPosition_RightTop) {
        flag = Qt::AlignRight | Qt::AlignTop;
        point = QPoint(rect.width() - osd.image.width(), rect.y());
    } else if (osd.position == OsdPosition_RightBottom) {
        flag = Qt::AlignRight | Qt::AlignBottom;
        point = QPoint(rect.width() - osd.image.width(), rect.height() - osd.image.height());
    } else if (osd.position == OsdPosition_Center) {
        flag = Qt::AlignVCenter | Qt::AlignHCenter;
        point = QPoint(rect.width() / 2 - osd.image.width() / 2, rect.height() / 2 - osd.image.height() / 2);
    } else if (osd.position == OsdPosition_Custom) {
        point = osd.point;
    }

    //不同的格式绘制不同的内容
    if (osd.format == OsdFormat_Image) {
        painter->drawImage(point, osd.image);
    } else {
        QString text = osd.text;
        QDateTime now = QDateTime::currentDateTime();
        if (osd.format == OsdFormat_Date) {
            text = now.toString("yyyy-MM-dd");
        } else if (osd.format == OsdFormat_Time) {
            text = now.toString("HH:mm:ss");
        } else if (osd.format == OsdFormat_DateTime) {
            text = now.toString("yyyy-MM-dd HH:mm:ss");
        }

        //设置颜色及字号
        QFont font;
        font.setPixelSize(osd.fontSize);
        font.setBold(osd.bold);
        painter->setFont(font);

        // 如果启用反色效果，绘制背景和白色文字
        if (osd.invertedColors) {
            QFontMetrics fm(font);
            QRect textRect = fm.boundingRect(text);
            int textW = textRect.width() + 8;
            int textH = textRect.height() + 4;

            QPoint bgPoint = point;
            if (osd.position == OsdPosition_Custom) {
                bgPoint = point;
            } else {
                // 对于非自定义位置，需要根据对齐方式调整背景位置
                if (osd.position == OsdPosition_LeftTop) {
                    bgPoint = QPoint(rect.x(), rect.y());
                } else if (osd.position == OsdPosition_LeftBottom) {
                    bgPoint = QPoint(rect.x(), rect.height() - textH);
                } else if (osd.position == OsdPosition_RightTop) {
                    bgPoint = QPoint(rect.width() - textW, rect.y());
                } else if (osd.position == OsdPosition_RightBottom) {
                    bgPoint = QPoint(rect.width() - textW, rect.height() - textH);
                } else if (osd.position == OsdPosition_Center) {
                    bgPoint = QPoint(rect.width() / 2 - textW / 2, rect.height() / 2 - textH / 2);
                }
            }

            // 绘制半透明背景
            QColor bgColor = osd.color;
            bgColor.setAlpha(200);
            painter->fillRect(bgPoint.x(), bgPoint.y(), textW, textH, bgColor);

            // 绘制白色文字
            painter->setPen(Qt::white);
            if (osd.position == OsdPosition_Custom) {
                painter->drawText(bgPoint.x() + 4, bgPoint.y() + textH - 6, text);
            } else {
                painter->drawText(bgPoint.x() + 4, bgPoint.y() + textH - 6, text);
            }
        } else {
            // 正常绘制，不带背景
            painter->setPen(osd.color);
            if (osd.position == OsdPosition_Custom) {
                painter->drawText(point, text);
            } else {
                painter->drawText(rect, flag, text);
            }
        }
    }
}

void WidgetHelper::drawRect(QPainter *painter, const QRect &rect, int borderWidth, QColor borderColor, bool angle)
{
    painter->setPen(QPen(borderColor, borderWidth));
    //背景颜色
    borderColor.setAlpha(50);
    //painter->setBrush(QBrush(borderColor));

    int x = rect.x();
    int y = rect.y();
    int width = rect.width();
    int height = rect.height();

    if (!angle) {
        painter->drawRect(x, y, width, height);
    } else {
        //绘制四个角
        int offset = 10;
        painter->drawLine(x, y, x + offset, y);
        painter->drawLine(x, y, x, y + offset);
        painter->drawLine(x + width - offset, y, x + width, y);
        painter->drawLine(x + width, y, x + width, y + offset);
        painter->drawLine(x, y + height - offset, x, y + height);
        painter->drawLine(x, y + height, x + offset, y + height);
        painter->drawLine(x + width - offset, y + height, x + width, y + height);
        painter->drawLine(x + width, y + height - offset, x + width, y + height);
    }
}

void WidgetHelper::drawPoints(QPainter *painter, const QList<QPoint> &pts, int borderWidth, QColor borderColor)
{
    //至少要两个点
    if (pts.size() < 2) {
        return;
    }

    painter->setPen(QPen(borderColor, borderWidth));
    //背景颜色
    borderColor.setAlpha(50);
    //painter->setBrush(QBrush(borderColor));

    //绘制多边形
    QPainterPath path;
    //先移动到起始点
    path.moveTo(pts.first());
    //逐个连接线条
    int size = pts.size();
    for (int i = 1; i < size; ++i) {
        path.lineTo(pts.at(i));
    }

    //闭合图形
    path.closeSubpath();
    painter->drawPath(path);
}

void WidgetHelper::drawPath(QPainter *painter, QPainterPath path, int borderWidth, QColor borderColor)
{
    painter->setPen(QPen(borderColor, borderWidth));
    painter->drawPath(path);
}

QLabel *WidgetHelper::showImage(QLabel *label, QWidget *widget, const QImage &image)
{
    //等比缩放下分辨率过大的图片
#ifdef Q_OS_ANDROID
    int maxWidth = widget->width();
    int maxHeight = widget->height();
#else
    int maxWidth = 1280;
    int maxHeight = 720;
#endif
    QPixmap pixmap = QPixmap::fromImage(image);
    if (pixmap.width() > maxWidth || pixmap.height() > maxHeight) {
        pixmap = pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    //实例化截图标签
    if (!label) {
        label = new QLabel;
        label->setWindowTitle("截图预览");
        label->setAlignment(Qt::AlignCenter);
    }

    //设置图片
    label->setPixmap(pixmap);

    //安卓上要用dialog窗体才正常
#ifdef Q_OS_ANDROID
    QDialog dialog;
    dialog.setWindowTitle("截图预览");
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(label);
    dialog.setLayout(layout);
    dialog.setGeometry(widget->geometry());
    dialog.exec();
    label = NULL;
#else
    label->resize(pixmap.size());
    label->raise();
    label->show();
#endif
    return label;
}

void WidgetHelper::addOsd(AbstractVideoWidget *widget, int &index, bool supportImage)
{
    index++;
    if (index > 7) {
        index = 1;
    }

    //演示添加多种标签(具体参数参见结构体)
    OsdInfo osd;
    if (index == 1) {
        //左上角文字
        osd.text = "摄像头: 测试";
        osd.fontSize = 30;
    } else if (index == 2) {
        //右上角图片或文字
        if (supportImage) {
            osd.image = QImage(":/image/bg_novideo.png");
            osd.format = OsdFormat_Image;
        } else {
            osd.text = "logo";
            osd.fontSize = 50;
            osd.color = Qt::yellow;
        }

        osd.position = OsdPosition_RightTop;
    } else if (index == 3) {
        //指定坐标显示标签
        osd.text = "QPoint(100, 100)";
        osd.fontSize = 30;
        osd.color = Qt::white;
        osd.point = QPoint(100, 100);
        osd.position = OsdPosition_Custom;
    } else if (index == 4) {
        //右下角时间
        osd.fontSize = 25;
        osd.color = Qt::green;
        osd.format = OsdFormat_Time;
        osd.position = OsdPosition_LeftBottom;
    } else if (index == 5) {
        //右下角日期时间
        osd.fontSize = 35;
        osd.color = "#A279C5";
        osd.format = OsdFormat_DateTime;
        osd.position = OsdPosition_RightBottom;
    } else if (index == 6) {
        //中间文字
        osd.text = "中间文字";
        osd.fontSize = 40;
        osd.position = OsdPosition_Center;
    }

    if (index <= 6) {
        //添加标签
        widget->appendOsd(osd);
    } else {
        //清空所有标签
        widget->clearOsd();
    }
}

void WidgetHelper::addOsd2(AbstractVideoWidget *widget, int count)
{
    int width = widget->getVideoWidth() - 100;
    int height = widget->getVideoHeight() - 100;
    static QStringList colors = QColor::colorNames();
    for (int i = 0; i < count; ++i) {
        OsdInfo osd;
        osd.fontSize = 10 + rand() % 20;
        osd.name = QString("graph%1").arg(i);
        osd.text = QString("文本%1").arg(i);
        osd.color = colors.at(rand() % colors.count());
        osd.point = QPoint(rand() % width, rand() % height);
        osd.position = OsdPosition_Custom;
        widget->setOsd(osd);
    }
}

void WidgetHelper::addGraph(AbstractVideoWidget *widget, int &index)
{
    index++;
    if (index > 5) {
        index = 1;
    }

    //演示添加多种图形(具体参数参见结构体)
    GraphInfo graph;
    if (widget->getWidgetPara().graphDrawMode == DrawMode_Source) {
        //随机生成矩形
        int width = widget->width() - 100;
        int height = widget->height() - 100;
        if (widget->getIsRunning()) {
            width = widget->getVideoWidth() - 100;
            height = widget->getVideoHeight() - 100;
        }

        int x = rand() % width;
        int y = rand() % height;
        int w = 80 + rand() % 30;
        int h = 30 + rand() % 40;
        graph.rect = QRect(x, y, w, h);
        static QStringList colors = QColor::colorNames();
        graph.borderColor = colors.at(rand() % colors.count());
    } else {
        if (index == 1) {
            //三个图形一起(矩形/点集合/路径)
            graph.rect = QRect(10, 30, 150, 100);
            graph.points = QList<QPoint>() << QPoint(200, 100) << QPoint(250, 50) << QPoint(300, 150);
            QPainterPath path;
            path.addEllipse(50, 50, 50, 50);
            graph.path = path;
        } else if (index == 2) {
            //矩形
            graph.borderColor = Qt::white;
            graph.rect = QRect(10, 160, 130, 80);
        } else if (index == 3) {
            //点集合
            graph.borderColor = Qt::green;
            graph.points = QList<QPoint>() << QPoint(300, 100) << QPoint(400, 50) << QPoint(500, 150) << QPoint(450, 200) << QPoint(350, 180);
        } else if (index == 4) {
            //路径(圆形/圆角矩形)
            graph.borderWidth = 3;
            graph.borderColor = "#22A3A9";
            QPainterPath path;
            path.addEllipse(500, 30, 100, 50);
            path.addRoundedRect(550, 100, 150, 100, 5, 5);
            graph.path = path;
        }
    }

    if (index <= 4) {
        //添加图形
        widget->appendGraph(graph);
    } else {
        //清空所有图形
        widget->clearGraph();
    }
}

void WidgetHelper::addGraph2(AbstractVideoWidget *widget, int count)
{
    int width = widget->getVideoWidth() - 100;
    int height = widget->getVideoHeight() - 100;
    static QStringList colors = QColor::colorNames();
    for (int i = 0; i < 100; ++i) {
        GraphInfo graph;
        graph.name = QString("graph%1").arg(i);
        int x = rand() % width;
        int y = rand() % height;
        int w = 30 + rand() % 30;
        int h = 10 + rand() % 40;
        graph.rect = QRect(x, y, w, h);
        graph.borderColor = colors.at(rand() % colors.count());
        widget->setGraph(graph);
    }
}
