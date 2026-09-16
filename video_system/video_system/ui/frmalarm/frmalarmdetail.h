#ifndef FRMALARMDETAIL_H
#define FRMALARMDETAIL_H

#include <QWidget>
#include <QPixmap>
#include "DetectionResult.h"

namespace Ui {
class frmAlarmDetail;
}

class frmAlarmDetail : public QWidget
{
    Q_OBJECT

public:
    explicit frmAlarmDetail(QWidget *parent = 0);
    ~frmAlarmDetail();

    void loadAlarm(int alarmId);
    void loadAlarmFromEvent(const AlarmEvent &alarm);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

signals:
    void backToList();

private:
    Ui::frmAlarmDetail *ui;

    int currentAlarmId;
    QPixmap originalImage;
    QPixmap overlayImage;

    struct BoundingBox {
        int x1, y1, x2, y2;
        QString label;
        double confidence;
        // Store normalized coordinates for later conversion
        double norm_x, norm_y, norm_w, norm_h;
        bool isNormalized;
    };

private slots:
    void initForm();
    void parseDetectionInfo(const QJsonObject &obj, BoundingBox &bbox);
    void drawBoundingBox(const QPixmap &source, const BoundingBox &bbox);
    void displayImage();
    void loadImageFromUrl(const QString &url, const BoundingBox &bbox);
    void onAlarmDataLoaded(const QJsonObject &data);

    void on_btnBack_clicked();
    void on_btnRefresh_clicked();
    void on_btnMarkProcessed_clicked();
};

#endif // FRMALARMDETAIL_H
