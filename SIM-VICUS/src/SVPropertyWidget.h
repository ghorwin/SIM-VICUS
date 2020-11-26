#ifndef SVPROPERTYWIDGET_H
#define SVPROPERTYWIDGET_H

#include <QWidget>

/*! The property widget is located at the right side of the screen.
	Depending on the mode the UI is in, it shows a different set of widgets.
*/
class SVPropertyWidget : public QWidget {
	Q_OBJECT
public:

	explicit SVPropertyWidget(QWidget * parent = nullptr);

	/*! Toggles visibility of different child widgets. */
	void setMode(int m);

private:

};

#endif // SVPROPERTYWIDGET_H
