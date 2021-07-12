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
