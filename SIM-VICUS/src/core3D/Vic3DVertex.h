/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

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
