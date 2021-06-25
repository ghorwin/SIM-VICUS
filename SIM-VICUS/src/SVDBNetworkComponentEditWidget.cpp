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

#include "SVDBNetworkComponentEditWidget.h"
#include "ui_SVDBNetworkComponentEditWidget.h"

#include "SVSettings.h"
#include "SVDBNetworkComponentTableModel.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"
#include "SVStyle.h"
#include <SVConstants.h>

#include <VICUS_NetworkComponent.h>
#include <VICUS_KeywordListQt.h>

#include <NANDRAD_HydraulicNetworkComponent.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Locale.h>
#include <QtExt_Conversions.h>


SVDBNetworkComponentEditWidget::SVDBNetworkComponentEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBNetworkComponentEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(),THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Component identification name"));

	// enter categories into combo box
	// block signals to avoid getting "changed" calls
	m_ui->comboBoxComponentType->blockSignals(true);
	for (int i=0; i<VICUS::NetworkComponent::NUM_MT; ++i)
		m_ui->comboBoxComponentType->addItem(QString("%1 (%2)")
											 .arg(VICUS::KeywordListQt::Keyword("NetworkComponent::ModelType", i))
											 .arg(VICUS::KeywordListQt::Description("NetworkComponent::ModelType", i)),
											 i);
	m_ui->comboBoxComponentType->blockSignals(false);

	// no headers
	m_ui->tableWidgetParameters->horizontalHeader()->setVisible(false);
	m_ui->tableWidgetParameters->verticalHeader()->setVisible(false);
	m_ui->tableWidgetParameters->setColumnCount(3);
	m_ui->tableWidgetParameters->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableWidgetParameters->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_ui->tableWidgetParameters->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetParameters);
	m_ui->tableWidgetParameters->setSortingEnabled(false);

	updateInput(-1);
}


SVDBNetworkComponentEditWidget::~SVDBNetworkComponentEditWidget() {
	delete m_ui;
}


void SVDBNetworkComponentEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBNetworkComponentTableModel*>(dbModel);
}


void SVDBNetworkComponentEditWidget::updateInput(int id) {
	FUNCID(SVDBNetworkComponentEditWidget::updateInput);

	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls - note: this does not disable signals of the components below
		setEnabled(false);

		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditSchedule1->clear();
		m_ui->lineEditSchedule2->clear();

		// construction property info fields
		m_ui->comboBoxComponentType->blockSignals(true);
		m_ui->comboBoxComponentType->setCurrentIndex(-1);
		m_ui->comboBoxComponentType->blockSignals(false);

		// Note: color button is disabled, hence color is gray
		return;
	}
	// re-enable all controls
	setEnabled(true);

	VICUS::NetworkComponent * comp = const_cast<VICUS::NetworkComponent*>(m_db->m_networkComponents[(unsigned int)id]);
	m_current = comp;

	// now update the GUI controls
	m_ui->lineEditName->setString(comp->m_displayName);

	m_ui->comboBoxComponentType->blockSignals(true);
	int typeIdx = m_ui->comboBoxComponentType->findData(comp->m_modelType);
	m_ui->comboBoxComponentType->setCurrentIndex(typeIdx);
	m_ui->comboBoxComponentType->blockSignals(false);

	m_ui->pushButtonComponentColor->blockSignals(true);
	m_ui->pushButtonComponentColor->setColor(m_current->m_color);
	m_ui->pushButtonComponentColor->blockSignals(false);

	// NOTE: we assume that ModelType enums are the same in both objects!
	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType =
			NANDRAD::HydraulicNetworkComponent::ModelType(m_current->m_modelType);

	// enable schedules tool buttons (based on required schedules)
	std::vector<std::string> reqScheduleNames = NANDRAD::HydraulicNetworkComponent::requiredScheduleNames(nandradModelType);
	m_ui->groupBoxSchedules->setEnabled(!reqScheduleNames.empty());
	m_ui->toolButtonSchedule1->setEnabled(reqScheduleNames.size()==1 || reqScheduleNames.size()==2);
	m_ui->toolButtonSchedule2->setEnabled(reqScheduleNames.size()==2);

	// update Schedule names (based on existing schedules)

		if (m_current->m_scheduleIds.size()>0){
			Q_ASSERT(m_db->m_schedules[m_current->m_scheduleIds[0]] != nullptr);
			m_ui->lineEditSchedule1->setText(QtExt::MultiLangString2QString(
												 m_db->m_schedules[m_current->m_scheduleIds[0]]->m_displayName));
		}
		if (m_current->m_scheduleIds.size()>1){
			Q_ASSERT(m_db->m_schedules[m_current->m_scheduleIds[1]] != nullptr);
			m_ui->lineEditSchedule2->setText(QtExt::MultiLangString2QString(
												 m_db->m_schedules[m_current->m_scheduleIds[1]]->m_displayName));
		}


	// populate table widget with properties
	m_ui->tableWidgetParameters->clearContents();

	// only insert parameters that are actually needed for the current model type
	std::vector<unsigned int> paraVec = NANDRAD::HydraulicNetworkComponent::requiredParameter(nandradModelType, 1);
	m_ui->tableWidgetParameters->setRowCount(paraVec.size());

	if (paraVec.empty())
		m_ui->groupBoxModelParameters->setEnabled(false);
	else
		m_ui->groupBoxModelParameters->setEnabled(true);

	m_ui->tableWidgetParameters->blockSignals(true);
	for (unsigned int i=0; i<paraVec.size(); ++i) {
		QTableWidgetItem * item = new QTableWidgetItem(VICUS::KeywordListQt::Keyword("NetworkComponent::para_t", (int)paraVec[i]));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetParameters->setItem((int)i, 0, item);
		try {
			IBK::Unit ioUnit(VICUS::KeywordListQt::Unit("NetworkComponent::para_t", (int)paraVec[i]));
			item = new QTableWidgetItem(QString::fromStdString(ioUnit.name()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetParameters->setItem((int)i, 2, item);

			// Mind: unit conversion needed if keyword-list unit does not match base SI unit
			//       but add check if there is no value yet
			if (m_current->m_para[paraVec[i]].name.empty())
				item = new QTableWidgetItem(); // TODO : Hauke, set some meaningful initial value?
			else
				item = new QTableWidgetItem(QString("%L1").arg(m_current->m_para[paraVec[i]].get_value(ioUnit)));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			m_ui->tableWidgetParameters->setItem((int)i, 1, item);
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}
	}
	m_ui->tableWidgetParameters->blockSignals(false);
	m_ui->tableWidgetParameters->resizeColumnsToContents();

	// for built-ins, disable editing/make read-only
	bool isEditable = !comp->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonComponentColor->setReadOnly(!isEditable);

}


void SVDBNetworkComponentEditWidget::on_lineEditName_editingFinished(){
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBNetworkComponentEditWidget::on_comboBoxComponentType_currentIndexChanged(int /*index*/) {
	if (m_current == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty"

	VICUS::NetworkComponent::ModelType ct = VICUS::NetworkComponent::ModelType(
													m_ui->comboBoxComponentType->currentData().toUInt());
	if (ct != m_current->m_modelType) {
		m_current->m_modelType = ct;
		modelModify();
		updateInput((int)m_current->m_id);
	}
}


void SVDBNetworkComponentEditWidget::on_pushButtonComponentColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonComponentColor->color()) {
		m_current->m_color = m_ui->pushButtonComponentColor->color();
		modelModify();
	}
}


void SVDBNetworkComponentEditWidget::on_toolButtonSchedule1_clicked()
{
	Q_ASSERT(m_current != nullptr);

	// open schedule edit dialog in selection mode
	unsigned int id = 0;
	if (m_current->m_scheduleIds.size()>0)
		id = m_current->m_scheduleIds[0];
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(id);
	if (id != newId) {
		if (m_current->m_scheduleIds.size()>0)
			m_current->m_scheduleIds[0] = newId;
		else
			m_current->m_scheduleIds.push_back(newId);
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBNetworkComponentEditWidget::on_toolButtonSchedule2_clicked()
{
	Q_ASSERT(m_current != nullptr);

	// open schedule edit dialog in selection mode
	unsigned int id = 0;
	if (m_current->m_scheduleIds.size()==2)
		id = m_current->m_scheduleIds[1];
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(id);
	if (id != newId) {
		if (m_current->m_scheduleIds.size()==2)
			m_current->m_scheduleIds[1] = newId;
		else
			m_current->m_scheduleIds.push_back(newId);
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBNetworkComponentEditWidget::on_tableWidgetParameters_cellChanged(int row, int column) {

	QString errMsg("");

	// first check if it is a double
	QString text = m_ui->tableWidgetParameters->item(row, column)->text();
	bool ok = false;
	double val = QtExt::Locale().toDouble(text, &ok);
	// but also allow fall-back on C-locale
	if (!ok)
		val = text.toDouble(&ok);
	if (!ok)
		errMsg = "Only numbers allowed!";

	std::string parName = m_ui->tableWidgetParameters->item(row, 0)->text().toStdString();
	NANDRAD::HydraulicNetworkComponent::para_t paraNum = NANDRAD::HydraulicNetworkComponent::para_t(
												VICUS::KeywordListQt::Enumeration("NetworkComponent::para_t", parName));

	// now do parameter specific checks
	if (ok){
		IBK::Parameter parameter(VICUS::KeywordListQt::Keyword("NetworkComponent::para_t", paraNum), val,
								 VICUS::KeywordListQt::Unit("NetworkComponent::para_t", paraNum));
		try {
			NANDRAD::HydraulicNetworkComponent::checkModelParameter(parameter, paraNum);
		} catch (IBK::Exception &ex) {
			errMsg = ex.what();
			ok = false;
		}
	}

	// modify item and show message box
	if (!ok){
		m_ui->tableWidgetParameters->blockSignals(true);
		if (m_current->m_para[paraNum].empty())
			m_ui->tableWidgetParameters->item(row, column)->setText("");
		else
			m_ui->tableWidgetParameters->item(row, column)->setText(QString("%1")
																	.arg(m_current->m_para[paraNum].value));
		m_ui->tableWidgetParameters->blockSignals(false);
		QMessageBox msgBox(QMessageBox::Critical, "Invalid Value", errMsg, QMessageBox::Ok, this);
		msgBox.exec();
		return;
	}

	// finally set value
	VICUS::KeywordList::setParameter(m_current->m_para, "NetworkComponent::para_t", paraNum, val);
	modelModify();
}

void SVDBNetworkComponentEditWidget::modelModify() {
	m_db->m_networkComponents.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id);

}
