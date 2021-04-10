#include "SVView3D.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVDatabase.h"

#include "VICUS_Project.h"
#include "VICUS_Surface.h"
#include "VICUS_ComponentInstance.h"
#include "VICUS_BoundaryCondition.h"

#include "IBK_FileUtils.h"

#include <QString>

#include <fstream>

SVView3D::SVView3D()
{

}

void SVView3D::exportView3d(IBK::Path fname) {

	std::vector<const VICUS::Surface*> selSurfaces;

	// We take all our selected surfaces
	project().selectedSurfaces(selSurfaces,VICUS::Project::SG_All);

	SVDatabase &db = SVSettings::instance().m_db;

	// We iterate through all selected surfaces
	// then we triangulate them and compose our View3D Objects
	unsigned int vertexId = 0;
	unsigned int surfaceId = 0;
	unsigned int offset = 1;

	for (const VICUS::Surface *surf : selSurfaces) {
		VICUS::Surface s = *surf;
		s.m_geometry.computeGeometry(); // is this necessary???

		const std::vector<VICUS::PlaneGeometry::triangle_t> &triangles = s.m_geometry.triangles();
		const std::vector<IBKMK::Vector3D> &vertexes = s.m_geometry.vertexes();

		// we compose our view 3D Surface
		for ( const IBKMK::Vector3D &v : vertexes ) {
			view3dVertex vView3d (++vertexId, v);
			m_vertexes.push_back(vView3d);
		}

		unsigned int surfId = 0;
		unsigned int counter = 0;
		for ( const VICUS::PlaneGeometry::triangle_t &triangle : triangles) { // mind that our surfaces have to point inwards
			view3dSurface sView3d (++surfaceId, s.m_id, offset + triangle.c, offset + triangle.b, offset + triangle.a, 0, counter, 0.9,
								   "[" + std::to_string(s.m_id) + "] " + s.m_displayName.toStdString() + "["  + std::to_string(++surfId) + "]" );
			m_surfaces.push_back(sView3d);
			counter == 0 ? counter = surfaceId : 0; // we combine all triangles
		}

		// now we are also looking for component instances
		if ( s.m_componentInstance != nullptr ){
			const VICUS::ComponentInstance &compInst = *s.m_componentInstance;
			const VICUS::Component *comp = db.m_components[compInst.m_componentID];
			if ( compInst.m_sideASurfaceID == s.m_id ){
				if ( comp != nullptr ) {
					const VICUS::BoundaryCondition *boundCond = db.m_boundaryConditions[comp->m_idSideABoundaryCondition];
					if ( boundCond != nullptr ) {
						m_surfaces.back().m_emittance = boundCond->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
					}
				}
			}
			else if ( compInst.m_sideBSurfaceID == s.m_id ){
				if ( comp != nullptr ) {
					const VICUS::BoundaryCondition *boundCond = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition];
					if ( boundCond != nullptr ) {
						m_surfaces.back().m_emittance = boundCond->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
					}
				}
			}
		}

		offset = vertexId + 1;
	}


	// now we compose a View3D Export file
	std::ofstream out("viewfactor.vs3");

	// Title of View3D file
	out << "T\tGenerated View3D-file by SIM-VICUS\n";
	out << "!--------------------------------------\n";

	/// (C c)The control line specifies white space separated parameters as name = value pairs.  Any of the followingparameters may be
	/// included in any order. For each parameter, the default value is shown in parentheses.
	///		– eps (1.0e-4):integration convergence
	/// criterion for both adaptive integration and view obstruction calcula-tions. This is not an exact measure of the accuracy of
	/// the computed view factors, but smaller values willusually lead to more precise values. The convergence criteria should not
	/// be less than about 1.0e-6 becausemany of the intermediate calculations are accurate only to single (32-bit) precision.
	///		– maxU (12):maximum recursions used in computing the unobstructed view factors.
	///		– maxO (8):maximum recursions used in computing the obstructed view factors.  Limiting the maximumnumber of recursions
	///  limits the total execution time of the program but may prevent reaching the specifiedconvergence.3
	///		– minO (0):minimum recursions used in computing the obstructed view factors. This can help in caseswhere an obstruction
	/// occurs very near the view between the edges of two surfaces.  The normal adaptiveintegration may miss the obstruction.
	/// Increasing this value from its normal value of 0 to 1 or 2 may catchthe obstruction. This is probably not necessary except
	/// when very accurate view factors are desired. It canadd considerably to execution time.
	///		– row (0):selected row for computing view factors (0 = all rows)– col (0):selected column for computing view factors
	/// (0 = all columns)– encl (0):1 indicates that the surfaces form an enclosure; 0 indicates that they do not. This data
	/// is used toadjust the view factors of an enclosure to guarantee conservation of energy.
	///		– emit (0):1 indicates that diffuse reflectance effects will be included in the computed view factors (Hottel’s“script-F”
	/// view factors); 0 indicates they will not, i.e., surfaces will be considered ’black’.
	///		– out (0):view factor output file format – 0 = simple text file; 1 = binary file.
	///		– list (0):computational summary written to the View3D.log file – 0 gives minimal information; 1 givesslightly more;
	/// 2 prints all the view factors; 3 causes dumping of some intermediate values.
	out << "C eps = 0.0001 col = 0 row = 0 encl = 0 maxU = 8 maxO = 8 minO = 0 emit = 0 out = 0 list = 3\n";
	out << "!--------------------------------------\n";
	// all vertices
	out << "!\t#\tx\ty\tz\tcoordinates of vertices\n";

	for (const view3dVertex &v : m_vertexes) {
		out << "V\t" + QString("%1\t%2\t%3\t%4\n").arg(v.m_id)
													.arg(v.m_vertex.m_x)
													.arg(v.m_vertex.m_y)
													.arg(v.m_vertex.m_z).toStdString();
	}
	out << "!--------------------------------------\n";
	// all surfaces

	out << "!\t#\tv1\tv2\tv3\tv4\tbase\tcmb\temit\tname\tsurface data\n";
	for (const view3dSurface &s : m_surfaces) {
		out << "S\t" << IBK::FormatString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t").arg(s.m_id)
																		.arg(s.m_v1)
																		.arg(s.m_v2)
																		.arg(s.m_v3)
																		.arg(s.m_v4)
																		.arg(0)
																		.arg(s.m_combId)
																		.arg(s.m_emittance) << s.m_name << "\n";
	}
	out << "!--------------------------------------\n";
	out << "End of data\n";

	out.close();
}
