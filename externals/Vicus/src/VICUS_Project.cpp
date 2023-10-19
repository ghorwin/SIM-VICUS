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

#include <QDateTime>

#include <algorithm>
#include <set>
#include <fstream>

#include <IBK_messages.h>
#include <IBK_assert.h>
#include <IBK_Exception.h>

#include <IBKMK_3DCalculations.h>

#include <NANDRAD_Utilities.h>
#include <NANDRAD_Project.h>

#include <tinyxml.h>

#include "VICUS_Constants.h"
#include "VICUS_utilities.h"
#include "VICUS_Drawing.h"


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
		pos = line.find("<Comment>");
		if (pos != std::string::npos) {
			size_t pos2 = line.find("</Comment>");
			if (pos2 != std::string::npos)
				m_projectInfo.m_comment = line.substr(pos + 9, pos2 - pos - 9);
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

	// clear existing grid planes
	m_viewSettings.m_gridPlanes.clear();

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

		// if we do not have a default grid, create it
		if (m_viewSettings.m_gridPlanes.empty()) {
			m_viewSettings.m_gridPlanes.push_back( VICUS::GridPlane(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,0,1),
																	IBKMK::Vector3D(1,0,0), QColor("white"), 200, 10 ) );
		}

		// update internal pointer-based links
		updatePointers();

		// set default colors for network objects
		for (VICUS::Network & net : m_geometricNetworks) {
			// updateColor is a const-function, this is possible since
			// the m_color property of edges and nodes is mutable
			net.setDefaultColors();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading project '%1'.").arg(filename), FUNC_ID);
	}
}

void Project::readXML(const QString & projectText) {
	FUNCID(Project::readXML);

	TiXmlDocument doc;
	TiXmlElement * xmlElem = NANDRAD::openXMLText(projectText.toStdString(), "VicusProject", doc);
	if (!xmlElem)
		return; // empty project, this means we are using only defaults

	// we read our subsections from this handle
	TiXmlHandle xmlRoot = TiXmlHandle(xmlElem);

	// clear existing grid planes
	m_viewSettings.m_gridPlanes.clear();

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

		// if we do not have a default grid, create it
		if (m_viewSettings.m_gridPlanes.empty()) {
			m_viewSettings.m_gridPlanes.push_back( VICUS::GridPlane(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,0,1),
																	IBKMK::Vector3D(1,0,0), QColor("white"), 200, 10 ) );
		}

		// update internal pointer-based links
		updatePointers();

		// set default colors for network objects
		for (VICUS::Network & net : m_geometricNetworks) {
			// updateColor is a const-function, this is possible since
			// the m_color property of edges and nodes is mutable
			net.setDefaultColors();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading project from text."), FUNC_ID);
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


void Project::clean() {
	// TODO : Implement cleanup, i.e. removal of ComponentInstances and SubSurfaceComponentInstance that no longer
	//        reference valid surfaces etc.
}

void Project::addChildSurface(const VICUS::Surface &s) {
    for (VICUS::Surface & childSurf : const_cast<std::vector<VICUS::Surface> &>(s.childSurfaces()) ) {
        addAndCheckForUniqueness(&childSurf);
        childSurf.m_componentInstance = nullptr;

        addChildSurface(childSurf);
    }
}

void Project::updatePointers() {
	FUNCID(Project::updatePointers);

	m_objectPtr.clear();
	m_objectPtr[VICUS::INVALID_ID] = nullptr; // this will help us detect invalid IDs in the data model

	// update hierarchy
	for (VICUS::Building & b : m_buildings)
		b.updateParents();

	for (VICUS::Building & b : m_buildings) {
		addAndCheckForUniqueness(&b);
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			addAndCheckForUniqueness(&bl);
			for (VICUS::Room & r : bl.m_rooms) {
				addAndCheckForUniqueness(&r);
				for (VICUS::Surface & s : r.m_surfaces) {
					addAndCheckForUniqueness(&s);
					s.m_componentInstance = nullptr;
					for (VICUS::SubSurface & sub : const_cast<std::vector<VICUS::SubSurface> &>(s.subSurfaces()) ) {
						addAndCheckForUniqueness(&sub);
						sub.m_subSurfaceComponentInstance = nullptr;
					}

                    addChildSurface(s);
				}
			}
		}
	}

	// update pointers
	for (VICUS::ComponentInstance & ci : m_componentInstances) {
		// lookup surfaces
		ci.m_sideASurface = surfaceByID(ci.m_idSideASurface);
		if (ci.m_sideASurface != nullptr) {
			// check that no two components reference the same surface
			if (ci.m_sideASurface->m_componentInstance != nullptr) {
				IBK::IBK_Message(IBK::FormatString("Surface id: %1 name: '%2' is referenced by multiple component instances!")
								 .arg(ci.m_idSideASurface).arg(ci.m_sideASurface->m_displayName.toStdString()), IBK::MSG_ERROR, FUNC_ID);

			}
			else {
				ci.m_sideASurface->m_componentInstance = &ci;
			}
		}

		ci.m_sideBSurface = surfaceByID(ci.m_idSideBSurface);
		if (ci.m_sideBSurface != nullptr) {
			// check that no two components reference the same surface
			if (ci.m_sideBSurface->m_componentInstance != nullptr) {
				IBK::IBK_Message(IBK::FormatString("Surface id: %1 name: '%2' is referenced by multiple component instances!")
								 .arg(ci.m_idSideBSurface).arg(ci.m_sideBSurface->m_displayName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
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
		if (ci.m_idSupplySystem != VICUS::INVALID_ID) {
			for (VICUS::SupplySystem & s : m_embeddedDB.m_supplySystems)
				if (s.m_id == ci.m_idSupplySystem)
					ci.m_supplySystem = &s;
		}
	}

	// update pointers in subsurfaces
	for (VICUS::SubSurfaceComponentInstance & ci : m_subSurfaceComponentInstances) {
		// lookup surfaces
		ci.m_sideASubSurface = subSurfaceByID(ci.m_idSideASurface);
		if (ci.m_sideASubSurface != nullptr) {
			// check that no two components reference the same surface
			if (ci.m_sideASubSurface->m_subSurfaceComponentInstance != nullptr) {
				IBK::IBK_Message(IBK::FormatString("Sub-Surface id: %1 name: '%2' is referenced by multiple component instances!")
								 .arg(ci.m_idSideASurface).arg(ci.m_sideASubSurface->m_displayName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
			}
			else {
				ci.m_sideASubSurface->m_subSurfaceComponentInstance = &ci;
			}
		}

		ci.m_sideBSubSurface = subSurfaceByID(ci.m_idSideBSurface);
		if (ci.m_sideBSubSurface != nullptr) {
			// check that no two components reference the same surface
			if (ci.m_sideBSubSurface->m_subSurfaceComponentInstance != nullptr) {
				IBK::IBK_Message(IBK::FormatString("Sub-Surface id: %1 name: '%2' is referenced by multiple component instances!")
								 .arg(ci.m_idSideBSurface).arg(ci.m_sideBSubSurface->m_displayName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
			}
			else {
				ci.m_sideBSubSurface->m_subSurfaceComponentInstance = &ci;
			}
		}

	}

	// networks

	for (VICUS::Network & n : m_geometricNetworks) {
		addAndCheckForUniqueness(&n);
		for (VICUS::NetworkNode & nod : n.m_nodes)
			addAndCheckForUniqueness(&nod);
		n.updateNodeEdgeConnectionPointers();
	}

	// plain geometry

	for (VICUS::Surface & s : m_plainGeometry.m_surfaces)
		addAndCheckForUniqueness(&s);


	// network edges

	// Note: VICUS::NetworkEdge objects do not save their unique IDs in the project file.
	//       Hence, we need to assign unique IDs on the first time the object is created,
	//       or, when the object pointer list is first updated.
	// CAUTION: This should always be the last step in this function, otherwise we may assign object ids here,
	// which are used by other objects that have not been added yet.
	for (VICUS::Network & n : m_geometricNetworks) {
		for (VICUS::NetworkEdge & e : n.m_edges) {
			if (e.m_id == VICUS::INVALID_ID)
				e.m_id = nextUnusedID();
			addAndCheckForUniqueness(&e);
		}
		n.updateNodeEdgeConnectionPointers();
	}

	for (VICUS::Drawing &d : m_drawings) {
		d.updateParents();
		addAndCheckForUniqueness(&d);
		for (VICUS::DrawingLayer &dl : d.m_drawingLayers) {
			addAndCheckForUniqueness(&dl);
		}
	}

}


unsigned int Project::nextUnusedID() const {
	if(m_objectPtr.empty())
		return 1;

	// the last element in m_objectPtr is always INVALID_ID
	auto it = m_objectPtr.rbegin();
	Q_ASSERT(it->first == VICUS::INVALID_ID);
	// we return the ID following that element before that
	++it;
	// do we have such an element?
	if (it == m_objectPtr.rend())
		return 1; // no element yet in the vector; this will hardly occur, only when loading a completely empty project

	return it->first + 1;
}


Object * Project::objectById(unsigned int id) {
	auto objPtrIt = m_objectPtr.find(id);
	if (objPtrIt != m_objectPtr.end())
		return objPtrIt->second;
	else
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
                        selectChildSurfaces(selectedObjs, s, takeSelected, takeVisible);
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

	// Drawing objects
	if (sg & SG_Drawing) {
		for (const VICUS::Drawing & d : m_drawings) {

			for (const VICUS::DrawingLayer & dl : d.m_drawingLayers) {
				if (selectionCheck(dl, takeSelected, takeVisible))
					selectedObjs.insert(&dl);
			}

			if (selectionCheck(d, takeSelected, takeVisible))
				selectedObjs.insert(&d);
		}
    }

}

void Project::selectChildSurfaces(std::set<const Object *> &selectedObjs, const Surface &s, bool takeSelected, bool takeVisible) const {
    for (const VICUS::Surface &childSurf : s.childSurfaces()) {
        if (selectionCheck(childSurf, takeSelected, takeVisible))
            selectedObjs.insert(&childSurf);
        selectChildSurfaces(selectedObjs, childSurf, takeSelected, takeVisible);
    }
}

bool Project::selectedSubSurfaces(std::vector<const SubSurface *> & subSurfaces, const Project::SelectionGroups & sg) const {
	std::set<const Object*> objs;
	selectObjects(objs, sg, true, true);

	// Note: sg = SG_Building will only select surfaces in the building hierarchy
	//       sg = SG_All will also select anonymous surfaces
	//       sg = SG_Network does nothing (network doesn't have any surfaces)

	subSurfaces.clear();
	for (const Object * o : objs) {
		const SubSurface * ss = dynamic_cast<const SubSurface *>(o);
		if (ss != nullptr)
			subSurfaces.push_back(ss);
	}

	return !subSurfaces.empty();
}


bool Project::selectedSurfaces(std::vector<const Surface*> &surfaces, const VICUS::Project::SelectionGroups &sg) const {
	std::set<const Object*> objs;
	selectObjects(objs, sg, true, true); // get all selected and visible

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

QString Project::newUniqueBuildingName(const QString & baseName) const	{
	std::set<QString> names;
	for (const VICUS::Building & b : m_buildings)
		names.insert(b.m_displayName);
	return uniqueName(baseName, names);
}

QString Project::newUniqueBuildingLevelName(const QString & baseName) const {
	std::set<QString> names;
	for (const VICUS::Building & b : m_buildings)
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
			names.insert(bl.m_displayName);
	return uniqueName(baseName, names);
}

QString Project::newUniqueRoomName(const QString & baseName) const {
	std::set<QString> names;
	for (const VICUS::Building & b : m_buildings)
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (const VICUS::Room & r : bl.m_rooms)
				names.insert(r.m_displayName);
	return uniqueName(baseName, names);
}

QString Project::newUniqueSurfaceName(const QString & baseName) const {
	std::set<QString> names;
	for (const VICUS::Building & b : m_buildings)
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (const VICUS::Room & r : bl.m_rooms)
				for (const VICUS::Surface & s : r.m_surfaces)
					names.insert(s.m_displayName);
	return uniqueName(baseName, names);
}

QString Project::newUniqueSubSurfaceName(const QString & baseName) const {
	std::set<QString> names;
	for (const VICUS::Building & b : m_buildings)
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (const VICUS::Room & r : bl.m_rooms)
				for (const VICUS::Surface & s : r.m_surfaces)
					for (const VICUS::SubSurface & sub : s.subSurfaces())
						names.insert(sub.m_displayName);
	return uniqueName(baseName, names);
}


template <typename t>
void drawingBoundingBox(const VICUS::Drawing &d,
						const std::vector<t> &drawingObjects,
						IBKMK::Vector3D &upperValues,
						IBKMK::Vector3D &lowerValues,
						bool transformPoints = true) {
	// FUNCID(Project::boundingBox);

	// store selected surfaces
	if (drawingObjects.empty())
		return;

	// process all drawings
	for (const t &drawObj : drawingObjects) {
		const VICUS::DrawingLayer *dl = dynamic_cast<const VICUS::DrawingLayer *>(drawObj.m_parentLayer);

		Q_ASSERT(dl != nullptr);

		if (!dl->m_visible)
			continue;

		const std::vector<IBKMK::Vector2D> &verts = drawObj.points2D();
		for (const IBKMK::Vector2D &p : verts) {

			// Create Vector from start and end point of the line,
			// add point of origin to each coordinate and calculate z value
			double zCoordinate = drawObj.m_zPosition * Z_MULTIPLYER + d.m_origin.m_z;
			IBKMK::Vector3D p1 = IBKMK::Vector3D(p.m_x + d.m_origin.m_x,
												 p.m_y + d.m_origin.m_y,
												 zCoordinate);

			QVector3D vec1(p1.m_x, p1.m_y, p1.m_z);

			if (transformPoints) {
				// scale Vector with selected unit
				p1 *= d.m_scalingFactor;

				// rotate Vectors
				vec1 = d.m_rotationMatrix.toQuaternion() * vec1;
			}

			upperValues.m_x = std::max(upperValues.m_x, (double)vec1.x());
			upperValues.m_y = std::max(upperValues.m_y, (double)vec1.y());
			upperValues.m_z = std::max(upperValues.m_z, (double)vec1.z());

			lowerValues.m_x = std::min(lowerValues.m_x, (double)vec1.x());
			lowerValues.m_y = std::min(lowerValues.m_y, (double)vec1.y());
			lowerValues.m_z = std::min(lowerValues.m_z, (double)vec1.z());
		}
	}
}


IBKMK::Vector3D Project::boundingBox(const std::vector<const Drawing *> & drawings,
									 const std::vector<const Surface*> &surfaces,
									 const std::vector<const SubSurface*> &subsurfaces,
									 IBKMK::Vector3D &center,
									 bool transformPoints)
{
	// NOTE: We do not reuse the other boundingBox() function, because this implementation is much faster.

	// store selected surfaces
	if ( surfaces.empty() && subsurfaces.empty() && drawings.empty() )
		return IBKMK::Vector3D ( 0,0,0 );

	IBKMK::Vector3D lowerValues(std::numeric_limits<double>::max(),
								std::numeric_limits<double>::max(),
								std::numeric_limits<double>::max());
	IBKMK::Vector3D upperValues(std::numeric_limits<double>::lowest(),
								std::numeric_limits<double>::lowest(),
								std::numeric_limits<double>::lowest());

	// process all surfaces
	for (const VICUS::Surface *s : surfaces ) {
		if (!s->geometry().polygon3D().isValid())
			continue;
		// TODO : also skip invisible surfaces?
		s->geometry().polygon3D().enlargeBoundingBox(lowerValues, upperValues);
	}

	// process all sub-surfaces
	for (const VICUS::SubSurface *sub : subsurfaces ) {
		const VICUS::Surface *s = dynamic_cast<const VICUS::Surface *>(sub->m_parent);
		// parent geometry must be valid and correctly triangulated
		if (!s->geometry().isValid()) continue;
		// now find our selected subsurfaec
		for (unsigned int i=0; i<s->subSurfaces().size(); ++i) {
			if (&(s->subSurfaces()[i]) != sub) continue;
			// TODO : also skip invisible sub-surfaces?
			for ( const IBKMK::Vector3D & v : s->geometry().holeTriangulationData()[i].m_vertexes ) {
				upperValues.m_x = std::max(upperValues.m_x, v.m_x);
				upperValues.m_y = std::max(upperValues.m_y, v.m_y);
				upperValues.m_z = std::max(upperValues.m_z, v.m_z);

				lowerValues.m_x = std::min(lowerValues.m_x, v.m_x);
				lowerValues.m_y = std::min(lowerValues.m_y, v.m_y);
				lowerValues.m_z = std::min(lowerValues.m_z, v.m_z);
			}
		}
	}

	for (const VICUS::Drawing *drawing : drawings) {
		drawingBoundingBox<VICUS::Drawing::Arc>(*drawing, drawing->m_arcs, upperValues, lowerValues, transformPoints);
		drawingBoundingBox<VICUS::Drawing::Circle>(*drawing, drawing->m_circles, upperValues, lowerValues, transformPoints);
		drawingBoundingBox<VICUS::Drawing::Ellipse>(*drawing, drawing->m_ellipses, upperValues, lowerValues, transformPoints);
		drawingBoundingBox<VICUS::Drawing::Line>(*drawing, drawing->m_lines, upperValues, lowerValues, transformPoints);
		drawingBoundingBox<VICUS::Drawing::PolyLine>(*drawing, drawing->m_polylines, upperValues, lowerValues, transformPoints);
		drawingBoundingBox<VICUS::Drawing::Point>(*drawing, drawing->m_points, upperValues, lowerValues, transformPoints);
		drawingBoundingBox<VICUS::Drawing::Solid>(*drawing, drawing->m_solids, upperValues, lowerValues, transformPoints);
		drawingBoundingBox<VICUS::Drawing::Text>(*drawing, drawing->m_texts, upperValues, lowerValues, transformPoints);
		drawingBoundingBox<VICUS::Drawing::LinearDimension>(*drawing, drawing->m_linearDimensions, upperValues, lowerValues, transformPoints);
	}

	// center point of bounding box
	center = 0.5*(lowerValues+upperValues);
	// difference between upper and lower values gives bounding box (dimensions of selected geometry)
	return (upperValues-lowerValues);
}


IBKMK::Vector3D Project::boundingBox(const std::vector<const Drawing *> & drawings, const std::vector<const NetworkEdge *> & edges, const std::vector<const NetworkNode *> & nodes, IBKMK::Vector3D & center) {

	if (nodes.empty() && edges.empty() && drawings.empty())
		return IBKMK::Vector3D ( 0,0,0 );

	IBKMK::Vector3D lowerValues(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	IBKMK::Vector3D upperValues(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest());

	for (const VICUS::NetworkNode *node: nodes) {
		upperValues.m_x = std::max(upperValues.m_x, node->m_position.m_x);
		upperValues.m_y = std::max(upperValues.m_y, node->m_position.m_y);
		upperValues.m_z = std::max(upperValues.m_z, node->m_position.m_z);

		lowerValues.m_x = std::min(lowerValues.m_x, node->m_position.m_x);
		lowerValues.m_y = std::min(lowerValues.m_y, node->m_position.m_y);
		lowerValues.m_z = std::min(lowerValues.m_z, node->m_position.m_z);
	}

	for (const VICUS::NetworkEdge *edge: edges) {
		upperValues.m_x = std::max(upperValues.m_x, edge->m_node1->m_position.m_x);
		upperValues.m_y = std::max(upperValues.m_y, edge->m_node1->m_position.m_y);
		upperValues.m_z = std::max(upperValues.m_z, edge->m_node1->m_position.m_z);
		upperValues.m_x = std::max(upperValues.m_x, edge->m_node2->m_position.m_x);
		upperValues.m_y = std::max(upperValues.m_y, edge->m_node2->m_position.m_y);
		upperValues.m_z = std::max(upperValues.m_z, edge->m_node2->m_position.m_z);

		lowerValues.m_x = std::min(lowerValues.m_x, edge->m_node1->m_position.m_x);
		lowerValues.m_y = std::min(lowerValues.m_y, edge->m_node1->m_position.m_y);
		lowerValues.m_z = std::min(lowerValues.m_z, edge->m_node1->m_position.m_z);
		lowerValues.m_x = std::min(lowerValues.m_x, edge->m_node2->m_position.m_x);
		lowerValues.m_y = std::min(lowerValues.m_y, edge->m_node2->m_position.m_y);
		lowerValues.m_z = std::min(lowerValues.m_z, edge->m_node2->m_position.m_z);
	}

	// center point of bounding box
	center = 0.5*(lowerValues+upperValues);

	// difference between upper and lower values gives bounding box (dimensions of selected geometry)
	return (upperValues-lowerValues);
}


IBKMK::Vector3D Project::boundingBox(std::vector<const Surface *> & surfaces,
									 std::vector<const SubSurface *> & subsurfaces,
									 IBKMK::Vector3D & center,
									 const IBKMK::Vector3D & offset, const IBKMK::Vector3D & xAxis,
									 const IBKMK::Vector3D & yAxis, const IBKMK::Vector3D & zAxis)
{
	FUNCID(Project::boundingBox);

	// store selected surfaces
	if ( surfaces.empty() && subsurfaces.empty())
		return IBKMK::Vector3D (0.,0.,0.);

	// TODO : Review this
	double maxX = std::numeric_limits<double>::lowest();
	double maxY = std::numeric_limits<double>::lowest();
	double maxZ = std::numeric_limits<double>::lowest();
	double minX = std::numeric_limits<double>::max();
	double minY = std::numeric_limits<double>::max();
	double minZ = std::numeric_limits<double>::max();
	for (const VICUS::Surface *s : surfaces ) {
		try {
			s->polygon3D().vertexes();
		}  catch (...) {
			IBK::IBK_Message(IBK::FormatString("Surface '%1' does not contain vertexes!")
							 .arg(s->m_displayName.toStdString()), IBK::MSG_WARNING, FUNC_ID);
			continue;
		}

		for ( IBKMK::Vector3D v : s->polygon3D().vertexes() ) {

			IBKMK::Vector3D vLocal, point;

			IBKMK::lineToPointDistance(offset, xAxis, v, vLocal.m_x, point);
			IBKMK::lineToPointDistance(offset, yAxis, v, vLocal.m_y, point);
			IBKMK::lineToPointDistance(offset, zAxis, v, vLocal.m_z, point);

			v = vLocal;

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
		if (!s->geometry().isValid()) continue;
		for (unsigned int i=0; i<s->subSurfaces().size(); ++i) {
			if (&(s->subSurfaces()[i]) == sub) {
				for ( IBKMK::Vector3D v : s->geometry().holeTriangulationData()[i].m_vertexes ) {

					IBKMK::Vector3D vLocal, point;

					IBKMK::lineToPointDistance(offset, xAxis, v, vLocal.m_x, point);
					IBKMK::lineToPointDistance(offset, yAxis, v, vLocal.m_y, point);
					IBKMK::lineToPointDistance(offset, zAxis, v, vLocal.m_z, point);

					v = vLocal;

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

	center.set( offset.m_x + (minX + 0.5*dX) * xAxis.m_x + (minY + 0.5*dY) * yAxis.m_x + (minZ + 0.5*dZ) * zAxis.m_x ,
				offset.m_y + (minX + 0.5*dX) * xAxis.m_y + (minY + 0.5*dY) * yAxis.m_y + (minZ + 0.5*dZ) * zAxis.m_y ,
				offset.m_z + (minX + 0.5*dX) * xAxis.m_z + (minY + 0.5*dY) * yAxis.m_z + (minZ + 0.5*dZ) * zAxis.m_z );

	// set bounding box;
	return IBKMK::Vector3D ( dX, dY, dZ );
}


bool Project::connectSurfaces(double maxDist, double maxAngle, const std::set<const Surface *> & selectedSurfaces,
							  std::vector<ComponentInstance> & newComponentInstances)
{
	// TODO : Dirk, implement algorithm
	qDebug() << "Not implemented, yet";


	return false;
}


void Project::addAndCheckForUniqueness(Object * o) {
	FUNCID(Project::addAndCheckForUniqueness);
	// try to insert the object's id into the map
	auto insertResult = m_objectPtr.insert(std::make_pair(o->m_id, o));
	if (!insertResult.second) {
		if (o->m_id == VICUS::INVALID_ID)
			throw IBK::Exception(IBK::FormatString("Object %1 does not have a valid ID.").arg(o->info().toStdString()), FUNC_ID);
		std::string objInfo = insertResult.first->second->info().toStdString();
		std::string duplicateInfo = o->info().toStdString();
		throw IBK::Exception(IBK::FormatString("Duplicate ID %1 in data model: First object with this ID is %2, "
											   "duplicate is %3").arg(o->m_id).arg(objInfo).arg(duplicateInfo), FUNC_ID);
	}
}


void Project::generateNandradProject(NANDRAD::Project & p, QStringList & errorStack, const std::string & nandradProjectPath) const {
	FUNCID(Project::generateNandradProject);

	// project information
	p.m_projectInfo.m_comment.append(IBK::FormatString("Project file: %1\n%2")
									 .arg(IBK::Path(nandradProjectPath).withoutExtension() + ".vicus")
									 .arg(m_projectInfo.m_comment).str());
	p.m_projectInfo.m_created = QDateTime::currentDateTime().toString(Qt::DateFormat::TextDate).toStdString();

	// simulation settings
	p.m_simulationParameter = m_simulationParameter;

	// solver parameters
	p.m_solverParameter = m_solverParameter;

	// location settings
	p.m_location = m_location;

	// create a standard sensor global horizontal radiation
	{
		// TODO Dirk das ist erstmal nur vorübergehend
		// es muss dafür ein Dialog im SV... erstellt werden und später die Werte übergeben werden

		//create sensors for all the 5 orientations
		for(unsigned int i = 0; i < NUM_ST; i++){
			NANDRAD::Sensor s;
			if(i == ST_Horizontal)
				s.createSensor(2000000 + i, 0, 0);
			else
				s.createSensor(2000000 + i, 90 * (i - 1),90);

			p.m_location.m_sensors.push_back(s);
		}
	}

	// do we have a climate path?
	if (!m_location.m_climateFilePath.isValid()) {
		errorStack.push_back(tr("A climate data file is needed. Please select a climate data file!"));
		throw IBK::Exception("Error during climate data conversion.", FUNC_ID);
	}

	// *** building geometry data and databases ***

	// Map of VICUS surface/sub-surface ids to NANDRAD construction instance/embedded object ids.
	// These ids are kept in the header of the shading file for later replacement of the ids.
	std::map<unsigned int, unsigned int>				surfaceIdsVicusToNandrad;
	std::vector<RoomMapping>	roomMappings;
	std::map<unsigned int, unsigned int>	componentInstanceMappings;

	try {
		generateBuildingProjectData(QString(IBK::Path(nandradProjectPath).filename().withoutExtension().c_str()),
								   p, errorStack, surfaceIdsVicusToNandrad, roomMappings, componentInstanceMappings);
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("%1\nCould not export NANDRAD project.").arg(ex.what()), FUNC_ID);
	}

	if (!errorStack.isEmpty())
		throw IBK::Exception("Error during building data generation.", FUNC_ID);

	if(!exportMappingTable(IBK::Path(nandradProjectPath), roomMappings, true)){
		errorStack.push_back(tr("Mapping table export failed!"));
		throw IBK::Exception("Mapping table export failed!", FUNC_ID);
	}

	// replace vicus ids in shading file with nandrad ids
	IBK::Path shadingFilePath;
	if (generateShadingFactorsFile(surfaceIdsVicusToNandrad, IBK::Path(nandradProjectPath), shadingFilePath))
		p.m_location.m_shadingFactorFilePath = IBK::Path(shadingFilePath);
	else {
		errorStack.push_back(tr("Shading factor file data invalid/outdated, try re-generating shading factor data!"));
		throw IBK::Exception("Error during shading factor file generation.", FUNC_ID);
	}

	// *** generate network data ***

	if (!m_geometricNetworks.empty()) {
		generateNetworkProjectData(p, errorStack, nandradProjectPath);
		if (!errorStack.isEmpty())
			throw IBK::Exception("Error during network data conversion.", FUNC_ID);
	}

	// *** outputs ***

	// transfer output grids
	p.m_outputs.m_grids = m_outputs.m_grids;

	// transfer options
	p.m_outputs.m_binaryFormat = m_outputs.m_flags[VICUS::Outputs::F_BinaryFormat];
	p.m_outputs.m_timeUnit = m_outputs.m_timeUnit;

	// generate output grid, if needed
	std::string refName;
	if (m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].isEnabled() ||
		m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs].isEnabled() ||
		m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultNetworkSummationModels].isEnabled() )
	{
		// we need at least one hourly grid
		if (p.m_outputs.m_grids.empty()) {
			// create one, if not yet existing
			NANDRAD::OutputGrid og;
			og.m_name = refName = tr("Hourly values").toStdString();
			NANDRAD::Interval iv;
			NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_Start, 0);
			NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_StepSize, 1);
			og.m_intervals.push_back(iv);
			p.m_outputs.m_grids.push_back(og);
		}
		// remember reference name of first grid
		refName = p.m_outputs.m_grids.front().m_name;
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

		{
			NANDRAD::OutputDefinition od;
			od.m_gridName = refName;
			od.m_quantity = "IdealHeatingLoad";
			od.m_timeType = NANDRAD::OutputDefinition::OTT_MEAN;
			od.m_objectListName = objectListAllZones;
			p.m_outputs.m_definitions.push_back(od);
		}

		{
			NANDRAD::OutputDefinition od;
			od.m_gridName = refName;
			od.m_quantity = "IdealCoolingLoad";
			od.m_timeType = NANDRAD::OutputDefinition::OTT_MEAN;
			od.m_objectListName = objectListAllZones;
			p.m_outputs.m_definitions.push_back(od);
		}

		{
			NANDRAD::OutputDefinition od;
			od.m_gridName = refName;
			od.m_quantity = "ConvectiveEquipmentHeatLoad";
			od.m_timeType = NANDRAD::OutputDefinition::OTT_MEAN;
			od.m_objectListName = objectListAllZones;
			p.m_outputs.m_definitions.push_back(od);
		}

		{
			NANDRAD::OutputDefinition od;
			od.m_gridName = refName;
			od.m_quantity = "ConvectiveLightingHeatLoad";
			od.m_timeType = NANDRAD::OutputDefinition::OTT_MEAN;
			od.m_objectListName = objectListAllZones;
			p.m_outputs.m_definitions.push_back(od);
		}

		{
			NANDRAD::OutputDefinition od;
			od.m_gridName = refName;
			od.m_quantity = "ConvectivePersonHeatLoad";
			od.m_timeType = NANDRAD::OutputDefinition::OTT_MEAN;
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

		// generate needed object lists and outputs for construction instances
		{
			// objlist
			NANDRAD::ObjectList olCI;
			olCI.m_name = "objListCI";
			olCI.m_filterID.setEncodedString("*");
			olCI.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
			p.m_objectLists.push_back(olCI);

			// output
			NANDRAD::OutputDefinition od;
			od.m_gridName = refName;
			od.m_quantity = "ActiveLayerThermalLoad";
			od.m_timeType = NANDRAD::OutputDefinition::OTT_MEAN;
			od.m_objectListName = olCI.m_name;
			p.m_outputs.m_definitions.push_back(od);
		}



		// object lists for location TODO check if exist !!!
		// make standard output for temperature direct and diffuse radiation
		std::string olName = "Location";
		{
			NANDRAD::ObjectList ol;
			ol.m_name = olName;
			ol.m_filterID.setEncodedString("*");
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
			p.m_objectLists.push_back(ol);
		}

		{
			NANDRAD::OutputDefinition od;
			od.m_gridName = refName;
			od.m_quantity = "Temperature";
			od.m_objectListName = olName;
			p.m_outputs.m_definitions.push_back(od);
		}

		{
			NANDRAD::OutputDefinition od;
			od.m_gridName = refName;
			od.m_quantity = "SWRadDirectNormal";
			od.m_timeType = NANDRAD::OutputDefinition::OTT_MEAN;
			od.m_objectListName = olName;
			p.m_outputs.m_definitions.push_back(od);
		}

		{
			NANDRAD::OutputDefinition od;
			od.m_gridName = refName;
			od.m_quantity = "SWRadDiffuseHorizontal";
			od.m_timeType = NANDRAD::OutputDefinition::OTT_MEAN;
			od.m_objectListName = olName;
			p.m_outputs.m_definitions.push_back(od);
		}



	}


	// default network outputs
	if (m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs].isEnabled()) {

		NANDRAD::IDGroup ids;
		ids.m_allIDs = true;
		NANDRAD::ObjectList objList;
		objList.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
		objList.m_filterID = ids;
		objList.m_name = "all network objects";
		p.m_objectLists.push_back(objList);
		std::vector<std::string> quantities = {"FluidMassFlux", "OutletNodeTemperature",
											   "FlowElementHeatLoss", "PressureDifference",
											   "TemperatureDifference", "ElectricalPower", "OutletNodePressure", "COP"};
		for (const std::string &q: quantities){
			NANDRAD::OutputDefinition def;
			def.m_quantity = q;
			def.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;
			def.m_gridName = refName;
			def.m_objectListName = objList.m_name;
			p.m_outputs.m_definitions.push_back(def);
		}
	}

	// outputs for network default summation models (mean + integral)
	if (m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultNetworkSummationModels].isEnabled()) {
		for (const NANDRAD::ObjectList &objList: p.m_objectLists){
			if (objList.m_name == "Network Summation Models"){
				NANDRAD::OutputDefinition def;
				def.m_quantity = "TotalHeatLoad";
				def.m_timeType = NANDRAD::OutputDefinition::OTT_MEAN;
				def.m_gridName = refName;
				def.m_objectListName = objList.m_name;
				p.m_outputs.m_definitions.push_back(def);
				def.m_timeType = NANDRAD::OutputDefinition::OTT_INTEGRAL;
				p.m_outputs.m_definitions.push_back(def);
			}
		}
	}

	// now add our custom definitions
	unsigned int objectListCount = 1;
	for (const VICUS::OutputDefinition & def : m_outputs.m_definitions) {
		NANDRAD::OutputDefinition d;
		d.m_gridName = def.m_gridName;
		d.m_timeType = (NANDRAD::OutputDefinition::timeType_t)def.m_timeType;
		d.m_quantity = def.m_quantity;
		if (!def.m_vectorIds.empty()) {
			NANDRAD::IDGroup idGroup;
			idGroup.m_ids = std::set<unsigned int>(def.m_vectorIds.begin(), def.m_vectorIds.end());
			d.m_quantity += "[" + idGroup.encodedString() + "]";
		}

		// now generate an object list for this output - don't mind if we get duplicate object lists
		NANDRAD::ObjectList ol;
		ol.m_name ="Outputs-" + IBK::val2string(objectListCount++);
		ol.m_filterID.m_ids = std::set<unsigned int>(def.m_sourceObjectIds.begin(), def.m_sourceObjectIds.end());
		try {
			ol.m_referenceType = (NANDRAD::ModelInputReference::referenceType_t)NANDRAD::KeywordList::Enumeration("ModelInputReference::referenceType_t", def.m_sourceObjectType);
		} catch (...) {
			IBK::IBK_Message(IBK::FormatString("Invalid/unknown source object type '%1' in output definition.").arg(def.m_sourceObjectType),
							 IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
		}
		p.m_objectLists.push_back(ol);

		d.m_objectListName = ol.m_name;
		p.m_outputs.m_definitions.push_back(d);
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


#if 0


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

#endif



#if 0

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

#endif

} // namespace VICUS

