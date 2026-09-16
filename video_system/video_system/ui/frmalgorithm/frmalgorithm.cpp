#include "frmalgorithm.h"
#include "ui_frmalgorithm.h"
#include "frmalgorithmlist.h"
#include "frmalgorithmform.h"

frmAlgorithm::frmAlgorithm(QWidget *parent) : QWidget(parent), ui(new Ui::frmAlgorithm)
{
    ui->setupUi(this);
    this->initForm();
    this->initWidget();
}

frmAlgorithm::~frmAlgorithm()
{
    delete ui;
}

void frmAlgorithm::initForm()
{
    // nothing extra needed
}

void frmAlgorithm::initWidget()
{
    pageList = new frmAlgorithmList(this);
    pageForm = new frmAlgorithmForm(this);

    ui->stackedWidget->addWidget(pageList);  // index 0
    ui->stackedWidget->addWidget(pageForm);  // index 1

    connect(pageList, SIGNAL(showForm(QString, int)), this, SLOT(showForm(QString, int)));
    connect(pageForm, SIGNAL(backToList()),            this, SLOT(showList()));
}

void frmAlgorithm::showForm(const QString &mode, int id)
{
    pageForm->loadAlgorithm(mode, id);
    ui->stackedWidget->setCurrentIndex(1);
}

void frmAlgorithm::showList()
{
    pageList->refreshList();
    ui->stackedWidget->setCurrentIndex(0);
}
