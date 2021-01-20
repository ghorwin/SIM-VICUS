#ifndef SVPROPERTYWIDGET_H
#define SVPROPERTYWIDGET_H

#include <QWidget>

namespace IBKMK {
	class Vector3D;
}

class QVBoxLayout;
class SVPropModeSelectionWidget;

#include <SVViewState.h>

/*! The property widget is located at the right side of the screen.
	Depending on the mode the UI is in, it shows a different set of widgets.
	This class is simply a manager that toggles visibility of the widgets,
	and creates them on first use. The latter help speed-up the start of the application,
	since less UI widgets need to be created (and less memory is occupied by the user interface).
*/
class SVPropertyWidget : public QWidget {
	Q_OBJECT
public:

	/*! This enum defines the property widgets that can be shown.
		These are usually mapped to view states.
	*/
	enum PropertyWidgets {
		M_ThermalSimulationProperties,
		M_EditGeometry,
		M_AddGeometry,
		M_AddVertexesMode,
		M_SiteProperties,
		M_EditNetwork,
		NUM_M
	};

	explicit SVPropertyWidget(QWidget * parent = nullptr);

public slots:

	/*! Connected to view state manager.
		Toggles the visibility of the individual property widgets based on the current viewstate.
	*/
	void onViewStateChanged();

private:

	/*! The layout, that holds all widgets. */
	QVBoxLayout				*m_layout = nullptr;
	/*! Pointer to property widget: add polygon */
	QWidget					*m_propWidgets[SVViewState::NUM_PM];

	/*! The widget with property edit selection mode, shown when view is in
		VM_PropertyEditMode.
	*/
	SVPropModeSelectionWidget * m_propModeSelectionWidget = nullptr;

//	/*! creates network property widget and adds it to layout (if not yet existing), sets it visible */
//	void showNetworkPropertyWidget();
};

#endif // SVPROPERTYWIDGET_H
