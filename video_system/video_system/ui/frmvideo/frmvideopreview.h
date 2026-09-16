#ifndef FRMVIDEOPREVIEW_H
#define FRMVIDEOPREVIEW_H

#include <QDialog>
#include <QDateTime>
#include "quisingleton.h"
class VideoWidget;

namespace Ui {
class frmVideoPreview;
}

class frmVideoPreview : public QDialog
{
    Q_OBJECT SINGLETON_DECL(frmVideoPreview)

public:
    explicit frmVideoPreview(QWidget *parent = 0);
    ~frmVideoPreview();

protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);

private:
    Ui::frmVideoPreview *ui;

    //视频控件
    VideoWidget *videoWidget;

    //录像时间
    int recordTime;
    //最后开始的时间
    QDateTime lastTime;
    //录像定时器
    QTimer *timerRecord;

private slots:
    //初始化界面数据
    void initForm();
    //校验录像
    void checkRecord();
    //播放成功
    void receivePlayStart(int time);

public slots:
    //打开播放地址(带录像时间)
    void open(const QString &url, int recordTime = 0);
};

#endif // FRMVIDEOPREVIEW_H
