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

#include "SVDBAcousticSoundAbsorptionEditWidget.h"
#include "ui_SVDBAcousticSoundAbsorptionEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <QtExt_Locale.h>
#include <QtExt_LanguageHandler.h>
#include <SVConversions.h>

#include "SVDBAcousticSoundAbsorptionTableModel.h"
#include "SVConstants.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVStyle.h"

SVDBAcousticSoundAbsorptionEditWidget::SVDBAcousticSoundAbsorptionEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBAcousticSoundAbsorptionEditWidget)
{
	m_ui->setupUi(this);
	//m_ui->verticalLayoutMaster->setMargin(4);

	setMinimumWidth(500);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->tableWidgetSoundAbsorptions->setColumnCount(NumCol);

	QStringList headers;
	headers << tr("Frequency [Hz]") << tr("Absorption value [---]");
	m_ui->tableWidgetSoundAbsorptions->setHorizontalHeaderLabels(headers);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSoundAbsorptions);
	m_ui->tableWidgetSoundAbsorptions->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetSoundAbsorptions->setSortingEnabled(false);

	// for changing thickness
	connect(m_ui->tableWidgetSoundAbsorptions, SIGNAL(itemChanged(QTableWidgetItem *)),
		this, SLOT(tableItemChanged(QTableWidgetItem *)));
	// for update construction view selection if table selection has changed
	connect(m_ui->tableWidgetSoundAbsorptions, SIGNAL(itemClicked(QTableWidgetItem*)),
			this, SLOT(tableItemClicked(QTableWidgetItem *)));
	connect(m_ui->tableWidgetSoundAbsorptions, SIGNAL(cellDoubleClicked(int,int)),
			this, SLOT(onCellDoubleClicked(int,int)));

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBAcousticSoundAbsorptionEditWidget::~SVDBAcousticSoundAbsorptionEditWidget() {
	delete m_ui;
}


void SVDBAcousticSoundAbsorptionEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBAcousticSoundAbsorptionTableModel*>(dbModel);
}


void SVDBAcousticSoundAbsorptionEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls
		setEnabled(false);

		// clear all inputs

		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		// disable the line edits (they change background color)
		m_ui->lineEditName->setEnabled(false);
		m_ui->tableWidgetSoundAbsorptions->setRowCount(VICUS::AcousticSoundAbsorption::NUM_SF);

		return;
	}
	setEnabled(true);

	VICUS::AcousticSoundAbsorption * bc = const_cast<VICUS::AcousticSoundAbsorption*>(m_db->m_acousticSoundAbsorptions[(unsigned int)id]);
	m_current = bc;
	m_ui->lineEditName->setString(bc->m_displayName);

	m_ui->lineEditName->setEnabled(!bc->m_builtIn);

	updateTable();

}

void SVDBAcousticSoundAbsorptionEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}



void SVDBAcousticSoundAbsorptionEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBAcousticSoundAbsorptionEditWidget::modelModify() {
	m_db->m_acousticSoundAbsorptions.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}

void SVDBAcousticSoundAbsorptionEditWidget::updateTable() {
	unsigned int rowCount = VICUS::AcousticSoundAbsorption::NUM_SF;

	m_ui->tableWidgetSoundAbsorptions->blockSignals(true);
	m_ui->tableWidgetSoundAbsorptions->setRowCount((int)rowCount);

	std::vector<double> frequencies{125,250,500,1000,2000,4000};

	for (unsigned int i=0; i < rowCount; ++i) {
		int row = (int)i;
		QTableWidgetItem *item = new QTableWidgetItem(QString::number(frequencies[i], 'd', 3));
		m_ui->tableWidgetSoundAbsorptions->setItem(row, ColFrequency, item);
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		item->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetSoundAbsorptions->setItem((int)i, ColValue, new QTableWidgetItem(QString::number(m_current->m_soundAbsorption[i], 'd', 3)));
		m_ui->tableWidgetSoundAbsorptions->item((int)i, ColValue)->setTextAlignment(Qt::AlignCenter);

	}
	m_ui->tableWidgetSoundAbsorptions->setEnabled(!m_current->m_builtIn);
	m_ui->tableWidgetSoundAbsorptions->blockSignals(false);
}




//void SVDBAcousticSoundAbsorptionEditWidget::on_tableWidgetSoundAbsorptions_cellChanged(int row, int column) {
//	if(column != 1)
//		return;

//	QTableWidgetItem *item = m_ui->tableWidgetSoundAbsorptions->item(row, column);

//	QString str = item->text();
//	bool isOk;
//	double val = QtExt::Locale().toDouble(item->text(), &isOk); // val in ---

//	if(!isOk)
//		item->setText(QString::number(m_current->m_soundAbsorption[(unsigned int)row], 'f', 2));
//	else{
//		m_current->m_soundAbsorption[(unsigned int)row] = val;
//		modelModify();
//	}

//}


void SVDBAcousticSoundAbsorptionEditWidget::tableItemChanged(QTableWidgetItem *item) {
	Q_ASSERT(item->column() == 1);
	Q_ASSERT(m_current != nullptr);

	bool ok;
	double val = QtExt::Locale().toDouble(item->text(), &ok);
	int row = item->row();
	int col = item->column();

	unsigned int soundAbsLayerIdx = (unsigned int)row;
	if(col == 1) {
		Q_ASSERT(soundAbsLayerIdx < VICUS::AcousticSoundAbsorption::NUM_SF);
		if (!ok || val < 0 || val > 1) {
			if (!ok) {
				QTableWidgetItem * item2 = m_ui->tableWidgetSoundAbsorptions->item(row, col);
				item2->setBackground(QBrush(SVStyle::instance().m_errorEditFieldBackground));
				item2->setToolTip(tr("Invalid number format, please enter a valid decimal number!"));
			}
			else {
				QTableWidgetItem * item2 = m_ui->tableWidgetSoundAbsorptions->item(row, col);
				item2->setBackground(QBrush(SVStyle::instance().m_errorEditFieldBackground));
				item2->setToolTip(tr("Absorption values must be larger than or equal 0 and lower or equal 1!"));
			}
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			return;
		}
		else {
			QTableWidgetItem * item2 = m_ui->tableWidgetSoundAbsorptions->item(row, col);
			item2->setBackground(QBrush());
		}
		// we only accept changes up to 0.0001  as different
		if (!IBK::nearly_equal<4>(m_current->m_soundAbsorption[soundAbsLayerIdx], val)) {
			m_current->m_soundAbsorption[soundAbsLayerIdx] = val;
			modelModify();
		}
	}
	updateTable();
}


