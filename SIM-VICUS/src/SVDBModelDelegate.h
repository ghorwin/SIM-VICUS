/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVDBModelDelegateH
#define SVDBModelDelegateH

#include <QItemDelegate>

/*!	Colors background and text of built-in database items in different colors.
	Expects built-in
*/
class SVDBModelDelegate : public QItemDelegate {
	Q_OBJECT
public:
	/*! Default constructor. */
	SVDBModelDelegate(QObject * parent, int builtInRole, int localRole, int referencedRole);
	/*! Default destructor. */
	~SVDBModelDelegate();

	void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

	/*! Data role to check for boolean (true) if item is a "built-in". */
	int m_builtInRole;

	/*! Data role to check for boolean (true) if item is a "local". */
	int m_localRole;

	/*! Data role to check for boolean (true) if item is referenced in current project. */
	int m_referencedRole;
};

#endif // SVDBModelDelegateH
