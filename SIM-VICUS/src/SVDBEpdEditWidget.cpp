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

#include "SVDBEPDEditWidget.h"
#include "ui_SVDBEPDEditWidget.h"

#include <QtExt_ConstructionLayer.h>
#include <QtExt_ConstructionView.h>
#include <QtExt_ConstructionViewWidget.h>
#include <QtExt_Locale.h>
#include <QtExt_LanguageHandler.h>
#include <SVConversions.h>

#include <QProgressDialog>
#include <QTableWidgetItem>

#include <VICUS_KeywordListQt.h>

#include <IBK_StopWatch.h>
#include <IBK_FileReader.h>
#include <IBK_StringUtils.h>

#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBEpdTableModel.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"



SVDBEPDEditWidget::SVDBEPDEditWidget(QWidget * parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBEPDEditWidget),
	m_current(nullptr)
{
	m_ui->setupUi(this);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetEpdData);
	m_ui->tableWidgetEpdData->setColumnCount(3);
	QStringList header;
	header << "Module" << "GWP" << "ODP" << "POCP" << "AP" << "EP";
	m_ui->tableWidgetEpdData->setHorizontalHeaderLabels(header);

	m_ui->tableWidgetEpdData->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	for (int i=1; i<3; ++i) {
		m_ui->tableWidgetEpdData->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
	}

	updateInput(-1); // update widget for status "nothing selected"
}


SVDBEPDEditWidget::~SVDBEPDEditWidget() {
	delete m_ui;
}


void SVDBEPDEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBEpdTableModel*>(dbModel);
}

void SVDBEPDEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->tableWidgetEpdData->setHidden(id == -1);
	if (id == -1) {
		// clear input controls
		m_ui->lineEditCategory->setString(IBK::MultiLanguageString());
		m_ui->lineEditManufacturer->setText("");
		m_ui->lineEditExpireYear->setText("");
		m_ui->lineEditDataSource->setText("");

		m_ui->tableWidgetEpdData->reset();

		return;
	}

	m_current = const_cast<VICUS::EpdDataset *>(m_db->m_epdDatasets[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditCategory->setString(m_current->m_category);
	m_ui->lineEditDataSource->setText(m_current->m_dataSource);
	m_ui->lineEditExpireYear->setText(m_current->m_expireYear);
	m_ui->lineEditManufacturer->setText(m_current->m_manufacturer);

	m_ui->lineEditRefQuantity->setText(QString("%1").arg(m_current->m_referenceQuantity));
	m_ui->lineEditRefUnit->setText(QString::fromStdString(m_current->m_referenceUnit.name()));

	m_ui->lineEditUUID->setText(m_current->m_uuid);

	std::vector<IBK::Parameter> paras;

	for(unsigned int j=0; j<m_current->m_epdCategoryDataset.size(); ++j) {
		for(unsigned int i=0; i<VICUS::EpdCategoryDataset::NUM_P; ++i) {
			IBK::Parameter &para = m_current->m_epdCategoryDataset[j].m_para[i];

			if(para.empty())
				continue;

			paras.push_back(para);
		}
	}

	m_ui->tableWidgetEpdData->setRowCount((int)paras.size());

	for(unsigned int i=0; i<paras.size(); ++i) {
		IBK::Parameter &para = paras[i];

		m_ui->tableWidgetEpdData->setItem((int)i, 0, new QTableWidgetItem(QString::fromStdString(para.name)));
		m_ui->tableWidgetEpdData->setItem((int)i, 1, new QTableWidgetItem(QString("%1").arg(para.get_value())));
		m_ui->tableWidgetEpdData->setItem((int)i, 2, new QTableWidgetItem(QString::fromStdString(para.unit().name())));
	}

}

SVDBEpdTableModel * SVDBEPDEditWidget::dbModel() const {
	return m_dbModel;
}

