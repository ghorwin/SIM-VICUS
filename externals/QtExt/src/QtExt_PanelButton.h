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

#ifndef QtExt_PanelButtonH
#define QtExt_PanelButtonH

#include <QToolButton>

class QWidget;



namespace QtExt {


/*! The shade/panel button that is used to hide/show the west/east/north/south panels.
	The panel button has two states:
	* checked, the corresponding panel widget is visible and the arrow points outwards
	* unchecked, the corresponding panel widget is invisible and the arrow points inwards
*/
class PanelButton : public QToolButton {
	Q_OBJECT
public:

	/*! The possible directions/alignments of the panel button within the GUI. */
	enum Direction{
		DD_NORTH,	///< Orientation north, horziontal upper edge of the screen.
		DD_SOUTH,	///< Orientation south, horziontal lower edge of the screen.
		DD_EAST,	///< Orientation east, vertical right edge of the screen.
		DD_WEST,	///< Orientation west, vertical left edge of the screen.
	};

	/*! Constructor, takes a \a parent and the panel button \a direction. */
	explicit PanelButton( QWidget *parent, Direction direction );

	/*! Makes run-time switching of international shortcuts possible */
	void retranslateUi();

private:

	/*! Holds the direction/location of the button.
		\sa Direction
	*/
	Direction m_direction;
};

} // namespace QtExt


#endif // QtExt_PanelButtonH
