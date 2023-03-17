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

#include "SVDBMaterialEditWidget.h"
#include "ui_SVDBMaterialEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>
#include <SVConstants.h>

#include "SVDBMaterialTableModel.h"


SVDBMaterialEditWidget::SVDBMaterialEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBMaterialEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(4);
	// adjust color botton width
	m_ui->pushButtonColor->setMaximumSize(m_ui->lineEditName->height(), m_ui->lineEditName->height());
	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Material identification name"));


	m_ui->lineEditDataSource->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(),THIRD_LANGUAGE, true);
	m_ui->lineEditDataSource->setDialog3Caption(tr("Data source information"));
	m_ui->lineEditManufacturer->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditManufacturer->setDialog3Caption(tr("Manufacturer name"));
	m_ui->lineEditNotes->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditNotes->setDialog3Caption(tr("Notes"));

	m_ui->lineEditDensity->setup(0.01, 10000, tr("Density"), true, true);
	m_ui->lineEditConductivity->setup(1e-5, 500, tr("Thermal conductivity"), true, true);
	m_ui->lineEditSpecHeatCapacity->setup(100, 5000, tr("Specific heat capacity"), true, true);

	// enter categories into combo box
	// block signals to avoid getting "changed" calls
	m_ui->comboBoxCategory->blockSignals(true);
	for (int i=0; i<VICUS::Material::NUM_MC; ++i)
		m_ui->comboBoxCategory->addItem(VICUS::KeywordListQt::Keyword("Material::Category", i), i);
	m_ui->comboBoxCategory->blockSignals(false);

	// color button size fix (overriding style settings)

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBMaterialEditWidget::~SVDBMaterialEditWidget() {
	delete m_ui;
}


void SVDBMaterialEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBMaterialTableModel*>(dbModel);
}


void SVDBMaterialEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	setMinimumWidth(500);

	if (id == -1) {
		// disable all controls
		m_ui->tabWidgetThermal->setEnabled(false); // makes all inactive

		// clear input controls
		m_ui->lineEditDensity->setText("");
		m_ui->lineEditConductivity->setText("");
		m_ui->lineEditSpecHeatCapacity->setText("");
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditDataSource->setString(IBK::MultiLanguageString());
		m_ui->lineEditManufacturer->setString(IBK::MultiLanguageString());
		m_ui->lineEditNotes->setString(IBK::MultiLanguageString());

		return;
	}
	m_ui->tabWidgetThermal->setEnabled(true);

	VICUS::Material * mat = const_cast<VICUS::Material *>(m_db->m_materials[(unsigned int)id]);
	m_current = mat;

	// now update the GUI controls

	m_ui->lineEditDensity->setValue(mat->m_para[VICUS::Material::P_Density].value);
	m_ui->lineEditConductivity->setValue(mat->m_para[VICUS::Material::P_Conductivity].value);
	m_ui->lineEditSpecHeatCapacity->setValue(mat->m_para[VICUS::Material::P_HeatCapacity].value);
	m_ui->lineEditName->setString(mat->m_displayName);
	m_ui->lineEditDataSource->setString(mat->m_dataSource);
	m_ui->lineEditManufacturer->setString(mat->m_manufacturer);
	m_ui->lineEditNotes->setString(mat->m_notes);
	m_ui->comboBoxCategory->setCurrentIndex(mat->m_category);
	m_ui->pushButtonColor->setColor(mat->m_color);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (mat->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->lineEditDataSource->setReadOnly(!isEditable);
	m_ui->lineEditManufacturer->setReadOnly(!isEditable);
	m_ui->lineEditNotes->setReadOnly(!isEditable);
	m_ui->lineEditDensity->setReadOnly(!isEditable);
	m_ui->lineEditConductivity->setReadOnly(!isEditable);
	m_ui->lineEditSpecHeatCapacity->setReadOnly(!isEditable);
	m_ui->comboBoxCategory->setEnabled(isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
}


void SVDBMaterialEditWidget::setCurrentTabIndex(int idx) {
	m_ui->tabWidgetThermal->setCurrentIndex(idx);
}


void SVDBMaterialEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBMaterialEditWidget::on_lineEditDataSource_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_dataSource != m_ui->lineEditDataSource->string()) {
		m_current->m_dataSource = m_ui->lineEditDataSource->string();
		modelModify();
	}
}


void SVDBMaterialEditWidget::on_lineEditManufacturer_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_manufacturer != m_ui->lineEditManufacturer->string()) {
		m_current->m_manufacturer = m_ui->lineEditManufacturer->string();
		m_db->m_materials.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBMaterialEditWidget::on_lineEditNotes_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_notes != m_ui->lineEditNotes->string()) {
		m_current->m_notes = m_ui->lineEditNotes->string();
		modelModify();
	}
}


void SVDBMaterialEditWidget::on_lineEditConductivity_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditConductivity->isValid() ) {
		double val = m_ui->lineEditConductivity->value();
		// update database but only if different from original
		if (m_current->m_para[VICUS::Material::P_Conductivity].empty() ||
			val != m_current->m_para[VICUS::Material::P_Conductivity].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "Material::para_t", VICUS::Material::P_Conductivity, val);
			modelModify();
		}
	}
}


void SVDBMaterialEditWidget::on_lineEditDensity_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditDensity->isValid() ) {
		double val = m_ui->lineEditDensity->value();
		// update database but only if different from original
		VICUS::Material::para_t paraName = VICUS::Material::P_Density;
		if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "Material::para_t", paraName, val);
			modelModify();
		}
	}

}


void SVDBMaterialEditWidget::on_lineEditSpecHeatCapacity_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditSpecHeatCapacity->isValid() ) {
		double val = m_ui->lineEditSpecHeatCapacity->value();
		VICUS::Material::para_t paraName = VICUS::Material::P_HeatCapacity;
		if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "Material::para_t", paraName, val);
			modelModify();
		}
	}
}


void SVDBMaterialEditWidget::on_comboBoxCategory_currentIndexChanged(int index){
	Q_ASSERT(m_current != nullptr);

	// update database but only if different from original
	if (index != (int)m_current->m_category)
	{
		m_current->m_category = static_cast<VICUS::Material::Category>(index);
		modelModify();
	}
}


void SVDBMaterialEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}

void SVDBMaterialEditWidget::modelModify() {
	m_db->m_materials.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}


