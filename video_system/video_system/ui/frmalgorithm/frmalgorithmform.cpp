#include "frmalgorithmform.h"
#include "ui_frmalgorithmform.h"
#include "quihelper.h"
#include "ApiClient.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>

frmAlgorithmForm::frmAlgorithmForm(QWidget *parent) : QWidget(parent), ui(new Ui::frmAlgorithmForm)
{
    ui->setupUi(this);
    m_currentId = 0;
    m_mode = "add";
    this->initForm();
}

frmAlgorithmForm::~frmAlgorithmForm()
{
    delete ui;
}

void frmAlgorithmForm::initForm()
{
    connect(ui->btnBack, SIGNAL(clicked()), this, SLOT(on_btnBack_clicked()));
    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(on_btnSave_clicked()));

    // Confidence threshold range
    ui->spinConfidence->setRange(0.0, 1.0);
    ui->spinConfidence->setSingleStep(0.01);
    ui->spinConfidence->setDecimals(2);

    // Risk level options
    ui->cboxRiskLevel->addItems(QStringList() << "高" << "中" << "低");

    // Alarm parameter ranges
    ui->spinAlarmCooldown->setRange(0, 3600);
    ui->spinAlarmWindow->setRange(0, 3600);
    ui->spinAlarmThreshold->setRange(0, 10000);
}

void frmAlgorithmForm::loadAlgorithm(const QString &mode, int id)
{
    m_mode = mode;
    m_currentId = id;

    ui->txtNameCn->clear();
    ui->txtNameEn->clear();
    ui->txtDescription->clear();
    ui->txtModelPath->clear();
    ui->txtBmodelPath->clear();
    ui->txtBmodelClassNames->clear();
    ui->spinConfidence->setValue(0.45);
    ui->txtLabelMap->clear();
    ui->cboxRiskLevel->setCurrentIndex(0);
    ui->spinAlarmCooldown->setValue(30);
    ui->spinAlarmWindow->setValue(15);
    ui->spinAlarmThreshold->setValue(15);
    ui->txtVoiceText->clear();
    ui->chkEnabled->setChecked(true);

    if (mode == "add") {
        ui->labTitle->setText("算法管理 - 新增");
        setReadOnly(false);
        ui->btnSave->setVisible(true);
        return;
    }

    if (mode == "edit") {
        ui->labTitle->setText("算法管理 - 编辑");
    } else {
        ui->labTitle->setText("算法管理 - 查看");
    }

    ApiClient::instance()->get(
        QString("/api/v1/algorithms/%1").arg(id),
        [this, mode](const QJsonObject &data) {
            fillForm(data);
            setReadOnly(mode == "view");
            ui->btnSave->setVisible(mode != "view");
        },
        [this](const QString &err) {
            qDebug() << TIMEMS << "loadAlgorithm error:" << err;
            QUIHelper::showMessageBoxError("加载算法数据失败！", 3);
        }
    );
}


void frmAlgorithmForm::fillForm(const QJsonObject &obj)
{
    ui->txtNameCn->setText(obj["name_cn"].toString());
    ui->txtNameEn->setText(obj["name_en"].toString());
    ui->txtDescription->setText(obj["description"].toString());
    ui->txtModelPath->setText(obj["model_path"].toString());
    ui->txtBmodelPath->setText(obj["bmodel_path"].toString());
    // bmodel_class_names: convert object to formatted JSON string
    QJsonValue bmodelClassNamesVal = obj["bmodel_class_names"];
    if (bmodelClassNamesVal.isObject()) {
        QJsonDocument doc(bmodelClassNamesVal.toObject());
        ui->txtBmodelClassNames->setPlainText(doc.toJson(QJsonDocument::Indented));
    } else if (bmodelClassNamesVal.isString()) {
        ui->txtBmodelClassNames->setPlainText(bmodelClassNamesVal.toString());
    }
    ui->spinConfidence->setValue(obj["confidence_threshold"].toDouble(0.45));

    // label_map: convert object to formatted JSON string
    QJsonValue labelMapVal = obj["label_map"];
    if (labelMapVal.isObject()) {
        QJsonDocument doc(labelMapVal.toObject());
        ui->txtLabelMap->setPlainText(doc.toJson(QJsonDocument::Indented));
    } else if (labelMapVal.isString()) {
        ui->txtLabelMap->setPlainText(labelMapVal.toString());
    }

    // Risk level
    QString riskLevel = obj["risk_level"].toString("高");
    int riskIdx = ui->cboxRiskLevel->findText(riskLevel);
    if (riskIdx >= 0) ui->cboxRiskLevel->setCurrentIndex(riskIdx);

    ui->spinAlarmCooldown->setValue(obj["alarm_cooldown"].toInt(30));
    ui->spinAlarmWindow->setValue(obj["alarm_window"].toInt(15));
    ui->spinAlarmThreshold->setValue(obj["alarm_threshold"].toInt(15));
    ui->txtVoiceText->setText(obj["voice_text"].toString());
    ui->chkEnabled->setChecked(obj["enabled"].toInt(1) != 0);
}

void frmAlgorithmForm::setReadOnly(bool readOnly)
{
    ui->txtNameCn->setReadOnly(readOnly);
    ui->txtNameEn->setReadOnly(readOnly);
    ui->txtDescription->setReadOnly(readOnly);
    ui->txtModelPath->setReadOnly(readOnly);
    ui->txtBmodelPath->setReadOnly(readOnly);
    ui->txtBmodelClassNames->setReadOnly(readOnly);
    ui->spinConfidence->setReadOnly(readOnly);
    ui->txtLabelMap->setReadOnly(readOnly);
    ui->cboxRiskLevel->setEnabled(!readOnly);
    ui->spinAlarmCooldown->setReadOnly(readOnly);
    ui->spinAlarmWindow->setReadOnly(readOnly);
    ui->spinAlarmThreshold->setReadOnly(readOnly);
    ui->txtVoiceText->setReadOnly(readOnly);
    ui->chkEnabled->setEnabled(!readOnly);
}

void frmAlgorithmForm::on_btnBack_clicked()
{
    emit backToList();
}

void frmAlgorithmForm::on_btnSave_clicked()
{
    if (ui->txtNameCn->text().trimmed().isEmpty()) {
        QUIHelper::showMessageBoxError("请填写中文名称！", 3);
        return;
    }
    if (ui->txtNameEn->text().trimmed().isEmpty()) {
        QUIHelper::showMessageBoxError("请填写英文名称！", 3);
        return;
    }
    if (ui->txtModelPath->text().trimmed().isEmpty()) {
        QUIHelper::showMessageBoxError("请填写模型路径！", 3);
        return;
    }

    QJsonObject bmodelClassNames;
    QString bmodelClassNamesText = ui->txtBmodelClassNames->toPlainText().trimmed();
    if (!bmodelClassNamesText.isEmpty()) {
        QJsonDocument bcnDoc = QJsonDocument::fromJson(bmodelClassNamesText.toUtf8());
        if (bcnDoc.isNull() || !bcnDoc.isObject()) {
            QUIHelper::showMessageBoxError("BModel类名格式不正确，请输入有效的JSON对象！", 3);
            return;
        }
        bmodelClassNames = bcnDoc.object();
    }

    QJsonObject labelMap;
    QString labelMapText = ui->txtLabelMap->toPlainText().trimmed();
    if (!labelMapText.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(labelMapText.toUtf8());
        if (doc.isNull() || !doc.isObject()) {
            QUIHelper::showMessageBoxError("标签映射格式不正确，请输入有效的JSON对象！", 3);
            return;
        }
        labelMap = doc.object();
    }

    QJsonObject body;
    body["name_cn"]               = ui->txtNameCn->text().trimmed();
    body["name_en"]               = ui->txtNameEn->text().trimmed();
    body["description"]           = ui->txtDescription->text().trimmed();
    body["model_path"]            = ui->txtModelPath->text().trimmed();
    body["bmodel_path"]           = ui->txtBmodelPath->text().trimmed();
    body["bmodel_class_names"]    = bmodelClassNames;
    body["confidence_threshold"]  = ui->spinConfidence->value();
    body["label_map"]             = labelMap;
    body["risk_level"]            = ui->cboxRiskLevel->currentText();
    body["alarm_cooldown"]        = ui->spinAlarmCooldown->value();
    body["alarm_window"]          = ui->spinAlarmWindow->value();
    body["alarm_threshold"]       = ui->spinAlarmThreshold->value();
    body["voice_text"]            = ui->txtVoiceText->text().trimmed();
    body["enabled"]               = ui->chkEnabled->isChecked() ? 1 : 0;

    auto onSuccess = [this](const QJsonObject &) {
        QUIHelper::showMessageBoxInfo(m_mode == "add" ? "新增算法成功！" : "修改算法成功！", 2);
        emit backToList();
    };

    auto onFailed = [this](const QString &errMsg) {
        QUIHelper::showMessageBoxError(
            QString(m_mode == "add" ? "新增失败：%1" : "修改失败：%1").arg(errMsg), 3);
    };

    if (m_mode == "add") {
        ApiClient::instance()->post("/api/v1/algorithms", body, onSuccess, onFailed);
    } else {
        ApiClient::instance()->put(
            QString("/api/v1/algorithms/%1").arg(m_currentId),
            body, onSuccess, onFailed);
    }
}
