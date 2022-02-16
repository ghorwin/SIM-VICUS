/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

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
