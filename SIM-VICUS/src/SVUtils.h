#ifndef SVUtilsH
#define SVUtilsH

#include <map>
#include "SVConstants.h"

template <typename T>
T * newDatabaseElement(typename std::map<unsigned int, T> & db) {
	// search for highest ID (in user ID space)
	unsigned int highest = USER_ID_SPACE;
	for (const typename std::pair<unsigned int, T> & d : db) {
		if (d.first >= highest)
			highest = d.first+1;
	}
	// now create a database element, set the highest ID and add it to the database
	T e;
	e.m_id = highest;
	db[highest] = e;
	return &db[highest];
}

template <typename T>
T * addDatabaseElement(typename std::map<unsigned int, T> & db,  const T & e) {
	// search for highest ID (in user ID space)
	unsigned int highest = USER_ID_SPACE;
	for (const typename std::pair<unsigned int, T> & d : db) {
		if (d.first >= highest)
			highest = d.first+1;
	}
	// now set the highest ID and add it to the database
	T e2(e);
	e2.m_id = highest;
	db[highest] = e2;
	return &db[highest];
}


template <typename T>
const T * databaseElement(typename std::map<unsigned int, T> & db, unsigned int id) {
	typename std::map<unsigned int, T>::const_iterator it = db.find(id);
	if (it == db.end())				return nullptr;
	else							return &(it->second);
}


#endif // SVUtilsH
