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

#ifndef VICUS_DatabaseH
#define VICUS_DatabaseH

#include <map>

#include <VICUS_AbstractDBElement.h>

#include <IBK_Path.h>
#include <IBK_messages.h>

#include <tinyxml.h>

namespace VICUS {

/*! Wrapper class for different databases.
	Template argument classes must provide a public bool member m_builtIn, and the functions
	readXML(const TiXmlElement * element) and writeXML(TiXmlElement * parent) const.

	Usage:
	\code
		VICUS::Database<Material> materials;

		VICUS::Material m;
		m.   .... set parameters

		unsigned ínt newId = materials.add(m);

		const VICUS::Material * materials[newId]; // -> either VICUS::Material pointer or nullptr, if object doesn't exist
	\endcode
*/
template<typename T>
class Database {
public:
	Database(unsigned int userIDSpaceStart) :
		USER_ID_SPACE_START(userIDSpaceStart), m_userIdCounter(userIDSpaceStart)
	{
	}

	/*! Returns const database element by ID, or nullptr if no element exists with this ID. */
	const T * operator[](unsigned int id) const {
		typename std::map<unsigned int, T>::const_iterator it = m_data.find(id);
		if (it == m_data.end())		return nullptr;
		else						return &(it->second);
	}

	/*! Returns database element by ID, or nullptr if no element exists with this ID. */
	T * operator[](unsigned int id) {
		typename std::map<unsigned int, T>::iterator it = m_data.find(id);
		if (it == m_data.end())		return nullptr;
		else						return &(it->second);
	}

	/*! Tries to find an element in the DB that matches content-wise the provided element
		(i.e. ignores ID during comparison, calls T::equal()).
		\return Returns nullptr if no such element exists, otherwise a pointer to the respective existing element.
	*/
	const T * findEqual(const T & elem) const {
		for (typename std::map<unsigned int, T>::const_iterator it = m_data.begin(); it != m_data.end(); ++it) {
			const T * elemPtr = &(it->second);
			// Note: we compare relatively strict here, and so two DB definitions are considered
			//       "different", even if only meta data like the displayname differs.
			//       The user must cleanup/merge such DB elements afterwards with the "remove duplicates" feature.
			if (elem.equal(elemPtr) == VICUS::AbstractDBElement::Equal)
				return elemPtr;
		}
		return nullptr;
	}

	/*! Returns begin for iterator-type read-only access to data store. */
	typename std::map<unsigned int, T>::const_iterator begin() const { return m_data.begin(); }
	/*! Returns end for iterator-type read-only access to data store. */
	typename std::map<unsigned int, T>::const_iterator end() const { return m_data.end(); }
	/*! Returns begin for iterator-type access to data store. */
	typename std::map<unsigned int, T>::iterator begin() { return m_data.begin(); }
	/*! Returns end for iterator-type access to data store. */
	typename std::map<unsigned int, T>::iterator end() { return m_data.end(); }
	/*! Returns number of DB elements. */
	size_t size() const { return m_data.size(); }
	/*! Returns true if database is empty. */
	bool empty() const { return m_data.empty(); }

	/*! Initializes database with data from a vector.
		Throws an exception if duplicate IDs are found.
	*/
	void setData(const std::vector<T> & data) {
		m_data.clear();
		for (const T & t : data)
			m_data[t.m_id] = t;
	}

	/*! Imports elements from other DB that do not yet exist in current DB. The imported DB is expected to
		holds only IDs that are not yet present in the current DB.
		For each element with duplicate ID a warning message is issued in IBK_message().
	*/
	void import(const Database<T> & other) {
		FUNCID(Database::import);
		for (typename std::map<unsigned int, T>::const_iterator it = other.begin(); it != other.end(); ++it) {
			typename std::map<unsigned int, T>::const_iterator this_it = m_data.find(it->first);
			if (this_it != m_data.end()) {
				IBK::IBK_Message(IBK::FormatString("DB element with duplicate ID #%1 was not imported.").arg(it->first), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
				continue;
			}
			m_data[it->first] = it->second;
		}
	}

	/*! Adds a new item to the database.
		\param newData New object to be added.
		\param suggestedId ID of object to insert (possibly when adding this data object from a project file), or 0,
			if object was newly created and needs a new ID anyway.

		\note New items are never built-in and will be local DB elements, automatically.
	*/
	unsigned int add(T & newData, unsigned int suggestedId = 0) {
		// check if suggestedId is already used
		bool used = false;
		if (suggestedId != 0) {
			typename std::map<unsigned int, T>::const_iterator it = m_data.find(suggestedId);
			if (it != m_data.end())
				used = true;
		}
		// if used, or suggestedId == 0 (new object), find first unused user-space ID
		if (suggestedId == 0 || used) {
			// object gets new unique Id, search for next free userId
			unsigned int newId = ++m_userIdCounter;
			while (m_data.find(newId) != m_data.end())
				newId = ++m_userIdCounter;
			newData.m_id = newId;
		}
		else {
			// object gets suggested Id
			if (newData.m_id != suggestedId) {
				newData.m_id = suggestedId; // this should be the case already, but just to be certain
			}
		}
		// set built-in flag to identify material as built-in or user-defined base on UI space
		newData.m_builtIn = false;
		newData.m_local = true;
		newData.m_isReferenced = false;
		m_data[newData.m_id] = newData;
		// for now database is always modified when data is set, callers have to ensure that
		// they don't re-set data with original data and id
		m_modified = true;
		return newData.m_id;
	}

	/*! Removes a database element identified by its ID
		\warning Throws an IBK::Exception if no element with given ID exists.
	*/
	void remove(unsigned int id) {
		FUNCID(Database::remove);
		typename std::map<unsigned int, T>::const_iterator it = m_data.find(id);
		if (it == m_data.end())
			throw IBK::Exception( IBK::FormatString("Error removing database element with id=%1. No such ID in database.").arg(id), FUNC_ID);
		m_data.erase(it);
		m_modified = true;
	}

	/*! Removes all user-defined elements (element not marked as built-in) */
	void removeUserElements() {
		// iterate over all elements - mind: no increment of the iterator needed here!
		for (typename std::map<unsigned int, T>::const_iterator it = m_data.begin(); it != m_data.end(); /* no increment here */) {
			if (it->second.m_builtIn || it->second.m_local)
				++it;
			else
				it = m_data.erase(it); // remove it, and set it to next following element iterator
		}
	}

	/*! Removes all user elements which are defined as local */
	void removeLocalElements() {
		// iterate over all elements - mind: no increment of the iterator needed here!
		for (typename std::map<unsigned int, T>::const_iterator it = m_data.begin(); it != m_data.end(); /* no increment here */) {
			if (it->second.m_local && !it->second.m_builtIn)
				it = m_data.erase(it); // remove it, and set it to next following element iterator
			else
				++it;
		}
	}

	/*! Removes all local, not-referenced elements */
	void removeNotReferencedLocalElements() {
		// iterate over all elements - mind: no increment of the iterator needed here!
		for (typename std::map<unsigned int, T>::const_iterator it = m_data.begin(); it != m_data.end(); /* no increment here */) {
			if (!it->second.m_isReferenced && it->second.m_local && !it->second.m_builtIn)
				it = m_data.erase(it); // remove it, and set it to next following element iterator
			else
				++it;
		}
	}

	/*! Local utility function which clears parents and children references using const cast */
	void clearChildren() {
		for (auto it=m_data.begin(); it!=m_data.end(); ++it)
			it->second.m_childrenRefs.clear();
	}

	/*! Reads database from xml file.
		Usage:
		\code
		// read built-in material DB
		db.readXML("db_materials.xml", "Materials", "Material", true);
		\endcode

		\warning Throws an IBK::Exception if reading fails.
	*/
	void readXML(const IBK::Path & fname, const std::string & topLevelTag,
				 const std::string & childTagName,
				 bool builtIn = false)
	{
		FUNCID(Datbase::readXML);

		TiXmlDocument doc;
		if (!fname.isFile() )
			return;

		try {
			if (!doc.LoadFile(fname.str().c_str(), TIXML_ENCODING_UTF8)) {
				throw IBK::Exception(IBK::FormatString("Error in line %1 of XML file '%2':\n%3")
									 .arg(doc.ErrorRow())
									 .arg(fname)
									 .arg(doc.ErrorDesc()), FUNC_ID);
			}

			// we use a handle so that NULL pointer checks are done during the query functions
			TiXmlHandle xmlHandleDoc(&doc);

			// read root element
			TiXmlElement * xmlElem = xmlHandleDoc.FirstChildElement().Element();
			if (!xmlElem)
				return; // empty file?
			std::string rootnode = xmlElem->Value();
			if (rootnode != topLevelTag)
				throw IBK::Exception( IBK::FormatString("Expected '%1' as root node in XML file.")
									  .arg(topLevelTag), FUNC_ID);

			const TiXmlElement * c2 = xmlElem->FirstChildElement();
			while (c2) {
				const std::string & c2Name = c2->ValueStr();
				if (c2Name != childTagName)
					IBK::IBK_Message(IBK::FormatString("Unknown/unsupported tag '%1' in line %2, expected '%3'.")
									 .arg(c2Name).arg(c2->Row()).arg(childTagName), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
				T obj;
				obj.readXML(c2);
				obj.m_builtIn = builtIn;
				obj.m_local = false;  // objects we read from the DB are not local by definition

				c2 = c2->NextSiblingElement();

				// Check for existing DB element with same ID, if both are built-in or both are user DB (not build-in), this raises an exception
				// If there is no element with this id yet OR if this is a built-in element: we store the element
				typename std::map<unsigned int, T>::iterator it = m_data.find(obj.m_id);
				if (it == m_data.end() || obj.m_builtIn )
					m_data[obj.m_id] = obj;
				else if (it->second.m_builtIn && obj.m_builtIn)
					throw IBK::Exception(IBK::FormatString("Build-In database '%1' contains duplicate ids %2 ")
														 .arg(fname.str()).arg(obj.m_id), FUNC_ID);
				else if (!it->second.m_builtIn && !obj.m_builtIn)
					throw IBK::Exception(IBK::FormatString("User database '%1' contains duplicate ids %2 ")
														 .arg(fname.str()).arg(obj.m_id), FUNC_ID);

			}
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			IBK::IBK_Message(IBK::FormatString("Error reading XML database '%1'.").arg(fname), IBK::MSG_ERROR, FUNC_ID);
		}
	}


	/*! Writes database to XML file.
		Usage:
		\code
		// write user material DB (writes only those materials marked not as built-in and not as local)
		db.writeXML("db_materials.xml", "Materials");
		\endcode
	*/
	void writeXML(const IBK::Path & fname, const std::string & topLevelTag) const {
		TiXmlDocument doc;
		TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
		doc.LinkEndChild( decl );

		TiXmlElement * root = new TiXmlElement( topLevelTag );
		doc.LinkEndChild(root);

		for (auto e : m_data)
			if (!e.second.m_builtIn && !e.second.m_local)
				e.second.writeXML(root);

		doc.SaveFile( fname.c_str() );
	}

	/*! Modified marker, should be changed to true, whenever a database object was modified.
		Will be automatically set to true in functions add() and remove().
	*/
	bool										m_modified = false;

private:
	/*! Holds actual data, key = (unique) ID of DB element, value = data object. */
	std::map<unsigned int, T>					m_data;

	/*! Counter that holds the first user material ID. */
	unsigned int								USER_ID_SPACE_START;
	/*! Counter that holds the next unused user material ID. */
	unsigned int								m_userIdCounter;
};


} // namespace VICUS


#endif // VICUS_DatabaseH
