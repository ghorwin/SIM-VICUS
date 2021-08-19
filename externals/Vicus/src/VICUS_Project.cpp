/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "VICUS_Project.h"

#include <algorithm>
#include <set>
#include <fstream>

#include <IBK_messages.h>
#include <IBK_assert.h>
#include <IBK_Exception.h>

#include <NANDRAD_Utilities.h>
#include <NANDRAD_Project.h>

#include <tinyxml.h>

#include "VICUS_Constants.h"
#include "VICUS_utilities.h"

#define PI				3.141592653589793238


namespace VICUS {

Project::Project() {
	// build test building

#if 0
	Room room;
	room.m_id = 1;
	room.m_displayName = "Room";

	Surface surf;
	surf.m_id = 1;
	surf.m_displayName = "Triangle";
	// flat on the ground, facing up
	surf.m_geometry = PlaneGeometry(PlaneGeometry::T_Triangle, IBKMK::Vector3D(0,0,1), IBKMK::Vector3D(10,0,1), IBKMK::Vector3D(0,10,1));
	surf.m_color = QColor(32,32,192);
	room.m_surfaces.push_back(surf);

	// in the y-z plane, facing into positive x direction
	surf.m_geometry = PlaneGeometry(PlaneGeometry::T_Rectangle, IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,10,0), IBKMK::Vector3D(0,0,10));
	surf.m_id = 2;
	surf.m_displayName = "Rect";
	surf.m_color = QColor(255,96,0);
	room.m_surfaces.push_back(surf);

	surf.m_geometry = PlaneGeometry(PlaneGeometry::T_Polygon);
	std::vector<IBKMK::Vector3D> vertexes;
	vertexes.push_back(IBKMK::Vector3D(-10,-5,0));
	vertexes.push_back(IBKMK::Vector3D(-2,-5,0));
	vertexes.push_back(IBKMK::Vector3D(-2,-5,6));
	vertexes.push_back(IBKMK::Vector3D(-4,-5,6));
	vertexes.push_back(IBKMK::Vector3D(-4,-5,3));
	vertexes.push_back(IBKMK::Vector3D(-10,-5,3));
	surf.m_geometry.setVertexes(vertexes);
	surf.m_id = 3;
	surf.m_displayName = "Poly";
	surf.m_color = QColor(255,255,64);
	room.m_surfaces.push_back(surf);

	BuildingLevel level;
	level.m_id = 1;
	level.m_rooms.push_back(room);
	level.m_displayName = "E0";

	Building build;
	build.m_id = 1;
	build.m_displayName = "Building";
	build.m_buildingLevels.push_back(level);

	m_buildings.push_back(build);

	NetworkFluid f1;
	f1.m_id = 12;
	f1.m_displayName = "water";
	f1.m_para[NetworkFluid::P_Density].set("Density", 1004, IBK::Unit("kg/m3"));
	f1.m_para[NetworkFluid::P_HeatCapacity].set("HeatCapacity", 4180, IBK::Unit("J/kgK"));
	f1.m_para[NetworkFluid::P_Conductivity].set("Conductivity", 0.56, IBK::Unit("W/mK"));
	f1.m_kinematicViscosity.m_name = "KinematicViscosity";
	f1.m_kinematicViscosity.m_xUnit.set("C");
	f1.m_kinematicViscosity.m_yUnit.set("m2/s");
	f1.m_kinematicViscosity.m_interpolationMethod = NANDRAD::LinearSplineParameter::I_LINEAR;

	std::vector<double> x( {20, 40} );
	std::vector<double> y( {1e-6, 0.8e-6} );
	f1.m_kinematicViscosity.m_values.setValues(x,y);

	m_networkFluidDB.push_back(f1);




#endif

#if 0

	// *** Example of geometric VICUS::Network definition and transformation to NANDRAD ***

	// geometric network
	VICUS::Network net;
	unsigned id1 = net.addNodeExt(IBKMK::Vector3D(0,0,0), VICUS::NetworkNode::NT_Source);
	unsigned id2 = net.addNodeExt(IBKMK::Vector3D(0,70,0), VICUS::NetworkNode::NT_Mixer);
	unsigned id3 = net.addNodeExt(IBKMK::Vector3D(100,70,0), VICUS::NetworkNode::NT_Building);
	unsigned id4 = net.addNodeExt(IBKMK::Vector3D(0,200,0), VICUS::NetworkNode::NT_Mixer);
	unsigned id5 = net.addNodeExt(IBKMK::Vector3D(100,200,0), VICUS::NetworkNode::NT_Building);
	unsigned id6 = net.addNodeExt(IBKMK::Vector3D(-100,200,0), VICUS::NetworkNode::NT_Building);
	net.addEdge(id1, id2, true);
	net.addEdge(id2, id3, true);
	net.addEdge(id2, id4, true);
	net.addEdge(id4, id5, true);
	net.addEdge(id4, id6, true);


	//	 components

	NANDRAD::HydraulicNetworkComponent pump;
	pump.m_id = 0;
	pump.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel;
	pump.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].set("PressureHead", 300, IBK::Unit("Pa"));
	net.m_hydraulicComponents.push_back(pump);

	NANDRAD::HydraulicNetworkComponent heatExchanger;
	heatExchanger.m_id = 1;
	heatExchanger.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger;
	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_HeatFlux].set("HeatFlux", 100, IBK::Unit("W"));
	net.m_hydraulicComponents.push_back(heatExchanger);

	net.m_nodes[id1].m_componentId = pump.m_id;
	net.m_nodes[id3].m_componentId = heatExchanger.m_id;
	net.m_nodes[id5].m_componentId = heatExchanger.m_id;
	net.m_nodes[id6].m_componentId = heatExchanger.m_id;


	// pipes
	NetworkPipe p;
	p.m_id = 0;
	p.m_displayName = "PE 32 x 3.2";
	p.m_diameterOutside = 32;
	p.m_sWall = 3.2;
	p.m_roughness = 0.007;
	net.m_networkPipeDB.push_back(p);
	NetworkPipe p2;
	p2.m_id = 1;
	p2.m_displayName = "PE 50 x 4.6";
	p2.m_diameterOutside = 60;
	p2.m_sWall = 4.6;
	p2.m_roughness = 0.007;
	net.m_networkPipeDB.push_back(p2);

	net.edge(id1, id2)->m_pipeId = p.m_id;
	net.edge(id1, id2)->m_modelType = NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;
	net.edge(id2, id3)->m_pipeId = p2.m_id;
	net.edge(id2, id3)->m_modelType = NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;
	net.edge(id2, id4)->m_pipeId = p.m_id;
	net.edge(id2, id4)->m_modelType = NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;
	net.edge(id4, id5)->m_pipeId = p2.m_id;
	net.edge(id4, id5)->m_modelType = NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;
	net.edge(id4, id6)->m_pipeId = p2.m_id;
	net.edge(id4, id6)->m_modelType = NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;

	updatePointers();


	// create Nandrad Network
	NANDRAD::HydraulicNetwork hydrNet;
	std::vector<NANDRAD::HydraulicNetworkComponent> hydraulicComponents;
	hydrNet.m_id = 1;
	hydrNet.m_displayName = "auto generated from geometric network";
	net.createNandradHydraulicNetwork(hydrNet, hydraulicComponents);
	hydrNet.m_fluid.defaultFluidWater(1);


	// write Nandrad project
	NANDRAD::Project prj;
	prj.m_hydraulicNetworks.clear();
	prj.m_hydraulicNetworks.push_back(hydrNet);
	prj.m_hydraulicComponents = hydraulicComponents;
	prj.writeXML(IBK::Path("../../data/hydraulicNetworks/auto_generated.nandrad"));

	updatePointers();

#endif
}


void Project::parseHeader(const IBK::Path & filename) {
	FUNCID(Project::parseHeader);

	std::ifstream inputStream;
#if defined(_WIN32)
#if defined(_MSC_VER)
	inputStream.open(filename.wstr().c_str(), std::ios_base::binary);
#else
	std::string filenameAnsi = IBK::WstringToANSI(filename.wstr(), false);
	inputStream.open(filenameAnsi.c_str(), std::ios_base::binary);
#endif
#else // _WIN32
	inputStream.open(filename.c_str(), std::ios_base::binary);
#endif
	if (!inputStream.is_open()) {
		throw IBK::Exception( IBK::FormatString("Cannot open input file '%1' for reading").arg(filename.c_str()), FUNC_ID);
	}
	std::string line;
	unsigned int i=12;
	while (std::getline(inputStream, line) && --i > 0) {
		// abort when <Project is in the line but not <ProjectInfo
		if (line.find("<Project") != std::string::npos && line.find("<ProjectInfo") == std::string::npos)
			break;
		size_t pos = line.find("<Created>");
		if (pos != std::string::npos) {
			size_t pos2 = line.find("</Created>");
			if (pos2 != std::string::npos)
				m_projectInfo.m_created = line.substr(pos + 9, pos2 - pos - 9);
		}
		pos = line.find("<LastEdited>");
		if (pos != std::string::npos) {
			size_t pos2 = line.find("</LastEdited>");
			if (pos2 != std::string::npos)
				m_projectInfo.m_lastEdited = line.substr(pos + 12, pos2 - pos - 12);
		}
	}
}


void Project::readXML(const IBK::Path & filename) {
	FUNCID(Project::readXML);

	TiXmlDocument doc;
	IBK::Path filenamePath(filename);
	std::map<std::string,IBK::Path> pathPlaceHolders; // only dummy for now, filenamePath does not contain placeholders
	TiXmlElement * xmlElem = NANDRAD::openXMLFile(pathPlaceHolders, filenamePath, "VicusProject", doc);
	if (!xmlElem)
		return; // empty project, this means we are using only defaults

	// we read our subsections from this handle
	TiXmlHandle xmlRoot = TiXmlHandle(xmlElem);

	try {
		xmlElem = xmlRoot.FirstChild("ProjectInfo").Element();
		if (xmlElem) {
			m_projectInfo.readXML(xmlElem);
		}

		// Directory Placeholders
		xmlElem = xmlRoot.FirstChild( "DirectoryPlaceholders" ).Element();
		if (xmlElem) {
			readDirectoryPlaceholdersXML(xmlElem);
		}

		xmlElem = xmlRoot.FirstChild("Project").Element();
		if (xmlElem) {
			readXML(xmlElem);
		}

		// update internal pointer-based links
		updatePointers();

		// set default colors for network objects
		for (const VICUS::Network & net : m_geometricNetworks) {
			// updateColor is a const-function, this is possible since
			// the m_color property of edges and nodes is mutable
			net.setDefaultColors();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading project '%1'.").arg(filename), FUNC_ID);
	}
}


void Project::writeXML(const IBK::Path & filename) const {
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "VicusProject" );
	doc.LinkEndChild(root);

	root->SetAttribute("fileVersion", VERSION);

	if (m_projectInfo != NANDRAD::ProjectInfo())
		m_projectInfo.writeXML(root);
	writeDirectoryPlaceholdersXML(root);

	writeXML(root);

	// other files

	doc.SaveFile( filename.c_str() );

}


void Project::readDirectoryPlaceholdersXML(const TiXmlElement * element) {

	// loop over all elements in this XML element
	for (const TiXmlElement * e=element->FirstChildElement(); e; e = e->NextSiblingElement()) {
		// get element name
		std::string name = e->Value();
		// handle known elements
		if (name == "Placeholder") {
			// search for attribute with given name
			const TiXmlAttribute* attrib = TiXmlAttribute::attributeByName(e, "name");
			if (attrib == nullptr) {
				IBK::IBK_Message(IBK::FormatString(
						"Missing '%1' attribute in Placeholder element.").arg("name"), IBK::MSG_WARNING);
				continue;
			}
			m_placeholders[attrib->Value()] = e->GetText();
		}
		else {
			IBK::IBK_Message(IBK::FormatString(
					"Unknown element '%1' in DirectoryPlaceholders section.").arg(name), IBK::MSG_WARNING);
		}
	}
}


void Project::writeDirectoryPlaceholdersXML(TiXmlElement * parent) const {
	const char * const FUNC_ID = "[Project::writeDirectoryPlaceholdersXML]";

	// lookup all used path placeholders
	std::set<std::string> usedPlaceholders;

	// first glob filenames used anywhere in the project
	std::vector<IBK::Path> fnames;
	fnames.push_back( m_location.m_climateFilePath );

	// TODO : add more file references with placeholders

	// now we extract all the placeholders
	for (std::vector<IBK::Path>::const_iterator it = fnames.begin(); it != fnames.end(); ++it) {
		std::string placeholder;
		IBK::Path myPath(*it);
		IBK::Path dummy;
		if ( myPath.extractPlaceholder( placeholder, dummy ) )
			usedPlaceholders.insert(placeholder);
	}

	// remove default placeholders for project directory
	usedPlaceholders.erase(IBK::PLACEHOLDER_PROJECT_DIR);

	// no placeholders used? nothing to write
	if (usedPlaceholders.empty())
		return;

	TiXmlComment::addComment(parent,
		"DirectoryPlaceholders section defines strings to be substituted with directories");

	TiXmlElement * e1 = new TiXmlElement( "DirectoryPlaceholders" );
	parent->LinkEndChild( e1 );

	for (std::set<std::string>::const_iterator it = usedPlaceholders.begin();
		 it != usedPlaceholders.end(); ++it)
	{

		std::map<std::string, IBK::Path>::const_iterator pit = m_placeholders.find(*it);
		// placeholder should exist, if not user may have tempered with the file and
		// manually inserted a placeholder without adding it to the placeholder section
		// This typically occurs when typos are present in the placeholder name.
		// In such cases, we silently ignore writing the placeholder, but simply write
		// a warning to the user.
		if (pit == m_placeholders.end()) {
			IBK::IBK_Message(IBK::FormatString("Placeholder '%1' is being used in one or more "
											   "referenced files, but not defined within the placeholder section")
							 .arg(*it), IBK::MSG_WARNING, FUNC_ID);
			continue;
		}
		// write placeholder to file
		TiXmlElement::appendSingleAttributeElement(e1, "Placeholder", "name", *it, pit->second.str());
	}
}
// ----------------------------------------------------------------------------

void Project::clean() {

}


void Project::updatePointers() {
	FUNCID(Project::updatePointers);
	// update hierarchy
	for (VICUS::Building & b : m_buildings)
		b.updateParents();

	// clear component/surface pointers (this is needed to check for duplicate IDs later on)
	for (VICUS::Building & b : m_buildings)
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			// TODO : Dirk, sum up net floor area and update bl.m_netFloorArea
			for (VICUS::Room & r : bl.m_rooms)
				for (VICUS::Surface & s : r.m_surfaces) {
					s.m_componentInstance = nullptr;
					for (VICUS::SubSurface & sub : const_cast<std::vector<VICUS::SubSurface> &>(s.subSurfaces()) )
						sub.m_subSurfaceComponentInstance = nullptr;
				}
		}

	// update pointers
	for (VICUS::ComponentInstance & ci : m_componentInstances) {
		// lookup surfaces
		ci.m_sideASurface = surfaceByID(ci.m_idSideASurface);
		if (ci.m_sideASurface != nullptr) {
			// check that no two components reference the same surface
			if (ci.m_sideASurface->m_componentInstance != nullptr) {
				IBK::IBK_Message(IBK::FormatString("Surface %1 is referenced by multiple component instances!")
								 .arg(ci.m_idSideASurface), IBK::MSG_ERROR, FUNC_ID);
			}
			else {
				ci.m_sideASurface->m_componentInstance = &ci;
			}
		}

		ci.m_sideBSurface = surfaceByID(ci.m_idSideBSurface);
		if (ci.m_sideBSurface != nullptr) {
			// check that no two components reference the same surface
			if (ci.m_sideBSurface->m_componentInstance != nullptr) {
				IBK::IBK_Message(IBK::FormatString("Surface %1 is referenced by multiple component instances!")
								 .arg(ci.m_idSideBSurface), IBK::MSG_ERROR, FUNC_ID);
			}
			else {
				ci.m_sideBSurface->m_componentInstance = &ci;
			}
		}
		if (ci.m_idSurfaceHeatingControlZone != VICUS::INVALID_ID) {
			for (VICUS::Building & b : m_buildings)
				for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
					for (VICUS::Room & r : bl.m_rooms)
						if (r.m_id == ci.m_idSurfaceHeatingControlZone)
							ci.m_surfaceHeatingControlZone = &r;
		}
	}

	// update pointers in subsurfaces
	for (VICUS::SubSurfaceComponentInstance & ci : m_subSurfaceComponentInstances) {
		// lookup surfaces
		ci.m_sideASubSurface = subSurfaceByID(ci.m_idSideASurface);
		if (ci.m_sideASubSurface != nullptr) {
			// check that no two components reference the same surface
			if (ci.m_sideASubSurface->m_subSurfaceComponentInstance != nullptr) {
				IBK::IBK_Message(IBK::FormatString("Sub-Surface %1 is referenced by multiple component instances!")
								 .arg(ci.m_idSideASurface), IBK::MSG_ERROR, FUNC_ID);
			}
			else {
				ci.m_sideASubSurface->m_subSurfaceComponentInstance = &ci;
			}
		}

		ci.m_sideBSubSurface = subSurfaceByID(ci.m_idSideBSurface);
		if (ci.m_sideBSubSurface != nullptr) {
			// check that no two components reference the same surface
			if (ci.m_sideBSubSurface->m_subSurfaceComponentInstance != nullptr) {
				IBK::IBK_Message(IBK::FormatString("Sub-Surface %1 is referenced by multiple component instances!")
								 .arg(ci.m_idSideBSurface), IBK::MSG_ERROR, FUNC_ID);
			}
			else {
				ci.m_sideBSubSurface->m_subSurfaceComponentInstance = &ci;
			}
		}

	}


	for (VICUS::Network & n : m_geometricNetworks) {
		n.updateNodeEdgeConnectionPointers();
	}
}


const VICUS::Object * Project::objectById(unsigned int uniqueID) const {
	FUNCID(Project::objectById);
	const VICUS::Object * obj = nullptr;
	// search in buildings
	for (const VICUS::Building & b : m_buildings) {
		obj = b.findChild(uniqueID);
		if (obj != nullptr)
			break;
	}
	// now look in plain geometry
	if (obj == nullptr) {
		for (const VICUS::Surface & s : m_plainGeometry) {
			if (s.uniqueID() == uniqueID) {
				obj = &s;
				break;
			}
		}
	}
	// now look in geometric networks
	if (obj == nullptr) {
		for (const VICUS::Network & n : m_geometricNetworks) {
			if (n.uniqueID() == uniqueID) {
				obj = &n;
				break;
			}
			obj = n.findChild(uniqueID);
			if (obj != nullptr)
				break;
		}
	}
	if (obj == nullptr)
		throw IBK::Exception(IBK::FormatString("Missing object with unique ID %1.").arg(uniqueID), FUNC_ID);
	return obj;
}


Surface * Project::surfaceByID(unsigned int surfaceID) {
	for (Building & b : m_buildings)
		for (BuildingLevel & bl : b.m_buildingLevels)
			for (Room & r : bl.m_rooms)
				for (Surface & s : r.m_surfaces) {
					if (s.m_id == surfaceID)
						return &s;
				}
	return nullptr;
}


SubSurface * Project::subSurfaceByID(unsigned int surfID) {
	for (Building & b : m_buildings)
		for (BuildingLevel & bl : b.m_buildingLevels)
			for (Room & r : bl.m_rooms)
				for (Surface & s : r.m_surfaces)
					for (const SubSurface & sub : s.subSurfaces())
					{
						if (sub.m_id == surfID)
							return const_cast<SubSurface*>(&sub);
					}
	return nullptr;
}


bool selectionCheck(const VICUS::Object & o, bool takeSelected, bool takeVisible) {
	bool selCheck = takeSelected ? o.m_selected : true;
	bool visCheck = takeVisible ? o.m_visible : true;
	return (selCheck && visCheck);
}

void Project::selectObjects(std::set<const Object*> &selectedObjs, SelectionGroups sg,
							bool takeSelected, bool takeVisible) const
{
	// Buildings
	if (sg & SG_Building) {
		for (const VICUS::Building & b : m_buildings) {
			for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
				for (const VICUS::Room & r : bl.m_rooms) {
					for (const VICUS::Surface & s : r.m_surfaces) {
						if (selectionCheck(s, takeSelected, takeVisible))
							selectedObjs.insert(&s);
						for (const VICUS::SubSurface & sub : s.subSurfaces()) {
							if (selectionCheck(sub, takeSelected, takeVisible))
								selectedObjs.insert(&sub);
						}
					}
					if (selectionCheck(r, takeSelected, takeVisible))
						selectedObjs.insert(&r);
				}
				if (selectionCheck(bl, takeSelected, takeVisible))
					selectedObjs.insert(&bl);
			}
			if (selectionCheck(b, takeSelected, takeVisible))
				selectedObjs.insert(&b);
		}
	}

	// Networks
	if (sg & SG_Network) {
		for (const VICUS::Network & n : m_geometricNetworks) {
			for (const VICUS::NetworkEdge & e : n.m_edges) {
				if (selectionCheck(e, takeSelected, takeVisible))
					selectedObjs.insert(&e);
			}

			for (const VICUS::NetworkNode & nod : n.m_nodes) {
				if (selectionCheck(nod, takeSelected, takeVisible))
					selectedObjs.insert(&nod);
			}
			if (selectionCheck(n, takeSelected, takeVisible))
				selectedObjs.insert(&n);
		}
	}

	// Dumb plain geometry
	if (sg & SG_Obstacle) {
		for (const VICUS::Surface & s : m_plainGeometry) {
			if (selectionCheck(s, takeSelected, takeVisible))
				selectedObjs.insert(&s);
		}
	}
}


bool Project::selectedSurfaces(std::vector<const Surface*> &surfaces, const VICUS::Project::SelectionGroups &sg) const {
	std::set<const Object*> objs;
	selectObjects(objs, sg, true, true);

	// Note: sg = SG_Building will only select surfaces in the building hierarchy
	//       sg = SG_All will also select anonymous surfaces
	//       sg = SG_Network does nothing (network doesn't have any surfaces)

	surfaces.clear();
	for (const Object * o : objs) {
		const Surface * s = dynamic_cast<const Surface *>(o);
		if (s != nullptr)
			surfaces.push_back(s);
	}

	return !surfaces.empty();
}


bool Project::selectedRooms(std::vector<const Room *> & rooms) const {
	std::set<const Object*> objs;
	selectObjects(objs, SG_Building, true, true);

	rooms.clear();
	for (const Object * o : objs) {
		const Room * r = dynamic_cast<const Room *>(o);
		if (r != nullptr)
			rooms.push_back(r);
	}

	return !rooms.empty();
}


IBKMK::Vector3D Project::boundingBox(std::vector<const Surface*> &surfaces,
									 std::vector<const SubSurface*> &subsurfaces,
									 IBKMK::Vector3D &center)
{

	// store selected surfaces
	if ( surfaces.empty() && subsurfaces.empty())
		return IBKMK::Vector3D ( 0,0,0 );

	double maxX = std::numeric_limits<double>::lowest();
	double maxY = std::numeric_limits<double>::lowest();
	double maxZ = std::numeric_limits<double>::lowest();
	double minX = std::numeric_limits<double>::max();
	double minY = std::numeric_limits<double>::max();
	double minZ = std::numeric_limits<double>::max();
	for (const VICUS::Surface *s : surfaces ) {
		for ( IBKMK::Vector3D v : s->polygon3D().vertexes() ) {
			( v.m_x > maxX ) ? maxX = v.m_x : 0;
			( v.m_y > maxY ) ? maxY = v.m_y : 0;
			( v.m_z > maxZ ) ? maxZ = v.m_z : 0;

			( v.m_x < minX ) ? minX = v.m_x : 0;
			( v.m_y < minY ) ? minY = v.m_y : 0;
			( v.m_z < minZ ) ? minZ = v.m_z : 0;
		}
	}
	for (const VICUS::SubSurface *sub : subsurfaces ) {
		const VICUS::Surface *s = dynamic_cast<const VICUS::Surface *>(sub->m_parent);
		for (unsigned int i=0; i<s->subSurfaces().size(); ++i) {
			if (&(s->subSurfaces()[i]) == sub) {
				for ( IBKMK::Vector3D v : s->geometry().holeTriangulationData()[i].m_vertexes ) {
					( v.m_x > maxX ) ? maxX = v.m_x : 0;
					( v.m_y > maxY ) ? maxY = v.m_y : 0;
					( v.m_z > maxZ ) ? maxZ = v.m_z : 0;

					( v.m_x < minX ) ? minX = v.m_x : 0;
					( v.m_y < minY ) ? minY = v.m_y : 0;
					( v.m_z < minZ ) ? minZ = v.m_z : 0;
				}
			}
		}
	}

	double dX = maxX - minX;
	double dY = maxY - minY;
	double dZ = maxZ - minZ;

	center.set( minX + 0.5*dX, minY + 0.5*dY, minZ + 0.5*dZ);

	// set bounding box;
	return IBKMK::Vector3D ( dX, dY, dZ );
}


bool Project::connectSurfaces(double maxDist, double maxAngle, const std::set<const Surface *> & selectedSurfaces,
							  std::vector<ComponentInstance> & newComponentInstances)
{
	// TODO : Dirk, implement algorithm

	return false;
}



void Project::generateNandradProject(NANDRAD::Project & p, QStringList & errorStack) const {
	FUNCID(Project::generateNandradProject);

	// simulation settings
	p.m_simulationParameter = m_simulationParameter;

	// solver parameters
	p.m_solverParameter = m_solverParameter;

	// location settings
	p.m_location = m_location;
	// do we have a climate path?
	if (!m_location.m_climateFilePath.isValid()) {
		errorStack.push_back(tr("A climate data file is needed. Please select a climate data file!"));
		throw IBK::Exception("Error during conversion.", FUNC_ID);
	}

	// directory placeholders
	for (const auto & placeholder : m_placeholders)
		p.m_placeholders[placeholder.first] = placeholder.second;

	// *** building geometry data and databases ***

	generateBuildingProjectDataNeu(p, errorStack);


	// *** generate network data ***

	generateNetworkProjectData(p);


	// *** outputs ***

	// transfer output grids
	p.m_outputs.m_grids = m_outputs.m_grids;

	// transfer options
	p.m_outputs.m_binaryFormat = m_outputs.m_flags[VICUS::Outputs::F_BinaryOutputs];
	p.m_outputs.m_timeUnit = m_outputs.m_timeUnit;

	// transfer pre-defined output definitions
	p.m_outputs.m_definitions = m_outputs.m_definitions;

	// generate output grid, if needed
	std::string refName;
	if (m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].isEnabled() ||
		m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs].isEnabled()) {

		// we need an hourly output grid, look if we have already one defined (should be!)
		int ogInd = -1;
		for (unsigned int i=0; i<p.m_outputs.m_grids.size(); ++i) {
			NANDRAD::OutputGrid & og = p.m_outputs.m_grids[i];
			if (og.m_intervals.size() == 1 &&
				og.m_intervals.back().m_para[NANDRAD::Interval::P_Start].value == 0.0 &&
				og.m_intervals.back().m_para[NANDRAD::Interval::P_End].name.empty() &&
				og.m_intervals.back().m_para[NANDRAD::Interval::P_StepSize].value == 3600.0)
			{
				ogInd = (int)i;
				break;
			}
		}
		// create one, if not yet existing
		if (ogInd == -1) {
			NANDRAD::OutputGrid og;
			og.m_name = refName = tr("Hourly values").toStdString();
			NANDRAD::Interval iv;
			NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_Start, 0);
			NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_StepSize, 1);
			og.m_intervals.push_back(iv);
			p.m_outputs.m_grids.push_back(og);
		}
		else {
			refName = p.m_outputs.m_grids[(unsigned int)ogInd].m_name;
		}

	}


	// default zone outputs
	if (m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].isEnabled()){

		// we already have a name for the output grid, start generating default outputs
		std::string objectListAllZones = tr("All zones").toStdString();
		{
			NANDRAD::OutputDefinition od;
			od.m_gridName = refName;
			od.m_quantity = "AirTemperature";
			od.m_objectListName = objectListAllZones;
			p.m_outputs.m_definitions.push_back(od);
		}

		// and also generate the needed object lists
		{
			NANDRAD::ObjectList ol;
			ol.m_name = objectListAllZones;
			ol.m_filterID.setEncodedString("*");
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			p.m_objectLists.push_back(ol);
		}
	}


	// default network outputs
	if (m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs].isEnabled()) {

		NANDRAD::IDGroup ids;
		ids.m_allIDs = true;
		NANDRAD::ObjectList objList;
		objList.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
		objList.m_filterID = ids;
		objList.m_name = "the network objects";
		p.m_objectLists.push_back(objList);

		NANDRAD::Outputs outputs;
		outputs.m_timeUnit = IBK::Unit("h");
		std::vector<std::string> quantities = {"FluidMassFlux", "OutletNodeTemperature" , "InletNodeTemperature",
											   "FlowElementHeatLoss", "PressureDifference", "TemperatureDifference"};

		for (const std::string &q: quantities){
			NANDRAD::OutputDefinition def;
			def.m_quantity = q;
			def.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;
			def.m_gridName = refName;
			def.m_objectListName = objList.m_name;
			p.m_outputs.m_definitions.push_back(def);
		}
	}


}

std::string createUniqueNandradObjListName(const std::map<std::string, std::vector<unsigned int>> &objListNames, const std::string &name){
	std::string newName = name;
	// if name empty set a name
	if(newName.empty())
		newName = createUniqueNandradObjListName(objListNames, "ZoneTemplate_1");
	else{
		for(const auto &objList : objListNames){
			std::string o = objList.first;
			//check if a obj list with the name exist and if possible increase last counter
			if(o == name){
				size_t pos =name.find_last_of('_');
				if( pos != std::string::npos){
					unsigned int counter;
					try {
						counter = IBK::string2val<unsigned int>(name.substr(pos+1));
						newName = name.substr(0, pos+1) + IBK::val2string(++counter);
					}  catch (...) {
						newName = name +"_1";
					}
					newName = createUniqueNandradObjListName(objListNames, newName);
				}
				else
					newName = createUniqueNandradObjListName(objListNames, name + "_1");

			}
		}
	}
	return newName;





}

std::string createUniqueNandradObjListAndName(const std::string &name,
										   const std::vector<unsigned int> &roomIds, NANDRAD::Project &p,
										   const NANDRAD::ModelInputReference::referenceType_t &type){
	//create an obj list
	NANDRAD::ObjectList objList;
	for(unsigned int id : roomIds)
		objList.m_filterID.m_ids.insert(id);
	objList.m_referenceType = type;

	std::set<std::string> objListNames;

	for(const NANDRAD::ObjectList &objL : p.m_objectLists){
		if(objList.m_filterID == objL.m_filterID &&
				objList.m_referenceType == objL.m_referenceType)
			return objL.m_name; //found same objList return name only
		objListNames.insert(objL.m_name);
	}

	//no equal object list was found
	//create a new name for these object list

	size_t sizeList = objListNames.size();
	std::string newName = name;
	objListNames.insert(newName);

	while (sizeList == objListNames.size()) {
		size_t pos = newName.find_last_of("_");
		if(pos == std::string::npos)
			newName += "_1";
		else{
			unsigned int val;
			try {
				val = IBK::string2val<unsigned int>(newName.substr(pos+1));
				newName = newName.substr(0, pos) + IBK::val2string(++val);
			}  catch (...) {
				newName += "_1";
			}
		}
		objListNames.insert(newName);
	}

	//add objList to NANDRAD project
	objList.m_name = newName;
	p.m_objectLists.push_back(objList);

	return newName;

	/*
	//add all ids to obj list
	for(unsigned int rId : areaToRoomIdsObj.second)
		objList.m_filterID.m_ids.insert(rId);
	objList.m_name = uniqueName;
	objList.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	//look if obj list already exists -> take this
	bool objListExist=false;
	for(NANDRAD::ObjectList &objListNAN : p.m_objectLists){
		if(objList.m_filterID == objListNAN.m_filterID &&
				objList.m_referenceType == objListNAN.m_referenceType){
			mapObjListNameToRoomIds.erase(mapObjListNameToRoomIds.find(uniqueName));
			uniqueName = objListNAN.m_name;
			objList.m_name = uniqueName;
			mapObjListNameToRoomIds[uniqueName]=areaToRoomIdsObj.second;
			objListExist = true;
			break;
		}
	}
	//only add new obj lists
	if(!objListExist)
		p.m_objectLists.push_back(objList);
	*/
}

/* Returns the possible merged day types. If no merge is possible returns dts. */
std::vector<NANDRAD::Schedule::ScheduledDayType> mergeDayType(const std::vector<int> &dts);





std::string Project::getRoomNameById(unsigned int id) const {
	for (const VICUS::Building & b : m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				if(r.m_id == id)
					return r.m_displayName.toStdString();
			}
		}
	}
	return "";
}


void Project::exportSubSurfaces(QStringList & errorStack, const std::vector<VICUS::SubSurface> &subSurfs, std::vector<IdMap> &idMaps,
								const VICUS::ComponentInstance & ci, NANDRAD::ConstructionInstance &cinst) const{

	double embArea = 0;
	// get area of surface
	double areaA = ci.m_sideASurface->geometry().area();

	for(const SubSurface &ss : subSurfs){
		NANDRAD::EmbeddedObject emb;
		emb.m_id = uniqueIdWithPredef2(ConstructionInstance, 1, idMaps, true);
		//get surface are of emb. obj.
		NANDRAD::KeywordList::setParameter(emb.m_para, "EmbeddedObject::para_t",
										   NANDRAD::EmbeddedObject::P_Area, ss.m_polygon2D.area());
		embArea += ss.m_polygon2D.area();
		if(embArea > areaA){
			errorStack << tr("Area of sub surfaces is bigger than area of parent surface #%1, '%2'.").arg(ci.m_sideASurface->m_id)
						  .arg(ci.m_sideASurface->m_displayName);
			continue;
		}
		emb.m_displayName = ss.m_displayName.toStdString();

		unsigned int subSurfaceComponentId = VICUS::INVALID_ID;
		//find sub surface component instance
		for(const VICUS::SubSurfaceComponentInstance &ssci : m_subSurfaceComponentInstances){
			if(ssci.m_idSideASurface == ss.m_id || ssci.m_idSideBSurface == ss.m_id){
				subSurfaceComponentId = ssci.m_idSubSurfaceComponent;

				break;
			}
		}
		if(subSurfaceComponentId == VICUS::INVALID_ID){
			errorStack << tr("No component was assigned to the SubSurface #%1 with name '%2'. Sub surface is not exported.")
						  .arg(ss.m_id).arg(ss.m_displayName);
			continue;
		}
		bool foundSubSurfComp = false;
		//search for sub surface component
		for(const VICUS::SubSurfaceComponent &ssc : m_embeddedDB.m_subSurfaceComponents){
			if(ssc.m_id == subSurfaceComponentId){
				foundSubSurfComp = true;
				//only simple windows are supported now
				if(ssc.m_type == VICUS::SubSurfaceComponent::CT_Window){
					//ssc.m_idConstructionType
					if(ssc.m_idWindow == VICUS::INVALID_ID){
						errorStack << tr("The sub surface component with #%1 and name '%2' has no valid window id. Sub surface #%3 is not exported.")
									  .arg(ssc.m_id).arg(QString::fromStdString(ssc.m_displayName.string())).arg(ss.m_id);
						break;
					}
					bool foundWindow =false;
					//search for the window
					for(const VICUS::Window &winV : m_embeddedDB.m_windows){
						if(winV.m_id == ssc.m_idWindow){
							if(winV.m_idGlazingSystem == VICUS::INVALID_ID){
								errorStack << tr("The window with #%1 and name '%2' has no valid glazing system. Sub surface #%3 is not exported.")
											  .arg(winV.m_id).arg(QString::fromStdString(winV.m_displayName.string())).arg(ss.m_id);
								break;
							}
							//save id for glazing system later
							emb.m_window.m_glazingSystemId = uniqueIdWithPredef2(Window, winV.m_idGlazingSystem, idMaps, true);
							foundWindow = true;
							break;
						}
					}
				}
				else{
					//TODO Dirk Fehler werfen
					errorStack << tr("The sub surface component with #%1 and name '%2' is not supported by the export.")
								  .arg(ssc.m_id).arg(QString::fromStdString(ssc.m_displayName.string()));
					continue;
				}
			}
		}
		if(!foundSubSurfComp){
			errorStack << tr("No component was found for the sub surface with #%1 and name '%2'. No export of this sub surface.")
						  .arg(ss.m_id).arg(ss.m_displayName);
			continue;
		}
		//add emb. obj. to nandrad project
		cinst.m_embeddedObjects.push_back(emb);
		//TODO Dirk Frame einbauen sobald verfügbar
		//TODO Dirk Divider einbauen sobald verfügbar
	}
}


bool Project::createThermostat(const VICUS::ZoneControlThermostat * thermo,
							   const std::string &zoneTemplateDisplayName,
							   NANDRAD::Project &p,
							   std::vector<IdMap> &idMaps,
							   const std::vector<unsigned int> &roomIds,
							   std::string &thermoObjListName) const{
	FUNCID(Project::createThermostat);
	QStringList errors;
	if(thermo == nullptr)
		return false;
	//now we found also an thermostat
	//first create an obj list for thermostat
	NANDRAD::Thermostat thermoN;
	thermoN.m_displayName = zoneTemplateDisplayName;
	thermoN.m_id = uniqueIdWithPredef2(Profile, 1, idMaps, true);
	thermoN.m_modelType = NANDRAD::Thermostat::MT_Scheduled;
	thermoN.m_zoneObjectList = createUniqueNandradObjListAndName(zoneTemplateDisplayName, roomIds, p,
																 NANDRAD::ModelInputReference::MRT_ZONE);
	thermoObjListName = thermoN.m_zoneObjectList;

	NANDRAD::KeywordList::setParameter(thermoN.m_para, "Thermostat::para_t",
									   NANDRAD::Thermostat::P_TemperatureTolerance,
									   thermo->m_para[VICUS::ZoneControlThermostat::P_Tolerance].get_value("K"));
	//add setpoint schedule in schedules
	unsigned int heatingSchedId = thermo->m_idHeatingSetpointSchedule;
	unsigned int coolingSchedId = thermo->m_idCoolingSetpointSchedule;

	std::string heatSchedParaName = NANDRAD::KeywordList::Keyword("Thermostat::para_t", NANDRAD::Thermostat::P_HeatingSetpoint);
	heatSchedParaName += "Schedule [C]";
	std::string coolSchedParaName = NANDRAD::KeywordList::Keyword("Thermostat::para_t", NANDRAD::Thermostat::P_CoolingSetpoint);
	coolSchedParaName += "Schedule [C]";
	bool foundHeatOrCoolSched = false;
	if(heatingSchedId != VICUS::INVALID_ID){
		//check if schedule is in data base
		const VICUS::Schedule *heatingSched = VICUS::element(m_embeddedDB.m_schedules, heatingSchedId);
		if(heatingSched != nullptr){
			foundHeatOrCoolSched = true;
			if(!heatingSched->isValid()){
				IBK::Exception(IBK::FormatString("Heating schedule with id %1 and name '%2' is not in valid.")
							   .arg(heatingSchedId).arg(heatingSched->m_displayName.string()), FUNC_ID);
				errors << tr("Heating schedule with id %1 and name '%2' is not in valid.")
						  .arg(heatingSchedId).arg(QString::fromUtf8(heatingSched->m_displayName.string().c_str()));
			}
			addVicusScheduleToNandradProject(*heatingSched, heatSchedParaName, p, thermoN.m_zoneObjectList);
		}
	}
	else{
		VICUS::Schedule *hs = new VICUS::Schedule();
		//create a heating schedule with a very low heating value
		hs->createConstSchedule(-100);
		addVicusScheduleToNandradProject(*hs, heatSchedParaName, p, thermoN.m_zoneObjectList);
	}
	if(coolingSchedId != VICUS::INVALID_ID){
		//check if schedule is in data base
		const VICUS::Schedule *coolingSched = VICUS::element(m_embeddedDB.m_schedules, coolingSchedId);
		if(coolingSched != nullptr){
			foundHeatOrCoolSched = true;
			if(!coolingSched->isValid()){
				IBK::Exception(IBK::FormatString("Cooling schedule with id %1 and name '%2' is not in valid.")
							   .arg(coolingSchedId).arg(coolingSched->m_displayName.string()), FUNC_ID);
				errors << tr("Cooling schedule with id %1 and name '%2' is not in valid.")
						  .arg(coolingSchedId).arg(QString::fromUtf8(coolingSched->m_displayName.string().c_str()));
			}
			addVicusScheduleToNandradProject(*coolingSched, coolSchedParaName, p, thermoN.m_zoneObjectList);
		}
	}
	else{
		VICUS::Schedule *cs = new VICUS::Schedule();
		//create a cooling schedule with a very high cooling value
		cs->createConstSchedule(200);
		addVicusScheduleToNandradProject(*cs, coolSchedParaName, p, thermoN.m_zoneObjectList);
	}

	if(!foundHeatOrCoolSched){
		errors << tr("No valid heating or cooling schedules were found. "
							"Error in Thermostat #%1 with name '%2'")
				  .arg(thermo->m_id).arg(QString::fromUtf8(thermo->m_displayName.string().c_str()));
		throw IBK::Exception(IBK::FormatString("No valid heating or cooling schedules were found. "
							"Error in Thermostat #%1 with name '%2'").arg(thermo->m_id).arg(thermo->m_displayName.string()), FUNC_ID);
	}

	//we dont support RadiantTemperature
	thermoN.m_temperatureType =  thermo->m_controlValue == VICUS::ZoneControlThermostat::CV_AirTemperature ?
				NANDRAD::Thermostat::TT_AirTemperature :
				NANDRAD::Thermostat::TT_OperativeTemperature;

	//for now only provide p controller
	thermoN.m_controllerType = thermo->m_controllerType == VICUS::ZoneControlThermostat::CT_Analog ?
				NANDRAD::Thermostat::CT_Analog : NANDRAD::Thermostat::CT_Digital;

	//add thermostat to NANDRAD project
	p.m_models.m_thermostats.push_back(thermoN);

	return true;
}

bool Project::createIdealHeatingCooling(const VICUS::ZoneIdealHeatingCooling * ideal,
							   NANDRAD::Project &p,
							   std::vector<IdMap> &idMaps,
							   const std::string &objListNameThermostat) const{

	if(ideal == nullptr)
		return false;

	//create an ideal heating cooling element
	NANDRAD::IdealHeatingCoolingModel idealN;
	idealN.m_id = uniqueIdWithPredef2(Profile, 1, idMaps, true);
	idealN.m_zoneObjectList = objListNameThermostat;

	// fill parameter
	double heatVal = ideal->m_para[VICUS::ZoneIdealHeatingCooling::P_HeatingLimit].empty() ?
				10000 : ideal->m_para[VICUS::ZoneIdealHeatingCooling::P_HeatingLimit].get_value("W/m2");

	NANDRAD::KeywordList::setParameter(idealN.m_para, "IdealHeatingCoolingModel::para_t",
									   NANDRAD::IdealHeatingCoolingModel::P_MaxHeatingPowerPerArea,
									   heatVal);
	double coolVal = ideal->m_para[VICUS::ZoneIdealHeatingCooling::P_CoolingLimit].empty() ?
				10000 : ideal->m_para[VICUS::ZoneIdealHeatingCooling::P_CoolingLimit].get_value("W/m2");

	NANDRAD::KeywordList::setParameter(idealN.m_para, "IdealHeatingCoolingModel::para_t",
									   NANDRAD::IdealHeatingCoolingModel::P_MaxCoolingPowerPerArea,
									   coolVal);

	//add ideal to project
	p.m_models.m_idealHeatingCoolingModels.push_back(idealN);
	return true;
}

// assume that setpoints hold 4 values lower and upper value for heating (value 1. and 2.) and cooling (value 3. and 4.)
bool calculateSupplyTemperature(const std::vector<double> &supplySetpoints,const std::vector<double> &outdoorSetpoints,
								const std::vector<double> &outdoorTemperatureSpline, std::vector<double> &supplyTemperature){
	if(supplySetpoints.size() != outdoorSetpoints.size() && supplySetpoints.size() == 4)
		return false;


	double lowerSupplyHeatLimit = std::min<double>(supplySetpoints[0], supplySetpoints[1]);
	double upperSupplyHeatLimit = std::max<double>(supplySetpoints[0], supplySetpoints[1]);
	double lowerSupplyCoolLimit = std::min<double>(supplySetpoints[2], supplySetpoints[3]);
	double upperSupplyCoolLimit = std::max<double>(supplySetpoints[2], supplySetpoints[3]);

	double lowerOutHeatLimit = std::min<double>(outdoorSetpoints[0], outdoorSetpoints[1]);
	double upperOutHeatLimit = std::max<double>(outdoorSetpoints[0], outdoorSetpoints[1]);
	double lowerOutCoolLimit = std::min<double>(outdoorSetpoints[2], outdoorSetpoints[3]);
	double upperOutCoolLimit = std::max<double>(outdoorSetpoints[2], outdoorSetpoints[3]);

	if(upperOutHeatLimit > lowerOutCoolLimit)
		return false;

	supplyTemperature.clear();

	double deltaX = outdoorSetpoints[0] - outdoorSetpoints[1];
	double mHeat = 0;
	if(deltaX != 0)
		mHeat = (supplySetpoints[0] - supplySetpoints[1]) / deltaX;
	double nHeat = mHeat * supplySetpoints[0] - outdoorSetpoints[0];

	deltaX = outdoorSetpoints[2] - outdoorSetpoints[3];
	double mCool = 0;
	if(deltaX != 0)
		mCool = (supplySetpoints[2] - supplySetpoints[3]) / deltaX;
	double nCool = mCool * supplySetpoints[2] - outdoorSetpoints[3];

	for(unsigned int i=0; i<outdoorTemperatureSpline.size(); ++i){
		double tOut = outdoorTemperatureSpline[i];
		double tSupply;
		if(tOut <= lowerOutHeatLimit)
			tSupply = lowerSupplyHeatLimit;
		else if(tOut > lowerOutHeatLimit && tOut <= upperOutHeatLimit)
			tSupply = mHeat * tOut + nHeat;
		else if(tOut > upperOutHeatLimit && tOut < lowerOutCoolLimit)
			tSupply = upperSupplyHeatLimit;
		else if(tOut >= lowerOutCoolLimit && tOut < upperOutCoolLimit)
			tSupply = mCool * tOut + nCool;
		else
			tSupply = upperSupplyCoolLimit;
		supplyTemperature.push_back(tSupply);
	}
	return true;
}

//#define test01
void Project::generateBuildingProjectData(NANDRAD::Project & p) const {
	FUNCID(Project::generateBuildingProjectData);
	// used to generate unique interface IDs
	unsigned int interfaceID = 1;

	// TODO : Andreas, for now, we generate interface IDs on the fly, which means they might be different when NANDRAD
	//        file is generated with small changes in the project. This will make it difficult to assign specific
	//        id associations with interfaces (once needed, maybe in FMUs?), so we may need to add interface IDs to
	//        the VICUS::ComponentInstance data structure.

	// we process all zones and buildings and create NANDRAD project data
	// we also transfer all database components

	QStringList errorStack;

	//zone template id
	//floor area (rounded)
	//vector of room ids
	std::map<unsigned int, std::map < double, std::vector< unsigned int> > > zoneTemplateIdToAreaToRoomIds;
	//container for unique ids
	std::vector<unsigned int>							allModelIds;
	std::map<unsigned int, unsigned int>				vicusToNandradIds;

	std::vector<IdMap>									m_idMaps{NUM_IdSpaces};

	std::map<unsigned int, unsigned int>				vicusToNandradConstructionIds;

	// *** m_zones ***

	for (const VICUS::Building & b : m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				// first create a NANDRAD zone for the room
				NANDRAD::Zone z;
				z.m_id = uniqueIdWithPredef2(Zone, r.m_id, m_idMaps);

				//unsigned int testId = uniqueIdWithPredef(allModelIds, r.m_id, vicusToNandradIds);
				z.m_displayName = r.m_displayName.toStdString();
				// Note: in the code below we expect the parameter's base units to be the same as the default unit for the
				//       populated parameters

				// TODO : what if we do not have an area or a zone volume, yet?
				NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Area, r.m_para[VICUS::Room::P_Area].value);
				NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Volume, r.m_para[VICUS::Room::P_Volume].value);

				// for now, zones are always active
				z.m_type = NANDRAD::Zone::ZT_Active;
				// finally append zone
				p.m_zones.push_back(z);

				//if zone template id is invalid skip it
				if ( r.m_idZoneTemplate != VICUS::INVALID_ID ){
					//check if zone template id exists in map
					bool wasFound =false;
					if(zoneTemplateIdToAreaToRoomIds.find(r.m_idZoneTemplate) != zoneTemplateIdToAreaToRoomIds.end()){
						//search for a equal floor area in this value map
						for(auto & obj : zoneTemplateIdToAreaToRoomIds[r.m_idZoneTemplate]){
							if(IBK::nearly_equal<1>(obj.first,r.m_para[VICUS::Room::P_Area].get_value("m2"))){
								obj.second.push_back(r.m_id);
								wasFound =true;
								break;
							}
						}

					}
					if(!wasFound)
						zoneTemplateIdToAreaToRoomIds[r.m_idZoneTemplate][r.m_para[VICUS::Room::P_Area].get_value("m2")].push_back(r.m_id);
				}

				//set a name for the NANDRAD objlist
				//if(mapZoneTemplateIdToNandradObjListName.find(r.m_idZoneTemplate) == mapZoneTemplateIdToNandradObjListName.end())
					//check name already exist in all NANDRAD objLists
					//mapZoneTemplateIdToNandradObjListName[r.m_idZoneTemplate] = getNandradObjListName(p, r.m_displayName.toStdString());
			}
		}
	}


	// ############################## Zone Templates


	//Check all models for validity


	//holds a bool for all sub zone templates if the model already assigned in NANDRAD project
	struct ztBool{
		std::vector<unsigned int>	m_subTemplateId;
	};

	//name of obj list
	//vector of assigned room ids to obj list
	std::map<std::string, std::vector<unsigned int>>	mapObjListNameToRoomIds;

	std::vector<ztBool>	ztBools(zoneTemplateIdToAreaToRoomIds.size());
	unsigned int counter = 0;

	//create a zero-value schedule for internal loads later
	VICUS::Schedule zeroValueSched;
	zeroValueSched.createConstSchedule();

	//key zone id to vector of component instance pointer
	struct DataSurfaceHeating{
		unsigned int					m_zoneId;

		struct Data{
			std::vector<unsigned int>	m_contructionInstanceIds;
			std::map<int, std::vector<unsigned int>> m_areaToIds;
		};

		std::map<unsigned int, Data>	m_idSurfaceSystemToIds;
	};

	std::vector<DataSurfaceHeating>	m_surfaceSystems;


	// *** m_constructionInstances ***


	// now process all components and generate construction instances
	for (const VICUS::ComponentInstance & ci : m_componentInstances) {
		// Note: component ID may be invalid or component may have been deleted from DB already
		const VICUS::Component * comp = VICUS::element(m_embeddedDB.m_components, ci.m_idComponent);
		if (comp == nullptr) {
			errorStack.push_back( tr("Component ID #%1 is referenced from component instance with id #%2, but there is no such component.")
							 .arg(ci.m_idComponent).arg(ci.m_id));
			throw IBK::Exception("Conversion error.", FUNC_ID);
		}

		// now generate a construction instance
		NANDRAD::ConstructionInstance cinst;
		//cinst.m_id = uniqueIdWithPredef(allModelIds, ci.m_id, vicusToNandradIds);
		cinst.m_id = uniqueIdWithPredef2(ConstructionInstance, ci.m_id, m_idMaps, true);

		// store reference to construction type (i.e. to be generated from component)
		// std::vector<unsigned int>	m_contructionInstanceIds;
		cinst.m_constructionTypeId = uniqueIdWithPredef2(Construction, comp->m_idConstruction, m_idMaps);

		// set construction instance parameters, area, orientation etc.

		const double SAME_DISTANCE_PARAMETER_ABSTOL = 1e-4;
		double area = 0;

		bool bothSidesHasSurfaces = false;
		// we have either one or two surfaces associated
		if (ci.m_sideASurface != nullptr) {
			// get area of surface A
			area = ci.m_sideASurface->geometry().area();
			// do we have surfaces at both sides?
			if (ci.m_sideBSurface != nullptr) {
				// have both
				bothSidesHasSurfaces = true;
				double areaB = ci.m_sideBSurface->geometry().area();
				// check if both areas are approximately the same
				if (std::fabs(area - areaB) > SAME_DISTANCE_PARAMETER_ABSTOL) {
					errorStack.push_back(tr("Component/construction #%1 references surfaces #%2 and #%3, with mismatching "
						   "areas %3 and %4 m2.")
								  .arg(ci.m_id).arg(ci.m_idSideASurface).arg(ci.m_idSideBSurface)
								  .arg(area).arg(areaB));
					throw IBK::Exception("Conversion error.", FUNC_ID);
				}

				// if we have both surfaces, then this is an internal construction and orientation/inclination are
				// not important and we just don't set these

				/// TODO Dirk : do we need to also store a displayname for each component instance/construction instance?
				///             We could also name internal walls automatically using zone names, such as
				///				"Wall between 'Bath' and 'Kitchen'".
				cinst.m_displayName = tr("Internal wall between surfaces '#%1' and '#%2'")
						.arg(ci.m_sideASurface->m_displayName).arg(ci.m_sideBSurface->m_displayName).toStdString();
			}
			else {

				// we only have side A, take orientation and inclination from side A
				const VICUS::Surface * s = ci.m_sideASurface;

				// set parameters
				NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Inclination, s->geometry().inclination());
				NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Orientation, s->geometry().orientation());

				cinst.m_displayName = ci.m_sideASurface->m_displayName.toStdString();
			}
			// set area parameter (computed from side A, but if side B is given as well, the area is the same
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Area, area);

			//for the first time we support only sub surfaces to outside air
			if(!bothSidesHasSurfaces){
				// sub surface
				const std::vector<SubSurface> & subSurfs = ci.m_sideASurface->subSurfaces();
				if(subSurfs.size()>0){
					//we have sub surfaces
					exportSubSurfaces(errorStack, subSurfs, m_idMaps, ci, cinst);
				}
			}
		}
		else {
			// we must have a side B surface, otherwise this is an invalid component instance
//			if (ci.m_sideBSurface == nullptr)
//				throw ConversionError(ConversionError::ET_InvalidID,
//									  tr("Component instance #%1 does neither reference a valid surface on side A nor on side B.")
//									  .arg(ci.m_id));

			const VICUS::Surface * s = ci.m_sideBSurface;

			// set parameters
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Inclination, s->geometry().inclination());
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Orientation, s->geometry().orientation());

			// set area parameter
			area = ci.m_sideBSurface->geometry().area();
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Area, area);

			cinst.m_displayName = ci.m_sideBSurface->m_displayName.toStdString();
			// sub surface
			const std::vector<SubSurface> & subSurfs = ci.m_sideBSurface->subSurfaces();
			if(subSurfs.size()>0){
				//we have sub surfaces
				exportSubSurfaces(errorStack, subSurfs, m_idMaps, ci, cinst);
			}
		}


		// now generate interfaces
		cinst.m_interfaceA = generateInterface(ci, comp->m_idSideABoundaryCondition, m_idMaps, ++interfaceID);
		cinst.m_interfaceB = generateInterface(ci, comp->m_idSideBBoundaryCondition, m_idMaps, ++interfaceID, false);

		// add to list of construction instances
		p.m_constructionInstances.push_back(cinst);
		//get an area check for surface heating systems
		if(area <= 1e-4){
			//todo errorstack
			continue;
		}
		//create surface heating system data
		if(ci.m_idSurfaceHeatingControlZone != INVALID_ID && ci.m_idSurfaceHeating != INVALID_ID){
			unsigned int zoneId = INVALID_ID;

			int posDataSH = -1;
			for(unsigned int i=0; i<m_surfaceSystems.size(); ++i){
				if(m_surfaceSystems[i].m_zoneId == ci.m_idSurfaceHeatingControlZone){
					//found zone id
					posDataSH = (int)i;
					zoneId = ci.m_idSurfaceHeatingControlZone;
					break;
				}
			}

			if(zoneId != INVALID_ID){
				//add new element
				if(posDataSH == -1){
					m_surfaceSystems.push_back(DataSurfaceHeating());
					posDataSH = m_surfaceSystems.size()-1;
					m_surfaceSystems.back().m_zoneId = zoneId;
				}
				DataSurfaceHeating &dsh = m_surfaceSystems[posDataSH];
				dsh.m_idSurfaceSystemToIds[ci.m_idSurfaceHeating].m_contructionInstanceIds.push_back(cinst.m_id);
				dsh.m_idSurfaceSystemToIds[ci.m_idSurfaceHeating].m_areaToIds[area*10000].push_back(cinst.m_id);
			}
			else{
				//todo errorstack füllen
			}
		}

	}

	//search all internal surface heating/cooling systems
	for (const VICUS::ComponentInstance & ci : m_componentInstances) {
		// Note: component ID may be invalid or component may have been deleted from DB already
		const VICUS::Component * comp = VICUS::element(m_embeddedDB.m_components, ci.m_idComponent);
//		if (comp == nullptr){
//			throw ConversionError(ConversionError::ET_InvalidID,
//				tr("Component ID #%1 is referenced from component instance with id #%2, but there is no such component.")
//							 .arg(ci.m_componentID).arg(ci.m_id));
//			continue;
//		}
	}

	// Create a outdoor air temperature data line for calculate the supply fluid temperature later
	IBK::LinearSpline outdoorTemp;
	//first vector -> timepoints; second temperature in C
	outdoorTemp.setValues(std::vector<double>{0,100,200,300,400,8760}, std::vector<double>{-10,-10,0,0,15,30});
	outdoorTemp.m_extrapolationMethod = IBK::LinearSpline::EM_Constant;


	//loop for zone templates
	for (const std::pair<unsigned int,std::map< double,  std::vector<unsigned int>>> &ob : zoneTemplateIdToAreaToRoomIds) {
		//take zone template

		const VICUS::ZoneTemplate *zt = VICUS::element(m_embeddedDB.m_zoneTemplates, ob.first);
		//const VICUS::ZoneTemplate *zt = dynamic_cast<const VICUS::ZoneTemplate *>(db.m_zoneTemplates[(unsigned int) ob.first ]);

		if ( zt == nullptr )
			throw IBK::Exception(IBK::FormatString("Zone Template with ID %1 does not exist in database.").arg(ob.first), FUNC_ID);

		//first look for internal loads
		//check if there are parametrization for area dependet values
		bool isAreaIndepent=false;

		//init all vals in vectors with invalid id
		ztBools[counter].m_subTemplateId.resize(VICUS::ZoneTemplate::NUM_ST, VICUS::INVALID_ID);

		//set of available internal load enums

		std::set<VICUS::ZoneTemplate::SubTemplateType> intLoadEnums;

		if(zt->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment] != VICUS::INVALID_ID ||
				zt->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] != VICUS::INVALID_ID ||
				zt->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadLighting] != VICUS::INVALID_ID)
			intLoadEnums = std::set<VICUS::ZoneTemplate::SubTemplateType>{VICUS::ZoneTemplate::ST_IntLoadEquipment,
																	VICUS::ZoneTemplate::ST_IntLoadPerson,
																   VICUS::ZoneTemplate::ST_IntLoadLighting};

		//check all internal loads for area depending
		for(auto e : intLoadEnums){
			//get id of int load template
			unsigned int idSubTemp = zt->m_idReferences[e];
			//save in ztBools
			ztBools[counter].m_subTemplateId[e] = idSubTemp;
			if(e == VICUS::ZoneTemplate::ST_IntLoadPerson){
				const VICUS::InternalLoad *intLoadModel = VICUS::element(m_embeddedDB.m_internalLoads, idSubTemp);
				if(intLoadModel != nullptr && intLoadModel->m_personCountMethod ==  VICUS::InternalLoad::PCM_PersonCount){
					isAreaIndepent = true;
				}
			}
			else{
				const VICUS::InternalLoad *intLoadModel = VICUS::element(m_embeddedDB.m_internalLoads, idSubTemp);
				if(intLoadModel != nullptr && intLoadModel->m_powerMethod == VICUS::InternalLoad::PM_Power){
					isAreaIndepent=true;
				}
			}
		}

		//now we have a separate template model for each zt and AREA

		std::map<double, std::vector<unsigned int>> mapAreaToRoomIds;
		std::vector<unsigned int> allRoomIdsForThisZt;				//this is for area sub template which have no area effects like infiltration

		//create one entry in the mapAreaToToomIds
		for(std::pair<double, std::vector<unsigned int>> e : ob.second)
			allRoomIdsForThisZt.insert(allRoomIdsForThisZt.end(), e.second.begin(), e.second.end());

		if(isAreaIndepent)
			mapAreaToRoomIds = ob.second;
		else
			mapAreaToRoomIds[1] = allRoomIdsForThisZt;

		if(!intLoadEnums.empty()){

			// *** predefinitions
			std::map<VICUS::ZoneTemplate::SubTemplateType, std::string>	subTempTypeToNameWithUnit;
			subTempTypeToNameWithUnit[VICUS::ZoneTemplate::ST_IntLoadPerson] =
					(std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_PersonHeatLoadPerArea)
					+ "Schedule [W/m2]";
			subTempTypeToNameWithUnit[VICUS::ZoneTemplate::ST_IntLoadEquipment] =
					(std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_EquipmentHeatLoadPerArea)
					+ "Schedule [W/m2]";
			subTempTypeToNameWithUnit[VICUS::ZoneTemplate::ST_IntLoadLighting] =
					(std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_LightingHeatLoadPerArea)
					+ "Schedule [W/m2]";

			//create all obj list with room ids
			for(std::pair<double, std::vector<unsigned int>> areaToRoomIdsObj : mapAreaToRoomIds){

				//only accept models with floor area > 0 m2
				if(areaToRoomIdsObj.first<=0)
					throw IBK::Exception(IBK::FormatString("The ground floor area of room with id %1 and name '%1'"
															" is <=0 m2 ").arg(areaToRoomIdsObj.second.front()).arg(getRoomNameById(areaToRoomIdsObj.second.front())), FUNC_ID);
				//create an obj list
				//std::string uniqueName = createUniqueNandradObjListName(mapObjListNameToRoomIds, zt->m_displayName.string());
				std::string uniqueName = createUniqueNandradObjListAndName(zt->m_displayName.string(), areaToRoomIdsObj.second, p, NANDRAD::ModelInputReference::MRT_ZONE);
				mapObjListNameToRoomIds[uniqueName] = areaToRoomIdsObj.second;

				//now create NANDRAD models
				NANDRAD::InternalLoadsModel intLoad;
				intLoad.m_displayName = zt->m_displayName.string();
				intLoad.m_modelType = NANDRAD::InternalLoadsModel::MT_Scheduled;
				intLoad.m_zoneObjectList = uniqueName;
				intLoad.m_id = uniqueIdWithPredef2(Profile, 1, m_idMaps, true); //VICUS::Project::uniqueId<unsigned int>(allModelIds);
				//save all model ids
				//allModelIds.push_back(intLoad.m_id);

				NANDRAD::KeywordList::setParameter(intLoad.m_para, "InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_EquipmentRadiationFraction, 0);
				NANDRAD::KeywordList::setParameter(intLoad.m_para, "InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_LightingRadiationFraction, 0);
				NANDRAD::KeywordList::setParameter(intLoad.m_para, "InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_PersonRadiationFraction, 0);

				//first create for all internal loads a zero-value schedule


				enum posIntLoad{
					P_Person,
					P_Electric,
					P_Lighting,
					P_Other,
					NUM_P
				};
				std::vector<VICUS::Schedule> intLoadScheds(NUM_P);

				for(unsigned int e : intLoadEnums){
					// wo werden die interpolationsmethode constant oder linear gesetzt?
					//FUCK wir können nur konstant oder linear für alle
					//wie machen wir das jetzt?
					//und das beste in den personen können sogar 2 unterschiedliche interpolationsmethoden drin liegen
					//das müssen die eingabemaske prüfen
					//auch schon das zonetemplate muss da ne prüfung machen
					//damit nicht ein electr. und personen unterschiedliche interpolationsmethoden haben
					//uiuiui

					switch (e) {
						case VICUS::ZoneTemplate::ST_IntLoadPerson:{
							//get schedule ids
							//check valid status
							const VICUS::InternalLoad *pers = VICUS::element(m_embeddedDB.m_internalLoads, ztBools[counter].m_subTemplateId[e]);
							if(pers == nullptr)
								addVicusScheduleToNandradProject(zeroValueSched,subTempTypeToNameWithUnit[VICUS::ZoneTemplate::SubTemplateType(e)],
									p, uniqueName);
							else{
								unsigned int schedId = pers->m_idActivitySchedule;
								const VICUS::Schedule *schedAct = VICUS::element(m_embeddedDB.m_schedules, schedId);
								if(schedAct == nullptr)
									IBK::Exception(IBK::FormatString("Activity schedule with id %1 is not in database.")
												   .arg(schedId), FUNC_ID);
								if(!schedAct->isValid())
									IBK::Exception(IBK::FormatString("Activity schedule with id %1 and name '%2' is not in valid.")
												   .arg(schedId).arg(schedAct->m_displayName.string()), FUNC_ID);

								unsigned int schedId2 = pers->m_idOccupancySchedule;
								const VICUS::Schedule *schedOcc = VICUS::element(m_embeddedDB.m_schedules, schedId2);
								//const VICUS::Schedule *schedOcc = const_cast<VICUS::Schedule *>(db.m_schedules[schedId2]);
								if(schedOcc == nullptr)
									IBK::Exception(IBK::FormatString("Occupancy schedule with id %1 is not in database.")
												   .arg(schedId2), FUNC_ID);
								if(!schedOcc->isValid())
									IBK::Exception(IBK::FormatString("Occupancy schedule with id %1 and name '%2' is not in valid.")
												   .arg(schedId2).arg(schedOcc->m_displayName.string()), FUNC_ID);

								posIntLoad enum1 = P_Person;
								VICUS::Schedule &intLoadSched = intLoadScheds[enum1];
								//multiply the two schedules and add this to vector
								intLoadSched = schedAct->multiply(*schedOcc);
								//id is not used
								intLoadSched.m_id = uniqueIdWithPredef2(Profile, 1, m_idMaps); //VICUS::Project::uniqueId<unsigned int>(allModelIds);

								//multiply sched and constant val

								switch(pers->m_personCountMethod){
									case VICUS::InternalLoad::PCM_PersonPerArea:
										intLoadSched = intLoadSched.multiply(pers->m_para[VICUS::InternalLoad::P_PersonPerArea].get_value("Person/m2"));
									break;
									case VICUS::InternalLoad::PCM_AreaPerPerson: {
										double val = pers->m_para[VICUS::InternalLoad::P_AreaPerPerson].get_value("m2/Person");
										//if value is zero do nothing
										if(val>0)
											intLoadSched = intLoadSched.multiply(1/val);
									} break;
									case VICUS::InternalLoad::PCM_PersonCount:{
										intLoadSched = intLoadSched.multiply(pers->m_para[VICUS::InternalLoad::P_PersonCount].get_value()/areaToRoomIdsObj.first);
									} break;
									case VICUS::InternalLoad::NUM_PCM:
									break;
								}
								addVicusScheduleToNandradProject(intLoadSched,subTempTypeToNameWithUnit[VICUS::ZoneTemplate::SubTemplateType(e)],
									p, uniqueName);

								//override zero values in NANDRAD model
								NANDRAD::KeywordList::setParameter(intLoad.m_para, "InternalLoadsModel::para_t",
																   NANDRAD::InternalLoadsModel::P_PersonRadiationFraction,
																   1 - pers->m_para[VICUS::InternalLoad::P_ConvectiveHeatFactor].get_value("---"));
							}

							//allModelIds.push_back(intLoadSched.m_id);
						}
						break;
						case VICUS::ZoneTemplate::ST_IntLoadEquipment:
						case VICUS::ZoneTemplate::ST_IntLoadLighting:
						case VICUS::ZoneTemplate::ST_IntLoadOther:{
							//get internal load model
							//get schedule ids
							//check valid status
							const VICUS::InternalLoad *intLoadMod = VICUS::element(m_embeddedDB.m_internalLoads, ztBools[counter].m_subTemplateId[e]);
							if(intLoadMod == nullptr)
								addVicusScheduleToNandradProject(zeroValueSched,subTempTypeToNameWithUnit[VICUS::ZoneTemplate::SubTemplateType(e)],
									p, uniqueName);
							else{
								unsigned int schedId = intLoadMod->m_idPowerManagementSchedule;
								const VICUS::Schedule *schedMan = VICUS::element(m_embeddedDB.m_schedules, schedId);
								if(schedMan == nullptr)
									IBK::Exception(IBK::FormatString("Power management schedule with id %1 is not in database.")
												   .arg(schedId), FUNC_ID);
								if(!schedMan->isValid())
									IBK::Exception(IBK::FormatString("Power management schedule with id %1 and name '%2' is not in valid.")
												   .arg(schedId).arg(schedMan->m_displayName.string()), FUNC_ID);

								posIntLoad enum1;
								switch(e){
									case VICUS::ZoneTemplate::ST_IntLoadEquipment:{
										enum1 = P_Electric;
										NANDRAD::KeywordList::setParameter(intLoad.m_para, "InternalLoadsModel::para_t",
																		   NANDRAD::InternalLoadsModel::P_EquipmentRadiationFraction,
																		   1 - intLoadMod->m_para[VICUS::InternalLoad::P_ConvectiveHeatFactor].get_value("---"));
									}break;
									case VICUS::ZoneTemplate::ST_IntLoadLighting:{
										enum1 = P_Lighting;
										NANDRAD::KeywordList::setParameter(intLoad.m_para, "InternalLoadsModel::para_t",
																		   NANDRAD::InternalLoadsModel::P_LightingRadiationFraction,
																		   1 - intLoadMod->m_para[VICUS::InternalLoad::P_ConvectiveHeatFactor].get_value("---"));
									}break;
									case VICUS::ZoneTemplate::ST_IntLoadOther:{
										enum1 = P_Other;
										//TODO Implement Model in NANDRAD
										/*NANDRAD::KeywordList::setParameter(intLoad.m_para, "InternalLoadsModel::para_t",
																		   NANDRAD::InternalLoadsModel::P_EquipmentRadiationFraction,
																		   1 - intLoadMod->m_para[VICUS::InternalLoad::P_ConvectiveHeatFactor].get_value("---")); */
									}break;
								}
								intLoadScheds[enum1] = *schedMan;
								intLoadScheds[enum1].m_id = uniqueIdWithPredef2(Profile, 1, m_idMaps);//VICUS::Project::uniqueId<unsigned int>(allModelIds);
								//get val
								//multiply sched*val
								//multiply sched and constant val

								switch(intLoadMod->m_powerMethod){
									case VICUS::InternalLoad::PM_PowerPerArea:
										intLoadScheds[enum1] = intLoadScheds[enum1].multiply(intLoadMod->m_para[VICUS::InternalLoad::P_PowerPerArea].get_value("W/m2"));
									break;
									case VICUS::InternalLoad::PM_Power: {
										double val = intLoadMod->m_para[VICUS::InternalLoad::P_Power].get_value("W");
										intLoadScheds[enum1] = intLoadScheds[enum1].multiply(val/areaToRoomIdsObj.first);
									} break;
									case VICUS::InternalLoad::NUM_PM:
									break;
								}
								addVicusScheduleToNandradProject(intLoadScheds[enum1],subTempTypeToNameWithUnit[VICUS::ZoneTemplate::SubTemplateType(e)],
									p, uniqueName);
							}
							//allModelIds.push_back(intLoadScheds[enum1].m_id);
						}
						break;
					}
				}
				//add internal loads model to NANDRAD project
				p.m_models.m_internalLoadsModels.push_back(intLoad);
			}
		}
		// *** check for ideal heating and cooling in zone template ***


		VICUS::ZoneTemplate::SubTemplateType type;
		if(zt->m_idReferences[VICUS::ZoneTemplate::ST_ControlThermostat] != VICUS::INVALID_ID){
			const VICUS::ZoneControlThermostat * thermo =  VICUS::element(m_embeddedDB.m_zoneControlThermostats,
																   zt->m_idReferences[VICUS::ZoneTemplate::ST_ControlThermostat]);
			std::string objListNameThermostat;
			if(!createThermostat(thermo, zt->m_displayName.string(), p, m_idMaps, allRoomIdsForThisZt, objListNameThermostat))
				errorStack << tr("Error in Export. Thermostat in zonetemplate #%1 with name '%2' was not exported. ")
							  .arg(zt->m_id).arg(QString::fromUtf8(zt->m_displayName.string().c_str()));
			else{
				type = VICUS::ZoneTemplate::ST_IdealHeatingCooling;
				const VICUS::ZoneIdealHeatingCooling *ideal = VICUS::element(m_embeddedDB.m_zoneIdealHeatingCooling,
																	  zt->m_idReferences[VICUS::ZoneTemplate::ST_IdealHeatingCooling]);
				if(!createIdealHeatingCooling(ideal, p, m_idMaps, objListNameThermostat))
					errorStack << tr("Error in Export. Ideal Heating Cooling in zonetemplate #%1 with name '%2' was not exported. ")
								  .arg(zt->m_id).arg(QString::fromUtf8(zt->m_displayName.string().c_str()));

				//create surface heating/cooling for each zone/room
				for(unsigned int i=0; i<allRoomIdsForThisZt.size(); ++i){
					//search in surface conditing system if there is a surface heating element
					for(unsigned int j=0; j<m_surfaceSystems.size(); ++j){
						DataSurfaceHeating &dsh = m_surfaceSystems[j];
						if(dsh.m_zoneId == allRoomIdsForThisZt[i]){
							//add all surface systems of this room
							for(std::map<unsigned int, DataSurfaceHeating::Data>::const_iterator it;
								it != dsh.m_idSurfaceSystemToIds.end();
								++it){
								const VICUS::SurfaceHeating * shSys = VICUS::element(m_embeddedDB.m_surfaceHeatings, it->first);

								if(shSys == nullptr){
									//todo errorstack füllen
									continue;
								}

								switch (shSys->m_type) {
									case VICUS::SurfaceHeating::T_Ideal:
									case VICUS::SurfaceHeating::NUM_T:{
										NANDRAD::IdealSurfaceHeatingCoolingModel surfSysNandrad;
										surfSysNandrad.m_thermostatZoneId = dsh.m_zoneId;
										surfSysNandrad.m_constructionObjectList =
												createUniqueNandradObjListAndName("SurfaceHeatingConstructions", it->second.m_contructionInstanceIds, p,
																													NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE);
										NANDRAD::KeywordList::setParameter(surfSysNandrad.m_para, "IdealSurfaceHeatingCoolingModel::para_t",
																		   NANDRAD::IdealSurfaceHeatingCoolingModel::P_MaxHeatingPowerPerArea, shSys->m_para[VICUS::SurfaceHeating::P_HeatingLimit].value);
										NANDRAD::KeywordList::setParameter(surfSysNandrad.m_para, "IdealSurfaceHeatingCoolingModel::para_t",
																		   NANDRAD::IdealSurfaceHeatingCoolingModel::P_MaxCoolingPowerPerArea, shSys->m_para[VICUS::SurfaceHeating::P_CoolingLimit].value);
										p.m_models.m_idealSurfaceHeatingCoolingModels.push_back(surfSysNandrad);
									}
									break;

									case VICUS::SurfaceHeating::T_IdealPipeRegister:{
										// get pipe data
										const VICUS::NetworkPipe * pipe = VICUS::element(m_embeddedDB.m_pipes, shSys->m_idPipe);
										if(pipe == nullptr){
											// TODO Dirk, error handling errorstack
											continue;
										}
										if (!pipe->isValid()) {
											// TODO Dirk, error handling errorstack
											continue;
										}

										//anlegen der grunddaten
										NANDRAD::HydraulicFluid hyFluid;
										hyFluid.defaultFluidWater();

										NANDRAD::IdealPipeRegisterModel idealPipe;

										NANDRAD::KeywordList::setParameter(idealPipe.m_para, "IdealPipeRegisterModel::para_t",
																		   NANDRAD::IdealPipeRegisterModel::P_UValuePipeWall, pipe->UValue());
										NANDRAD::KeywordList::setParameter(idealPipe.m_para, "IdealPipeRegisterModel::para_t",
																		   NANDRAD::IdealPipeRegisterModel::P_PipeInnerDiameter, pipe->diameterInside());
										idealPipe.m_thermostatZoneId = dsh.m_zoneId;
										idealPipe.m_fluid = hyFluid;

										//alle gleich großen Flächen bekommen das gleiche
										double pipeSpacing = shSys->m_para[VICUS::SurfaceHeating::P_PipeSpacing].value;
										for(std::map<int, std::vector<unsigned int>>::const_iterator it2 = it->second.m_areaToIds.begin();
														it2 != it->second.m_areaToIds.end();
														++it2){
											double length = it2->first / (pipeSpacing * 10000); //Achtung umrechnung beachten
											int parallelPipes = 1;
											//get parallel pipes and adjust length
											if(length > 100){
												parallelPipes = std::ceil(length / 100);
												length /= parallelPipes;
											}

											NANDRAD::KeywordList::setParameter(idealPipe.m_para, "IdealPipeRegisterModel::para_t",
																			   NANDRAD::IdealPipeRegisterModel::P_PipeLength, length);
											NANDRAD::KeywordList::setIntPara(idealPipe.m_intPara, "IdealPipeRegisterModel::intPara_t",
																			 NANDRAD::IdealPipeRegisterModel::IP_NumberParallelPipes,
																			 parallelPipes);
											idealPipe.m_constructionObjectList =
													createUniqueNandradObjListAndName("SurfaceHeatingConstructions", it2->second, p,
																					  NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE);
											p.m_models.m_idealPipeRegisterModels.push_back(idealPipe);

											//TODO add heating and cooling schedule
											//dazu muss die klimadatei geparst werden die lufttemperatur ermitteln
											//dann mit den punkten für heiz und kühllastkurve die fluidtemperaturen ermitteln
											//und einen jahreszeitplan rausschreiben
											//TODO Dirk Name herausfinden

											std::vector<double> supplyTemperatureVec;
											calculateSupplyTemperature(shSys->m_heatingCoolingCurvePoints.m_values.at("Tsupply"),
																	   shSys->m_heatingCoolingCurvePoints.m_values.at("Tout"),
																	   outdoorTemp.y(), supplyTemperatureVec);

											NANDRAD::LinearSplineParameter tSupply("FluidSupplyTemperature",NANDRAD::LinearSplineParameter::I_LINEAR,
																				   outdoorTemp.x(), supplyTemperatureVec,
																				   IBK::Unit("h"),IBK::Unit("K"));
											p.m_schedules.m_annualSchedules[idealPipe.m_constructionObjectList].push_back(tSupply);
										}
									}
									break;
								}
							}
						}
					}
				}
			}
		}

		// *** infiltration and ventilation ***
		{
			unsigned int idNatVentCtrl = zt->m_idReferences[VICUS::ZoneTemplate::ST_ControlVentilationNatural];
			const VICUS::ZoneControlNaturalVentilation * natVentCtrl = nullptr;
			bool isCtrl = false;
			if(idNatVentCtrl != VICUS::INVALID_ID){
				natVentCtrl = VICUS::element(m_embeddedDB.m_zoneControlVentilationNatural, idNatVentCtrl);
				if(natVentCtrl != nullptr){
					//set bool for ventilation models later
					isCtrl = true;

				}
			}

			//TODO Dirk
			enum VentiType{
				Infiltration,
				Ventilation,
				InfAndVenti,
			};
			//get ids and check
			type = VICUS::ZoneTemplate::ST_Infiltration;
			unsigned int idSubTempInf = zt->m_idReferences[type];
			bool isInf = idSubTempInf != VICUS::INVALID_ID;
			type = VICUS::ZoneTemplate::ST_VentilationNatural;
			unsigned int idSubTempVent = zt->m_idReferences[type];
			bool isVenti = idSubTempVent != VICUS::INVALID_ID;

			VentiType ventiType = Infiltration;
			//check which type
			if(isInf && !isVenti)				ventiType = Infiltration;
			else if(!isInf && isVenti)			ventiType = Ventilation;
			else if(isInf && isVenti)			ventiType = InfAndVenti;

			if( isInf || isVenti){
				//create a NANDRAD natural ventilation model
				NANDRAD::NaturalVentilationModel natVentMod;
				natVentMod.m_displayName = zt->m_displayName.string();
				natVentMod.m_id = uniqueIdWithPredef2(Profile, 1, m_idMaps, true);//VICUS::Project::uniqueId<unsigned int>(allModelIds);
				//allModelIds.push_back(natVentMod.m_id);
				natVentMod.m_zoneObjectList =
						createUniqueNandradObjListAndName(zt->m_displayName.string(), allRoomIdsForThisZt, p,
														  NANDRAD::ModelInputReference::MRT_ZONE);
				switch (ventiType) {
					case Infiltration:{
						type = VICUS::ZoneTemplate::ST_Infiltration;
						ztBools[counter].m_subTemplateId[type] = idSubTempInf;
						const VICUS::Infiltration * inf = VICUS::element(m_embeddedDB.m_infiltration, idSubTempInf);
						if(inf != nullptr){
							natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_Constant;
							switch(inf->m_airChangeType){
								case VICUS::Infiltration::AC_normal:{
									NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
																	   NANDRAD::NaturalVentilationModel::P_VentilationRate,
																	   inf->m_para[VICUS::Infiltration::P_AirChangeRate].get_value("1/h"));
								}break;
								case VICUS::Infiltration::AC_n50:{
									double val = inf->m_para[VICUS::Infiltration::P_AirChangeRate].get_value("1/h");
									val *= inf->m_para[VICUS::Infiltration::P_ShieldingCoefficient].get_value("-");
									NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
																	   NANDRAD::NaturalVentilationModel::P_VentilationRate,
																	   val);
								}break;
								case VICUS::Infiltration::NUM_AC:{
									NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
																	   NANDRAD::NaturalVentilationModel::P_VentilationRate,
																	   0);
								}break;
							}
						}

					}break;
					case Ventilation:{
						type = VICUS::ZoneTemplate::ST_VentilationNatural;
						ztBools[counter].m_subTemplateId[type] = idSubTempVent;
						const VICUS::VentilationNatural* vent = VICUS::element(m_embeddedDB.m_ventilationNatural, idSubTempVent);
						if(vent != nullptr){
							if(isCtrl){
								natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR;
								NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
																   NANDRAD::NaturalVentilationModel::P_VentilationRate,
																   0);
								//set all control values
								NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
																   NANDRAD::NaturalVentilationModel::P_MaximumEnviromentAirTemperatureACRLimit,
																   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMax].get_value("C"));
								NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
																   NANDRAD::NaturalVentilationModel::P_MaximumRoomAirTemperatureACRLimit,
																   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMax].get_value("C"));
								NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
																   NANDRAD::NaturalVentilationModel::P_MinimumEnviromentAirTemperatureACRLimit,
																   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMin].get_value("C"));
								NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
																   NANDRAD::NaturalVentilationModel::P_MinimumRoomAirTemperatureACRLimit,
																   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMin].get_value("C"));
								NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
																   NANDRAD::NaturalVentilationModel::P_DeltaTemperatureACRLimit,
																   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureDifference].get_value("K"));
								NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
																   NANDRAD::NaturalVentilationModel::P_WindSpeedACRLimit,
																   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_WindSpeedMax].get_value("m/s"));

							}
							else
								natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_Scheduled;
							unsigned int schedId = vent->m_idSchedule;
							const VICUS::Schedule *schedMan = VICUS::element(m_embeddedDB.m_schedules, schedId);
							if(schedMan == nullptr)
								IBK::Exception(IBK::FormatString("Air change rate modification schedule with id #%1 is not in database.")
											   .arg(schedId), FUNC_ID);
							if(!schedMan->isValid())
								IBK::Exception(IBK::FormatString("Air change rate modification schedule with id #%1 and name '%2' is not in valid.")
											   .arg(schedId).arg(schedMan->m_displayName.string()), FUNC_ID);
							VICUS::Schedule ventSched = schedMan->multiply(vent->m_para[VICUS::VentilationNatural::P_AirChangeRate].get_value("1/h"));

							addVicusScheduleToNandradProject(ventSched, "VentilationRateSchedule [1/h]", p, natVentMod.m_zoneObjectList);
						}
					}break;
					case InfAndVenti:{
						VICUS::ZoneTemplate::SubTemplateType type1 = VICUS::ZoneTemplate::ST_Infiltration;
						ztBools[counter].m_subTemplateId[type1] = idSubTempInf;
						const VICUS::Infiltration * inf = VICUS::element(m_embeddedDB.m_infiltration, idSubTempInf);
						VICUS::ZoneTemplate::SubTemplateType type2 = VICUS::ZoneTemplate::ST_VentilationNatural;
						ztBools[counter].m_subTemplateId[type2] = idSubTempVent;
						const VICUS::VentilationNatural* vent = VICUS::element(m_embeddedDB.m_ventilationNatural, idSubTempVent);
						if(inf == nullptr || vent == nullptr)
							throw IBK::Exception(IBK::FormatString("Infiltration id #%1 and/or ventilation id #%2 model is not found.")
												 .arg(idSubTempInf).arg(idSubTempVent), FUNC_ID);
						if(!inf->isValid())
							throw IBK::Exception(IBK::FormatString("Infiltration id #%1 is not valid.")
												 .arg(idSubTempInf), FUNC_ID);
						/* TODO Dirk->Andreas wie bekomm ich hier die Datenbanken rein?
						if(!vent->isValid())
							throw IBK::Exception(IBK::FormatString("Ventilation id #%1 is not valid.")
												 .arg(idSubTempVent), FUNC_ID);
						*/

						//check infiltration value
						double infVal = 0;
						switch(inf->m_airChangeType){
							case VICUS::Infiltration::AC_normal:{
								infVal = inf->m_para[VICUS::Infiltration::P_AirChangeRate].get_value("1/h");
							}break;
							case VICUS::Infiltration::AC_n50:{
								infVal = inf->m_para[VICUS::Infiltration::P_AirChangeRate].get_value("1/h");
								infVal *= inf->m_para[VICUS::Infiltration::P_ShieldingCoefficient].get_value("-");
							}break;
							case VICUS::Infiltration::NUM_AC: break;		//only for compiler
						}

						// get ventilation schedule
						// multiply const value with scheduele
						unsigned int schedId = vent->m_idSchedule;
						const VICUS::Schedule *schedMan = VICUS::element(m_embeddedDB.m_schedules, schedId);
						if(schedMan == nullptr)
							IBK::Exception(IBK::FormatString("Air change rate modification schedule with id #%1 is not in database.")
										   .arg(schedId), FUNC_ID);
						if(!schedMan->isValid())
							IBK::Exception(IBK::FormatString("Air change rate modification schedule with id %1 and name '%2' is not in valid.")
										   .arg(schedId).arg(schedMan->m_displayName.string()), FUNC_ID);
						VICUS::Schedule ventSched = schedMan->multiply(vent->m_para[VICUS::VentilationNatural::P_AirChangeRate].get_value("1/h"));

						if(isCtrl){
							natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR;
							NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
															   NANDRAD::NaturalVentilationModel::P_VentilationRate,
															   infVal);
							//all control values
							NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
															   NANDRAD::NaturalVentilationModel::P_MaximumEnviromentAirTemperatureACRLimit,
															   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMax].get_value("C"));
							NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
															   NANDRAD::NaturalVentilationModel::P_MaximumRoomAirTemperatureACRLimit,
															   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMax].get_value("C"));
							NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
															   NANDRAD::NaturalVentilationModel::P_MinimumEnviromentAirTemperatureACRLimit,
															   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMin].get_value("C"));
							NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
															   NANDRAD::NaturalVentilationModel::P_MinimumRoomAirTemperatureACRLimit,
															   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMin].get_value("C"));
							NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
															   NANDRAD::NaturalVentilationModel::P_DeltaTemperatureACRLimit,
															   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureDifference].get_value("K"));
							NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
															   NANDRAD::NaturalVentilationModel::P_WindSpeedACRLimit,
															   natVentCtrl->m_para[VICUS::ZoneControlNaturalVentilation::ST_WindSpeedMax].get_value("m/s"));
						}
						else{
							natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_Scheduled;
							if(infVal > 0)
								ventSched = ventSched.add(infVal);
						}
						addVicusScheduleToNandradProject(ventSched, "VentilationRateSchedule [1/h]", p, natVentMod.m_zoneObjectList);
					}break;
				}
				p.m_models.m_naturalVentilationModels.push_back(natVentMod);
			}
		}
		++counter;
	}
	// ############################## Zone Templates




	// database elements

	// Construction instances reference construction types
	// Construction types reference materials

	// we have constructions and materials already in the embedded database, so we can just copy them over
	for (const VICUS::Material & m : m_embeddedDB.m_materials) {
		NANDRAD::Material matdata;

		matdata.m_id = uniqueIdWithPredef2(Material, m.m_id, m_idMaps);
		matdata.m_displayName = m.m_displayName.string(IBK::MultiLanguageString::m_language, "en");

		// now transfer parameters - fortunately, they have the same keywords, what a coincidence :-)
		matdata.m_para[NANDRAD::Material::P_Density] = m.m_para[VICUS::Material::P_Density];
		matdata.m_para[NANDRAD::Material::P_HeatCapacity] = m.m_para[VICUS::Material::P_HeatCapacity];
		matdata.m_para[NANDRAD::Material::P_Conductivity] = m.m_para[VICUS::Material::P_Conductivity];

		// addConstruction to material list
		p.m_materials.push_back(matdata);
	}

	for (const VICUS::Construction & c : m_embeddedDB.m_constructions) {

		// now create a construction type
		NANDRAD::ConstructionType conType;
		conType.m_id = uniqueIdWithPredef2(Construction, c.m_id, m_idMaps);
		conType.m_displayName = c.m_displayName.string(IBK::MultiLanguageString::m_language, "en");

		for (const VICUS::MaterialLayer & ml : c.m_materialLayers) {
			NANDRAD::MaterialLayer mlayer;
			mlayer.m_matId = uniqueIdWithPredef2(Material, ml.m_idMaterial, m_idMaps);
			mlayer.m_thickness = ml.m_thickness.value;
			conType.m_materialLayers.push_back(mlayer);
		}

		// add to construction type list
		p.m_constructionTypes.push_back(conType);
	}

	for(const VICUS::WindowGlazingSystem &w : m_embeddedDB.m_windowGlazingSystems){
		if(w.m_modelType != VICUS::WindowGlazingSystem::MT_Simple){
			errorStack << tr("The window glazing system with #%1 and name '%2' is not supported by the export.")
						  .arg(w.m_id).arg(QString::fromStdString(w.m_displayName.string()));
			continue;
		}
		else{
			NANDRAD::WindowGlazingSystem winG;
			if(!w.isValid()){
				errorStack << tr("The window glazing system with #%1 and name '%2' is not valid. Export failed.")
							  .arg(w.m_id).arg(QString::fromStdString(w.m_displayName.string()));
				continue;
			}
			winG.m_displayName = w.m_displayName.string();
			winG.m_id = uniqueIdWithPredef2(Window, w.m_id, m_idMaps);
			winG.m_modelType = NANDRAD::WindowGlazingSystem::MT_Simple;
			NANDRAD::KeywordList::setParameter(winG.m_para, "WindowGlazingSystem::para_t",
											   NANDRAD::WindowGlazingSystem::P_ThermalTransmittance,
											   w.uValue());
			winG.m_splinePara[NANDRAD::WindowGlazingSystem::SP_SHGC]= w.m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC];

			p.m_windowGlazingSystems.push_back(winG);
		}
	}
	// TODO Andreas, Dirk, Stephan: Transfer other database elements/generate NANDRAD data objects

}




void Project::generateNetworkProjectData(NANDRAD::Project & p) const {
	FUNCID(Project::generateNetworkProjectData);

	// get selected Vicus Network
	unsigned int networkId = VICUS::INVALID_ID;
	for (const VICUS::Network &net: m_geometricNetworks){
		if (net.m_selectedForSimulation){
			networkId = net.m_id;
			// TODO Hauke, multiple (connected) networks?
			break;
		}
	}
	// if there is no network selected return - this is not an error, but the usual case for simple building energy
	// simulations
	if (VICUS::element(m_geometricNetworks, networkId) == nullptr)
		return;

	VICUS::Network vicusNetwork = *VICUS::element(m_geometricNetworks, networkId);

	// buildings can only have one connected edge
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Building && node.m_edges.size()>1 )
			throw IBK::Exception(IBK::FormatString("Node with id #%1 has more than one edge connected, but is a building.")
								 .arg(node.m_id), FUNC_ID);
	}

	// check network type
	if (vicusNetwork.m_type != VICUS::Network::NET_DoublePipe)
		throw IBK::Exception("This NetworkType is not yet implemented. Use networkType 'DoublePipe'", FUNC_ID);


	// create dummy zone
	NANDRAD::Zone z;
	z.m_id = 1;
	z.m_displayName = "dummy";
	z.m_type = NANDRAD::Zone::ZT_Active;
	NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Volume, 100);
	NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Area, 10);
	p.m_zones.push_back(z);


	// *** create Nandrad Network
	p.m_hydraulicNetworks.clear();
	NANDRAD::HydraulicNetwork nandradNetwork;
	nandradNetwork.m_modelType = NANDRAD::HydraulicNetwork::ModelType(vicusNetwork.m_modelType);
	nandradNetwork.m_id = vicusNetwork.m_id;
	nandradNetwork.m_displayName = vicusNetwork.m_displayName.toStdString();
	nandradNetwork.m_para[NANDRAD::HydraulicNetwork::P_DefaultFluidTemperature] =
			vicusNetwork.m_para[VICUS::Network::P_DefaultFluidTemperature];
	nandradNetwork.m_para[NANDRAD::HydraulicNetwork::P_InitialFluidTemperature] =
			vicusNetwork.m_para[VICUS::Network::P_InitialFluidTemperature];
	nandradNetwork.m_para[NANDRAD::HydraulicNetwork::P_ReferencePressure] =
			vicusNetwork.m_para[VICUS::Network::P_ReferencePressure];


	// *** Transfer FLUID from Vicus to Nandrad

	const VICUS::NetworkFluid *fluid = VICUS::element(m_embeddedDB.m_fluids, vicusNetwork.m_idFluid);
	if (fluid == nullptr)
		throw IBK::Exception(IBK::FormatString("Fluid with id #%1 does not exist in database!").arg(vicusNetwork.m_idFluid), FUNC_ID);
	if (!fluid->isValid())
		throw IBK::Exception(IBK::FormatString("Fluid with id #%1 has invalid parameters!").arg(vicusNetwork.m_idFluid), FUNC_ID);
	nandradNetwork.m_fluid.m_displayName = fluid->m_displayName.string();
	nandradNetwork.m_fluid.m_kinematicViscosity = fluid->m_kinematicViscosity;
	for (int i=0; i<VICUS::NetworkFluid::NUM_P; ++i)
		nandradNetwork.m_fluid.m_para[i] = fluid->m_para[i];


	// create Databases from embedded Databases
	Database<SubNetwork> dbSubNetworks = Database<SubNetwork>(1);
	dbSubNetworks.setData(m_embeddedDB.m_subNetworks);
	Database<NetworkComponent> dbNetworkComps = Database<NetworkComponent>(1); // we dont care
	dbNetworkComps.setData(m_embeddedDB.m_networkComponents);
	Database<NetworkController> dbNetworkCtrl = Database<NetworkController>(1); // we dont care
	dbNetworkCtrl.setData(m_embeddedDB.m_networkControllers);
	Database<Schedule> dbSchedules = Database<Schedule>(1); // we dont care
	dbSchedules.setData(m_embeddedDB.m_schedules);

	// *** Transfer COMPONENTS from Vicus to Nandrad

	// --> collect sub networks
	std::set<unsigned int> subNetworkIds, componentIds, controllerIds;
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Mixer)
			continue;
		subNetworkIds.insert(node.m_idSubNetwork);
	}

	// --> collect and check sub networks
	unsigned int maxNumberElements = 1;
	for (unsigned int subId: subNetworkIds){
		const VICUS::SubNetwork *sub = VICUS::element(m_embeddedDB.m_subNetworks, subId);

		// some checks
		if (sub == nullptr)
			throw IBK::Exception(IBK::FormatString("Sub Network with id #%1 does not exist in database").arg(subId), FUNC_ID);
		if (!sub->isValid(dbNetworkComps, dbNetworkCtrl, dbSchedules))
			throw IBK::Exception(IBK::FormatString("Sub Network with id #%1 has invalid parameters").arg(subId), FUNC_ID);

		// determine maximum number of elements of all sub networks
		if (sub->m_elements.size() > maxNumberElements)
			maxNumberElements = sub->m_elements.size();

		for (const NANDRAD::HydraulicNetworkElement &el: sub->m_elements){
			componentIds.insert(el.m_componentId);
			if (el.m_controlElementId != NANDRAD::INVALID_ID)
				controllerIds.insert(el.m_controlElementId);
		}
	}

	// --> transfer components
	for (unsigned int compId: componentIds){
		const VICUS::NetworkComponent *comp = VICUS::element(m_embeddedDB.m_networkComponents, compId);

		if (comp == nullptr)
			throw IBK::Exception(IBK::FormatString("Network Component with id #%1 does not exist in database").arg(compId), FUNC_ID);
		if (!comp->isValid(dbSchedules))
			throw IBK::Exception(IBK::FormatString("Network Component with id #%1 has invalid parameters").arg(compId), FUNC_ID);

		NANDRAD::HydraulicNetworkComponent nandradComp;
		nandradComp.m_id = comp->m_id;
		nandradComp.m_displayName = comp->m_displayName.string(IBK::MultiLanguageString::m_language, "en");
		nandradComp.m_modelType = (NANDRAD::HydraulicNetworkComponent::ModelType) comp->m_modelType;
		nandradComp.m_polynomCoefficients = comp->m_polynomCoefficients;
		for (int i=0; i<VICUS::NetworkComponent::NUM_P; ++i)
			nandradComp.m_para[i] = comp->m_para[i];

		nandradNetwork.m_components.push_back(nandradComp);
	}

	// --> transfer controllers
	for (unsigned int ctrId: controllerIds){

		const VICUS::NetworkController *ctr = VICUS::element(m_embeddedDB.m_networkControllers, ctrId);
		if (ctr == nullptr)
			throw IBK::Exception(IBK::FormatString("Network Controller with id #%1 does not exist in database").arg(ctrId), FUNC_ID);
		if (!ctr->isValid(dbSchedules))
			throw IBK::Exception(IBK::FormatString("Network Controller with id #%1 has invalid parameters").arg(ctrId), FUNC_ID);

		NANDRAD::HydraulicNetworkControlElement nandradCtr;
		nandradCtr.m_id = ctr->m_id;
		nandradCtr.m_modelType = NANDRAD::HydraulicNetworkControlElement::ModelType(ctr->m_modelType);
		nandradCtr.m_controllerType = NANDRAD::HydraulicNetworkControlElement::ControllerType(ctr->m_controllerType);
		nandradCtr.m_controlledProperty = NANDRAD::HydraulicNetworkControlElement::ControlledProperty(ctr->m_controlledProperty);
		nandradCtr.m_maximumControllerResultValue = ctr->m_maximumControllerResultValue;
		for (unsigned int i=0; i<NANDRAD::HydraulicNetworkControlElement::NUM_P; ++i)
			nandradCtr.m_para[i] = ctr->m_para[i];
		for (unsigned int i=0; i<NANDRAD::HydraulicNetworkControlElement::NUM_ID; ++i)
			nandradCtr.m_idReferences[i] = ctr->m_idReferences[i];

		nandradNetwork.m_controlElements.push_back(nandradCtr);
	}


	// *** Transform PIPES from Vicus to NANDRAD

	// --> collect all pipeIds used in vicus network
	std::set<unsigned int> pipeIds;
	for (const VICUS::NetworkEdge &edge: vicusNetwork.m_edges){
		if(edge.m_idPipe == VICUS::INVALID_ID)
				throw IBK::Exception(IBK::FormatString("Edge '%1'->'%2' has no referenced pipe")
									 .arg(edge.nodeId1()).arg(edge.nodeId2()), FUNC_ID);
		pipeIds.insert(edge.m_idPipe);
	}

	// --> transfer
	for(unsigned int pipeId: pipeIds) {

		const VICUS::NetworkPipe *pipe = VICUS::element(m_embeddedDB.m_pipes, pipeId);
		if (pipe == nullptr)
			throw IBK::Exception(IBK::FormatString("Pipe with id #%1 does not exist in database").arg(pipeId), FUNC_ID);
		if (!pipe->isValid())
			throw IBK::Exception(IBK::FormatString("Network Pipe with id #%1 has invalid parameters").arg(pipeId), FUNC_ID);

		NANDRAD::HydraulicNetworkPipeProperties pipeProp;
		pipeProp.m_id = pipe->m_id;

		// set pipe properties
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter,
										   pipe->m_para[VICUS::NetworkPipe::P_DiameterOutside].get_value("mm"));
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter,
										   pipe->diameterInside() * 1000); // m -> mmm
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeRoughness,
										   pipe->m_para[VICUS::NetworkPipe::P_RoughnessWall].get_value("mm"));
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall, pipe->UValue());
		nandradNetwork.m_pipeProperties.push_back(pipeProp);
	}


	// *** Transfer ELEMENTS from Vicus to Nandrad

	// estimated number of elements
	nandradNetwork.m_elements.reserve(vicusNetwork.m_nodes.size() * maxNumberElements + 2 * vicusNetwork.m_edges.size());

	std::map<unsigned int, std::vector<unsigned int>> componentElementMap; // this map stores the element ids for each component
	std::vector<unsigned int> allNodeIds = {0};			// stores all nodeIds of the network (the ids which are used to connect elements)
	std::vector<unsigned int> allElementIds = {0};		// stores all element ids of the network
	std::map<unsigned int, unsigned int> supplyNodeIdMap; // a map that stores for each VICUS geometric node the NANDRAD inlet node
	std::map<unsigned int, unsigned int> returnNodeIdMap; // a map that stores for each VICUS geometric node the NANDRAD outlet node

	// iterate over all geometric network nodes
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes) {

		// for each vicus geometric node: store two new node ids (supply and return) for the nandrad network
		supplyNodeIdMap[node.m_id] = VICUS::uniqueIdAdd(allNodeIds);
		returnNodeIdMap[node.m_id] = VICUS::uniqueIdAdd(allNodeIds);

		// if this is a mixer continue
		if (node.m_type == VICUS::NetworkNode::NT_Mixer)
			continue;

		// check if there is a valid sub network
		const VICUS::SubNetwork *sub = VICUS::element(m_embeddedDB.m_subNetworks, node.m_idSubNetwork);
		if (sub == nullptr)
			throw IBK::Exception(IBK::FormatString("Node with id #%1 has not a valid referenced sub network!").arg(node.m_id), FUNC_ID);


		// Check if we have at least one element that has inletNodeId=INLET_ID (this is the sub network inlet)
		// and at least one element with outletNodeId=OUTLET_ID (this is the sub network outlet)
		// Moreover: for the nodes which are not sub network inlet/outlet, but (local) inlet/outlet node ids:
		// create a map that stores new unique ids for them
		bool subInletFound = false;
		bool subOutletFound = false;
		std::map<unsigned int, unsigned int> subNetNodeIdMap;
		for (const NANDRAD::HydraulicNetworkElement &elem: sub->m_elements){

			if (elem.m_inletNodeId == VICUS::SubNetwork::INLET_ID){
				subInletFound = true;
				if (node.m_type == VICUS::NetworkNode::NT_Source)
					subNetNodeIdMap[elem.m_inletNodeId] = returnNodeIdMap[node.m_id];
				else
					subNetNodeIdMap[elem.m_inletNodeId] = supplyNodeIdMap[node.m_id];
			}

			if (elem.m_outletNodeId == VICUS::SubNetwork::OUTLET_ID){
				subOutletFound = true;
				if (node.m_type == VICUS::NetworkNode::NT_Source)
					subNetNodeIdMap[elem.m_outletNodeId] = supplyNodeIdMap[node.m_id];
				else
					subNetNodeIdMap[elem.m_outletNodeId] = returnNodeIdMap[node.m_id];
			}

			if (subNetNodeIdMap.find(elem.m_inletNodeId) == subNetNodeIdMap.end())
				subNetNodeIdMap[elem.m_inletNodeId] = uniqueIdAdd(allNodeIds);
			if (subNetNodeIdMap.find(elem.m_outletNodeId) == subNetNodeIdMap.end())
				subNetNodeIdMap[elem.m_outletNodeId] = uniqueIdAdd(allNodeIds);

		}
		if (!subInletFound)
			throw IBK::Exception(IBK::FormatString("Sub Network with id #%1 does not have an element with inletNodeId=0, "
												   "This is required for every sub network").arg(node.m_id), FUNC_ID);
		if (!subOutletFound)
			throw IBK::Exception(IBK::FormatString("Sub Network with id #%1 does not have an element with outletNodeId=0, "
												   "This is required for every sub network").arg(node.m_id), FUNC_ID);


		// now we can create the new nandrad elements and map the repsctive nodes
		for (const NANDRAD::HydraulicNetworkElement &elem: sub->m_elements){

			// 1. copy the element and create a unique element id for it
			NANDRAD::HydraulicNetworkElement newElement = elem;
			newElement.m_id = VICUS::uniqueIdAdd(allElementIds);

			// 2. set the new elements inlet and outlet id using the map that we created
			newElement.m_inletNodeId = subNetNodeIdMap[elem.m_inletNodeId];
			newElement.m_outletNodeId = subNetNodeIdMap[elem.m_outletNodeId];

			// 3. get component name in display name
			const VICUS::NetworkComponent *comp = VICUS::element(m_embeddedDB.m_networkComponents, newElement.m_componentId);
			Q_ASSERT(comp!=nullptr);
			newElement.m_displayName = IBK::FormatString("%1_%2#%3")
					.arg(comp->m_displayName.string()).arg(node.m_displayName.toStdString()).arg(node.m_id).str();

			// 4. if this is a source node: set the respective reference element id of the network (for pressure calculation)
			if (node.m_type == VICUS::NetworkNode::NT_Source)
				nandradNetwork.m_referenceElementId = newElement.m_id;

			// 5. if this element is the one which shall exchange heat: we copy the respective heat exchange properties from the node
			// we recognize this using the original element id (origElem.m_id)
			if (elem.m_id == sub->m_idHeatExchangeElement)
				newElement.m_heatExchange = node.m_heatExchange;

			// 6. add element to the nandrad network
			nandradNetwork.m_elements.push_back(newElement);

			// we store the element ids for each component, with this info we can create the schedules and object lists
			componentElementMap[elem.m_componentId].push_back(newElement.m_id);
		}


	}  // end of iteration over network nodes



	// *** Transfer SCHEDULES from Vicus to Nandrad

	for (auto it=componentElementMap.begin(); it!=componentElementMap.end(); ++it){

		const VICUS::NetworkComponent *comp = VICUS::element(m_embeddedDB.m_networkComponents, it->first);

		if (comp->m_scheduleIds.empty() &&
		// THIS IS JUST TEMPORARY !!!!
				comp->m_modelType != VICUS::NetworkComponent::MT_ConstantMassFluxPump)
			continue;

		// create and add object list
		NANDRAD::ObjectList objList;
		objList.m_name = comp->m_displayName.string(IBK::MultiLanguageString::m_language, "en") + " elements";
		objList.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
		for (unsigned int elementId: it->second)
			objList.m_filterID.m_ids.insert(elementId);
		p.m_objectLists.push_back(objList);

		// get a list with required schedule names, they correspond to the component schedule ids
		std::vector<std::string> scheduleNames= NANDRAD::HydraulicNetworkComponent::requiredScheduleNames(
					NANDRAD::HydraulicNetworkComponent::ModelType(comp->m_modelType));
		Q_ASSERT(scheduleNames.size() == comp->m_scheduleIds.size());

		// add schedules of component to nandrad
		for (unsigned int i = 0; i<comp->m_scheduleIds.size(); ++i){
			const VICUS::Schedule *sched = VICUS::element(m_embeddedDB.m_schedules, comp->m_scheduleIds[i]);
			if (sched == nullptr)
				throw IBK::Exception(IBK::FormatString("Schedule with id #%1, referenced in network component with id #%2"
													   " does not exist").arg(comp->m_scheduleIds[i]).arg(comp->m_id), FUNC_ID);
			if (!sched->isValid())
				throw IBK::Exception(IBK::FormatString("Schedule with id #%1 has invalid parameters").arg(sched->m_id), FUNC_ID);
			addVicusScheduleToNandradProject(*sched, scheduleNames[i], p, objList.m_name);
		}
	}


	// *** Transfer EDGES / PIPE ELEMENTS from Vicus to Nandrad

	unsigned int fmiValueRef = 42; // start value
	unsigned int idSoilModel = 0; // start value

	// find source node and create set of edges, which are ordered according to their distance to the source node
	std::set<const VICUS::NetworkNode *> dummyNodeSet;
	std::vector<const VICUS::NetworkEdge *> orderedEdges;
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Source){
			node.setInletOutletNode(dummyNodeSet, orderedEdges);
			break;
		}
	}

	// now iterate over edges
	std::vector<unsigned int> compIds(componentIds.begin(), componentIds.end());
	for (const VICUS::NetworkEdge *edge: orderedEdges) {

		// check and transform pipe model type
		NANDRAD::HydraulicNetworkComponent::ModelType pipeModelType = NANDRAD::HydraulicNetworkComponent::NUM_MT;
		switch (edge->m_pipeModel ) {
			case VICUS::NetworkEdge::PM_SimplePipe:{
				pipeModelType = NANDRAD::HydraulicNetworkComponent::MT_SimplePipe;
			} break;
			case VICUS::NetworkEdge::PM_DynamicPipe:{
				pipeModelType = NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe;
			} break;
			case VICUS::NetworkEdge::NUM_PM:
				throw IBK::Exception(IBK::FormatString("Edge %1->%2 has no valid pipe model type")
									 .arg(edge->m_node1->m_id).arg(edge->m_node2->m_id), FUNC_ID);
		}

		// check if a pipe component with this model type exists already
		NANDRAD::HydraulicNetworkComponent pipeComp;
		bool pipeComponentExists = false;
		for (const NANDRAD::HydraulicNetworkComponent &comp: nandradNetwork.m_components){
			if (comp.m_modelType == pipeModelType){
				pipeComp = comp;
				pipeComponentExists = true;
				break;
			}
		}

		// if there was none, add respective component for pipe
		if (!pipeComponentExists){
			NANDRAD::HydraulicNetworkComponent comp;
			comp.m_id = VICUS::uniqueIdAdd(compIds);
			comp.m_modelType = pipeModelType;
			if (pipeModelType == NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe){

				if (vicusNetwork.m_para[VICUS::Network::P_MaxPipeDiscretization].empty())
					throw IBK::Exception(IBK::FormatString("Missing Parameter '%1' in network with id #%2")
										.arg(VICUS::KeywordList::Keyword("Network::para_t", VICUS::Network::P_MaxPipeDiscretization))
										.arg(vicusNetwork.m_id), FUNC_ID);
				NANDRAD::KeywordList::setParameter(comp.m_para, "HydraulicNetworkComponent::para_t",
													NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth,
													vicusNetwork.m_para[VICUS::Network::P_MaxPipeDiscretization].value);
			}
			nandradNetwork.m_components.push_back(comp);
			pipeComp = comp;
		}

		// check if there is a reference to a pipe from DB
		const VICUS::NetworkPipe *pipe = VICUS::element(m_embeddedDB.m_pipes, edge->m_idPipe);
		if (pipe == nullptr)
			throw IBK::Exception(IBK::FormatString("Edge %1->%2 has no defined pipe from database")
								 .arg(edge->m_node1->m_id).arg(edge->m_node2->m_id), FUNC_ID);

		// create name
		IBK::FormatString pipeName = IBK::FormatString("%1#%2_%3#%4")
				.arg(vicusNetwork.nodeById(edge->m_idNodeInlet)->m_displayName.toStdString())
				.arg(vicusNetwork.nodeById(edge->m_idNodeInlet)->m_id)
				.arg(vicusNetwork.nodeById(edge->m_idNodeOutlet)->m_displayName.toStdString())
				.arg(vicusNetwork.nodeById(edge->m_idNodeOutlet)->m_id);

		// add inlet pipe element
		unsigned int inletNode = supplyNodeIdMap[edge->m_idNodeInlet];
		unsigned int outletNode = supplyNodeIdMap[edge->m_idNodeOutlet];
		NANDRAD::HydraulicNetworkElement supplyPipe(uniqueIdAdd(allElementIds),
													inletNode,
													outletNode,
													pipeComp.m_id,
													edge->m_idPipe,
													edge->length());
		supplyPipe.m_displayName = "SupplyPipe." + pipeName.str();
		supplyPipe.m_heatExchange = edge->m_heatExchange;
		nandradNetwork.m_elements.push_back(supplyPipe);

		// add outlet pipe element
		inletNode = returnNodeIdMap[edge->m_idNodeOutlet];
		outletNode = returnNodeIdMap[edge->m_idNodeInlet];
		NANDRAD::HydraulicNetworkElement returnPipe(uniqueIdAdd(allElementIds),
													inletNode,
													outletNode,
													pipeComp.m_id,
													edge->m_idPipe,
													edge->length());
		returnPipe.m_displayName = "ReturnPipe." + pipeName.str();
		returnPipe.m_heatExchange = edge->m_heatExchange;
		nandradNetwork.m_elements.push_back(returnPipe);


		// Create FMI Input Output Definitions
		if (vicusNetwork.m_hasHeatExchangeWithGround){

			// create FMI input definitions
			// --> supply pipe
			NANDRAD::FMIVariableDefinition inputDefSupplyPipeTemp;
			inputDefSupplyPipeTemp.m_fmiVarName = supplyPipe.m_displayName + ".Temperature"; // custom name
			inputDefSupplyPipeTemp.m_varName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",
																NANDRAD::ModelInputReference::MRT_NETWORKELEMENT );
			inputDefSupplyPipeTemp.m_varName += ".HeatExchangeTemperature";
			inputDefSupplyPipeTemp.m_unit = "K";
			inputDefSupplyPipeTemp.m_fmiValueRef = ++fmiValueRef;
			inputDefSupplyPipeTemp.m_fmiVarDescription = "Pre-described external temperature";
			inputDefSupplyPipeTemp.m_fmiStartValue = vicusNetwork.m_para[VICUS::Network::P_InitialFluidTemperature].value;
			p.m_fmiDescription.m_inputVariables.push_back(inputDefSupplyPipeTemp);
			// --> return pipe
			NANDRAD::FMIVariableDefinition inputDefReturnPipeTemp = inputDefSupplyPipeTemp;
			inputDefReturnPipeTemp.m_fmiVarName = returnPipe.m_displayName + ".Temperature";
			inputDefReturnPipeTemp.m_fmiValueRef = ++fmiValueRef;
			p.m_fmiDescription.m_inputVariables.push_back(inputDefReturnPipeTemp);

			// create FMI output definitions
			// --> supply pipe
			NANDRAD::FMIVariableDefinition outputDefSupplyPipeTemp;
			outputDefSupplyPipeTemp.m_fmiVarName = supplyPipe.m_displayName + ".HeatLoss"; // custom name
			outputDefSupplyPipeTemp.m_varName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",
																NANDRAD::ModelInputReference::MRT_NETWORKELEMENT );
			outputDefSupplyPipeTemp.m_varName += ".FlowElementHeatLoss";
			outputDefSupplyPipeTemp.m_unit = "W";
			outputDefSupplyPipeTemp.m_fmiValueRef = ++fmiValueRef;
			outputDefSupplyPipeTemp.m_fmiVarDescription = "Heat flux from flow element into environment";
			outputDefSupplyPipeTemp.m_fmiStartValue = 0;
			p.m_fmiDescription.m_outputVariables.push_back(outputDefSupplyPipeTemp);
			// --> return pipe
			NANDRAD::FMIVariableDefinition outputDefReturnPipeTemp = outputDefSupplyPipeTemp;
			outputDefReturnPipeTemp.m_fmiVarName = returnPipe.m_displayName + ".Temperature";
			outputDefReturnPipeTemp.m_fmiValueRef = ++fmiValueRef;
			p.m_fmiDescription.m_inputVariables.push_back(outputDefReturnPipeTemp);


			// store Nandrad element id in edge so they can be used later on
			const_cast<NetworkEdge*>(edge)->m_idNandradSupplyPipe = supplyPipe.m_id;
			const_cast<NetworkEdge*>(edge)->m_idNandradReturnPipe = returnPipe.m_id;

			// einfacher Ansatz: für jede Edge ein Delphin Modell
			NANDRAD::HydraulicNetworkSoilModel soilModel;
			soilModel.m_id = ++idSoilModel;

			soilModel.m_supplyPipeId = supplyPipe.m_id;
			soilModel.m_returnPipeId = returnPipe.m_id;
			if (edge->m_para[NetworkEdge::P_PipeDepth].empty())
				throw IBK::Exception(IBK::FormatString("Edge %1->%2 has no determined pipe depth. This is required for FMI coupling.")
									 .arg(edge->m_node1->m_id).arg(edge->m_node2->m_id), FUNC_ID);
			soilModel.m_pipeDepth = edge->m_para[NetworkEdge::P_PipeDepth];
			if (edge->m_para[NetworkEdge::P_PipeSpacing].empty())
				throw IBK::Exception(IBK::FormatString("Edge %1->%2 has no determined pipe spacing. This is required for FMI coupling.")
									 .arg(edge->m_node1->m_id).arg(edge->m_node2->m_id), FUNC_ID);
			soilModel.m_pipeSpacing = edge->m_para[NetworkEdge::P_PipeSpacing];
			soilModel.m_pipeOuterDiameter = pipe->m_para[NetworkPipe::P_DiameterOutside];

			nandradNetwork.m_soilModels.push_back(soilModel);
		}
	}

	// besserer Ansatz: entlang der Pfade gehen und entsprechend des TempChangeIndicator die Delphin Modelle zuweisen...

//	vicusNetwork.calcTemperatureChangeIndicator()
//	std::map<unsigned int, std::vector<NetworkEdge *> > shortestPaths;
//	vicusNetwork.findShortestPathForBuildings(shortestPaths);

//	for (auto it = shortestPaths.begin(); it != shortestPaths.end(); ++it){
//		std::vector<NetworkEdge *> &shortestPath = it->second; // for readability
//		for (NetworkEdge * edge: shortestPath)
//			edge->m_supplyPipeId ...
//	}



	 // we are DONE !!!
	 // finally add to nandrad project
	p.m_hydraulicNetworks.push_back(nandradNetwork);

}



NANDRAD::Interface Project::generateInterface(const VICUS::ComponentInstance & ci, unsigned int bcID,
											  std::vector<IdMap> &maps,
											  unsigned int & interfaceID, bool takeASide) const
{
	// no boundary condition ID? -> no interface
	if (bcID == VICUS::INVALID_ID)
		return NANDRAD::Interface();

	const VICUS::Surface * s;
	if(takeASide)
		s = ci.m_sideASurface;
	else
		s = ci.m_sideBSurface;

	// lookup boundary condition definition in embedded database
	const VICUS::BoundaryCondition * bc = VICUS::element(m_embeddedDB.m_boundaryConditions, bcID);
//	if (bc == nullptr)
//		throw ConversionError(ConversionError::ET_InvalidID,
//			tr("Component #%1 has invalid boundary condition ID reference #%2.")
//				.arg(ci.m_componentID).arg(s->m_id));
//	if (!bc->isValid())
//		throw ConversionError(ConversionError::ET_NotValid,
//			tr("Boundary condition #%1 has invalid/incomplete parameters.").arg(bc->m_id));

	// do we have a surface to a zone?
	if (s != nullptr) {
		// get the zone that this interface is connected to
		const VICUS::Object * obj = s->m_parent;
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(obj);
//		if (room == nullptr)
//			throw ConversionError(ConversionError::ET_MissingParentZone,
//				tr("Component instance #%1 references surface %2, which is not associated to a zone.")
//					.arg(ci.m_id).arg(s->m_id));

		// generate a new interface to the zone, which always only includes heat conduction
		NANDRAD::Interface iface;
		//iface.m_id = uniqueIdWithPredef(allModelIds, interfaceID);
		iface.m_id = uniqueIdWithPredef2(Interface, interfaceID, maps, true);
		if(maps[Zone].m_vicusToNandrad.find(room->m_id) != maps[Zone].m_vicusToNandrad.end())
			iface.m_zoneId = maps[Zone].m_vicusToNandrad[room->m_id];
		else
			iface.m_zoneId = room->m_id;

		// only transfer heat conduction parameters
		iface.m_heatConduction.m_modelType = (NANDRAD::InterfaceHeatConduction::modelType_t)bc->m_heatConduction.m_modelType;
		iface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient] = bc->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient];
		return iface;
	}
	else {
		// no surface? must be an interface to the outside

		// generate a new interface to the zone, which always only includes heat conduction
		NANDRAD::Interface iface;
		iface.m_id = uniqueIdWithPredef2(Interface, interfaceID, maps, true);
		iface.m_zoneId = 0; // outside zone
		iface.m_heatConduction.m_modelType = (NANDRAD::InterfaceHeatConduction::modelType_t)bc->m_heatConduction.m_modelType;
		iface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient] = bc->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient];
		iface.m_solarAbsorption = bc->m_solarAbsorption;
		iface.m_longWaveEmission = bc->m_longWaveEmission;
		return iface;
	}
}


unsigned int Project::uniqueIdWithPredef2(Project::IdSpaces idSpace, unsigned int id, std::vector<Project::IdMap> &maps, bool makeNewId){

	Q_ASSERT(idSpace != NUM_IdSpaces);

	unsigned int idOriginal = id;

	IdMap &idMap = maps[idSpace];

	//check if the id has already a other reference in this id space
	if(idMap.m_vicusToNandrad.find(id) != idMap.m_vicusToNandrad.end() && !makeNewId)
		return idMap.m_vicusToNandrad[id];

	if(!makeNewId){
		if(std::find(idMap.m_ids.begin(), idMap.m_ids.end(),id) != idMap.m_ids.end())
			return id;
	}

	//find a unique id

	bool foundId = false;
	while (!foundId) {

		//check if the id exists already in a other NANDRAD model
		for(unsigned int i=0; i<NUM_IdSpaces; ++i){
			std::vector<unsigned int> &vec = maps[(IdSpaces)i].m_ids;
			for(unsigned int j=0; j<vec.size(); ++j){
				if(vec[j] == id){
					foundId = true;
					break;
				}
			}
		}
		if(foundId){
			++id;
			foundId = false;
		}
		else{
			//exit loop
			foundId = true;
		}
	}
	if(idOriginal != id)
		idMap.m_vicusToNandrad[idOriginal] = id;
	idMap.m_ids.push_back(id);

	return id;
	//if(idMap.m_vicusToNandrad.find(id) == idMap.m_vicusToNandrad.end())
	//	idMap.m_vicusToNandrad[id] = uniqueIdWithPredef(idMap.m_ids, id);
	//return idMap.m_vicusToNandrad[id];
}



} // namespace VICUS

