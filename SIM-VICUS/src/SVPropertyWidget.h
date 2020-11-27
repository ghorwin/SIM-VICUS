#ifndef SVPROPERTYWIDGET_H
#define SVPROPERTYWIDGET_H

#include <QWidget>

class SVPropAddPolygonWidget;
class QVBoxLayout;

/*! The property widget is located at the right side of the screen.
	Depending on the mode the UI is in, it shows a different set of widgets.
*/
class SVPropertyWidget : public QWidget {
	Q_OBJECT
public:
	/*! This enum defines which property widgets are to be shown. */
	enum PropertyWidgetMode {
		M_ThermalSimulationProperties,
		M_EditGeometry,
		M_AddVertexesMode,
		NUM_M
	};

	explicit SVPropertyWidget(QWidget * parent = nullptr);

	/*! Toggles visibility of different child widgets. */
	void setMode(PropertyWidgetMode m);

private:
	/*! The layout, that holds all widgets. */
	QVBoxLayout				*m_layout = nullptr;
	/*! Pointer to property widget: add polygon */
	QWidget					*m_propWidgets[NUM_M];
};

#endif // SVPROPERTYWIDGET_H
