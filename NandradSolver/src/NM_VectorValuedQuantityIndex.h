/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef NM_VectorValuedQuantityIndexH
#define NM_VectorValuedQuantityIndexH

#include <string>

namespace NANDRAD_MODEL {

/*! Struct defining a vector index via model id or vector index.
	The primary purpose of this utility class is encoding and decoding of typed indices to/from std::string.
	It is also used to identify a specific element in a VectorValuedQuantity data structure.

	\note Note, that for identification purposes actually only the index/ID is needed, the additional m_keyType
		can be used for compatibility checking, so that we prevent giving a modelID where an index is expected
		(and vice versa).
*/
struct VectorValuedQuantityIndex {

	/*! Allowed types for specifying vector indices. */
	enum IndexKeyType {
		/*! Vector element is referenced per index */
		IK_Index,
		/*! Vector element is referenced per id number of a referenceing model. */
		IK_ModelID,
		NUM_IndexKeyType
	};

	/*! Default constructor, creates invalid index key. */
	VectorValuedQuantityIndex() :
		m_keyType(NUM_IndexKeyType), m_keyValue(0)
	{
	}

	/*! Initializing constructor. */
	VectorValuedQuantityIndex(const IndexKeyType keyType, const unsigned int keyValue) :
		m_keyType(keyType), m_keyValue(keyValue)
	{
	}

	/*! Decodes a string with encoded index into type and value.
		\code
			// Index-Format
			std::string keyString = "index=5";
			// Simplified Index-Format
			keyString = "5";
			// ID-Format
			keyString = "id=5";
		\endcode
		Throws an IBK::Exception if expression is malformed.
	*/
	void set(const std::string &keyString);

	/*! Sets keytype and value at the same time.
		This is a convenience function as alternative to setting m_keyType and m_keyValue directly.
	*/
	void set(const IndexKeyType keyType, const unsigned int keyValue);

	/*! Returns encoded string version of keytype and value.
		\sa set(const std::string &keyString)
	*/
	std::string	toString() const;

	/*! The type of index specification (either by index or model ID). */
	IndexKeyType			m_keyType;
	/*! The corresponding index number/model ID number. */
	unsigned int			m_keyValue;
};


inline bool operator == (const VectorValuedQuantityIndex& lidx, const VectorValuedQuantityIndex& ridx) {
	return lidx.m_keyType == ridx.m_keyType && lidx.m_keyValue == ridx.m_keyValue;
}

inline bool operator < (const VectorValuedQuantityIndex& lidx, const VectorValuedQuantityIndex& ridx) {
	return lidx.m_keyValue < ridx.m_keyValue;
}



} // namespace NANDRAD_MODEL

#endif // NM_VectorValuedQuantityIndexH
