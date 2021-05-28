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

#include "SVDBComponentEditWidget.h"
#include "ui_SVDBComponentEditWidget.h"

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Conversions.h>

#include "SVSettings.h"
#include "SVDBComponentTableModel.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"
#include "SVConstants.h"

SVDBComponentEditWidget::SVDBComponentEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBComponentEditWidget)
{
	m_ui->setupUi(this);
	m_ui->masterLayout->setMargin(4);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Component identification name"));

	m_ui->lineEditRoughness->setup(0, 1, tr("Roughness according to the given unit"), true, true);
	m_ui->lineEditSpecularity->setup(0, 1, tr("Specularity"), true, true);

	// enter categories into combo box
	// block signals to avoid getting "changed" calls
	m_ui->comboBoxComponentType->blockSignals(true);
	for(int i=0; i<VICUS::Component::NUM_CT; ++i)
		m_ui->comboBoxComponentType->addItem(VICUS::KeywordListQt::Keyword("Component::ComponentType", i), i);
	m_ui->comboBoxComponentType->blockSignals(false);

	//construction group box
	m_ui->lineEditUValue->setReadOnly(true);
	m_ui->lineEditConstructionName->setReadOnly(true);

	//Daylight
	m_ui->lineEditDaylightName->setReadOnly(true);
	m_ui->pushButtonDaylightColor->setEnabled(false);
	m_ui->lineEditRoughness->setReadOnly(true);
	m_ui->lineEditSpecularity->setReadOnly(true);

	updateInput(-1);
}


SVDBComponentEditWidget::~SVDBComponentEditWidget() {
	delete m_ui;
}


void SVDBComponentEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBComponentTableModel*>(dbModel);
}


void SVDBComponentEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls
		setEnabled(false);

		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());

		// construction property info fields
		m_ui->comboBoxComponentType->setCurrentText("");
		m_ui->lineEditConstructionName->setText("");
		m_ui->lineEditUValue->setText("");

		m_ui->lineEditBoundaryConditionSideAName->setText("");

		m_ui->lineEditBoundaryConditionSideBName->setText("");

		m_ui->lineEditDaylightName->setText("");
		m_ui->lineEditRoughness->setText("");
		m_ui->lineEditSpecularity->setText("");
		m_ui->pushButtonComponentColor->setColor(Qt::black);

		return;
	}
	// re-enable all controls
	setEnabled(true);

	VICUS::Component * comp = const_cast<VICUS::Component*>(m_db->m_components[(unsigned int)id]);
	m_current = comp;

	// now update the GUI controls
	m_ui->comboBoxComponentType->blockSignals(true);
	m_ui->lineEditName->setString(comp->m_displayName);
	int typeIdx = m_ui->comboBoxComponentType->findData(comp->m_type);
	m_ui->comboBoxComponentType->setCurrentIndex(typeIdx);
	m_ui->comboBoxComponentType->blockSignals(false);

	m_ui->pushButtonComponentColor->blockSignals(true);
	m_ui->pushButtonComponentColor->setColor(m_current->m_color);
	m_ui->pushButtonComponentColor->blockSignals(false);

	m_ui->lineEditBoundaryConditionSideAName->setEnabled(true);
	m_ui->lineEditBoundaryConditionSideBName->setEnabled(true);

	double surfaceResistanceSideA = 0;
	double surfaceResistanceSideB = 0;

	const VICUS::BoundaryCondition *bcA = m_db->m_boundaryConditions[comp->m_idSideABoundaryCondition];
	if (bcA != nullptr){
		m_ui->lineEditBoundaryConditionSideAName->setText(QtExt::MultiLangString2QString(bcA->m_displayName));
		m_ui->textBrowserBCSideA->setHtml(bcA->htmlDescription());

		if(bcA->m_heatConduction.m_modelType == NANDRAD::InterfaceHeatConduction::MT_Constant){
			double hc = bcA->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
			surfaceResistanceSideA = hc > 0 ? 1/hc : 0;
		}
	}
	else {
		m_ui->lineEditBoundaryConditionSideAName->clear();
		m_ui->textBrowserBCSideA->clear();
	}

	const VICUS::BoundaryCondition *bcB = m_db->m_boundaryConditions[comp->m_idSideBBoundaryCondition];
	if (bcB != nullptr){
		m_ui->lineEditBoundaryConditionSideBName->setText(QtExt::MultiLangString2QString(bcB->m_displayName));
		m_ui->textBrowserBCSideB->setHtml(bcB->htmlDescription());

		if(bcB->m_heatConduction.m_modelType == NANDRAD::InterfaceHeatConduction::MT_Constant){
			double hc = bcB->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
			surfaceResistanceSideB = hc > 0 ? 1/hc : 0;
		}
	}
	else {
		m_ui->lineEditBoundaryConditionSideBName->clear();
		m_ui->textBrowserBCSideB->clear();
	}

	const VICUS::Construction *con = m_db->m_constructions[comp->m_idConstruction];
	if (con != nullptr) {
		m_ui->lineEditConstructionName->setText(QtExt::MultiLangString2QString(con->m_displayName));
		double UValue;
		/* Take for uvalue calculation the surface resistance from the side A and B if this exist.
			If all resistance are zero -> take standard resistance of 0.17+0.04 = 0.21
		*/
		bool validUValue = false;
		if(surfaceResistanceSideA>0 || surfaceResistanceSideB>0)
			validUValue = con->calculateUValue(UValue, m_db->m_materials, surfaceResistanceSideA, surfaceResistanceSideB);
		else
			validUValue = con->calculateUValue(UValue, m_db->m_materials, 0.13, 0.04);

		if (validUValue)
			m_ui->lineEditUValue->setText(QString("%L1").arg(UValue, 0, 'f', 4));
	}
	else {
		m_ui->lineEditUValue->setText("---");
		m_ui->lineEditConstructionName->setText("");
	}

	// for built-ins, disable editing/make read-only
	bool isEditable = !comp->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonComponentColor->setReadOnly(!isEditable);
	m_ui->comboBoxComponentType->setEnabled(isEditable);
	m_ui->toolButtonSelectConstruction->setEnabled(isEditable);
	m_ui->toolButtonSelectBoundaryConditionSideAName->setEnabled(isEditable);
	m_ui->toolButtonSelectBoundaryConditionSideBName->setEnabled(isEditable);
	m_ui->pushButtonDaylight->setEnabled(isEditable);

	m_ui->lineEditBoundaryConditionSideAName->setReadOnly(!isEditable);
	m_ui->lineEditBoundaryConditionSideBName->setReadOnly(!isEditable);

	///TODO Dirk später durchführen wenn datenbanken da sind

	m_ui->lineEditDaylightName->setEnabled(true);
	m_ui->lineEditRoughness->setEnabled(true);
	m_ui->lineEditSpecularity->setEnabled(true);
	m_ui->lineEditDaylightName->setText("");
	m_ui->lineEditRoughness->setText("---");
	m_ui->lineEditSpecularity->setText("---");
}


void SVDBComponentEditWidget::on_lineEditName_editingFinished(){
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBComponentEditWidget::on_comboBoxComponentType_currentIndexChanged(int /*index*/){
	if (m_current == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty"

	VICUS::Component::ComponentType ct = static_cast<VICUS::Component::ComponentType>(m_ui->comboBoxComponentType->currentData().toInt());
	if (ct != m_current->m_type) {
		m_current->m_type = ct;
		modelModify();
	}
}


void SVDBComponentEditWidget::on_toolButtonSelectConstruction_clicked() {
	// get construction edit dialog from mainwindow
	SVDatabaseEditDialog * conEditDialog = SVMainWindow::instance().dbConstructionEditDialog();
	unsigned int conId = conEditDialog->select(m_current->m_idConstruction);
	if (conId != m_current->m_idConstruction) {
		m_current->m_idConstruction = conId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBComponentEditWidget::on_toolButtonSelectBoundaryConditionSideAName_clicked() {
	// get boundary condition edit dialog from mainwindow
	SVDatabaseEditDialog * bcEditDialog = SVMainWindow::instance().dbBoundaryConditionEditDialog();
	unsigned int bcId = bcEditDialog->select(m_current->m_idSideABoundaryCondition);
	if (bcId != m_current->m_idSideABoundaryCondition) {
		m_current->m_idSideABoundaryCondition = bcId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBComponentEditWidget::on_toolButtonSelectBoundaryConditionSideBName_clicked() {
	// get boundary condition edit dialog from mainwindow
	SVDatabaseEditDialog * bcEditDialog = SVMainWindow::instance().dbBoundaryConditionEditDialog();
	unsigned int bcId = bcEditDialog->select(m_current->m_idSideBBoundaryCondition);
	if (bcId != m_current->m_idSideBBoundaryCondition) {
		m_current->m_idSideBBoundaryCondition = bcId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBComponentEditWidget::on_pushButtonComponentColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonComponentColor->color()) {
		m_current->m_color = m_ui->pushButtonComponentColor->color();
		modelModify();
	}

}

void SVDBComponentEditWidget::modelModify(){
	m_db->m_components.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}

