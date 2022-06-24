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

#include "SVDBSupplySystemEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <QtExt_LanguageHandler.h>
#include <SV_Conversions.h>

#include <QFileDialog>

#include "SVDBSupplySystemTableModel.h"
#include "SVConstants.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"


SVDBSupplySystemEditWidget::SVDBSupplySystemEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBSupplySystemEditWidget)
{
	m_ui->setupUi(this);

	// TODO: Anne
//	// connect browse filename widget
//	connect(m_ui->widgetBrowseFileNameTSVFile, SIGNAL(editingFinished()), this, SLOT(on_heatExchangeDataFile_editingFinished()));
//	// and set up
//	m_ui->widgetBrowseFileNameTSVFile->setup("", true, true, tr("Data files (*.tsv)"), SVSettings::instance().m_dontUseNativeDialogs);

//	m_ui->widgetBrowseFileNameSupplyFMU->filename();


	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Supply system name");

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	// fill combo box
	m_ui->comboBoxSupplyType->blockSignals(true);
	for (unsigned int i=0; i < VICUS::SupplySystem::NUM_ST; ++i)
		m_ui->comboBoxSupplyType->addItem(QString("%1 [%2]")
								  .arg(VICUS::KeywordListQt::Description("SupplySystem::supplyType_t", (int)i))
								  .arg(VICUS::KeywordListQt::Keyword("SupplySystem::supplyType_t", (int)i)), i);
	// set invalid supply type
	m_ui->comboBoxSupplyType->setCurrentIndex(VICUS::SupplySystem::NUM_ST);
	m_ui->comboBoxSupplyType->blockSignals(false);
	// and deactivate box
	m_ui->comboBoxSupplyType->setEnabled(false);
	// add all options to staggered widget: reorder
	m_ui->stackedWidgetSupply->insertWidget(VICUS::SupplySystem::ST_StandAlone, m_ui->pageStandAlone);
	m_ui->stackedWidgetSupply->insertWidget(VICUS::SupplySystem::ST_DatabaseFMU, m_ui->pageDatabaseFMU);
	m_ui->stackedWidgetSupply->insertWidget(VICUS::SupplySystem::ST_UserDefinedFMU, m_ui->pageUserDefinedFMU);
	m_ui->stackedWidgetSupply->insertWidget(VICUS::SupplySystem::NUM_ST, m_ui->pageEmpty);

	// simply deactivate staggered widget
	m_ui->stackedWidgetSupply->setCurrentIndex(VICUS::SupplySystem::NUM_ST);

	// set all minimum and maximum value
	m_ui->lineEditMaxMassFlux->setMinimum(0.0);
	m_ui->lineEditSupplyTemp->setMinimum(0.0);
	m_ui->lineEditMaxMassFluxFMU->setMinimum(0.0);
	m_ui->lineEditHeatingPowerFMU->setMinimum(0.0);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBSupplySystemEditWidget::~SVDBSupplySystemEditWidget() {
	delete m_ui;
}


void SVDBSupplySystemEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBSupplySystemTableModel*>(dbModel);
}


void SVDBSupplySystemEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;

	m_ui->lineEditName->setEnabled(isEnabled);
	m_ui->pushButtonColor->setEnabled(isEnabled);
	m_ui->labelDisplayName->setEnabled(isEnabled);

	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		// simply deactivate staggered widget
		m_ui->stackedWidgetSupply->setCurrentIndex(VICUS::SupplySystem::NUM_ST);

		return;
	}

	VICUS::SupplySystem * supplySys = const_cast<VICUS::SupplySystem *>(m_db->m_supplySystems[(unsigned int)id]);
	m_current = supplySys;
	Q_ASSERT(m_current!=nullptr);

	// now update the GUI controls
	m_ui->lineEditName->setString(supplySys->m_displayName);

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (supplySys->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);

	m_ui->comboBoxSupplyType->setEnabled(true);
	m_ui->comboBoxSupplyType->setCurrentIndex(m_ui->comboBoxSupplyType->findData(m_current->m_supplyType));

	// enable stagged widget
	m_ui->stackedWidgetSupply->blockSignals(true);
	// choose page in staggered widget for suitable parametrization
	m_ui->stackedWidgetSupply->setCurrentIndex(m_current->m_supplyType);

	// set FMU file path
	if(m_current->m_supplyFMUPath.isEmpty()) {
		m_ui->widgetBrowseFileNameSupplyFMU->setFilename(QString());
	}
	else {
		QFileInfo supplyFMUInfo(m_current->m_supplyFMUPath);
		m_ui->widgetBrowseFileNameSupplyFMU->setFilename(supplyFMUInfo.fileName());
	}

	// set all defined parameters (0 otherwise):
	// maximum mass flux
	if(m_current->m_para[VICUS::SupplySystem::P_MaximumMassFlux].name.empty()) {
		m_ui->lineEditMaxMassFlux->setValue(0.0);
	}
	else {
		m_ui->lineEditMaxMassFlux->setValue(m_current->m_para[VICUS::SupplySystem::P_MaximumMassFlux].get_value("kg/s"));
	}
	// supply temperature
	if(m_current->m_para[VICUS::SupplySystem::P_SupplyTemperature].name.empty()) {
		m_ui->lineEditSupplyTemp->setValue(0.0);
	}
	else {
		m_ui->lineEditSupplyTemp->setValue(m_current->m_para[VICUS::SupplySystem::P_SupplyTemperature].get_value("C"));
	}
	// FMU maximum mass flux
	if(m_current->m_para[VICUS::SupplySystem::P_MaximumMassFluxFMU].name.empty()) {
		m_ui->lineEditMaxMassFluxFMU->setValue(0.0);
	}
	else {
		m_ui->lineEditMaxMassFluxFMU->setValue(m_current->m_para[VICUS::SupplySystem::P_MaximumMassFluxFMU].get_value("kg/s"));
	}
	// FMU heating power
	if(m_current->m_para[VICUS::SupplySystem::P_HeatingPowerFMU].name.empty()) {
		m_ui->lineEditHeatingPowerFMU->setValue(0.0);
	}
	else {
		m_ui->lineEditHeatingPowerFMU->setValue(m_current->m_para[VICUS::SupplySystem::P_HeatingPowerFMU].get_value("kW"));
	}

	// enable stagged widget
	m_ui->stackedWidgetSupply->blockSignals(false);
}


void SVDBSupplySystemEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBSupplySystemEditWidget::on_lineEditMaxMassFlux_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditMaxMassFlux->value();
	// set values
	IBK::Parameter &para = m_current->m_para[VICUS::SupplySystem::P_MaximumMassFlux];
	IBK::Unit unit("kg/s");
	if(para.name.empty() || !IBK::near_equal(val, para.get_value(unit)) ) {
		std::string errmsg;
		para.set(VICUS::KeywordList::Keyword("SupplySystem::para_t", VICUS::SupplySystem::P_MaximumMassFlux),
				 val, unit);

		modelModify();
	}
}


void SVDBSupplySystemEditWidget::on_lineEditSupplyTemp_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditSupplyTemp->value();
	// update database but only if different from original
	IBK::Parameter &para = m_current->m_para[VICUS::SupplySystem::P_SupplyTemperature];
	IBK::Unit unit("C");
	if(para.name.empty() || !IBK::near_equal(val, para.get_value(unit)) ) {
		std::string errmsg;
		para.set(VICUS::KeywordList::Keyword("SupplySystem::para_t", VICUS::SupplySystem::P_SupplyTemperature),
				 val, unit);

		modelModify();
	}
}


void SVDBSupplySystemEditWidget::on_lineEditMaxMassFluxFMU_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditMaxMassFluxFMU->value();
	// update database but only if different from original
	IBK::Parameter &para = m_current->m_para[VICUS::SupplySystem::P_MaximumMassFluxFMU];
	IBK::Unit unit("kg/s");
	if(para.name.empty() || !IBK::near_equal(val, para.get_value(unit)) ) {
		std::string errmsg;
		para.set(VICUS::KeywordList::Keyword("SupplySystem::para_t", VICUS::SupplySystem::P_MaximumMassFluxFMU),
				 val, unit);

		modelModify();
	}
}


void SVDBSupplySystemEditWidget::on_lineEditHeatingPowerFMU_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditHeatingPowerFMU->value();
	// update database but only if different from original
	IBK::Parameter &para = m_current->m_para[VICUS::SupplySystem::P_HeatingPowerFMU];
	IBK::Unit unit("kW");
	if(para.name.empty() || !IBK::near_equal(val, para.get_value(unit)) ) {
		std::string errmsg;
		para.set(VICUS::KeywordList::Keyword("SupplySystem::para_t", VICUS::SupplySystem::P_HeatingPowerFMU),
				 val, unit);

		modelModify();
	}
}


void SVDBSupplySystemEditWidget::on_comboBoxSupplyType_currentIndexChanged(int index) {
	// set supply type
	Q_ASSERT(m_current != nullptr);

	if(index != m_current->m_supplyType) {

		// set supply type
		m_current->m_supplyType = (VICUS::SupplySystem::supplyType_t) index;
		// set suitable staged widget
		m_ui->stackedWidgetSupply->blockSignals(true);
		// choose page in staggered widget for suitable parametrization
		m_ui->stackedWidgetSupply->setCurrentIndex(index);
		m_ui->stackedWidgetSupply->blockSignals(false);

		// update view
		modelModify();
	}
}


void SVDBSupplySystemEditWidget::on_pushButtonFMUPath_clicked()
{
	Q_ASSERT(m_current != nullptr);
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();

	// request file name
	QString filename = QFileDialog::getOpenFileName(
							this,
							tr("Select supply FMU path"),
							nullptr,
							tr("Functional mockup interface (*.fmu)"), nullptr,
							SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
						);

	if (filename.isEmpty()) return;

	QFile f1(filename);
	if (!f1.exists()) {
		QMessageBox::critical(
					this,
					tr("FMU path not found"),
					tr("The FMU path '%1' does not exist or cannot be accessed.").arg(filename)
			);
		return;
	}

	// set fmu path
	m_current->m_supplyFMUPath = filename;

	// update view:
	// visualize FMU file name
	QFileInfo supplyFMUInfo(filename);
	m_ui->widgetBrowseFileNameSupplyFMU->setFilename(supplyFMUInfo.fileName());
}


void SVDBSupplySystemEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBSupplySystemEditWidget::modelModify() {
	m_db->m_supplySystems.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}


