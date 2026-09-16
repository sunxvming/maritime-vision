#include "frmmsglistitem.h"
#include "ui_frmmsglistitem.h"
#include "quihelper.h"
#include "ApiClient.h"

#include <QPainter>
#include <QPen>
#include <QFont>
#include <QMouseEvent>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>

frmMsgListItem::frmMsgListItem(QWidget *parent) : QWidget(parent), ui(new Ui::frmMsgListItem)
{
    ui->setupUi(this);
    this->initForm();
}

frmMsgListItem::~frmMsgListItem()
{
    delete ui;
}

void frmMsgListItem::initForm()
{
    ui->labAlarmImage->setText("加载中...");
    ui->btnStatus->setEnabled(false);

    this->setStyleSheet(
        "QWidget { background-color: #1a2332; border: 1px solid #3a4a5a; }"
        "QLabel { color: #66b3ff; background-color: transparent; border: none; }"
        "QPushButton { background-color: #4a5a6a; color: white; border: 1px solid #5a6a7a; padding: 3px 8px; }"
    );

    this->setCursor(Qt::PointingHandCursor);
}

void frmMsgListItem::setAlarmData(const AlarmEvent &alarm)
{
    m_alarmData = alarm;
    m_alarmId = alarm.id;

    ui->labDataSource->setText(alarm.cameraName);

    // Map alarm_type to Chinese display name
    QString alarmTypeCN = alarm.alarmType;
    if (alarm.alarmType == "smoking") alarmTypeCN = "吸烟检测";
    else if (alarm.alarmType == "phone_use") alarmTypeCN = "使用手机";
    else if (alarm.alarmType == "no_helmet") alarmTypeCN = "未戴安全帽";
    else if (alarm.alarmType == "no_lifejacket") alarmTypeCN = "未穿救生衣";
    else if (alarm.alarmType == "no_workwear") alarmTypeCN = "未穿工作服";
    else if (alarm.alarmType == "fire") alarmTypeCN = "明火检测";
    else if (alarm.alarmType == "smoke") alarmTypeCN = "烟雾检测";
    else if (alarm.alarmType == "fatigue") alarmTypeCN = "疲劳检测";
    else if (alarm.alarmType == "absence") alarmTypeCN = "岗位无人";

    ui->labAlarmType->setText(alarmTypeCN);
    ui->labAlarmTime->setText(alarm.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
    ui->labRiskLevel->setText("中");
    ui->btnStatus->setText("未处理");

    // Load screenshot if path is provided
    if (!alarm.screenshotPath.isEmpty()) {
        loadImageFromCamera(alarm.cameraId, alarm.screenshotPath);
    } else {
        ui->labAlarmImage->setText("无截图");
    }
}

void frmMsgListItem::loadImageFromCamera(const QString &cameraId, const QString &screenshotPath)
{
    // Build the full URL to the screenshot
    // The screenshotPath from alarm_manager.py is like: data/screenshots/1/20260803/162923_123_fire.jpg
    // The HTTP server serves /data as static files
    QString url = QString("/%1").arg(screenshotPath);

    qDebug() << "Loading alarm image from:" << url;
    qDebug() << "Screenshot path:" << screenshotPath;

    // Use QPointer to safely check if object still exists in callback
    QPointer<frmMsgListItem> self(this);

    ApiClient::instance()->loadImage(
        url,
        [self, screenshotPath](const QByteArray &imageData) {
            if (!self) return;  // Object was destroyed, abort

            qDebug() << "Image loaded successfully, size:" << imageData.size() << "bytes";
            QPixmap px;
            if (px.loadFromData(imageData)) {
                qDebug() << "Image decoded successfully, size:" << px.width() << "x" << px.height();
                self->m_originalImage = px;

                if (self->m_alarmData.hasBbox) {
                    BoundingBox bbox;
                    bbox.x = self->m_alarmData.bbox_x;
                    bbox.y = self->m_alarmData.bbox_y;
                    bbox.w = self->m_alarmData.bbox_w;
                    bbox.h = self->m_alarmData.bbox_h;
                    bbox.isNormalized = true;

                    self->drawBoundingBox(self->m_originalImage, bbox, self->m_alarmData.alarmType, self->m_alarmData.confidence);
                } else {
                    self->displayImage();
                }
            } else {
                qDebug() << "Failed to decode image data";
                if (self && self->ui) {
                    self->ui->labAlarmImage->setText("图片格式无效");
                }
            }
        },
        [self, url](const QString &err) {
            if (!self) return;  // Object was destroyed, abort

            qDebug() << "Failed to load alarm image from" << url << ":" << err;
            if (self->ui) {
                self->ui->labAlarmImage->setText(QString("加载失败"));
            }
        }
    );
}

void frmMsgListItem::drawBoundingBox(const QPixmap &source, const BoundingBox &bbox, const QString &label, double confidence)
{
    if (source.isNull()) return;

    QPixmap overlayImage = source.copy();
    QPainter painter(&overlayImage);

    int imgWidth = source.width();
    int imgHeight = source.height();

    // bbox.x/y are center coordinates (normalized YOLO format), convert to top-left
    int w = static_cast<int>(bbox.w * imgWidth);
    int h = static_cast<int>(bbox.h * imgHeight);
    int x = static_cast<int>(bbox.x * imgWidth)  - w / 2;
    int y = static_cast<int>(bbox.y * imgHeight) - h / 2;

    if (w > 0 && h > 0) {
        QPen pen(Qt::red, 3, Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        painter.drawRect(x, y, w, h);

        QString labelText = label;
        if (confidence > 0.0) {
            labelText += QString(" %1%").arg(confidence * 100.0, 0, 'f', 1);
        }

        if (!labelText.isEmpty()) {
            QFont font = painter.font();
            font.setPointSize(12);
            font.setBold(true);
            painter.setFont(font);

            QFontMetrics fm(font);
            QRect textRect = fm.boundingRect(labelText);
            int textW = textRect.width() + 10;
            int textH = textRect.height() + 6;

            int textX = x;
            int textY = (y - textH >= 0) ? (y - textH) : (y + h);

            painter.fillRect(textX, textY, textW, textH, QColor(255, 0, 0, 200));
            painter.setPen(Qt::white);
            painter.drawText(textX + 5, textY + textH - 8, labelText);
        }
    }

    painter.end();
    m_originalImage = overlayImage;
    displayImage();
}

void frmMsgListItem::displayImage()
{
    if (m_originalImage.isNull()) return;

    QSize labelSize = ui->labAlarmImage->size();
    QPixmap scaled = m_originalImage.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labAlarmImage->setPixmap(scaled);
}

void frmMsgListItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit itemClicked(m_alarmId);
    }
    QWidget::mousePressEvent(event);
}

bool frmMsgListItem::eventFilter(QObject *watched, QEvent *event)
{
    return QWidget::eventFilter(watched, event);
}

