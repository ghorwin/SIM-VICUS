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

#ifndef SVDBEpdTableModelH
#define SVDBEpdTableModelH

#include "SVAbstractDatabaseEditWidget.h"

#include "SVDatabase.h"

#include <VICUS_EpdDataset.h>

#include <QSortFilterProxyModel>

/*! Model for accessing the constructions in the construction database. */
class SVDBEpdTableModel : public SVAbstractDatabaseTableModel {
	Q_OBJECT
public:
	/*! Columns shown in the table view. */
	enum Columns {
		ColId,
		ColCheck,
		ColName,
        ColManufacturer,
        ColModules,
		ColType,
		NumColumns
	};

	/*! Constructor, requires a read/write pointer to the central database object.
		\note Pointer to database must be valid throughout the lifetime of the Model!
		*/
	SVDBEpdTableModel(QObject * parent, SVDatabase & db);

	// ** QAbstractItemModel interface **

	virtual int columnCount ( const QModelIndex & ) const override { return NumColumns; }
	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;
	virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

	// ** SVAbstractDatabaseTableModel interface **

	int columnIndexId() const override { return ColId; }
	SVDatabase::DatabaseTypes databaseType() const override { return SVDatabase::DT_EPDDatasets; }
	virtual void resetModel() override;
	QModelIndex addNewItem() override;
	QModelIndex copyItem(const QModelIndex & index) override;
	void deleteItem(const QModelIndex & index) override;
	void setColumnResizeModes(QTableView * tableView) override;
	void setItemLocal(const QModelIndex &index, bool local) override;

	// ** other members **
	/*! Add EPD to Database. */
	void importDatasets(const std::map<QString, VICUS::EpdDataset> & epds);

	/*! Tells the model that an item has been modified, triggers a dataChanged() signal. */
	void setItemModified(unsigned int id);

private:
	/*! Returns an index for a given Id. */
	QModelIndex indexById(unsigned int id) const;

	/*! The sort filter model (owned). */
	QSortFilterProxyModel			*m_proxyModel	= nullptr;

	/*! Pointer to the entire database (not owned). */
	SVDatabase						*m_db;
};

#endif // SVDBEpdTableModelH
