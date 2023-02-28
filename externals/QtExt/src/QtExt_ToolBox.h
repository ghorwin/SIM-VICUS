#ifndef QtExt_ToolBoxH
#define QtExt_ToolBoxH

#include <QWidget>

#include "QtExt_ClickableLabel.h"


class QVBoxLayout;
class QFrame;

namespace QtExt {

/*! The ToolBox class implements a widget similar to QToolBox with some more flexibility and custom style.
	Basically, this class holds multiple pages that can be expanded/collapsed. The idea of this class is that only ONE page is expanded at a time.
	Each page contains a header with collapsed/expanded arrow icon. An additional icon can be added to the header as well.
	Header label and icons are all QtExt::ClickableLabels, so they have no click animation but still emit a clicked(id) signal
	that also tells their given id.
*/
class ToolBox : public QWidget {
	Q_OBJECT

public:

	explicit ToolBox(QWidget *parent = nullptr);
	~ToolBox() override;

	/*! Adds a page with given header name, widget and custom icon. */
	void addPage(const QString & headerName, QWidget * widget, QIcon * icon = nullptr, int headerFontSize=10);

	/*! Returns widget according to given index. */
	QWidget * widget(unsigned int index) const;

	/*! Set index from outside */
	void setCurrentIndex(unsigned int index);

	/*! Returns index of currently expanded page */
	unsigned int currentIndex();

	void updatePageBackgroundColorFromStyle();

private slots:
	/*! Changes arrow icons and visibility of given page, connected to ClickableLabels */
	void onLabelClicked();

signals:
	void indexChanged(unsigned int currentIndex);

private:
	class Page;

	/*! Stores pointer to all pages. */
	std::vector<Page*>						m_pages;
	/*! Stores pointer to layouts. */
	QVBoxLayout								*m_layout = nullptr;
	/*! Collapsed arrow icon */
	QPixmap									m_arrowRight;
	/*! Expanded arrow icon */
	QPixmap									m_arrowDown;

};

} // namespace QtExt

#endif // QtExt_ToolBoxH
