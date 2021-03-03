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
	virtual ~SVAbstractDatabaseTableModel();
	virtual void setColumnResizeModes(QTableView * tableView) = 0;
	virtual void resetModel() = 0;
	virtual QModelIndex addNewItem() = 0;
	virtual QModelIndex copyItem(const QModelIndex & existingItemIndex) = 0;
	virtual void deleteItem(const QModelIndex & sourceIndex) = 0;
	virtual int columnIndexId() const = 0;
	virtual SVDatabase::DatabaseTypes databaseType() const = 0;
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
