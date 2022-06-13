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

#include "SVSupplySystemSelectionDialog.h"
//#include "SVPropBuildingSurfaceHeatingWidget.h"
#include "ui_SVSupplySystemSelectionDialog.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QListWidgetItem>
#include <QPushButton>

#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "SVUndoModifySupplySystem.h"
#include "SVUndoModifyProject.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Project.h>


SVSupplySystemSelectionDialog::SVSupplySystemSelectionDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSupplySystemSelectionDialog)
{
	m_ui->setupUi(this);

	SVStyle::formatListView(m_ui->listWidgetSupply);

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
	m_ui->doubleSpinBoxMaxMassFlux->setMinimum(0.0);
	m_ui->doubleSpinBoxSupplyTemp->setMinimum(0.0);
	m_ui->doubleSpinBoxMaxMassFluxFMU->setMinimum(0.0);
	m_ui->doubleSpinBoxHeatingPowerFMU->setMinimum(0.0);

	// update dialog
	updateUi();
}


SVSupplySystemSelectionDialog::~SVSupplySystemSelectionDialog() {
	delete m_ui;
}

void SVSupplySystemSelectionDialog::updateUi()
{
	// block all signals
	m_ui->buttonBox->button(QDialogButtonBox::Ok)->blockSignals(true);
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->blockSignals(true);
	m_ui->listWidgetSupply->blockSignals(true);
	m_ui->listWidgetSupply->selectionModel()->blockSignals(true);
	m_ui->comboBoxSupplyType->blockSignals(true);
	m_ui->listWidgetSupply->setSortingEnabled(false);
	m_ui->listWidgetSupply->clear();

	// select network
	const std::vector<VICUS::SupplySystem> &supplies = project().m_embeddedDB.m_supplySystems;

	// add all networks to dialog
	for (const VICUS::SupplySystem &supply : supplies) {
		// add new network
		QListWidgetItem * item = new QListWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setText(QString::fromStdString(supply.m_displayName.string()));
		item->setData(Qt::UserRole,supply.m_id);
		m_ui->listWidgetSupply->addItem(item);
	}

	m_ui->listWidgetSupply->setSortingEnabled(true);
	// set invalid supply type
	m_ui->comboBoxSupplyType->setCurrentIndex(VICUS::SupplySystem::NUM_ST);
	// and activate supply type box
	m_ui->comboBoxSupplyType->setEnabled(false);
	// deactivate stacked widget
	m_ui->stackedWidgetSupply->setCurrentIndex(VICUS::SupplySystem::NUM_ST);

	// activate list widget for choice
	m_ui->listWidgetSupply->blockSignals(false);
	m_ui->listWidgetSupply->selectionModel()->blockSignals(false);
	m_ui->buttonBox->button(QDialogButtonBox::Ok)->blockSignals(false);
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->blockSignals(false);
	m_ui->comboBoxSupplyType->blockSignals(false);
}


void SVSupplySystemSelectionDialog::updateCurrent()
{
	Q_ASSERT(m_current != nullptr);

	m_ui->comboBoxSupplyType->setEnabled(true);
	m_ui->comboBoxSupplyType->setCurrentIndex(m_ui->comboBoxSupplyType->findData(m_current->m_supplyType));


	// enable stagged widget
	m_ui->stackedWidgetSupply->blockSignals(true);
	// choose page in staggered widget for suitable parametrization
	m_ui->stackedWidgetSupply->setCurrentIndex(m_current->m_supplyType);

	// set FMU file path
	if(m_current->m_supplyFMUPath.isEmpty()) {
		m_ui->lineEditSupplyFMUName->clear();
	}
	else {
		QFileInfo supplyFMUInfo(m_current->m_supplyFMUPath);
		m_ui->lineEditSupplyFMUName->setText(supplyFMUInfo.fileName());
	}

	// set all defined parameters (0 otherwise):
	// maximum mass flux
	if(m_current->m_para[VICUS::SupplySystem::P_MaximumMassFlux].name.empty()) {
		m_ui->doubleSpinBoxMaxMassFlux->setValue(0.0);
	}
	else {
		m_ui->doubleSpinBoxMaxMassFlux->setValue(m_current->m_para[VICUS::SupplySystem::P_MaximumMassFlux].get_value("kg/s"));
	}
	// supply temperature
	if(m_current->m_para[VICUS::SupplySystem::P_SupplyTemperature].name.empty()) {
		m_ui->doubleSpinBoxSupplyTemp->setValue(0.0);
	}
	else {
		m_ui->doubleSpinBoxSupplyTemp->setValue(m_current->m_para[VICUS::SupplySystem::P_SupplyTemperature].get_value("C"));
	}
	// FMU maximum mass flux
	if(m_current->m_para[VICUS::SupplySystem::P_MaximumMassFluxFMU].name.empty()) {
		m_ui->doubleSpinBoxMaxMassFluxFMU->setValue(0.0);
	}
	else {
		m_ui->doubleSpinBoxMaxMassFluxFMU->setValue(m_current->m_para[VICUS::SupplySystem::P_MaximumMassFluxFMU].get_value("kg/s"));
	}
	// FMU heating power
	if(m_current->m_para[VICUS::SupplySystem::P_HeatingPowerFMU].name.empty()) {
		m_ui->doubleSpinBoxHeatingPowerFMU->setValue(0.0);
	}
	else {
		m_ui->doubleSpinBoxHeatingPowerFMU->setValue(m_current->m_para[VICUS::SupplySystem::P_HeatingPowerFMU].get_value("kW"));
	}

	// enable stagged widget
	m_ui->stackedWidgetSupply->blockSignals(false);
}


unsigned int SVSupplySystemSelectionDialog::SupplySystemId()
{
	if(m_current == nullptr)
		return VICUS::INVALID_ID;

	return m_current->m_id;
}


void SVSupplySystemSelectionDialog::on_listWidgetSupply_itemSelectionChanged()
{
	// disable ok button if selection is empty
	const QItemSelection &selection = m_ui->listWidgetSupply->selectionModel()->selection();

	if (selection.isEmpty()) {
		m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		// set invalid supply type
		on_comboBoxSupplyType_currentIndexChanged(VICUS::SupplySystem::NUM_ST);
		// deactivate supply type widget
		m_ui->comboBoxSupplyType->setEnabled(false);
		// set current null ptr
		m_current = nullptr;
	}
	else {
		m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
		// store ID of selected zone
		const QModelIndexList &indexList = selection.indexes();
		const QModelIndex &index = indexList.front();

		// select network
		unsigned int idSupply = (unsigned int) m_ui->listWidgetSupply->item(index.row())->data(Qt::UserRole).toInt();
		// find selected external supply object
		m_current = nullptr;
		for(const VICUS::SupplySystem &supplyObj : project().m_embeddedDB.m_supplySystems) {
			if(supplyObj.m_id == idSupply) {
				m_current = &supplyObj;
				break;
			}
		}
		Q_ASSERT(m_current != nullptr);

		// update all views
		updateCurrent();
	}
}


void SVSupplySystemSelectionDialog::on_listWidgetSupply_itemDoubleClicked(QListWidgetItem *item)
{
	// store ID of selected network
	unsigned int idSupply = (unsigned int) item->data(Qt::UserRole).toInt();

	// find selected external supplöy object
	m_current = nullptr;
	for(const VICUS::SupplySystem &supplyObj : project().m_embeddedDB.m_supplySystems) {
		if(supplyObj.m_id == idSupply) {
			m_current = &supplyObj;
			break;
		}
	}
	Q_ASSERT(m_current != nullptr);

	// update view
	updateCurrent();

	accept();
}



void SVSupplySystemSelectionDialog::on_comboBoxSupplyType_currentIndexChanged(int index) {
	// set supply type
	Q_ASSERT(m_current != nullptr);


	// invalid type
	if(index == VICUS::SupplySystem::NUM_ST)
		return;

	// retrieve a copy of current data
	VICUS::SupplySystem supply = *m_current;
	// set supply type
	supply.m_supplyType = (VICUS::SupplySystem::supplyType_t) index;

	// find external supply index in project
	unsigned int idx = 0;
	for(; idx < project().m_embeddedDB.m_supplySystems.size(); ++idx) {
		if(project().m_embeddedDB.m_supplySystems[idx].m_id == m_current->m_id)
			break;
	}

	Q_ASSERT(idx < project().m_embeddedDB.m_supplySystems.size());

	// undo action
	SVUndoModifySupplySystem * undo = new SVUndoModifySupplySystem(tr("Changed external supply"), idx, supply);
	undo->push();

	// update view
	updateCurrent();
}


void SVSupplySystemSelectionDialog::on_doubleSpinBoxMaxMassFlux_valueChanged(double val)
{
	Q_ASSERT(m_current != nullptr);
	// retrieve a copy of current data
	VICUS::SupplySystem supply = *m_current;
	// set values
	IBK::Parameter &para = supply.m_para[VICUS::SupplySystem::P_MaximumMassFlux];
	IBK::Unit unit("kg/s");
	if(para.name.empty()) {
		std::string errmsg;
		para.set(VICUS::KeywordList::Keyword("SupplySystem::para_t", VICUS::SupplySystem::P_MaximumMassFlux),
				 val, unit);
	}
	else {
		para.set(val, unit);
	}

	// find external supply index in project
	unsigned int index = 0;
	for(; index < project().m_embeddedDB.m_supplySystems.size(); ++index) {
		if(project().m_embeddedDB.m_supplySystems[index].m_id == m_current->m_id)
			break;
	}

	Q_ASSERT(index < project().m_embeddedDB.m_supplySystems.size());

	// undo action
	SVUndoModifySupplySystem * undo = new SVUndoModifySupplySystem(tr("Changed external supply"), index, supply);
	undo->push();
}


void SVSupplySystemSelectionDialog::on_doubleSpinBoxSupplyTemp_valueChanged(double val)
{
	Q_ASSERT(m_current != nullptr);
	// retrieve a copy of current data
	VICUS::SupplySystem supply = *m_current;
	// set values
	IBK::Unit unit("C");

	IBK::Parameter &para = supply.m_para[VICUS::SupplySystem::P_SupplyTemperature];
	if(para.name.empty()) {
		para.set(VICUS::KeywordList::Keyword("SupplySystem::para_t", VICUS::SupplySystem::P_SupplyTemperature),
				 val, unit);
	}
	else {
		para.set(val, unit);
	}

	// find external supply index in project
	unsigned int index = 0;
	for(; index < project().m_embeddedDB.m_supplySystems.size(); ++index) {
		if(project().m_embeddedDB.m_supplySystems[index].m_id == m_current->m_id)
			break;
	}

	Q_ASSERT(index < project().m_embeddedDB.m_supplySystems.size());

	// undo action
	SVUndoModifySupplySystem * undo = new SVUndoModifySupplySystem(tr("Changed external supply"), index, supply);
	undo->push();
}


void SVSupplySystemSelectionDialog::on_pushButtonFMUPath_clicked()
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

	// retrieve a copy of current data
	VICUS::SupplySystem supply = *m_current;
	// set fmu path
	supply.m_supplyFMUPath = filename;

	// find external supply index in project
	unsigned int index = 0;
	for(; index < project().m_embeddedDB.m_supplySystems.size(); ++index) {
		if(project().m_embeddedDB.m_supplySystems[index].m_id == m_current->m_id)
			break;
	}

	Q_ASSERT(index < project().m_embeddedDB.m_supplySystems.size());

	// undo action
	SVUndoModifySupplySystem * undo = new SVUndoModifySupplySystem(tr("Changed external supply"), index, supply);
	undo->push();

	// udpate view:
	// visualize FMU file name
	QFileInfo supplyFMUInfo(filename);
	m_ui->lineEditSupplyFMUName->setText(supplyFMUInfo.fileName());
}


void SVSupplySystemSelectionDialog::on_doubleSpinBoxMaxMassFluxFMU_valueChanged(double val)
{
	Q_ASSERT(m_current != nullptr);
	// retrieve a copy of current data
	VICUS::SupplySystem supply = *m_current;
	// set values
	IBK::Unit unit("kg/s");

	IBK::Parameter &para = supply.m_para[VICUS::SupplySystem::P_MaximumMassFluxFMU];
	if(para.name.empty()) {
		para.set(VICUS::KeywordList::Keyword("SupplySystem::para_t", VICUS::SupplySystem::P_MaximumMassFluxFMU),
				 val, unit);
	}
	else {
		para.set(val, unit);
	}

	// find external supply index in project
	unsigned int index = 0;
	for(; index < project().m_embeddedDB.m_supplySystems.size(); ++index) {
		if(project().m_embeddedDB.m_supplySystems[index].m_id == m_current->m_id)
			break;
	}

	Q_ASSERT(index < project().m_embeddedDB.m_supplySystems.size());

	// undo action
	SVUndoModifySupplySystem * undo = new SVUndoModifySupplySystem(tr("Changed FMU maximum mass flux towards building"), index, supply);
	undo->push();
}


void SVSupplySystemSelectionDialog::on_doubleSpinBoxHeatingPowerFMU_valueChanged(double val)
{
	Q_ASSERT(m_current != nullptr);
	// retrieve a copy of current data
	VICUS::SupplySystem supply = *m_current;
	// set values
	IBK::Unit unit("kW");

	IBK::Parameter &para = supply.m_para[VICUS::SupplySystem::P_HeatingPowerFMU];
	if(para.name.empty()) {
		para.set(VICUS::KeywordList::Keyword("SupplySystem::para_t", VICUS::SupplySystem::P_HeatingPowerFMU),
				 val, unit);
	}
	else {
		para.set(val, unit);
	}

	// find external supply index in project
	unsigned int index = 0;
	for(; index < project().m_embeddedDB.m_supplySystems.size(); ++index) {
		if(project().m_embeddedDB.m_supplySystems[index].m_id == m_current->m_id)
			break;
	}

	Q_ASSERT(index < project().m_embeddedDB.m_supplySystems.size());

	// undo action
	SVUndoModifySupplySystem * undo = new SVUndoModifySupplySystem(tr("Changed FMU heating power"), index, supply);
	undo->push();
}


void SVSupplySystemSelectionDialog::on_pushButtonCreateNew_clicked()
{
	// add an external supply option
	VICUS::SupplySystem newSupply;

	QInputDialog dlg(this);
	dlg.setLabelText("Enter name");
	if(dlg.exec() == QDialog::Rejected)
		return;

	newSupply.m_displayName = IBK::MultiLanguageString(dlg.textValue().toStdString());
	// set id
	newSupply.m_id = project().nextUnusedID();
	// set supply type to invalid
	newSupply.m_supplyType = VICUS::SupplySystem::NUM_ST;

	// add supply to project copy
	VICUS::Project prj = project();

	// add supply at the beginning of list
	prj.m_embeddedDB.m_supplySystems.insert(prj.m_embeddedDB.m_supplySystems.begin(), newSupply);
	SVUndoModifyProject * undo = new SVUndoModifyProject(tr("Added new external supply"), prj);
	undo->push();

	// update list
	updateUi();

	// set selection to new supply element
	m_current = &project().m_embeddedDB.m_supplySystems[0];
	m_ui->listWidgetSupply->setCurrentRow(0);

	updateCurrent();
}
