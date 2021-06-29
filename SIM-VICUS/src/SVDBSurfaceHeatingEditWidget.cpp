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

#include "SVDBSurfaceHeatingEditWidget.h"
#include "ui_SVDBSurfaceHeatingEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <QtExt_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBSurfaceHeatingTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"

SVDBSurfaceHeatingEditWidget::SVDBSurfaceHeatingEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBSurfaceHeatingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	// *** populate combo boxes ***

	m_ui->comboBoxType->blockSignals(true);

	for (unsigned int i=0; i<VICUS::SurfaceHeating::NUM_T; ++i) {
		m_ui->comboBoxType->addItem(QString("%1")
			.arg(VICUS::KeywordListQt::Description("SurfaceHeating::Type", (int)i)));
			//.arg(VICUS::KeywordListQt::Keyword("SurfaceHeating::Type", (int)i)), i);
	}
	m_ui->comboBoxType->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("SurfaceHeating Model Name"));

	m_ui->lineEditHeatingLimit->setup(0, 10000, tr("Maximum heating limit in W/m2."), true, true);
	m_ui->lineEditCoolingLimit->setup(0, 10000, tr("Maximum heating limit in W/m2."), true, true);
	m_ui->lineEditPipeSpacing->setup(0, 5, tr("Maximum fluid velocity in m/s."), false, true);
	m_ui->lineEditTemperaturDifference->setup(0, 80, tr("Temperature difference of supply and return fluid."), false, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBSurfaceHeatingEditWidget::~SVDBSurfaceHeatingEditWidget() {
	delete m_ui;
}


void SVDBSurfaceHeatingEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBSurfaceHeatingTableModel*>(dbModel);
}


void SVDBSurfaceHeatingEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditCoolingLimit->setText("");
		m_ui->lineEditHeatingLimit->setText("");
		m_ui->stackedWidget->setCurrentIndex(0);
		m_ui->comboBoxType->setCurrentIndex(0);
		m_ui->stackedWidget->setEnabled(false);
		return;
	}
	m_ui->stackedWidget->setEnabled(true);

	m_current = const_cast<VICUS::SurfaceHeating *>(m_db->m_surfaceHeatings[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);

	//set method
	m_ui->comboBoxType->blockSignals(true);
	m_ui->comboBoxType->setCurrentIndex(m_current->m_type);
	Q_ASSERT(m_ui->comboBoxType->currentIndex() != -1);
	m_ui->stackedWidget->setCurrentIndex(m_ui->comboBoxType->currentIndex());
	m_ui->comboBoxType->blockSignals(false);

	try {
		// test user data for value input
		m_current->m_para[VICUS::SurfaceHeating::P_HeatingLimit].checkedValue("HeatingLimit", "W/m2", "W/m2",
																			  0, true, 1000, false, nullptr);
	}
	catch (...) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", VICUS::SurfaceHeating::P_HeatingLimit, 50);
		modelModify();
	}
	m_ui->lineEditHeatingLimit->setValue(m_current->m_para[VICUS::SurfaceHeating::P_HeatingLimit].value); // input is in base SI unit

	try {
		m_current->m_para[VICUS::SurfaceHeating::P_CoolingLimit].checkedValue("CoolingLimit", "W/m2", "W/m2",
																			  0, true, 1000, false, nullptr);
	}  catch (...) {
		//set up a new value
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", VICUS::SurfaceHeating::P_CoolingLimit, 40);
		modelModify();
	}
	m_ui->lineEditCoolingLimit->setValue(m_current->m_para[VICUS::SurfaceHeating::P_CoolingLimit].value);


	try {
		m_current->m_para[VICUS::SurfaceHeating::P_MaxFluidVelocity].checkedValue("MaxFluidVelocity", "m/s", "m/s", 0, false, 10, true, nullptr);
	}  catch (...) {
		//set up a new value
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", VICUS::SurfaceHeating::P_MaxFluidVelocity, 10);
		modelModify();
	}
	m_ui->lineEditFluidVelocity->setValue(m_current->m_para[VICUS::SurfaceHeating::P_MaxFluidVelocity].value);

	try {
		m_current->m_para[VICUS::SurfaceHeating::P_TemperatureDifferenceSupplyReturn].checkedValue("TemperatureDifferenceSupplyReturn",
																								   "K", "K", 1, true, 80, true, nullptr);
	}  catch (...) {
		//set up a new value
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", VICUS::SurfaceHeating::P_TemperatureDifferenceSupplyReturn, 7);
		modelModify();
	}
	m_ui->lineEditTemperaturDifference->setValue(m_current->m_para[VICUS::SurfaceHeating::P_TemperatureDifferenceSupplyReturn].value);

	try {
		m_current->m_para[VICUS::SurfaceHeating::P_PipeSpacing].checkedValue("PipeSpacing", "m", "m", 0, false, 10, true, nullptr);
	}  catch (...) {
		//set up a new value
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", VICUS::SurfaceHeating::P_PipeSpacing, 0.1);
		modelModify();
	}
	m_ui->lineEditPipeSpacing->setValue(m_current->m_para[VICUS::SurfaceHeating::P_PipeSpacing].value);

	// lookup corresponding dataset entry in database
	const VICUS::NetworkPipe * pipe = m_db->m_pipes[m_current->m_idPipe];
	if (pipe == nullptr)
		m_ui->lineEditPipeName->setText("");
	else
		m_ui->lineEditPipeName->setText( QtExt::MultiLangString2QString(pipe->m_displayName) );

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxType->setEnabled(!isbuiltIn);
	m_ui->lineEditHeatingLimit->setEnabled(!isbuiltIn);
	m_ui->lineEditCoolingLimit->setEnabled(!isbuiltIn);
	m_ui->lineEditTemperaturDifference->setEnabled(!isbuiltIn);
	m_ui->lineEditFluidVelocity->setEnabled(!isbuiltIn);
	m_ui->lineEditPipeSpacing->setEnabled(!isbuiltIn);
	m_ui->lineEditPipeName->setReadOnly(isbuiltIn);
	m_ui->toolButtonSelectPipes->setEnabled(!isbuiltIn);
}


void SVDBSurfaceHeatingEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBSurfaceHeatingEditWidget::on_comboBoxType_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	if (static_cast<VICUS::SurfaceHeating::Type>(index) != m_current->m_type) {
		m_current->m_type = static_cast<VICUS::SurfaceHeating::Type>(index);
		modelModify();
	}
	m_ui->stackedWidget->setCurrentIndex(index);
}


void SVDBSurfaceHeatingEditWidget::modelModify() {
	m_db->m_surfaceHeatings.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}


void SVDBSurfaceHeatingEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBSurfaceHeatingEditWidget::on_lineEditPipeSpacing_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditPipeSpacing->value();
	VICUS::SurfaceHeating::para_t paraName= VICUS::SurfaceHeating::P_PipeSpacing;
	if (m_current->m_para[paraName].empty() || val != m_current->m_para[paraName].value) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", paraName, val);
		modelModify();
	}
}


void SVDBSurfaceHeatingEditWidget::on_lineEditFluidVelocity_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditFluidVelocity->value();

	VICUS::SurfaceHeating::para_t paraName= VICUS::SurfaceHeating::P_MaxFluidVelocity;
	if (m_current->m_para[paraName].empty() || val != m_current->m_para[paraName].value) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", paraName, val);
		modelModify();
	}
}


void SVDBSurfaceHeatingEditWidget::on_lineEditTemperaturDifference_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditTemperaturDifference->value();

	VICUS::SurfaceHeating::para_t paraName= VICUS::SurfaceHeating::P_TemperatureDifferenceSupplyReturn;
	if (m_current->m_para[paraName].empty() || val != m_current->m_para[paraName].value) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", paraName, val);
		modelModify();
	}
}

void SVDBSurfaceHeatingEditWidget::on_lineEditHeatingLimit_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditHeatingLimit->value();

	VICUS::SurfaceHeating::para_t paraName= VICUS::SurfaceHeating::P_HeatingLimit;
	if (m_current->m_para[paraName].empty() || val != m_current->m_para[paraName].value) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", paraName, val);
		modelModify();
	}
}

void SVDBSurfaceHeatingEditWidget::on_lineEditCoolingLimit_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditCoolingLimit->value();

	VICUS::SurfaceHeating::para_t paraName= VICUS::SurfaceHeating::P_CoolingLimit;
	if (m_current->m_para[paraName].empty() || val != m_current->m_para[paraName].value) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", paraName, val);
		modelModify();
	}
}
