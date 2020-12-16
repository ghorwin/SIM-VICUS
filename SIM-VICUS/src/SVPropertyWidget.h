#ifndef SVPROPERTYWIDGET_H
#define SVPROPERTYWIDGET_H

#include <QWidget>

namespace IBKMK {
	class Vector3D;
}

class QVBoxLayout;

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
};

#endif // SVPROPERTYWIDGET_H
