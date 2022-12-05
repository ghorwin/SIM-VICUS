#include "QtExt_ToolBox.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QtDebug>

#include <IBK_Exception.h>

#include "QtExt_ClickableLabel.h"

namespace QtExt {

ToolBox::ToolBox(QWidget *parent): QWidget{parent}
{
	m_layout = new QVBoxLayout;
	setLayout(m_layout);
	m_arrowDown = QPixmap(":/gfx/master/arrow_down.png");
	m_arrowRight = QPixmap(":/gfx/master/arrow_right.png");
}


void ToolBox::addPage(const QString & headerName, QWidget * widget, QIcon * icon,
						QFont::Weight headerFontWeight, int headerFontSize) {

	QFont fnt;
	fnt.setPointSize(headerFontSize);

	// create header layout and add arrow, icon (if existing) and label to it
	QHBoxLayout *hLay = new QHBoxLayout;

	ClickableLabel *labelArrow = new ClickableLabel(m_widgets.size(), "", this);
	labelArrow->setPixmap(m_arrowRight);
	QSizePolicy pol;
	pol.setHorizontalPolicy(QSizePolicy::Fixed);
	labelArrow->setSizePolicy(pol);
	hLay->addWidget(labelArrow);
	m_arrowIcons.push_back(labelArrow);

	if (icon != nullptr) {
		ClickableLabel *labelIcon = new ClickableLabel(m_widgets.size(), "", this);
		labelIcon->setPixmap(icon->pixmap(10));
		labelIcon->setSizePolicy(pol);
		hLay->addWidget(labelIcon);
	}

	ClickableLabel *label = new ClickableLabel(m_widgets.size(), headerName, this);
	label->setFont(fnt);
	hLay->addWidget(label);
	m_labels.push_back(label);

	// we put the header layout into a frame for consistent background colors
	hLay->setContentsMargins(0,0,0,0);

	QFrame *hFrame = new QFrame;
	hFrame->setLayout(hLay);

	// create vertical layout that contains the header frame and widget
	QVBoxLayout *vLay = new QVBoxLayout;
	vLay->addWidget(hFrame);
	widget->layout()->setContentsMargins(20, 0, 10, 10);
	vLay->addWidget(widget);
	vLay->setContentsMargins(2,2,2,2);
	m_widgets.push_back(widget);

	// the vertical layout is added to a frame again
	QFrame *vFrame = new QFrame;
	vFrame->setObjectName(QString("vFrame%1").arg(m_frames.size()));
	vFrame->setFrameShape(QFrame::Box);
	vFrame->setStyleSheet(QString("QFrame#%1 {border-radius: 5px}").arg(vFrame->objectName()));
	vFrame->setLayout(vLay);
	m_layout->addWidget(vFrame);
	m_frames.push_back(vFrame);

	// connections
	connect(labelArrow, qOverload<int>(&ClickableLabel::clicked),
			this, &ToolBox::onLabelClicked);
	connect(label, qOverload<int>(&ClickableLabel::clicked),
			this, &ToolBox::onLabelClicked);

	Q_ASSERT(m_widgets.size()==m_arrowIcons.size());

	// update
	onLabelClicked(0);
}


void ToolBox::setCurrentIndex(unsigned int index){
	onLabelClicked(index);
}

void ToolBox::onLabelClicked(unsigned int index) {

	for (ClickableLabel *l: m_labels) {
		l->setStyleSheet("QLabel { font-weight: normal }");
		l->m_active = false;
	}
	m_labels[index]->setStyleSheet("QLabel { font-weight: bold }");
	m_labels[index]->m_active = true;


	for (QWidget *w: m_widgets)
		w->setVisible(false);
	m_widgets[index]->setVisible(true);

	for (ClickableLabel *label: m_arrowIcons)
		label->setPixmap(m_arrowRight);
	m_arrowIcons[index]->setPixmap(m_arrowDown);

	emit indexChanged(index);
}


unsigned int ToolBox::currentIndex(){
	for (unsigned int i=0; i<m_widgets.size(); ++i) {
		if (m_widgets[i]->isVisible())
			return i;
	}
	return 0; // for compiler
}



} // namespace QtExt
