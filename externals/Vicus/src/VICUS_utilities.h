#ifndef VICUS_UTILITIES_H
#define VICUS_UTILITIES_H

#include <algorithm>
#include "VICUS_Surface.h"
#include "VICUS_SubSurface.h"

namespace VICUS {


/*! Function to find an element by ID. */
template <typename T>
T * element(std::vector<T>& vec, unsigned int id) {
	typename std::vector<T>::iterator it = std::find(vec.begin(), vec.end(), id);
	if (it == vec.end())
		return nullptr;
	else
		return &(*it);
}

/*! Checks if an object with m_id matching the searched id exists in the vector. */
template <typename T>
bool contains(const std::vector<T> & vec, unsigned int id) {
	for (auto & t : vec)
		if (t->m_id == id)
			return true;
	return false;
}


/*! Function to find an element by ID (const-version). */
template <typename T>
const T * element(const std::vector<T>& vec, unsigned int id) {
	typename std::vector<T>::const_iterator it = std::find(vec.begin(), vec.end(), id);
	if (it == vec.end())
		return nullptr;
	else
		return &(*it);
}


/*! Utility function to find a vector element by name. */
template <typename T>
const T * element(const std::vector<T> & vec, const std::string & name) {
	unsigned int i=0;
	for (;i<vec.size(); ++i)
		if (vec[i].m_name == name)
			return &vec[i];
	return nullptr;
}


/*! Utility function to find a vector element by name. */
template <typename T>
unsigned int elementIndex(const std::vector<T> & vec, unsigned int id) {
	unsigned int i=0;
	for (;i<vec.size(); ++i)
		if (vec[i].m_id == id)
			break;
	return i;
}


/*! Utility function to find a vector element's index by name. */
template <typename T>
unsigned int elementIndex(const std::vector<T> & vec, const std::string & name, bool caseSensitiv = true) {
	unsigned int i=0;
	for (;i<vec.size(); ++i){

		if(caseSensitiv){
			if (vec[i].m_name == name)
				break;
		}
		else
			if (IBK::string_nocase_compare(vec[i].m_name, name))
				break;

	}
	return i;
}


/*! Function to generate unique (lowest not yet used) ID. */
template <typename T>
unsigned int uniqueId(const std::vector<T>& vec) {
	std::set<unsigned int> idSet;
	for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it)
		idSet.insert(it->m_id);
	unsigned int id = 1;
	for (std::set<unsigned int>::const_iterator it = idSet.begin(); it != idSet.end(); ++it) {
		if (*it - id > 0) // Mind, that we want to find all spaces inside the id set
			return id;
		++id;
	}
	return id;
}


/*! Function to generate unique (lowest not yet used) ID. */
template <>
inline unsigned int uniqueId(const std::vector<unsigned int>& vec) {
	std::set<unsigned int> idSet(vec.begin(), vec.end());
	unsigned int id = 1;
	for (std::set<unsigned int>::const_iterator it = idSet.begin(); it != idSet.end(); ++it) {
		if (*it - id > 1)
			return id+1;
		++id;
	}
	return ++id;
}


/*! Function to generate unique (lowest not yet used) ID. */
template <typename T>
unsigned int uniqueId(const std::set<T>& vec) {
	for (unsigned id=1; id<std::numeric_limits<unsigned>::max(); ++id) {
		if (std::find(vec.begin(), vec.end(), id) == vec.end())
			return id;
	}
	return 999999; // just to make compiler happy, we will find an unused ID in the loop above
}


/*! Function to generate unique (lowest not yet used) ID. */
template <>
inline unsigned int uniqueId(const std::set<unsigned int>& vec) {
	unsigned int id = 1;
	for (std::set<unsigned int>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
		if (*it - id > 1)
			return id+1;
		++id;
	}
	return ++id;
}


/*! Function to generate unique ID and add this ID to the vector. */
template <typename T>
unsigned int uniqueIdAdd(std::vector<T>& vec) {
	unsigned int id = uniqueId(vec);
	vec.push_back(id);
	return id;
}

/*! Function to generate unique ID. First check predefined id. And the Id to the container. */
template <typename T>
unsigned int uniqueIdWithPredef(std::vector<T>& vec, unsigned int newId) {
	if(std::find(vec.begin(), vec.end(), newId) == vec.end()){
		vec.push_back(newId);
		return newId;
	}
	unsigned id=uniqueId(vec);
	vec.push_back(id);
	return id;
}

/*! Function to generate unique ID. First check predefined id. Add the Id to the container.  */
template <typename T>
unsigned int uniqueIdWithPredef(std::vector<T>& vec, unsigned int id, std::map<T,T> mapOldToNewId) {
	if (mapOldToNewId.find(id) == mapOldToNewId.end())
		mapOldToNewId[id] = uniqueIdWithPredef(vec, id);
	return mapOldToNewId[id];
}

/*! Function to generate a unique ID that is larger than all the other IDs used.
	This is useful if a series of objects with newly generated IDs shall be added to a container.
*/
template <typename T>
unsigned int largestUniqueId(const std::vector<T>& vec) {
	unsigned int largest = 0;
	for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it)
		largest = std::max(largest, it->m_id);
	return largest+1; // Mind: plus one, to get past the largest _existing_ ID
}

/*! Generates a new unique name in format "basename" or "basename (<nr>)" with increasing numbers until
	the name no longer exists in set existingNames.
	NOTE: we cannot use [<nr>] because when generating output variable names, this interferes with the
		unit specification.

	NOTE: basename is always trimmed.

	\param baseName Contains the original name, which may include already "(<nr>)"
*/
QString uniqueName(const QString & baseName, const std::set<QString> & existingNames);


/*! Test function that checks that all objects in the given vector have unique m_id parameters.
	The set passed as second argument is used for comparison. Pass an empty set if only the elements
	in the vector itself shall be tested. You can re-use the populated set for another call
	to this function, if you want to ensure uniqueness of several object vectors.
*/
template <typename T>
void checkForUniqueIDs(const std::vector<T> & vec, std::set<unsigned int> & usedIDs) {
	FUNCID(VICUS::Utilities::checkForUniqueIDs);

	for (const T & t : vec) {
		if (usedIDs.find(t.m_id) != usedIDs.end())
			throw IBK::Exception(IBK::FormatString("Duplicate model/object ID #%1.")
								 .arg(t.m_id), FUNC_ID);
		usedIDs.insert(t.m_id);
	}
}

/*! Same as function above, but for containers holding const pointers instead of objects. */
template <typename T>
void checkForUniqueIDs(const std::vector<const T*> & vec, std::set<unsigned int> & usedIDs) {
	FUNCID(VICUS::Utilities::checkForUniqueIDs);

	for (const T * t : vec) {
		if (usedIDs.find(t->m_id) != usedIDs.end())
			throw IBK::Exception(IBK::FormatString("Duplicate model/object ID #%1.")
								 .arg(t->m_id), FUNC_ID);
		usedIDs.insert(t->m_id);
	}
}

/*! Takes a string in camel case notation as input and returns a QString whith a space before each capital letter. */
QString camelCase2ReadableString(const std::string &original);

} // namespace VICUS

#endif // VICUS_UTILITIES_H
