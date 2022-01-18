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

#include "QtExt_ColorListModel.h"

#include <QColor>

namespace QtExt {

const int NUM_DEFAULT_COLORS = 16;
const QColor DEFAULT_COLORS[NUM_DEFAULT_COLORS] = {
	QColor( "#416fce"),
	QColor( "#ffaa00"),
	QColor( "#ff007f"),
	QColor( "#7141d1"),
	QColor( "#ffee00"),
	QColor( "#45ad45"),
	QColor( "#aa0000"),
	QColor( "#55aaff"),
	QColor( "#005500"),
	QColor( "#550000"),
	QColor( "#ffaaff"),
	QColor( "#c66300"),
	QColor( "#ff5500"),
	QColor( "#aa55ff"),
	QColor( "#aaaa00"),
	QColor( "#ffffff")
};

int ColorListModel::m_colorCounter = 0;

ColorListModel::ColorListModel(QObject *parent) :
	QAbstractListModel(parent)
{
	// set default colors for db
	for (int i=0; i<NUM_DEFAULT_COLORS; ++i)
		m_dbColors.append(DEFAULT_COLORS[i]);

	addColors();
}

int ColorListModel::rowCount(const QModelIndex &/*parent*/) const {
	return m_colors.size();
}

QVariant ColorListModel::data(const QModelIndex &index, int role) const {

	if (!index.isValid())
		return QVariant();

	if (role == Qt::BackgroundRole) {
		int pos = ( index.row() + m_dbColors.count() ) % m_dbColors.count();
		return QColor(m_colors.at( pos ));
	}

	if (role == ColorListModel::ColorRole)
		return QColor(m_colors.at(index.row()));

	if (role == Qt::DisplayRole)
		return QString(" ");
	else
		return QVariant();
}

QModelIndex ColorListModel::index ( int row, int column, const QModelIndex & /*parent*/ ) const {

	return createIndex(row,column);
}

void ColorListModel::addColors() {

	m_colors.clear();
	// add default and usercolors from settings
	m_colors.append( m_dbColors );
}

void ColorListModel::appendUserColor( QColor color ){
	m_colors.push_back( color );
}

void ColorListModel::appendUserColor( QList< QColor > evenMoreColor ){
	m_colors.append( evenMoreColor );
}

} // namespace PPP
