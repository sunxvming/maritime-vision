#include "frmscene.h"
#include "ui_frmscene.h"
#include "frmscenelist.h"
#include "frmsceneform.h"

frmScene::frmScene(QWidget *parent) : QWidget(parent), ui(new Ui::frmScene)
{
    ui->setupUi(this);
    this->initForm();
    this->initWidget();
}

frmScene::~frmScene()
{
    delete ui;
}

void frmScene::initForm()
{
    // Nothing extra needed
}

void frmScene::initWidget()
{
    pageList = new frmSceneList(this);
    pageForm = new frmSceneForm(this);

    ui->stackedWidget->addWidget(pageList);  // index 0
    ui->stackedWidget->addWidget(pageForm);  // index 1

    connect(pageList, SIGNAL(showForm(QString, int)), this, SLOT(showForm(QString, int)));
    connect(pageForm, SIGNAL(backToList()), this, SLOT(showList()));

    // Show list by default
    ui->stackedWidget->setCurrentIndex(0);
}

void frmScene::showForm(const QString &mode, int id)
{
    pageForm->loadScene(mode, id);
    ui->stackedWidget->setCurrentIndex(1);
}

void frmScene::showList()
{
    pageList->refreshList();
    ui->stackedWidget->setCurrentIndex(0);
}
