#ifndef SVDBComponentTableModelH
#define SVDBComponentTableModelH

#include <QAbstractTableModel>

#include "SVDatabase.h"


/*! Model for accessing the components in the component database. */
class SVDBComponentTableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	/*! Columns shown in the table view. */
	enum Columns {
		ColId,
		ColColor,
		ColCheck,
		ColName,
		ColType,
		NumColumns
	};

	/*! Constructor, requires a read/write pointer to the central database object.
		\note Pointer to database must be valid throughout the lifetime of the Model!
		*/
	SVDBComponentTableModel(QObject * parent, SVDatabase & db);
	virtual ~SVDBComponentTableModel();

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
	QModelIndex addNewItem(VICUS::Component c);

	/*! Locates the item with the requested ID and returns the matching model index (to first column), or
		an invalid model index, if item cannot be found.
	*/
	QModelIndex findItem(unsigned int componentId) const;

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

#endif // SVDBComponentTableModelH
