#include "frmalarmdetail.h"
#include "ui_frmalarmdetail.h"
#include "quihelper.h"
#include "ApiClient.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QPainter>
#include <QPen>
#include <QFont>

frmAlarmDetail::frmAlarmDetail(QWidget *parent) : QWidget(parent), ui(new Ui::frmAlarmDetail)
{
    ui->setupUi(this);
    currentAlarmId = -1;
    this->initForm();
}

frmAlarmDetail::~frmAlarmDetail()
{
    delete ui;
}

void frmAlarmDetail::showEvent(QShowEvent *event)
{
    resize(1524, 814);
}

void frmAlarmDetail::initForm()
{
    connect(ui->btnBack,         SIGNAL(clicked()), this, SLOT(on_btnBack_clicked()));
    connect(ui->btnRefresh,      SIGNAL(clicked()), this, SLOT(on_btnRefresh_clicked()));
    connect(ui->btnMarkProcessed, SIGNAL(clicked()), this, SLOT(on_btnMarkProcessed_clicked()));
}

void frmAlarmDetail::loadAlarm(int alarmId)
{
    currentAlarmId = alarmId;

    ui->labIdValue->setText("-");
    ui->labCameraValue->setText("-");
    ui->labAlgoValue->setText("-");
    ui->labTimeValue->setText("-");
    ui->labRiskValue->setText("-");
    ui->labStatusValue->setText("-");
    ui->labConfValue->setText("-");
    ui->labImage->setText("加载中...");
    ui->labImage->setPixmap(QPixmap());

    ApiClient::instance()->get(
        QString("/api/v1/alarm-events/%1").arg(alarmId),
        [this](const QJsonObject &data) {
            onAlarmDataLoaded(data);
        },
        [this](const QString &err) {
            qDebug() << TIMEMS << "loadAlarm error:" << err;
            ui->labImage->setText("加载失败");
            QUIHelper::showMessageBoxError("加载报警详情失败！", 3);
        }
    );
}

void frmAlarmDetail::onAlarmDataLoaded(const QJsonObject &alarmData)
{
    ui->labIdValue->setText(QString::number(alarmData["id"].toInt()));
    ui->labCameraValue->setText(alarmData["camera_name"].toString());
    ui->labAlgoValue->setText(alarmData["algorithm_name"].toString());

    QString alarmTime = alarmData["alarm_time"].toString();
    if (alarmTime.contains('T')) {
        QDateTime dt = QDateTime::fromString(alarmTime, Qt::ISODate);
        if (dt.isValid()) {
            alarmTime = dt.toString("yyyy-MM-dd HH:mm:ss");
        }
    } else if (alarmTime.contains('.')) {
        alarmTime = alarmTime.left(19);
    }
    ui->labTimeValue->setText(alarmTime);
    ui->labRiskValue->setText(alarmData["risk_level"].toString());
    ui->labStatusValue->setText(alarmData["status"].toString());

    QJsonObject detectionInfoObj = alarmData["detection_info"].toObject();
    BoundingBox bbox;
    bbox.x1 = bbox.y1 = bbox.x2 = bbox.y2 = 0;
    bbox.confidence = 0.0;
    bbox.isNormalized = false;
    parseDetectionInfo(detectionInfoObj, bbox);

    QString screenshotPath = alarmData["screenshot_path"].toString();
    if (!screenshotPath.isEmpty()) {
        loadImageFromUrl(screenshotPath, bbox);
    } else {
        ui->labImage->setText("无截图");
    }
}

void frmAlarmDetail::parseDetectionInfo(const QJsonObject &obj, BoundingBox &bbox)
{
    if (obj.isEmpty()) return;

    bbox.label      = obj.value("label").toString();
    bbox.confidence = obj.value("confidence").toDouble(0.0);

    QString alarmType = obj.value("alarm_type").toString();
    if (bbox.label.isEmpty()) bbox.label = alarmType;


    if (bbox.confidence > 0.0)
        ui->labConfValue->setText(QString::number(bbox.confidence * 100.0, 'f', 1) + "%");

    // Check if bbox is an array (normalized [x, y, w, h]) or object ({x1, y1, x2, y2})
    QJsonValue bboxValue = obj.value("bbox");
    if (bboxValue.isArray()) {
        // Normalized coordinates [x, y, w, h] where values are 0.0-1.0
        QJsonArray bboxArray = bboxValue.toArray();
        if (bboxArray.size() >= 4) {
            bbox.norm_x = bboxArray[0].toDouble();
            bbox.norm_y = bboxArray[1].toDouble();
            bbox.norm_w = bboxArray[2].toDouble();
            bbox.norm_h = bboxArray[3].toDouble();
            bbox.isNormalized = true;


        }
    } else if (bboxValue.isObject()) {
        // Pixel coordinates {x1, y1, x2, y2}
        QJsonObject bboxObj = bboxValue.toObject();
        if (!bboxObj.isEmpty()) {
            bbox.x1 = bboxObj.value("x1").toInt(0);
            bbox.y1 = bboxObj.value("y1").toInt(0);
            bbox.x2 = bboxObj.value("x2").toInt(0);
            bbox.y2 = bboxObj.value("y2").toInt(0);
            bbox.isNormalized = false;


        }
    }
}

void frmAlarmDetail::drawBoundingBox(const QPixmap &source, const BoundingBox &bbox)
{
    if (source.isNull()) return;

    overlayImage = source.copy();
    QPainter painter(&overlayImage);

    int x = 0, y = 0, w = 0, h = 0;

    // Convert normalized [cx, cy, w, h] to pixel top-left coordinates
    if (bbox.isNormalized) {
        int imgWidth = source.width();
        int imgHeight = source.height();

        w = static_cast<int>(bbox.norm_w * imgWidth);
        h = static_cast<int>(bbox.norm_h * imgHeight);
        x = static_cast<int>(bbox.norm_x * imgWidth)  - w / 2;
        y = static_cast<int>(bbox.norm_y * imgHeight) - h / 2;
    } else {
        // Use pixel coordinates directly
        x = bbox.x1;
        y = bbox.y1;
        w = bbox.x2 - bbox.x1;
        h = bbox.y2 - bbox.y1;
    }

    // Draw bounding box if coordinates are valid
    bool hasBbox = (w > 0 && h > 0);
    if (hasBbox) {
        // Red bounding box
        QPen pen(Qt::red, 3, Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        painter.drawRect(x, y, w, h);

        // Label background
        QString labelText = bbox.label;
        if (bbox.confidence > 0.0) {
            labelText += QString(" %1%").arg(bbox.confidence * 100.0, 0, 'f', 1);
        }

        if (!labelText.isEmpty()) {
            QFont font = painter.font();
            font.setPointSize(18);
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
            painter.drawText(textX + 5, textY + textH - 10, labelText);
        }
    }

    painter.end();
    displayImage();
}

void frmAlarmDetail::displayImage()
{
    if (overlayImage.isNull()) return;

    // Scale to fit the label while maintaining aspect ratio
    QSize labelSize = ui->labImage->size();
    QPixmap scaled = overlayImage.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labImage->setPixmap(scaled);
}

void frmAlarmDetail::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // Redraw the image when window is resized
    if (!overlayImage.isNull()) {
        displayImage();
    }
}

void frmAlarmDetail::on_btnBack_clicked()
{
    emit backToList();
}

void frmAlarmDetail::on_btnRefresh_clicked()
{
    if (currentAlarmId > 0) {
        loadAlarm(currentAlarmId);
    }
}

void frmAlarmDetail::loadImageFromUrl(const QString &url, const BoundingBox &bbox)
{
    ApiClient::instance()->loadImage(
        url,
        [this, bbox](const QByteArray &imageData) {
            QPixmap px;
            if (px.loadFromData(imageData)) {
                originalImage = px;
                drawBoundingBox(originalImage, bbox);
            } else {
                ui->labImage->setText("图片格式无效");
            }
        },
        [this](const QString &err) {
            ui->labImage->setText(QString("加载失败: %1").arg(err));
        }
    );
}

void frmAlarmDetail::on_btnMarkProcessed_clicked()
{
    if (currentAlarmId <= 0) return;

    QJsonObject body;
    body["status"] = QString("已处理");

    ApiClient::instance()->put(
        QString("/api/v1/alarm-events/%1/status").arg(currentAlarmId),
        body,
        [this](const QJsonObject &) {
            ui->labStatusValue->setText("已处理");
            QUIHelper::showMessageBoxInfo("已标记为已处理", 2);
        },
        [](const QString &err) {
            qDebug() << TIMEMS << "markProcessed error:" << err;
            QUIHelper::showMessageBoxError("标记失败！", 3);
        }
    );
}

void frmAlarmDetail::loadAlarmFromEvent(const AlarmEvent &alarm)
{
    currentAlarmId = -1;

    ui->labIdValue->setText("-");
    ui->labCameraValue->setText(alarm.cameraName);

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

    ui->labAlgoValue->setText(alarmTypeCN);
    ui->labTimeValue->setText(alarm.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
    ui->labRiskValue->setText("中");
    ui->labStatusValue->setText("未处理");

    if (alarm.confidence > 0.0) {
        ui->labConfValue->setText(QString::number(alarm.confidence * 100.0, 'f', 1) + "%");
    } else {
        ui->labConfValue->setText("-");
    }

    // Load screenshot image
    if (!alarm.screenshotPath.isEmpty()) {
        BoundingBox bbox;
        bbox.x1 = bbox.y1 = bbox.x2 = bbox.y2 = 0;
        bbox.confidence = alarm.confidence;
        bbox.label = alarmTypeCN;
        bbox.isNormalized = false;

        if (alarm.hasBbox) {
            bbox.norm_x = alarm.bbox_x;
            bbox.norm_y = alarm.bbox_y;
            bbox.norm_w = alarm.bbox_w;
            bbox.norm_h = alarm.bbox_h;
            bbox.isNormalized = true;
        }

        // Load image from HTTP URL with bbox data
        QString url = QString("/%1").arg(alarm.screenshotPath);
        loadImageFromUrl(url, bbox);
    } else {
        ui->labImage->setText("无截图");
    }
}
