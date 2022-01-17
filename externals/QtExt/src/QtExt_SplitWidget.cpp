/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#include "QtExt_SplitWidget.h"

#include <QWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>

namespace QtExt {

SplitWidgetHandle::SplitWidgetHandle(Qt::Orientation o, QSplitter *parent) :
	QSplitterHandle(o, parent)
{
	if (o == Qt::Vertical)
		setMinimumHeight(8);
	else
		setMinimumWidth(8);
}

QSplitterHandle *SplitWidgetSplitter::createHandle() {
//	if (orientation() == Qt::Vertical) {
		SplitWidgetHandle * handle = new SplitWidgetHandle(orientation(), this);
		return handle;
/*	}
	else {
		return QSplitter::createHandle(); // horizontal splitter is default for now
	}
*/
}

void SplitWidgetHandle::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	QLinearGradient gradient;
	QRect r = rect();
	gradient.setColorAt(0, Qt::gray);
	gradient.setColorAt(0.5, Qt::blue);
	gradient.setColorAt(1, Qt::gray);
	if (orientation() == Qt::Horizontal) {
		gradient.setStart(r.left(), r.height()/2);
		gradient.setFinalStop(r.right(), r.height()/2);
	}
	else {
		gradient.setStart(r.width()/2, r.top());
		gradient.setFinalStop(r.width()/2, r.bottom());
	}
	painter.fillRect(event->rect(), QBrush(gradient));
}

// *** SplitWidget implementation ***

SplitWidget::SplitWidget(QWidget *parent) :
	QWidget(parent),
	m_rootWidget(NULL),
	m_layout(new QVBoxLayout(this))
{
	m_layout->setMargin(0);
}


SplitWidget::~SplitWidget() {
}


QWidget * SplitWidget::releaseRootWidget() {
	QWidget * oldRoot = m_rootWidget;
	m_rootWidget = NULL;
	if (oldRoot != NULL) {
		m_layout->removeWidget(oldRoot);
		oldRoot->setParent(NULL);
	}
	return oldRoot;
}


void SplitWidget::setRootWidget(QWidget * widget) {
	QWidget * oldRoot = releaseRootWidget();
	delete oldRoot; // safe to call on NULL
	m_rootWidget = widget;
	widget->setParent(this); // we are now parent of the widget
	m_layout->addWidget(widget);
}


QWidget * SplitWidget::child(QSplitter * splitter, bool left) {
	Q_ASSERT(splitter->count() == 2);
	if (left)
		return splitter->widget(0);
	else
		return splitter->widget(1);
}


bool SplitWidget::hasChilds(const QWidget * widget) const {
	const QSplitter * splitter = dynamic_cast<const QSplitter *>(widget);
	if (splitter != NULL) {
		Q_ASSERT(splitter->count() > 0);
		return true;
	}
	else
		return false;
}

QWidget * SplitWidget::neighbor(QWidget * widget) {
	if (widget == m_rootWidget)
		return NULL;
	QWidget * parent = dynamic_cast<QWidget*>(widget->parent());
	QSplitter * parentSplitter = dynamic_cast<QSplitter *>(parent);
	QWidget * other = parentSplitter->widget(0);
	if (other == widget)
		other = parentSplitter->widget(1); // may be a QWidget, or a QSplitter
	QSplitter * splitter = dynamic_cast<QSplitter*>(other);
	if (splitter != NULL)
		return searchWidget(splitter);
	else
		return other;
}

QWidget * SplitWidget::searchWidget(QSplitter * splitter) {
	QWidget * leftChild = splitter->widget(0);
	QSplitter * leftSplitter = dynamic_cast<QSplitter*>(leftChild);
	if (leftSplitter == NULL)
		return leftChild; // non-splitter widget type found
	else
		return searchWidget(leftSplitter);
}

void SplitWidget::split(QWidget * parentWidget, QWidget * newWidget, Qt::Orientation orientation, bool left) {
	parentWidget->hide();
	//QSize s = parentWidget->size();
	// get parent splitter widget, may be NULL if parentWidget is not contained in a splitter
	QWidget * parent = dynamic_cast<QWidget*>(parentWidget->parent());
	QSplitter * parentSplitter = dynamic_cast<QSplitter *>(parent);
	// remember insert position of parentWidget later
	int insertPosition = 0;
	if (parentSplitter != NULL) {
		Q_ASSERT(parentSplitter->count() == 2);
		if (parentSplitter->widget(1) == parentWidget)
			insertPosition = 1;
	}
	// create new splitter
	Qt::Orientation splitterOrientation;
	if (orientation == Qt::Horizontal)		splitterOrientation = Qt::Vertical;
	else									splitterOrientation = Qt::Horizontal;

	QSplitter * splitter = new QSplitter(splitterOrientation, parent);
//	QSplitter * splitter = new SplitWidgetSplitter(splitterOrientation, parent);


	splitter->setChildrenCollapsible(false);
	// add parentWidget to splitter
	splitter->addWidget(parentWidget);
	// reparenting parentWidget will remove it from the old QSplitter
	parentWidget->setParent(splitter);
	// insert newWidget in splitter depending on left
	if (left)
		splitter->insertWidget(0, newWidget);
	else
		splitter->addWidget(newWidget);
	// reparent the widget
	newWidget->setParent(splitter);

	// special handling for root widget
	if (parentWidget == m_rootWidget) {
		m_rootWidget = splitter;
		m_layout->removeWidget(parentWidget);
		m_layout->addWidget(splitter);
		// since the top-level widget is most likely in a layout, we need to findout the layout position where
	}
	else {
		Q_ASSERT(parentSplitter != NULL);
		parentSplitter->insertWidget(insertPosition, splitter);
	}
	parentWidget->show();
	QList<int> sizes;
	sizes.append(1);
	sizes.append(1);
	splitter->setSizes(sizes);
}


void SplitWidget::removeWidget(QWidget * widget) {
	// widget must not be the top-level widget
	Q_ASSERT(widget != m_rootWidget);
	// widget must have a splitter parent
	QWidget * parent = dynamic_cast<QWidget*>(widget->parent());
	QSplitter * parentSplitter = dynamic_cast<QSplitter *>(parent);
	Q_ASSERT(parentSplitter != NULL);
	// get the neighbor of the widget to be removed (might be a splitter itself)
	QWidget * neighbor = parentSplitter->widget(0);
	if (parentSplitter->widget(0) == widget)
		neighbor = parentSplitter->widget(1);
	// special casse, parentSplitter is our root widget
	if (parentSplitter == m_rootWidget) {
		// set the widget's neighbor as new root widget
		neighbor->setParent(this);
		m_layout->addWidget(neighbor);
		// delete splitter and widget
		delete parentSplitter; // will delete its remaining child widget as well
		// set neighbor as new root widget
		m_rootWidget = neighbor;
	}
	else {
		// parentSplitter must have a parent splitter itself
		QSplitter * grandParentSplitter = dynamic_cast<QSplitter *>(parentSplitter->parent());
		Q_ASSERT(grandParentSplitter != NULL);
		// determine insert position
		int insertPosition = 0;
		if (grandParentSplitter->widget(1) == parentSplitter)
			insertPosition = 1;
		neighbor->setParent(grandParentSplitter);
		// delete splitter and widget
		delete parentSplitter; // will delete its remaining child widget as well
		grandParentSplitter->insertWidget(insertPosition, neighbor);
	}
}


} // namespace QtExt
