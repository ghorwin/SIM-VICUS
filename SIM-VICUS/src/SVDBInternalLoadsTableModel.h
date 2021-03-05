#ifndef SVDBInternalLoadsTableModelH
#define SVDBInternalLoadsTableModelH


#include <QAbstractTableModel>

#include "SVDatabase.h"

/*! Model for accessing the internal loads in the internal loads database.

	The individual columns have different values for the DisplayRole (and some
	also for alignment). The column 'ColCheck' shows an indication if all data
	in internal loads is valid.
	All columns (i.e. all model indexes) return custom role data for global
	id and built-in roles (see SVConstants.h).
*/
class SVDBInternalLoadsTableModel : public QAbstractTableModel {
public:

	enum Type{
		T_Person,
		T_ElectricEquipment,
		T_Ligthing,
		T_Other,
		NUM_T
	};

	/*! Columns shown in the table view. */
	enum Columns {
		ColId,
		ColCheck,
		ColName,
		//ColCategory,
		ColSource,
		NumColumns
	};

	/*! Constructor, requires a read/write pointer to the central database object.
		\note Pointer to database must be valid throughout the lifetime of the Model!
	*/
	SVDBInternalLoadsTableModel(QObject * parent, SVDatabase & db, Type t);
	virtual ~SVDBInternalLoadsTableModel();


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
	QModelIndex addNewItem(VICUS::InternalLoad intLoad);

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

#endif // SVDBInternalLoadsTableModelH
