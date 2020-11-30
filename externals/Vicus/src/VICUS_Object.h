#ifndef VICUS_ObjectH
#define VICUS_ObjectH


namespace VICUS {

/*! Base class for all objects that have a parent and are shown in the
	navigation tree widget.

	Also, each object has a unique ID that is used throughout the user interface to
	identify it.

	Basically, whenever such an object is newly created, it will get a new unique ID.
*/
class Object {
public:
	Object() : m_uniqueID(++LAST_ID) {}

	Object	*m_parent = nullptr;

	unsigned int uniqueID() const { return m_uniqueID; }

private:
	unsigned int m_uniqueID;

	/*! Unique ID marker, incremented in each constructor call. */
	static unsigned int LAST_ID;
};

} // namespace VICUS

#endif // VICUS_ObjectH
