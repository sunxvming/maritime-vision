#ifndef FILTERHELPER_H
#define FILTERHELPER_H

#include "widgethead.h"

class FilterHelper
{
public:
    //其他滤镜
    static QString getFilter();
    //旋转滤镜
    static QString getFilter(int rotate, bool hardware);
    //根据标签信息获取对应滤镜字符串
    static QString getFilter(const OsdInfo &osd, bool hardware);
    //根据图形信息获取对应滤镜字符串
    static QString getFilter(const GraphInfo &graph, bool hardware);
};

#endif // FILTERHELPER_H
