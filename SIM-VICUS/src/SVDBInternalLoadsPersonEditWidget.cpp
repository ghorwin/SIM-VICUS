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

#include "SVDBInternalLoadsPersonEditWidget.h"
#include "ui_SVDBInternalLoadsPersonEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <SV_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBInternalLoadsTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"
#include "SVSettings.h"

SVDBInternalLoadsPersonEditWidget::SVDBInternalLoadsPersonEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBInternalLoadsPersonEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	// *** populate combo boxes ***

	m_ui->comboBoxPersonMethod->blockSignals(true);

	for (unsigned int i=0; i<VICUS::InternalLoad::NUM_PCM; ++i) {
		m_ui->comboBoxPersonMethod->addItem(QString("%1").arg(VICUS::KeywordListQt::Description("InternalLoad::PersonCountMethod", (int)i)));
		//.arg(VICUS::KeywordListQt::Keyword("InternalLoad::PersonCountMethod", (int)i)), i);
	}
	m_ui->comboBoxPersonMethod->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Internal loads person model name"));

	m_ui->lineEditPersonCount->setup(0, 10000, tr("Person count according to the given unit"), true, true);
	m_ui->lineEditConvectiveFactor->setup(0, 1, tr("Convective heat factor"), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBInternalLoadsPersonEditWidget::~SVDBInternalLoadsPersonEditWidget() {
	delete m_ui;
}


void SVDBInternalLoadsPersonEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBInternalLoadsTableModel*>(dbModel);
}


void SVDBInternalLoadsPersonEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->labelPersonCountUnit->setText("");
		m_ui->lineEditPersonCount->setText("-");
		m_ui->lineEditConvectiveFactor->setText("-");
		return;
	}

	m_current = const_cast<VICUS::InternalLoad *>(m_db->m_internalLoads[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	//change invalid person count method to a valid one
	if (m_current->m_personCountMethod == VICUS::InternalLoad::PersonCountMethod::NUM_PCM){
		m_current->m_personCountMethod = VICUS::InternalLoad::PersonCountMethod::PCM_PersonCount;
		m_db->m_internalLoads.m_modified=true;
	}

	m_ui->comboBoxPersonMethod->blockSignals(true);
	m_ui->comboBoxPersonMethod->setCurrentIndex(m_current->m_personCountMethod);
	m_ui->comboBoxPersonMethod->blockSignals(false);

	m_ui->lineEditConvectiveFactor->setValue(m_current->m_para[VICUS::InternalLoad::P_ConvectiveHeatFactor].value);

	switch (m_current->m_personCountMethod) {
		case VICUS::InternalLoad::PCM_AreaPerPerson:{
			m_ui->labelPersonCountUnit->setText(tr("m2 per Person"));
			m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_AreaPerPerson].value);

		}break;
		case VICUS::InternalLoad::PCM_PersonPerArea: {
			m_ui->labelPersonCountUnit->setText(tr("Person per m2"));
			m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_PersonPerArea].value);
		}break;
		case VICUS::InternalLoad::PCM_PersonCount:
		case VICUS::InternalLoad::NUM_PCM:{
			m_ui->labelPersonCountUnit->setText(tr("Person"));
			m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_PersonCount].value);
		}break;
	}

	VICUS::Schedule * occSched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idOccupancySchedule ]);
	if (occSched != nullptr)
		m_ui->lineEditOccupancyScheduleName->setText(QtExt::MultiLangString2QString(occSched->m_displayName));
	else
		m_ui->lineEditOccupancyScheduleName->setText(tr("<select schedule>"));

	VICUS::Schedule * actSched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idActivitySchedule ]);
	if (actSched != nullptr)
		m_ui->lineEditActivityScheduleName->setText(QtExt::MultiLangString2QString(actSched->m_displayName));
	else
		m_ui->lineEditActivityScheduleName->setText(tr("<select schedule>"));

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxPersonMethod->setEnabled(!isbuiltIn);
	m_ui->lineEditPersonCount->setEnabled(!isbuiltIn);
	m_ui->lineEditConvectiveFactor->setEnabled(!isbuiltIn);
	m_ui->lineEditActivityScheduleName->setEnabled(!isbuiltIn);
	m_ui->lineEditOccupancyScheduleName->setEnabled(!isbuiltIn);
	m_ui->toolButtonRemoveActivitySchedule->setEnabled(!isbuiltIn);
	m_ui->toolButtonRemoveOccupancySchedule->setEnabled(!isbuiltIn);

}


void SVDBInternalLoadsPersonEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBInternalLoadsPersonEditWidget::on_comboBoxPersonMethod_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::InternalLoad::PersonCountMethod::NUM_PCM; ++i){
		if(index == i){
			m_current->m_personCountMethod = static_cast<VICUS::InternalLoad::PersonCountMethod>(i);
			modelModify();
			switch (m_current->m_personCountMethod) {
				case VICUS::InternalLoad::PCM_PersonPerArea:
					m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_PersonPerArea].value);
				break;
				case VICUS::InternalLoad::PCM_AreaPerPerson:
					m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_AreaPerPerson].value);
				break;
				case VICUS::InternalLoad::PCM_PersonCount:
				case VICUS::InternalLoad::NUM_PCM:
					m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_PersonCount].value);
				break;

			}
			break;
		}
	}

	m_ui->labelPersonCountUnit->setText(VICUS::KeywordListQt::Description("InternalLoad::PersonCountMethod", index));
}


void SVDBInternalLoadsPersonEditWidget::on_lineEditPersonCount_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditPersonCount->value();
	// update database but only if different from original
	VICUS::InternalLoad::para_t paraName;
	switch (m_current->m_personCountMethod) {
		case VICUS::InternalLoad::PCM_PersonPerArea:				paraName = VICUS::InternalLoad::P_PersonPerArea;	break;
		case VICUS::InternalLoad::PCM_AreaPerPerson:				paraName = VICUS::InternalLoad::P_AreaPerPerson;	break;
		case VICUS::InternalLoad::PCM_PersonCount:					paraName = VICUS::InternalLoad::P_PersonCount;		break;
		case VICUS::InternalLoad::NUM_PCM:							paraName = VICUS::InternalLoad::NUM_P;				break;
	}
	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, "InternalLoad::para_t", paraName, val);
		modelModify();
	}
}


void SVDBInternalLoadsPersonEditWidget::on_lineEditConvectiveFactor_editingFinishedSuccessfully() {
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


void SVDBInternalLoadsPersonEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBInternalLoadsPersonEditWidget::on_toolButtonSelectOccupancy_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idOccupancySchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idOccupancySchedule != newId) {
		m_current->m_idOccupancySchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);

}

void SVDBInternalLoadsPersonEditWidget::on_toolButtonSelectActivity_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idActivitySchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idActivitySchedule != newId) {
		m_current->m_idActivitySchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);

}

void SVDBInternalLoadsPersonEditWidget::modelModify() {
	m_db->m_internalLoads.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	updateInput((int)m_current->m_id);
}

void SVDBInternalLoadsPersonEditWidget::on_toolButtonRemoveOccupancySchedule_clicked() {

	m_current->m_idOccupancySchedule = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}

void SVDBInternalLoadsPersonEditWidget::on_toolButtonRemoveActivitySchedule_clicked() {

	m_current->m_idActivitySchedule = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}
