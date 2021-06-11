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

#include "SVDBZoneIdealHeatingCoolingEditWidget.h"
#include "ui_SVDBZoneIdealHeatingCoolingEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <QtExt_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBZoneIdealHeatingCoolingTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"

SVDBZoneIdealHeatingCoolingEditWidget::SVDBZoneIdealHeatingCoolingEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBZoneIdealHeatingCoolingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	// *** populate combo boxes ***


	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Zone ideal heating/cooling model name"));

	m_ui->lineEditHeatingLimit->setup(0, 1000, tr("Maximum heating capacity limit."), true, true);
	m_ui->lineEditCoolingLimit->setup(0, 1000, tr("Maximum cooling capacity limit."), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBZoneIdealHeatingCoolingEditWidget::~SVDBZoneIdealHeatingCoolingEditWidget() {
	delete m_ui;
}


void SVDBZoneIdealHeatingCoolingEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBZoneIdealHeatingCoolingTableModel*>(dbModel);
}


void SVDBZoneIdealHeatingCoolingEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditHeatingLimit->setText("");
		m_ui->lineEditCoolingLimit->setText("");
		m_ui->checkBoxHeatingLimit->blockSignals(true);
		m_ui->checkBoxHeatingLimit->setChecked(false);
		m_ui->checkBoxHeatingLimit->blockSignals(false);
		m_ui->checkBoxCoolingLimit->blockSignals(true);
		m_ui->checkBoxCoolingLimit->setChecked(false);
		m_ui->checkBoxCoolingLimit->blockSignals(false);
		return;
	}

	m_current = const_cast<VICUS::ZoneIdealHeatingCooling *>(m_db->m_zoneIdealHeatingCooling[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	bool isHeatEmpty = m_current->m_para[VICUS::ZoneIdealHeatingCooling::P_HeatingLimit].empty();
	bool isCoolEmpty = m_current->m_para[VICUS::ZoneIdealHeatingCooling::P_CoolingLimit].empty();

	m_ui->checkBoxHeatingLimit->blockSignals(true);
	m_ui->checkBoxHeatingLimit->setChecked(!isHeatEmpty);
	m_ui->checkBoxHeatingLimit->blockSignals(false);

	m_ui->checkBoxCoolingLimit->blockSignals(true);
	m_ui->checkBoxCoolingLimit->setChecked(!isCoolEmpty);
	m_ui->checkBoxCoolingLimit->blockSignals(false);
	if(isHeatEmpty)
		m_ui->lineEditHeatingLimit->setText("");
	else
		m_ui->lineEditHeatingLimit->setValue(m_current->m_para[VICUS::ZoneIdealHeatingCooling::P_HeatingLimit].value);

	if(isCoolEmpty)
		m_ui->lineEditCoolingLimit->setText("");
	else
		m_ui->lineEditCoolingLimit->setValue(m_current->m_para[VICUS::ZoneIdealHeatingCooling::P_CoolingLimit].value);

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);


	m_ui->lineEditHeatingLimit->setEnabled(!isbuiltIn && m_ui->checkBoxHeatingLimit->isChecked());
	m_ui->lineEditCoolingLimit->setEnabled(!isbuiltIn && m_ui->checkBoxCoolingLimit->isChecked());
}


void SVDBZoneIdealHeatingCoolingEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
	updateInput((int)m_current->m_id);
}

void SVDBZoneIdealHeatingCoolingEditWidget::on_lineEditHeatingLimit_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if(m_ui->lineEditHeatingLimit->isValid()){
		double val = m_ui->lineEditHeatingLimit->value();

		VICUS::ZoneIdealHeatingCooling::para_t paraName= VICUS::ZoneIdealHeatingCooling::P_HeatingLimit;
		if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "ZoneIdealHeatingCooling::para_t", paraName, val);
			modelModify();
		}
	}
	updateInput((int)m_current->m_id);
}

void SVDBZoneIdealHeatingCoolingEditWidget::on_lineEditCoolingLimit_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if(m_ui->lineEditCoolingLimit->isValid()){
		double val = m_ui->lineEditCoolingLimit->value();

		VICUS::ZoneIdealHeatingCooling::para_t paraName = VICUS::ZoneIdealHeatingCooling::P_CoolingLimit;
		if (m_current->m_para[paraName].empty() ||
				val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "ZoneIdealHeatingCooling::para_t", paraName, val);
			modelModify();
		}
	}
	updateInput((int)m_current->m_id);
}

void SVDBZoneIdealHeatingCoolingEditWidget::modelModify() {
	m_db->m_zoneIdealHeatingCooling.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}

void SVDBZoneIdealHeatingCoolingEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
	updateInput((int)m_current->m_id);
}

void SVDBZoneIdealHeatingCoolingEditWidget::on_checkBoxHeatingLimit_toggled(bool checked) {
	Q_ASSERT(m_current != nullptr);

	VICUS::ZoneIdealHeatingCooling::para_t paraName= VICUS::ZoneIdealHeatingCooling::P_HeatingLimit;

	if(checked){
		VICUS::KeywordList::setParameter(m_current->m_para, "ZoneIdealHeatingCooling::para_t", paraName,
										 0);
		on_lineEditHeatingLimit_editingFinished();
	}
	else
		m_current->m_para[paraName].clear();
	modelModify();
	updateInput((int)m_current->m_id);
}

void SVDBZoneIdealHeatingCoolingEditWidget::on_checkBoxCoolingLimit_toggled(bool checked) {
	Q_ASSERT(m_current != nullptr);

	VICUS::ZoneIdealHeatingCooling::para_t paraName= VICUS::ZoneIdealHeatingCooling::P_CoolingLimit;

	if(checked){
		VICUS::KeywordList::setParameter(m_current->m_para, "ZoneIdealHeatingCooling::para_t", paraName,
										 0);
		on_lineEditCoolingLimit_editingFinished();
	}
	else
		m_current->m_para[paraName].clear();
	modelModify();
	updateInput((int)m_current->m_id);
}