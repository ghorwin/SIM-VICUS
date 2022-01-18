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

#ifndef QtExt_TextFrameWidgetBaseH
#define QtExt_TextFrameWidgetBaseH

#include <memory>

#include <QObject>
#include <QWidget>
#include <QTextDocument>

class QPaintDevice;
class QPainter;

namespace QtExt {

class TextFrameWidgetBase : public QWidget
{
	Q_OBJECT
public:
	explicit TextFrameWidgetBase(QWidget *parent = nullptr);

	/*! Destructor.
		Implemented because this class is a abstract base class.
	*/
	virtual ~TextFrameWidgetBase() {}

	/*! Renders the complete frame with the given painter.
		\param p Painter for drawing.
		\param frame External frame for drawing (maximum width, height and position).
	*/
	virtual void print(QPainter * p, const QRectF & frame) = 0;

protected:
	/*! Overrides paint event.*/
	virtual void paintEvent ( QPaintEvent * event );

signals:

protected:
	std::unique_ptr<QTextDocument>	m_textDocument;			///< Pointer to text document object responsible for rendering text and images.
};

} // namespace QtExt

#endif // QtExt_TextFrameWidgetBaseH
