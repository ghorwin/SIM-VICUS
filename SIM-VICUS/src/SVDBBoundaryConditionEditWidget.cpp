#include "SVDBBoundaryConditionEditWidget.h"
#include "ui_SVDBBoundaryConditionEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"

#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <QtExt_LanguageHandler.h>

#include "SVDBBoundaryConditionTableModel.h"


SVDBBoundaryConditionEditWidget::SVDBBoundaryConditionEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBBoundaryConditionEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	m_ui->lineEditSolarAbsorptionCoefficient->setup(0, 1, tr("Solar Absorption (short wave)"), true, true);
	m_ui->lineEditLongWaveEmissivity->setup(0, 1, tr("Thermal Absorption (long wave)"), true, true);
	m_ui->lineEditHeatTransferCoefficient->setup(0.001, 500, tr("Thermal conductivity"), true, true);

	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(true);
	for (unsigned int i=0; i < NANDRAD::InterfaceHeatConduction::NUM_MT; ++i)
		m_ui->comboBoxHeatTransferCoeffModelType->addItem(NANDRAD::KeywordListQt::Keyword("InterfaceHeatConduction::modelType_t", (int)i), i);
	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(false);

	m_ui->comboBoxLWModelType->blockSignals(true);
	for(unsigned int i=0; i < NANDRAD::InterfaceLongWaveEmission::NUM_MT; ++i)
		m_ui->comboBoxLWModelType->addItem(NANDRAD::KeywordListQt::Keyword("InterfaceLongWaveEmission::modelType_t", (int)i), i);
	m_ui->comboBoxLWModelType->blockSignals(false);

	m_ui->comboBoxSWModelType->blockSignals(true);
	for(unsigned int i=0; i < NANDRAD::InterfaceSolarAbsorption::NUM_MT; ++i)
		m_ui->comboBoxSWModelType->addItem(NANDRAD::KeywordListQt::Keyword("InterfaceSolarAbsorption::modelType_t", (int)i), i);
	m_ui->comboBoxSWModelType->blockSignals(false);


	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBBoundaryConditionEditWidget::~SVDBBoundaryConditionEditWidget() {
	delete m_ui;
}


void SVDBBoundaryConditionEditWidget::setup(SVDatabase * db, SVDBBoundaryConditionTableModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBBoundaryConditionEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;

	m_ui->lineEditName->setEnabled(isEnabled);
	m_ui->pushButtonColor->setEnabled(isEnabled);
	m_ui->labelDisplayName->setEnabled(isEnabled);
	// disable all the group boxes - this disables all their subwidgets as well
	m_ui->groupBox->setEnabled(isEnabled);
	m_ui->groupBox_2->setEnabled(isEnabled);
	m_ui->groupBox_3->setEnabled(isEnabled);

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

	m_ui->lineEditHeatTransferCoefficient->setValue(bc->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value);
	m_ui->lineEditSolarAbsorptionCoefficient->setValue(bc->m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].value);
	m_ui->lineEditLongWaveEmissivity->setValue(bc->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value);
	//ToDo wie greife ich auf ein item zu das im

	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(true);
	m_ui->comboBoxHeatTransferCoeffModelType->setCurrentIndex(m_ui->comboBoxHeatTransferCoeffModelType->findData(bc->m_heatConduction.m_modelType));
	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(false);

	m_ui->comboBoxLWModelType->blockSignals(true);
	m_ui->comboBoxLWModelType->setCurrentIndex(m_ui->comboBoxLWModelType->findData(bc->m_longWaveEmission.m_modelType));
	m_ui->comboBoxLWModelType->blockSignals(false);

	m_ui->comboBoxSWModelType->blockSignals(true);
	m_ui->comboBoxSWModelType->setCurrentIndex(m_ui->comboBoxSWModelType->findData(bc->m_solarAbsorption.m_modelType));
	m_ui->comboBoxSWModelType->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (bc->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->lineEditSolarAbsorptionCoefficient->setReadOnly(!isEditable);
	m_ui->lineEditLongWaveEmissivity->setReadOnly(!isEditable);
	m_ui->lineEditHeatTransferCoefficient->setReadOnly(!isEditable);
	m_ui->comboBoxHeatTransferCoeffModelType->setEnabled(isEditable);
	m_ui->comboBoxLWModelType->setEnabled(isEditable);
	m_ui->comboBoxSWModelType->setEnabled(isEditable);

}




void SVDBBoundaryConditionEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		m_db->m_boundaryConditions.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}


void SVDBBoundaryConditionEditWidget::on_lineEditHeatTransferCoefficient_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditHeatTransferCoefficient->isValid() ) {
		double val = m_ui->lineEditHeatTransferCoefficient->value();
		// update database but only if different from original
		if (m_current->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].empty() ||
			val != m_current->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value)
		{
			NANDRAD::KeywordList::setParameter(m_current->m_heatConduction.m_para, "InterfaceHeatConduction::para_t", NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient, val);
			m_db->m_boundaryConditions.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit tableDataChanged();
		}
	}
}


void SVDBBoundaryConditionEditWidget::on_lineEditSolarAbsorptionCoefficient_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditSolarAbsorptionCoefficient->isValid() ) {
		double val = m_ui->lineEditSolarAbsorptionCoefficient->value();
		// update database but only if different from original
		if (m_current->m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].empty() ||
			val != m_current->m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].value)
		{
			NANDRAD::KeywordList::setParameter(m_current->m_solarAbsorption.m_para, "InterfaceSolarAbsorption::para_t", NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient, val);
			m_db->m_boundaryConditions.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit tableDataChanged();
		}
	}
}


void SVDBBoundaryConditionEditWidget::on_lineEditLongWaveEmissivity_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditLongWaveEmissivity->isValid() ) {
		double val = m_ui->lineEditLongWaveEmissivity->value();
		// update database but only if different from original
		if (m_current->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].empty() ||
			val != m_current->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value)
		{
			NANDRAD::KeywordList::setParameter(m_current->m_longWaveEmission.m_para, "InterfaceLongWaveEmission::para_t", NANDRAD::InterfaceLongWaveEmission::P_Emissivity, val);
			m_db->m_boundaryConditions.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit tableDataChanged();
		}
	}
}

void SVDBBoundaryConditionEditWidget::on_comboBoxHeatTransferCoeffModelType_currentIndexChanged(int index)
{
	Q_ASSERT(m_current != nullptr);
	// update database but only if different from original
	if (index != (int)m_current->m_heatConduction.m_modelType)
	{
		m_current->m_heatConduction.m_modelType = static_cast<NANDRAD::InterfaceHeatConduction::modelType_t>(index);
		m_db->m_boundaryConditions.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}

void SVDBBoundaryConditionEditWidget::on_comboBoxLWModelType_currentIndexChanged(int index)
{
	Q_ASSERT(m_current != nullptr);
	// update database but only if different from original
	if (index != (int)m_current->m_longWaveEmission.m_modelType)
	{
		m_current->m_longWaveEmission.m_modelType = static_cast<NANDRAD::InterfaceLongWaveEmission::modelType_t>(index);
		m_db->m_boundaryConditions.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}

void SVDBBoundaryConditionEditWidget::on_comboBoxSWModelType_currentIndexChanged(int index)
{
	Q_ASSERT(m_current != nullptr);
	// update database but only if different from original
	if (index != (int)m_current->m_solarAbsorption.m_modelType)
	{
		m_current->m_solarAbsorption.m_modelType = static_cast<NANDRAD::InterfaceSolarAbsorption::modelType_t>(index);
		m_db->m_boundaryConditions.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}


