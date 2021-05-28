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

#ifndef Vic3DVertexH
#define Vic3DVertexH

#include <QVector3D>
#include <QColor>

namespace Vic3D {

/*! A container class to store data (coordinates, normals) of a vertex, used for interleaved
	storage.

	Memory layout (each char is a byte): xxxxyyyyzzzziiiijjjjkkkk = 6*4 = 24 Bytes

	You can define a vector<Vertex> and use this directly as input to the vertex buffer.

	Mind implicit padding by compiler! Hence, for allocation use:
	- sizeof(Vertex) as stride
	- offsetof(Vertex, nx) as start offset for the normal vector

	This will only become important, if mixed data types are used in the struct.
	Read http://www.catb.org/esr/structure-packing/ for an in-depth explanation.
*/
struct Vertex {
	Vertex() {}
	Vertex(const QVector3D & coords, const QVector3D & normal) :
		m_coords(coords), m_normal(normal)
	{
	}

	/*! Coordinates of the vertex. */
	QVector3D m_coords;
	/*! Normal vector associated with vertex (should be normalized). */
	QVector3D m_normal;
};

/*! Vertex struct holding only coordinates. */
struct VertexC {
	VertexC() {}
	VertexC(const QVector3D & coords) :
		m_coords(coords)
	{
	}

	/*! Coordinates of the vertex. */
	QVector3D m_coords;
};

/*! Vertex struct holding coordinates and colors.
	Use in shaders that take coordinate/color vertex data.
*/
struct VertexCR {
	VertexCR() {}
	VertexCR(const QVector3D & coords, const QVector3D & cols) :
		m_coords(coords), m_colors(cols)
	{
	}

	/*! Coordinates of the vertex. */
	QVector3D m_coords;
	/*! Colors. */
	QVector3D m_colors;
};


/*! RGBA data vertex. */
struct ColorRGBA {
	ColorRGBA() {}
	ColorRGBA(const QColor & c) :
		r(c.red()),
		g(c.green()),
		b(c.blue()),
		a(c.alpha())
	{
	}

	/*! Color components (single byte). */
	char r,g,b,a;
};

} // namespace Vic3D

#endif // Vic3DVertexH
