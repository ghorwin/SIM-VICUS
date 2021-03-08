#ifndef SVDBZoneTemplateTreeModelH
#define SVDBZoneTemplateTreeModelH

#include "SVAbstractDatabaseEditWidget.h"

#include "SVDatabase.h"


/*! Model for accessing zone templates.
	Child nodes correspond to sub-templates, role Qt::UserRole + 20 delivers the type of the sub-template.
*/
class SVDBZoneTemplateTreeModel : public QAbstractItemModel {
	Q_OBJECT
public:
	/*! Columns shown in the table view. */
	enum Columns {
		ColId,
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

	/*! Returns an index for a given Id. */
	QModelIndex indexById(unsigned int id) const;

private:

	/*! Pointer to the entire database (not owned). */
	SVDatabase	* m_db;

};


#endif // SVDBZoneTemplateTreeModelH
