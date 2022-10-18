/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVDBBoundaryConditionEditWidget.h"
#include "ui_SVDBBoundaryConditionEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <QtExt_LanguageHandler.h>
#include <SVConversions.h>

#include "SVDBBoundaryConditionTableModel.h"
#include "SVConstants.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"

SVDBBoundaryConditionEditWidget::SVDBBoundaryConditionEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBBoundaryConditionEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayoutMaster->setMargin(4);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditSolarAbsorptionCoefficient->setup(0, 1, tr("Short wave solar absorption (0..1)"), true, true);
	m_ui->lineEditLongWaveEmissivity->setup(0, 1, tr("Long wave emissivity (0..1)"), true, true);
	m_ui->lineEditHeatTransferCoefficient->setup(0, 1000, tr("Thermal conductivity, should be a value between 0  and 1000"), true, true);

	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(true);
	for (unsigned int i=0; i <= VICUS::InterfaceHeatConduction::NUM_MT; ++i)
		m_ui->comboBoxHeatTransferCoeffModelType->addItem(QString("%1 [%2]")
														  .arg(VICUS::KeywordListQt::Description("InterfaceHeatConduction::modelType_t", (int)i))
														  .arg(VICUS::KeywordListQt::Keyword("InterfaceHeatConduction::modelType_t", (int)i)), i);
	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(false);

	m_ui->comboBoxConnectedZoneType->blockSignals(true);
	for (unsigned int i=0; i < VICUS::InterfaceHeatConduction::NUM_OZ; ++i)
		m_ui->comboBoxConnectedZoneType->addItem(QString("%1 [%2]")
														  .arg(VICUS::KeywordListQt::Description("InterfaceHeatConduction::OtherZoneType", (int)i))
														  .arg(VICUS::KeywordListQt::Keyword("InterfaceHeatConduction::OtherZoneType", (int)i)), i);
	m_ui->comboBoxConnectedZoneType->blockSignals(false);

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

	// For now hide all heat cond model type combo boxes
	m_ui->comboBoxHeatTransferCoeffModelType->setVisible(false);
	m_ui->labelHeatTransferCoeffModelType->setVisible(false);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBBoundaryConditionEditWidget::~SVDBBoundaryConditionEditWidget() {
	delete m_ui;
}


void SVDBBoundaryConditionEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBBoundaryConditionTableModel*>(dbModel);
}


void SVDBBoundaryConditionEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;

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

	m_ui->comboBoxConnectedZoneType->blockSignals(true);
	VICUS::InterfaceHeatConduction::OtherZoneType oz = bc->m_heatConduction.m_otherZoneType;
	if (oz == VICUS::InterfaceHeatConduction::NUM_OZ)
		oz = VICUS::InterfaceHeatConduction::OZ_Standard;
	m_ui->comboBoxConnectedZoneType->setCurrentIndex(m_ui->comboBoxConnectedZoneType->findData(oz));
	m_ui->comboBoxConnectedZoneType->blockSignals(false);
	// update model-specific input states
	on_comboBoxConnectedZoneType_currentIndexChanged(m_ui->comboBoxConnectedZoneType->currentIndex());

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

	m_ui->lineEditHeatTransferCoefficient->setValue(bc->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value);
	if (!bc->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_ConstTemperature].name.empty())
		m_ui->lineEditZoneConstTemperature->setValue(bc->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_ConstTemperature].get_value("C"));
	else
		m_ui->lineEditZoneConstTemperature->setValue(8);
	if (bc->m_heatConduction.m_idSchedule == VICUS::INVALID_ID) {
		m_ui->lineEditTemperatureScheduleName->setText("");
	}
	else {
		const VICUS::Schedule * sched = SVSettings::instance().m_db.m_schedules[bc->m_heatConduction.m_idSchedule];
		if (sched == nullptr)
			m_ui->lineEditTemperatureScheduleName->setText(tr("<select schedule>"));
		else
			m_ui->lineEditTemperatureScheduleName->setText(QtExt::MultiLangString2QString(sched->m_displayName));
	}

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
	m_ui->lineEditZoneConstTemperature->setReadOnly(!isEditable);
	m_ui->comboBoxHeatTransferCoeffModelType->setEnabled(isEditable);
	m_ui->comboBoxLWModelType->setEnabled(isEditable);
	m_ui->comboBoxSWModelType->setEnabled(isEditable);
	m_ui->comboBoxConnectedZoneType->setEnabled(isEditable);

}


void SVDBBoundaryConditionEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBBoundaryConditionEditWidget::on_lineEditHeatTransferCoefficient_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditHeatTransferCoefficient->value();
	// update database but only if different from original
	if (m_current->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].empty() ||
			val != m_current->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value)
	{
		NANDRAD::KeywordList::setParameter(m_current->m_heatConduction.m_para,
										   "InterfaceHeatConduction::para_t", NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient, val);
		modelModify();
	}

}


void SVDBBoundaryConditionEditWidget::on_lineEditSolarAbsorptionCoefficient_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditSolarAbsorptionCoefficient->value();
	// update database but only if different from original
	if (m_current->m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].empty() ||
			val != m_current->m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].value)
	{
		NANDRAD::KeywordList::setParameter(m_current->m_solarAbsorption.m_para,
										   "InterfaceSolarAbsorption::para_t", NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient, val);
		modelModify();
	}
}


void SVDBBoundaryConditionEditWidget::on_lineEditLongWaveEmissivity_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditLongWaveEmissivity->value();
	// update database but only if different from original
	if (m_current->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].empty() ||
			val != m_current->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value) {
		NANDRAD::KeywordList::setParameter(m_current->m_longWaveEmission.m_para,
										   "InterfaceLongWaveEmission::para_t", NANDRAD::InterfaceLongWaveEmission::P_Emissivity, val);
		modelModify();
	}
}


void SVDBBoundaryConditionEditWidget::on_comboBoxHeatTransferCoeffModelType_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);
	// update database but only if different from original
	if (index != (int)m_current->m_heatConduction.m_modelType) {
		m_current->m_heatConduction.m_modelType = static_cast<VICUS::InterfaceHeatConduction::modelType_t>(index);
		if (m_current->m_heatConduction.m_modelType == VICUS::InterfaceHeatConduction::NUM_MT)
			m_current->m_heatConduction = VICUS::InterfaceHeatConduction(); // reset entire object
		modelModify();
	}
	// by default disable all inputs
	m_ui->labelHeatTransferCoefficient->setEnabled(false);
	m_ui->lineEditHeatTransferCoefficient->setEnabled(false);
	// enable/disable inputs based on selected model type, but only if our groupbox itself is enabled
	if (m_ui->groupBoxHeatTransfer->isEnabled()) {
		switch (m_current->m_heatConduction.m_modelType) {
			case VICUS::InterfaceHeatConduction::MT_Constant:
				m_ui->labelHeatTransferCoefficient->setEnabled(true);
				m_ui->lineEditHeatTransferCoefficient->setEnabled(true);
			break;
			case VICUS::InterfaceHeatConduction::NUM_MT: break;
		}
	}
}


void SVDBBoundaryConditionEditWidget::on_comboBoxLWModelType_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);
	// update database but only if different from original
	if (index != (int)m_current->m_longWaveEmission.m_modelType) {
		m_current->m_longWaveEmission.m_modelType = static_cast<NANDRAD::InterfaceLongWaveEmission::modelType_t>(index);
		if (m_current->m_longWaveEmission.m_modelType == NANDRAD::InterfaceLongWaveEmission::NUM_MT)
			m_current->m_longWaveEmission = NANDRAD::InterfaceLongWaveEmission(); // reset entire object
		modelModify();
	}
	// by default disable all inputs
	m_ui->labelLongWaveEmissivity->setEnabled(false);
	m_ui->lineEditLongWaveEmissivity->setEnabled(false);
	// enable/disable inputs based on selected model type, but only if our groupbox itself is enabled
	if (m_ui->groupBoxLongWaveExchange->isEnabled()) {
		switch (m_current->m_longWaveEmission.m_modelType) {
			case NANDRAD::InterfaceLongWaveEmission::MT_Constant:
				m_ui->labelLongWaveEmissivity->setEnabled(true);
				m_ui->lineEditLongWaveEmissivity->setEnabled(true);
			break;
			case NANDRAD::InterfaceLongWaveEmission::NUM_MT: break;
		}
	}
}


void SVDBBoundaryConditionEditWidget::on_comboBoxSWModelType_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);
	// update database but only if different from original
	if (index != (int)m_current->m_solarAbsorption.m_modelType)
	{
		m_current->m_solarAbsorption.m_modelType = static_cast<NANDRAD::InterfaceSolarAbsorption::modelType_t>(index);
		if (m_current->m_solarAbsorption.m_modelType == NANDRAD::InterfaceSolarAbsorption::NUM_MT)
			m_current->m_solarAbsorption = NANDRAD::InterfaceSolarAbsorption(); // reset entire object
		modelModify();
	}
	// by default disable all inputs
	m_ui->labelSolarAbsorptionCoefficient->setEnabled(false);
	m_ui->lineEditSolarAbsorptionCoefficient->setEnabled(false);
	// enable/disable inputs based on selected model type, but only if our groupbox itself is enabled
	if (m_ui->groupBoxShortWaveRad->isEnabled()) {
		switch (m_current->m_solarAbsorption.m_modelType) {
			case NANDRAD::InterfaceSolarAbsorption::MT_Constant:
				m_ui->labelSolarAbsorptionCoefficient->setEnabled(true);
				m_ui->lineEditSolarAbsorptionCoefficient->setEnabled(true);
			break;
			case NANDRAD::InterfaceSolarAbsorption::NUM_MT: break;
		}
	}
}


void SVDBBoundaryConditionEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBBoundaryConditionEditWidget::modelModify() {
	m_db->m_boundaryConditions.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}


void SVDBBoundaryConditionEditWidget::on_comboBoxConnectedZoneType_currentIndexChanged(int index) {
	m_current->m_heatConduction.m_otherZoneType = (VICUS::InterfaceHeatConduction::OtherZoneType)index;
	m_ui->labelZoneConstTemperature->setEnabled(index == 1);
	m_ui->lineEditZoneConstTemperature->setEnabled(index == 1);
	m_ui->labelScheduleZoneTemperature->setEnabled(index == 2);
	m_ui->lineEditTemperatureScheduleName->setEnabled(index == 2);
	m_ui->toolButtonSelectTemperatureSchedule->setEnabled(index == 2);
	m_ui->toolButtonRemoveTemperatureSchedule->setEnabled(index == 2 && m_current->m_heatConduction.m_idSchedule != VICUS::INVALID_ID);
}


void SVDBBoundaryConditionEditWidget::on_toolButtonSelectTemperatureSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_heatConduction.m_idSchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_heatConduction.m_idSchedule != newId) {
		m_current->m_heatConduction.m_idSchedule = newId;
		modelModify();
	}
	m_ui->toolButtonRemoveTemperatureSchedule->setEnabled(m_current->m_heatConduction.m_idSchedule != VICUS::INVALID_ID);
	updateInput((int)m_current->m_id);
}


void SVDBBoundaryConditionEditWidget::on_toolButtonRemoveTemperatureSchedule_clicked() {
	m_current->m_heatConduction.m_idSchedule = VICUS::INVALID_ID;
	m_ui->toolButtonRemoveTemperatureSchedule->setEnabled(false);
	m_ui->lineEditTemperatureScheduleName->setText(tr("<select schedule>"));
	modelModify();
}


void SVDBBoundaryConditionEditWidget::on_lineEditZoneConstTemperature_editingFinishedSuccessfully() {
	VICUS::KeywordList::setParameter(m_current->m_heatConduction.m_para, "InterfaceHeatConduction::para_t",
									 VICUS::InterfaceHeatConduction::P_ConstTemperature, m_ui->lineEditZoneConstTemperature->value());
	modelModify();
}
