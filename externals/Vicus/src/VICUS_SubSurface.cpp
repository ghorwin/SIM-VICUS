#include "VICUS_SubSurface.h"

namespace VICUS {

SubSurface SubSurface::clone() const {
	SubSurface r(*this); // create new SubSurface with same unique ID
	Object & o = r;
	(Object&)r = o.clone(); // assign new ID only
	return r;
}

} // namespace VICUS
