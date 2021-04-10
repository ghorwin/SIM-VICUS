/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "VICUS_Project.h"

#include <algorithm>
#include <set>
#include <fstream>

#include <IBK_messages.h>
#include <IBK_assert.h>
#include <IBK_Exception.h>

#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

#include "VICUS_Constants.h"

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
		throw IBK::Exception( IBK::FormatString("Cannot open input file '%1' for reading").arg(filename), FUNC_ID);
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
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (VICUS::Room & r : bl.m_rooms)
				for (VICUS::Surface & s : r.m_surfaces)
					s.m_componentInstance = nullptr;
	// update pointers
	for (VICUS::ComponentInstance & ci : m_componentInstances) {
		// lookup surfaces
		ci.m_sideASurface = surfaceByID(ci.m_sideASurfaceID);
		if (ci.m_sideASurface != nullptr) {
			// check that no two components reference the same surface
			if (ci.m_sideASurface->m_componentInstance != nullptr) {
				IBK::IBK_Message(IBK::FormatString("Surface %1 is referenced by multiple component instances!")
								 .arg(ci.m_sideASurfaceID), IBK::MSG_ERROR, FUNC_ID);
			}
			else {
				ci.m_sideASurface->m_componentInstance = &ci;
			}
		}

		ci.m_sideBSurface = surfaceByID(ci.m_sideBSurfaceID);
		if (ci.m_sideBSurface != nullptr) {
			// check that no two components reference the same surface
			if (ci.m_sideBSurface->m_componentInstance != nullptr) {
				IBK::IBK_Message(IBK::FormatString("Surface %1 is referenced by multiple component instances!")
								 .arg(ci.m_sideBSurfaceID), IBK::MSG_ERROR, FUNC_ID);
			}
			else {
				ci.m_sideBSurface->m_componentInstance = &ci;
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

bool selectionCheck(const VICUS::Object & o, bool takeSelected, bool takeVisible) {
	bool selCheck = takeSelected ? o.m_selected : true;
	bool visCheck = takeVisible ? o.m_visible : true;
	return (selCheck && visCheck);
}

void Project::selectedBuildingObjects(std::set<const Object *> &selectedObjs, Object *obj) const {

	bool takeSelected = true;
	bool takeVisible = false;

	selectedObjs.clear();

	for (const VICUS::Building & b : m_buildings) {
		//
		VICUS::Building *bcheck = dynamic_cast<VICUS::Building *>(obj);
		if ( bcheck != nullptr) {
			 if (selectionCheck(b, takeSelected, takeVisible) )
				selectedObjs.insert(&b);
			 continue;
		}

		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {

			VICUS::BuildingLevel *blcheck = dynamic_cast<VICUS::BuildingLevel *>(obj);
			if ( blcheck != nullptr ) {
				if (selectionCheck(bl, takeSelected, takeVisible) )
					selectedObjs.insert(&bl);
				continue;
			}

			for (const VICUS::Room & r : bl.m_rooms) {

				VICUS::Room *rcheck = dynamic_cast<VICUS::Room *>(obj);
				if ( rcheck != nullptr){
					if (selectionCheck(r, takeSelected, takeVisible) )
						selectedObjs.insert(&r);
					continue;
				}

				for (const VICUS::Surface & s : r.m_surfaces) {

					VICUS::Surface *scheck = dynamic_cast<VICUS::Surface *>(obj);
					if ( scheck != nullptr ) {
						if ( selectionCheck(s, takeSelected, takeVisible) )
							selectedObjs.insert(&s);
						continue;
					}
				}
			}
		}
	}
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
	if (sg == SG_All) {
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


IBKMK::Vector3D Project::boundingBox(std::vector<const Surface*> &surfaces, IBKMK::Vector3D &center) {

	// store selected surfaces
	if ( surfaces.empty() )
		return IBKMK::Vector3D ( 0,0,0 );

	double maxX = std::numeric_limits<double>::lowest();
	double maxY = std::numeric_limits<double>::lowest();
	double maxZ = std::numeric_limits<double>::lowest();
	double minX = std::numeric_limits<double>::max();
	double minY = std::numeric_limits<double>::max();
	double minZ = std::numeric_limits<double>::max();
	for (const VICUS::Surface *s : surfaces ) {
		for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
			( v.m_x > maxX ) ? maxX = v.m_x : 0;
			( v.m_y > maxY ) ? maxY = v.m_y : 0;
			( v.m_z > maxZ ) ? maxZ = v.m_z : 0;

			( v.m_x < minX ) ? minX = v.m_x : 0;
			( v.m_y < minY ) ? minY = v.m_y : 0;
			( v.m_z < minZ ) ? minZ = v.m_z : 0;
		}
	}

	double dX = maxX - minX;
	double dY = maxY - minY;
	double dZ = maxZ - minZ;

	center.set( minX + 0.5*dX, minY + 0.5*dY, minZ + 0.5*dZ);

	// set bounding box;
	return IBKMK::Vector3D ( dX, dY, dZ );
}



Project::ConversionError::~ConversionError() {
}


void Project::generateNandradProject(NANDRAD::Project & p) const {

	// simulation settings
	p.m_simulationParameter = m_simulationParameter;

	// solver parameters
	p.m_solverParameter = m_solverParameter;

	// location settings
	p.m_location = m_location;
	// do we have a climate path?
	if (!m_location.m_climateFilePath.isValid())
		throw ConversionError(ConversionError::ET_MissingClimate,
							  tr("A climate data file is needed. Please select a climate data file!"));

	// directory placeholders
	for (const auto & placeholder : m_placeholders)
		p.m_placeholders[placeholder.first] = placeholder.second;


	// *** building geometry data and databases ***

	generateBuildingProjectData(p);


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

	// generate default output definitions, if requested
	if (m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].isEnabled()) {

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
		std::string refName;
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


		// now we have a name for the output grid, start generating default outputs
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


}


void Project::generateBuildingProjectData(NANDRAD::Project & p) const {

	// used to generate unique interface IDs
	unsigned int interfaceID = 1;

	// TODO : Andreas, for now, we generate interface IDs on the fly, which means they might be different when NANDRAD
	//        file is generated with small changes in the project. This will make it difficult to assign specific
	//        id associations with interfaces (once needed, maybe in FMUs?), so we may need to add interface IDs to
	//        the VICUS::ComponentInstance data structure.

	// we process all zones and buildings and create NANDRAD project data
	// we also transfer all database components


	// *** m_zones ***

	for (const VICUS::Building & b : m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				// first create a NANDRAD zone for the room
				NANDRAD::Zone z;
				z.m_id = r.m_id;
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
			}
		}
	}

	// *** m_constructionInstances ***

	// now process all components and generate construction instances
	for (const VICUS::ComponentInstance & ci : m_componentInstances) {
		// Note: component ID may be invalid or component may have been deleted from DB already
		const VICUS::Component * comp = element(m_embeddedDB.m_components, ci.m_componentID);
		if (comp == nullptr)
			throw ConversionError(ConversionError::ET_InvalidID,
				tr("Component ID #%1 is referenced from component instance with id #%2, but there is no such component.")
							 .arg(ci.m_componentID).arg(ci.m_id));

		// now generate a construction instance
		NANDRAD::ConstructionInstance cinst;
		cinst.m_id = ci.m_id;

		// store reference to construction type (i.e. to be generated from component)
		cinst.m_constructionTypeId = comp->m_idConstruction;

		// set construction instance parameters, area, orientation etc.

		const double SAME_DISTANCE_PARAMETER_ABSTOL = 1e-4;

		// we have either one or two surfaces associated
		if (ci.m_sideASurface != nullptr) {
			// get area of surface A
			double areaA = ci.m_sideASurface->m_geometry.area();
			// do we have surfaces at both sides?
			if (ci.m_sideBSurface != nullptr) {
				// have both
				double areaB = ci.m_sideBSurface->m_geometry.area();
				// check if both areas are approximately the same
				if (std::fabs(areaA - areaB) > SAME_DISTANCE_PARAMETER_ABSTOL) {
					throw ConversionError(ConversionError::ET_MismatchingSurfaceArea,
						tr("Component/construction #%1 references surfaces #%2 and #%3, with mismatching "
						   "areas %3 and %4 m2.")
								  .arg(ci.m_id).arg(ci.m_sideASurfaceID).arg(ci.m_sideBSurfaceID)
								  .arg(areaA).arg(areaB));
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
												   NANDRAD::ConstructionInstance::P_Inclination, s->m_geometry.inclination());
				NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Orientation, s->m_geometry.orientation());

				cinst.m_displayName = ci.m_sideASurface->m_displayName.toStdString();
			}
			// set area parameter (computed from side A, but if side B is given as well, the area is the same
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Area, areaA);
		}
		else {
			// we must have a side B surface, otherwise this is an invalid component instance
			if (ci.m_sideBSurface == nullptr)
				throw ConversionError(ConversionError::ET_InvalidID,
									  tr("Component instance #%1 does neither reference a valid surface on side A nor on side B.")
									  .arg(ci.m_id));

			const VICUS::Surface * s = ci.m_sideBSurface;

			// set parameters
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Inclination, s->m_geometry.inclination());
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Orientation, s->m_geometry.orientation());

			// set area parameter
			double area = ci.m_sideBSurface->m_geometry.area();
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Area, area);

			cinst.m_displayName = ci.m_sideBSurface->m_displayName.toStdString();
		}


		// now generate interfaces
		cinst.m_interfaceA = generateInterface(ci, ci.m_sideASurface, comp->m_idSideABoundaryCondition, interfaceID);
		cinst.m_interfaceB = generateInterface(ci, ci.m_sideBSurface, comp->m_idSideBBoundaryCondition, interfaceID);

		// add to list of construction instances
		p.m_constructionInstances.push_back(cinst);
	}


	// database elements

	// Construction instances reference construction types
	// Construction types reference materials

	// we have constructions and materials already in the embedded database, so we can just copy them over
	for (const VICUS::Material & m : m_embeddedDB.m_materials) {
		NANDRAD::Material matdata;
		matdata.m_id = m.m_id;
		matdata.m_displayName = m.m_displayName.string(IBK::MultiLanguageString::m_language, "en");

		// now transfer parameters - fortunately, they have the same keywords, what a coincidence :-)
		matdata.m_para[NANDRAD::Material::P_Density] = m.m_para[VICUS::Material::P_Density];
		matdata.m_para[NANDRAD::Material::P_HeatCapacity] = m.m_para[VICUS::Material::P_HeatCapacity];
		matdata.m_para[NANDRAD::Material::P_Conductivity] = m.m_para[VICUS::Material::P_Conductivity];

		// add to material list
		p.m_materials.push_back(matdata);
	}

	for (const VICUS::Construction & c : m_embeddedDB.m_constructions) {

		// now create a construction type
		NANDRAD::ConstructionType conType;
		conType.m_id = c.m_id;
		conType.m_displayName = c.m_displayName.string(IBK::MultiLanguageString::m_language, "en");

		for (const VICUS::MaterialLayer & ml : c.m_materialLayers) {
			NANDRAD::MaterialLayer mlayer;
			mlayer.m_matId = ml.m_matId;
			mlayer.m_thickness = ml.m_thickness.value;
			conType.m_materialLayers.push_back(mlayer);
		}

		// add to construction type list
		p.m_constructionTypes.push_back(conType);
	}


	// TODO Andreas, Dirk, Stephan: Transfer other database elements/generate NANDRAD data objects
}


void Project::generateNetworkProjectData(NANDRAD::Project & p) const {
	// TODO : Hauke

}


NANDRAD::Interface Project::generateInterface(const VICUS::ComponentInstance & ci,
											  const VICUS::Surface * s, unsigned int bcID,
											  unsigned int & interfaceID) const
{
	// no boundary condition ID? -> no interface
	if (bcID == VICUS::INVALID_ID)
		return NANDRAD::Interface();

	// lookup boundary condition definition in embedded database
	const VICUS::BoundaryCondition * bc = element(m_embeddedDB.m_boundaryConditions, bcID);
	if (bc == nullptr)
		throw ConversionError(ConversionError::ET_InvalidID,
			tr("Component #%1 has invalid boundary condition ID reference #%2.")
				.arg(ci.m_componentID).arg(s->m_id));
	if (!bc->isValid())
		throw ConversionError(ConversionError::ET_NotValid,
			tr("Boundary condition #%1 has invalid/incomplete parameters.").arg(bc->m_id));

	// do we have a surface to a zone?
	if (s != nullptr) {
		// get the zone that this interface is connected to
		const VICUS::Object * obj = s->m_parent;
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(obj);
		if (room == nullptr)
			throw ConversionError(ConversionError::ET_MissingParentZone,
				tr("Component instance #%1 references surface %2, which is not associated to a zone.")
					.arg(ci.m_id).arg(s->m_id));

		// generate a new interface to the zone, which always only includes heat conduction
		NANDRAD::Interface iface;
		iface.m_id = ++interfaceID;
		iface.m_zoneId = room->m_id;

		// only transfer heat conduction parameters
		iface.m_heatConduction = bc->m_heatConduction;
		return iface;
	}
	else {
		// no surface? must be an interface to the outside

		// generate a new interface to the zone, which always only includes heat conduction
		NANDRAD::Interface iface;
		iface.m_id = ++interfaceID;
		iface.m_zoneId = 0; // outside zone
		iface.m_heatConduction = bc->m_heatConduction;
		iface.m_solarAbsorption = bc->m_solarAbsorption;
		iface.m_longWaveEmission = bc->m_longWaveEmission;
		return iface;
	}
}



} // namespace VICUS

