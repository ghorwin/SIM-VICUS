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
