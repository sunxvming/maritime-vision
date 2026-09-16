#ifndef QUISTYLE_H
#define QUISTYLE_H

#include <QObject>
class QLabel;

class QUIStyle
{
public:
    //样式枚举
    enum Style {
        Style_BlackVideo = 0,   //视频黑样式
        Style_FlatGray = 1     //扁平灰样式
    };

    //获取是否是加深样式
    static bool isDark1(const QString &styleName);
    static bool isDark2(const QString &styleName);

    //获取皮肤样式中文名称和对应的样式表文件
    static void getStyle(QStringList &styleNames, QStringList &styleFiles);
    //设置全局样式
    static void setStyle(const QString &qss);
    static void setStyle(const QUIStyle::Style &style);
    static void setStyleFile(const QString &qssFile);
    static void setStyleFile(const QString &qssFile, QString &paletteColor, QString &textColor);
    static void setStyleFile(const QString &qssFile, QString &textColor,
                             QString &panelColor, QString &borderColor,
                             QString &normalColorStart, QString &normalColorEnd,
                             QString &darkColorStart, QString &darkColorEnd,
                             QString &highColor);

    //根据QSS样式获取对应颜色值
    static void getQssColor(const QString &qss, const QString &flag, QString &color);
    static void getQssColor(const QString &qss, QString &textColor,
                            QString &panelColor, QString &borderColor,
                            QString &normalColorStart, QString &normalColorEnd,
                            QString &darkColorStart, QString &darkColorEnd,
                            QString &highColor);

    //设置标签颜色
    static void setLabStyle(QLabel *lab, quint8 type, const QString &bgColor = "", const QString &textColor = "");
};

#endif // QUISTYLE_H
