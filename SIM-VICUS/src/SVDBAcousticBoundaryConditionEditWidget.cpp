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

#include "SVDBAcousticBoundaryConditionEditWidget.h"
#include "ui_SVDBAcousticBoundaryConditionEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Locale.h>
#include <SVConversions.h>

#include "SVDBAcousticBoundaryConditionTableModel.h"
#include "SVConstants.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVStyle.h"

SVDBAcousticBoundaryConditionEditWidget::SVDBAcousticBoundaryConditionEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBAcousticBoundaryConditionEditWidget)
{
	m_ui->setupUi(this);
	//m_ui->verticalLayoutMaster->setMargin(4);

	setMinimumWidth(500);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->tableWidgetSoundAbsorptionLayers->setColumnCount(NumCol);

	QStringList headers;
	headers << tr("ID") << tr("Name") << tr("Area fraction [---]");
	m_ui->tableWidgetSoundAbsorptionLayers->setHorizontalHeaderLabels(headers);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSoundAbsorptionLayers);
	m_ui->tableWidgetSoundAbsorptionLayers->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetSoundAbsorptionLayers->setSortingEnabled(false);
	m_ui->tableWidgetSoundAbsorptionLayers->setColumnWidth(ColName, 200);
	// for changing area fraction
	connect(m_ui->tableWidgetSoundAbsorptionLayers, SIGNAL(itemChanged(QTableWidgetItem *)),
		this, SLOT(tableItemChanged(QTableWidgetItem *)));
	connect(m_ui->tableWidgetSoundAbsorptionLayers, SIGNAL(cellDoubleClicked(int,int)),
			this, SLOT(onLayerChosen(int,int)));


	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBAcousticBoundaryConditionEditWidget::~SVDBAcousticBoundaryConditionEditWidget() {
	delete m_ui;
}


void SVDBAcousticBoundaryConditionEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBAcousticBoundaryConditionTableModel*>(dbModel);
}


void SVDBAcousticBoundaryConditionEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls
		setEnabled(false);

		// clear all inputs

		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->spinBoxLayerCount->setValue(0);
		// disable the line edits (they change background color)
		m_ui->lineEditName->setEnabled(false);
		m_ui->tableWidgetSoundAbsorptionLayers->setRowCount(0);

		return;
	}
	setEnabled(true);

	VICUS::AcousticBoundaryCondition * bc = const_cast<VICUS::AcousticBoundaryCondition *>(m_db->m_acousticBoundaryConditions[(unsigned int)id]);
	m_current = bc;
	m_ui->lineEditName->setString(bc->m_displayName);

	int n = std::max<int>(0, bc->m_soundAbsorptionLayers.size());
	m_ui->spinBoxLayerCount->setValue(n);
	m_ui->lineEditName->setEnabled(!bc->m_builtIn);
	m_ui->spinBoxLayerCount->setEnabled(!bc->m_builtIn);

	QPalette pal;
	if (bc->m_builtIn)
		pal.setColor(QPalette::Base, SVStyle::instance().m_readOnlyEditFieldBackground);
	m_ui->spinBoxLayerCount->setPalette(pal);

	on_spinBoxLayerCount_valueChanged(n);

	updateTable();

}

void SVDBAcousticBoundaryConditionEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}



void SVDBAcousticBoundaryConditionEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBAcousticBoundaryConditionEditWidget::modelModify() {
	m_db->m_acousticBoundaryConditions.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}

void SVDBAcousticBoundaryConditionEditWidget::updateTable() {
	unsigned int rowCount = m_current->m_soundAbsorptionLayers.size();

	m_ui->tableWidgetSoundAbsorptionLayers->blockSignals(true);
	m_ui->tableWidgetSoundAbsorptionLayers->setRowCount(rowCount);

	for (unsigned int i=0; i < rowCount; ++i) {
		const VICUS::SoundAbsorptionLayer &layer = m_current->m_soundAbsorptionLayers[i];
		const VICUS::AcousticSoundAbsorption *soundAbs = m_db->m_acousticSoundAbsorptions[layer.m_idSoundAbsorption];

		Q_ASSERT(soundAbs != nullptr);
		QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(layer.m_idSoundAbsorption));
		idItem->setTextAlignment(Qt::AlignCenter);
		QTableWidgetItem *fractionItem = new QTableWidgetItem(QString::number(layer.m_para[VICUS::SoundAbsorptionLayer::P_AreaFraction].value));
		fractionItem->setTextAlignment(Qt::AlignCenter);

		m_ui->tableWidgetSoundAbsorptionLayers->setItem(i, ColId, idItem);
		m_ui->tableWidgetSoundAbsorptionLayers->setButtonAndText(i, ColName, QString::fromStdString(soundAbs->m_displayName.string()));
		m_ui->tableWidgetSoundAbsorptionLayers->setItem(i, ColFraction,fractionItem);

	}
	m_ui->tableWidgetSoundAbsorptionLayers->setEnabled(!m_current->m_builtIn);
	m_ui->tableWidgetSoundAbsorptionLayers->blockSignals(false);
}


void SVDBAcousticBoundaryConditionEditWidget::on_spinBoxLayerCount_valueChanged(int layerCount) {
	if (m_current == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty",

	m_ui->tableWidgetSoundAbsorptionLayers->setRowCount(layerCount);
	// only update, if number of layers has changed
	if ((int)m_current->m_soundAbsorptionLayers.size() != layerCount) {
		// update content of table widget based on data in m_current
		while (m_current->m_soundAbsorptionLayers.size() < static_cast<unsigned int>(layerCount)) {
			unsigned int defaultSoundAbsorptionId = 0;
			if (!m_db->m_acousticSoundAbsorptions.empty())
				defaultSoundAbsorptionId = m_db->m_acousticSoundAbsorptions.begin()->first;
			VICUS::SoundAbsorptionLayer layer(0.1, defaultSoundAbsorptionId);
			m_current->m_soundAbsorptionLayers.push_back(layer);
		}
		// shrink vectors
		m_current->m_soundAbsorptionLayers.resize(layerCount);
		modelModify();

		updateTable();
		//updateConstructionView();
	}
}


void SVDBAcousticBoundaryConditionEditWidget::tableItemChanged(QTableWidgetItem *item) {
	Q_ASSERT(item->column() == 2);
	Q_ASSERT(m_current != nullptr);

	bool ok;
	double val = QtExt::Locale().toDouble(item->text(), &ok); // val in cm
	int row = item->row();
	int col = item->column();

	unsigned int soundAbsLayerIdx = (unsigned int)row;
	if(col == 2) {
		Q_ASSERT(soundAbsLayerIdx < m_current->m_soundAbsorptionLayers.size());
		if (!ok || val < 0) {
			if (!ok) {
				QTableWidgetItem * item2 = m_ui->tableWidgetSoundAbsorptionLayers->item(row, col);
				item2->setBackground(QBrush(SVStyle::instance().m_errorEditFieldBackground));
				item2->setToolTip(tr("Invalid number format, please enter a valid decimal number!"));
			}
			else {
				QTableWidgetItem * item2 = m_ui->tableWidgetSoundAbsorptionLayers->item(row, col);
				item2->setBackground(QBrush(SVStyle::instance().m_errorEditFieldBackground));
				item2->setToolTip(tr("Area fraction values must be larger than 0. Ignore smaller area fractions!"));
			}
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			return;
		}
		else {
			QTableWidgetItem * item2 = m_ui->tableWidgetSoundAbsorptionLayers->item(row, col);
			item2->setBackground(QBrush());
		}
		// we only accept changes up to 0.01  as different
		if (!IBK::nearly_equal<4>(m_current->m_soundAbsorptionLayers[soundAbsLayerIdx].m_para[VICUS::SoundAbsorptionLayer::P_AreaFraction].value, val)) {
			m_current->m_soundAbsorptionLayers[soundAbsLayerIdx].m_para[VICUS::SoundAbsorptionLayer::P_AreaFraction].value = val;
			modelModify();
		}
	}
}

void SVDBAcousticBoundaryConditionEditWidget::onLayerChosen(int row, int column) {
	if(column != 1)
		return;

	if(m_current == nullptr || m_current->m_builtIn)
		return;

	showSoundAbsorptionSelectionDialog(row);
}

void SVDBAcousticBoundaryConditionEditWidget::showSoundAbsorptionSelectionDialog(int index) {
	Q_ASSERT(index >= 0);

	// get material edit dialog (owned/managed by main window)
	SVDatabaseEditDialog * soundAbsSelect = SVMainWindow::instance().dbAcousticSoundAbsorptionEditDialog();
	// ask to select a material
	unsigned int soundAbsId = soundAbsSelect->select(m_current->m_soundAbsorptionLayers[(unsigned int)index].m_idSoundAbsorption);
	if (soundAbsId == VICUS::INVALID_ID)
		return; // dialog was canceled, no change here
	if (soundAbsId != m_current->m_soundAbsorptionLayers[(unsigned int)index].m_idSoundAbsorption) {
		m_current->m_soundAbsorptionLayers[(unsigned int)index].m_idSoundAbsorption = soundAbsId;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
	updateTable();
}
