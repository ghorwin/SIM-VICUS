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
#include "SVDBEpdTableModel.h"



SVDBEPDEditWidget::SVDBEPDEditWidget(QWidget * parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBEPDEditWidget),
	m_current(nullptr)
{
	m_ui->setupUi(this);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetEpdData);
	m_ui->tableWidgetEpdData->setColumnCount(8);
	QStringList header;
	header << "Module" << "GWP\n[kg]" << "ODP\n[kg]" << "POCP\n[kg]" << "AP\n[kg]" << "EP\n[kg]" << "PERT\n[W/mK]" << "PENRT\n[W/mK]";
	m_ui->tableWidgetEpdData->setHorizontalHeaderLabels(header);

	m_ui->tableWidgetEpdData->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	for (int i=1; i<8; ++i) {
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
		m_ui->textBrowser->setText("");

		m_ui->tableWidgetEpdData->reset();

		return;
	}

	m_current = const_cast<VICUS::EpdDataset *>(m_db->m_epdDatasets[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditCategory->setString(m_current->m_category);
	m_ui->textBrowser->setText(m_current->m_dataSource);
	m_ui->lineEditExpireYear->setText(m_current->m_expireYear);
	m_ui->lineEditManufacturer->setText(m_current->m_manufacturer);

	m_ui->lineEditRefQuantity->setText(QString("%1").arg(m_current->m_referenceQuantity));
	m_ui->lineEditRefUnit->setText(QString::fromStdString(m_current->m_referenceUnit.name()));

	m_ui->lineEditUUID->setText(m_current->m_uuid);
	m_ui->tableWidgetEpdData->setRowCount((int)m_current->m_epdCategoryDataset.size());

	for(unsigned int j=0; j<m_current->m_epdCategoryDataset.size(); ++j) {
		QString moduleString = "";
		for (unsigned int i=0; i<m_current->m_epdCategoryDataset[j].m_modules.size(); ++i) {
			const VICUS::EpdCategoryDataset::Module &module = m_current->m_epdCategoryDataset[j].m_modules[i];
			moduleString += QString(i > 0 ? ", " : "") + VICUS::KeywordList::Description("EpdCategoryDataset::Module", module); // add ", " in between beginning from the second module
		}
		m_ui->tableWidgetEpdData->setItem((int)j, 0, new QTableWidgetItem(moduleString));

		for(unsigned int i=0; i<VICUS::EpdCategoryDataset::NUM_P; ++i) {
			IBK::Parameter &para = m_current->m_epdCategoryDataset[j].m_para[i];

			int row = 1;
			if(para.name == "GWP")
				row = 1;
			else if(para.name == "ODP")
				row = 2;
			else if(para.name == "POCP")
				row = 3;
			else if(para.name == "AP")
				row = 4;
			else if(para.name == "EP")
				row = 5;
			else if(para.name == "PENRT")
				row = 6;
			else if(para.name == "PERT")
				row = 7;
			else
				continue;
			//            else if(para.name == "PENRT")
			//                row = 6;
			//            else if(para.name == "PERT")
			//                row = 2;

			QString val;
			if(para.empty())
				val = "-";
			else {
				val = QString::number(para.value);
			}


			m_ui->tableWidgetEpdData->setItem((int)j, row, new QTableWidgetItem(val))   ;
		}
	}

}

SVDBEpdTableModel * SVDBEPDEditWidget::dbModel() const {
	return m_dbModel;
}

