#ifndef SVDBModelDelegateH
#define SVDBModelDelegateH

#include <QItemDelegate>

/*!	Colors background and text of built-in database items in different colors.
	Expects built-in
*/
class SVDBModelDelegate : public QItemDelegate {
	Q_OBJECT
public:
	/*! Default constructor. */
	SVDBModelDelegate(QObject * parent, int builtInRole);
	/*! Default destructor. */
	~SVDBModelDelegate();

	void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

	/*! Data role to check for boolean (true) if item is a "built-in". */
	int m_builtInRole;
};

#endif // SVDBModelDelegateH
