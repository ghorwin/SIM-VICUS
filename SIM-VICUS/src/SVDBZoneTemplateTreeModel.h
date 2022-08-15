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

#ifndef SVDBZoneTemplateTreeModelH
#define SVDBZoneTemplateTreeModelH

#include "SVAbstractDatabaseEditWidget.h"

#include "SVDatabase.h"

class QTreeView;

/*! Model for accessing zone templates.
	Child nodes correspond to sub-templates, role Role_SubTemplateType delivers the type of the sub-template.
*/
class SVDBZoneTemplateTreeModel : public QAbstractItemModel {
	Q_OBJECT
public:
	/*! Columns shown in the table view. */
	enum Columns {
		ColId,
		ColType,
		ColColor,
		ColCheck,
		ColName,
		NumColumns
	};

	/*! Constructor, requires a read/write pointer to the central database object.
		\note Pointer to database must be valid throughout the lifetime of the Model!
		*/
	SVDBZoneTemplateTreeModel(QObject * parent, SVDatabase & db);

	// ** QAbstractItemModel interface **

	virtual int columnCount ( const QModelIndex & ) const override { return NumColumns; }
	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;
	virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
	QModelIndex index(int row, int column, const QModelIndex & parent) const override;
	QModelIndex parent(const QModelIndex & child) const override;

	// ** SVAbstractDatabaseTableModel interface **

	virtual void resetModel();
	QModelIndex addNewItem();
	/*! This modifies the model correctly so that the already modified data structure is correctly reflected in tree model changes. */
	QModelIndex addChildItem(const QModelIndex & templateIndex, int subTemplateType, unsigned int subTemplateID);
	QModelIndex copyItem(const QModelIndex & index);
	void deleteItem(const QModelIndex & index);
	void deleteChildItem(const QModelIndex & templateIndex, int subTemplateType);

	// ** other members **

	/*! Tells the model that an item has been modified, triggers a dataChanged() signal. */
	void setItemModified(unsigned int id);

	/*! Sets property m_local of element, does nothing if index does not exist. */
	void setItemLocal(const QModelIndex &index, bool local);

	/*! Returns an index for a given Id. */
	QModelIndex indexById(unsigned int id) const;

private:

	/*! Pointer to the entire database (not owned). */
	SVDatabase	*m_db;
};


#endif // SVDBZoneTemplateTreeModelH
