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

#include "QtExt_Splitter.h"
#include "QtExt_FeatureWidget.h"

#include <QDebug>
#include <QVBoxLayout>


namespace QtExt {


// *** Wrapper Class Implementation ***


Splitter::Splitter( QWidget* parent ):
	QWidget(parent)
{
	setup(Qt::Vertical);
}


Splitter::Splitter( Qt::Orientation orientation, QWidget* parent ):
		QWidget(parent)
{
	setup(orientation);
}


Splitter::~Splitter() {
	// nothing to do
}


void Splitter::setup( Qt::Orientation orientation) {
	m_splitter = new QSplitter( orientation, this );
	// prevent collapsing of children.
	m_splitter->setChildrenCollapsible(false);
	m_dummy = new QWidget(this);
	m_dummy->setVisible(false);
	QSizePolicy policy = m_dummy->sizePolicy();
	policy.setVerticalPolicy(QSizePolicy::Expanding);
	policy.setVerticalStretch(1);
	m_dummy->setSizePolicy(policy);

	if (orientation == Qt::Horizontal ){

		m_hBLayout = new QHBoxLayout(this);
		m_hBLayout->addWidget( m_splitter);
		m_hBLayout->addWidget( m_dummy );
		m_hBLayout->setContentsMargins(0,0,0,0);

		setLayout( m_hBLayout );

	}
	else {

		m_vBLayout = new QVBoxLayout(this);
		m_vBLayout->addWidget( m_splitter );
		m_vBLayout->addWidget( m_dummy );
		m_vBLayout->setContentsMargins(0,0,0,0);

		setLayout( m_vBLayout );

	}
}


int Splitter::widgetIndex(QWidget * w) {
	for (int i=0;i<m_splitter->count(); ++i)
		if (m_splitter->widget(i) == w)
			return i;
	return -1;
}


void Splitter::addWidget ( QWidget * widget ) {
	int oldIndex = widgetIndex(widget);
	// ensure that we do not have this widget already
	Q_ASSERT(oldIndex == -1);

	m_splitter->addWidget( widget );
	m_collapsedWidgets.push_back(false);
	adjustSizes();
}


void Splitter::insertWidget (int index, QWidget * widget ) {
	int oldIndex = widgetIndex(widget);
	// ensure that we do not have this widget already
	Q_ASSERT(oldIndex == -1);
	Q_ASSERT(index < m_splitter->count());

	m_splitter->insertWidget(index, widget );
	m_collapsedWidgets.insert(index, false);
	adjustSizes();
}


void Splitter::setWidgetCollapsed(QWidget * w, bool collapsed) {
	int index = widgetIndex(w);
	// ensure that this widget exists already in the splitter
	Q_ASSERT(index != -1);
	m_collapsedWidgets[index] = collapsed;
	adjustSizes();
}


void Splitter::resizeWidgetsCollapsed( QtExt::FeatureWidget * widget ){
	setWidgetCollapsed(widget, true);
}


void Splitter::resizeWidgetsExpand( QtExt::FeatureWidget * widget ){
	setWidgetCollapsed(widget, false);
}


void Splitter::adjustSizes() {
	int widgetCount = m_splitter->count();
	Q_ASSERT(widgetCount == m_collapsedWidgets.count());
	// determine, how many widgets are un-collapsed
	int expandedWidgetsCount = widgetCount;
	QList<int> widgetSizes;
	for (int i=0; i<widgetCount; ++i) {
		widgetSizes.append(m_splitter->widget(i)->height());
		if (m_collapsedWidgets[i])
			--expandedWidgetsCount;
	}

	// special case, when all widgets are collapsed, show the dummy widget and set splitter widget sizes to be 1
	if (expandedWidgetsCount == 0) {
		// remove old dummy widget
		m_dummy->setVisible(true);
		for (int i=0; i<widgetCount; ++i)
			widgetSizes[i] = 1;
		m_splitter->setSizes(widgetSizes);

		// disable all size grips
		m_dummy->resize(100,10000);
		return;
	}

	// at least one widget is visible,
	m_dummy->setVisible(false);

	// assume we have 10000 pixels of space and we distribute that evenly between visible widgets
	int normalSize = 10000/expandedWidgetsCount;

	for (int i = 0; i < widgetCount; i++){

		// collapsed widgets get size 1
		if ( m_collapsedWidgets[i] )
			widgetSizes[i] = 1;
		// expanded widgets get evenly distributed size
		else
			widgetSizes[i] = normalSize;
	}
	// we now set these sizes in the splitter

	// Note: The splitter first computes the total space available. Then it scales the widget's sizes
	//       to fill the available space. At the end, the minimumSizeHint()s are evaluated. By setting
	//       the height of our collapsed widgets to 1 we ensure that the computed scaled size is always
	//       less then the minimimSizeHint() and thus the collapsed widgets have minimum size.
	m_splitter->setSizes( widgetSizes );
}

} // namespace QtExt
