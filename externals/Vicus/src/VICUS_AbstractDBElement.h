#ifndef VICUS_ABSTRACTDBELEMENT_H
#define VICUS_ABSTRACTDBELEMENT_H

namespace VICUS {

class AbstractDBElement {
public:
	/* If true, this is a built-in DB element and cannot be modified/removed. */
	bool m_builtIn = false;
};

} // namespace VICUS

#endif // VICUS_ABSTRACTDBELEMENT_H
