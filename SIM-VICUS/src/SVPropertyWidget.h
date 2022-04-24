/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVPropertyWidgetH
#define SVPropertyWidgetH

#include <QWidget>
#include <QLayout>

namespace IBKMK {
	class Vector3D;
}

class SVPropModeSelectionWidget;
class ModificationInfo;

#include <SVViewState.h>

/*! The property widget is located at the right side of the screen.
	Depending on the mode the UI is in, it shows a different set of widgets.
	This class is simply a manager that toggles visibility of the widgets,
	and creates them on first use. The latter helps speed-up the start of the application,
	since less UI widgets need to be created (and less memory is occupied by the user interface).

	When the view is in property edit mode (SVViewState::VM_PropertyEditMode), the embedded
	SVPropModeSelectionWidget is used to toggle/switch between individual mode widgets.

*/
class SVPropertyWidget : public QWidget {
	Q_OBJECT
public:

	/*! This enum defines the property widgets that can be shown.
		These usually correspond to view states.
	*/
	enum PropertyWidgets {
		M_AddGeometry,
		M_EditGeometry,
		M_VertexListWidget,
		M_SiteProperties,
		M_NetworkProperties,
		M_BuildingProperties,
		M_FloorManager,
		M_AddWindowWidget,
		NUM_M
	};

	explicit SVPropertyWidget(QWidget * parent = nullptr);

public slots:

	/*! Connected to view state manager.
		Toggles the visibility of the individual property widgets based on the current viewstate.
	*/
	void onViewStateChanged();

private:
	/*! Convenience function for creating a widget, if not existing yet, adding it to
		the layout and showing it.
	*/
	template <typename T>
	void showPropertyWidget(PropertyWidgets propWidget) {
		if (m_propWidgets[propWidget] == nullptr) {
			T * w = new T(this);
			m_propWidgets[propWidget] = w;
			m_layout->addWidget(m_propWidgets[propWidget]);
		}
		m_propWidgets[propWidget]->setVisible(true);
	}

	/*! The layout, that holds all widgets. */
	QVBoxLayout				*m_layout = nullptr;
	/*! Pointer to property widget: add polygon */
	QWidget					*m_propWidgets[NUM_M];

	/*! The widget with property edit selection mode, shown when view is in
		VM_PropertyEditMode.
	*/
	SVPropModeSelectionWidget * m_propModeSelectionWidget = nullptr;
};

#endif // SVPropertyWidgetH
