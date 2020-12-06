#ifndef SVPROPERTYWIDGET_H
#define SVPROPERTYWIDGET_H

#include <QWidget>

class QVBoxLayout;

/*! The property widget is located at the right side of the screen.
	Depending on the mode the UI is in, it shows a different set of widgets.
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
		M_AddVertexesMode,
		NUM_M
	};

	explicit SVPropertyWidget(QWidget * parent = nullptr);

	/*! Toggles visibility of different child widgets.
		Also, constructs property widgets on the fly on first show.
	*/
	void setMode(PropertyWidgets m);

public slots:

	/*! Connected to view state manager. */
	void onViewStateChanged();

private:
	/*! The layout, that holds all widgets. */
	QVBoxLayout				*m_layout = nullptr;
	/*! Pointer to property widget: add polygon */
	QWidget					*m_propWidgets[NUM_M];
};

#endif // SVPROPERTYWIDGET_H
