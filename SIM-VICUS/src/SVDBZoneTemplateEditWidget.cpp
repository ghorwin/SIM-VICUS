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

#include "SVDBZoneTemplateEditWidget.h"
#include "ui_SVDBZoneTemplateEditWidget.h"

#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <VICUS_KeywordList.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Conversions.h>

#include "SVDBZoneTemplateTreeModel.h"
#include "SVConstants.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"

SVDBZoneTemplateEditWidget::SVDBZoneTemplateEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBZoneTemplateEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(4);
	m_ui->verticalLayoutWidget->setMargin(0);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	// initial state is "nothing selected"
	updateInput(-1, -1, 0);
}


SVDBZoneTemplateEditWidget::~SVDBZoneTemplateEditWidget() {
	delete m_ui;
}


void SVDBZoneTemplateEditWidget::setup(SVDatabase * db, SVDBZoneTemplateTreeModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBZoneTemplateEditWidget::updateInput(int id, int subTemplateId, int subTemplateType) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;

	m_currentSubTemplateType = subTemplateType;

	m_ui->lineEditName->setEnabled(isEnabled);
	m_ui->pushButtonColor->setEnabled(isEnabled);

	m_ui->pushButtonAddPersonLoad->setEnabled(false);					m_ui->pushButtonAddPersonLoad->setChecked(false);
	m_ui->pushButtonAddElectricLoad->setEnabled(false);					m_ui->pushButtonAddElectricLoad->setChecked(false);
	m_ui->pushButtonAddLightLoad->setEnabled(false);					m_ui->pushButtonAddLightLoad->setChecked(false);

	m_ui->pushButtonAddInfiltration->setEnabled(false);					m_ui->pushButtonAddInfiltration->setChecked(false);
	m_ui->pushButtonAddVentilationNatural->setEnabled(false);			m_ui->pushButtonAddVentilationNatural->setChecked(false);

	m_ui->pushButtonAddThermostat->setEnabled(false);					m_ui->pushButtonAddThermostat->setChecked(false);

	///TODO Dirk später aktivieren wenn funktion eingebaut ist
	m_ui->pushButtonAddShading->setEnabled(false);						m_ui->pushButtonAddShading->setChecked(false);
	m_ui->pushButtonAddVentilationNaturalControl->setEnabled(false);	m_ui->pushButtonAddVentilationNaturalControl->setChecked(false);

	m_ui->pushButtonAddShading->setVisible(false);
	m_ui->pushButtonAddVentilationNaturalControl->setVisible(false);


	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditSubComponent->setText(QString());
		// hide widget with references
		m_ui->widget->setVisible(false);
		return;
	}

	VICUS::ZoneTemplate * item = const_cast<VICUS::ZoneTemplate*>(m_db->m_zoneTemplates[(unsigned int)id]);
	m_current = item;

	// now update the GUI controls
	m_ui->lineEditName->setString(m_current->m_displayName);

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (m_current->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);

	// re-enable add buttons only when not yet set
	if (item->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] == VICUS::INVALID_ID)
		m_ui->pushButtonAddPersonLoad->setEnabled(true);
	else {
		m_ui->pushButtonAddPersonLoad->setEnabled(false);
		m_ui->pushButtonAddPersonLoad->setChecked(true);
	}
	if (item->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment] == VICUS::INVALID_ID)
		m_ui->pushButtonAddElectricLoad->setEnabled(true);
	else {
		m_ui->pushButtonAddElectricLoad->setEnabled(false);
		m_ui->pushButtonAddElectricLoad->setChecked(true);
	}
	if (item->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadLighting] == VICUS::INVALID_ID)
		m_ui->pushButtonAddLightLoad->setEnabled(true);
	else {
		m_ui->pushButtonAddLightLoad->setEnabled(false);
		m_ui->pushButtonAddLightLoad->setChecked(true);
	}
	if (item->m_idReferences[VICUS::ZoneTemplate::ST_Infiltration] == VICUS::INVALID_ID)
		m_ui->pushButtonAddInfiltration->setEnabled(true);
	else{
		m_ui->pushButtonAddInfiltration->setEnabled(false);
		m_ui->pushButtonAddInfiltration->setChecked(true);
	}
	if (item->m_idReferences[VICUS::ZoneTemplate::ST_VentilationNatural] == VICUS::INVALID_ID)
		m_ui->pushButtonAddVentilationNatural->setEnabled(true);
	else{
		m_ui->pushButtonAddVentilationNatural->setEnabled(false);
		m_ui->pushButtonAddVentilationNatural->setChecked(true);
	}
	if (item->m_idReferences[VICUS::ZoneTemplate::ST_ControlThermostat] == VICUS::INVALID_ID)
		m_ui->pushButtonAddThermostat->setEnabled(true);
	else{
		m_ui->pushButtonAddThermostat->setEnabled(false);
		m_ui->pushButtonAddThermostat->setChecked(true);
	}
	if (item->m_idReferences[VICUS::ZoneTemplate::ST_IdealHeatingCooling] == VICUS::INVALID_ID)
		m_ui->pushButtonAddIdealHeatingCooling->setEnabled(true);
	else{
		m_ui->pushButtonAddIdealHeatingCooling->setEnabled(false);
		m_ui->pushButtonAddIdealHeatingCooling->setChecked(true);
	}


	// now the sub-template stuff
	if (subTemplateId == -1) {
		// hide widget with references
		m_ui->widget->setVisible(false);
		return;
	}
	// show widget with references
	m_ui->widget->setVisible(true);

	// determine which sub-template was selected
	switch ((VICUS::ZoneTemplate::SubTemplateType)subTemplateType) {
		case VICUS::ZoneTemplate::ST_Infiltration: {
			m_ui->labelSubTemplate->setText(tr("Infiltration:"));
			// lookup corresponding dataset entry in database
			const VICUS::Infiltration * inf = m_db->m_infiltration[(unsigned int)subTemplateId];
			if (inf == nullptr)
				m_ui->lineEditSubComponent->setText(tr("<select>"));
			else
				m_ui->lineEditSubComponent->setText( QtExt::MultiLangString2QString(inf->m_displayName) );
		} break;
		case VICUS::ZoneTemplate::ST_VentilationNatural: {
			m_ui->labelSubTemplate->setText(tr("Natural Ventilation:"));
			// lookup corresponding dataset entry in database
			const VICUS::VentilationNatural * venti = m_db->m_ventilationNatural[(unsigned int)subTemplateId];
			if (venti == nullptr)
				m_ui->lineEditSubComponent->setText(tr("<select>"));
			else
				m_ui->lineEditSubComponent->setText( QtExt::MultiLangString2QString(venti->m_displayName) );
		} break;
		case VICUS::ZoneTemplate::ST_IntLoadPerson: {
			m_ui->labelSubTemplate->setText(tr("Internal Loads - Person loads:"));
			// lookup corresponding dataset entry in database
			const VICUS::InternalLoad * iload = m_db->m_internalLoads[(unsigned int)subTemplateId];
			if (iload == nullptr) {
				m_ui->lineEditSubComponent->setText(tr("<select>"));
			}
			else {
				m_ui->lineEditSubComponent->setText( QtExt::MultiLangString2QString(iload->m_displayName) );
			}
		} break;
		case VICUS::ZoneTemplate::ST_IntLoadEquipment:{
			m_ui->labelSubTemplate->setText(tr("Internal Loads - Equipment loads:"));
			// lookup corresponding dataset entry in database
			const VICUS::InternalLoad * iload = m_db->m_internalLoads[(unsigned int)subTemplateId];
			if (iload == nullptr) {
				m_ui->lineEditSubComponent->setText(tr("<select>"));
			}
			else {
				m_ui->lineEditSubComponent->setText( QtExt::MultiLangString2QString(iload->m_displayName) );
			}
		} break;
		break;
		case VICUS::ZoneTemplate::ST_IntLoadLighting:{
			m_ui->labelSubTemplate->setText(tr("Internal Loads - Lighting loads:"));
			// lookup corresponding dataset entry in database
			const VICUS::InternalLoad * iload = m_db->m_internalLoads[(unsigned int)subTemplateId];
			if (iload == nullptr) {
				m_ui->lineEditSubComponent->setText(tr("<select>"));
			}
			else {
				m_ui->lineEditSubComponent->setText( QtExt::MultiLangString2QString(iload->m_displayName) );
			}
		}
		break;
		case VICUS::ZoneTemplate::ST_IntLoadOther:
		break;
		case VICUS::ZoneTemplate::ST_ControlThermostat:{
			m_ui->labelSubTemplate->setText(tr("Thermostat:"));
			// lookup corresponding dataset entry in database
			const VICUS::ZoneControlThermostat * thermo = m_db->m_zoneControlThermostat[(unsigned int)subTemplateId];
			if (thermo == nullptr)
				m_ui->lineEditSubComponent->setText(tr("<select>"));
			else
				m_ui->lineEditSubComponent->setText( QtExt::MultiLangString2QString(thermo->m_displayName) );
		}
		break;
		case VICUS::ZoneTemplate::ST_IdealHeatingCooling:{
			m_ui->labelSubTemplate->setText(tr("Ideal Heating/Cooling:"));
			// lookup corresponding dataset entry in database
			const VICUS::ZoneIdealHeatingCooling * ideal = m_db->m_zoneIdealHeatingCooling[(unsigned int)subTemplateId];
			if (ideal == nullptr)
				m_ui->lineEditSubComponent->setText(tr("<select>"));
			else
				m_ui->lineEditSubComponent->setText( QtExt::MultiLangString2QString(ideal->m_displayName) );
		}
		break;

		case VICUS::ZoneTemplate::NUM_ST:
		break;
	}

}


void SVDBZoneTemplateEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
	refreshUi();
}

void SVDBZoneTemplateEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
	refreshUi();
}

void SVDBZoneTemplateEditWidget::on_toolButtonSelectSubComponent_clicked() {
	unsigned int id = VICUS::INVALID_ID;

	if (id != VICUS::INVALID_ID) {
		// modify existing
		m_current->m_idReferences[m_currentSubTemplateType] = id;
		modelModify();
	}
	// we must assume that the name of the referenced sub-template has changed, so update controls accordingly
	refreshUi();
}

void SVDBZoneTemplateEditWidget::on_toolButtonRemoveSubComponent_clicked() {
	m_dbModel->deleteChildItem( m_dbModel->indexById(m_current->m_id), m_currentSubTemplateType);
	emit selectSubTemplate(m_current->m_id, VICUS::ZoneTemplate::NUM_ST);

	switch (m_currentSubTemplateType) {
		case VICUS::ZoneTemplate::ST_Infiltration:
			m_ui->pushButtonAddInfiltration->setChecked(false);
		break;
		case VICUS::ZoneTemplate::ST_VentilationNatural:
			m_ui->pushButtonAddVentilationNatural->setChecked(false);
		break;
		case VICUS::ZoneTemplate::ST_IntLoadPerson:
			m_ui->pushButtonAddPersonLoad->setChecked(false);
		break;
		case VICUS::ZoneTemplate::ST_IntLoadEquipment:
			m_ui->pushButtonAddElectricLoad->setChecked(false);
		break;
		case VICUS::ZoneTemplate::ST_IntLoadLighting:
			m_ui->pushButtonAddLightLoad->setChecked(false);
		break;
		case VICUS::ZoneTemplate::ST_ControlThermostat:
			m_ui->pushButtonAddThermostat->setChecked(false);
		break;
		case VICUS::ZoneTemplate::ST_IdealHeatingCooling:
			m_ui->pushButtonAddIdealHeatingCooling->setChecked(false);
		break;
	}
	refreshUi();
}


void SVDBZoneTemplateEditWidget::on_pushButtonAddPersonLoad_clicked() {
	Q_ASSERT(m_current != nullptr);

	// open the Internal Loads DB dialog and let user select one
	unsigned int id = SVMainWindow::instance().dbInternalLoadsPersonEditDialog()->select(VICUS::INVALID_ID);
	if (id == VICUS::INVALID_ID){
		m_ui->pushButtonAddPersonLoad->setChecked(false);
		return;
	}
	if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] != id) {
		if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] == VICUS::INVALID_ID) {
			// add new child
			m_dbModel->addChildItem( m_dbModel->indexById(m_current->m_id), VICUS::ZoneTemplate::ST_IntLoadPerson, id);
			emit selectSubTemplate(m_current->m_id, (int)VICUS::ZoneTemplate::ST_IntLoadPerson);
		}
		else {
			// modify existing
			m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] = id;
			modelModify();
		}
	}
	// we must assume that the name of the referenced sub-template has changed, so update controls accordingly
	refreshUi();
}


void SVDBZoneTemplateEditWidget::on_pushButtonAddElectricLoad_clicked() {
	Q_ASSERT(m_current != nullptr);

	// open the Internal Loads DB dialog and let user select one
	unsigned int id = SVMainWindow::instance().dbInternalLoadsElectricEquipmentEditDialog()->select(VICUS::INVALID_ID);
	if (id == VICUS::INVALID_ID){
		m_ui->pushButtonAddElectricLoad->setChecked(false);
		return;
	}
	if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment] != id) {
		if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment] == VICUS::INVALID_ID) {
			// add new child
			m_dbModel->addChildItem( m_dbModel->indexById(m_current->m_id), VICUS::ZoneTemplate::ST_IntLoadEquipment, id);
			emit selectSubTemplate(m_current->m_id, (int)VICUS::ZoneTemplate::ST_IntLoadEquipment);
		}
		else {
			// modify existing
			m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment] = id;
			modelModify();
		}
	}
	refreshUi();
}

void SVDBZoneTemplateEditWidget::on_pushButtonAddLightLoad_clicked() {
	Q_ASSERT(m_current != nullptr);

	// open the Internal Loads DB dialog and let user select one
	unsigned int id = SVMainWindow::instance().dbInternalLoadsLightsEditDialog()->select(VICUS::INVALID_ID);
	if (id == VICUS::INVALID_ID){
		m_ui->pushButtonAddLightLoad->setChecked(false);
		return;
	}
	if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadLighting] != id) {
		if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadLighting] == VICUS::INVALID_ID) {
			// add new child
			m_dbModel->addChildItem( m_dbModel->indexById(m_current->m_id), VICUS::ZoneTemplate::ST_IntLoadLighting, id);
			emit selectSubTemplate(m_current->m_id, (int)VICUS::ZoneTemplate::ST_IntLoadLighting);
		}
		else {
			// modify existing
			m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadLighting] = id;
			modelModify();
		}
	}
	refreshUi();
}

void SVDBZoneTemplateEditWidget::on_pushButtonAddInfiltration_clicked() {
	Q_ASSERT(m_current != nullptr);
	VICUS::ZoneTemplate::SubTemplateType subType = VICUS::ZoneTemplate::ST_Infiltration;
	// open the infiltration DB dialog and let user select one
	unsigned int id = SVMainWindow::instance().dbInfiltrationEditDialog()->select(VICUS::INVALID_ID);
	if (id == VICUS::INVALID_ID) {
		m_ui->pushButtonAddInfiltration->setChecked(false);
		return;
	}
	if (m_current->m_idReferences[subType] != id) {
		if (m_current->m_idReferences[subType] == VICUS::INVALID_ID) {
			// add new child
			m_dbModel->addChildItem( m_dbModel->indexById(m_current->m_id), subType, id);
			emit selectSubTemplate(m_current->m_id, (int)subType);
		}
		else {
			// modify existing
			m_current->m_idReferences[subType] = id;
			modelModify();
		}
	}
	refreshUi();
}

void SVDBZoneTemplateEditWidget::on_pushButtonAddVentilationNatural_clicked(){
	Q_ASSERT(m_current != nullptr);
	VICUS::ZoneTemplate::SubTemplateType subType = VICUS::ZoneTemplate::ST_VentilationNatural;
	// open the ventilation DB dialog and let user select one
	unsigned int id = SVMainWindow::instance().dbVentilationNaturalEditDialog()->select(VICUS::INVALID_ID);
	if (id == VICUS::INVALID_ID){
		m_ui->pushButtonAddVentilationNatural->setChecked(false);
		return;
	}
	if (m_current->m_idReferences[subType] != id) {
		if (m_current->m_idReferences[subType] == VICUS::INVALID_ID) {
			// add new child
			m_dbModel->addChildItem( m_dbModel->indexById(m_current->m_id), subType, id);
			emit selectSubTemplate(m_current->m_id, (int)subType);
		}
		else {
			// modify existing
			m_current->m_idReferences[subType] = id;
			modelModify();
		}
	}
	refreshUi();
}

void SVDBZoneTemplateEditWidget::on_pushButtonAddThermostat_clicked(){
	Q_ASSERT(m_current != nullptr);
	VICUS::ZoneTemplate::SubTemplateType subType = VICUS::ZoneTemplate::ST_ControlThermostat;
	// open the thermostat DB dialog and let user select one
	unsigned int id = SVMainWindow::instance().dbZoneControlThermostatEditDialog()->select(VICUS::INVALID_ID);
	if (id == VICUS::INVALID_ID){
		m_ui->pushButtonAddThermostat->setChecked(false);
		return;
	}
	if (m_current->m_idReferences[subType] != id) {
		if (m_current->m_idReferences[subType] == VICUS::INVALID_ID) {
			// add new child
			m_dbModel->addChildItem( m_dbModel->indexById(m_current->m_id), subType, id);
			emit selectSubTemplate(m_current->m_id, (int)subType);
		}
		else {
			// modify existing
			m_current->m_idReferences[subType] = id;
			modelModify();
		}
	}
	refreshUi();
}

void SVDBZoneTemplateEditWidget::on_pushButtonAddIdealHeatingCooling_clicked(){
	Q_ASSERT(m_current != nullptr);
	VICUS::ZoneTemplate::SubTemplateType subType = VICUS::ZoneTemplate::ST_IdealHeatingCooling;
	// open the ideal heating cooling DB dialog and let user select one
	unsigned int id = SVMainWindow::instance().dbZoneIdealHeatingCoolingEditDialog()->select(VICUS::INVALID_ID);
	if (id == VICUS::INVALID_ID){
		m_ui->pushButtonAddIdealHeatingCooling->setChecked(false);
		return;
	}
	if (m_current->m_idReferences[subType] != id) {
		if (m_current->m_idReferences[subType] == VICUS::INVALID_ID) {
			// add new child
			m_dbModel->addChildItem( m_dbModel->indexById(m_current->m_id), subType, id);
			emit selectSubTemplate(m_current->m_id, (int)subType);
		}
		else {
			// modify existing
			m_current->m_idReferences[subType] = id;
			modelModify();
		}
	}
	refreshUi();
}

void SVDBZoneTemplateEditWidget::modelModify() {
	m_db->m_zoneTemplates.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data

}

void SVDBZoneTemplateEditWidget::refreshUi() {
	updateInput((int)m_current->m_id, (int)m_current->m_idReferences[m_currentSubTemplateType], m_currentSubTemplateType);
}
