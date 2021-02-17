#ifndef SVPropFloorManagerItemDelegateH
#define SVPropFloorManagerItemDelegateH

#include <QItemDelegate>

/*! The item delegate that displays the object state in the navigation tree widget. */
class SVPropFloorManagerItemDelegate : public QItemDelegate {
	Q_OBJECT
public:
	SVPropFloorManagerItemDelegate(QWidget * parent = nullptr);

	// QAbstractItemDelegate interface

	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // SVPropFloorManagerItemDelegateH
