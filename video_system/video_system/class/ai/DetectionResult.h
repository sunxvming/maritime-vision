#pragma once
#include <QString>
#include <QRectF>
#include <QList>
#include <QDateTime>

struct BoundingBox {
    float x;       // x_center normalized [0,1]
    float y;       // y_center normalized [0,1]
    float width;   // normalized [0,1]
    float height;  // normalized [0,1]

    // Convert to pixel rect within the displayed image area (imageRect)
    QRect toRect(const QRect &imageRect) const {
        float cx = imageRect.x() + x * imageRect.width();
        float cy = imageRect.y() + y * imageRect.height();
        float w  = width  * imageRect.width();
        float h  = height * imageRect.height();
        return QRect(qRound(cx - w / 2), qRound(cy - h / 2), qRound(w), qRound(h));
    }
};

struct Detection {
    int     trackId;
    QString label;
    float   confidence;
    BoundingBox bbox;
    QString alarmType;
};

struct DetectionResult {
    QString          cameraId;
    qint64           timestamp;
    QList<Detection> detections;
};


struct AlarmEvent {
    QString id;
    QString cameraId;
    QString cameraName;
    QString alarmType;
    QString description;
    QDateTime timestamp;
    int trackId = -1;
    double confidence = 0.0;
    bool hasBbox = false;
    float bbox_x = 0.0f;
    float bbox_y = 0.0f;
    float bbox_w = 0.0f;
    float bbox_h = 0.0f;
    QString screenshotPath;
    bool acknowledged = false;
    int algorithmId = -1;
    QString voiceText;
};
