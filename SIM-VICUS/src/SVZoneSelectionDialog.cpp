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

#include "SVZoneSelectionDialog.h"
#include "ui_SVZoneSelectionDialog.h"

#include <QSortFilterProxyModel>
#include <QItemSelectionModel>
#include <QPushButton>

#include "SVProjectHandler.h"
#include "SVStyle.h"

#include <VICUS_Project.h>

// A model that provides a list of zones and their IDs via UserRole.
class SVZoneListModel : public QAbstractListModel {
	Q_OBJECT
public:

	SVZoneListModel(QObject * parent, const VICUS::Project &project);

	int rowCount(const QModelIndex & parent) const override;
	QVariant data(const QModelIndex & index, int role) const override;

private:
	QStringList					m_roomNames;
	std::vector<unsigned int>	m_roomIds;
};



SVZoneSelectionDialog::SVZoneSelectionDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVZoneSelectionDialog)
{
	m_ui->setupUi(this);

	// constructor create an up-to-date list
	m_listModel = new SVZoneListModel(this, project());

	SVStyle::formatListView(m_ui->listView);

	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setSourceModel(m_listModel);
	m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	m_ui->listView->setModel(m_proxyModel);

	// connect list view selection change signal
	connect(m_ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &SVZoneSelectionDialog::onZoneListSelectionChanged);
}


SVZoneSelectionDialog::~SVZoneSelectionDialog() {
	delete m_ui;
}


void SVZoneSelectionDialog::on_lineEditFilter_textChanged(const QString &pattern) {
	m_proxyModel->setFilterWildcard(pattern);

	QItemSelectionModel *selectionModel = m_ui->listView->selectionModel();
	// if no selection is available set curser to first position
	if (selectionModel->selectedRows().empty() && m_proxyModel->rowCount() > 0) {
		m_ui->listView->setCurrentIndex(m_proxyModel->index(0,0));
	}
}


void SVZoneSelectionDialog::on_listView_doubleClicked(const QModelIndex &index) {
	m_idZone = index.data(Qt::UserRole).toUInt();
	accept();
}


void SVZoneSelectionDialog::onZoneListSelectionChanged(const QItemSelection & selected, const QItemSelection & /*deselected*/) {
	// disable ok button if selection is empty
	if (selected.isEmpty())
		m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	else {
		m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
		// store ID of selected zone
		QModelIndex index = selected.indexes().front();
		m_idZone = index.data(Qt::UserRole).toUInt();
	}
}


// *** SVZoneListModel ***

SVZoneListModel::SVZoneListModel(QObject * parent, const VICUS::Project &project)
	: QAbstractListModel(parent)
{
	// suggest all rooms (of all buildings and building levels)
	// at the first litter
	const std::vector<VICUS::Building> &buildings = project.m_buildings;

	// loop over all buildings
	for(const VICUS::Building &building : buildings) {
		const std::vector<VICUS::BuildingLevel> &blevels = building.m_buildingLevels;
		// loop over all building levels
		for(const VICUS::BuildingLevel &blevel : blevels) {
			const std::vector<VICUS::Room> rooms = blevel.m_rooms;
			// loop over all rooms
			for(const VICUS::Room &room : rooms) {
				QString roomName = tr("%1.%2.%3").arg(building.m_displayName).arg(blevel.m_displayName).arg(room.m_displayName);
				m_roomNames.push_back(roomName);
				m_roomIds.push_back(room.m_id);
			}
		}
	}
}


int SVZoneListModel::rowCount(const QModelIndex & /*parent*/) const {
	return m_roomNames.size();
}


QVariant SVZoneListModel::data(const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	int row = index.row();
	switch (role) {
		case Qt::DisplayRole :{
			Q_ASSERT(row < m_roomNames.size());
			return m_roomNames[row];
		}
		// UserRole returns value reference
		case Qt::UserRole : {
			Q_ASSERT(row < (int) m_roomIds.size());
			return m_roomIds[(unsigned int) row];
		}
	}
	return QVariant();
}
