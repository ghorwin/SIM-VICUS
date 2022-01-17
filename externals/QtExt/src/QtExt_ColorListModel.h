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

#ifndef QtExt_ColorListModelH
#define QtExt_ColorListModelH

#include <QAbstractListModel>
#include <QList>
#include <QColor>

namespace QtExt {


/*! \brief The class ColorListModel is a model for a combobox.
*/
class ColorListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	/*!  Custom item roles supported by this model.	*/
	enum CustomItemRole {
		ColorRole	  = Qt::UserRole + 1		///< Role for color of the item.
	};

	explicit ColorListModel(QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role) const;

	/*! Returns the index for the actual item including a internalPointer to the data item. */
	virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

	static int	m_colorCounter;

	/*! Append all user given colors. */
	void appendUserColor( QColor );

	/*! Append all user given colors. */
	void appendUserColor( QList< QColor > );

	/*! Set a new color count.*/
	void setColorCount(unsigned int count);

private:

	/*! Add predefined colors to the color list. */
	void addColors();

	/*! Holds all predefined colors. s*/
	QList<QColor>	m_colors;

	/*! User colors used in db views.*/
	QList< QColor >	m_dbColors;

};

} // namespace QtExt

#endif // QtExt_ColorListModelH
