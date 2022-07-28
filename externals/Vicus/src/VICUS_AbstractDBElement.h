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

#ifndef VICUS_AbstractDBElementH
#define VICUS_AbstractDBElementH

#include <IBK_MultiLanguageString.h>

#include <QColor>

#include "VICUS_Constants.h"

class TiXmlElement;

namespace VICUS {

/*! Abstract base class of all database elements, needed to have generic DB table model implementations.
	ID and display name are mandatory elements for each db element.
*/
class AbstractDBElement {
public:
	enum ComparisonResult {
		/*! All parameters are identical (Caution: rounding errors!), except maybe the ID. */
		Equal,
		/*! All critical parameters are identical, yet some metadata (name etc.) may be different. */
		OnlyMetaDataDiffers,
		/*! Objects are completely different (at least one critical parameter differs). */
		Different
	};

	/*! Default constructor. */
	AbstractDBElement() = default;

	/*! Initializing constructor. */
	AbstractDBElement(unsigned int id, const IBK::MultiLanguageString & name) :
		m_id(id),
		m_displayName(name)
	{}

	/*! Virtual destructor, definition is in VICUS_Material.cpp. */
	virtual ~AbstractDBElement();

	virtual void readXML(const TiXmlElement * element) = 0;
	virtual TiXmlElement * writeXML(TiXmlElement * parent) const = 0;

	/*! Comparison of database element by content, without ID. */
	virtual ComparisonResult equal(const AbstractDBElement * other) const = 0;

	/*! Collects all pointers to child elements */
	void collectChildren(std::set<const AbstractDBElement *> & allChildrenRefs) const;

	/*! Collects all pointers to child elements which are local */
	void collectLocalChildren(std::set<AbstractDBElement *> & localChildrenRefs) const;

	/*! Source name for display purposes. */
	const QString sourceName() const;

	// *** Properties to be read/write in XML via code generator ***

	/*! Unique ID of this DB element. */
	unsigned int					m_id = INVALID_ID;

	/*! The description text for this DB element. */
	IBK::MultiLanguageString		m_displayName;

	/*! False-color for this DB element (optional, may not be necessary for all DB elements). */
	QColor							m_color;


	// *** Run time variables ***

	/*! If true, this is a built-in DB element and cannot be modified/removed. */
	bool							m_builtIn = false;

	/*! Determines wether this element is kept local in the current project or should be stored into the userDB (false). */
	bool							m_local = true;

	/*! Pointer to elements which are referenced by this element */
	std::set<AbstractDBElement*>	m_childrenRefs;

	/*! If true, this element is referenced in the current project */
	mutable bool					m_isReferenced = false;

	/*! Holds error string in order to give users a tooltip in db dialog. */
	mutable std::string				m_errorMsg;

};

} // namespace VICUS

#endif // VICUS_AbstractDBElementH
