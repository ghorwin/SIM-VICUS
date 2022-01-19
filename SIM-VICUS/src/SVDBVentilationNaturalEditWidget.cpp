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

#include "SVDBVentilationNaturalEditWidget.h"
#include "ui_SVDBVentilationNaturalEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <SV_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBVentilationNaturalTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"
#include "SVSettings.h"

SVDBVentilationNaturalEditWidget::SVDBVentilationNaturalEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBVentilationNaturalEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Zone natural ventilation model name"));

	m_ui->lineEditAirChangeRate->setup(0, 50, tr("Air change rate."), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBVentilationNaturalEditWidget::~SVDBVentilationNaturalEditWidget() {
	delete m_ui;
}


void SVDBVentilationNaturalEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBVentilationNaturalTableModel*>(dbModel);
}


void SVDBVentilationNaturalEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->labelAirChangeRate->setText(tr("Air change rate:"));
	m_ui->labelScheduleHeating->setText(tr("Schedule name:"));

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditAirChangeRate->setText("");
		m_ui->lineEditScheduleName->setText("");
		return;
	}

	m_current = const_cast<VICUS::VentilationNatural *>(m_db->m_ventilationNatural[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);

	try {
		m_ui->lineEditAirChangeRate->setValue(m_current->m_para[VICUS::VentilationNatural::P_AirChangeRate].get_value(IBK::Unit("1/h")));
	}  catch (IBK::Exception &ex) {
		m_ui->lineEditAirChangeRate->setValue(0);
	}

	VICUS::Schedule * sched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idSchedule]);
	if (sched != nullptr)
		m_ui->lineEditScheduleName->setText(QtExt::MultiLangString2QString(sched->m_displayName));
	else
		m_ui->lineEditScheduleName->setText(tr("<select schedule>"));

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->toolButtonSelectSchedule->setEnabled(!isbuiltIn);
	m_ui->lineEditScheduleName->setEnabled(!isbuiltIn);

	m_ui->lineEditAirChangeRate->setEnabled(!isbuiltIn);
}


void SVDBVentilationNaturalEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}

void SVDBVentilationNaturalEditWidget::on_lineEditAirChangeRate_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditAirChangeRate;
	VICUS::VentilationNatural::para_t paraName = VICUS::VentilationNatural::P_AirChangeRate;
	std::string keywordList = "VentilationNatural::para_t";

	double val = lineEdit->value();

	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
		modelModify();
	}
}

void SVDBVentilationNaturalEditWidget::modelModify() {
	m_db->m_ventilationNatural.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}

void SVDBVentilationNaturalEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}

void SVDBVentilationNaturalEditWidget::on_toolButtonSelectSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idSchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idSchedule != newId) {
		m_current->m_idSchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}



