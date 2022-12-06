#ifndef QTEXT_TOOLBOXH
#define QTEXT_TOOLBOXH

#include <QWidget>

#include "QtExt_ClickableLabel.h"


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

	struct Page: public QWidget {

		Page(QtExt::ClickableLabel *pageName, QtExt::ClickableLabel *arrowIcon, QtExt::ClickableLabel *icon, QWidget *widget, QFrame *frame, QWidget *parent):
			m_label(pageName),
			m_arrowIcon(arrowIcon),
			m_icon(icon),
			m_widget(widget),
			m_frame(frame),
			m_parent(parent)
		{}

		/*! Stores pointer to label, needed to toggle font weight */
		QtExt::ClickableLabel		*m_label = nullptr;
		/*! Stores pointer to arrow icon, neded to toggle icon. */
		QtExt::ClickableLabel		*m_arrowIcon = nullptr;
		/*! Stores pointer to icon. */
		QtExt::ClickableLabel		*m_icon = nullptr;
		/*! Stores pointer to widget, neded to toggle visibility */
		QWidget						*m_widget = nullptr;
		/*! Stores pointer to vertical frame */
		QFrame						*m_frame = nullptr;
		/*! Parent widget. */
		QWidget						*m_parent = nullptr;

	};


public:

	explicit ToolBox(QWidget *parent = nullptr);
	~ToolBox() override;

	/*!
	 * Adds a page with given header name, widget and custom icon
	 */
	void addPage(const QString & headerName, QWidget * widget, QIcon * icon = nullptr, int headerFontSize=10);

	/*! Returns widget according to given index */
	QWidget * widget(unsigned int index) const {
		return m_pages[index]->m_widget;
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
	/*! Stores pointer to all pages. */
	std::vector<Page*>						m_pages;
	/*! Stores pointer to layouts. */
	QVBoxLayout								*m_layout;
	/*! Collapsed arrow icon */
	QPixmap									m_arrowRight;
	/*! Expanded arrow icon */
	QPixmap									m_arrowDown;

};

} // namespace QtExt

#endif // QTEXT_TOOLBOXH
