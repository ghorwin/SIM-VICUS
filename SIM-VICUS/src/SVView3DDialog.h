#ifndef SVView3DH
#define SVView3DH

#include <vector>

#include <VICUS_Surface.h>

#include <IBKMK_Vector3D.h>

#include <IBK_Path.h>

#include <QString>
#include <QDialog>

class SVView3D : public QDialog
{
	Q_OBJECT

public:
	SVView3D();


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

	struct extendedSurfaces {

		extendedSurfaces( ) {}

		extendedSurfaces( const VICUS::Surface *vicusSurf ) :
			m_vicusSurface(vicusSurf)
		{}

		const VICUS::Surface *					m_vicusSurface;

		std::map<const VICUS::Surface*, double> m_vicSurfToViewFactor;

	};

	struct view3dRoom {

		view3dRoom()  {}

		view3dRoom(unsigned int roomId, QString displayName) :
			m_roomId(roomId),
			m_displayName(displayName)
		{ }

		unsigned int							m_roomId;

		QString									m_displayName;

		std::vector<view3dSurface>				m_surfaces;
		std::vector<view3dVertex>				m_vertexes;

		std::vector<extendedSurfaces>			m_extendedSurfaces;


	};


	/*! Exports a View3D File */
	void exportView3d();

	/*! Reads an View3D Log file with results */
	void readView3dResults(IBK::Path fname, const view3dRoom &v3dRoom);

	QString													m_solverExecutable;

	std::vector<const VICUS::Surface*>						m_selSurfaces;

	std::map<unsigned int, view3dRoom>						m_vicusRoomIdToView3dRoom;		///> Map with View3D Rooms

};

#endif // SVView3DH
