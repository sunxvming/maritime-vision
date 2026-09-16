#ifndef FRMMSGLISTITEM_H
#define FRMMSGLISTITEM_H

#include <QWidget>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPointer>
#include "DetectionResult.h"

namespace Ui {
class frmMsgListItem;
}

class frmMsgListItem : public QWidget
{
    Q_OBJECT

public:
    explicit frmMsgListItem(QWidget *parent = 0);
    ~frmMsgListItem();

    void setAlarmData(const AlarmEvent &alarm);
    QString getAlarmId() const { return m_alarmId; }
    AlarmEvent getAlarmData() const { return m_alarmData; }

signals:
    void itemClicked(const QString &alarmId);

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    Ui::frmMsgListItem *ui;

    QString m_alarmId;
    QPixmap m_originalImage;
    AlarmEvent m_alarmData;

    struct BoundingBox {
        float x, y, w, h;
        bool isNormalized;
    };

private slots:
    void initForm();
    void loadImageFromCamera(const QString &cameraId, const QString &timestamp);
    void drawBoundingBox(const QPixmap &source, const BoundingBox &bbox, const QString &label, double confidence);
    void displayImage();
};

#endif // FRMMSGLISTITEM_H
