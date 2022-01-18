#include "EP_Project.h"

#include "EP_IDFParser.h"

namespace EP {

void Project::readIDF(const IDFParser & idfData) {
	FUNCID(Project::readIDF);

	m_version = idfData.m_version;

	try {
		// now read all sections one after another
		// attention all class names in lower case
		idfData.readClassObj("buildingsurface:detailed", m_bsd);
		idfData.readClassObj("fenestrationsurface:detailed", m_fsd);
		idfData.readClassObj("zone", m_zones);
		idfData.readClassObj("material", m_materials);
		idfData.readClassObj("material:nomass", m_materials); // data get's added to m_materials
		idfData.readClassObj("windowmaterial:simpleglazingsystem", m_windowMaterial);
		idfData.readClassObj("construction", m_constructions);
		idfData.readClassObj("shading:building:detailed", m_shadingBuildingDetailed);
		idfData.readClassObj("windowproperty:frameanddivider", m_frames);
		idfData.readClassObj("windowmaterial:blind", m_shadingSystems);
		idfData.readClassObj("windowmaterial:shade", m_shadingSystems);
		idfData.readClassObj("windowmaterial:screen", m_shadingSystems);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error extracting data from IDF.", FUNC_ID);
	}
}


void Project::writeIDF(const IBK::Path & filename) {
#if 0

#if defined(_WIN32)
	std::ofstream out(IBK::UTF8ToANSIString(filename.str()));
#else
	std::ofstream out(filename.c_str());
#endif

	//out.open(filename.str());

	out << "! Created by Dirk & Stephan & Katja" << std::endl<< std::endl;
	out << "Version, 8.3;" << std::endl<< std::endl;

	m_version = EP::Version::VN_8_3 ;

	out << writeClassObj(m_bsd, m_version);
	out << writeClassObj(m_zones, m_version);
	out << writeClassObj(m_windowMaterial, m_version);
	out << writeClassObj(m_materials, m_version);
	out << writeClassObj(m_constructions, m_version);
	out << writeClassObj(m_fsd, m_version);
	//out.close();
//	for (size_t i=0; i<m_bsd.size(); ++i) {
//		const BuildingSurfaceDetailed &bsd = m_bsd[i];
//		std::string str;
//		bsd.write(str, m_version);
//		out << str;
//	}
//	for (size_t i=0; i<m_zones.size(); ++i) {
//		const Zone &zone = m_zones[i];
//		std::string str;
//		zone.write(str, m_version);
//		out << str;
//	}
//	for (size_t i=0; i<m_windowMaterial.size(); ++i) {
//		const WindowMaterial &windowMat = m_windowMaterial[i];
//		std::string str;
//		windowMat.write(str, m_version);
//		out << str;
//	}
//	for (size_t i=0; i<m_materials.size(); ++i) {
//		const Material &material = m_materials[i];
//		std::string str;
//		material.write(str, m_version);
//		out << str;
//	}
//	for (size_t i=0; i<m_fsd.size(); ++i) {
//		const FenestrationSurfaceDetailed &fsd = m_fsd[i];
//		std::string str;
//		fsd.write(str, m_version);
//		out << str;
//	}
//	for (size_t i=0; i<m_constructions.size(); ++i) {
//		const Construction &construct = m_constructions[i];
//		std::string str;
//		construct.write(str, m_version);
//		out << str;
//	}
	//....
	std::cout << "Finished writing IDF." << std::endl;
#endif
}



Project Project::mergeProjects(const Project & other,const IBKMK::Vector3D &shift)
{
	FUNCID(Project::mergeProjects);
	std::set<std::string>	allZoneNames, otherZoneNames,
							allBSD, otherBSD,
							allFSD, otherFSD,
							allMaterials, allConstructions;
	std::map<std::string, std::string>	oldToNewZoneName, oldToNewBSDName, oldToNewMaterialName,
										oldToNewConstrName;
	//new prj
	Project prj;
	prj.m_zones = m_zones;
	prj.m_bsd = m_bsd;
	prj.m_fsd = m_fsd;
	prj.m_materials = m_materials;
	prj.m_windowMaterial = m_windowMaterial;
	prj.m_constructions = m_constructions;
	prj.m_version = m_version;

	//add zones of this project
	for (const Zone &zA : m_zones)
		allZoneNames.insert(zA.m_name);

	//add zones of other project
	for (const Zone &zA : other.m_zones)
		otherZoneNames.insert(zA.m_name);

	//add zones of other project
	for (const Zone &otherObj : other.m_zones) {
		//insert unique zone name in set
		std::string newName = findId(allZoneNames, otherZoneNames, otherObj.m_name);
		//check if old name has changed
		//if change link old to new name
		prj.m_zones.push_back(otherObj);
		if(newName != otherObj.m_name){
			oldToNewZoneName[otherObj.m_name] = newName;
			prj.m_zones.back().m_name = newName;
		}
	}


	//add bsd of this project
	for (const BuildingSurfaceDetailed &bsd : prj.m_bsd)
		allBSD.insert(bsd.m_name);

	//add bsd of other project
	for (const BuildingSurfaceDetailed &bsd : other.m_bsd)
		otherBSD.insert(bsd.m_name);


	//add materials of this project
	for (EP::Material &mat : m_materials) {
		allMaterials.insert(mat.m_name);
	}
	for (EP::WindowMaterial &mat : m_windowMaterial) {
		allMaterials.insert(mat.m_name);
	}

	//add and compare materials
	for (const EP::Material &mat : other.m_materials) {
		bool foundSameMat = false;
		for (size_t i=0; i<m_materials.size(); ++i) {
			if(mat.behavesLike(m_materials[i])){
				oldToNewMaterialName[mat.m_name] = m_materials[i].m_name;
				foundSameMat=true;
				break;
			}
		}
		if(!foundSameMat){
			prj.m_materials.push_back(mat);
			std::string newName = findId(allMaterials, mat.m_name);
			if(newName != mat.m_name){
				oldToNewMaterialName[mat.m_name] = newName;
				prj.m_materials.back().m_name = newName;
			}

		}
	}
	for (const EP::WindowMaterial &mat : other.m_windowMaterial) {
		bool foundSameMat = false;
		for (size_t i=0; i<m_windowMaterial.size(); ++i) {
			if(mat.behavesLike(m_windowMaterial[i])){
				oldToNewMaterialName[mat.m_name] = m_windowMaterial[i].m_name;
				foundSameMat=true;
				break;
			}
		}
		if(!foundSameMat){
			prj.m_windowMaterial.push_back(mat);
			std::string newName = findId(allMaterials, mat.m_name);
			if(newName != mat.m_name){
				oldToNewMaterialName[mat.m_name] = newName;
				prj.m_windowMaterial.back().m_name = newName;
			}
		}
	}

	//add and compare constructions
	for (const Construction &constr : prj.m_constructions)
		allConstructions.insert(constr.m_name);

	for (size_t i=0; i<other.m_constructions.size(); ++i) {
		bool foundSameConstr = false;

		Construction C2 = other.m_constructions[i];
		for (EP::Construction &constr : m_constructions) {
			Construction C1 = constr;
			if(constr.behavesLike(other.m_constructions[i],
							   m_materials, other.m_materials,
								  m_windowMaterial, other.m_windowMaterial)){
				oldToNewConstrName[other.m_constructions[i].m_name] = constr.m_name;
				foundSameConstr = true;
				break;
			}
		}
		if(!foundSameConstr){

			Construction constrCopy = other.m_constructions[i];

			for(size_t iLay = 0; iLay < constrCopy.m_layers.size(); ++iLay){
				std::string nameLayer = constrCopy.m_layers[iLay];
				if(oldToNewMaterialName.find(nameLayer) != oldToNewMaterialName.end())
					constrCopy.m_layers[iLay] = oldToNewMaterialName[nameLayer];
			}

			std::string newName = findId(allConstructions, constrCopy.m_name);
			prj.m_constructions.push_back(constrCopy);
			if(newName != constrCopy.m_name){
				oldToNewConstrName[constrCopy.m_name] = newName;
				prj.m_constructions.back().m_name = newName;
			}
		}
	}



	//add bsd of other project
	for (const BuildingSurfaceDetailed &otherObj : other.m_bsd) {
		prj.m_bsd.push_back(otherObj);
		BuildingSurfaceDetailed &bsd = prj.m_bsd.back();

		//change zone name
		if(oldToNewZoneName.find(bsd.m_zoneName) != oldToNewZoneName.end())
			bsd.m_zoneName = oldToNewZoneName[bsd.m_zoneName];

		//change construction name if necessary
		if(oldToNewConstrName.find(bsd.m_constructionName) != oldToNewConstrName.end())
			bsd.m_constructionName = oldToNewConstrName[bsd.m_constructionName];

		//check if bsdName exist in old names
		std::string bsdName="", bsdObjName="";
		if(oldToNewBSDName.find(bsd.m_name) != oldToNewBSDName.end()){
			bsdName = oldToNewBSDName[bsd.m_name];
			bsd.m_name = bsdName;
		}
		bool hasSurfaceLink = bsd.m_outsideBoundaryCondition == BuildingSurfaceDetailed::OutsideBoundaryCondition::OC_Surface;
		if(hasSurfaceLink && oldToNewBSDName.find(bsd.m_outsideBoundaryConditionObject) != oldToNewBSDName.end()){
			bsdObjName = oldToNewBSDName[bsd.m_outsideBoundaryConditionObject];
			bsd.m_outsideBoundaryConditionObject = bsdObjName;
		}

		if(bsdName.empty()){
			//insert unique name in set
			std::string newName = findId(allBSD, otherBSD, bsd.m_name);
			if(newName != bsd.m_name){
				oldToNewBSDName[bsd.m_name] = newName;
				bsd.m_name = newName;
			}
		}
		if(hasSurfaceLink && bsdObjName.empty()){
			std::string newName = findId(allBSD, otherBSD, bsd.m_name);
			if(newName != bsd.m_outsideBoundaryConditionObject ){
				oldToNewBSDName[bsd.m_outsideBoundaryConditionObject ] = newName;
				bsd.m_outsideBoundaryConditionObject  = newName;
			}
		}

		//shift the surface
		for(IBKMK::Vector3D &p : bsd.m_polyline)
			p += shift;
	}

	//add fsd of this project
	for(FenestrationSurfaceDetailed &fsd : prj.m_fsd )
		allFSD.insert(fsd.m_name);

	//add fsd of other project
	for(const FenestrationSurfaceDetailed &fsd : other.m_fsd )
		otherFSD.insert(fsd.m_name);


	for (const FenestrationSurfaceDetailed &otherObj : other.m_fsd) {
		prj.m_fsd.push_back(otherObj);
		FenestrationSurfaceDetailed &fsd = prj.m_fsd.back();

		//change construction name if necessary
		if(oldToNewConstrName.find(fsd.m_constructionName) != oldToNewConstrName.end())
			fsd.m_constructionName = oldToNewConstrName[fsd.m_constructionName];

		if(oldToNewBSDName.find(fsd.m_bsdName) != oldToNewBSDName.end())
			fsd.m_bsdName = oldToNewBSDName[fsd.m_bsdName];

		std::string newName = findId(allFSD, otherFSD, fsd.m_name);
		if(newName != fsd.m_name)
			fsd.m_name = newName;


		//shift the surface
		for(IBKMK::Vector3D &p : fsd.m_polyline)
			p += shift;
	}

	//check function
	if(hasProjectDuplicateIds()){
		throw IBK::Exception(IBK::FormatString("Merged project has duplicate names."), FUNC_ID);
	}

	return prj;
}

std::string Project::findId(std::set<std::string> &ids, std::string id){

	while (true){
		if(ids.find(id) == ids.end()){
			ids.insert(id);
			return	id;
		}
		else{
			size_t pos = id.find_last_of("_");
			if(pos != std::string::npos){
				unsigned int number = 0;
				try {
					number = IBK::string2val<unsigned int>(id.substr(pos+1));
					id = id.substr(0,pos+1) + IBK::val2string(++number);
				} catch (std::exception &ex) {
					id += "_1";
				}
			}
			else {
				id+= "_0";
			}
		}
	}
}

std::string Project::findId(std::set<std::string> &ids,const std::set<std::string> &otherIds, std::string id){

	while (true){
		if(ids.find(id) == ids.end() && otherIds.find(id) == otherIds.end()){
			ids.insert(id);
			return	id;
		}
		else{
			size_t pos = id.find_last_of("_");
			if(pos != std::string::npos){
				unsigned int number = 0;
				try {
					number = IBK::string2val<unsigned int>(id.substr(pos+1));
					id = id.substr(0,pos+1) + IBK::val2string(++number);
				} catch (std::exception &ex) {
					id += "_1";
				}
			}
			else {
				id+= "_0";
			}
		}
	}
}

bool Project::hasProjectDuplicateIds()
{
	FUNCID(Project::hasProjectDuplicateIds);

	std::set<std::string>	bsdName, bsdNameRefObj;
	std::map<std::string,unsigned int>	namesToCount, namesToCount2;
	for (auto e : m_bsd) {

		if(bsdName.find(e.m_name) == bsdName.end()){
			bsdName.insert(e.m_name);
		}
		else {
			throw IBK::Exception(IBK::FormatString("This name: '%1' exists several times.").arg(e.m_name),FUNC_ID);
		}


		if(e.m_outsideBoundaryCondition == EP::BuildingSurfaceDetailed::OC_Surface){

			if(bsdNameRefObj.find(e.m_outsideBoundaryConditionObject) == bsdNameRefObj.end()){
				bsdNameRefObj.insert(e.m_outsideBoundaryConditionObject);
			}
			else {
				throw IBK::Exception(IBK::FormatString("This name: '%1' exists several times. (Reference/Link).\n"
													   "Object name: '%2'").arg(e.m_outsideBoundaryConditionObject).arg(e.m_name),FUNC_ID);
			}
		}
		else {
			if(bsdNameRefObj.find(e.m_name) == bsdNameRefObj.end()){
				bsdNameRefObj.insert(e.m_name);
			}
			else {
				throw IBK::Exception(IBK::FormatString("This name: '%1' is linked in other objects, but it is a no-linked surface object.\n").arg(e.m_name),FUNC_ID);
			}
		}

		continue;






		unsigned int count = namesToCount[e.m_name];
		++count;
		if(count>1)
		{
			return true;
		}
		namesToCount[e.m_name] = count;
		if(e.m_outsideBoundaryCondition == EP::BuildingSurfaceDetailed::OC_Surface){
			unsigned int count2 = namesToCount2[e.m_outsideBoundaryConditionObject];
			++count2;
			namesToCount2[e.m_outsideBoundaryConditionObject]=count2;
			if(count2>1)
			{
				return true;
			}
		}
	}
	return  false;
}



}
