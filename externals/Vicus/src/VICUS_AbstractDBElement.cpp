/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "VICUS_AbstractDBElement.h"

#include <QCoreApplication>

namespace VICUS {

void AbstractDBElement::collectChildren(std::set<const AbstractDBElement *> & allChildrenRefs) const {
	for (VICUS::AbstractDBElement * child: m_childrenRefs){
		if (child != nullptr){
			allChildrenRefs.insert(child);
			child->collectChildren(allChildrenRefs);
		}
	}
}


void AbstractDBElement::collectLocalChildren(std::set<AbstractDBElement *> & localChildrenRefs) const {
	for (VICUS::AbstractDBElement * child: m_childrenRefs){
		if (child != nullptr && child->m_local){
			localChildrenRefs.insert(child);
			child->collectLocalChildren(localChildrenRefs);
		}
	}
}


const QString AbstractDBElement::sourceName() const {
	if (m_builtIn)
		return qApp->translate("AbstractDBElement", "BuiltIn DB");
	else if (m_local)
		return qApp->translate("AbstractDBElement", "Local");
	else
		return qApp->translate("AbstractDBElement", "User DB");
}

} // namespace VICUS

