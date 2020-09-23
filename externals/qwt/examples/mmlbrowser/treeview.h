#ifndef TREE_VIEW_H
#define TREE_VIEW_H

#include <qtreeview.h>

class TreeView: public QTreeView
{
    Q_OBJECT

public:
    TreeView( QWidget *parent = NULL );

Q_SIGNALS:
    void selected( const QString& );

public Q_SLOTS:
    void setRootPath( const QString& );

protected:
    virtual void currentChanged( const QModelIndex &, const QModelIndex & );
};

#endif
