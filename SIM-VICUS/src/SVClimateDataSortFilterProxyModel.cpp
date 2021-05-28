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

#include "SVClimateDataSortFilterProxyModel.h"

#include "SVConstants.h"

SVClimateDataSortFilterProxyModel::SVClimateDataSortFilterProxyModel(QObject * parent) :
	QSortFilterProxyModel(parent)
{

}


SVClimateDataSortFilterProxyModel::~SVClimateDataSortFilterProxyModel() {
	}


void SVClimateDataSortFilterProxyModel::setFilterText(const QString & filterText) {
	beginResetModel();
	m_filterText = filterText;
	endResetModel();
}


bool SVClimateDataSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex & /*source_parent*/) const {
	if (filterRegExp().isEmpty())
		return true; // no filter, always accepted

	// search through all text columns, if any of the texts in the first columns contains the filter text,
	// accept the row
	for (unsigned int i=0; i<=SVClimateDataTableModel::C_City; ++i) {
		QString text = sourceModel()->data(sourceModel()->index(source_row, i)).toString();
		if (text.indexOf(filterRegExp().pattern(), 0, Qt::CaseInsensitive) != -1)
			return true;
	}
	return false;
}


bool SVClimateDataSortFilterProxyModel::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const {
	// different comparison operations
	switch ((SVClimateDataTableModel::Columns)source_left.column()) {
		case SVClimateDataTableModel::C_Region:
		case SVClimateDataTableModel::C_Country:
		case SVClimateDataTableModel::C_Sub:
		case SVClimateDataTableModel::C_City:
		case SVClimateDataTableModel::NUM_C:
			break;

		case SVClimateDataTableModel::C_Longitude:
		case SVClimateDataTableModel::C_Latitude:
		case SVClimateDataTableModel::C_TimeZone:
		case SVClimateDataTableModel::C_Elevation:
			return source_left.data(Role_Value).toDouble() < source_right.data(Role_Value).toDouble();
	}
	return source_left.data().toString() < source_right.data().toString();
}

