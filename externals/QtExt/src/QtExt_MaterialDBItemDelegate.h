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

#ifndef QtExt_MaterialDBItemDelegateH
#define QtExt_MaterialDBItemDelegateH

#include <QItemDelegate>

namespace QtExt {


/*! This class implementation is only used to draw the background colors for the material list.
*/
class MaterialDBItemDelegate : public QItemDelegate {
	Q_OBJECT
public:
	explicit MaterialDBItemDelegate(QObject *parent = 0);

	/*! Set view to use default color setting (\sa QtExt::Style).*/
	void setDefaultColors();

	/*! Set colors for user defined color setting for built-in materials.*/
	void setUserDefinedColors(const QColor& bright, const QColor& dark);

	/*! Set color for not built-in materials.*/
	void setUserColor(const QColor& col);

	/*! Set use of alternating colors for rows in view.
		Default setting is on.
		Colors can be set by using setUserColors or setDefaultColors.
		\param alternate If true alternating colors are used.
	*/
	void setAlternatingColors(bool alternate);

protected:
	virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

private:
	bool									m_alternatingColors;
	bool									m_defaultColors;
	QColor									m_colorUserMaterials;
	QColor									m_colorAlternativeBackgroundBright;
	QColor									m_colorAlternativeBackgroundDark;
};

} // end namespace

#endif // QtExt_MaterialDBItemDelegateH
