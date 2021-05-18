#include "SVView3DDialog.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVDatabase.h"

#include "VICUS_Project.h"
#include "VICUS_Surface.h"
#include "VICUS_Room.h"
#include "VICUS_ComponentInstance.h"
#include "VICUS_BoundaryCondition.h"

#include "IBK_FileUtils.h"
#include "IBK_FileReader.h"

#include <QString>
#include <QTranslator>

#include <fstream>

SVView3D::SVView3D() {
	m_solverExecutable = SVSettings::view3dExecutable();
}

void SVView3D::exportView3d() {
	FUNCID(SVView3D::exportView3d);



	// We take all our selected surfaces
	project().selectedSurfaces(m_selSurfaces,VICUS::Project::SG_All);

	SVDatabase &db = SVSettings::instance().m_db;

	unsigned int vertexId = 0;
	unsigned int surfaceId = 0;
	unsigned int offset = 1;

	std::map< const view3dRoom *, std::map<const VICUS::Surface*,double> > surfToViewFacorMap;

	for (const VICUS::Surface *surf : m_selSurfaces) {
		// We iterate through all selected surfaces
		// then we triangulate them and compose our View3D Objects

		VICUS::Surface s = *surf;

		// TODO : Stephan/Dirk, review if this still works when there are windows in the wall

		const std::vector<IBKMK::Triangulation::triangle_t> &triangles = s.geometry().triangulationData().m_triangles;
		const std::vector<IBKMK::Vector3D> &vertexes = s.geometry().polygon().vertexes();


		const VICUS::Room *r = dynamic_cast<const VICUS::Room *>(s.m_parent);
		if ( r == nullptr ) // we only want surfaces that are assigned to rooms
			continue;

		if ( m_vicusRoomIdToView3dRoom.find(r->m_id) == m_vicusRoomIdToView3dRoom.end() ) { // we find the entry
			m_vicusRoomIdToView3dRoom[r->m_id] = view3dRoom(r->m_id, r->m_displayName);
		}

		view3dRoom &v3dRoom = m_vicusRoomIdToView3dRoom[r->m_id];

		extendedSurfaces extendedSurf ( surf );
		v3dRoom.m_extendedSurfaces.push_back( extendedSurf );

		// we compose our view 3D Surface
		for ( const IBKMK::Vector3D &v : vertexes ) {

			if ( !v3dRoom.m_vertexes.empty() )
				vertexId = v3dRoom.m_vertexes.back().m_id;
			else {
				vertexId = 0;
				offset = 1;
			}

			view3dVertex vView3d (++vertexId, v);
			v3dRoom.m_vertexes.push_back(vView3d);
		}

		unsigned int surfId = 0;
		unsigned int counter = 0;


		// we take all triangles from triangulation and combine them in view3D
		for ( const IBKMK::Triangulation::triangle_t &triangle : triangles) { // mind that our surfaces have to point inwards

			if ( !v3dRoom.m_surfaces.empty() )
				surfaceId = v3dRoom.m_surfaces.back().m_id;
			else
				surfaceId = 0;

			view3dSurface sView3d (++surfaceId, s.m_id, offset + triangle.i3, offset + triangle.i2, offset + triangle.i1, 0, counter, 0.001,
								   "[" + std::to_string(s.m_id) + "] " + s.m_displayName.toStdString() + "["  + std::to_string(++surfId) + "]" );

			v3dRoom.m_surfaces.push_back(sView3d);

			surfToViewFacorMap[&v3dRoom] = std::map<const VICUS::Surface*,double> ();
			surfToViewFacorMap[&v3dRoom][surf] = 0.0;

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
						v3dRoom.m_surfaces.back().m_emittance = boundCond->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
					}
				}
			}
			else if ( compInst.m_sideBSurfaceID == s.m_id ){
				if ( comp != nullptr ) {
					const VICUS::BoundaryCondition *boundCond = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition];
					if ( boundCond != nullptr ) {
						v3dRoom.m_surfaces.back().m_emittance = boundCond->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
					}
				}
			}
		}

		offset = vertexId + 1;
	}




	for ( std::map<unsigned int, view3dRoom>::iterator itRoom=m_vicusRoomIdToView3dRoom.begin();
		  itRoom != m_vicusRoomIdToView3dRoom.end(); ++itRoom) {

		view3dRoom &room = itRoom->second;

		for ( extendedSurfaces &extSurf : room.m_extendedSurfaces ) {
			for ( extendedSurfaces &extSurf2 : room.m_extendedSurfaces ) {

//				if ( extSurf.m_vicusSurface->m_id == extSurf2.m_vicusSurface->m_id )
//					continue;

				extSurf.m_vicSurfToViewFactor[extSurf2.m_vicusSurface] = 0.0;
			}
		}

		unsigned int id = itRoom->first;
		const std::vector<view3dSurface> &surfaces = room.m_surfaces;
		const std::vector<view3dVertex> &vertexes = room.m_vertexes;

		QString project = QFileInfo(SVProjectHandler::instance().projectFile()).completeBaseName();
		QString view3dPath = QFileInfo(SVProjectHandler::instance().projectFile()).dir().filePath(project) + "_view3D/";

		QDir dirView3d (view3dPath);

		if( !dirView3d.exists() )
			dirView3d.mkdir(view3dPath);

		QString roomName = room.m_displayName;

		roomName.replace(" ", "");

		QString path = QFileInfo(SVProjectHandler::instance().projectFile()).dir().filePath(project) + "_view3D/" + roomName + ".v3s";
		QString result = QFileInfo(SVProjectHandler::instance().projectFile()).dir().filePath(project) + "_view3D/" + roomName + "_results.txt";

		std::ofstream out( path.toStdString() );

		if( !out.is_open() )
			throw IBK::Exception(IBK::FormatString(), FUNC_ID);

		// Title of View3D file
		out << "T\tGenerated View3D-file by SIM-VICUS for room " << roomName.toStdString() << "\n";
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

		for (const view3dVertex &v : vertexes) {
			out << "V\t" + QString("%1\t%2\t%3\t%4\n").arg(v.m_id)
				   .arg(v.m_vertex.m_x)
				   .arg(v.m_vertex.m_y)
				   .arg(v.m_vertex.m_z).toStdString();
		}
		out << "!--------------------------------------\n";
		// all surfaces

		out << "!\t#\tv1\tv2\tv3\tv4\tbase\tcmb\temit\tname\tsurface data\n";
		for (const view3dSurface &s : surfaces) {
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


		//SVSettings::TerminalEmulators runOption = SVSettings::TerminalEmulators::TE_XTerm;
		SVSettings::TerminalEmulators runOption = SVSettings::TerminalEmulators::TE_None;

		/// Example for terminal:
		/// ===================================================
		/// view3d.exe WE0.0_Sleeping.v3s viewfactorResults.txt
		/// ===================================================

		QStringList commandLineArgs;
		commandLineArgs << path;

		// delete working directory if requested
		// launch solver - run option is only needed for linux, and otherwise it will always be -1
		bool success = SVSettings::startProcess(m_solverExecutable, commandLineArgs, result, runOption);
		if (!success) {
			QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(m_solverExecutable));
		}

		// all ok, solver is running
		// results are loaded into data model
		readView3dResults(IBK::Path(result.toStdString() ), room );
	}




}

void SVView3D::readView3dResults(IBK::Path fname, const view3dRoom &v3dRoom) {
	FUNCID(SVView3D::readView3dResults);
#if 0
	std::vector<std::string> cont;
	// we take the IBK File Reader in order to read result files
	try {
		IBK::FileReader::readAll( fname, cont, std::vector<std::string>() );

	} catch (IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("Could not read View3D Results."), FUNC_ID);
	}

	std::vector<double> area(v3dRoom.m_extendedSurfaces.size() );
	// extract vector of string-xy-pairs
	std::vector<std::string> tokens;
	for ( unsigned int i=0; i<cont.size(); ++i ) {
		if ( i == 0 )
			continue;

		std::string &line = cont[i];
		IBK::explode(line, tokens, " ", IBK::EF_NoFlags);

		for ( unsigned int j=0; j<tokens.size(); ++j ) {
			std::string &token = tokens[j];
			if ( i == 1 ) {
				area[j] = IBK::string2val<double>(token); // we take this to check our surface
			}
			else {
				for ( extendedSurfaces surfs : v3dRoom.m_extendedSurfaces) {
					for ( std::map<const VICUS::Surface *, double>::iterator itSurf = surfs.m_vicSurfToViewFactor.begin();
						  itSurf != surfs.m_vicSurfToViewFactor.end(); ++itSurf) {


						itSurf->second = IBK::string2val<double>(token);
					}
				}
			}
		}
	}

#endif
//	}


}
