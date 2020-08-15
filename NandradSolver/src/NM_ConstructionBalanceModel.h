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


#ifndef ConstructionSolverModelH
#define ConstructionSolverModelH


namespace NANDRAD {
	class ModelInputReference;
	class ConstructionInstance;
	class OutputGrid;
	class ParametrizationDefaults;
	class SimulationParameter;
}

#include <MM_Material.h>
#include <DELPHIN_ConstructionType.h>

#include <WM_WallModel.h>
#include <WM_OutputHandler.h>

#include "NM_DefaultModel.h"
#include "NM_DefaultStateDependency.h"
#include "NM_AbstractTimeDependency.h"


namespace NANDRAD_MODEL {

class ConstructionBoundaryCondition;

/*!	\brief Declaration for class ConstructionSolverModel
	\author Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	This model encapsulates the whole computation functionality for a
	1D construction solver. It inherits from the WallModel and
	extends it towards a state and time dependent model object that
	can be integrated into the framework.

	When setTime() is called, only the time point in the wall solver module
	is updated.

	When setY() is called, the solver simply updates all states in the
	wall, that means all temperatures and also the surface temperatures.

	When update() is called, the model retrieves all heat fluxes computed
	by boundary condition models (ConstructionOutsideBCModel etc.) for both
	sides. Then it updates the m_fluxes vector and updates all divergences (m_ydot vector).
	Additionally active components may be defined for each wall layer using sources.
	For this purpose the wall layers may be adressed by calculation models from the
	explicit model section, and the current model translates the corresponding heating fluxes
	into heat sources. Use the following ImplicitModelFeedback tag inside the model block
	of the project file	to adress an active wall layer:

	<Model>
	...
		<ImplicitModelFeedback>
			<!-- Name of the quantity inside the construction solver model
				The index labels the index of the active material layer. Count
				from left to right side starting with index 0. -->
			<TargetName>FieldFlux[1]</TargetName>
			<!-- Name of the object list containing the requested wall. Always
				use the reference type 'ConstructionInstance'!!!-->
			<ObjectListName>...</ObjectListName>
			<!-- Name of the result quantity inside the explicit model-->
			<Quantity>...</Quantity>
			<!-- We add a new quantity to the construction solver model-->
			<Operation>Add</Operation>
		</ImplicitModelFeedback>
	...
	</Model>

	Since the framework updates this module with setY(), then the states
	can be accessed within the update procedure.

	When ydot() is called, it retrieves the updated state from the module wall solver
	instance and copies the data into the ydot vector.
*/


//#define CalculateVaporPressureAtFineGrid
//#define FineBoundaryLayerForMixedGrid

class ConstructionSolverModel : public DefaultModel, public DefaultStateDependency, public AbstractTimeDependency,
	private WALL_MODEL::WallModel, public WALL_MODEL::OutputHandler
{
public:

	// ***KEYWORDLIST-START***
	enum WallMoistureBalanceCalculationMode {
		CM_Average,							// Keyword: Average								'Vvapor transport calculation through average wall layers including boundaries.'
		CM_Detailed,						// Keyword: Detailed							'Detailed vapor storage and transport calculation including spacial discretization.'
		CM_None								// Keyword: None								'No wall moisture calculation.'
	};
	enum VectorValuedResults {
		VVR_ThermalLoad,					// Keyword: ThermalLoad				[W]			'Optional field fluxes for all material layers with given layer index.'
		NUM_VVR
	};
	enum InputReferences {
		InputRef_FieldFlux,					// Keyword: FieldFlux				[W]			'Optional field flux for a given material layer.'
		NUM_InputRef
	};
	// ***KEYWORDLIST-END***

	/*! Constructor. */
	ConstructionSolverModel(unsigned int constructionSolverId, const std::string &displayName);

	/*! Destructor, free allocated memory for ModuleWallSolver instances. */
	virtual ~ConstructionSolverModel();

	/*! Construction solve model can be referenced via ConstructionInstance type and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const {
		return NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const { return "ConstructionSolverModel"; }

	/*! Returns a priority number for the ordering in model evaluation.*/
	virtual int priorityOfModelEvaluation() const;

	/*! Returns number of primary state results (number of unknows
		per element times number of elements).
	*/
	unsigned int nPrimaryStateResults() const;

	/*! Sets initial states in y vector.
		This function is called after setup(), so that parameters needed for
		computing the initial condition are already present.
	*/
	void yInitial(double * y);

	/*! Sets new states by passing a linear memory array with the construction solve element states.
		This function is called from the main model interface setY() function.
		A call to this function triggers the re-calculation of the states in the construction.
		The fluxes and divergences are calculated when the function ydot() is called.
	*/
	void setY(const double * y);

	/*! Stores the previously computed divergences in all elements in vector ydot.
		\return Returns 0 when calculation was successful, 1 when a recoverable error has been detected, 2 when something is badly wrong
	*/
	int ydot(double* ydot);


	/*! Adds a new output quantity string to the model. */
	bool addOutputDefinition(const WALL_MODEL::OutputHandler::OutputDefinition &outputDef);

	/*! Initializes wall model output.
		\param outputDir Construction instance specific output directory
		\param outputGrid (optional) Pointer to output grid.
		\param startYear Start year to write in DataIO files
	*/
	void initOutputs(const std::string & outputDir, const NANDRAD::OutputGrid *outputGrid, int startYear);

	/*! Writes wall model outputs for time active grid point.
		\param t Output time point as offset to Midnight January 1st of the start year (not simulation time!).
	*/
	void writeOutputs(const double t);

	/*! Returns construction instance (data member) used to create this wall solver. */
	const NANDRAD::ConstructionInstance *constructionInstance() const { return m_constructionInstance; }

	/*! Returns construction type of current wall. */
	const DELPHIN_LIGHT::ConstructionType *constructionType() const { return &m_construction; }

	/*! Returns moisture calculation mode. */
	WallMoistureBalanceCalculationMode moistureCalculationMode() const { return m_moistureCalculationMode; }

	/*! Returns number of moisture states. */
	unsigned int nMoistureStates() const {
		if (m_moistureCalculationMode == CM_Detailed)
			return m_nElements;
		else if (m_moistureCalculationMode == CM_Average) {
			IBK_ASSERT(m_coarseGridWallModel != NULL);
			return (unsigned int)m_coarseGridWallModel->m_nElements;
		}
		return 0;
	}

	/*! Initializes model by providing construction instance parameters.
		The relevant parameters are cached in the ConstructionSolverModel results
		for access by other models.
		This function also checks for require parameters and tests for validity of values.
	*/
	void setup(const NANDRAD::ConstructionInstance & parameters,
		const std::map<std::string, MM::Material> & materials,
		const std::map<unsigned int, DELPHIN_LIGHT::ConstructionType> &constructionTypes,
		const NANDRAD::ParametrizationDefaults &defaultPara,
		const NANDRAD::SimulationParameter &simPara,
		const WALL_MODEL::DiscretizationOptions & discOptions);

	/*! Sets boundary condtion module at location A.
	*/
	void setBoundaryConditionA(ConstructionBoundaryCondition & boundaryCond);
	/*! Sets boundary condtion module at location B.
	*/
	void setBoundaryConditionB(ConstructionBoundaryCondition & boundaryCond);

	/*! Resizes m_results vector.*/
	virtual void initResults(const std::vector<AbstractModel*> & models);

	/*! Populates the vector resDesc with descriptions of all results provided by this model.
	*/
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const;


	/*! Populates the vector refDesc with descriptions of all input references
	requested by this model.
	*/
	virtual void inputReferenceDescriptions(std::vector<QuantityDescription> & refDesc) const;

	/*! Returns vector of all scalar and vector valued results pointer.
	*/
	virtual void resultValueRefs(std::vector<const double *> &res) const;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\return Returns pointer to memory location with this quantity, otherwise NULL if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const;

	/*! No input references are initilaized per default. */
	virtual void initInputReferences(const std::vector<AbstractModel*> & /*models*/) { }

	/*! Creates a model input reference from an implicit model feedback.
		We allow field conditions for single constructions and single material layers.
		The reference type is MRT_CONSTRUCTIONINSTANCE. The material layer is part of the target name.
		The requested target name is 'FieldFlux[index = ...]'
		with 'index=..' marking the material layer index.
		Unit [W].
		Always use operation 'Add'.
	*/
	virtual bool registerInputReference(unsigned int sourceID,
		NANDRAD::ModelInputReference::referenceType_t referenceType,
		const QuantityName &quantity,
		const QuantityName &targetName,
		NANDRAD::ImplicitModelFeedback::operation_t operation);

	/*! Adds dependencies between ydot and y to default pattern.
	*/
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > &resultInputValueReferences) const;

	/*! Updates time in wall solver. */
	virtual int setTime(double t);

	/*! Updates the fluxes and computes the divergences in all elements. */
	virtual int update();

	/*! Calculates a vector index from key string for all field conditions. Because we use index notation for
		specifying material layers the implementation differs from the default. In detail the default index
		notation expects id ordering instead of direct index access.
	*/
	virtual unsigned int decodeVectorValuedQuantityIndex(const std::string & vectorQuantity, const std::string & keyString) const;
	virtual const std::string encodeVectorValuedQuantityIndex(const std::string & vectorQuantity, unsigned int keyValue) const;

private:
	/*! Setup function for averaging from detailed grid to a corse grid reperesented by elements vector. */
	void setupGridAveraging(std::vector<WALL_MODEL::Element> &elems);
	/*! Translator flux from detailed to average quantity. */
	void averageFluxFromDetailedFlux(double *fluxAverage, const double *fluxDetailed) const;
	/*! Translator function from detailed to average quantity. */
	void averageFieldFromDetailedField(double *vAverage, const double *vDetailed) const;
	/*! Translator function from average quantity at a detailed grid. */
	void detailedFieldFromAverageField(double *vDetailed, const double *vAverage) const;

	/*! Moisture calculation mode. */
	WallMoistureBalanceCalculationMode			m_moistureCalculationMode;
	/*! Boundary condition calculation modules. */
	ConstructionBoundaryCondition				*m_boundaryConditionA;
	ConstructionBoundaryCondition				*m_boundaryConditionB;
	/*! Storage of solution quantities: wall temperature on left side of construction. */
	double										m_surfaceTemperatureA;
	/*! Storage of solution quantities: wall temperature on right side of construction. */
	double										m_surfaceTemperatureB;
	/*! Storage of solution quantities: vapor pressure on left side of construction. */
	double										m_surfaceVaporPressureA;
	/*! Storage of solution quantities: vapor pressure on right side of construction. */
	double										m_surfaceVaporPressureB;
	/*! Storage of solution quantities: relative humidity on left side of construction. */
	double										m_surfaceRelativeHumidityA;
	/*! Storage of solution quantities: relative humidity on right side of construction. */
	double										m_surfaceRelativeHumidityB;
	/*! Optional wall solver module witzh coarse discretization. NULL if detailed mode ore no moisture calculation
		are chosen.*/
	WALL_MODEL::WallModel						*m_coarseGridWallModel;
	/*! Output handler for coarse grid.*/
	WALL_MODEL::OutputHandler					*m_coarseGridOutputHandler;
	/*! Vector with cached states, updated at last call to setY(). */
	std::vector<double>							m_y;
	/*! Temperary vector for internal calculations: size of y in orginal grid. */
	std::vector<double>							m_tempY;
	/*! Temperary vector for internal calculations: size of y in coarse grid. */
	std::vector<double>							m_tempYCoarse;
	/*! Temperary vector for average quantity. */
	std::vector<double>							m_tempAv;
	/*! Temperary vector for detailed quantity. */
	std::vector<double>							m_tempDet;
	/*! Vector with cached divergences for moisture average mode.*/
	std::vector<double>							m_ydotAverage;
	/*! Position of energy balance equation in ydot and y vector for each discretization element.*/
	std::vector<unsigned int>					m_energyBalanceEquation;
	/*! Position of moisture balance equation in ydot and y vector for each discretization element.*/
	std::vector<unsigned int>					m_moistureBalanceEquation;
	/*! Vector with element offsets thnat correspond to a moisture balance element,
		only active in average mode.*/
	std::vector<unsigned int>					m_averageElementOffset;

	/*! Solver Time. */
	double										m_t;
	/*! Constant reference to zone parameter block. */
	const NANDRAD::ConstructionInstance			*m_constructionInstance;
	/*! Indices of material layers that are occupied by a field condition.
		We start counting the material layers at the left side of the construction
		(location A) with index 0.*/
	std::set<unsigned int>						m_activeLayerIndices;

	/*! Initial temperature. */
	double										m_TInitial;

	/*! Initial relative humidity. */
	double										m_rHInitial;

	/*! Wall area. */
	double										m_wallArea;

	/*! Output time interval. */
	const NANDRAD::OutputGrid					*m_outputGrid;

	/*! Set with output quantities requested from project file. */
	std::set<WALL_MODEL::OutputHandler::OutputDefinition>
												m_outputDefs;

	/*! Exclusively allow the main model class to access private attributes (i.e. the conversion to the
		private class WallModel). */
	friend class NandradModelImpl;
	friend class ConstructionSolverMoistureModel;
	friend class ConstructionStatesModel;
	friend class ConstructionStatesMoistureModel;
};

} // namespace NANDRAD_MODEL

#endif // ConstructionSolverModelH
