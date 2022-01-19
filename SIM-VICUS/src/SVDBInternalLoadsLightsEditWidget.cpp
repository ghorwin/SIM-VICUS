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

#include "SVDBInternalLoadsLightsEditWidget.h"
#include "ui_SVDBInternalLoadsLightsEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <SV_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBInternalLoadsTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"
#include "SVSettings.h"

SVDBInternalLoadsLightsEditWidget::SVDBInternalLoadsLightsEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBInternalLoadsLightsEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	// *** populate combo boxes ***

	m_ui->comboBoxMethod->blockSignals(true);

	for (unsigned int i=0; i<VICUS::InternalLoad::NUM_PM; ++i) {
		m_ui->comboBoxMethod->addItem(QString("%1 [%2]")
									  .arg(VICUS::KeywordListQt::Description("InternalLoad::PowerMethod", (int)i))
									  .arg(VICUS::KeywordListQt::Keyword("InternalLoad::PowerMethod", (int)i)), i);
	}
	m_ui->comboBoxMethod->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Internal loads lights model name"));

	m_ui->lineEditPower->setup(0, 250000, tr("Lights power according to the given unit"), true, true);
	m_ui->lineEditConvectiveFactor->setup(0, 1, tr("Convective heat factor"), true, true);
	//m_ui->lineEditLatentFactor->setup(0, 1, tr("Latent heat factor"), true, true);
	//m_ui->lineEditLossFactor->setup(0, 1, tr("Loss heat factor"), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBInternalLoadsLightsEditWidget::~SVDBInternalLoadsLightsEditWidget() {
	delete m_ui;
}


void SVDBInternalLoadsLightsEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBInternalLoadsTableModel*>(dbModel);
}


void SVDBInternalLoadsLightsEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->labelLightsInput->setText(tr("Power:"));
	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->labelLightsInputUnit->setText("");
		m_ui->lineEditPower->setText("-");
		m_ui->lineEditConvectiveFactor->setText("-");
		//m_ui->lineEditLatentFactor->setText("-");
		//m_ui->lineEditLossFactor->setText("-");
		return;
	}

	m_current = const_cast<VICUS::InternalLoad *>(m_db->m_internalLoads[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	//change invalid Lights count method to a valid one
	if (m_current->m_powerMethod == VICUS::InternalLoad::PowerMethod::NUM_PM){
		m_current->m_powerMethod = VICUS::InternalLoad::PowerMethod::PM_Power;
		modelModify();
	}

	updateLabel();

	//set method
	m_ui->comboBoxMethod->blockSignals(true);
	m_ui->comboBoxMethod->setCurrentIndex(m_current->m_powerMethod);
	m_ui->comboBoxMethod->blockSignals(false);

	m_ui->lineEditConvectiveFactor->setValue(m_current->m_para[VICUS::InternalLoad::P_ConvectiveHeatFactor].value);
	//m_ui->lineEditLatentFactor->setValue(m_current->m_para[VICUS::InternalLoad::P_LatentHeatFactor].value);
	//m_ui->lineEditLossFactor->setValue(m_current->m_para[VICUS::InternalLoad::P_LossHeatFactor].value);

	VICUS::Schedule * sched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idPowerManagementSchedule ]);
	if (sched != nullptr)
		m_ui->lineEditManagementScheduleName->setText(QtExt::MultiLangString2QString(sched->m_displayName));
	else
		m_ui->lineEditManagementScheduleName->setText(tr("<select schedule>"));

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxMethod->setEnabled(!isbuiltIn);
	m_ui->lineEditPower->setEnabled(!isbuiltIn);
	m_ui->lineEditManagementScheduleName->setEnabled(!isbuiltIn);
	m_ui->lineEditConvectiveFactor->setEnabled(!isbuiltIn);
	m_ui->toolButtonRemovePowerManagementSchedule->setEnabled(!isbuiltIn);
	//m_ui->lineEditLatentFactor->setEnabled(!isbuiltIn);
	//m_ui->lineEditLossFactor->setEnabled(!isbuiltIn);
}


void SVDBInternalLoadsLightsEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBInternalLoadsLightsEditWidget::on_comboBoxMethod_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::InternalLoad::PowerMethod::NUM_PM; ++i){
		if(index == i){
			m_current->m_powerMethod = static_cast<VICUS::InternalLoad::PowerMethod>(i);
			modelModify();

		}
	}
	updateLabel();

	m_ui->labelLightsInput->setText(VICUS::KeywordListQt::Description("InternalLoad::PowerMethod", index) + ":");
}


void SVDBInternalLoadsLightsEditWidget::on_lineEditPower_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditPower->value();
	VICUS::InternalLoad::para_t paraName;
	// update database but only if different from original
	switch (m_current->m_powerMethod){
		case VICUS::InternalLoad::PowerMethod::PM_PowerPerArea:		paraName = VICUS::InternalLoad::P_PowerPerArea;	break;
		case VICUS::InternalLoad::PowerMethod::PM_Power:			paraName = VICUS::InternalLoad::P_Power;		break;
			//this should not happen
		case VICUS::InternalLoad::NUM_PM:	break;
	}

	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, "InternalLoad::para_t", paraName, val);
		modelModify();
	}
}


void SVDBInternalLoadsLightsEditWidget::on_lineEditConvectiveFactor_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditConvectiveFactor->value();
	// update database but only if different from original
	VICUS::InternalLoad::para_t paraName = VICUS::InternalLoad::P_ConvectiveHeatFactor;
	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, "InternalLoad::para_t", paraName, val);
		modelModify();
	}
}

void SVDBInternalLoadsLightsEditWidget::modelModify() {
	m_db->m_internalLoads.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data

}

void SVDBInternalLoadsLightsEditWidget::updateLabel()
{
	Q_ASSERT(m_current != nullptr);

	switch (m_current->m_powerMethod) {
		case VICUS::InternalLoad::PM_PowerPerArea: 	{
			m_ui->lineEditPower->setValue(m_current->m_para[VICUS::InternalLoad::P_PowerPerArea].value);
			m_ui->labelLightsInputUnit->setText(tr("W/m2"));
		} break;
		case VICUS::InternalLoad::PM_Power:			{
			m_ui->lineEditPower->setValue(m_current->m_para[VICUS::InternalLoad::P_Power].value);
			m_ui->labelLightsInputUnit->setText(tr("W"));
		} break;
			// set invalid method to power
		case VICUS::InternalLoad::NUM_PM:{
			m_ui->lineEditPower->setValue(m_current->m_para[VICUS::InternalLoad::P_Power].value);
			m_ui->labelLightsInputUnit->setText(tr("W"));
			m_current->m_powerMethod = VICUS::InternalLoad::PM_Power;
			modelModify();
		} break;
	}
}


void SVDBInternalLoadsLightsEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBInternalLoadsLightsEditWidget::on_toolButtonSelectSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idPowerManagementSchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idPowerManagementSchedule != newId) {
		m_current->m_idPowerManagementSchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}

void SVDBInternalLoadsLightsEditWidget::on_toolButtonRemovePowerManagementSchedule_clicked() {

	m_current->m_idPowerManagementSchedule = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}
