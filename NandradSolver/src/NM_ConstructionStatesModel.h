/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#ifndef NM_ConstructionStatesModelH
#define NM_ConstructionStatesModelH


#include "NM_AbstractModel.h"
#include "NM_VectorValuedQuantity.h"

namespace NANDRAD {
	class ConstructionInstance;
	class SimulationParameter;
	class SolverParameter;
	class Material;
}

namespace NANDRAD_MODEL {

class Material;
class Loads;

/*!	This model computes temperatures (and if moisture balance is enabled in constructions, also
	intensive moisture variables like relative humidity/vapor pressure) from the conserved quantities.

	It is updated first during the model update (a "head" model).
*/
class ConstructionStatesModel : public AbstractModel {
public:

	enum VectorValuedResults {
		VVR_ElementTemperature,			// Keyword: ElementTemperature			[C]		'Finite-volume mean element temperature'
		/*! The index key is the ID of the interconnected construction instance. */
		VVR_EmittedLongWaveRadiationA,	// Keyword: EmittedLongWaveRadiationA	[W]		'Emitted internal long-wave radiation from side A.'
		/*! The index key is the ID of the interconnected construction instance. */
		VVR_EmittedLongWaveRadiationB,	// Keyword: EmittedLongWaveRadiationB	[W]		'Emitted internal long-wave radiation from side B.'
		NUM_VVR
	};

	/// \todo maybe add integral quantities like total moisture mass stored in construction or total energy stored in construction
	enum Results {
		R_SurfaceTemperatureA,			// Keyword: SurfaceTemperatureA		[C]		'Surface temperature at interface A'
		R_SurfaceTemperatureB,			// Keyword: SurfaceTemperatureB		[C]		'Surface temperature at interface B'
		R_SolarRadiationFluxA,			// Keyword: SolarRadiationFluxA		[W/m2]	'Solar radiation flux density into surface A'
		R_SolarRadiationFluxB,			// Keyword: SolarRadiationFluxB		[W/m2]	'Solar radiation flux density into surface B'
		R_LongWaveRadiationFluxA,		// Keyword: LongWaveRadiationFluxA	[W/m2]	'Absorbed minus emitted ambient long wave radiation flux density for surface A'
		R_LongWaveRadiationFluxB,		// Keyword: LongWaveRadiationFluxB	[W/m2]	'Absorbed minus emitted ambient long wave radiation flux density for surface B'
		R_EmittedLongWaveRadiationFluxA,	// Keyword: EmittedLongWaveRadiationFluxA	[W/m2]	'Emitted long wave radiation flux density for surface A (sum of all emissions to all other inside surface)'
		R_EmittedLongWaveRadiationFluxB,	// Keyword: EmittedLongWaveRadiationFluxB	[W/m2]	'Emitted long wave radiation flux density for surface B (sum of all emissions to all other inside surface)'
		NUM_R
	};

	/*! Constructor */
	ConstructionStatesModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes model. */
	void setup(const NANDRAD::ConstructionInstance & con,
			   const NANDRAD::SimulationParameter & simPara,
			   const NANDRAD::SolverParameter & solverPara,
			   Loads & loads);


	// *** Re-implemented from AbstractModel

	/*! Room balance model can be referenced via Zone and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "ConstructionStatesModel";}

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Returns display name of this model instance. */
	virtual const char * displayName() const override { return m_displayName.c_str(); }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\note Quantity "y" gives address of cached input states.
		\return Returns pointer to memory location with this quantity, otherwise nullptr if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const InputReference & quantity) const override;

	// *** Other public member functions

	/*! Returns number of conserved variables (i.e. length of y vector passed to yInitial() and update() ). */
	unsigned int nPrimaryStateResults() const;

	/*! Returns a vector of dependencies of all result quantities (including ydots) from input variables). */
	void stateDependencies(std::vector< std::pair<const double *, const double *> > & resultInputValueReferences) const;


	/*! Returns ID of associated zone at interface A (> 0 if a room zone is connected). */
	unsigned int interfaceAZoneID() const;
	/*! Returns ID of associated zone at interface B (> 0 if a room zone is connected). */
	unsigned int interfaceBZoneID() const;

	/*! Sets initial states in y vector.
		This function is called after setup(), so that parameters needed for
		computing the initial condition are already present.

		\param y Pointer to the memory array holding all initial states for this model (to be written into).
	*/
	void yInitial(double * y) const;

	/*! Computes intensive variables in Finite Volumes of construction.
		This function is called directly from NandradModel as first step in the model evaluation.

		\param y Pointer to the memory array holding all states for this room model.

		This function also evaluates the outside boundary conditions.
	*/
	int update(const double * y);

	/*! Access to underlaying data model - provided for convenience. */
	const NANDRAD::ConstructionInstance * construction() const { return m_con; }

private:
	/*! Generates computational grid and all associated data structures. */
	void generateGrid();

	/*! Construction instance ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! True if moisture balance is enabled. */
	bool											m_moistureBalanceEnabled;

	/*! Results, computed/updated during the calculation. */
	std::vector<double>								m_results;

	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>				m_vectorValuedResults;

	/*! Cached pointer to source construction instance data structure. */
	const NANDRAD::ConstructionInstance *			m_con = nullptr;
	/*! Cached pointer to source simulation parameter data structure. */
	const NANDRAD::SimulationParameter *			m_simPara = nullptr;
	/*! Cached pointer to source solver parameter data structure. */
	const NANDRAD::SolverParameter *				m_solverPara = nullptr;
	/*! Cached pointer to climate loads model, to retrieve climatic loads. */
	const Loads *									m_loads = nullptr;

	// the variables below are accessed/used also by the ConstructionBalanceModel

	/*! Element structure, containing data FV-data for a single element (finite-volume). */
	struct Element {
		/*! Default constructor. */
		Element() : i(0), mat(nullptr)
		{
		}

		/*! Initializing constructor. */
		Element(unsigned int i_, double x_, double dx_, double wL_, double wR_, const NANDRAD::Material * mat_) :
			i(i_),
			x(x_),
			dx(dx_),
			wL(wL_),
			wR(wR_),
			mat(mat_)
		{
		}

		unsigned int					i;		///< Element index
		double							x;		///< Element center points x-coordinate [m]
		double							dx;		///< Element width [m]
		double							wL;		///< Weight factor left
		double							wR;		///< Weight factor right
		const NANDRAD::Material			*mat;	///< Pointer to material data.
	};

	/*! Vector containing all discretized elements, size: m_nElements. */
	std::vector<Element>			m_elements;
	/*! Cached total number of discretized elements. */
	size_t							m_nElements;
	/*! Cached total number of unknowns, size: m_nElements*m_nBalanceEquations. */
	size_t							m_n;
	/*! Vector containing offset of the element number for each material layer,
		last element contains m_nElements
		size: m_construction.m_materialLayers.size() + 1. */
	std::vector<size_t>				m_materialLayerElementOffset;
	/*! Total construction width [m]. */
	double							m_constructionWidth;
	/*! Storage of active layer index.*/
	unsigned int					m_activeLayerIndex = 999;
	/*! Active layer volume [m3]. */
	double							m_activeLayerVolume = 6666;
	/*! Stores the mean temperature of the active layer (area-weighted) in [K]. */
	double							m_activeLayerMeanTemperature = 999;

	/*! Cached conserved quantities (size nPrimaryStateResults() ) */
	std::vector<double>				m_y;

	/*! Vector with heat conduction fluxes [W/m2] across elements.
		Note: vector has size m_elements+1 and does only contain heat conduction fluxes
			between elements. m_fluxes_q[0] and m_fluxes_q[m_nElements] are unused.
	*/
	std::vector<double>				m_fluxes_q;

	/*! Pre-calculated energy storage capacity rho*ce in each element [J/K]. */
	std::vector<double>				m_rhoce;
	/*! Inverse thermal resistance between element centers (size m_nElements + 1) [W/m2K]. */
	std::vector<double>				m_rTInv;

	/*! Surface temperature at left side (side A) [K]. */
	double							m_TsA;
	/*! Surface temperature at right side (side B) [K]. */
	double							m_TsB;

//	/*! Emitted long wave radiation to all construction instances visible from side A. The values are stored in a map
//		with the key being the id of the targeted (radiation receiving) construction instance.
//		The value of the map is the heat flux in [W].
//	*/
//	std::map<unsigned int, double>	m_emittedLongWaveRadiationA;
//	/*! Emitted long wave radiation to all construction instances visible from side B. The values are stored in a map
//		with the key being the id of the targeted (radiation receiving) construction instance.
//		The value of the map is the heat flux in [W].
//	*/
//	std::map<unsigned int, double>	m_emittedLongWaveRadiationB;


	friend class ConstructionBalanceModel; // Our balance model can directly take the data from us
};

} // namespace NANDRAD_MODEL

#endif // NM_ConstructionStatesModelH
