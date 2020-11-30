/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

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
