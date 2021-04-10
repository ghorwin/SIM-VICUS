#ifndef SVView3DH
#define SVView3DH

#include <vector>

#include <IBKMK_Vector3D.h>
#include <IBK_Path.h>

class SVView3D
{
public:
	SVView3D();

	/*! Exports a View3D File */
	void exportView3d(IBK::Path fname = IBK::Path("C:\test.v3s") );


	struct view3dVertex
	{
		view3dVertex() {}

		view3dVertex(unsigned int id, const IBKMK::Vector3D &v) :
			m_id(id),
			m_vertex(v)
		{

		}

		view3dVertex(unsigned int id, double x, double y, double z) :
			m_id(id),
			m_vertex(IBKMK::Vector3D (x, y, z) )
		{}

		unsigned int		m_id;

		IBKMK::Vector3D		m_vertex;

	};

	struct view3dSurface
	{

		view3dSurface() {}

		view3dSurface(unsigned int id, unsigned int vicusId, unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4,
					  unsigned int combId, double emittance, const std::string &name) :
			m_id(id),
			m_vicusId(vicusId),
			m_v1(v1),
			m_v2(v2),
			m_v3(v3),
			m_v4(v4),
			m_combId(combId),
			m_emittance(emittance),
			m_name(name)
		{}

		unsigned int					m_id;

		unsigned int					m_vicusId;

		unsigned int					m_v1;
		unsigned int					m_v2;
		unsigned int					m_v3;
		unsigned int					m_v4;

		unsigned int					m_combId;

		double							m_emittance;

		std::string						m_name;


	};


	std::vector<view3dVertex>		m_vertexes; ///> Vector with View3D Surfaces
	std::vector<view3dSurface>		m_surfaces; ///> Vector with View3D Surfaces

};

#endif // SVView3DH
