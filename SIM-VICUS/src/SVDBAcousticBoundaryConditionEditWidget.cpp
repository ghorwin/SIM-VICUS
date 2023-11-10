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

	QStringList headerSAL;
	headerSAL << tr("ID") << tr("Name") << "" << tr("Area fraction [---]");
	m_ui->tableWidgetSoundAbsorptionLayers->setHorizontalHeaderLabels(headerSAL);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSoundAbsorptionLayers);
	m_ui->tableWidgetSoundAbsorptionLayers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_ui->tableWidgetSoundAbsorptionLayers->horizontalHeader()->setSectionResizeMode(ColNameButton, QHeaderView::Fixed);
	m_ui->tableWidgetSoundAbsorptionLayers->horizontalHeader()->setSectionResizeMode(ColId, QHeaderView::Fixed);
	m_ui->tableWidgetSoundAbsorptionLayers->horizontalHeader()->setSectionResizeMode(ColFraction, QHeaderView::Fixed);
	m_ui->tableWidgetSoundAbsorptionLayers->setColumnWidth(ColNameButton, 24);
	m_ui->tableWidgetSoundAbsorptionLayers->setColumnWidth(ColId, 100);
	m_ui->tableWidgetSoundAbsorptionLayers->setColumnWidth(ColFraction, 130);
	m_ui->tableWidgetSoundAbsorptionLayers->setSortingEnabled(false);


	unsigned int rowCount = VICUS::AcousticSoundAbsorption::NUM_SF;
	m_ui->tableWidgetResult->setColumnCount(R_NumCol);
	m_ui->tableWidgetResult->setRowCount(rowCount);

	QStringList headerResult;
	headerResult << tr("Frequency [Hz]") << tr("Result");
	m_ui->tableWidgetResult->setHorizontalHeaderLabels(headerResult);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetResult);
	m_ui->tableWidgetResult->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetResult->setSortingEnabled(false);

	std::vector<double> frequencies{125,250,500,1000,2000,4000};


	for(unsigned int i = 0; i < rowCount; ++i){
		QTableWidgetItem *itemFreq = new QTableWidgetItem(QString::number(frequencies[i], 'd', 0));
		itemFreq->setTextAlignment(Qt::AlignCenter);
		itemFreq->setFlags(itemFreq->flags() ^ Qt::ItemIsEditable);
		m_ui->tableWidgetResult->setItem((int)i, ColFrequency, itemFreq);

		QTableWidgetItem *itemResult = new QTableWidgetItem("---");
		itemResult->setTextAlignment(Qt::AlignCenter);
		itemResult->setFlags(itemResult->flags() ^ Qt::ItemIsEditable);
		m_ui->tableWidgetResult->setItem((int)i, ColResult, itemResult);
	}


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

    int n = std::max<int>(0, bc->m_acousticSoundAbsorptionPartitions.size());
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
	Q_ASSERT(m_current != nullptr);

	unsigned int rowCount = m_current->m_acousticSoundAbsorptionPartitions.size();

	m_ui->tableWidgetSoundAbsorptionLayers->blockSignals(true);
	m_ui->tableWidgetSoundAbsorptionLayers->setRowCount(rowCount);

    for (unsigned int i=0; i < rowCount; ++i) {
        const VICUS::AcousticSoundAbsorptionPartition &layer = m_current->m_acousticSoundAbsorptionPartitions[i];
        const VICUS::AcousticSoundAbsorption *soundAbs = m_db->m_acousticSoundAbsorptions[layer.m_idSoundAbsorption];

		QTableWidgetItem *idItem;
		QTableWidgetItem *nameItem;
		QTableWidgetItem *fractionItem;

		if(soundAbs == nullptr){
			QBrush brush(Qt::red);
			idItem = new QTableWidgetItem(QString("Invalid"));
			nameItem = new QTableWidgetItem(QString("Invalid"));
			fractionItem = new QTableWidgetItem(QString::number(0));
			idItem->setForeground(brush);
			nameItem->setForeground(brush);
			fractionItem->setForeground(brush);
		} else
		{
			idItem = new QTableWidgetItem(QString::number(layer.m_idSoundAbsorption));
			nameItem = new QTableWidgetItem(QString::fromStdString(soundAbs->m_displayName.string()));
			fractionItem = new QTableWidgetItem(QString::number(layer.m_para[VICUS::AcousticSoundAbsorptionPartition::P_AreaFraction].value));
		}

		idItem->setTextAlignment(Qt::AlignCenter);
		nameItem->setTextAlignment(Qt::AlignCenter);
		fractionItem->setTextAlignment(Qt::AlignCenter);

		idItem->setFlags(idItem->flags() ^ Qt::ItemIsEditable);
		nameItem->setFlags(nameItem->flags() ^ Qt::ItemIsEditable);

		m_ui->tableWidgetSoundAbsorptionLayers->setItem(i, ColId, idItem);
		m_ui->tableWidgetSoundAbsorptionLayers->setItem(i, ColName, nameItem);
		m_ui->tableWidgetSoundAbsorptionLayers->setButton(i, ColNameButton);
		m_ui->tableWidgetSoundAbsorptionLayers->setItem(i, ColFraction,fractionItem);
	}

	fillTableWidgetResult();
	m_ui->tableWidgetSoundAbsorptionLayers->setEnabled(!m_current->m_builtIn);
	m_ui->tableWidgetSoundAbsorptionLayers->blockSignals(false);
}

void SVDBAcousticBoundaryConditionEditWidget::fillTableWidgetResult()
{
	if(m_current->m_acousticSoundAbsorptionPartitions.size() == 0){
		m_ui->tableWidgetResult->setEnabled(false);
		for(int i = 0; i < VICUS::AcousticSoundAbsorption::NUM_SF ; ++i){
			QTableWidgetItem *item = m_ui->tableWidgetResult->item(i, ColResult);
			item->setText("---");
		}
	} else {
		m_ui->tableWidgetResult->setEnabled(true);
	}

	m_ui->tableWidgetResult->blockSignals(true);

	// initialises vector to hold all sums while iterating over all layers
	// initialises values to 0
	std::vector<double> sums(VICUS::AcousticSoundAbsorption::NUM_SF, 0);

	//iterates over all layers
	for(unsigned int i = 0; i < m_current->m_acousticSoundAbsorptionPartitions.size(); ++i){
		VICUS::AcousticSoundAbsorption* ac = m_db->m_acousticSoundAbsorptions[m_current->m_acousticSoundAbsorptionPartitions[i].m_idSoundAbsorption];

		if(ac == nullptr){
			// if sound absorption is invalid, table is filled with invalid values
			for(int k = 0; k < VICUS::AcousticSoundAbsorption::NUM_SF ; ++k){
				QTableWidgetItem *item = m_ui->tableWidgetResult->item(i, ColResult);
				item->setText("---");
			}
			m_ui->tableWidgetResult->blockSignals(false);
			return;
		}

		//iterates over all frequencies
		for(unsigned int j = 0; j < VICUS::AcousticSoundAbsorption::NUM_SF; j++){
			sums[j] += ac->m_soundAbsorption[j] * m_current->m_acousticSoundAbsorptionPartitions[i].m_para[VICUS::AcousticSoundAbsorptionPartition::P_AreaFraction].value;
		}
	}


	//updates table with calculated sums
	for(unsigned int i = 0; i < VICUS::AcousticSoundAbsorption::NUM_SF; i++){
		QTableWidgetItem *item = m_ui->tableWidgetResult->item(i, ColResult);
		item->setText(QString::number(sums[i]));
	}

	m_ui->tableWidgetResult->blockSignals(false);
}


void SVDBAcousticBoundaryConditionEditWidget::on_spinBoxLayerCount_valueChanged(int layerCount) {
	if (m_current == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty",

	m_ui->tableWidgetSoundAbsorptionLayers->setRowCount(layerCount);
	// only update, if number of layers has changed
        if ((int)m_current->m_acousticSoundAbsorptionPartitions.size() != layerCount) {
		// update content of table widget based on data in m_current
            while (m_current->m_acousticSoundAbsorptionPartitions.size() < static_cast<unsigned int>(layerCount)) {
			unsigned int defaultSoundAbsorptionId = 0;
			if (!m_db->m_acousticSoundAbsorptions.empty())
				defaultSoundAbsorptionId = m_db->m_acousticSoundAbsorptions.begin()->first;
			VICUS::AcousticSoundAbsorptionPartition layer(0.1, defaultSoundAbsorptionId);
                        m_current->m_acousticSoundAbsorptionPartitions.push_back(layer);
		}
		// shrink vectors
            m_current->m_acousticSoundAbsorptionPartitions.resize(layerCount);
		modelModify();

		updateTable();
		//updateConstructionView();
	}
}


void SVDBAcousticBoundaryConditionEditWidget::tableItemChanged(QTableWidgetItem *item) {
	Q_ASSERT(item->column() == ColFraction);
	Q_ASSERT(m_current != nullptr);

	bool ok;
	double val = QtExt::Locale().toDouble(item->text(), &ok); // val in cm
	int row = item->row();
	int col = item->column();

	unsigned int soundAbsLayerIdx = (unsigned int)row;
        Q_ASSERT(soundAbsLayerIdx < m_current->m_acousticSoundAbsorptionPartitions.size());
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
        if (!IBK::nearly_equal<4>(m_current->m_acousticSoundAbsorptionPartitions[soundAbsLayerIdx].m_para[VICUS::AcousticSoundAbsorptionPartition::P_AreaFraction].value, val)) {
            m_current->m_acousticSoundAbsorptionPartitions[soundAbsLayerIdx].m_para[VICUS::AcousticSoundAbsorptionPartition::P_AreaFraction].value = val;
        modelModify();
    }

    updateTable();

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
		unsigned int idSoundAbsorption = m_current->m_acousticSoundAbsorptionPartitions[index].m_idSoundAbsorption;
	VICUS::AcousticSoundAbsorption *ac = m_db->m_acousticSoundAbsorptions[idSoundAbsorption];

	unsigned int soundAbsId;

	if( ac != nullptr){
		soundAbsId = soundAbsSelect->select(idSoundAbsorption);
	} else {
		soundAbsId = soundAbsSelect->select(VICUS::INVALID_ID);
	}


	if (soundAbsId == VICUS::INVALID_ID)
		return; // dialog was canceled, no change here
		if (soundAbsId != m_current->m_acousticSoundAbsorptionPartitions[(unsigned int)index].m_idSoundAbsorption) {
			m_current->m_acousticSoundAbsorptionPartitions[(unsigned int)index].m_idSoundAbsorption = soundAbsId;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
	updateTable();
}
