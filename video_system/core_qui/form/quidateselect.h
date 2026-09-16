#ifndef QUIDATESELECT_H
#define QUIDATESELECT_H

#include <QDialog>
#include "quisingleton.h"

namespace Ui {
class QUIDateSelect;
}

class QUIDateSelect : public QDialog
{
    Q_OBJECT SINGLETON_DECL(QUIDateSelect)

public:
    explicit QUIDateSelect(QWidget *parent = 0);
    ~QUIDateSelect();

protected:
    void showEvent(QShowEvent *);

private:
    Ui::QUIDateSelect *ui;
    QString format;

private slots:
    //初始化窗体数据
    void initForm();

private slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

public:
    //获取当前选择的开始时间和结束时间
    QString getDateTimeStart()  const;
    QString getDateTimeEnd()    const;

public Q_SLOTS:
    //设置日期格式
    void setFormat(const QString &format);
};

#endif // QUIDATESELECT_H
