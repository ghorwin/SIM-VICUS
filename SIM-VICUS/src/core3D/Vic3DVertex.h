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
		x(float(coords.x())),
		y(float(coords.y())),
		z(float(coords.z())),
		nx(float(normal.x())),
		ny(float(normal.y())),
		nz(float(normal.z()))
	{
	}

	/*! Coordinates of the vertex. */
	float x,y,z;
	/*! Normal vector associated with vertex (should be normalized). */
	float nx,ny,nz;
};

/*! RGBA data vertex. */
struct ColorRGBA {
	ColorRGBA() {}
	ColorRGBA(const QColor & c) :
		r(c.redF()),
		g(c.greenF()),
		b(c.blueF()),
		a(c.alphaF())
	{
	}

	/*! Color components. */
	float r,g,b,a;
};

} // namespace Vic3D

#endif // Vic3DVertexH
