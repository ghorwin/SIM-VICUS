/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

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
