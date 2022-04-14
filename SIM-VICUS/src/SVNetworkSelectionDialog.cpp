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

#include "SVNetworkSelectionDialog.h"
#include "SVPropBuildingSurfaceHeatingWidget.h"
#include "ui_SVNetworkSelectionDialog.h"

#include <QItemSelectionModel>
#include <QListWidgetItem>
#include <QPushButton>

#include "SVProjectHandler.h"
#include "SVStyle.h"

#include <VICUS_Project.h>


SVNetworkSelectionDialog::SVNetworkSelectionDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkSelectionDialog)
{
	m_ui->setupUi(this);

	SVStyle::formatListView(m_ui->listWidget);
	// update dialog
	updateUi();
}


SVNetworkSelectionDialog::~SVNetworkSelectionDialog() {
	delete m_ui;
}

void SVNetworkSelectionDialog::updateUi()
{
	// block all signals
	m_ui->buttonBox->button(QDialogButtonBox::Ok)->blockSignals(true);
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->blockSignals(true);
	m_ui->listWidget->blockSignals(true);
	m_ui->listWidget->selectionModel()->blockSignals(false);
	m_ui->listWidget->setSortingEnabled(false);
	m_ui->listWidget->clear();

	// select network
	std::map<unsigned int, const VICUS::GenericNetwork*> networks = dynamic_cast<SVPropBuildingSurfaceHeatingWidget*>
			(parent()) ->genericNetworks();

	// add all networks to dialog
	for (std::map<unsigned int, const VICUS::GenericNetwork*>::const_iterator
		 networkIt = networks.begin();
		 networkIt != networks.end(); ++networkIt) {
		// add new network
		QListWidgetItem * item = new QListWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setText(networkIt->second->m_displayName);
		item->setData(Qt::UserRole,networkIt->first);
		m_ui->listWidget->addItem(item);
	}

	m_ui->listWidget->setSortingEnabled(true);
	m_ui->listWidget->blockSignals(false);
	m_ui->listWidget->selectionModel()->blockSignals(false);
	m_ui->buttonBox->button(QDialogButtonBox::Ok)->blockSignals(false);
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->blockSignals(false);
}


void SVNetworkSelectionDialog::on_listWidget_itemSelectionChanged()
{
	// disable ok button if selection is empty
	const QItemSelection &selection = m_ui->listWidget->selectionModel()->selection();

	if (selection.isEmpty()) {
		m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
	else {
		m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
		// store ID of selected zone
		const QModelIndex &index = selection.indexes().front();

		// select network
		m_idNetwork = (unsigned int) m_ui->listWidget->item(index.row())->data(Qt::UserRole).toInt();
	}
}


void SVNetworkSelectionDialog::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
	// store ID of selected network
	m_idNetwork = (unsigned int) item->data(Qt::UserRole).toInt();
	accept();
}


