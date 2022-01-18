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

#ifndef QtExt_SplitWidgetH
#define QtExt_SplitWidgetH

#include <QWidget>
#include <QSplitter>
#include <QSplitterHandle>

class QWidget;
class QSplitter;
class QVBoxLayout;


namespace QtExt {

/*! A manager class for handling widgets that can be "split" and "joined".
	This class manages a tree of widgets that is composed of splitters.

	\code
	// Initially the tree should receive a root widget
	splitWidget->setRootWidget(myWidget); // myWidget now belongs to splitWidget
	// the split widget manager now holds:
	// myWidget (root)

	// then, we can split the widget vertically, and insert the new widget on the left side
	splitWidget->split(myWidget, anotherWidget, Qt::Vertical, true);
	// the split widget manager now holds:
	// QSplitter:Vertical (root)
	//   - anotherWidget
	//   - myWidget

	// if we split the widget to the right again, we call
	splitWidget->split(myWidget, widgetThree, Qt::horizontal, false);
	// the split widget manager now holds:
	// QSplitter:Vertical (root)
	//   - anotherWidget
	//   - QSplitter:Horizontal
	//     - myWidget
	//     - widgetThree

	// we can remove a widget
	splitWidget->remove(anotherWidget);
	// QSplitter:Horizontal (root)
	//   - myWidget
	//   - widgetThree
	\endcode
*/
class SplitWidget : public QWidget {
	Q_OBJECT
public:
	SplitWidget(QWidget *parent);
	~SplitWidget();


	/*! Returns the root widget, may be NULL if no root widget has been set. */
	QWidget * rootWidget() { return m_rootWidget; }

	/*! Returns pointer to current root widget (may be NULL) and removes parent
		from widget so that it is no longer owned by SplitWidgetManager.
		After a call to this function the SplitWidgetManager does not have a root widget
		anylonger.
	*/
	QWidget * releaseRootWidget();

	/*! Sets/replaces root widget.
		If a root widget previously existed, it will be deleted (and all its children).
	*/
	void setRootWidget(QWidget * widget);

	/*! Returns the left/top widget in the splitter.
		This is a convenience function, even though the member function of QSplitter
		could be used instead as well.
		\note This function raises an Assertion if the parent does not have a child.
		\return Returns a child widget/splitter (you may have to cast QWidget to QSplitter to
				check this).
	*/
	QWidget * child(QSplitter * splitter, bool left);

	/*! Returns the nearest neighbor of a widget inside the split widget.
		If the widget is the root widget, the function returns NULL.
		The nearest neighbor widget is the widget that is found if the widget's parent splitter is taken
		as starting point and the tree is traversed taking always left widgets in a splitter, until
		a widget is found.
	*/
	QWidget * neighbor(QWidget * widget);

	/*! Returns the first non-QSplitter widget in a left/top sided recursive search through the tree. */
	QWidget * searchWidget(QSplitter * splitter);

	/*! Returns true if widget (may be a splitter) has children. */
	bool hasChilds(const QWidget * widget) const;

	/*! Splits a widget into two.
		\param parentWidget The widget that should be split into two.
		\param newWidget The widget that is inserted besides parentWidget
		\param orientation The split direction.
		\param left Insert new widget either to the left/top or right/bottom of parentWidget.
	*/
	void split(QWidget * parentWidget, QWidget * newWidget, Qt::Orientation orientation, bool left);

	/*! Removes a widget.
		\note Widget is unparented and deleted.
	*/
	void removeWidget(QWidget * widget);

private:
	QWidget		*m_rootWidget;
	QVBoxLayout	*m_layout;
};

/*! Custom splitter handle. */
class SplitWidgetHandle : public QSplitterHandle {
	Q_OBJECT
public:
	SplitWidgetHandle(Qt::Orientation o, QSplitter *parent);

protected:
	virtual void paintEvent(QPaintEvent *event);

};

/*! Custom splitter with own splitter handle for our split widget. */
class SplitWidgetSplitter : public QSplitter {
	Q_OBJECT
public:
	SplitWidgetSplitter(Qt::Orientation orientation, QWidget *parent = 0) :
		QSplitter(orientation, parent)
	{}

protected:
	QSplitterHandle *createHandle();
};


} // namespace QtExt

#endif // QtExt_SplitWidgetH
