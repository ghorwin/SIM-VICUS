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
#include <VICUS_utilities.h>

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

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	// enter categories into combo box
	// block signals to avoid getting "changed" calls
	m_ui->comboBoxComponentType->blockSignals(true);
	for (int i=0; i<VICUS::NetworkComponent::NUM_MT; ++i)
		m_ui->comboBoxComponentType->addItem(QString("%1")
											 .arg(VICUS::KeywordListQt::Keyword("NetworkComponent::ModelType", i)),
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

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType =
			VICUS::NetworkComponent::nandradNetworkComponentModelType(m_current->m_modelType);

	// enable schedules tool buttons (based on required schedules)
	std::vector<std::string> reqScheduleNames = NANDRAD::HydraulicNetworkComponent::requiredScheduleNames(nandradModelType);
	m_ui->groupBoxSchedules->setEnabled(!reqScheduleNames.empty());
	m_ui->toolButtonSchedule1->setEnabled(reqScheduleNames.size()==1 || reqScheduleNames.size()==2);
	m_ui->toolButtonSchedule2->setEnabled(reqScheduleNames.size()==2);


	// update Schedule names (based on existing schedules)
	if (m_current->m_scheduleIds.size()>0){
		if (m_db->m_schedules[m_current->m_scheduleIds[0]] == nullptr)
			m_ui->lineEditSchedule1->setText(tr("Invalid Schedule"));
		else
			m_ui->lineEditSchedule1->setText(QtExt::MultiLangString2QString(
											 m_db->m_schedules[m_current->m_scheduleIds[0]]->m_displayName));
	}
	if (m_current->m_scheduleIds.size()>1){
		if (m_db->m_schedules[m_current->m_scheduleIds[1]] == nullptr)
			m_ui->lineEditSchedule2->setText(tr("Invalid Schedule"));
		else
			m_ui->lineEditSchedule2->setText(QtExt::MultiLangString2QString(
											 m_db->m_schedules[m_current->m_scheduleIds[1]]->m_displayName));
	}

	// update pipe properties
	m_ui->lineEditPipeProperties->clear();
	m_ui->groupBoxPipeProperties->setEnabled(false);
	if (VICUS::NetworkComponent::hasPipeProperties(m_current->m_modelType)){
		m_ui->groupBoxPipeProperties->setEnabled(true);
		const VICUS::NetworkPipe *pipe = m_db->m_pipes[m_current->m_pipePropertiesId];
		if(pipe != nullptr)
			m_ui->lineEditPipeProperties->setText(QtExt::MultiLangString2QString(pipe->m_displayName));
	}

	// populate table widget with parameters
	populateTableWidget();

	// for built-ins, disable editing/make read-only
	bool isEditable = !comp->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);

}



void SVDBNetworkComponentEditWidget::populateTableWidget(){
	FUNCID(SVDBNetworkComponentEditWidget::populateTableWidget);

	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType =
			VICUS::NetworkComponent::nandradNetworkComponentModelType(m_current->m_modelType);

	// populate table widget with properties
	m_ui->tableWidgetParameters->clearContents();

	// get required parameters of the current component
	std::vector<unsigned int> paraVecStd = NANDRAD::HydraulicNetworkComponent::requiredParameter(nandradModelType, 1);
	std::vector<unsigned int> paraVecAdd = m_current->additionalRequiredParameter(m_current->m_modelType);
	std::vector<unsigned int> paraVec;
	for (unsigned int i: paraVecStd)
		paraVec.push_back(i);
	for (unsigned int i: paraVecAdd)
		paraVec.push_back(i);

	// get integer parameters
	std::vector<unsigned int> paraVecInt = m_current->requiredIntParameter(m_current->m_modelType);

	// get optional parameters
	std::vector<unsigned int> paraVecOpt = m_current->optionalParameter(m_current->m_modelType);

	// populate table widget with parameters
	m_ui->tableWidgetParameters->setRowCount(paraVec.size() + paraVecInt.size() + paraVecOpt.size());
	if (paraVec.empty() && paraVecInt.empty() && paraVecOpt.empty())
		m_ui->groupBoxModelParameters->setEnabled(false);
	else
		m_ui->groupBoxModelParameters->setEnabled(true);

	m_ui->tableWidgetParameters->blockSignals(true);

	int rowCount = 0;
	for (unsigned int para: paraVec) {
		QTableWidgetItem * item = new QTableWidgetItem(VICUS::KeywordListQt::Keyword("NetworkComponent::para_t", (int)para));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetParameters->setItem(rowCount, 0, item);
		try {
			IBK::Unit ioUnit(VICUS::KeywordListQt::Unit("NetworkComponent::para_t", (int)para));
			item = new QTableWidgetItem(QString::fromStdString(ioUnit.name()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetParameters->setItem(rowCount, 2, item);

			if (m_current->m_para[para].name.empty())
				item = new QTableWidgetItem(); // TODO : Hauke, set some meaningful initial value?
			else
				item = new QTableWidgetItem(QString("%L1").arg(m_current->m_para[para].get_value(ioUnit)));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			if ((unsigned int)rowCount < paraVecStd.size())
				item->setData(Qt::UserRole, DT_DoubleStd);
			else
				item->setData(Qt::UserRole, DT_DoubleAdditional);
			item->setData(Qt::UserRole+1, para);
			m_ui->tableWidgetParameters->setItem(rowCount, 1, item);

			++rowCount;
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}
	}


	// populate table widget with integer parameters

	for (unsigned int paraInt: paraVecInt) {
		// parameter name
		QTableWidgetItem * item = new QTableWidgetItem(VICUS::KeywordListQt::Keyword("NetworkComponent::intPara_t", (int)paraInt));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetParameters->setItem(rowCount, 0, item);
		try {
			// parameter unit
			item = new QTableWidgetItem("-");
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetParameters->setItem(rowCount, 2, item);
			// parameter value
			if (m_current->m_intPara[paraInt].name.empty())
				item = new QTableWidgetItem(); // TODO : Hauke, set some meaningful initial value?
			else
				item = new QTableWidgetItem(QString("%L1").arg(m_current->m_intPara[paraInt].value));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			item->setData(Qt::UserRole, DT_Integer);
			item->setData(Qt::UserRole+1, paraInt);
			m_ui->tableWidgetParameters->setItem(rowCount, 1, item);

			++rowCount;
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}
	}


	// populate table widget with optional parameters

	QFont fnt;
	fnt.setItalic(true);
	for (unsigned int paraOpt: paraVecOpt) {
		// parameter name
		QTableWidgetItem * item = new QTableWidgetItem(VICUS::KeywordListQt::Keyword("NetworkComponent::para_t", (int)paraOpt));
		item->setFont(fnt);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetParameters->setItem(rowCount, 0, item);
		try {
			// parameter unit
			IBK::Unit ioUnit(VICUS::KeywordListQt::Unit("NetworkComponent::para_t", (int)paraOpt));
			item = new QTableWidgetItem(QString::fromStdString(ioUnit.name()));
			item->setFont(fnt);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetParameters->setItem(rowCount, 2, item);
			// parameter value
			if (m_current->m_para[paraOpt].name.empty())
				item = new QTableWidgetItem(); // TODO : Hauke, set some meaningful initial value?
			else
				item = new QTableWidgetItem(QString("%L1").arg(m_current->m_para[paraOpt].get_value(ioUnit)));
			item->setFont(fnt);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			item->setData(Qt::UserRole, DT_DoubleOptional);
			item->setData(Qt::UserRole+1, paraOpt);
			m_ui->tableWidgetParameters->setItem(rowCount, 1, item);

			++rowCount;
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}
	}

	m_ui->tableWidgetParameters->blockSignals(false);
	m_ui->tableWidgetParameters->resizeColumnsToContents();
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

	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType =
			VICUS::NetworkComponent::nandradNetworkComponentModelType(m_current->m_modelType);
	std::vector<unsigned int> paraVecStd = NANDRAD::HydraulicNetworkComponent::requiredParameter(nandradModelType, 1);
	std::vector<unsigned int> paraVecAdd = m_current->additionalRequiredParameter(m_current->m_modelType);
	std::vector<unsigned int> paraVecOpt = m_current->optionalParameter(m_current->m_modelType);
	std::vector<unsigned int> paraVecInt = m_current->requiredIntParameter(m_current->m_modelType);

	if (ct != m_current->m_modelType) {
		// set new model type and name
		QString name = QString("new %1").arg(VICUS::KeywordList::Keyword("NetworkComponent::ModelType", ct));
		m_current->m_displayName = IBK::MultiLanguageString(name.toStdString());
		m_current->m_modelType = ct;
		// we keep parameters which are still valid
		for (unsigned int i=0; i<VICUS::NetworkComponent::NUM_P; ++i){
			if (std::find(paraVecStd.begin(), paraVecStd.end(), i) != paraVecStd.end() ||
				std::find(paraVecAdd.begin(), paraVecAdd.end(), i) != paraVecAdd.end() ||
				std::find(paraVecOpt.begin(), paraVecOpt.end(), i) != paraVecOpt.end())
				continue;
			m_current->m_para[i].clear();
		}
		for (unsigned int i=0; i<VICUS::NetworkComponent::NUM_IP; ++i){
			if (std::find(paraVecInt.begin(), paraVecInt.end(), i) == paraVecInt.end())
				m_current->m_intPara[i].clear();
		}
		// clear all other properties
		m_current->m_notes.clear();
		m_current->m_dataSource.clear();
		m_current->m_scheduleIds.clear();
		m_current->m_manufacturer.clear();
		m_current->m_polynomCoefficients.m_values.clear();
		m_current->m_pipePropertiesId = VICUS::INVALID_ID;

		modelModify();
		updateInput((int)m_current->m_id);
	}
}


void SVDBNetworkComponentEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
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

	// if dialog was canceled do nothing
	if (newId == VICUS::INVALID_ID)
		return;

	// else if we have a new id set it
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

	// if dialog was canceled do nothing
	if (newId == VICUS::INVALID_ID)
		return;

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

	Q_ASSERT(column==1); // only values can be edited

	QString errMsg("");

	// get pointer to item and its content
	QTableWidgetItem * item = m_ui->tableWidgetParameters->item(row, 1);
	QString text = item->text();

	bool ok = false;

	// Double parameters

	if (item->data(Qt::UserRole) == DT_DoubleStd ||
		item->data(Qt::UserRole) == DT_DoubleAdditional ||
		item->data(Qt::UserRole) == DT_DoubleOptional ) {

		VICUS::NetworkComponent::para_t paraNum = VICUS::NetworkComponent::para_t(
												item->data(Qt::UserRole + 1).toUInt());
		// empty parameters are allowed
		if (text.isEmpty()){
			m_current->m_para[paraNum] = IBK::Parameter();
			modelModify();
			return;
		}

		// check number
		double val = QtExt::Locale().toDouble(text, &ok);
		if (!ok)
			val = text.toDouble(&ok);
		if (!ok)
			errMsg = "Only numbers allowed!";

		// now do parameter specific checks
		if (ok) {
			IBK::Parameter parameter(VICUS::KeywordListQt::Keyword("NetworkComponent::para_t", paraNum), val,
									 VICUS::KeywordListQt::Unit("NetworkComponent::para_t", paraNum));
			try {
				if (item->data(Qt::UserRole) == DT_DoubleStd ||
					item->data(Qt::UserRole) == DT_DoubleOptional)
					NANDRAD::HydraulicNetworkComponent::checkModelParameter(parameter, paraNum);
				else
					VICUS::NetworkComponent::checkAdditionalParameter(parameter, paraNum);
			} catch (IBK::Exception &ex) {
				errMsg = ex.what();
				ok = false;
			}
			// finally set value
			VICUS::KeywordList::setParameter(m_current->m_para, "NetworkComponent::para_t", paraNum, val);
			modelModify();
		}
		else {
			m_ui->tableWidgetParameters->blockSignals(true);
			if (m_current->m_para[paraNum].empty())
				item->setText("");
			else
				item->setText(QString("%1").arg(m_current->m_para[paraNum].value));
			m_ui->tableWidgetParameters->blockSignals(false);
		}

	}


	// Integer parameters

	else if (item->data(Qt::UserRole) == DT_Integer)  {

		VICUS::NetworkComponent::intPara_t paraNum = VICUS::NetworkComponent::intPara_t(
												item->data(Qt::UserRole + 1).toUInt());
		// empty parameters are allowed
		if (text.isEmpty()){
			m_current->m_intPara[paraNum] = IBK::IntPara();
			modelModify();
			return;
		}

		// check if is integer and if it is double then cast it to integer
		int val = QtExt::Locale().toInt(text, &ok);
		if (!ok){
			val = (int)QtExt::Locale().toDouble(text, &ok);
		}
		if (!ok)
			errMsg = "Only numbers allowed!";

		// now do parameter specific checks
		std::string paraName = VICUS::KeywordListQt::Keyword("NetworkComponent::intPara_t", paraNum);
		if (ok) {
			IBK::IntPara parameter(paraName, val);
			try {
				VICUS::NetworkComponent::checkIntParameter(parameter, paraNum);
			} catch (IBK::Exception &ex) {
				errMsg = ex.what();
				ok = false;
			}
			// finally set value
			m_current->m_intPara[paraNum] = IBK::IntPara(paraName, val);
			modelModify();
		}
		else {
			m_ui->tableWidgetParameters->blockSignals(true);
			if (m_current->m_para[paraNum].empty())
				item->setText("");
			else
				item->setText(QString("%1").arg(m_current->m_intPara[paraNum].value));
			m_ui->tableWidgetParameters->blockSignals(false);
		}
	}

	if (!ok){
		QMessageBox msgBox(QMessageBox::Critical, "Invalid Value", errMsg, QMessageBox::Ok, this);
		msgBox.exec();
	}
}



void SVDBNetworkComponentEditWidget::modelModify() {
	m_db->m_networkComponents.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id);
	populateTableWidget();
}


void SVDBNetworkComponentEditWidget::on_toolButtonPipeProperties_clicked()
{
	Q_ASSERT(m_current != nullptr);

	// open schedule edit dialog in selection mode
	unsigned int id = 0;
	if (m_current->m_pipePropertiesId != VICUS::INVALID_ID)
		id = m_current->m_pipePropertiesId;

	unsigned int newId = SVMainWindow::instance().dbPipeEditDialog()->select(id);

	// if dialog was canceled do nothing
	if (newId == VICUS::INVALID_ID)
		return;

	// else if we have a new id set it
	if (id != newId) {
		m_current->m_pipePropertiesId = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}
