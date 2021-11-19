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

#ifndef SVAbstractDatabaseEditWidgetH
#define SVAbstractDatabaseEditWidgetH

#include <QWidget>
#include <QAbstractTableModel>
#include "SVDatabase.h"

class QTableView;

/*! Abstract interface for tables used in database edit dialogs.
	Derived models need to provide at least Role_BuiltIn and Role_Id.
*/
class SVAbstractDatabaseTableModel : public QAbstractTableModel {
public:
	SVAbstractDatabaseTableModel(QObject * parent) : QAbstractTableModel(parent) {}
	virtual ~SVAbstractDatabaseTableModel();
	/*! Returns the index of the ID column. */
	virtual int columnIndexId() const = 0;
	/*! Returns the database type. */
	virtual SVDatabase::DatabaseTypes databaseType() const = 0;
	/*! Tells model to reset and thus update all views completely. */
	virtual void resetModel() = 0;
	/*! Inserts a new item and returns the model index of the new item. */
	virtual QModelIndex addNewItem() = 0;
	/*! Copies item for given item index and returns the model index of the new item. */
	virtual QModelIndex copyItem(const QModelIndex & index) = 0;
	/*! Removes an item, does nothing if index doesn exist. */
	virtual void deleteItem(const QModelIndex & index) = 0;
	/*! Sets property m_local of element, does nothing if index doesn exist. */
	virtual void setItemLocal(const QModelIndex & index, bool local) = 0;
	/*! Sets the column-specific resize modes in the table view. */
	virtual void setColumnResizeModes(QTableView * tableView) = 0;
};


/*! This class declares common interface functions needed for communication between the generic
	database dialog and the edit widget.
*/
class SVAbstractDatabaseEditWidget : public QWidget {
	Q_OBJECT
public:
	explicit SVAbstractDatabaseEditWidget(QWidget *parent = nullptr) : QWidget(parent) {}
	virtual ~SVAbstractDatabaseEditWidget();
	virtual void updateInput(int id) = 0;
	virtual void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) = 0;
};



#endif // SVAbstractDatabaseEditWidgetH
