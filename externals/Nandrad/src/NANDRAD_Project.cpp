/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#include "NANDRAD_Project.h"
#include "NANDRAD_KeywordList.h"

#include <algorithm>
#include <set>

#include <IBK_messages.h>
#include <IBK_assert.h>

#include <tinyxml.h>

#include "NANDRAD_Utilities.h"

namespace NANDRAD {


/*! Test function that checks that all objects in the given vector have different m_id parameters. */
template <typename T>
void checkForUniqueIDs(const std::vector<T> & vec, const char * const typeIDString) {
	FUNCID(NANDRAD::checkForUniqueIDs);

	std::set<unsigned int> usedIDs;

	for (const T & t : vec) {
		if (usedIDs.find(t.m_id) != usedIDs.end())
			throw IBK::Exception(IBK::FormatString("Duplicate ID #%1 in list of type '%2'.")
								 .arg(t.m_id).arg(typeIDString), FUNC_ID);
		usedIDs.insert(t.m_id);
	}
}

/*! Test function that checks that all objects in the given vector have different m_id parameters.
	This function accepts comparison between other geometric objects and fills id space in usedIDs
	container.
*/
template <typename T>
void checkForUniqueGeometryIDs(const std::vector<T> & vec, std::set<unsigned int> & usedIDs) {
	FUNCID(NANDRAD::checkForUniqueGeometryIDs);

	for (const T & t : vec) {
		if (usedIDs.find(t.m_id) != usedIDs.end())
			throw IBK::Exception(IBK::FormatString("Duplicate model/object ID #%1.")
								 .arg(t.m_id), FUNC_ID);
		usedIDs.insert(t.m_id);
	}
}

/*! Test function that checks that all objects in the given vector have different m_id parameters. */
template <typename T>
void checkForUniqueNames(const std::vector<T> & vec, const char * const typeIDString) {
	FUNCID(NANDRAD::checkForUniqueNames);

	std::set<std::string> usedNames;

	for (const T & t : vec) {
		if (usedNames.find(t.m_name) != usedNames.end())
			throw IBK::Exception(IBK::FormatString("Duplicate name '%1' in list of type '%2'.")
								 .arg(t.m_name).arg(typeIDString), FUNC_ID);
		usedNames.insert(t.m_name);
	}
}


void Project::readXML(const IBK::Path & filename) {
	FUNCID(Project::readXML);

	TiXmlDocument doc;
	IBK::Path filenamePath(filename);
	TiXmlElement * xmlElem = openXMLFile(m_placeholders, filenamePath, "NandradProject", doc);
	if (!xmlElem)
		return; // empty project, this means we are using only defaults

	// we read our subsections from this handle
	TiXmlHandle xmlRoot = TiXmlHandle(xmlElem);

	// Directory Placeholders
	xmlElem = xmlRoot.FirstChild("DirectoryPlaceholders").Element();
	if (xmlElem) {
		readDirectoryPlaceholdersXML(xmlElem);
		// check for duplicate sections
		xmlElem = xmlRoot.Child("DirectoryPlaceholders", 1).Element();
		if (xmlElem != nullptr) {
			throw IBK::Exception(IBK::FormatString("Duplicate section 'DirectoryPlaceholders'."), FUNC_ID);
		}
	}

	// add the project directory to the placeholders map
	m_placeholders[IBK::PLACEHOLDER_PROJECT_DIR] = filenamePath.parentPath();

	try {
		xmlElem = xmlRoot.FirstChild("Project").Element();
		if (xmlElem) {
			readXMLPrivate(xmlElem);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading project '%1'.").arg(filename), FUNC_ID);
	}

	// check uniqueness of all IDs in all separate id spaces
	// Note: all objects in the ID-based object lists have an m_id data member. Hence, we can implement
	//       the check in a template function.

	checkForUniqueIDs(m_materials, "Material");
	checkForUniqueIDs(m_constructionTypes, "ConstructionType");
	checkForUniqueIDs(m_zones, "Zone");
	checkForUniqueIDs(m_constructionInstances, "ConstructionInstance");

	if(!m_hydraulicNetworks.empty()) {
		// put all flow elements into a common id space
		std::vector<HydraulicNetworkElement> flowElems;
		// check all networks
		for(const HydraulicNetwork &nw : m_hydraulicNetworks) {
			checkForUniqueIDs(nw.m_components, "HydraulicNetworkComponent");
			checkForUniqueIDs(nw.m_pipeProperties, "HydraulicNetworkPipeProperties");
			// at the moment just store elements into vector
			if(!flowElems.empty())
				flowElems.insert(flowElems.end(), nw.m_elements.begin(), nw.m_elements.end());
			else
				flowElems = nw.m_elements;
		}
		// enforce a unique id for flow elements of all networks
		checkForUniqueIDs(flowElems,"HydraulicNetworkElement");
	}

	// we check for duplicate object lists here, because these are referenced by name, rather than ID
	checkForUniqueNames(m_outputs.m_grids, "OutputGrid");
	checkForUniqueNames(m_objectLists, "ObjectList");


	// The following objects share a common ID space and must be unique:
	// - Zone
	// - ConstructionInstance
	// - EmbeddedObjects
	// - Location.Sensors
	std::set<unsigned int> usedIDs;

	try {
		// check all zone ids against each other and against
		// all entries in usedIDs container
		checkForUniqueGeometryIDs(m_zones, usedIDs);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Zone ID was previously used (Zones, Constructions, Sensors share same ID space).", FUNC_ID);
	}
	try {
		// check all constructionInstances for unique ids and store
		// it in usedIDs container
		checkForUniqueGeometryIDs(m_constructionInstances, usedIDs);
		// loop through all constructions and check for unique ids of
		// embedded objects
		for(const ConstructionInstance &conInstance : m_constructionInstances) {
			checkForUniqueGeometryIDs(conInstance.m_embeddedObjects, usedIDs);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Construction ID was previously used (Zones, Constructions, Sensors share same ID space).", FUNC_ID);
	}
	try {
		// check all sensor ids against each other and against
		// all entries in usedIDs container
		checkForUniqueGeometryIDs(m_location.m_sensors, usedIDs);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Sensor ID was previously used (Zones, Constructions, Sensors share same ID space).", FUNC_ID);
	}

	// Note:
	// - the check for duplicate output definitions is done during output initialization

	// uniqueness check for models
	m_models.checkForUniqueIDs();
}


void Project::writeXML(const IBK::Path & filename) const {
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "NandradProject" );
	doc.LinkEndChild(root);

	root->SetAttribute("fileVersion", VERSION);

	// update interface-zone comment
	for (ConstructionInstance & con : const_cast<std::vector<ConstructionInstance>&>(m_constructionInstances)) {
		con.m_interfaceA.updateComment(m_zones);
		con.m_interfaceB.updateComment(m_zones);
	}

	writeDirectoryPlaceholdersXML(root);
	writeXMLPrivate(root);

	doc.SaveFile( filename.c_str() );
}


void Project::initDefaults() {
	m_solverParameter.initDefaults();
	m_simulationParameter.initDefaults();
	m_schedules.initDefaults();
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
			m_placeholders[attrib->Value()] = std::string(e->GetText());
		}
		else {
			IBK::IBK_Message(IBK::FormatString(
					"Unknown element '%1' in DirectoryPlaceholders section.").arg(name), IBK::MSG_WARNING);
		}

	}
}


void Project::writeDirectoryPlaceholdersXML(TiXmlElement * parent) const {

	if (m_placeholders.empty() ||
		(m_placeholders.size() == 1 && m_placeholders.begin()->first == IBK::PLACEHOLDER_PROJECT_DIR))
		return;

	TiXmlComment::addComment(parent,
		"DirectoryPlaceholders section defines strings to be substituted with directories");

	TiXmlElement * e1 = new TiXmlElement( "DirectoryPlaceholders" );
	parent->LinkEndChild( e1 );

	for (std::map<std::string, IBK::Path>::const_iterator it = m_placeholders.begin();
		it != m_placeholders.end(); ++it)
	{
		if (it->first != IBK::PLACEHOLDER_PROJECT_DIR)
			TiXmlElement::appendSingleAttributeElement(e1, "Placeholder",
													   "name", it->first,
													   it->second.str());
	}

	TiXmlComment::addSeparatorComment(parent);
}


void Project::mergeSameConstructions() {
	FUNCID(Project::mergeSameConstructions);

	IBK::IBK_Message("Merging redundant construction instances\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

#if 0

	// general concept:
	// - all modifications are done in temporary memory first, and then applied collectively
	//   to project data structure

	// algorithm steps

	// 1. first determine which construction may not be merged due to other models referencing
	//    the construction (e.g. for internal loads, or when the zone has view factors).
	//    Store these construction IDs in a set,  since they will be probably much less than the
	//    original construction list, and we need a fast search for such construction IDs later on.

	// 2. create a vector with "merge" entries for all constructions (index = -1 > not merged; index >= 0 -> construction index)

	// 3. process all construction (outer loop) -> current construction
	//      skip all constructions that are in the "excluded" set
	//      skip all constructions that are marked "merged"
	//      skip constructions with near zero area (they are ignored in the calculation anyways) - DISCUSS THREASHOLD!
	//      (alternatively, try to merge those constructions, since then outputs will be available for these constructions as well
	//      due to reverse mapping)
	//      process all constructions with index > current construction
	//        skip all constructions that are in the "excluded" set
	//        skip all constructions that are marked "merged"
	//        compare constructions based on:
	//          - construction type
	//          - boundary conditions
	//          - connected zones (mind, that construction may be flipped)
	//        if constructions are the same, record possible merge:
	//          - label construction to be mergable with current construction (store index of current construction)
	//          - mark current construction to be merged with self (store index of current construction in position of current construction)

	// 4. create new construction instances for merged constructions:
	//    - process "merged" record vector and for each entry "marked as self":
	//      - create a new construction instance as copy
	//      - store index to new construction in "merged" vector in place of original construction
	//    - process "merged" record vector and for each entry "marked as merged":
	//      - if mergable-construction index is in the range of the new construction instances, simply add an entry in the merge-info table
	//      - for other constructions, look up index in place of mergabe construction (this will now point to a newly created instance)
	//        -> add own cross section area to area of construction
	//        -> add own embedded objects to the list of embedded objects in the merged construction
	//        -> add merge entry
	//        -> change "merge" marker to hold the index of the merged new construction instance
	//    - copy all construction instances that are not merged into separate vector

	// Now the merge-entry vector holds for each merged constructionn the index of the associated newly create construction instance.

	// 5. update embedded object references
	//   process all embedded objects
	//     look up the construction index from referenced ID
	//     check merge-entry vector if construction was merged
	//     if so, record a "embedded object modified" info and adjust the construction instance ID in the embedded object

	// the algorithm works on indexed constructions, so create a vector with all construction instances
	std::vector<ConstructionInstance> conInstances;
	unsigned int numConInstances = m_constructionInstances.size();
	conInstances.reserve(numConInstances); // avoid re-allocs during copy
	for (const std::pair<unsigned int, ConstructionInstance> & c : m_constructionInstances)
		conInstances.push_back(c.second);


	// create a set of "excluded" construction instances

	std::set<unsigned int> excludedConstructionsIDs;
	const char * const ModelsThatReferenceConstructionInstances[] = {
		"ConstructionInternalSourceModel", "UnderfloorHeatingModel"
	};
	// loop over all models
	for (const Model & m : m_models.m_models) { // C++11 range loop here (I'm lazy)
		for (const char * modelID : ModelsThatReferenceConstructionInstances) {
			if (m.m_modelIdName == modelID) {
				// retrieve matching object list for all model feedbacks
				for (const ImplicitModelFeedback & mf : m.m_implicitModelFeedbacks) {
					const std::string & ol = mf.m_objectList;
					std::map<std::string, ObjectList>::const_iterator it = m_objectLists.m_objectLists.find(ol);
					if (it != m_objectLists.m_objectLists.end()) { // should be an assert, but we haven't checked the object structure yet, will be done during model init
						// process all construction instances and remember those that are in the given ID range
						for (const ConstructionInstance & c : conInstances) {
							if (it->second.m_filterID.contains(c.m_id))
								excludedConstructionsIDs.insert(c.m_id);
						}
					}
				}
				// add construction IDs of all referenced constructions to 'excludedConstructionsIDs'
			}
		}
	}
	// loop over all construction instances and their interfaces and associated zones: if the zone has view factors, the
	// construction instance may not be merged
	for (unsigned int i=0; i<numConInstances; ++i) {
		// exclude construction with FMU export or import
		if (!conInstances[i].m_FMUExportReferences.empty() ||
			!conInstances[i].m_FMUImportReferences.empty()) {

			excludedConstructionsIDs.insert(conInstances[i].m_id);
			continue;
		}
		// process all interfaces
		for (const Interface & iface : conInstances[i].m_interfaces) {
			if (iface.m_zoneId == 0) {
				// check if outside has shading factors
				if (!m_location.m_shadingFactorFileName.str().empty()) {
					excludedConstructionsIDs.insert(conInstances[i].m_id);
					break;
				}
			}
			std::map< unsigned int, Zone>::const_iterator it = m_zones.find(iface.m_zoneId);
			if (it == m_zones.end()) continue; // invalid zone ID -> the error will be triggered later again
			// check if zone has view factors
			if (!it->second.m_viewFactors.empty())
				excludedConstructionsIDs.insert(conInstances[i].m_id);
		}
	}

	const char * const ModelsWithViewFactors[] = {
		"HeatingPointSourceModel", "LightingPointSourceModel"
	};
	// search for point soucrs with view factors
	// loop over all models
	for (const Model & m : m_models.m_models) { // C++11 range loop here (I'm lazy)
		for (const char * modelID : ModelsWithViewFactors) {
			if (m.m_modelIdName == modelID) {
				// retrieve matching object list for all model feedbacks

				std::map <unsigned int, double> viewFactors;
				try {
					extractViewFactorsFromModel(m, viewFactors);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error checking #%1 with id #%2!")
						.arg(modelID).arg(m.m_id), FUNC_ID);
				}
				// process all construction instances that neighbor interfaces from view factors or embedded objects
				for (const ConstructionInstance & c : conInstances) {
					const std::vector<Interface> &ifaces = c.m_interfaces;
					const std::vector<EmbeddedObject> &objects = c.m_embeddedObjects;
					// check all interfaces
					for (const Interface & i : ifaces) {
						if (viewFactors.find(i.m_id) != viewFactors.end()) {
							excludedConstructionsIDs.insert(c.m_id);
							break;
						}
					}
					// check all embedded objects
					for (const EmbeddedObject & o : objects) {
						if (viewFactors.find(o.m_id) != viewFactors.end()) {
							excludedConstructionsIDs.insert(c.m_id);
							break;
						}
					}
				}
				// add construction IDs of all referenced constructions to 'excludedConstructionsIDs'
			}
		}
	}

	// create vector with merge markers
	std::vector<int> canBeMergedTo(numConInstances, -1); // initialized with -1 - not merged


	// process all construction instances
	for (unsigned int i=0; i<numConInstances; ++i) {
		const ConstructionInstance & currentConInstance = conInstances[i]; // readability improvement
		// skip construction instance, if already merged
		if (canBeMergedTo[i] != -1)
			continue;
		// skip construction instance, if in excluded
		if (excludedConstructionsIDs.find(currentConInstance.m_id) != excludedConstructionsIDs.end())
			continue;

		// now process all subsequent construction instances
		for (unsigned int j=i+1; j<numConInstances; ++j) {
			const ConstructionInstance & c = conInstances[j]; // readability improvement
			// skip construction instance, if already merged
			if (canBeMergedTo[j] != -1)
				continue;
			// skip construction instance, if in excluded
			if (excludedConstructionsIDs.find(c.m_id) != excludedConstructionsIDs.end())
				continue;

			// now we compare currentConInstance with c
			if (c.behavesLike(currentConInstance)) {
				// can merge constructions, record merge-self marker
				canBeMergedTo[i] = i;
				// record merge-other marker
				canBeMergedTo[j] = i;
			}
		}
	}

	struct t_mergeRecord {
		int IDMergedFrom;
		int IDMergedTo;
		double originalCrossSection;
	};
	std::vector<t_mergeRecord> mergeRecord;

	// now process all possible merge constructions and create new copies at the end
	for (unsigned int i=0; i<numConInstances; ++i) {
		if (canBeMergedTo[i] == (int)i) {
			canBeMergedTo[i] = conInstances.size(); // remember index of newly created construction instance
			// create new instance
			conInstances.push_back(conInstances[i]);
			ConstructionInstance & c = conInstances.back();
			// set new ID
			c.m_id += 660000000; // TODO : improve this! - ID must not be become larger than 4,294,967,295
			// record merge
			t_mergeRecord r;
			r.IDMergedTo = c.m_id;
			r.IDMergedFrom = conInstances[i].m_id;
			r.originalCrossSection = conInstances[i].m_para[ConstructionInstance::CP_AREA].value;
			mergeRecord.push_back(r);
			IBK::IBK_Message(IBK::FormatString("Merging construction id %1 -> id %2\n").arg(r.IDMergedFrom).arg(r.IDMergedTo), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		}
	}

	std::vector<ConstructionInstance> remainingOriginalConstructions;
	remainingOriginalConstructions.reserve(numConInstances);

	// now do the actual merging
	for (unsigned int i=0; i<numConInstances; ++i) {
		// skip all constructions that cannot be merged
		if (canBeMergedTo[i] == -1) {
			remainingOriginalConstructions.push_back(conInstances[i]);
			continue;
		}
		// skip constructions that were already merged (actually copied to new construction instances)
		if (canBeMergedTo[i] >= (int)numConInstances)
			continue;

		// lookup merge-to construction
		int mergePartnerIndex = canBeMergedTo[i];
		int newConInstanceIndex = canBeMergedTo[mergePartnerIndex];

		ConstructionInstance & c = conInstances[newConInstanceIndex];
		// add own cross section area
		c.m_para[ConstructionInstance::CP_AREA].value += conInstances[i].m_para[ConstructionInstance::CP_AREA].value;
		// add own embedded objects
		c.m_embeddedObjects.insert(c.m_embeddedObjects.end(), conInstances[i].m_embeddedObjects.begin(), conInstances[i].m_embeddedObjects.end());

		// record merge
		t_mergeRecord r;
		r.IDMergedTo = c.m_id;
		r.IDMergedFrom = conInstances[i].m_id;
		r.originalCrossSection = conInstances[i].m_para[ConstructionInstance::CP_AREA].value;
		IBK::IBK_Message(IBK::FormatString("Merging construction id %1 -> id %2\n").arg(r.IDMergedFrom).arg(r.IDMergedTo), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		mergeRecord.push_back(r);
	}

	// now finally replace original constructionInstances
	m_constructionInstances.clear();

	// first original construction instances (not merged)
	for (const ConstructionInstance & c : remainingOriginalConstructions) {
		m_constructionInstances[c.m_id] = c;
	}
	// now the newly created construction instances
	for (unsigned int i=numConInstances; i<conInstances.size(); ++i) {
		const ConstructionInstance & c = conInstances[i];
		m_constructionInstances[c.m_id] = c;
	}
	if (numConInstances != conInstances.size())
		IBK::IBK_Message( IBK::FormatString("Construction instance count reduced from %1 to %2\n").arg(numConInstances).arg(m_constructionInstances.size()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

#endif
}
// ----------------------------------------------------------------------------


} // namespace NANDRAD
