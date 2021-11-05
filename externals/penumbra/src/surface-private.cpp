/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */


#include "surface-private.h"
#include "penumbra.h"

namespace Pumbra {


SurfacePrivate::SurfacePrivate() {}

SurfacePrivate::SurfacePrivate(const Polygon &polygon) : polygon(polygon) {}


TessData SurfacePrivate::tessellate() {

	  TESStesselator *tess = nullptr;
	  tess = tessNewTess(nullptr);

//	  if (!tess) {
//		showMessage(MSG_ERR, "Unable to create tessellator.");
//	  }

	  // Add primary polygon
	  tessAddContour(tess, TessData::polySize, &polygon[0], sizeof(float) * TessData::vertexSize,
					 (int)polygon.size() / TessData::vertexSize);

	  // Add holes
	  for (auto &hole : holes) {
		tessAddContour(tess, TessData::polySize, &hole[0], sizeof(float) * TessData::vertexSize,
					   (int)hole.size() / TessData::vertexSize);
	  }

	  if (!tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, TessData::polySize,
						 TessData::vertexSize, nullptr)) {
	//	showMessage(MSG_ERR, "Unable to tessellate surface.");
	  }

	  // For now convert to glDrawArrays() style of vertices, sometime may change to glDrawElements
	  // (with element buffers)
	  std::vector<float> vertexArray;
	  const TESSreal *verts = tessGetVertices(tess);
	  const int nelems = tessGetElementCount(tess);
	  const TESSindex *elems = tessGetElements(tess);
	  for (int i = 0; i < nelems * TessData::polySize; ++i) {
		const int vert = *(elems + i);
		for (int j = 0; j < TessData::vertexSize; ++j) {
		  vertexArray.push_back(verts[vert * TessData::vertexSize + j]);
		}
	  }

	  TessData data(&vertexArray[0], vertexArray.size());

	  tessDeleteTess(tess);

	  return data;
	}

} // end namespace
