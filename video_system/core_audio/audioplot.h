#ifndef AUDIOPLOT_H
#define AUDIOPLOT_H

#include "head.h"

class AudioPlot
{
public:
#ifdef qcustomplot
    //初始化图表
    static void initCustomPlot(QCustomPlot *customPlot, int xMin = 0, int xMax = 100, int xCount = 11, int yMin = 0, int yMax = 10000, int yCount = 5, int smooth = 2);
    //添加振幅数据
    static void addLevelData(QCustomPlot *customPlot, qreal level, double &key, int xMax = 100);
    //添加音频数据
    static void addAudioData(QCustomPlot *customPlot, const QByteArray &data, int xMax = 8000);
#endif
#ifdef charts
    //初始化图表
    static void initChartView(QChartView *chartView, int xMin = 0, int xMax = 100, int xCount = 11, int yMin = 0, int yMax = 10000, int yCount = 5);
    //添加振幅数据
    static void addLevelData(QChartView *chartView, qreal level, double &key, int xMax = 100);
    //添加音频数据
    static void addAudioData(QChartView *chartView, const QByteArray &data, int xMax = 8000);
#endif
};

#endif // AUDIOPLOT_H
