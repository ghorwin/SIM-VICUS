/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <vector>
#include <array>

// Penumbra
#include "surface.h"
#include "surface-private.h"
#include "error.h"

namespace Pumbra {

/*
void* stdAlloc(void* userData, unsigned size)
{
		int* allocated = ( int*)userData;
		TESS_NOTUSED(userData);
		*allocated += (int)size;
		return malloc(size);
}

void stdFree(void* userData, void* ptr)
{
		TESS_NOTUSED(userData);
		free(ptr);
}
*/

TessData::TessData(const float *array, unsigned numVerts) : numVerts(numVerts) {
  vertices.insert(vertices.end(), (const float *)array, (const float *)array + numVerts);
}

Surface::Surface() { surface = std::make_shared<SurfacePrivate>(); }

Surface::Surface(const Polygon &polygon) { surface = std::make_shared<SurfacePrivate>(polygon); }

Surface::Surface(const Surface &srf) { surface = srf.surface; }

Surface::~Surface() {}

int Surface::setOuterPolygon(const Polygon &polygon) {
  surface->polygon = polygon;
  return 0;
}

int Surface::addHole(const Polygon &hole) {
  surface->holes.push_back(hole);
  return 0;
}

} // namespace Pumbra
