#ifndef FRMALARM_H
#define FRMALARM_H

#include <QWidget>

namespace Ui {
class frmAlarm;
}

class frmAlarmList;
class frmAlarmDetail;

class frmAlarm : public QWidget
{
    Q_OBJECT

public:
    explicit frmAlarm(QWidget *parent = 0);
    ~frmAlarm();

private:
    Ui::frmAlarm *ui;

    frmAlarmList   *pageList;
    frmAlarmDetail *pageDetail;

private slots:
    void initForm();
    void initWidget();
    void showDetail(int alarmId);
    void showList();
};

#endif // FRMALARM_H
