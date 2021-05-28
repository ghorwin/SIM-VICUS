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
	m_ui->lineEditName->setDialog3Caption(tr("Pipe"));

	m_ui->lineEditWallLambda->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Thermal conductivity"), false, true);
	m_ui->lineEditOuterDiameter->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Outer diameter"), true, true);
	m_ui->lineEditWallRoughness->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Roughness"), true, true);
	m_ui->lineEditWallThickness->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Wall thickness"), true, true);
	m_ui->lineEditInsulationLambda->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Thermal conductivity of the insulation material"), true, true);
	m_ui->lineEditInsulationThickness->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Insulation thickness"), true, true);
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

	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditOuterDiameter->clear();
		m_ui->lineEditWallThickness->clear();
		m_ui->lineEditWallLambda->clear();
		m_ui->lineEditWallRoughness->clear();

		return;
	}

	VICUS::NetworkPipe * pipe = const_cast<VICUS::NetworkPipe*>(m_db->m_pipes[(unsigned int)id]);
	m_current = pipe;

	// now update the GUI controls
	m_ui->lineEditName->setString(pipe->m_displayName);
	m_ui->lineEditWallLambda->setValue(pipe->m_lambdaWall);
	m_ui->lineEditOuterDiameter->setValue(pipe->m_diameterOutside);
	m_ui->lineEditWallThickness->setValue(pipe->m_wallThickness);
	m_ui->lineEditWallRoughness->setValue(pipe->m_roughness);

	m_ui->pushButtonPipeColor->blockSignals(true);
	m_ui->pushButtonPipeColor->setColor(pipe->m_color);
	m_ui->pushButtonPipeColor->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (pipe->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->lineEditWallLambda->setReadOnly(!isEditable);
	m_ui->lineEditOuterDiameter->setReadOnly(!isEditable);
	m_ui->lineEditWallThickness->setReadOnly(!isEditable);
	m_ui->lineEditWallRoughness->setReadOnly(!isEditable);
	m_ui->pushButtonPipeColor->setReadOnly(!isEditable);
}


void SVDBPipeEditWidget::on_lineEditOuterDiameter_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_ui->lineEditOuterDiameter->isValid()) {
		m_current->m_diameterOutside = m_ui->lineEditOuterDiameter->value();
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditWallThickness_editingFinished() {
	if (m_ui->lineEditWallThickness->isValid()) {
		m_current->m_wallThickness = m_ui->lineEditWallThickness->value();
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditWallLambda_editingFinished() {
	if (m_ui->lineEditWallLambda->isValid()) {
		m_current->m_lambdaWall = m_ui->lineEditWallLambda->value();
		modelModify();
	}
}


void SVDBPipeEditWidget::on_lineEditWallRoughness_editingFinished() {
	if (m_ui->lineEditWallRoughness->isValid()) {
		m_current->m_roughness = m_ui->lineEditWallRoughness->value();
		modelModify();
	}
}

void SVDBPipeEditWidget::on_lineEditInsulationThickness_editingFinished(){
	if (m_ui->lineEditInsulationThickness->isValid()) {
		m_current->m_insulationThickness = m_ui->lineEditInsulationThickness->value();
		modelModify();
	}
}

void SVDBPipeEditWidget::on_lineEditInsulationLambda_editingFinished(){
	if (m_ui->lineEditInsulationLambda->isValid()) {
		m_current->m_lambdaInsulation = m_ui->lineEditInsulationLambda->value();
		modelModify();
	}
}

void SVDBPipeEditWidget::on_pushButtonPipeColor_colorChanged() {

	if (m_current->m_color != m_ui->pushButtonPipeColor->color()) {
		m_current->m_color = m_ui->pushButtonPipeColor->color();
		modelModify();
	}
}

void SVDBPipeEditWidget::modelModify() {
	m_db->m_pipes.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}
