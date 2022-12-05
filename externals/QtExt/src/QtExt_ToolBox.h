#ifndef QTEXT_TOOLBOXH
#define QTEXT_TOOLBOXH

#include <QWidget>

namespace QtExt {
	class ClickableLabel;
}

class QVBoxLayout;
class QFrame;

namespace QtExt {

/*!
 * \brief The ToolBox class implements a widget similar to QToolBox with some more flexibility and custom style.
 * Basically, this class holds multiple pages (given as QWidgets) that can be expanded/collapsed. The idea of this class
 * is that only ONE page is expanded at a time. Each page contains a header with collapsed/expanded arrow icon. An additional
 * icon can be added to the header as well. Each page has a unique id that needs to be provided when adding the widget.
 * Header label and icons are all QtExt::ClickableLabels, so they have no click animation but still emit a clicked(id) signal
 * that also tells their given id.
 */

class ToolBox : public QWidget
{
	Q_OBJECT

public:
	explicit ToolBox(QWidget *parent = nullptr);

	/*!
	 * Adds a page with given header name, widget and custom icon
	 */
	void addPage(const QString & headerName, QWidget * widget, QIcon * icon = nullptr,
				   QFont::Weight headerFontWeight = QFont::Bold, int headerFontSize=10);

	/*! Returns widget according to given index */
	QWidget * widget(unsigned int index) const {
		return m_widgets[index];
	}

	/*! Set index from outside, triggers slot onLabelClicked() and thereby emits indexChanged() signal  */
	void setCurrentIndex(unsigned int index);

	/*! Returns index of currently expanded page */
	unsigned int currentIndex();

public slots:
	/*! Changes arrow icons and visibility of given page, connected to ClickableLabels */
	void onLabelClicked(unsigned int id);

signals:
	void indexChanged(unsigned int currentIndex);

private:
	/*! Stores pointer to all widgets, neded to toggle visibility */
	std::vector<QWidget*>					m_widgets;
	/*! Stores pointer to all arrow icons, neded to toggle icon. Always has same length like m_widgets. */
	std::vector<QtExt::ClickableLabel*>		m_arrowIcons;
	std::vector<QtExt::ClickableLabel*>		m_labels;

	std::vector<QFrame*>					m_frames;

	/*! Holds the layout. */
	QVBoxLayout								*m_layout;
	/*! Stores collapsed arrow icon */
	QPixmap									m_arrowRight;
	/*! Stores expanded arrow icon */
	QPixmap									m_arrowDown;

};

} // namespace QtExt

#endif // QTEXT_TOOLBOXH
