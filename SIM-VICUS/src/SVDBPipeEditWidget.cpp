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

#include "SVDBPipeEditWidget.h"
#include "ui_SVDBPipeEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"
#include "SVDBPipeTableModel.h"

#include <VICUS_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <QtExt_LanguageHandler.h>
#include <SVConstants.h>

SVDBPipeEditWidget::SVDBPipeEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBPipeEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Pipe identification name"));

	// The name is not supposed to be changed by the user! Its still possible to use whatever category.
	m_ui->lineEditName->setReadOnly(true);

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditCategory->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditCategory->setDialog3Caption(tr("Pipe category"));

	m_ui->lineEditWallLambda->setup(0, std::numeric_limits<double>::max(), tr("Thermal conductivity"), false, true);
	m_ui->lineEditOuterDiameter->setup(0, std::numeric_limits<double>::max(), tr("Outer diameter"), false, true);
	m_ui->lineEditWallRoughness->setup(0, std::numeric_limits<double>::max(), tr("Roughness"), false, true);
	m_ui->lineEditWallThickness->setup(0, std::numeric_limits<double>::max(), tr("Wall thickness"), false, true);
	m_ui->lineEditInsulationLambda->setup(0, std::numeric_limits<double>::max(), tr("Thermal conductivity of the insulation material"), false, true);
	m_ui->lineEditInsulationThickness->setup(0, std::numeric_limits<double>::max(), tr("Insulation thickness"), true, true);
	m_ui->lineEditWallHeatCapacity->setup(0, std::numeric_limits<double>::max(), tr("Pipe wall heat capacity"), false, true);
	m_ui->lineEditWallDensity->setup(0, std::numeric_limits<double>::max(), tr("Pipe wall density"), false, true);

}


SVDBPipeEditWidget::~SVDBPipeEditWidget() {
	delete m_ui;
}


void SVDBPipeEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBPipeTableModel*>(dbModel);
}


void SVDBPipeEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;

	m_ui->lineEditName->setEnabled(isEnabled);
	m_ui->lineEditOuterDiameter->setEnabled(isEnabled);
	m_ui->lineEditWallThickness->setEnabled(isEnabled);
	m_ui->lineEditWallLambda->setEnabled(isEnabled);
	m_ui->lineEditWallRoughness->setEnabled(isEnabled);
	m_ui->lineEditInsulationLambda->setEnabled(isEnabled);
	m_ui->lineEditInsulationThickness->setEnabled(isEnabled);
	m_ui->lineEditWallHeatCapacity->setEnabled(isEnabled);
	m_ui->lineEditWallDensity->setEnabled(isEnabled);
	m_ui->pushButtonColor->setEnabled(isEnabled);

	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditCategory->setString(IBK::MultiLanguageString());
		m_ui->lineEditOuterDiameter->clear();
		m_ui->lineEditWallThickness->clear();
		m_ui->lineEditWallLambda->clear();
		m_ui->lineEditWallRoughness->clear();
		m_ui->lineEditInsulationLambda->clear();
		m_ui->lineEditInsulationThickness->clear();
		m_ui->lineEditWallHeatCapacity->clear();
		m_ui->lineEditWallDensity->clear();
		m_ui->lineEditUValue->clear();
		m_ui->pushButtonColor->setColor(Qt::gray);

		m_autoNameGeneration = false;
		return;
	}

	VICUS::NetworkPipe * pipe = const_cast<VICUS::NetworkPipe*>(m_db->m_pipes[(unsigned int)id]);
	m_current = pipe;
	Q_ASSERT(m_current != nullptr);

	// enable auto-naming if the current displayname matches the auto-generated name
	IBK::MultiLanguageString defaultName = m_current->nameFromData();
	m_autoNameGeneration = (m_current->m_displayName == defaultName);

	// now update the GUI controls
	m_ui->lineEditName->setString(pipe->m_displayName);
	m_ui->lineEditCategory->setString(pipe->m_categoryName);

	// Mind: some parameters may not be set, yet
	//       however, we expect the parameter to hold values converted to the base SI-unit -> if no unit, or an
	//       invalid unit is given, we might just show invalid values, but there are invalid anyway
	m_ui->lineEditWallLambda->setValue(pipe->m_para[VICUS::NetworkPipe::P_ThermalConductivityWall].value);
	m_ui->lineEditOuterDiameter->setValue(pipe->m_para[VICUS::NetworkPipe::P_DiameterOutside].value * 1000);
	m_ui->lineEditWallThickness->setValue(pipe->m_para[VICUS::NetworkPipe::P_ThicknessWall].value * 1000);
	m_ui->lineEditWallRoughness->setValue(pipe->m_para[VICUS::NetworkPipe::P_RoughnessWall].value * 1000);
	m_ui->lineEditInsulationLambda->setValue(pipe->m_para[VICUS::NetworkPipe::P_ThermalConductivityInsulation].value);
	m_ui->lineEditInsulationThickness->setValue(pipe->m_para[VICUS::NetworkPipe::P_ThicknessInsulation].value * 1000);
	m_ui->lineEditWallHeatCapacity->setValue(pipe->m_para[VICUS::NetworkPipe::P_HeatCapacityWall].value);
	m_ui->lineEditWallDensity->setValue(pipe->m_para[VICUS::NetworkPipe::P_DensityWall].value);
	if (pipe->isValid())
		m_ui->lineEditUValue->setText(QString("%L1").arg(pipe->UValue(),0, 'f', 3));

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(pipe->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (pipe->m_builtIn)
		isEditable = false;

	m_ui->lineEditCategory->setReadOnly(!isEditable);
	m_ui->lineEditWallLambda->setReadOnly(!isEditable);
	m_ui->lineEditOuterDiameter->setReadOnly(!isEditable);
	m_ui->lineEditWallThickness->setReadOnly(!isEditable);
	m_ui->lineEditWallRoughness->setReadOnly(!isEditable);
	m_ui->lineEditInsulationLambda->setReadOnly(!isEditable);
	m_ui->lineEditInsulationThickness->setReadOnly(!isEditable);
	m_ui->lineEditWallHeatCapacity->setReadOnly(!isEditable);
	m_ui->lineEditWallDensity->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
}


void SVDBPipeEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		m_autoNameGeneration = false;
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditOuterDiameter_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);
	if (m_ui->lineEditOuterDiameter->isValid()) {
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkPipe::para_t", VICUS::NetworkPipe::P_DiameterOutside,
										 m_ui->lineEditOuterDiameter->value());
		updateNameFromData();
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditWallThickness_editingFinishedSuccessfully() {
	if (m_ui->lineEditWallThickness->isValid()) {
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkPipe::para_t", VICUS::NetworkPipe::P_ThicknessWall,
										 m_ui->lineEditWallThickness->value());
		updateNameFromData();
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditWallLambda_editingFinishedSuccessfully() {
	if (m_ui->lineEditWallLambda->isValid()) {
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkPipe::para_t", VICUS::NetworkPipe::P_ThermalConductivityWall,
										 m_ui->lineEditWallLambda->value());
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditWallRoughness_editingFinishedSuccessfully() {
	if (m_ui->lineEditWallRoughness->isValid()) {
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkPipe::para_t", VICUS::NetworkPipe::P_RoughnessWall,
										 m_ui->lineEditWallRoughness->value());
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditInsulationThickness_editingFinishedSuccessfully() {
	if (m_ui->lineEditInsulationThickness->isValid()) {
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkPipe::para_t", VICUS::NetworkPipe::P_ThicknessInsulation,
										 m_ui->lineEditInsulationThickness->value());
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditInsulationLambda_editingFinishedSuccessfully() {
	if (m_ui->lineEditInsulationLambda->isValid()) {
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkPipe::para_t", VICUS::NetworkPipe::P_ThermalConductivityInsulation,
										 m_ui->lineEditInsulationLambda->value());
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditWallHeatCapacity_editingFinished() {
	if (m_ui->lineEditWallHeatCapacity->isValid()) {
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkPipe::para_t", VICUS::NetworkPipe::P_HeatCapacityWall,
										 m_ui->lineEditWallHeatCapacity->value());
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditWallDensity_editingFinished() {
	if (m_ui->lineEditWallDensity->isValid()) {
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkPipe::para_t", VICUS::NetworkPipe::P_DensityWall,
										 m_ui->lineEditWallDensity->value());
		modelModify();
	}
}


void SVDBPipeEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditCategory_editingFinished() {
	if (m_current->m_categoryName != m_ui->lineEditCategory->string()) {
		m_current->m_categoryName = m_ui->lineEditCategory->string();
		updateNameFromData();
		modelModify();
	}
}


void SVDBPipeEditWidget::modelModify() {
	m_db->m_pipes.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}


void SVDBPipeEditWidget::updateNameFromData() {
	if (m_autoNameGeneration) {
		IBK::MultiLanguageString newName = m_current->nameFromData();
		m_current->m_displayName = newName;
		// Note: no need to call modelModify here, since it is called anyway from callers of this function
		m_ui->lineEditName->setString(m_current->m_displayName);
	}
}
