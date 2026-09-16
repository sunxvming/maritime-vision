#include "frmvideopreview.h"
#include "ui_frmvideopreview.h"
#include "quihelper.h"
#include "devicehelper.h"
#include "videomanage.h"

SINGLETON_IMPL(frmVideoPreview)
frmVideoPreview::frmVideoPreview(QWidget *parent) : QDialog(parent), ui(new Ui::frmVideoPreview)
{
    ui->setupUi(this);
    this->initForm();
}

frmVideoPreview::~frmVideoPreview()
{
    if (timerRecord->isActive()) {
        timerRecord->stop();
    }
    delete ui;
}

void frmVideoPreview::closeEvent(QCloseEvent *)
{
    videoWidget->setParent(0);
    videoWidget->stop();
}

void frmVideoPreview::showEvent(QShowEvent *)
{
    videoWidget->setBorderWidth(0);
}

void frmVideoPreview::initForm()
{
    QUIHelper::setFramelessForm(this, ui->widgetTitle, ui->labIco, ui->btnMenu_Close);
    connect(ui->btnMenu_Close, SIGNAL(clicked()), this, SLOT(close()));
    this->setWindowTitle(ui->labTitle->text());
    this->setAttribute(Qt::WA_DeleteOnClose);

    //实例化视频控件并加入布局
    videoWidget = new VideoWidget;
    connect(videoWidget, SIGNAL(sig_receivePlayStart(int)), this, SLOT(receivePlayStart(int)));
    DeviceHelper::initVideoWidget(videoWidget);
    ui->widget->layout()->addWidget(videoWidget);

    //实例化录像定时器
    timerRecord = new QTimer(this);
    connect(timerRecord, SIGNAL(timeout()), this, SLOT(checkRecord()));
    timerRecord->setInterval(1000);
}

void frmVideoPreview::checkRecord()
{
    //到了设定的录像时间则关闭窗体(自动释放的时候会自动停止录像)
    int offset = lastTime.secsTo(QDateTime::currentDateTime());
    if (offset >= recordTime) {
        this->close();
    }

    //显示倒计时
    QString text = QString("报警视频  关闭倒计时 %1 s").arg(recordTime - offset);
    ui->labTitle->setText(text);
}

void frmVideoPreview::receivePlayStart(int time)
{
    //如果视频尺寸超过了最大尺寸则等比例缩放到合适尺寸
    VideoThread *thread = videoWidget->getVideoThread();
    int width = thread->getVideoWidth();
    int height = thread->getVideoHeight();
    if (width > 1280 || height > 720) {
        QSize size(width, height);
        size.scale(1280, 720, Qt::KeepAspectRatio);
        width = size.width();
        height = size.height();
    }

    this->setFixedSize(width + 2, height + ui->widgetTitle->height() + 2);
    QUIHelper::setFormInCenter(this);
    this->show();

    //如果设置了录像时间则启动录像
    if (recordTime > 0) {
        lastTime = QDateTime::currentDateTime();
        timerRecord->start();
        checkRecord();

        //根据地址找到通道
        QString url = videoWidget->getVideoPara().videoUrl;
        QString name = "test";
        int index = AppData::VideoUrls.indexOf(url);
        if (index >= 0) {
            name = VideoManage::Instance()->getVideoWidgets().at(index)->objectName();
        }

        QString savePath = QString("%1/video_alarm/%2").arg(QUIHelper::appPath()).arg(QDATE);
        QString fileName = QString("%1/%2_%3.mp4").arg(savePath).arg(name).arg(STRDATETIMEMS);
        videoWidget->getVideoThread()->setSaveAudioType(SaveAudioType_None);
        videoWidget->recordStart(fileName);
    }
}

void frmVideoPreview::open(const QString &url, int recordTime)
{
    //如果设置了录像时间则需要采用独立解码线程才能正常录像
    this->recordTime = recordTime;
    if (recordTime > 0) {
        videoWidget->setSharedData(false);
    }

    //设置图片拉伸策略
    videoWidget->setScaleMode(ScaleMode_Auto);
    videoWidget->open(url);
}
