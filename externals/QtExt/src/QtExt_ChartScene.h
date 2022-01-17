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

#ifndef QtExt_ChartSceneH
#define QtExt_ChartSceneH

#include <QObject>
#include <QGraphicsScene>

namespace QtExt {

class ChartScene : public QGraphicsScene {
	Q_OBJECT
public:
	ChartScene(bool onScreen, QPaintDevice *device, QObject* parent = 0);

	/*! The main interface for painting the diagram.*/
	void setup(QRect frame, QPaintDevice *device, double res);

protected:
	qreal penWidth(qreal minWidth, qreal resWidth);

	double m_topBorder;
	double m_bottomBorder;
	double m_leftBorder;
	double m_rightBorder;

	/*! Line width of the frame rectangle.*/
	double				m_frameWidth;

	/*! Scene rectangle.*/
	QRectF				m_frame;

	/*! Chart rectangle.*/
	QRectF				m_chartFrame;

	/*! True if chart is drawn on screen, false if chart is printed
		or exported as picture.
	*/
	bool				m_onScreen;

	/*! The resolution of the chart on a printer or exported image in pixel/mm.
		This is used to calculate the various distances and lengths.
	*/
	double				m_res;
	QPaintDevice*		m_device;					///< Paintdevice.

};

} // end namespace QtExt

#endif // QtExt_ChartSceneH
