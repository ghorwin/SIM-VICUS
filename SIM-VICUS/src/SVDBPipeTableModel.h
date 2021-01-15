#ifndef SVDBPIPETABLEMODEL_H
#define SVDBPIPETABLEMODEL_H

#include <QAbstractTableModel>

#include "SVDatabase.h"

class SVDBPipeTableModel: public QAbstractTableModel {
	Q_OBJECT
public:
	/*! Columns shown in the table view. */
	enum Columns {
		ColId,
		ColCheck,
		ColName,
		NumColumns
	};

	/*! Constructor, requires a read/write pointer to the central database object.
		\note Pointer to database must be valid throughout the lifetime of the Model!
	*/
	SVDBPipeTableModel(QObject * parent, SVDatabase & db);
	virtual ~SVDBPipeTableModel();

	// ** QAbstractItemModel interface **

	virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

	// ** other members **

	/*! Tells the model that an item has been modified, triggers a dataChanged() signal. */
	void setItemModified(unsigned int id);

	/*! Inserts a new item and returns the model index of the new item. */
	QModelIndex addNewItem();

	/*! Inserts a new item and returns the model index of the new item.
		\note Pass-by-value is intended.
	*/
	QModelIndex addNewItem(VICUS::NetworkPipe bc);

	/*! Removes a selected item.
		\return Returns true on success, false if the item wasn't deleted (invalid index etc.)
	*/
	bool deleteItem(QModelIndex index);

	void resetModel();

private:
	/*! Returns an index for a given Id. */
	QModelIndex indexById(unsigned int id) const;

	/*! Pointer to the entire database (not owned). */
	SVDatabase	* m_db;
};

#endif // SVDBPIPETABLEMODEL_H
