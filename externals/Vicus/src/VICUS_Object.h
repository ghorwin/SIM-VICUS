#ifndef VICUS_ObjectH
#define VICUS_ObjectH

#include <vector>
#include <set>

namespace VICUS {

/*! Base class for all objects that have a parent and are shown in the
	navigation tree widget.

	Also, each object has a unique ID that is used throughout the user interface to
	identify it.

	Basically, whenever such an object is newly created, it will get a new unique ID.

	Note: whenever you add/remove an object in the item hierarchy, make sure to call updateParents() in top level node.
*/
class Object {
public:
	/*! Standard C'tor. */
	Object() : m_uniqueID(++LAST_ID) {}
	/*! Default copy constructor. */
	Object(Object const&) = default;
	/*! D'tor. */
	virtual ~Object();

	unsigned int uniqueID() const { return m_uniqueID; }

	/*! Recursively searches through data hierarchy and returns pointer to object matching the given unique ID.
		\return Returns pointer to wanted object or nullptr, if it couldn't be found.
	*/
	Object * findChild(unsigned int uID);
	/*! Same as function above, const version. */
	const Object * findChild(unsigned int uID) const;

	/*! Recursively selected all unique IDs of children. */
	void collectChildIDs(std::set<unsigned int> & nodeContainer) const;

	/*! Takes an old object and returns a new object with a new unique ID, but same m_parent and m_children. */
	Object clone() const {
		Object o(*this);
		o.m_uniqueID = ++LAST_ID;
		return o;
	}



	Object	*m_parent = nullptr;

	/*! Stores selected information for this surface (not serialized, for now). */
	bool								m_selected = false;
	/*! Stores visibility information for this surface (serialized manually in derived classes). */
	bool								m_visible = true;

protected:
	/*! List of all children. */
	std::vector<Object *>	m_children;

private:
	unsigned int			m_uniqueID;

	/*! Unique ID marker, incremented in each constructor call. */
	static unsigned int LAST_ID;
};

} // namespace VICUS

#endif // VICUS_ObjectH
