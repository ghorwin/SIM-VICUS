#ifndef VICUS_ObjectH
#define VICUS_ObjectH


namespace VICUS {

/*! Base class for all objects that have a parent. */
class Object {
public:
	Object	*m_parent = nullptr;
};

} // namespace VICUS

#endif // VICUS_ObjectH
