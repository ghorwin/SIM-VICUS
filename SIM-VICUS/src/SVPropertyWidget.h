#ifndef SVPROPERTYWIDGET_H
#define SVPROPERTYWIDGET_H

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
		M_Geometry,
		M_VertexListWidget,
		M_SiteProperties,
		M_NetworkProperties,
		M_BuildingProperties,
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

	/*! This function configures the viewstate object vs in property-edit-mode to match the
		edit mode selection in the SVPropModeSelectionWidget.
		This function should be called when the property mode is just switched on.
	*/
	void updateViewStateToEditMode(SVViewState & vs) const;

	/*! The layout, that holds all widgets. */
	QVBoxLayout				*m_layout = nullptr;
	/*! Pointer to property widget: add polygon */
	QWidget					*m_propWidgets[NUM_M];

	/*! The widget with property edit selection mode, shown when view is in
		VM_PropertyEditMode.
	*/
	SVPropModeSelectionWidget * m_propModeSelectionWidget = nullptr;
};

#endif // SVPROPERTYWIDGET_H
