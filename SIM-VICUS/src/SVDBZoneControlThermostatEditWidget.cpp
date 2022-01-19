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

#include "SVDBZoneControlThermostatEditWidget.h"
#include "ui_SVDBZoneControlThermostatEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <SV_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBZoneControlThermostatTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"
#include "SVSettings.h"

SVDBZoneControlThermostatEditWidget::SVDBZoneControlThermostatEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBZoneControlThermostatEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridMasterLayout->setMargin(4);

	// *** populate combo boxes ***

	m_ui->comboBoxMethod->blockSignals(true);

	for (unsigned int i=0; i<VICUS::ZoneControlThermostat::NUM_CV; ++i) {
		m_ui->comboBoxMethod->addItem(QString("%1 [%2]")
									  .arg(VICUS::KeywordListQt::Description("ZoneControlThermostat::ControlValue", (int)i))
									  .arg(VICUS::KeywordListQt::Keyword("ZoneControlThermostat::ControlValue", (int)i)), i);
	}
	m_ui->comboBoxMethod->blockSignals(false);

	m_ui->comboBoxControllerType->blockSignals(true);

	for (unsigned int i=0; i<VICUS::ZoneControlThermostat::NUM_CT; ++i) {
		m_ui->comboBoxControllerType->addItem(QString("%1 [%2]")
											  .arg(VICUS::KeywordListQt::Description("ZoneControlThermostat::ControllerType", (int)i))
											  .arg(VICUS::KeywordListQt::Keyword("ZoneControlThermostat::ControllerType", (int)i)), i);
	}
	m_ui->comboBoxControllerType->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Zone control thermostat model name"));

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditTolerance->setup(0.1, 50, tr("Thermostat tolerance for heating and/or cooling mode. Min 0.1 C, Max 50 C."), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBZoneControlThermostatEditWidget::~SVDBZoneControlThermostatEditWidget() {
	delete m_ui;
}


void SVDBZoneControlThermostatEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBZoneControlThermostatTableModel*>(dbModel);
}


void SVDBZoneControlThermostatEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditTolerance->setText("");
		m_ui->lineEditHeatingScheduleName->setText("");
		m_ui->lineEditCoolingScheduleName->setText("");
		return;
	}

	m_current = const_cast<VICUS::ZoneControlThermostat *>(m_db->m_zoneControlThermostat[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);


	//set method
	m_ui->comboBoxMethod->blockSignals(true);
	m_ui->comboBoxMethod->setCurrentIndex(m_current->m_controlValue);
	m_ui->comboBoxMethod->blockSignals(false);

	//set controller type
	m_ui->comboBoxControllerType->blockSignals(true);
	m_ui->comboBoxControllerType->setCurrentIndex(m_current->m_controllerType);
	m_ui->comboBoxControllerType->blockSignals(false);


	m_ui->lineEditTolerance->setValue(m_current->m_para[VICUS::ZoneControlThermostat::P_Tolerance].value);

	VICUS::Schedule * sched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idHeatingSetpointSchedule]);
	if (sched != nullptr)
		m_ui->lineEditHeatingScheduleName->setText(QtExt::MultiLangString2QString(sched->m_displayName));
	else
		m_ui->lineEditHeatingScheduleName->setText(tr("<select schedule>"));

	VICUS::Schedule * schedC = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idCoolingSetpointSchedule]);
	if (schedC != nullptr)
		m_ui->lineEditCoolingScheduleName->setText(QtExt::MultiLangString2QString(schedC->m_displayName));
	else
		m_ui->lineEditCoolingScheduleName->setText(tr("<select schedule>"));

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxMethod->setEnabled(!isbuiltIn);
	m_ui->comboBoxControllerType->setEnabled(!isbuiltIn);
	m_ui->lineEditHeatingScheduleName->setEnabled(!isbuiltIn);
	m_ui->lineEditCoolingScheduleName->setEnabled(!isbuiltIn);

	m_ui->toolButtonRemoveHeatingSetpointSchedule->setEnabled(!isbuiltIn);
	m_ui->toolButtonRemoveCoolingSetpointSchedule->setEnabled(!isbuiltIn);

	m_ui->lineEditTolerance->setEnabled(!isbuiltIn);
}


void SVDBZoneControlThermostatEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBZoneControlThermostatEditWidget::on_comboBoxControlValue_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::ZoneControlThermostat::ControlValue::NUM_CV; ++i){
		if(index == i){
			m_current->m_controlValue = static_cast<VICUS::ZoneControlThermostat::ControlValue>(i);
			modelModify();

		}
	}
}

void SVDBZoneControlThermostatEditWidget::on_comboBoxControllerType_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::ZoneControlThermostat::ControllerType::NUM_CT; ++i){
		if(index == i){
			m_current->m_controllerType= static_cast<VICUS::ZoneControlThermostat::ControllerType>(i);
			modelModify();

		}
	}
}


void SVDBZoneControlThermostatEditWidget::on_lineEditTolerance_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditTolerance->value();

	VICUS::ZoneControlThermostat::para_t paraName= VICUS::ZoneControlThermostat::P_Tolerance;
	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, "ZoneControlThermostat::para_t", paraName, val);
		modelModify();
	}

}


void SVDBZoneControlThermostatEditWidget::modelModify() {
	m_db->m_zoneControlThermostat.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}


void SVDBZoneControlThermostatEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBZoneControlThermostatEditWidget::on_toolButtonSelectHeatingSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idHeatingSetpointSchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idHeatingSetpointSchedule != newId) {
		m_current->m_idHeatingSetpointSchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBZoneControlThermostatEditWidget::on_toolButtonSelectCoolingSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idCoolingSetpointSchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idCoolingSetpointSchedule != newId) {
		m_current->m_idCoolingSetpointSchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBZoneControlThermostatEditWidget::on_toolButtonRemoveHeatingSetpointSchedule_clicked() {

	m_current->m_idHeatingSetpointSchedule = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}


void SVDBZoneControlThermostatEditWidget::on_toolButtonRemoveCoolingSetpointSchedule_clicked() {

	m_current->m_idCoolingSetpointSchedule = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}
