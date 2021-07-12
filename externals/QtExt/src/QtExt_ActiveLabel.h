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

#ifndef QtExt_ActiveLabelH
#define QtExt_ActiveLabelH

#include <QLabel>
#include <QPalette>

class QString;


namespace QtExt {


/*! The active label is just a label that has a hover event implemented which switches
	the label's palette from normal color to hover color.
*/
class ActiveLabel : public QLabel {
	Q_OBJECT
public:
	explicit ActiveLabel(const QString & text);
	explicit ActiveLabel(QWidget * parent);
	virtual ~ActiveLabel();

	void setHoverColor(const QColor & c);
	const QColor & hoverColor() const;

	void setNormalColor(const QColor & c);
	const QColor & normalColor() const;

	/*! Resets colors to normal. */
	void reset();

public slots:
	/*! Simply emits the clicked() signal. */
	void click();

signals:
	/*! Signal of QLabel passed through. */
	void clicked(bool);

protected:
	virtual void enterEvent ( QEvent * event );
	virtual void leaveEvent ( QEvent * event );
	virtual void mousePressEvent ( QMouseEvent * event );

private:
	void initColors();

	QColor		m_hoverColor;
	QColor		m_normalColor;
};

} // namespace QtExt {


#endif // QtExt_ActiveLabelH
