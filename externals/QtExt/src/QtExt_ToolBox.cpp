#include "QtExt_ToolBox.h"

#include "QtExt_Style.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QtDebug>

#include <IBK_Exception.h>


namespace QtExt {

/*! Private container class for a widget with meta data. */
class ToolBox::Page: public QWidget {
public:

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


ToolBox::ToolBox(QWidget *parent):
	QWidget(parent), m_layout(new QVBoxLayout(this))
{
	setLayout(m_layout);
	m_layout->setContentsMargins(0,0,0,0);
	m_layout->setSpacing(3);
	m_arrowDown = QPixmap(":/gfx/master/arrow_down.png");
	m_arrowRight = QPixmap(":/gfx/master/arrow_right.png");
}


ToolBox::~ToolBox() {
	for (Page *page: m_pages)
		delete page;
}


void ToolBox::addPage(const QString & headerName, QWidget * widget, QIcon * icon, int headerFontSize) {

	// create header layout and add arrow, icon (if existing) and label to it
	QHBoxLayout *hLay = new QHBoxLayout(); // no parent pointer here!

	ClickableLabel *labelArrow = new ClickableLabel(m_pages.size(), "", this);
	labelArrow->setPixmap(m_arrowRight);
	QSizePolicy pol;
	pol.setHorizontalPolicy(QSizePolicy::Fixed);
	labelArrow->setSizePolicy(pol);
	hLay->addWidget(labelArrow);

	ClickableLabel *labelIcon = nullptr;
	if (icon != nullptr) {
		labelIcon = new ClickableLabel(m_pages.size(), "", this);
		labelIcon->setPixmap(icon->pixmap(10));
		labelIcon->setSizePolicy(pol);
		hLay->addWidget(labelIcon);
	}

	ClickableLabel *labelPageName = new ClickableLabel(m_pages.size(), headerName, this);
	QFont fnt;
	fnt.setPointSize(headerFontSize);
	labelPageName->setFont(fnt);
	hLay->addWidget(labelPageName);

	// we put the header layout into a frame for consistent background colors
	hLay->setContentsMargins(2,2,2,2);
	QFrame *hFrame = new QFrame(this);
	hFrame->setLayout(hLay);

	// create vertical layout that contains the header frame and widget
	QVBoxLayout *vLay = new QVBoxLayout();
	vLay->addWidget(hFrame);
	widget->layout()->setContentsMargins(20, 0, 2, 2);
	vLay->addWidget(widget);
	vLay->setContentsMargins(2,2,2,2);

	// we add the vertical layout to a frame again
	QFrame *vFrame = new QFrame(this);
	vFrame->setObjectName(QString("vFrame%1").arg(m_pages.size()));
	vFrame->setFrameShape(QFrame::Box);
	vFrame->setStyleSheet(QString("QFrame#%1 {border-radius: 5px; background-color: %2; border: 2px %3}")
						  .arg(vFrame->objectName()).arg(Style::ToolBoxPageBackground).arg(Style::ToolBoxPageEdge));

	vFrame->setLayout(vLay);
	m_layout->addWidget(vFrame);

	Page *page = new Page(labelPageName, labelArrow, labelIcon, widget, vFrame, this);
	m_pages.push_back(page);

	// add connections
	connect(labelArrow, &ClickableLabel::clicked, this, &ToolBox::onLabelClicked);
	connect(labelPageName, &ClickableLabel::clicked, this, &ToolBox::onLabelClicked);
	if (labelIcon != nullptr)
		connect(labelIcon, &ClickableLabel::clicked, this, &ToolBox::onLabelClicked);

	// update
	setCurrentIndex(0);
}


QWidget * ToolBox::widget(unsigned int index) const {
	Q_ASSERT(index < m_pages.size());
	return m_pages[index]->m_widget;
}


void ToolBox::setCurrentIndex(unsigned int index) {
	Q_ASSERT(index < m_pages.size());
	// Note: We first collapse everything and AFTER that expand the target widget.
	// Otherwise we might have two widgets visible for a short moment, which could destroy the layout as there is not enough space.
	for (Page *page: m_pages) {
		page->m_widget->setVisible(false);
		page->m_arrowIcon->setPixmap(m_arrowRight);
		page->m_label->setActive(false);
	}

	m_pages[index]->m_widget->setVisible(true);
	m_pages[index]->m_arrowIcon->setPixmap(m_arrowDown);
	m_pages[index]->m_label->setActive(true);

	emit indexChanged(index);
}


void ToolBox::onLabelClicked() {
	unsigned int index = qobject_cast<ClickableLabel*>(sender())->id();
	setCurrentIndex(index);
}


unsigned int ToolBox::currentIndex(){
	for (unsigned int i=0; i<m_pages.size(); ++i) {
		if (m_pages[i]->m_widget->isVisible())
			return i;
	}
	return 0; // for compiler
}


void ToolBox::updatePageBackgroundColorFromStyle() {
	for (Page *page: m_pages) {
		page->m_frame->setStyleSheet(QString("QFrame#%1 {border-radius: 5px; background-color: %2; border: 2px %3}")
									 .arg(page->m_frame->objectName()).arg(Style::ToolBoxPageBackground).arg(Style::ToolBoxPageEdge));
	}
}



} // namespace QtExt
