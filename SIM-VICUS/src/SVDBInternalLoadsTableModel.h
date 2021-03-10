#ifndef SVDBInternalLoadsTableModelH
#define SVDBInternalLoadsTableModelH

#include "SVAbstractDatabaseEditWidget.h"

#include "SVDatabase.h"

#include <VICUS_InternalLoad.h>

/*! Model for accessing the internal loads in the internal loads database. */
class SVDBInternalLoadsTableModel : public SVAbstractDatabaseTableModel {
public:

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
	SVDBInternalLoadsTableModel(QObject * parent, SVDatabase & db, VICUS::InternalLoad::Category t);

	// ** QAbstractItemModel interface **

	virtual int columnCount ( const QModelIndex & ) const override { return NumColumns; }
	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;
	virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

	// ** SVAbstractDatabaseTableModel interface **

	int columnIndexId() const override { return ColId; }
	SVDatabase::DatabaseTypes databaseType() const override { return SVDatabase::DT_InternalLoads; }
	virtual void resetModel() override;
	QModelIndex addNewItem() override;
	QModelIndex copyItem(const QModelIndex & index) override;
	void deleteItem(const QModelIndex & index) override;
	void setColumnResizeModes(QTableView * tableView) override;

	// ** other members **

	/*! Tells the model that an item has been modified, triggers a dataChanged() signal. */
	void setItemModified(unsigned int id);

private:
	/*! Returns an index for a given Id. */
	QModelIndex indexById(unsigned int id) const;

	/*! Pointer to the entire database (not owned). */
	SVDatabase		* m_db;

	/*! Type of table model. */
	VICUS::InternalLoad::Category			m_category;
};

#endif // SVDBInternalLoadsTableModelH
