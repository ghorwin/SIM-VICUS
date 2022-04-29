/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "Vic3DNewSubSurfaceObject.h"

#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include <VICUS_Project.h>
#include <SV_Conversions.h>

#include <IBKMK_3DCalculations.h>

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVViewStateHandler.h"
#include "SVPropVertexListWidget.h"
#include "Vic3DGeometryHelpers.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DShaderProgram.h"

namespace Vic3D {

NewSubSurfaceObject::NewSubSurfaceObject() :
	m_vertexBufferObject(QOpenGLBuffer::VertexBuffer), // VertexBuffer is the default, so default constructor would have been enough
	m_indexBufferObject(QOpenGLBuffer::IndexBuffer) // make this an Index Buffer
{
	// make us known to the world
	SVViewStateHandler::instance().m_newSubSurfaceObject = this;
}


void NewSubSurfaceObject::create(QOpenGLShaderProgram * shaderProgram) {
	if (m_vao.isCreated())
		return;

	// *** create buffers on GPU memory ***

	// create a new buffer for the vertices and colors, separate buffers because we will modify colors way more often than geometry
	m_vertexBufferObject.create();
	m_vertexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw); // usage pattern will be used when tranferring data to GPU

	m_colorBufferObject.create();
	m_colorBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// create a new buffer for the indexes
	m_indexBufferObject = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer); // Note: make sure this is an index buffer
	m_indexBufferObject.create();
	m_indexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw);


	// *** create and bind Vertex Array Object ***

	// Note: VAO must be bound *before* the element buffer is bound,
	//       because the VAO remembers associated element buffers.
	m_vao.create();
	m_vao.bind(); // now the VAO is active and remembers states modified in following calls

	m_indexBufferObject.bind(); // this registers this index buffer in the currently bound VAO


	// *** set attribute arrays for shader fetch stage ***

#define VERTEX_ARRAY_INDEX 0
#define NORMAL_ARRAY_INDEX 1
#define COLOR_ARRAY_INDEX 2

	m_vertexBufferObject.bind(); // this registers this buffer data object in the currently bound vao; in subsequent
				  // calls to shaderProgramm->setAttributeBuffer() the buffer object is associated with the
				  // respective attribute array that's fed into the shader. When the vao is later bound before
				  // rendering, this association is remembered so that the vertex fetch stage pulls data from
				  // this vbo

	// coordinates
	shaderProgram->enableAttributeArray(VERTEX_ARRAY_INDEX);
	shaderProgram->setAttributeBuffer(VERTEX_ARRAY_INDEX, GL_FLOAT, 0, 3 /* vec3 */, sizeof(Vertex));

	// normals
	shaderProgram->enableAttributeArray(NORMAL_ARRAY_INDEX);
	shaderProgram->setAttributeBuffer(NORMAL_ARRAY_INDEX, GL_FLOAT, offsetof(Vertex, m_normal), 3 /* vec3 */, sizeof(Vertex));


	m_colorBufferObject.bind(); // now color buffer is active in vao

	// colors
	shaderProgram->enableAttributeArray(COLOR_ARRAY_INDEX);
	shaderProgram->setAttributeBuffer(COLOR_ARRAY_INDEX, GL_UNSIGNED_BYTE, 0, 4, 4 /* bytes = sizeof(char) */);

	// Release (unbind) all

	// Mind: you can release the buffer data objects (vbo and vboColors) before or after releasing vao. It does not
	//       matter, because the buffers are associated already with the attribute arrays.
	//       However, YOU MUST NOT release the index buffer (ebo) before releasing the vao, since this would remove
	//       the index buffer association with the vao and when binding the vao before rendering, the element buffer
	//       would not be known and a call to glDrawElements() crashes!
	m_vao.release();

	m_vertexBufferObject.release();
	m_colorBufferObject.release();
	m_indexBufferObject.release();
}


void NewSubSurfaceObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_indexBufferObject.destroy();
}


bool NewSubSurfaceObject::generateSubSurfaces(const std::vector<const VICUS::Surface*> & sel,
											  const WindowComputationData & inputData, QString & errorMsg)
{
	FUNCID(NewSubSurfaceObject::generateSubSurfaces);
	// populate PlaneGeometry-objects

	qDebug() << "Generating windows for" << sel.size() << " surfaces.";
	m_generatedSurfaces.clear();

	// input data consistency checks
	double wallWindowRatio	= inputData.m_percentage/100;
	double heightWinPre		= inputData.m_height;
	double widthWinPre		= inputData.m_width;
	double heightSill		= inputData.m_windowSillHeight;

	// tests only for "by percentage"
	if (inputData.m_byPercentage) {
		if (wallWindowRatio > 1 || wallWindowRatio <=0 ) {
			errorMsg = tr("Percentage value is out of range!");
			return false;
		}
	}

	// tests for all generation variants
	if (heightWinPre < 0 || widthWinPre < 0 || heightSill < 0) {
		errorMsg = tr("Only positive values for parameters allowed!");
		return false;
	}
	if (heightWinPre <= 0.01 || widthWinPre <= 0.01) {
		errorMsg = tr("Height and width must be both > 0.01 m!");
		return false;
	}

	// process all selected surfaces
	for (const VICUS::Surface* s : sel) {
		const VICUS::PlaneGeometry &surfacePoly = s->geometry();

		// sanity check
		if (!surfacePoly.isValid()) {
			IBK::IBK_Message(IBK::FormatString("Invalid geometry of surface #%1").arg(s->m_id), IBK::MSG_WARNING, FUNC_ID);
			continue;
		}

		// get normal
		IBKMK::Vector3D n = surfacePoly.normal();
		double lowestZ = std::numeric_limits<unsigned int>::max();
		// if normal vector points exactly downwards?
		if (n.m_z == 1. || n.m_z == -1.) {
			//now we have a flat roof or floor
			//so we need a ground line
			//TODO Dirk
		}
		else{
			//we take the ground line from the lowest z-value
			//We assume that there is a baseline with the point that has the smallest z-value
		}


		// we assume that the local coordinate system is ok and we take the x-axis as baseline
		const std::vector<IBKMK::Vector2D> & vertexes2D = surfacePoly.polygon2D().vertexes();

		double xMin = std::numeric_limits<double>::max();
		double yMin = xMin;
		double xMax = -std::numeric_limits<double>::min();
		double yMax = xMax;

		for (unsigned int i=0; i<vertexes2D.size(); ++i){
			const IBKMK::Vector2D & pt = vertexes2D[i];
			//find highest/lowest point (x- and y-values)
			if (pt.m_x < xMin)	xMin = pt.m_x;
			if (pt.m_y < yMin)	yMin = pt.m_y;
			if (pt.m_x > xMax)	xMax = pt.m_x;
			if (pt.m_y > yMax)	yMax = pt.m_y;
		}
		// now we have the bounding box
		double heightSurface = yMax - yMin;
		double widthSurface = xMax - xMin;
		double minDistance = 0.01; // this is the minimum distance - 1 cm hardcoded for now

		if (heightSurface <= minDistance * 2 || widthSurface <= minDistance * 2) {
			// area too small kick out this surface
			IBK::IBK_Message(IBK::FormatString("Surface dimensions too small (%1 x %2), skipped.").arg(widthSurface).arg(heightSurface), IBK::MSG_WARNING, FUNC_ID);
			continue;
		}

		// Compute maximum window dimensions, currently assuming orthogonal
		// surface boundary lines; i.e. if you have slanted rectangle, the
		// maximum dimensions will be too large. But that is tested later.
		double hMax = heightSurface - 2 * minDistance;
		double wMax = widthSurface - 2 * minDistance;

		// vector for each window to be created as polygon within the surface polygon
		std::vector<VICUS::Polygon2D> windows;


		// *** Generate by percentage ***

		if (inputData.m_byPercentage) {

			if (hMax<heightWinPre)	heightWinPre = hMax;
			if (wMax<widthWinPre)	widthWinPre = wMax;
			if (heightSill < minDistance)
				heightSill = minDistance;

			double height = heightWinPre;
			double width = widthWinPre;
			double sillHeight = heightSill;

			enum Priority{
				Height,
				Width,
				SillHeight,
				Distance
			};

			Priority prio = Height;

			if (inputData.m_priorities[0] == 1)	prio = Width;
			if (inputData.m_priorities[1] == 1)	prio = Height;
			if (inputData.m_priorities[2] == 1)	prio = SillHeight;
			if (inputData.m_priorities[3] == 1)	prio = Distance;

			// das sind die neuen programmierungen für die Fenstereinbindung
#ifdef USE_DEBUG_VALUES
			double widthSurface = 5;	// in m
			double heightSurface = 3;	// in m
			heightSill = 0.8;	// in m
			heightWinPre = 2.5;	// in m
			widthWinPre = 1;		// in m
			wallWindowRatio = 0.8;			// window wall ratio
			double areaSurface = 1212;	// in m2
#else
			// Werte übernehmen aus den Vorgaben von oben (Werte wurden bereits ganz am Anfang auf Gültigkeit geprüft)
			double areaSurface		= surfacePoly.area();
#endif

			double areaWindow = areaSurface * wallWindowRatio;	// in m2

			// some constant variables
			// Note: we checked for heightSurface > 2*minDistance etc. before
			double heightWinMax1 = heightSurface - 2 * minDistance;
			double widthWinMax = widthSurface - 2 * minDistance;

			// compute the number of windows based on given window width
			// TODO : Dirk - why split only horizontally? For very tall walls with prio "height = 1", this gives stupid results...

			// we split the wall into | gap | window | gap | window | gap |, where
			// 'gap = minDistance' and 'window = widthWinPre'
			unsigned int count = std::max<unsigned int>(1, (unsigned int)std::floor((widthSurface - minDistance) / (widthWinPre + minDistance)));
			// since we have rounded down, we recompute the window's width from equation:
			//
			//   totalWidth = (count+1)*gap + count*window
			//
			widthWinMax = (widthSurface - (count + 1)*minDistance) / count;

			// result variables
			double heightWin, widthWin;

			// only for information
			double wwrError;

			// checks
			/// TODO : Dirk, are these really necessary -> see checks hMax and wMax above!
			///        introduce meaningful variables (using a sketch) and define variables only once
			if (heightWinPre > heightWinMax1 )	heightWinPre = heightWinMax1;
			if (widthWinPre > widthWinMax )		widthWinPre = widthWinMax;

			// we keep precomputed count, but adjust geometry based on prio
			switch (prio){
				case Height: {
					heightWin = std::min(heightWinPre, heightWinMax1);
					double areaSurfaceMax = (widthSurface - 2 * minDistance) * heightWin;

					if (areaSurfaceMax > areaWindow){
						widthWin = areaWindow / ( heightWin * count);
					}
					else {
						widthWin = ( widthSurface - (count + 1) * minDistance ) / count;
						heightWin = std::min(heightWinMax1, areaWindow / ( widthWin * count));
					}
				}
				break;
				case Width:{
					widthWin = std::min(widthWinPre, widthWinMax);
					double areaSurfaceWidth = widthWinPre * count * heightWinMax1;
					if(areaSurfaceWidth >= areaWindow){
						heightWin = areaWindow / ( widthWin * count);
					}
					else{
						heightWin = heightWinMax1;
						// if necessary adjust width
						// we know we get a little error on the wwr
						widthWin = std::min (widthWinMax, areaWindow / ( heightWin * count));
						wwrError = 1 - heightWin * widthWin * count / areaWindow;
					}
				}
				break;
				case SillHeight:{
					// Priority 1 sill height
					if (inputData.m_priorities[2] < inputData.m_priorities[1] && inputData.m_priorities[2] < inputData.m_priorities[0]) {
						// Priority 2 window height
						double heightWinMax2 = heightSurface - minDistance - heightSill;
						// This area is calculated from the width and height minus the sill height
						double areaSurfaceSill = (widthSurface - 2 * minDistance) * heightWinMax2;

						// height is next prio
						if(inputData.m_priorities[0] > inputData.m_priorities[1] ){
							/// TODO Dirk das wäre die ganz normale Berechnung für die
							/// Höhe als oberste Priorität
							heightWin = std::min(heightWinPre, heightWinMax2);
							areaSurfaceSill = (widthSurface - 2 * minDistance) * heightWin;
							if(areaSurfaceSill > areaWindow){
								widthWin = areaWindow / ( heightWin * count);
							}
							else{
								widthWin = ( widthSurface - (count + 1) * minDistance ) / count;
								heightWin = std::min(heightWinMax1, areaWindow / ( widthWin * count));
							}
						}
						// Priority 2 window width
						else if(true){
							// This is the maximum possible area that results when the windows are calculated
							// with the maximum possible window height, taking into account the sill height.
							widthWin = std::min(widthWinPre, widthWinMax);
							double areaSurfaceWidth = widthWinPre * count * heightWinMax2;
							if( areaSurfaceWidth >= areaWindow){
								heightWin = areaWindow / ( widthWin * count );
							}
							else{
								if(areaSurfaceSill >= areaWindow){
									heightWin = heightWinMax2;
									// if necessary adjust width
									// we know we get a little error on the wwr
									widthWin = std::min (widthWinMax, areaWindow / ( heightWin * count));
									wwrError = 1 - heightWin * widthWin * count / areaWindow;
								}
								else{
									/// TODO Dirk das wäre die ganz normale Berechnung für die
									/// Breite als oberste Priorität
									widthWin = std::min(widthWinPre, widthWinMax);
									areaSurfaceWidth = widthWin * count * heightWinMax1;
									if(areaSurfaceWidth >= areaWindow){
										heightWin = areaWindow / ( widthWin * count);
									}
									else{
										heightWin = heightWinMax1;
										// if necessary adjust width
										// we know we get a little error on the wwr
										widthWin = std::min (widthWinMax, areaWindow / ( heightWin * count));
										wwrError = 1 - heightWin * widthWin * count / areaWindow;
									}
								}
							}
						}
					}
					else {
						/// TODO : Dirk, else case is missing
						continue;
					}
				}
				break;
				case Distance:{
					/// TODO Dirk implement
					continue;
				}
			}
			height = heightWin;
			width = widthWin;
			//check sill height
			if( heightSurface - height -minDistance < heightSill)
				sillHeight = heightSurface - height - minDistance;
			else
				sillHeight = heightSill;

			double dist = (widthSurface - count * width) / (count + 1);

			// now create the windows
			for (unsigned int i=0; i<count; ++i ){
				std::vector<IBKMK::Vector2D> verts;
				verts.push_back(IBKMK::Vector2D(dist + i * (dist + width), sillHeight));
				verts.push_back(IBKMK::Vector2D((1 + i) * (dist + width), sillHeight));
				verts.push_back(IBKMK::Vector2D((1 + i) * (dist + width), sillHeight + height));
				verts.push_back(IBKMK::Vector2D(dist + i * (dist + width), sillHeight + height));
				windows.push_back(VICUS::Polygon2D(verts));
			}
		}
		else {

			// *** Generate via given geometry ***

			/*
			// TODO : Dirk

			// now create the windows
			double dist = inputData.m_distance;

			unsigned int count = std::min<unsigned int>(inputData.m_maxHoleCount,(int)((widthSurface - dist) / (dist + widthWinPre)));

			for (unsigned int i=0; i<count; ++i){
				std::vector<IBKMK::Vector2D > verts;
				verts.push_back(IBKMK::Vector2D(dist + i * (dist + wPre), heightPreSill));
				verts.push_back((IBKMK::Vector2D((1 + i) * (dist + wPre), hPreSill));
				verts.push_back((IBKMK::Vector2D((1 + i) * (dist + wPre), hPreSill + hPre));
				verts.push_back((IBKMK::Vector2D(dist + i * (dist + wPre), hPreSill + hPre));
				windows.push_back(VICUS::Polygon2D(verts)));
			}
*/
		}

		// no windows generated? skip surface
		if (windows.empty())
			continue;

		// successfully generated windows

		// add surface
		m_generatedSurfaces.push_back(surfacePoly);

		// add the windows to the surface as subsurfaces
		// Mind: each of the generated window surfaces may result in an invalid subsurface, because it may lie
		//       outside the outer surface's polygon
		m_generatedSurfaces.back().setHoles(windows);

		// Note: invalid hole polygons (i.e. overlapping, or outside surface polygon) will still be added
		//       as hole polygons, yet their triangulation data -> holeTriangulationData()[holeIdx] will be empty.
		//       This way the renderer can distinguish between valid hole geometries and invalid geometries.
		//       Invalid geometries are drawn with red outline.
	}

	updateBuffers();
	return true;
}


void NewSubSurfaceObject::updateBuffers() {
	m_indexBufferData.clear();
	m_vertexBufferData.clear();
	m_colorBufferData.clear();

	// populate buffers
	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	// buffer layout:
	// 1 - opaque polygon geometry of all surfaces
	// 2 - transparent polygon geometry of all surfaces (i.e. existing holes/windows)
	// 3 - polygon outline lines

	// first we store the opaque geometry
	for (const VICUS::PlaneGeometry & geometry : m_generatedSurfaces) {
		// change color depending on visibility state and selection state
		QColor col("#8040d0");
		// first add the plane regular
		addPlane(geometry.triangulationData(), col, currentVertexIndex, currentElementIndex, m_vertexBufferData, m_colorBufferData, m_indexBufferData, false);
		// then add the plane again inverted
		addPlane(geometry.triangulationData(), col, currentVertexIndex, currentElementIndex, m_vertexBufferData, m_colorBufferData, m_indexBufferData, true);

		// for now leave out the holes
		// TODO : draw previously existing holes
	}

	m_lineIndex = currentElementIndex;
	m_transparentStartIndex = currentElementIndex;
	// now we add vertexes of the polygon outline and the outlines of the holes
	// first we store the opaque geometry
	// TODO : Andreas
	for (const VICUS::PlaneGeometry & geometry : m_generatedSurfaces) {

	}

	if (m_indexBufferData.empty())
		return;

	// transfer data stored in m_vertexBufferData
	m_vertexBufferObject.bind();
	m_vertexBufferObject.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(Vertex));
	m_vertexBufferObject.release();

	m_indexBufferObject.bind();
	m_indexBufferObject.allocate(m_indexBufferData.data(), m_indexBufferData.size()*sizeof(GLuint));
	m_indexBufferObject.release();

	// also update the color buffer
	m_colorBufferObject.bind();
	m_colorBufferObject.allocate(m_colorBufferData.data(), m_colorBufferData.size()*sizeof(ColorRGBA) );
	m_colorBufferObject.release();
}



void NewSubSurfaceObject::renderOpaque() {
	if (m_indexBufferData.empty())
		return;

	m_vao.bind();

	// first render opaque polygon
	glDrawElements(GL_TRIANGLES, (GLsizei)m_lineIndex, GL_UNSIGNED_INT, nullptr);

	// here we render the lines around the sub-surfaces; invalid surfaces get a red line
//	glDrawElements(GL_LINES, (GLsizei)m_lineIndex, GL_UNSIGNED_INT,
//				   (const GLvoid*)(sizeof(GLuint) * (unsigned long)((GLsizei)m_indexBufferData.size() - m_transparentStartIndex)) );
	// release buffers again
	m_vao.release();
}


void NewSubSurfaceObject::renderTransparent() {
	// we expect:
	//   glDepthMask(GL_FALSE);
	//   shader program has already transform uniform set
	//   glDisable(GL_CULL_FACE);

	return;
	// the render code below is the same for all geometry types, since only the index buffer is used
	if (!m_indexBufferData.empty()) {
		// bind all buffers ("position", "normal" and "color" arrays)
		m_vao.bind();

		glDrawElements(GL_TRIANGLES, (GLsizei)(m_lineIndex - m_transparentStartIndex),
			GL_UNSIGNED_INT, (const GLvoid*)(sizeof(GLuint) * (unsigned long)m_transparentStartIndex));

		// put OpenGL in offset mode
		glEnable(GL_POLYGON_OFFSET_FILL);
		// offset plane geometry a bit so that our transparent planes are always drawn in front of opaque planes
		glPolygonOffset(-0.1f, -2.0f);
		// now draw the geometry
		glDrawElements(GL_TRIANGLES, 0,
			GL_UNSIGNED_INT, (const GLvoid*)(sizeof(GLuint) * (unsigned long)m_lineIndex));
		// turn off line offset mode
		glDisable(GL_POLYGON_OFFSET_FILL);

		// release buffers again
		m_vao.release();
	}
}

} // namespace Vic3D
