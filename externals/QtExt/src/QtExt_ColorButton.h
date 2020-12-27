/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef QtExt_ColorButtonH
#define QtExt_ColorButtonH

#include <QPushButton>
#include <QColor>

namespace QtExt {

/*! A tool button with special color background that can be used to select this color
*/
class ColorButton : public QPushButton {
	Q_OBJECT
public:
	explicit ColorButton(QWidget *parent = nullptr);

	/*! Sets the color and triggers a repaint. */
	void setColor(const QColor & c);

	/*! Returns the currently selected color. */
	const QColor color() const { return m_color; }

	/*! Sets a special "disabled" state - button will be disabled
		but being painted as regular.
	*/
	void setReadOnly(bool readOnly);

	void setEnabled(bool enabled);

signals:
	/*! Emitted, when color has been changed by user. */
	void colorChanged();

protected:
	virtual void paintEvent( QPaintEvent* );

private slots:
	void onClicked();

private:
	/*! Holds the color to be drawn on the button. */
	QColor	m_color;

	bool	m_readOnly;

};

} // namespace QtExt


#endif // QtExt_ColorButtonH
