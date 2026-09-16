#ifndef FRMVIDEOWIDGET_H
#define FRMVIDEOWIDGET_H

#include <QWidget>
class VideoWidget;

namespace Ui {
class frmVideoWidget;
}

class frmVideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit frmVideoWidget(VideoWidget *videoWidget, QWidget *parent = 0);
    ~frmVideoWidget();

protected:
    //尺寸发生变化调整位置
    void resizeEvent(QResizeEvent *);

private:
    Ui::frmVideoWidget *ui;
    VideoWidget *videoWidget;

private slots:
    //音频数据振幅
    void receiveLevel(qreal leftLevel, qreal rightLevel);
};

#endif // FRMVIDEOWIDGET_H
