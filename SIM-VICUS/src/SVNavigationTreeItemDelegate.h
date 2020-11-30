#ifndef SVNavigationTreeItemDelegateH
#define SVNavigationTreeItemDelegateH

#include <QItemDelegate>

/*! The item delegate that displays the object state in the navigation tree widget. */
class SVNavigationTreeItemDelegate : public QItemDelegate {
	Q_OBJECT
public:
	SVNavigationTreeItemDelegate(QWidget * parent = nullptr);

	// QAbstractItemDelegate interface

	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
	bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index) override;

private:
	QImage m_lightBulbOn;
	QImage m_lightBulbOff;
};

#endif // SVNavigationTreeItemDelegateH
