#ifndef SVAbstractDatabaseEditWidgetH
#define SVAbstractDatabaseEditWidgetH

#include <QWidget>
#include <QAbstractTableModel>
#include "SVDatabase.h"

class SVDatabase;
class SVDBComponentTableModel;
class QTableView;

/*! Abstract interface for tables used in database edit dialogs.
	Needs to provide Role_BuiltIn and Role_Id.
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
	/*! Sets the column-specific resize modes in the table view. */
	virtual void setColumnResizeModes(QTableView * tableView) = 0;
};


/*! This class declares common interface functions needed for communication between the generic
	database dialog and the edit widget.
*/
class SVAbstractDatabaseEditWidget : public QWidget {
	Q_OBJECT
public:
	virtual ~SVAbstractDatabaseEditWidget();
	virtual void updateInput(int id) = 0;
	virtual void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) = 0;
};



#endif // SVAbstractDatabaseEditWidgetH
