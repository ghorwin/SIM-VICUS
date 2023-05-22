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

#ifndef SVDatabaseSortFilterProxyModel_H
#define SVDatabaseSortFilterProxyModel_H

#include <QSortFilterProxyModel>

#include "SVDatabaseEditDialog.h"

/*! Sorting/filtering of database entries by name. */
class SVDatabaseSortFilterProxyModel : public QSortFilterProxyModel  {
public:
	SVDatabaseSortFilterProxyModel(QObject *parent = nullptr);
	virtual ~SVDatabaseSortFilterProxyModel();

	void setFilterText(const QString & filterText);

	// QSortFilterProxyModel interface
protected:
	bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;
	bool lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const;

private:
	QString m_filterText;
};

#endif // SVDATABASESORTFILTERPROXYMODEL_H
