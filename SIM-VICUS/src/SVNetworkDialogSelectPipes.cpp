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

#include "SVNetworkDialogSelectPipes.h"
#include "ui_SVNetworkDialogSelectPipes.h"

#include "SVSettings.h"
#include "SVStyle.h"

#include <VICUS_Network.h>

#include <QTableWidgetItem>

#include <SVConversions.h>

SVNetworkDialogSelectPipes::SVNetworkDialogSelectPipes(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkDialogSelectPipes)
{
	m_ui->setupUi(this);

	// setup table widgets
	m_ui->tableWidgetDatabase->setColumnCount(2);
	m_ui->tableWidgetDatabase->setHorizontalHeaderLabels(QStringList() << QString() << tr("Database"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetDatabase);
	m_ui->tableWidgetDatabase->setSortingEnabled(false);
	m_ui->tableWidgetDatabase->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetDatabase->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetDatabase->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetDatabase->setSelectionMode(QTableWidget::ExtendedSelection);

	m_ui->tableWidgetNetwork->setColumnCount(2);
	m_ui->tableWidgetNetwork->setHorizontalHeaderLabels(QStringList() << QString() << tr("Current Network"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetNetwork);
	m_ui->tableWidgetNetwork->setSortingEnabled(false);
	m_ui->tableWidgetNetwork->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetNetwork->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetNetwork->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetNetwork->setSelectionMode(QTableWidget::ExtendedSelection);
}

SVNetworkDialogSelectPipes::~SVNetworkDialogSelectPipes()
{
	delete m_ui;
}

void SVNetworkDialogSelectPipes::edit(VICUS::Network &network)
{
	// title of right table
	m_ui->tableWidgetNetwork->setHorizontalHeaderLabels(QStringList() << QString() <<
														QString("Network: %1").arg(network.m_displayName));

	const SVDatabase & db = SVSettings::instance().m_db;

	//  *** Update database table widget ***
	m_ui->tableWidgetDatabase->clearContents();
	m_ui->tableWidgetDatabase->setRowCount(db.m_pipes.size());
	int row = 0;
	for (auto it = db.m_pipes.begin(); it != db.m_pipes.end(); ++it, ++row){

		// color
		QTableWidgetItem * item = new QTableWidgetItem();
		item->setBackground(it->second.m_color);
		item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		m_ui->tableWidgetDatabase->setItem(row, 0, item);

		// pipe name
		item = new QTableWidgetItem();
		auto t = it->second.m_displayName;
		item->setText(QtExt::MultiLangString2QString(it->second.m_displayName));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setData(Qt::UserRole, it->second.m_id);
		m_ui->tableWidgetDatabase->setItem(row, 1, item);
	}

	// we copy the currently available pipes list
	m_selectedPipeIds = network.m_availablePipes;

	//  Update network table widget
	updateNetworkTableWidget();

	// if accepted, copy the selected pipes back
	if (exec() == QDialog::Accepted){
		network.m_availablePipes = m_selectedPipeIds;
	}
}


void SVNetworkDialogSelectPipes::on_pushButtonAdd_clicked()
{
	QList<QTableWidgetItem*> itemList = m_ui->tableWidgetDatabase->selectedItems();

	for (QTableWidgetItem *itemName: itemList){
		// get pipe id
		unsigned int id = itemName->data(Qt::UserRole).toUInt();
		// check if it is already in availablePipes, if not add it
		if (std::find(m_selectedPipeIds.begin(), m_selectedPipeIds.end(), id) == m_selectedPipeIds.end())
			m_selectedPipeIds.push_back(id);
	}

	updateNetworkTableWidget();
}


void SVNetworkDialogSelectPipes::on_pushButtonRemove_clicked()
{
	QList<QTableWidgetItem*> itemList = m_ui->tableWidgetNetwork->selectedItems();

	// get all ids that shall be removed
	std::vector<unsigned int> removedIds;
	for (QTableWidgetItem *itemName: itemList){
		unsigned int id = itemName->data(Qt::UserRole).toUInt();
		removedIds.push_back(id);
	}

	// create new vector and add only ids which shall not be removed
	std::vector<unsigned int> tmp;
	for (unsigned int id: m_selectedPipeIds){
		if (std::find(removedIds.begin(), removedIds.end(), id) == removedIds.end())
			tmp.push_back(id);
	}

	m_selectedPipeIds = tmp;

	updateNetworkTableWidget();
}


void SVNetworkDialogSelectPipes::updateNetworkTableWidget()
{
	const SVDatabase & db = SVSettings::instance().m_db;

	m_ui->tableWidgetNetwork->clearContents();
	m_ui->tableWidgetNetwork->setRowCount(m_selectedPipeIds.size());
	int row = 0;
	for (unsigned int id: m_selectedPipeIds){

		// color
		const VICUS::NetworkPipe * pipe = db.m_pipes[id];
		QTableWidgetItem * item = new QTableWidgetItem();
		if (pipe == nullptr){
			item->setBackground(QColor(64,64,64));
		}
		else{
			item->setBackground(pipe->m_color);
		}
		item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		m_ui->tableWidgetNetwork->setItem(row, 0, item);

		// pipe name
		item = new QTableWidgetItem();
		if (pipe == nullptr)
			item->setText(tr("<invalid pipe id>"));
		else
			item->setText(QtExt::MultiLangString2QString(pipe->m_displayName));
		item->setData(Qt::UserRole, id);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetNetwork->setItem(row, 1, item);

		++row;
	}

	// deselect previously selected pipes (more user-friendly for multi-selection mode)
	m_ui->tableWidgetDatabase->clearSelection();
}

