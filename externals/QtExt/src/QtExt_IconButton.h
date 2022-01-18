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

#ifndef QtExt_IconButtonH
#define QtExt_IconButtonH

#include <QWidget>
#include <QPixmap>

class QShortcut;
class QKeysequence;


namespace QtExt {

/*! An improved and unified tool button. */
class IconButton : public QWidget {
	Q_OBJECT

	/*! Button states defined for the icon button. */
	enum ButtonState {
		NormalState,		///< Normal button icon.
		ActiveState,		///< Highlighted (mouse hover).
		DisabledState,		///< Disabled.
		NumButtonStates
	};

public:
	/*! Constructor. */
	explicit IconButton(QWidget * parent);

	~IconButton();

	/*! Function to set the icons for the normal button mode. */
	void setNormalIcons(const QPixmap & normalIcon,
		const QPixmap & activeIcon,
		const QPixmap & disabledIcon) ;

	/*! Function to set the icons for the checked button mode. */
	void setCheckedIcons(const QPixmap & normalIcon,
		const QPixmap & activeIcon,
		const QPixmap & disabledIcon);

	/*! Overloaded member function to trigger an update of the button. */
	void setEnabled(bool checked);

	/*! Sets whether the icon button can be checked. */
	void setCheckable(bool enabled);
	/*! Returns whether the icon button can be checked. */
	bool isCheckable() const;

	/*! Sets the check state of the icon button. */
	void setChecked(bool checked);
	/*! Returns whether the icon button is currently checked. */
	bool isChecked() const { return m_checked; }

	/*! Sets the shortcut for this icon button. */
	void setShortCut( const QKeySequence keySequence );

	/*! Returns the assigned short cut */
	QShortcut * shortcut() const { return m_shortcut; }

signals:
	/*! Emitted when the button is left-clicked. */
	void clicked();

	/*! Emitted when the button is left-clicked and checkable. */
	void clicked(bool);

protected:
	/*! Re-implemented to draw the icon button. */
	virtual void paintEvent ( QPaintEvent * event );
	/*! Re-implemented to highlight the button (if enabled). */
	virtual void enterEvent ( QEvent * event );
	/*! Re-implemented to remove highlighting. */
	virtual void leaveEvent ( QEvent * event );
	/*! Re-implemented to check for click events. */
	virtual void mousePressEvent ( QMouseEvent * event );
	/*! Re-implemented to toggle active/not active state when moving mouse while pressing a button. */
	virtual void mouseMoveEvent ( QMouseEvent * event );
	/*! Re-implemented to check for release events.
		The release event emits the clicked() signal, but only if the user is inside the widget. */
	virtual void mouseReleaseEvent ( QMouseEvent * event );

private slots:
	/*! Connected to the shortcut.
		If triggered, emits the clicked() or clicked(bool) signal.
		*/
	void shortcutClicked();

private:

	/*! Shortcut of the icon button. */
	QShortcut		*m_shortcut;

	/*! If true, Icon button is checked. */
	bool			m_checked;
	/*! If true, Icon button is checkable and can emit the clicked(bool) signal. */
	bool			m_checkable;

	/*! True, when mouse cursor hovers over widget.
		The mouseReleaseEvent() only fires the clicked() or clicked(bool) signal
		when m_insideWidget is true.
	*/
	bool			m_insideWidget;
	/*! True, if user holds the mouse button.
		Together with m_checked this flag is used to show a toggled view of the check state.
		m_checked is only changed if the user actually releases the mouse button within the widget.
	*/
	bool			m_mouseButtonDown;

	/*! Pixmaps for normal button. */
	QPixmap	m_icons[NumButtonStates];
	/*! Pixmaps for checked button. */
	QPixmap	m_checkedIcons[NumButtonStates];
};

} //namespace QtExt {


#endif // QtExt_IconButtonH
