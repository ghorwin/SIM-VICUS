#ifndef VIC3DGEOMETRYHELPERS_H
#define VIC3DGEOMETRYHELPERS_H

#include <VICUS_Surface.h>
#include <vector>
#include "Vic3DVertex.h"
#include "qgl.h"

namespace Vic3D {

/*! This function adds a surface object to a vertex, color and index buffer. */
void addSurface(const VICUS::Surface & s,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData);

} // namespace Vic3D


#endif // VIC3DGEOMETRYHELPERS_H
