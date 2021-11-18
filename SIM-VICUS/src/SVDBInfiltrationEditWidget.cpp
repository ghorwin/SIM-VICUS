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

#include "SVDBInfiltrationEditWidget.h"
#include "ui_SVDBInfiltrationEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <QtExt_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBInfiltrationTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"
#include "SVSettings.h"

SVDBInfiltrationEditWidget::SVDBInfiltrationEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBInfiltrationEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	// *** populate combo boxes ***

	m_ui->comboBoxMethod->blockSignals(true);

	for (unsigned int i=0; i<VICUS::Infiltration::NUM_AC; ++i) {
		m_ui->comboBoxMethod->addItem(QString("%1 [%2]")
									  .arg(VICUS::KeywordListQt::Description("Infiltration::AirChangeType", (int)i))
									  .arg(VICUS::KeywordListQt::Keyword("Infiltration::AirChangeType", (int)i)), i);
	}
	m_ui->comboBoxMethod->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Infiltration Model Name"));

	m_ui->lineEditAirChangeRate->setup(0, 100, tr("Houly air change rate of entire zone air volume."), true, true);
	m_ui->lineEditShieldCoefficient->setup(0, 1, tr("Shield coefficient DIN EN 13789."), true, true); //Vorinit auf 0.07?

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBInfiltrationEditWidget::~SVDBInfiltrationEditWidget() {
	delete m_ui;
}


void SVDBInfiltrationEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBInfiltrationTableModel*>(dbModel);
}


void SVDBInfiltrationEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->labelDisplayName->setText(tr("Name:"));
	m_ui->labelCategory_2->setText(tr("Method:"));
	m_ui->labelAirChangeRate->setText(tr("Air Change Rate:"));
	m_ui->labelShieldCoefficient->setText(tr("Shield Coefficient:"));

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditAirChangeRate->setText("");
		m_ui->lineEditShieldCoefficient->setText("");

		return;
	}

	m_current = const_cast<VICUS::Infiltration *>(m_db->m_infiltration[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);

	//set method
	m_ui->comboBoxMethod->blockSignals(true);
	m_ui->comboBoxMethod->setCurrentIndex(m_current->m_airChangeType);
	m_ui->comboBoxMethod->blockSignals(false);

	try {
		m_ui->lineEditAirChangeRate->setValue(m_current->m_para[VICUS::Infiltration::P_AirChangeRate].get_value(IBK::Unit("1/h")));
	}  catch (IBK::Exception &ex) {
		//set up a new value
		m_ui->lineEditAirChangeRate->setValue(0);
		modelModify();
	}
	m_ui->lineEditShieldCoefficient->setValue(m_current->m_para[VICUS::Infiltration::P_ShieldingCoefficient].value);

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxMethod->setEnabled(!isbuiltIn);
	m_ui->lineEditAirChangeRate->setEnabled(!isbuiltIn);
	m_ui->lineEditShieldCoefficient->setEnabled(!isbuiltIn);

	if(m_current->m_airChangeType == VICUS::Infiltration::AC_normal){
		m_ui->lineEditShieldCoefficient->setEnabled(false);
		m_ui->lineEditShieldCoefficient->setText("");
	}
}


void SVDBInfiltrationEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBInfiltrationEditWidget::on_comboBoxMethod_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	if (static_cast<VICUS::Infiltration::AirChangeType>(index) != m_current->m_airChangeType) {
		m_current->m_airChangeType = static_cast<VICUS::Infiltration::AirChangeType>(index);
		modelModify();
	}

	switch(m_current->m_airChangeType){
		case VICUS::Infiltration::AC_normal:
		case VICUS::Infiltration::NUM_AC:{
			m_ui->lineEditShieldCoefficient->setEnabled(false);
			m_ui->lineEditShieldCoefficient->setText("");
		} break;
		case VICUS::Infiltration::AC_n50:{
			m_ui->lineEditShieldCoefficient->setEnabled(true);
			m_ui->lineEditShieldCoefficient->setValue(m_current->m_para[VICUS::Infiltration::P_ShieldingCoefficient].value);
		}break;
	}
}


void SVDBInfiltrationEditWidget::on_lineEditShieldCoefficient_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditShieldCoefficient->value();

	VICUS::Infiltration::para_t paraName = VICUS::Infiltration::P_ShieldingCoefficient;
	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, "Infiltration::para_t", paraName, val);
		modelModify();
	}


}

void SVDBInfiltrationEditWidget::on_lineEditAirChangeRate_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditAirChangeRate->value();

	VICUS::Infiltration::para_t paraName= VICUS::Infiltration::P_AirChangeRate;
	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, "Infiltration::para_t", paraName, val);
		modelModify();

	}
}


void SVDBInfiltrationEditWidget::modelModify() {
	m_db->m_infiltration.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}

void SVDBInfiltrationEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}




