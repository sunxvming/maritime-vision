#include "frmvideoplaylocal.h"
#include "ui_frmvideoplaylocal.h"
#include "quihelper.h"
#include "dbquery.h"
#include "devicehelper.h"
#include "videowidgetx.h"

frmVideoPlayLocal::frmVideoPlayLocal(QWidget *parent) : QWidget(parent), ui(new Ui::frmVideoPlayLocal)
{
    ui->setupUi(this);
    this->initForm();
    this->initIcon();
    this->initVideo();
    on_btnSelect_clicked();
}

frmVideoPlayLocal::~frmVideoPlayLocal()
{
    delete ui;
}

VideoWidget *frmVideoPlayLocal::getVideoWidget()
{
    return this->videoWidget;
}

void frmVideoPlayLocal::initForm()
{
    //存储规则约定
    //1. 默认存储主目录 video_normal
    //2. 主目录下按照日期目录存放(2025-10-01/2025-10-31)
    //3. 日期目录下是单个视频文件(ch01_2021-04-07-14-08-11.mp4/ch02_2021-04-07-14-08-11.mp4)
    //4. 拓展功能可以存储对应的数据文件比如经纬度数据和视频文件一个目录(名称一样并且拓展名可以是txt)

    ui->widgetRight->setFixedWidth(AppData::RightWidth);

    ui->cboxCh->addItem("所有通道");
    for (int i = 1; i <= AppConfig::VideoCount; ++i) {
        ui->cboxCh->addItem(QString("通道%1").arg(i, 2, 10, QChar('0')));
    }

    ui->cboxType->addItem("存储视频");

    ui->dateStart->calendarWidget()->setLocale(QLocale::Chinese);
    ui->dateEnd->calendarWidget()->setLocale(QLocale::Chinese);
    ui->dateStart->setDate(QDate::currentDate().addDays(-20));
    ui->dateEnd->setDate(QDate::currentDate().addDays(1));

    isStop = false;
    iconSize = AppData::BtnMinWidth / 2.5;
    doubleClickTime = QDateTime::currentDateTime();    
    ui->labTip->setText(QString("共找到 %1 个").arg(0));

    ui->cboxSpeed->addItem("0.5 倍速", "0.5");
    ui->cboxSpeed->addItem("1.0 倍速", "1.0");
    ui->cboxSpeed->addItem("2.0 倍速", "2.0");
    ui->cboxSpeed->addItem("4.0 倍速", "4.0");
    ui->cboxSpeed->addItem("8.0 倍速", "8.0");
    ui->cboxSpeed->setCurrentIndex(1);

    //设置图形字体图标
    IconHelper::setIcon(ui->btnPlay, 0xeb12, iconSize);
    IconHelper::setIcon(ui->btnStop, 0xeb13, iconSize - 5);
    IconHelper::setIcon(ui->btnMute, 0xe674, iconSize - 10);

    //关联样式改变信号自动重新设置图标
    connect(AppEvent::Instance(), SIGNAL(changeStyle()), this, SLOT(initIcon()));
}

void frmVideoPlayLocal::initIcon()
{
    //设置按钮图标
    CommonNav::setIconBtn(ui->frameRight);
}

void frmVideoPlayLocal::initVideo()
{
    videoWidget = new VideoWidget;
    DeviceHelper::initVideoWidget(videoWidget);
    //设置背景文字
    videoWidget->setBgText("视频文件");
    //加入到布局
    ui->gridLayout->addWidget(videoWidget, 0, 0);
}

void frmVideoPlayLocal::addItem(const QString &text, const QString &data)
{
    QListWidgetItem *item = new QListWidgetItem;
    item->setText(text);
    item->setData(Qt::UserRole, QString(data));
    item->setCheckState(Qt::Unchecked);
    ui->listWidget->addItem(item);
}

void frmVideoPlayLocal::receivePlayStart(int time)
{    
    IconHelper::setIcon(ui->btnPlay, 0xeb11, iconSize);
    IconHelper::setIcon(ui->btnMute, 0xe674, iconSize - 10);

    //设置播放速度
    on_cboxSpeed_currentIndexChanged(ui->cboxSpeed->currentIndex());
}

void frmVideoPlayLocal::receivePlayFinsh()
{
    IconHelper::setIcon(ui->btnPlay, 0xeb12, iconSize);
    IconHelper::setIcon(ui->btnMute, 0xe674, iconSize - 10);

    ui->labDuration->setText("00:00");
    ui->labPosition->setText("00:00");
    ui->sliderPosition->setValue(0);

    //自动切换到下一个视频并执行模拟双击事件
    if (!isStop && ui->listWidget->currentRow() < ui->listWidget->count() - 1) {
        QDateTime now = QDateTime::currentDateTime();
        if (doubleClickTime.msecsTo(now) > 500) {
            ui->listWidget->setCurrentRow(ui->listWidget->currentRow() + 1);
            on_listWidget_doubleClicked();
        }
    }
}

void frmVideoPlayLocal::receiveDuration(qint64 duration)
{
    //将整数分钟附近的相差1s的全部换成整数分钟 579000=580000 581000=580000
    int len = duration;
#if 1
    if ((len + 1000) % 10000 == 0) {
        len = len + 1000;
    } else if ((len - 1000) % 10000 == 0) {
        len = len - 1000;
    }
#endif

    //设置进度条最大进度以及总时长
    ui->sliderPosition->setValue(0);
    ui->sliderPosition->setMaximum(duration);
    ui->labDuration->setText(QUIHelper::getTimeString(len));
}

void frmVideoPlayLocal::receivePosition(qint64 position)
{
    //设置当前进度及已播放时长
    ui->sliderPosition->setValue(position);
    ui->labPosition->setText(QUIHelper::getTimeString(position));
    ui->labDuration->update();
}

QStringList frmVideoPlayLocal::getSelectFile(bool checked)
{
    //拿到所有勾选的文件集合
    QStringList fileNames;
    int count = ui->listWidget->count();
    for (int row = 0; row < count; row++) {
        QListWidgetItem *item = ui->listWidget->item(row);
        if (checked) {
            if (item->checkState() == Qt::Checked) {
                fileNames << item->data(Qt::UserRole).toString();
            }
        } else {
            fileNames << item->data(Qt::UserRole).toString();
        }
    }

    return fileNames;
}

void frmVideoPlayLocal::on_btnSelect_clicked()
{
    QDate dateStart = ui->dateStart->date();
    QDate dateEnd = ui->dateEnd->date();
    if (dateStart > dateEnd) {
        QUIHelper::showMessageBoxError("开始时间不能大于结束时间!", 3);
        return;
    }

    //将日期转换为日期时间计算相差的天数,超过最大天数则提示不用继续
    if (dateStart.daysTo(dateEnd) >= 60) {
        QUIHelper::showMessageBoxError("每次最大只能查询60天内!", 3);
        return;
    }

    ui->listWidget->clear();
    QString filePath;
    if (ui->cboxType->currentText() == "存储视频") {
        filePath = AppData::VideoNormalPath;
    } else {
        filePath = AppData::VideoAlarmPath;
    }

    //获取所有文件夹名称,根据时间查询对应通道对应类型视频
    //如果开始时间小于或者等于结束时间,则将开始时间对应文件夹下的视频文件添加到列表
    //然后将开始时间加一天,直到大于结束时间
    while (dateStart <= dateEnd) {
        //生成对应日期的文件夹
        QString savePath = QString("%1/%2").arg(filePath).arg(dateStart.toString("yyyy-MM-dd"));
        QDir saveDir(savePath);
        //判断文件夹是否存在
        if (saveDir.exists()) {
            //指定文件拓展名过滤,按照时间升序排序
            QStringList filter;
            filter << "*.mp4" << "*.h264" << "*.avi" << "*.mkv";
            QStringList fileNames = saveDir.entryList(filter, QDir::NoFilter, QDir::Time | QDir::Reversed);

            foreach (QString fileName, fileNames) {
                //过滤不符合要求的命名的文件
                if (!fileName.startsWith("ch")) {
                    continue;
                }

                //过滤太小的文件
                QString fullName = savePath + "/" + fileName;
                qint64 size = QFile(fullName).size();
                if (size < (1 * 1024)) {
                    continue;
                }

                //如果是选择的所有通道,则不过滤视频文件
                if (ui->cboxCh->currentText() == "所有通道") {
                    addItem(fileName, fullName);
                } else {
                    //对应通道的视频文件添加进来
                    QString chVideo = fileName.split("_").first();
                    QString chName = QString("ch%1").arg(ui->cboxCh->currentIndex(), 2, 10, QChar('0'));
                    if (chVideo == chName) {
                        addItem(fileName, fullName);
                    }
                }
            }
        }

        dateStart = dateStart.addDays(1);
    }

    ui->labTip->setText(QString("共找到 %1 个").arg(ui->listWidget->count()));
}

void frmVideoPlayLocal::on_btnOpenDir_clicked()
{
    // 打开保存的文件夹，而不是下载
    QString filePath = AppData::VideoNormalPath;
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));

}

void frmVideoPlayLocal::on_btnDelete_clicked()
{
    QStringList fileNames = getSelectFile(true);
    if (fileNames.size() == 0) {
        QUIHelper::showMessageBoxError("没有选中任何文件!", 3);
        return;
    }

    if (QUIHelper::showMessageBoxQuestion("确定要删除选中文件吗? 删除后不能恢复!") != QMessageBox::Yes) {
        return;
    }

    //挨个删除文件
    foreach (QString fileName, fileNames) {
        QFile::remove(fileName);
    }

    DbQuery::addUserLog("删除视频文件");
    QUIHelper::showMessageBoxInfo("删除视频文件完成!", 3);
    //立即重新查询
    on_btnSelect_clicked();
}

void frmVideoPlayLocal::on_btnClear_clicked()
{
    QStringList fileNames = getSelectFile(false);
    if (fileNames.size() == 0) {
        QUIHelper::showMessageBoxError("没有任何文件!", 3);
        return;
    }

    if (QUIHelper::showMessageBoxQuestion("确定要清空所有文件吗? 清空后不可恢复!") != QMessageBox::Yes) {
        return;
    }

    //挨个删除文件
    foreach (QString fileName, fileNames) {
        QFile::remove(fileName);
    }

    DbQuery::addUserLog("清空视频文件");
    QUIHelper::showMessageBoxInfo("清空视频文件完成!", 3);
    //立即重新查询
    on_btnSelect_clicked();
}

void frmVideoPlayLocal::on_btnPlay_clicked()
{
    //运行阶段取真实值
    static bool pause = true;
    if (videoWidget->getIsRunning()) {
        pause = videoWidget->getIsPause();
    } else {
        //不在运行阶段并且选中了则主动触发双击
        if (ui->listWidget->currentRow() >= 0) {
            on_listWidget_doubleClicked();
            return;
        }
    }

    if (pause) {
        videoWidget->next();
        IconHelper::setIcon(ui->btnPlay, 0xeb11, iconSize);
    } else {
        videoWidget->pause();
        IconHelper::setIcon(ui->btnPlay, 0xeb12, iconSize);
    }

    pause = !pause;
}

void frmVideoPlayLocal::on_btnStop_clicked()
{
    isStop = true;
    videoWidget->stop();
}

void frmVideoPlayLocal::on_btnMute_clicked()
{
    //运行阶段取真实值
    static bool muted = false;
    if (videoWidget->getIsRunning()) {
        muted = videoWidget->getMuted();
    }

    if (muted) {
        videoWidget->setMuted(false);
        IconHelper::setIcon(ui->btnMute, 0xe674, iconSize - 10);
    } else {
        videoWidget->setMuted(true);
        IconHelper::setIcon(ui->btnMute, 0xe667, iconSize - 10);
    }

    muted = !muted;
}

void frmVideoPlayLocal::on_listWidget_doubleClicked()
{
    isStop = false;
    doubleClickTime = QDateTime::currentDateTime();
    QListWidgetItem *item = ui->listWidget->currentItem();
    QString fullName = item->data(Qt::UserRole).toString();

    //设置图片拉伸策略
    videoWidget->setScaleMode(ScaleMode_Auto);

    //关联信号槽并打开视频
    videoWidget->open(fullName);
    VideoThread *thread = videoWidget->getVideoThread();
    connect(thread, SIGNAL(receivePlayStart(int)), this, SLOT(receivePlayStart(int)));
    connect(thread, SIGNAL(receivePlayFinsh()), this, SLOT(receivePlayFinsh()));
    connect(thread, SIGNAL(receiveDuration(qint64)), this, SLOT(receiveDuration(qint64)));
    connect(thread, SIGNAL(receivePosition(qint64)), this, SLOT(receivePosition(qint64)));
}

void frmVideoPlayLocal::on_listWidget_itemPressed(QListWidgetItem *item)
{
    //按下的时候自动切换复选框状态
    bool checked = (item->checkState() == Qt::Checked);
    item->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
}

void frmVideoPlayLocal::on_sliderPosition_clicked()
{
    videoWidget->setPosition(ui->sliderPosition->value());
}

void frmVideoPlayLocal::on_sliderPosition_sliderMoved(int value)
{
    videoWidget->setPosition(value);
}

void frmVideoPlayLocal::on_cboxSpeed_currentIndexChanged(int index)
{
    if (isVisible()) {
        double speed = ui->cboxSpeed->itemData(index).toDouble();
        videoWidget->setSpeed(speed);
    }
}
