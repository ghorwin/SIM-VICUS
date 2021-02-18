#include "SVDBScheduleEditWidget.h"
#include "ui_SVDBScheduleEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"

#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <VICUS_Schedule.h>

#include <QtExt_LanguageHandler.h>

#include "SVDBScheduleTableModel.h"

/*
SVDBScheduleEditWidget::SVDBScheduleEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBScheduleEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	m_ui->lineEditSolarAbsorptionCoefficient->setup(0, 1, tr("Solar Absorption (short wave)"), true, true);
	m_ui->lineEditLongWaveEmissivity->setup(0, 1, tr("Thermal Absorption (long wave)"), true, true);
	m_ui->lineEditHeatTransferCoefficient->setup(0.001, 500, tr("Thermal conductivity"), true, true);

	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(true);
	for (unsigned int i=0; i <= NANDRAD::InterfaceHeatConduction::NUM_MT; ++i)
		m_ui->comboBoxHeatTransferCoeffModelType->addItem(QString("%1 [%2]")
			.arg(NANDRAD::KeywordListQt::Description("InterfaceHeatConduction::modelType_t", (int)i))
			.arg(NANDRAD::KeywordListQt::Keyword("InterfaceHeatConduction::modelType_t", (int)i)), i);
	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(false);

	m_ui->comboBoxLWModelType->blockSignals(true);
	for(unsigned int i=0; i <= NANDRAD::InterfaceLongWaveEmission::NUM_MT; ++i)
		m_ui->comboBoxLWModelType->addItem(QString("%1 [%2]")
			.arg(NANDRAD::KeywordListQt::Description("InterfaceLongWaveEmission::modelType_t", (int)i))
			.arg(NANDRAD::KeywordListQt::Keyword("InterfaceLongWaveEmission::modelType_t", (int)i)), i);
	m_ui->comboBoxLWModelType->blockSignals(false);

	m_ui->comboBoxSWModelType->blockSignals(true);
	for(unsigned int i=0; i <= NANDRAD::InterfaceSolarAbsorption::NUM_MT; ++i)
		m_ui->comboBoxSWModelType->addItem(QString("%1 [%2]")
			.arg(NANDRAD::KeywordListQt::Description("InterfaceSolarAbsorption::modelType_t", (int)i))
			.arg(NANDRAD::KeywordListQt::Keyword("InterfaceSolarAbsorption::modelType_t", (int)i)), i);
	m_ui->comboBoxSWModelType->blockSignals(false);


	// initial state is "nothing selected"
	updateInput(-1);
}*/


SVDBScheduleEditWidget::SVDBScheduleEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBScheduleEditWidget)
{
	m_ui->setupUi(this);

	// initial state is "nothing selected"
	updateInput(-1);
}

SVDBScheduleEditWidget::~SVDBScheduleEditWidget()
{
	delete m_ui;
}

void SVDBScheduleEditWidget::setup(SVDatabase * db, SVDBScheduleTableModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}



void SVDBScheduleEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;
/*
	m_ui->lineEditName->setEnabled(isEnabled);
	m_ui->pushButtonColor->setEnabled(isEnabled);
	m_ui->labelDisplayName->setEnabled(isEnabled);
	// disable all the group boxes - this disables all their subwidgets as well
	m_ui->groupBoxHeatTransfer->setEnabled(isEnabled);
	m_ui->groupBoxLongWaveExchange->setEnabled(isEnabled);
	m_ui->groupBoxShortWaveRad->setEnabled(isEnabled);

	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditSolarAbsorptionCoefficient->setText("");
		m_ui->lineEditLongWaveEmissivity->setText("");
		m_ui->lineEditHeatTransferCoefficient->setText("");

		return;
	}

	VICUS::BoundaryCondition * bc = const_cast<VICUS::BoundaryCondition *>(m_db->m_boundaryConditions[(unsigned int)id]);
	m_current = bc;

	// now update the GUI controls
	m_ui->lineEditName->setString(bc->m_displayName);

	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(true);
	m_ui->comboBoxHeatTransferCoeffModelType->setCurrentIndex(m_ui->comboBoxHeatTransferCoeffModelType->findData(bc->m_heatConduction.m_modelType));
	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(false);
	// update model-specific input states
	on_comboBoxHeatTransferCoeffModelType_currentIndexChanged(m_ui->comboBoxHeatTransferCoeffModelType->currentIndex());

	m_ui->comboBoxLWModelType->blockSignals(true);
	m_ui->comboBoxLWModelType->setCurrentIndex(m_ui->comboBoxLWModelType->findData(bc->m_longWaveEmission.m_modelType));
	m_ui->comboBoxLWModelType->blockSignals(false);
	// update model-specific input states
	on_comboBoxLWModelType_currentIndexChanged(m_ui->comboBoxLWModelType->currentIndex());

	m_ui->comboBoxSWModelType->blockSignals(true);
	m_ui->comboBoxSWModelType->setCurrentIndex(m_ui->comboBoxSWModelType->findData(bc->m_solarAbsorption.m_modelType));
	m_ui->comboBoxSWModelType->blockSignals(false);
	// update model-specific input states
	on_comboBoxSWModelType_currentIndexChanged(m_ui->comboBoxSWModelType->currentIndex());

	m_ui->lineEditHeatTransferCoefficient->setValue(bc->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value);
	m_ui->lineEditSolarAbsorptionCoefficient->setValue(bc->m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].value);
	m_ui->lineEditLongWaveEmissivity->setValue(bc->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value);

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (bc->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
	m_ui->lineEditSolarAbsorptionCoefficient->setReadOnly(!isEditable);
	m_ui->lineEditLongWaveEmissivity->setReadOnly(!isEditable);
	m_ui->lineEditHeatTransferCoefficient->setReadOnly(!isEditable);
	m_ui->comboBoxHeatTransferCoeffModelType->setEnabled(isEditable);
	m_ui->comboBoxLWModelType->setEnabled(isEditable);
	m_ui->comboBoxSWModelType->setEnabled(isEditable);
*/
}

void SVDBScheduleEditWidget::on_toolButtonAddPeriod_clicked(){
	Q_ASSERT(m_current != nullptr);

}
