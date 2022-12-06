#include "QtExt_ToolBox.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QtDebug>

#include <IBK_Exception.h>


namespace QtExt {

ToolBox::ToolBox(QWidget *parent):
	QWidget(parent), m_layout(new QVBoxLayout(this))
{
	setLayout(m_layout);
	m_arrowDown = QPixmap(":/gfx/master/arrow_down.png");
	m_arrowRight = QPixmap(":/gfx/master/arrow_right.png");
}

ToolBox::~ToolBox() {
	for (Page *page: m_pages)
		delete page;
}

void ToolBox::addPage(const QString & headerName, QWidget * widget, QIcon * icon, int headerFontSize) {

	// create header layout and add arrow, icon (if existing) and label to it
	QHBoxLayout *hLay = new QHBoxLayout(this);

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
	hLay->setContentsMargins(0,0,0,0);
	QFrame *hFrame = new QFrame(this);
	hFrame->setLayout(hLay);

	// create vertical layout that contains the header frame and widget
	QVBoxLayout *vLay = new QVBoxLayout(this);
	vLay->addWidget(hFrame);
	widget->layout()->setContentsMargins(20, 0, 10, 10);
	vLay->addWidget(widget);
	vLay->setContentsMargins(2,2,2,2);

	// we add the vertical layout to a frame again
	QFrame *vFrame = new QFrame(this);
	vFrame->setObjectName(QString("vFrame%1").arg(m_pages.size()));
	vFrame->setFrameShape(QFrame::Box);
	vFrame->setStyleSheet(QString("QFrame#%1 {border-radius: 5px}").arg(vFrame->objectName()));
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
	return m_pages[index]->m_widget;
}

void ToolBox::setCurrentIndex(unsigned int index) {
	// Note: We first collapse everything and AFTER that expand the target widget.
	// Otherwise we might have two widgets visible for a short moment, which could destroy the layout as there is not enough space.
	for (Page *page: m_pages) {
		page->m_widget->setVisible(false);
		page->m_arrowIcon->setPixmap(m_arrowRight);
	}

	m_pages[index]->m_widget->setVisible(true);
	m_pages[index]->m_arrowIcon->setPixmap(m_arrowDown);
}


void ToolBox::onLabelClicked() {
	int index = qobject_cast<ClickableLabel*>(sender())->id();
	setCurrentIndex(index);
	emit indexChanged(index);
}


unsigned int ToolBox::currentIndex(){
	for (unsigned int i=0; i<m_pages.size(); ++i) {
		if (m_pages[i]->m_widget->isVisible())
			return i;
	}
	return 0; // for compiler
}



} // namespace QtExt
