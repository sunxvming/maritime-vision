#include "audioplot.h"

#ifdef qcustomplot
void AudioPlot::initCustomPlot(QCustomPlot *customPlot, int xMin, int xMax, int xCount, int yMin, int yMax, int yCount, int smooth)
{
    //设置拉伸填充策略
    //customPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //设置XY轴范围值
    customPlot->xAxis->setRange(xMin, xMax);
    customPlot->yAxis->setRange(yMin, yMax);

    //设置XY轴步长
#ifdef qcustomplot_v1_3
    customPlot->xAxis->setAutoTickCount(xCount);
    customPlot->yAxis->setAutoTickCount(yCount);
#else
    customPlot->xAxis->ticker()->setTickCount(xCount);
    customPlot->yAxis->ticker()->setTickCount(yCount);
#endif

    //创建画布并设置平滑策略
    QCPGraph *graph = customPlot->addGraph();
    graph->setSmooth(smooth);

    //设置线条颜色
    QPalette palette;
    QPen pen;
    pen.setWidth(1);
    pen.setColor(palette.color(QPalette::Highlight));
    graph->setPen(pen);
}

void AudioPlot::addLevelData(QCustomPlot *customPlot, qreal level, double &key, int xMax)
{
    key++;
    QCPGraph *graph = customPlot->graph(0);
    int count = graph->dataCount();

    //先取出之前的数据超过则移除最前面一个
    if (count >= xMax) {
        graph->data()->removeBefore(key - xMax);
        customPlot->xAxis->setRange(key - xMax, key);
    }

    graph->addData(key, level * 10000);
    customPlot->replot();
}

void AudioPlot::addAudioData(QCustomPlot *customPlot, const QByteArray &data, int xMax)
{
    QCPGraph *graph = customPlot->graph(0);
    int count = graph->dataCount();
    int dataCount = data.size();

    //先取出之前的数据不够则添加超过则移除
    QVector<qreal> keys, values;
    if (count < xMax) {
        for (int i = 0; i < count; ++i) {
            keys << graph->data()->at(i)->key;
            values << graph->data()->at(i)->value;
        }
    } else {
        for (int i = dataCount; i < count; ++i) {
            keys << i - dataCount;
            values << graph->data()->at(i)->value;
        }
    }

    //新来的数据加在后面保证永远有最新的数据
    qint64 pointCount = keys.size();
    for (int i = 0; i < dataCount; ++i) {
        keys << i + pointCount;
        values << (quint8)data.at(i);
    }

    graph->setData(keys, values);
    customPlot->replot();
}
#endif

#ifdef charts
void AudioPlot::initChartView(QChartView *chartView, int xMin, int xMax, int xCount, int yMin, int yMax, int yCount)
{
    //设置拉伸填充策略
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

    //创建图表
    QChart *chart = new QChart;
    chart->setTitle("音频信号");
    //隐藏图例
    chart->legend()->hide();
    //设置背景区域圆角角度
    chart->setBackgroundRoundness(0);
    //设置内边界边距
    chart->setMargins(QMargins(9, 9, 9, 9));
    //设置外边界边距
    chart->layout()->setContentsMargins(0, 0, 0, 0);

    //创建线条
    QLineSeries *lineSeries = new QLineSeries();
    chart->addSeries(lineSeries);

    //创建X轴
    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(xMin, xMax);
    axisX->setTickCount(xCount);
    axisX->setLabelFormat("%g");
    //axisX->setTitleText("采样点");
    chart->setAxisX(axisX, lineSeries);

    //创建Y轴
    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(yMin, yMax);
    axisY->setTickCount(yCount);
    axisY->setLabelFormat("%g");
    //axisY->setTitleText("采样值");
    chart->setAxisY(axisY, lineSeries);

    //设置图表
    chartView->setChart(chart);
}

void AudioPlot::addLevelData(QChartView *chartView, qreal level, double &key, int xMax)
{
    key++;
    QChart *chart = chartView->chart();
    QLineSeries *line = (QLineSeries *)chart->series().at(0);
    int count = line->count();

    //先取出之前的数据超过则移除最前面一个
    if (count >= xMax) {
        line->pointsVector().removeFirst();
        chart->axisX()->setRange(key - xMax, key);
    }

    line->append(key, level * 10000);
}

void AudioPlot::addAudioData(QChartView *chartView, const QByteArray &data, int xMax)
{
    QChart *chart = chartView->chart();
    QLineSeries *line = (QLineSeries *)chart->series().at(0);
    int count = line->count();
    int dataCount = data.size();

    //先取出之前的数据不够则添加超过则移除
    QVector<QPointF> newPoints;
    QVector<QPointF> oldPoints = line->pointsVector();
    if (count < xMax) {
        newPoints = oldPoints;
    } else {
        for (int i = dataCount; i < count; ++i) {
            newPoints << QPointF(i - dataCount, oldPoints.at(i).y());
        }
    }

    //新来的数据加在后面保证永远有最新的数据
    qint64 pointCount = newPoints.size();
    for (int i = 0; i < dataCount; ++i) {
        newPoints << QPointF(i + pointCount, (quint8)data.at(i));
    }

    //说是用这个方法速度最快
    line->replace(newPoints);
}
#endif
