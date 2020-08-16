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

#ifndef ConstructionStatesModelH
#define ConstructionStatesModelH


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

/*!	This model computes temperatures (and if moisture balance is enabled in constructions, also
	intensive moisture variables like relative humidity/vapor pressure) from the conserved quantities.

	It is updated first during the model update (a "head" model).
*/
class ConstructionStatesModel : public AbstractModel {
public:

	enum VectorValuedResults {
		VVR_LayerTemperature,			// Keyword: LayerTemperature		[C]		'Mean layer temperature for requested quanties.'
		NUM_VVR
	};

	/// \todo maybe add integral quantities like total moisture mass stored in construction or total energy stored in construction
	enum Results {
		R_SurfaceTemperatureA,			// Keyword: SurfaceTemperatureA		[C]		'Surface temperature at interface A.'
		R_SurfaceTemperatureB,			// Keyword: SurfaceTemperatureB		[C]		'Surface temperature at interface B.'
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
			   const NANDRAD::SolverParameter & solverPara);


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

	/*! Returns vector of all scalar and vector valued results pointer. */
	virtual void resultValueRefs(std::vector<const double *> &res) const override;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\return Returns pointer to memory location with this quantity, otherwise nullptr if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;


	// *** Other public member functions

	/*! Sets initial states in y vector.
		This function is called after setup(), so that parameters needed for
		computing the initial condition are already present.

		\param Pointer to the memory array holding all initial states for this model (to be written into).
	*/
	void yInitial(double * y) const;

	/*! Computes intensive variables in Finite Volumes of construction.
		This function is called directly from NandradModel as first step in the model evaluation.

		\param Pointer to the memory array holding all states for this room model.
	*/
	int update(const double * y);

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

	/*! Energy densities in elements [J/m3] */
	std::vector<double>				m_statesU;
	/*! Temperatures in elements [K] */
	std::vector<double>				m_statesT;

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

};

} // namespace NANDRAD_MODEL

#endif // ConstructionStatesModelH
