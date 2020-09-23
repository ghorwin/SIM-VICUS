#include "treeview.h"
#include <qfilesystemmodel.h>

class FileSystemModel: public QFileSystemModel
{
public:
    FileSystemModel()
    {
        setNameFilters( QStringList() << "*.mml" );
        setNameFilterDisables( false );
    }

    virtual int columnCount( const QModelIndex &parent ) const
    {
        return ( parent.column() > 0 ) ? 0 : 1;
    }
};

TreeView::TreeView( QWidget *parent ):
    QTreeView( parent )
{
    setModel( new FileSystemModel() );
}

void TreeView::setRootPath( const QString& rootPath )
{
    QFileSystemModel* fsModel = qobject_cast< QFileSystemModel* >( model() );

    const QModelIndex index = fsModel->index( rootPath );
    fsModel->setRootPath( rootPath );

    setRootIndex( index );
    setExpanded( index, true );
}

void TreeView::currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
    QTreeView::currentChanged( current, previous );

    QFileSystemModel* fsModel = qobject_cast< QFileSystemModel* >( model() );
    Q_EMIT selected( fsModel->filePath( current ) );
}
