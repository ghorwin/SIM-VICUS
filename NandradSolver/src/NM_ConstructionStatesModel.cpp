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

	// we initialize only those variables needed for state calculation:
	// - grid and grid-element to layer association
	// - initial conditions/states

	// *** grid generation

	generateGrid();


}

#if 0
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

#endif

void ConstructionStatesModel::generateGrid() {
	//Number of material layers
	size_t nLayers = construction.m_materialLayers.size();
	if (nLayers == 0) {
		throw IBK::Exception(IBK::FormatString("Missing material layers."),
			FUNC_ID);
	}

	// Resize layer offsets
	m_materialLayerElementOffset.resize(nLayers,0);

	// for either cases (density == 0 and all others):
	// - discretize all material layers into several elements
	// vector containing all element widths
	std::vector<double> dx_vec;
	// vector containing all element midpoints
	std::vector<double> x_vec;
	// layer width
	double dLayer;
	// current total material width
	double x = 0;

	if (elements != nullptr) {
		// populate x_vec and dx_vec but merge in the middle

		const std::vector<size_t> lOffsets = *layerOffsets;

		// transfer left-most layer
		m_materialLayerElementOffset[0] = 0;
		for (int i=0; i<lOffsets[1]; ++i) {
			x_vec.push_back((*elements)[i].x );
			dx_vec.push_back((*elements)[i].dx);
		}
		if (nLayers > 1) {
			m_materialLayerElementOffset[1] = lOffsets[1];

			// now process all middle layers
			for (int i=1; i<lOffsets.size()-2; ++i) {
				unsigned int startElementIdx = lOffsets[i];
				unsigned int endElementIdx = lOffsets[i+1];

				double xLayerLeftCoord = (*elements)[startElementIdx].x - 0.5*(*elements)[startElementIdx].dx;
				double xLayerRightCoord = (*elements)[endElementIdx].x - 0.5*(*elements)[endElementIdx].dx;
				// first sum up entire layer width
				double dxMergedLayer = xLayerRightCoord - xLayerLeftCoord;
				x_vec.push_back( xLayerLeftCoord + 0.5*dxMergedLayer);
				dx_vec.push_back( dxMergedLayer);
				m_materialLayerElementOffset[i+1] = m_materialLayerElementOffset[i]+1;
			}

			// process last layer
			for (int i=lOffsets[lOffsets.size()-2]; i<elements->size(); ++i) {
				x_vec.push_back((*elements)[i].x );
				dx_vec.push_back((*elements)[i].dx);
			}
			m_materialLayerElementOffset.push_back(x_vec.size());
		}

	}

	// case: density is set to 0
	// internal layers are used as elements directly
	// boundary layers are split into 2 elements
	else if (discOptions.m_density == 0.0) {
		// loop over all material layers
		for (unsigned int i=0; i<nLayers; ++i) {
			// update element offset
			m_materialLayerElementOffset[i] = dx_vec.size();
			// material layer width
			dLayer = construction.m_materialLayers[i].m_width.value;
			// error empty or negative layer width
			if (dLayer <= 0) {
				throw IBK::Exception(IBK::FormatString("Invalid width %1m of material layer #%2!")
					.arg(dLayer).arg(i),
					FUNC_ID);
			}
			// boundary material layers are split into 2 equal sizes elements
			if (i == 0 || i == nLayers - 1) {
				dx_vec.push_back(dLayer/2);
				dx_vec.push_back(dLayer/2);
				x_vec.push_back(x + dLayer/4);
				x_vec.push_back(x + dLayer*3.0/4);
			}
			else {
				// internal material layers are used as elements directly
				dx_vec.push_back(dLayer);
				x_vec.push_back(x + dLayer/2);
			}
			// update current material width
			x += dLayer;
		}
	}

	// Case: m_density == 1, equidistant discretization
	// element width is set with m_minDx
	else {
		if (discOptions.m_density == 1.0) {
			// loop over all material layers
			for (unsigned int i=0; i<nLayers; ++i) {
				// update element offset
				m_materialLayerElementOffset[i] = dx_vec.size();
				// material layer width
				dLayer = construction.m_materialLayers[i].m_width.value;
				// error empty or negative layer width
				if (dLayer <= 0) {
					throw IBK::Exception(IBK::FormatString("Invalid width %1m of material layer #%2!")
						.arg(dLayer).arg(i),
						FUNC_ID);
				}
				// calculate number of discretized elements for current material layer
				double n_x = dLayer / discOptions.m_minDx;
				// compute the size of the last element
				double x_last = dLayer - (n_x - 1) * discOptions.m_minDx;
				double dx_new;
				// if the last element gets very small, neglect this element
				if (x_last < discOptions.m_minDx/2){
					n_x -= 1;
				}
				// final element width
				dx_new = dLayer / n_x;
				// save elements in dx_vec and x_vec, and update current material width
				for (unsigned int e=0; e<n_x; ++e){
					dx_vec.push_back(dx_new);
					x_vec.push_back(x + dx_new/2);
					x += dx_new;
				}

			}
		}
		// Case: m_density != 0 and != 1
		else {
			if (discOptions.m_density < 1) {
				throw IBK::Exception( IBK::FormatString("Grid density must be == 0 or >= 1, density was %1.").arg(discOptions.m_density), FUNC_ID);
			}
			if (discOptions.m_minDx <= 1e-10) {
				throw IBK::Exception( IBK::FormatString("Minimum element width is too small, element width was %1.").arg(discOptions.m_minDx), FUNC_ID);
			}
			Mesh grid(Mesh::TanHDouble, discOptions.m_density); // double-sided grid

			// determine required min dx by enforcing at least 3 elements per layer
			double rmin_dx = 1;
			for (unsigned int i=0; i<nLayers; ++i) {
				rmin_dx = std::min(rmin_dx, construction.m_materialLayers[i].m_width.value/3.0); // three elements per layer
			}
			// TODO : adjust min_dx or skip layer if very thin

			std::vector<double> xElem;
			std::vector<double> dxElem;
			// loop over all layers
			for (unsigned int i=0; i<nLayers; ++i) {
				// store offset of element number for the current layer
				// m_materialLayerElementOffset[i] = xElem.size();
				m_materialLayerElementOffset[i] = dx_vec.size();

				unsigned int n = 2;	// start with 2 elements per layer
				dLayer = construction.m_materialLayers[i].m_width.value;
				// error empty or negative layer width
				if (dLayer <= 0) {
					throw IBK::Exception(IBK::FormatString("Invalid width %1m of material layer #%2!")
						.arg(dLayer).arg(i),
						FUNC_ID);
				}
				// repeatedly refine grid for this layer until our minimum element width at the boundary is no longer exceeded
				do {
					++n;
					grid.generate(static_cast<int>(n), x, x + dLayer, dxElem, xElem);
					if (dxElem[0] <= 1.1*discOptions.m_minDx) break;
				} while (n < discOptions.m_nMaxElements); // do not go beyond maximum element count
				if (n >= discOptions.m_nMaxElements) {
					throw IBK::Exception( IBK::FormatString("Maximum number of element per layer is reached in material layer #%1 (d=%2m).").arg(i+1).arg(dLayer), FUNC_ID);
				}
				// insert into into global discretization vector
				x_vec.insert(x_vec.end(), xElem.begin(), xElem.end() );
				dx_vec.insert(dx_vec.end(), dxElem.begin(), dxElem.end() );
				x += dLayer;
			}
		}
	}

	// total number of discretized elements
	m_nElements = dx_vec.size();
	// total number of unkowns
	m_n = m_nElements*m_nBalanceEquations;
	// total construction width
	m_constructionWidth = x;

	m_materialLayerElementOffset.push_back(m_nElements);

	// compute weight factors for coefficient averaging
	for (unsigned int i=0; i<m_nElements; ++i) {
		double wL, wR;
		// left weight factors
		if (i == 0)
			// left boundary element
			wL = 1;
		else
			// internal elements
			wL = dx_vec[i]/(dx_vec[i-1] + dx_vec[i]);
		// right weight factors
		if (i == m_nElements-1)
			// right boundary element
			wR = 1;
		else
			// internal elements
			wR = dx_vec[i]/(dx_vec[i] + dx_vec[i+1]);
		// add to element vector
		m_elements.push_back(Element(i, x_vec[i], dx_vec[i], wL, wR, NULL));
	}

}

} // namespace NANDRAD_MODEL

