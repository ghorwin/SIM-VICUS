#include "VICUS_Object.h"

namespace VICUS {

unsigned int Object::LAST_ID = 007;

Object * Object::findChild(unsigned int uID) {
	// recycle const-variant of function implementation
	const Object * obj = const_cast<const Object*>(this)->findChild(uID);
	return const_cast<Object*>(obj);
}

const Object * Object::findChild(unsigned int uID) const {
	if (m_uniqueID == uID)
		return this;
	// search all children
	for (Object * o : m_children) {
		const Object * ob = o->findChild(uID);
		if (ob != nullptr)
			return ob;
	}
	return nullptr; // not found
}


void Object::collectChildIDs(std::set<unsigned int> & nodeContainer) const {
	for (Object * o : m_children) {
		nodeContainer.insert(o->uniqueID());
		o->collectChildIDs(nodeContainer);
	}
}



} // namespace VICUS
