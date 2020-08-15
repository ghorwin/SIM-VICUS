/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#include "NM_ConstructionStatesModel.h"

#include <NANDRAD_ConstructionInstance.h>

#include "NM_KeywordList.h"

using namespace std;

namespace NANDRAD_MODEL {


void ConstructionStatesModel::setup(const NANDRAD::ConstructionInstance & con, const NANDRAD::SimulationParameter & simPara) {
	// copy wall model pointer
	m_wallModel = wallModel;
}

void ConstructionStatesModel::initResults(const std::vector<AbstractModel*> &  models) {

	const char * const FUNC_ID = "[ConstructionStatesModel::initResults]";

	try {
		std::string category = "ConstructionStatesModel::VectorValuedResults";

		// resize layer temperatures
		m_layerTemperatures.resize(m_wallModel->m_construction.m_materialLayers.size(),
			VectorValuedQuantityIndex::IK_Index);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("Error inializing results for %1 with id #%2!")
			.arg(ModelIDName()).arg(id()), FUNC_ID);
	}
}

void ConstructionStatesModel::initInputReferences(const std::vector<AbstractModel*> &  /*models*/) {

	std::string category	= "ConstructionStatesModel::InputReferences";
	// compose a reference to the global solver solution quantity (internal energy desnity)
	InputReference &yRef			= inputReference(InputRef_InternalEnergyDensity);
	yRef.m_referenceType			= NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	yRef.m_id						= id(); // solution variable is provided by the corresponding room balance model
	yRef.m_sourceName				= std::string("y");
	yRef.m_targetName				= KeywordList::Keyword(category.c_str(), InputRef_InternalEnergyDensity);
	yRef.m_constant					= true;

	// compose a reference to the temperatures
	InputReference &tempRef = inputReference(InputRef_Temperature);
	tempRef.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	tempRef.m_id = id(); // solution variable is provided by the corresponding room balance model
	tempRef.m_sourceName = KeywordList::Keyword(category.c_str(), InputRef_Temperature);
	tempRef.m_targetName = KeywordList::Keyword(category.c_str(), InputRef_Temperature);
	tempRef.m_constant = true;

	// compose a reference to heat sources
	InputReference &heatSourceRef = inputReference(InputRef_HeatSources);
	heatSourceRef.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	heatSourceRef.m_id = id(); // solution variable is provided by the corresponding room balance model
	heatSourceRef.m_sourceName = KeywordList::Keyword(category.c_str(), InputRef_HeatSources);
	heatSourceRef.m_targetName = KeywordList::Keyword(category.c_str(), InputRef_HeatSources);
	heatSourceRef.m_constant = true;
}


void ConstructionStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const
{
	DefaultModel::resultDescriptions(resDesc);

	std::string category = "ConstructionStatesModel::VectorValuedResults";
	// delete missing entries
	for (unsigned int i = 0; i < resDesc.size(); ++i) {
		if (resDesc[i].m_name != KeywordList::Keyword(category.c_str(), VVR_LayerTemperature) )
			continue;

		resDesc[i].resize(m_wallModel->m_construction.m_materialLayers.size());
	}

	category = "ConstructionStatesModel::InputReferences";
	// add wall temperatures
	QuantityDescription result;
	result.m_constant = true;
	result.m_name = KeywordList::Keyword(category.c_str(), InputRef_Temperature);
	result.m_description = KeywordList::Description(category.c_str(), InputRef_Temperature);
	result.m_unit = KeywordList::Unit(category.c_str(), InputRef_Temperature);
	result.m_size = m_wallModel->m_n;
	resDesc.push_back(result);

	// add heat sources
	result.m_name = KeywordList::Keyword(category.c_str(), InputRef_HeatSources);
	result.m_description = KeywordList::Description(category.c_str(), InputRef_HeatSources);
	result.m_unit = KeywordList::Unit(category.c_str(), InputRef_HeatSources);
	result.m_size = m_wallModel->m_n;
	resDesc.push_back(result);
}


void ConstructionStatesModel::resultValueRefs(std::vector<const double *> &res) const {

	// add surafce temperatures
	res.push_back(&m_wallModel->m_surfaceTemperatureA);
	res.push_back(&m_wallModel->m_surfaceTemperatureB);

	// add layer temperatures
	for (std::vector<double>::const_iterator it =
		m_layerTemperatures.begin(); it != m_layerTemperatures.end(); ++it)
		res.push_back(&(*it));

	// add all wall temperatures
	for(unsigned int i = 0; i < m_wallModel->
		m_states[WALL_MODEL::WallModel::STATE_TEMPERATURE].size(); ++i)
		res.push_back(&m_wallModel->m_states[WALL_MODEL::WallModel::STATE_TEMPERATURE][i]);
	// add all wall heat sources
	for (unsigned int i = 0; i < m_wallModel->
		m_sources[WALL_MODEL::WallModel::SOURCE_HEAT_PRODUCTION_RATE].size(); ++i)
		res.push_back(&m_wallModel->m_sources[WALL_MODEL::WallModel::SOURCE_HEAT_PRODUCTION_RATE][i]);
}


const double * ConstructionStatesModel::resultValueRef(const QuantityName & quantityName) const {

	std::string category = "ConstructionStatesModel::Results";
	// check scalar results
	if (quantityName == KeywordList::Keyword(category.c_str(), R_SurfaceTemperatureA))
	{
		// only scalar quantities are allowed any longer
		if (quantityName.index() != -1)
			return nullptr;

		return &m_wallModel->m_surfaceTemperatureA;
	}
	if (quantityName == KeywordList::Keyword(category.c_str(), R_SurfaceTemperatureB))
	{
		// only scalar quantities are allowed any longer
		if (quantityName.index() != -1)
			return nullptr;

		return &m_wallModel->m_surfaceTemperatureB;
	}

	category = "ConstructionStatesModel::VectorValuedResults";
	// check vector valued results
	if (quantityName.name() == KeywordList::Keyword(category.c_str(), VVR_LayerTemperature)) {
		// malformed index
		if (quantityName.index() == -1) {
			return nullptr;
		}
		//// invalid index
		if ((unsigned int)quantityName.index() >= m_layerTemperatures.size())
			return 0;
		return &m_layerTemperatures[(unsigned int)quantityName.index()];
	}

	category = "ConstructionStatesModel::InputReferences";
	// Additionally we provide a reference to the solution quantity.
	// This reference will be accessed by the corresponding RoomStatesModel.
	if (quantityName == KeywordList::Keyword(category.c_str(), InputRef_Temperature) )
	{
		return &m_wallModel->m_states[WALL_MODEL::WallModel::STATE_TEMPERATURE][0];
	}
	else if (quantityName == KeywordList::Keyword(category.c_str(), InputRef_HeatSources))
	{
		return &m_wallModel->m_sources[WALL_MODEL::WallModel::SOURCE_HEAT_PRODUCTION_RATE][0];
	}
	return nullptr;
}

int ConstructionStatesModel::update()
{
	const double *inputRef = &m_wallModel->m_states[WALL_MODEL::WallModel::STATE_TEMPERATURE][0];
	std::vector<double>::iterator layerTempIt = m_layerTemperatures.begin();

	// loop over active layers and their temperatures
	for (std::set<unsigned int>::const_iterator
		it = m_layerTemperatures.indexKeys().begin();
		it != m_layerTemperatures.indexKeys().end(); ++it, ++layerTempIt) {

		IBK_ASSERT(*it < m_wallModel->m_materialLayerElementOffset.size() - 1);
		// for each requested layer calculate mean temperature
		double layerTemperature = 0.0;
		unsigned int nElements = m_wallModel->m_materialLayerElementOffset[*it + 1] -
			m_wallModel->m_materialLayerElementOffset[*it];
		IBK_ASSERT(nElements != nullptr);

		// calculate average temperature
		for (unsigned int index = m_wallModel->m_materialLayerElementOffset[*it];
			index < m_wallModel->m_materialLayerElementOffset[*it + 1];
			++index) {

			const double temperature = *(inputRef + index);
			layerTemperature += temperature /
				double(nElements);
		}
		// copy temperature value
		*layerTempIt = layerTemperature;
	}

	return 0;
}


void ConstructionStatesModel::stateDependencies(std::vector< std::pair<const double *, const double *> > &resultInputValueReferences) const
{
	// clear pattern
	if(!resultInputValueReferences.empty() )
		resultInputValueReferences.clear();

	// register dependencies for wall temperatures (not explicitely listed but available
	// as result value reference)
	const double *inputRef = inputValueRefs()[InputRef_InternalEnergyDensity];
	const double *resultRef = &m_wallModel->m_states[WALL_MODEL::WallModel::STATE_TEMPERATURE][0];
	// create pattern for wall temperatures
	for (unsigned int i = 0; i < m_wallModel->m_nElements; ++i,
		++inputRef, ++resultRef)
	{
		resultInputValueReferences.push_back(
			std::make_pair(resultRef, inputRef));
	}

	// mean layer temperatures
	inputRef = &m_wallModel->m_states[WALL_MODEL::WallModel::STATE_TEMPERATURE][0];
	std::vector<double>::const_iterator layerTempIt = m_layerTemperatures.begin();

	// loop over all layers and their temperatures
	for (std::set<unsigned int>::const_iterator
		it = m_layerTemperatures.indexKeys().begin();
		it != m_layerTemperatures.indexKeys().end(); ++it, ++layerTempIt) {

		IBK_ASSERT(*it < m_wallModel->m_materialLayerElementOffset.size() - 1);
		// register all tempertaure values of current layer
		for (unsigned int index = m_wallModel->m_materialLayerElementOffset[*it];
			index < m_wallModel->m_materialLayerElementOffset[*it + 1];
			++index) {

			resultInputValueReferences.push_back(
				std::make_pair(&(*layerTempIt), inputRef + index) );
		}
	}

	// connect first and second value of y...
	inputRef = inputValueRefs()[InputRef_InternalEnergyDensity];
	IBK_ASSERT(inputRef != nullptr);
	IBK_ASSERT(inputRef + 1 != nullptr);
	// ... to the surface temperature at location A
	resultRef = &m_wallModel->m_surfaceTemperatureA;
	IBK_ASSERT(resultRef != nullptr);
	// store the pair of adresses of input value and result
	// inside pattern list
	resultInputValueReferences.push_back(
		std::make_pair(resultRef, inputRef) );
	resultInputValueReferences.push_back(
		std::make_pair(resultRef, inputRef + 1));

	unsigned int n = m_wallModel->m_n;
	// connect last values of y...
	inputRef = inputValueRefs()[InputRef_InternalEnergyDensity] + (int)(n - 2);
	IBK_ASSERT(inputRef != nullptr);
	IBK_ASSERT(inputRef + 1 != nullptr);
	// ...to the surface temperature at location B
	resultRef = &m_wallModel->m_surfaceTemperatureB;
	IBK_ASSERT(resultRef != nullptr);
	// store the pair of adresses of result value and input
	// value inside pattern list
	resultInputValueReferences.push_back(
		std::make_pair(resultRef, inputRef) );
	resultInputValueReferences.push_back(
		std::make_pair(resultRef, inputRef + 1) );
}

const std::vector<size_t>	&ConstructionStatesModel::materialLayerElementOffset() const {
	if (m_wallModel == nullptr)
		return m_dummyVector;
	return m_wallModel->m_materialLayerElementOffset;
}

} // namespace NANDRAD_MODEL

