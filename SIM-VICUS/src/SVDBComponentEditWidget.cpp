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
#include <SV_Conversions.h>

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

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditRoughness->setup(0, 1, tr("Roughness according to the given unit"), true, true);
	m_ui->lineEditSpecularity->setup(0, 1, tr("Specularity"), true, true);

	// enter categories into combo box
	// block signals to avoid getting "changed" calls
	m_ui->comboBoxComponentType->blockSignals(true);
	for(int i=0; i<VICUS::Component::NUM_CT; ++i)
		m_ui->comboBoxComponentType->addItem(VICUS::KeywordListQt::Keyword("Component::ComponentType", i), i);
	m_ui->comboBoxComponentType->blockSignals(false);

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
		m_ui->pushButtonColor->setColor(Qt::black);

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

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	m_ui->lineEditBoundaryConditionSideAName->setEnabled(true);
	m_ui->lineEditBoundaryConditionSideBName->setEnabled(true);

	double surfaceResistanceSideA = 0;
	double surfaceResistanceSideB = 0;

	const VICUS::BoundaryCondition *bcA = m_db->m_boundaryConditions[comp->m_idSideABoundaryCondition];
	if (bcA != nullptr){
		m_ui->lineEditBoundaryConditionSideAName->setText(QtExt::MultiLangString2QString(bcA->m_displayName));
		m_ui->textBrowserBCSideA->setHtml(bcA->htmlDescription(m_db->m_schedules));

		if (bcA->m_heatConduction.m_modelType == VICUS::InterfaceHeatConduction::MT_Constant){
			double hc = bcA->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
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
		m_ui->textBrowserBCSideB->setHtml(bcB->htmlDescription(m_db->m_schedules));

		if (bcB->m_heatConduction.m_modelType == VICUS::InterfaceHeatConduction::MT_Constant){
			double hc = bcB->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
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
		// Take for uvalue calculation the surface resistance from the side A and B if this exist.
		// If all resistance are zero -> take standard resistance of 0.17+0.04 = 0.21
		bool validUValue = false;
		if (surfaceResistanceSideA>0 || surfaceResistanceSideB>0)
			validUValue = con->calculateUValue(UValue, m_db->m_materials, surfaceResistanceSideA, surfaceResistanceSideB);
		else
			validUValue = con->calculateUValue(UValue, m_db->m_materials, 0.13, 0.04);

		if (validUValue)
			m_ui->lineEditUValue->setText(QString("%L1").arg(UValue, 0, 'f', 4));

		m_ui->checkBoxActiveLayerEnabled->setEnabled(true);
		m_ui->checkBoxActiveLayerEnabled->blockSignals(true);
		m_ui->checkBoxActiveLayerEnabled->setChecked(m_current->m_activeLayerIndex != VICUS::INVALID_ID);
		m_ui->checkBoxActiveLayerEnabled->blockSignals(false);
		m_ui->spinBoxActiveLayerIndex->blockSignals(true);
		m_ui->spinBoxActiveLayerIndex->setEnabled(m_ui->checkBoxActiveLayerEnabled->isChecked());
		m_ui->labelSurfaceHeatingIndex->setEnabled(m_ui->checkBoxActiveLayerEnabled->isChecked());
		m_ui->spinBoxActiveLayerIndex->setMaximum((int)con->m_materialLayers.size()+1);
		if (m_current->m_activeLayerIndex != VICUS::INVALID_ID)
			m_ui->spinBoxActiveLayerIndex->setValue((int)m_current->m_activeLayerIndex+1);
		m_ui->spinBoxActiveLayerIndex->blockSignals(false);

		QVector<QtExt::ConstructionLayer> layers;
		for (unsigned int i=0; i<con->m_materialLayers.size(); ++i) {
			QtExt::ConstructionLayer layer;
			unsigned int matID = con->m_materialLayers[i].m_idMaterial;
			const VICUS::Material * mat = m_db->m_materials[matID];
			if (mat != nullptr) {
				layer.m_name = QString::fromStdString(mat->m_displayName("de", true));
	//			layer.m_color = mat->m_color;
			}
			else {
				layer.m_name = tr("<select material>");
			}

			layer.m_width = con->m_materialLayers[i].m_thickness.value;
	//		if (!layer.m_color.isValid())
				layer.m_color = QtExt::ConstructionView::ColorList[i % 12];
			layer.m_id = (int)matID;
			layers.push_back(layer);
		}
		m_ui->graphicsViewConstruction->m_leftSideLabel = tr("Sida A");
		m_ui->graphicsViewConstruction->m_rightSideLabel = tr("Sida B");
		m_ui->graphicsViewConstruction->setData(this, layers, 1.0,
												QtExt::ConstructionGraphicsScene::VI_BoundaryLabels |
												QtExt::ConstructionGraphicsScene::VI_MaterialNames);
	}
	else {
		m_ui->checkBoxActiveLayerEnabled->setEnabled(false);
		m_ui->checkBoxActiveLayerEnabled->setChecked(false);
		m_ui->spinBoxActiveLayerIndex->setEnabled(false);
		m_ui->labelSurfaceHeatingIndex->setEnabled(false);
		m_ui->spinBoxActiveLayerIndex->setValue(1);
		m_ui->lineEditUValue->setText("---");
		m_ui->lineEditConstructionName->setText("");
		m_ui->graphicsViewConstruction->clear();
	}

	// for built-ins, disable editing/make read-only
	bool isEditable = !comp->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
	m_ui->comboBoxComponentType->setEnabled(isEditable);
	m_ui->toolButtonSelectConstruction->setEnabled(isEditable);
	m_ui->toolButtonSelectBoundaryConditionSideAName->setEnabled(isEditable);
	m_ui->toolButtonSelectBoundaryConditionSideBName->setEnabled(isEditable);
	m_ui->toolButtonRemoveConstruction->setEnabled(isEditable);
	m_ui->toolButtonRemoveBoundaryConditionSideA->setEnabled(isEditable);
	m_ui->toolButtonRemoveBoundaryConditionSideB->setEnabled(isEditable);
	m_ui->pushButtonDaylight->setEnabled(isEditable);

	m_ui->lineEditBoundaryConditionSideAName->setReadOnly(!isEditable);
	m_ui->lineEditBoundaryConditionSideBName->setReadOnly(!isEditable);

	if (!isEditable) {
		m_ui->checkBoxActiveLayerEnabled->setEnabled(false);
		m_ui->spinBoxActiveLayerIndex->setEnabled(false);
		m_ui->labelSurfaceHeatingIndex->setEnabled(false);
	}

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
	if (conId != VICUS::INVALID_ID && conId != m_current->m_idConstruction) {
		m_current->m_idConstruction = conId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBComponentEditWidget::on_toolButtonSelectBoundaryConditionSideAName_clicked() {
	// get boundary condition edit dialog from mainwindow
	SVDatabaseEditDialog * bcEditDialog = SVMainWindow::instance().dbBoundaryConditionEditDialog();
	unsigned int bcId = bcEditDialog->select(m_current->m_idSideABoundaryCondition);
	if (bcId != VICUS::INVALID_ID && bcId != m_current->m_idSideABoundaryCondition) {
		m_current->m_idSideABoundaryCondition = bcId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBComponentEditWidget::on_toolButtonSelectBoundaryConditionSideBName_clicked() {
	// get boundary condition edit dialog from mainwindow
	SVDatabaseEditDialog * bcEditDialog = SVMainWindow::instance().dbBoundaryConditionEditDialog();
	unsigned int bcId = bcEditDialog->select(m_current->m_idSideBBoundaryCondition);
	if (bcId != VICUS::INVALID_ID && bcId != m_current->m_idSideBBoundaryCondition) {
		m_current->m_idSideBBoundaryCondition = bcId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBComponentEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}

}


void SVDBComponentEditWidget::modelModify(){
	m_db->m_components.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	SVProjectHandler::instance().setModified(SVProjectHandler::ComponentInstancesModified);
}


void SVDBComponentEditWidget::on_toolButtonRemoveConstruction_clicked() {

	m_current->m_idConstruction = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}


void SVDBComponentEditWidget::on_toolButtonRemoveBoundaryConditionSideA_clicked() {

	m_current->m_idSideABoundaryCondition = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}


void SVDBComponentEditWidget::on_toolButtonRemoveBoundaryConditionSideB_clicked() {

	m_current->m_idSideBBoundaryCondition = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}

void SVDBComponentEditWidget::on_checkBoxActiveLayerEnabled_toggled(bool checked) {

	m_ui->spinBoxActiveLayerIndex->setEnabled(checked);
	m_ui->labelSurfaceHeatingIndex->setEnabled(checked);
	if (checked) {
		m_current->m_activeLayerIndex = (unsigned int)m_ui->spinBoxActiveLayerIndex->value()-1;
		modelModify();
	}
	else {

		bool askForDeletion = true;
		// We have to make a special handling if the component is applied in component instances
		// Then we also have handle the surface heating ID reset in the specific component instance
		for (const VICUS::ComponentInstance &ci : SVProjectHandler::instance().project().m_componentInstances) {
			VICUS::ComponentInstance & compInst = const_cast<VICUS::ComponentInstance &>(ci);
			if (compInst.m_idComponent == m_current->m_id) {

				// if we have assigned components in component instances
				if (askForDeletion) {
					m_ui->checkBoxActiveLayerEnabled->blockSignals(true);
					m_ui->spinBoxActiveLayerIndex->setEnabled(true);
					m_ui->labelSurfaceHeatingIndex->setEnabled(true);
					m_ui->checkBoxActiveLayerEnabled->setChecked(true);

					// delete dialog
					askForDeletion = false;
					QMessageBox dlg(QMessageBox::Question, tr("Delete surface heating"), tr("Do you want to delete surface heating in component instances?"), QMessageBox::Cancel, this);

					QPushButton * btn = new QPushButton("Yes");
					dlg.addButton(btn, (QMessageBox::ButtonRole)(QMessageBox::YesRole));
					dlg.setDefaultButton(btn);
					if(dlg.exec() == QMessageBox::Cancel) {
						// user aborts the deletion
						m_ui->checkBoxActiveLayerEnabled->blockSignals(false);
						break;
					}
					m_ui->spinBoxActiveLayerIndex->setEnabled(false);
					m_ui->labelSurfaceHeatingIndex->setEnabled(checked);
					m_ui->checkBoxActiveLayerEnabled->setChecked(false);
					m_ui->checkBoxActiveLayerEnabled->blockSignals(false);
				}
				// we reset the surface heating
				compInst.m_idSurfaceHeating = VICUS::INVALID_ID; // reset ID since we do not have anymore a component with active layer
			}
		}

		m_current->m_activeLayerIndex = VICUS::INVALID_ID;

		modelModify();
	}
}


void SVDBComponentEditWidget::on_spinBoxActiveLayerIndex_valueChanged(int arg1) {
	m_current->m_activeLayerIndex = (unsigned int)arg1;
	modelModify();
}
