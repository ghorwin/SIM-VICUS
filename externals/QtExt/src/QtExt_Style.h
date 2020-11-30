/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef QtExt_StyleH
#define QtExt_StyleH

#include <QString>
#include <QWidget>

namespace QtExt {

class FeatureWidget;

/*! This class encapsulates all style-related properties that affect the appearance of widgets
	in the QtExt library.

	By using the constants in this class, a uniform look-and-feel of all QtExt widgets is ensured.

	If an application wishes to use different colors, the style object's static members must be set prior
	to creating any widgets in the UI since active monitoring of style changes is not implemented in the widgets.
*/
class Style {
public:

	/*! To be used for any edit fields (line edits etc.) when they are in normal/valid state. */
	static QString EditFieldBackground;
	/*! To be used as alternative edit field backgrounds, e.g. when having tables with editable lines. */
	static QString AlternativeEditFieldBackground;
	/*! Background color when input is in invalid state. */
	static QString ErrorEditFieldBackground;
	/*! Background color when input is in invalid state but for alternative row colors. */
	static QString AlternativeErrorEditFieldBackground;
	/*! Background color when input is read-only. */
	static QString ReadOnlyEditFieldBackground;

	static QString AlternativeBackgroundBright;
	static QString AlternativeBackgroundDark;
	static QString AlternativeBackgroundText;

	/*! This adjusts layout and palette style of a QtExt::FeatureWidget.
		This function expects a content widget as second widget of the
		feature widget's layout and calls adjustContentWidget() on it.
	*/
	static void adjustFeatureWidget(FeatureWidget * widget);

	/*!
		Adjusts a layout that contains just property widgets.
		\sa adjustFeatureWidget
	*/
	static void adjustFeaturePropertyWidget(FeatureWidget * widget);

	/*! Style adjustment function for a widget with a QVBoxLayout which contains feature widgets.
		This will adjust the widget and also its layout containing feature widgets.
		This function calls adjustFeatureWidget() for each feature widget in the widget's layout.
	*/
	static void adjustScrollAreaWithFeatureWidgets(QWidget * widget);


	/*!
		Design for feature widgets that contains a property widgets.
		There must be only one property widget.
		\sa adjustScrollAreaWithFeatureWidgets
	*/
	static void adjustScrollAreaWithFeaturePropertyWidgets(QWidget * widget);

private:
	/*! This will adjust the content widget and also its layout if set.
		This function expects a QBoxLayout (or derived) set as layout in the widget.
	*/
	static void adjustContentWidget(QWidget * widget);

	/*!
		Adjusts a content widget that contains a property widget.
		\sa adjustContentWidget
	*/
	static void adjustContentPropertyWidget(QWidget * widget);
};


} // namespace QtExt


#endif // QtExt_StyleH
