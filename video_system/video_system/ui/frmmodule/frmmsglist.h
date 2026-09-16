#ifndef FRMMSGLIST_H
#define FRMMSGLIST_H

#include <QWidget>
#include <QHash>
#include "DetectionResult.h"

namespace Ui {
class frmMsgList;
}

class frmMsgList : public QWidget
{
    Q_OBJECT

public:
    explicit frmMsgList(QWidget *parent = 0);
    ~frmMsgList();

protected:
    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *);

private:
    Ui::frmMsgList *ui;
    int msgListCount;

    // Algorithm voice_text cache: algorithmId -> voiceText.
    // Populated on first alarm by fetching /api/v1/algorithms once.
    QHash<int, QString> m_algoVoiceCache;
    bool m_algoCacheLoaded = false;

    void speakAlarm(const QString &cameraName, int algorithmId, const QString &fallback);
    void loadAlgoCacheThenSpeak(const QString &cameraName, int algorithmId, const QString &fallback);

private slots:
    void initForm();
    void initPanel();
    void initAction();
    void doAction();
    void checkCount();

    void onAlarmReceived(const AlarmEvent &alarm);
    void onAlarmItemClicked(const QString &alarmId);

public slots:
    void clearMsg();
    void addMsg(const QString &msg, const QString &result, const QImage &image, const QString &time);
};

#endif // FRMMSGLIST_H
