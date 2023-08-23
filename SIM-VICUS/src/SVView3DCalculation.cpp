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

#include "SVView3DCalculation.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVDatabase.h"
#include "SVProjectHandler.h"

#include "VICUS_Project.h"
#include "VICUS_Surface.h"
#include "VICUS_Room.h"
#include "VICUS_ComponentInstance.h"
#include "VICUS_BoundaryCondition.h"
#include "VICUS_PlaneTriangulationData.h"

#include "SVUndoModifySurfaceGeometry.h"

#include "IBK_FileReader.h"

#include <QString>
#include <QTranslator>
#include <QProcess>
#include <QProgressDialog>
#include <QPolygonF>

#include <fstream>

const VICUS::Room* parentRoom(const VICUS::Surface &surf) {
	if (surf.m_parent == nullptr)
		return nullptr;

	const VICUS::Room *r = dynamic_cast<const VICUS::Room *>(surf.m_parent);
	if (r != nullptr)
		return r;

	const VICUS::Surface *parentSurf = dynamic_cast<const VICUS::Surface *>(surf.m_parent);
	return parentRoom(*parentSurf);
}


void SVView3DCalculation::calculateViewFactors(QWidget * parent, std::vector<const VICUS::Surface *> selSurfaces) {

	FUNCID(SVView3DDialog::exportView3d);

	// TODO Stephan: child surfaces ....

	// calculate the number of rooms that will be processed
	std::set<unsigned int> roomIds;
	for (const VICUS::Surface *surf : selSurfaces) {
		qDebug() << "Surface name: " << surf->m_displayName;
		const VICUS::Room *r = dynamic_cast<const VICUS::Room *>(surf->m_parent);
		if (r != nullptr){
			roomIds.insert(r->m_id);
		}
	}
	int numberOfRooms = roomIds.size();


	//exit if the whole room wasn't selected
	for(unsigned int roomId : roomIds){
		// for each room
		const VICUS::Room * r = SVProjectHandler::instance().project().roomByID(roomId);
		for (const VICUS::Surface & s : r->m_surfaces){
			// check if the surface was selected
			bool found = false;
			for (const VICUS::Surface * selS : selSurfaces){
				if(s.m_id == selS->m_id){
					found = true;
				}
			}
			if (!found) {
				selSurfaces.push_back(&s);

				//				QMessageBox::critical(&SVMainWindow::instance(), QString(),
				//									  tr("All surfaces of a room must be selected for view factor calculation! Surface '%1' of room '%2' is not selected").arg(s.m_displayName).arg(r->m_displayName));
				//				return;
			}
		}
	}


	//show a progressDialog
	QProgressDialog dlg(tr("Calculating view factors"), tr("Abort"), 0, numberOfRooms, parent);
	dlg.setWindowModality(Qt::WindowModal);
	dlg.setMinimumDuration(1000);

	// this value will be changed and set to the dialog
	int progressCount = 0;
	dlg.setValue(progressCount);

	SVDatabase &db = SVSettings::instance().m_db;

	unsigned int vertexId = 0;
	unsigned int surfaceId = 0;

	std::map< const view3dRoom *, std::map<const VICUS::Surface*,double> > surfToViewFactorMap;

	// Map with View3D Rooms
	std::map<unsigned int, view3dRoom> vicusRoomIdToView3dRoom;
	std::vector<VICUS::Surface> modifiedSurfaces;				///>modified surfaces for the Undo Action

	for (const VICUS::Surface *surf : selSurfaces) {

		qDebug() << surf->m_displayName;

		// We iterate through all selected surfaces
		// then we triangulate them and compose our View3D Objects
		VICUS::Surface s = *surf;

		// TODO : Stephan/Dirk, review if this still works when there are windows in the wall
		const std::vector<IBKMK::Triangulation::triangle_t> &triangles = s.geometry().triangulationData().m_triangles;
		const std::vector<IBKMK::Vector3D> &vertexes = s.geometry().triangulationData().m_vertexes;
		const std::vector<VICUS::PlaneTriangulationData> &holesTriangles = s.geometry().holeTriangulationData();	// we get all holes
		const std::vector<VICUS::SubSurface> &subSurfs = s.subSurfaces();								// we get all subsurfaces
		const std::vector<VICUS::PlaneGeometry::Hole> &holes = s.geometry().holes();

		// we skip all dump geometries
		const VICUS::Room *r = parentRoom(s);
		if ( r == nullptr ) // we only want surfaces that are assigned to rooms
			continue;

		// if we did not yet add a room object
		std::map<unsigned int, view3dRoom>::const_iterator it = vicusRoomIdToView3dRoom.find(r->m_id);
		if (it == vicusRoomIdToView3dRoom.end()) {
			// we found no entry
			vicusRoomIdToView3dRoom[r->m_id] = view3dRoom(r->m_id, r->m_displayName);
		}

		view3dRoom &v3dRoom = vicusRoomIdToView3dRoom[r->m_id];
		unsigned int &offset = v3dRoom.m_offset;

		view3dExtendedSurfaces extendedSurf ( surf->m_id );
		v3dRoom.m_extendedSurfaces.push_back( extendedSurf );
		for (const VICUS::SubSurface &subSurf : s.subSurfaces() ) {
			view3dExtendedSurfaces extendedSubSurf ( subSurf.m_id, true );
			v3dRoom.m_extendedSurfaces.push_back( extendedSubSurf );	// in extended surfaces we share the view factor
		}

		// save the outline vertexes of the polygon
		for ( const IBKMK::Vector3D &v : vertexes ) {

			if ( !v3dRoom.m_vertexes.empty() )
				vertexId = v3dRoom.m_vertexes.back().m_id;
			else {
				vertexId = 0;
				offset = 1;
			}

			view3dVertex vView3d (++vertexId, v);
			v3dRoom.m_vertexes.push_back(vView3d);	// we add the vertexes

		}

		unsigned int surfId = 0;
		unsigned int counter = 0;

		// we take all triangles from triangulation and combine them in view3D
		// first we take all triangles of the surfaces
		for ( const IBKMK::Triangulation::triangle_t &triangle : triangles) { // mind that our surfaces have to point inwards
			if (triangle.isDegenerated())
				continue;

			if ( !v3dRoom.m_surfaces.empty() )
				surfaceId = v3dRoom.m_surfaces.back().m_id;
			else
				surfaceId = 0;

			std::vector<IBKMK::Vector3D> verts3d(3);
			verts3d[0] = v3dRoom.m_vertexes[offset - 1 + triangle.i1].m_vertex;
			verts3d[1] = v3dRoom.m_vertexes[offset - 1 + triangle.i2].m_vertex;
			verts3d[2] = v3dRoom.m_vertexes[offset - 1 + triangle.i3].m_vertex;

			IBKMK::Polygon3D poly;
			try {
				poly.setVertexes(verts3d, false);
			}
			catch (IBK::Exception &ex) {
				continue;
			}

			if (!poly.isValid())
				continue;

			view3dSurface sView3d (++surfaceId, s.m_id, offset + triangle.i3, offset + triangle.i2, offset + triangle.i1, 0, counter, 0.001,
								   "[" + std::to_string(s.m_id) + "] " + s.m_displayName.toStdString() + "["  + std::to_string(++surfId) + "]" );

			v3dRoom.m_surfaces.push_back(sView3d);

			surfToViewFactorMap[&v3dRoom] = std::map<const VICUS::Surface*,double> ();
			surfToViewFactorMap[&v3dRoom][surf] = 0.0;

			counter == 0 ? counter = surfaceId : 0; // we combine all triangles
		}

		// =====================================
		// we take also all holes with windows
		// =====================================
		for (unsigned int i=0; i<holesTriangles.size(); ++i) {
			counter = 0;
			offset = v3dRoom.m_vertexes.size()+1;

			const VICUS::PlaneTriangulationData &planeTriangulation = holesTriangles[i];

			if (holes[i].m_isChildSurface)
				continue;

			const VICUS::SubSurface &subS = subSurfs[i];

			for (const IBKMK::Vector3D &vHole : planeTriangulation.m_vertexes) {
				if ( !v3dRoom.m_vertexes.empty() ) {
					vertexId = v3dRoom.m_vertexes.back().m_id;
				}
				else {
					vertexId = 0;
					offset = 1;
				}

				view3dVertex vView3d (++vertexId, vHole);
				v3dRoom.m_vertexes.push_back(vView3d);

			}

			// we take all triangles from triangulation and combine them in view3D
			for (unsigned int i=0; i<planeTriangulation.m_triangles.size(); ++i) { // mind that our surfaces have to point inwards

				const IBKMK::Triangulation::triangle_t &triangle = planeTriangulation.m_triangles[i];

				if (triangle.isDegenerated())
					continue;

				if ( !v3dRoom.m_surfaces.empty() )
					surfaceId = v3dRoom.m_surfaces.back().m_id;
				else
					surfaceId = 0;

				std::vector<IBKMK::Vector3D> verts3d(3);
				verts3d[0] = v3dRoom.m_vertexes[offset - 1 + triangle.i1].m_vertex;
				verts3d[1] = v3dRoom.m_vertexes[offset - 1 + triangle.i2].m_vertex;
				verts3d[2] = v3dRoom.m_vertexes[offset - 1 + triangle.i3].m_vertex;

				IBKMK::Polygon3D poly;
				try {
					poly.setVertexes(verts3d, false);
				}
				catch (IBK::Exception &ex) {
					continue;
				}

				if (!poly.isValid())
					continue;

				view3dSurface sView3d (++surfaceId, s.m_id, offset + triangle.i3, offset + triangle.i2, offset + triangle.i1, 0, counter, 0.001,
									   "[" + std::to_string(s.m_id) + "] " + s.m_displayName.toStdString() + " " + subS.m_displayName.toStdString() + " " + "["  + std::to_string(++surfId) + "]" );

				v3dRoom.m_surfaces.push_back(sView3d);

				surfToViewFactorMap[&v3dRoom] = std::map<const VICUS::Surface*,double> ();
				surfToViewFactorMap[&v3dRoom][surf] = 0.0;

				counter == 0 ? counter = surfaceId : 0; // we combine all triangles
			}
		}
		// ==================================================================

		// now we are also looking for component instances
		if ( s.m_componentInstance != nullptr ){
			const VICUS::ComponentInstance &compInst = *s.m_componentInstance;
			const VICUS::Component *comp = db.m_components[compInst.m_idComponent];
			if ( compInst.m_idSideASurface == s.m_id ){
				if ( comp != nullptr ) {
					const VICUS::BoundaryCondition *boundCond = db.m_boundaryConditions[comp->m_idSideABoundaryCondition];
					if ( boundCond != nullptr ) {
						v3dRoom.m_surfaces.back().m_emittance = boundCond->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
					}
				}
			}
			else if ( compInst.m_idSideBSurface == s.m_id ){
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

	// ==================================
	// we now initiate the FILE EXPORT
	// ==================================

	// we store view3D files within our project's base directory
	QString projectBasePath = QFileInfo(SVProjectHandler::instance().projectFile()).absoluteFilePath();



	// remove extension
	int pos = projectBasePath.lastIndexOf(".");
	if (pos != -1)
		projectBasePath = projectBasePath.left(pos);
	QString view3dPath = projectBasePath + "/view3D/";

	QDir dirView3d (view3dPath);

	if( !dirView3d.exists() )
		dirView3d.mkpath(view3dPath); // create base directory and view3D subdirectory as well

	// we generate a view3D input file for each room, run the solver and parse the results
	for ( std::map<unsigned int, view3dRoom>::iterator itRoom = vicusRoomIdToView3dRoom.begin();
		  itRoom != vicusRoomIdToView3dRoom.end(); ++itRoom)
	{
		view3dRoom &room = itRoom->second;

		for ( view3dExtendedSurfaces &extSurf : room.m_extendedSurfaces ) {
			for ( view3dExtendedSurfaces &extSurf2 : room.m_extendedSurfaces ) {

				//	if ( extSurf.m_vicusSurface->m_id == extSurf2.m_vicusSurface->m_id )
				//		continue;
				unsigned int id = extSurf2.m_idVicusSurface;
				extSurf.m_vicSurfIdToViewFactor[extSurf2.m_idVicusSurface] = 999;
			}
		}

		const std::vector<view3dSurface> &surfaces = room.m_surfaces;
		const std::vector<view3dVertex> &vertexes = room.m_vertexes;

		QString roomName = room.m_displayName;
		// TODO : Stephan, display names may contain characters that are invalid for file names;
		//        suggest to process all room names before hand and eleminate all characters that are not
		//        in the whitelist (ie. [a-z], [A-Z], [0..9], "-_![()]|"

		roomName.replace(" ", "");

		// generate a unique file name
		roomName = QString("%1_[%2]").arg(roomName).arg(room.m_roomId);

		QString path = view3dPath + roomName + ".v3s";
		QString result = view3dPath + roomName + "_results.txt";

		QFile file( path );

		if( !file.open(QIODevice::WriteOnly | QIODevice::Text) )
			throw IBK::Exception(IBK::FormatString("Could not create view3D file at path '%1'").arg(path.toStdString()), FUNC_ID);

		// TODO Stephan, currently we write the file in the standard encoding of the platform...
		QTextStream out(&file);
		// Title of View3D file
		out << "T\tGenerated View3D-file by SIM-VICUS for room " << roomName << "\n";
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
				   .arg(v.m_vertex.m_z);
		}
		out << "!--------------------------------------\n";

		// all surfaces
		out << "!\t#\tv1\tv2\tv3\tv4\tbase\tcmb\temit\tname\tsurface data\n";
		for (const view3dSurface &s : surfaces) {
			out << "S\t" << QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t").arg(s.m_id)
				   .arg(s.m_v1)
				   .arg(s.m_v2)
				   .arg(s.m_v3)
				   .arg(s.m_v4)
				   .arg(0)
				   .arg(s.m_combId)
				   .arg(s.m_emittance) << QString::fromStdString(s.m_name) << "\n";
		}
		out << "!--------------------------------------\n";
		out << "End of data\n";

		file.close();

		// compose command line for View3D executable

		/// Example for terminal:
		/// ===================================================
		/// view3d.exe WE0.0_Sleeping.v3s viewfactorResults.txt
		/// ===================================================

		QStringList commandLineArgs;
		commandLineArgs << path << result;

		// we run the process and wait for it to finish - this might take a second on larger projects and freeze the
		// UI. If this becomes an issue, we need to write it multi-threaded.
		IBK::IBK_Message(IBK::FormatString("Running View3D for room '%1' [#%2].\n").arg(room.m_displayName.toStdString()).arg(room.m_roomId),
						 IBK::MSG_PROGRESS, FUNC_ID);

		QProcess p;
		p.setWorkingDirectory(view3dPath);
		p.start(SVSettings::view3dExecutable(), commandLineArgs);

		p.waitForFinished();

		if (p.exitCode() != 0) {
			std::string file = view3dPath.toStdString() + "View3D.log";

			QFile fileIn(QString::fromStdString((file)));
			if(!fileIn.open(QIODevice::ReadOnly)) {
				QMessageBox::information(0, "error", fileIn.errorString());
			}

			QTextStream in(&fileIn);

			QString log;
			while(!in.atEnd()) {
				log += in.readLine() + "\n";
			}

			if (log.isEmpty())
				log = "View3D exited without any error log";

			dlg.cancel();

			QMessageBox box(parent);
			box.setDetailedText(QString::fromStdString(log.toStdString()));
			box.setIcon(QMessageBox::Critical);
			box.setText(tr("Error running view-factor calculcation with View3D for room '%2'. See Error-log below.").arg(room.m_displayName));
			box.setWindowTitle(tr("View-factor generation error"));
			box.setFixedHeight(600);
			box.exec();

			return; // abort export
		}

		readView3dResults(modifiedSurfaces, IBK::Path(result.toStdString() ), room );

		dlg.setValue(++progressCount);
		//check if it was cancelled, if so exit the loop
		if(dlg.wasCanceled()) {
			break;
		}
	}

	dlg.hide();
	if(!dlg.wasCanceled()){
		QMessageBox::information(parent, QString(), tr("View factors have been calculated for all selected rooms."));
		// trigger the undo action with the modified surfaces
		SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("View factors added"), modifiedSurfaces );
		undo->push();
	} else {
		QMessageBox::critical(parent, QString(), tr("Calculation of View factors was canceled."));
	}

}


// return the object area depending if its a surface or subsurface
double areaFromVicusObjectId(unsigned int id) {
	const VICUS::Object * obj = SVProjectHandler::instance().project().objectById(id);
	const VICUS::Surface * surf = dynamic_cast< const VICUS::Surface *>(obj);
	if(surf != nullptr) {
		return surf->areaWithoutSubsurfaces();
	} else {
		const VICUS::SubSurface * subSurf = dynamic_cast< const VICUS::SubSurface *>(obj);
		if (subSurf != nullptr) {
			return subSurf->m_polygon2D.area(3);
		}
	}
	return -1;
}


void SVView3DCalculation::readView3dResults(std::vector<VICUS::Surface> &modifiedSurfaces, IBK::Path fname, view3dRoom &v3dRoom) {
	FUNCID(SVView3DDialog::readView3dResults);

	std::vector<std::string> cont;

	// we take the IBK File Reader in order to read result files
	try {
		IBK::FileReader::readAll( fname, cont, std::vector<std::string>() );

	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Could not read View3D Results."), FUNC_ID);
	}

	std::vector<double> area(v3dRoom.m_extendedSurfaces.size() );

	// extract vector of string-xy-pairs
	std::vector<std::string> tokens;
	for ( unsigned int i=0; i< cont.size(); ++i ) {
		if ( i == 0 || i+1 == cont.size())	// skip first and last line
			continue;

		std::string &line = cont[i];
		IBK::explode(line, tokens, " ", IBK::EF_NoFlags);

		for ( unsigned int j=0; j<tokens.size(); ++j ) {
			std::string &token = tokens[j];
			if ( i == 1 ) {
				// store all the areas from the first line of the result file
				area[j] = IBK::string2val<double>(token);
				continue;
			}
			// check if the area is almost matching
			if(areaFromVicusObjectId(v3dRoom.m_extendedSurfaces[j].m_idVicusSurface) - area[j] < 0.1){
				// get the sub(surface) from the v3dRoom
				const VICUS::Object * obj = SVProjectHandler::instance().project().objectById(v3dRoom.m_extendedSurfaces[i-2].m_idVicusSurface);
				// check if the current object is a surface or a subsurface
				const VICUS::Surface * surf = dynamic_cast< const VICUS::Surface *>(obj);
				if (surf != nullptr) {
					//check if the surface is already in the modified list
					bool foundSurface = false;
					for(VICUS::Surface & modS : modifiedSurfaces){
						// skip view factor to itself, since its always 0
						if(modS.m_id == surf->m_id){
							// already exists, add the value and go to next
							modS.m_viewFactors.m_values[v3dRoom.m_extendedSurfaces[j].m_idVicusSurface] = std::vector<double>{IBK::string2val<double>(token)};
							foundSurface = true;
							break;
						}
					}
					if(!foundSurface && surf->m_id != v3dRoom.m_extendedSurfaces[j].m_idVicusSurface){
						// does not exist already, create a copy and append to the modified surfaces
						VICUS::Surface modS(*surf);
						modS.m_viewFactors.m_values.clear();
						// is a surface
						// store the viewFactor
						modS.m_viewFactors.m_values[v3dRoom.m_extendedSurfaces[j].m_idVicusSurface] = std::vector<double>{IBK::string2val<double>(token)};
						modifiedSurfaces.push_back(modS);
					}
				}
				else {
					// then the object should be a subsurface
					const VICUS::SubSurface * subSurf = dynamic_cast< const VICUS::SubSurface *>(obj);
					if (subSurf != nullptr) {
						// is a subsurface
						// store the viewFactor

						//check if parent is already in list
						const VICUS::Surface * surf = dynamic_cast< const VICUS::Surface *>(subSurf->m_parent);
						bool foundSurface = false;
						for(VICUS::Surface & modS : modifiedSurfaces){
							if(modS.m_id == surf->m_id){
								//modified surface is already there
								//get the subsurface with the mathcing id and change its view factors
								for(const VICUS::SubSurface & modSs : modS.subSurfaces()){
									if(modSs.m_id == subSurf->m_id){
										const_cast<VICUS::SubSurface *>(&modSs)->m_viewFactors.m_values[v3dRoom.m_extendedSurfaces[j].m_idVicusSurface] = std::vector<double>{IBK::string2val<double>(token)};
										foundSurface = true;
										break;
									}
								}
							}
						}
						if(!foundSurface){
							// parent is not in list yet
							throw IBK::Exception(IBK::FormatString("SubSurface was selected for view factor calculation, but not its parent surface!"), FUNC_ID);
						}
					}
					else {
						throw IBK::Exception(IBK::FormatString("Exported Object is wether a surface nor a subsurface"), FUNC_ID);
					}
				}
			}
			else {
				throw IBK::Exception(IBK::FormatString("Area of (sub)surface does not match, should be: %1 but was: %2!").arg(area[j]).arg(areaFromVicusObjectId(v3dRoom.m_extendedSurfaces[j].m_idVicusSurface)), FUNC_ID);
			}

		}
	}
}
