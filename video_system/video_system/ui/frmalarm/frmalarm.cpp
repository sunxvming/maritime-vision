#include "frmalarm.h"
#include "ui_frmalarm.h"
#include "frmalarmlist.h"
#include "frmalarmdetail.h"

frmAlarm::frmAlarm(QWidget *parent) : QWidget(parent), ui(new Ui::frmAlarm)
{
    ui->setupUi(this);
    this->initForm();
    this->initWidget();
}

frmAlarm::~frmAlarm()
{
    delete ui;
}

void frmAlarm::initForm()
{
    // nothing extra needed
}

void frmAlarm::initWidget()
{
    pageList   = new frmAlarmList(this);
    pageDetail = new frmAlarmDetail(this);

    ui->stackedWidget->addWidget(pageList);    // index 0
    ui->stackedWidget->addWidget(pageDetail);  // index 1

    connect(pageList,   SIGNAL(showDetail(int)), this, SLOT(showDetail(int)));
    connect(pageDetail, SIGNAL(backToList()),     this, SLOT(showList()));
}

void frmAlarm::showDetail(int alarmId)
{
    pageDetail->loadAlarm(alarmId);
    ui->stackedWidget->setCurrentIndex(1);
}

void frmAlarm::showList()
{
    ui->stackedWidget->setCurrentIndex(0);
}
