#ifndef QtExt_MaterialDBItemDelegateH
#define QtExt_MaterialDBItemDelegateH

#include <QItemDelegate>

namespace QtExt {


/*! This class implementation is only used to draw the background colors for the material list.
*/
class MaterialDBItemDelegate : public QItemDelegate {
	Q_OBJECT
public:
	explicit MaterialDBItemDelegate(QObject *parent = 0);

	/*! Set view to use default color setting (\sa QtExt::Style).*/
	void setDefaultColors();

	/*! Set colors for user defined color setting for built-in materials.*/
	void setUserDefinedColors(const QColor& bright, const QColor& dark);

	/*! Set color for not built-in materials.*/
	void setUserColor(const QColor& col);

	/*! Set use of alternating colors for rows in view.
		Default setting is on.
		Colors can be set by using setUserColors or setDefaultColors.
		\param alternate If true alternating colors are used.
	*/
	void setAlternatingColors(bool alternate);

protected:
	virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

private:
	bool									m_alternatingColors;
	bool									m_defaultColors;
	QColor									m_colorUserMaterials;
	QColor									m_colorAlternativeBackgroundBright;
	QColor									m_colorAlternativeBackgroundDark;
};

} // end namespace

#endif // QtExt_MaterialDBItemDelegateH
