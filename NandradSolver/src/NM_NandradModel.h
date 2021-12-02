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

#ifndef NM_NandradModelH
#define NM_NandradModelH

#include <string>
#include <vector>

#include <SOLFRA_ModelInterface.h>
#include <SOLFRA_OutputScheduler.h>
#include <SOLFRA_SolverFeedback.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <NANDRAD_ArgsParser.h>

#include <ZEPPELIN_DependencyGraph.h>

#include "NM_Directories.h"

namespace NANDRAD {
	class ArgsParser;
	class Project;
}

/*! The namespace NANDRAD_MODEL contains all classes/functions of the
	NANDRAD command line solver. The central class is NandradModel, which implements
	the integrator interface used by the SOLFRA integrator classes.
*/
namespace NANDRAD_MODEL {

class AbstractModel;
class AbstractODEStatesModel;
class AbstractODEBalanceModel;
class AbstractTimeDependency;
class AbstractStateDependency;
class SteadyStateSolver;
class StateModelGroup;
class ParallelStateObjects;

class Loads;
class Schedules;
class FMIInputOutput;
class RoomBalanceModel;
class RoomStatesModel;
class OutputHandler;

class ConstructionStatesModel;
class ConstructionBalanceModel;

class ThermalNetworkStatesModel;
class ThermalNetworkBalanceModel;

/*! Main NANDRAD model implementation class.
	This class implements the interface of SOLFRA::ModelInterface and SOLFRA::OutputScheduler and contains
	the entire logic of the model evaluation.
*/
class NandradModel : public SOLFRA::ModelInterface, public SOLFRA::OutputScheduler {
public:

	// *** typedefs

	/*! Vector holding references/pointers to time-state objects. */
	typedef std::vector<AbstractStateDependency*>			ParallelTimeStateObjects;
	/*! Vector holding references/pointers to state objects. */
	typedef std::vector<AbstractStateDependency*>			ParallelStateObjects;



	/*! Constructor. */
	NandradModel();
	/*! Destructor. */
	~NandradModel() override;

	/*! Initializes model. */
	void init(const NANDRAD::ArgsParser & args);

	/*! Sets up directories so that log files and output files can be written.
		Throws an exception if the project file doesn't exist or the directories
		cannot be created. Also, the command-line option -o is handled here.
		The paths are stored in the m_dirs object.
		\param args Command-line argument parser object.
	*/
	void setupDirectories(const NANDRAD::ArgsParser & args);

	/*! Returns solver/project directories, initialized in init(). */
	const Directories & dirs() const { return m_dirs; }


	// *** MEMBER FUNCTIONS REQUIRED BY SOLVER FRAMEWORK ***

	/*! Number of unknowns/states, size n. */
	virtual unsigned int n() const override { return m_n; }

	/*! Initial condition vector, size n.
		\return Returns a pointer to a linear memory array with the	initial states.
		\warning The lifetime of the pointer may be limited. Specifically,
				 the pointer may be invalidated during initialization.
	*/
	virtual const double * y0() const override { return &m_y0[0];}
	/*! Start time point in [s]. */
	virtual double t0() const override { return m_t0; }
	/*! Initial time step. */
	virtual double dt0() const override;
	/*! End time point in [s]. */
	virtual double tEnd() const override { return m_tEnd; }

	/*! Update state of model to new time point. */
	virtual CalculationResult setTime(double t) override;

	/*! Update state of model to new set of unknowns. */
	virtual CalculationResult setY(const double * y) override;

	/*! Stores the computed derivatives of the solution results in the vector ydot. */
	virtual CalculationResult ydot(double * ydot) override;

	/*! Writes outputs at time t_out using the states y_out.
		This function is called for ODE-type models.
		The pointer y_out has limited lifetime and should only be used in this
		function and never be kept for later calls.
		This function may alter the state of the model implementation object to match
		the output time and solution.
		Default implementation does nothing.
		\param t_out Output time point (in seconds, simulation time).
		\param y_out Pointer to linear memory array of size n() containing the corresponding solution.
		\param ydot_out Pointer to linear memory array of size n() containing the corresponding
			time derivatives (nullptr Pointer for ODE-type models).
	*/
	virtual void writeOutputs(double t_out, const double * y_out) override;

	/*! Called at end of simulation for final output file caching/closing of output files. */
	virtual void writeFinalOutputs() override;

	/*! Function to convert relative simulation time t into a date/time string representation. */
	virtual std::string simTime2DateTimeString(double t) const override;

	/*! Informs the model that a step was successfully completed.
		The time point and value vector passed to the function correspond to
		the current state in the integrator object.
		This function can be used to write restart info.
		Default implementation does nothing.
	*/
	virtual void stepCompleted(double t, const double * y) override;

	/*! Returns linear equation system solver. */
	virtual SOLFRA::LESInterface * lesInterface() override;

	/*! Returns model-specific Jacobian matrix generation method for use with iterative LES solvers.
		Default implementation returns nullptr (no jacobian generator).
	*/
	virtual SOLFRA::JacobianInterface * jacobianInterface() override;

	/*! Returns integrator to be used for this model.
		Default implementation returns nullptr, which results in CVODE integrator for
		ODE type models and IDA for DAE-type models.
	*/
	virtual SOLFRA::IntegratorInterface * integratorInterface() override;

	/*! Returns model-specific pre-conditioner.
		Default implementation returns nullptr (no preconditioner).
	*/
	virtual SOLFRA::PrecondInterface * preconditionerInterface() override;

	/*! Returns output scheduler to be used with the framework. */
	virtual SOLFRA::OutputScheduler * outputScheduler() override { return this; }

	/*! Returns error weights to the integrator if defined. Otherwise nullptr. */
	virtual CalculationResult calculateErrorWeights(const double *y, double *weights) override;

	/*! Informs the integrator whether the model owns an error weighting function. */
	virtual bool hasErrorWeightsFunction() override;

	/*! Computes and returns serialization size, by default returns  returns an invalid value (-1). */
	virtual std::size_t serializationSize() const override;

	/*! Stores content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.*/
	virtual void serialize(void* & dataPtr) const override;

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.*/
	virtual void deserialize(void* & dataPtr) override;

	/*! Writes currently collected solver metrics/statistics to output.
		\param simtime Totel elapsed wall clock time of simulation in [s] (needed for percentage calculation)
		\param metricsFile If not nullptr, computer-readible metrics are written to the file.
	*/
	virtual void writeMetrics(double simtime, std::ostream * metricsFile = nullptr) override;


	// *** MEMBER FUNCTIONS REQUIRED BY SOLFRA::OutputScheduler ***

	/*! Returns next output time point after t, where t is in simulation time of the integrator (always starts with 0). */
	virtual double nextOutputTime(double t) override;


	// *** STATIC FUNCTIONS ***

	/*! Prints version strings of used libraries to the IBK message handler. */
	static void printVersionStrings();

protected:
	// *** Outputs ***

	/*! The central managing object for outputs. */
	OutputHandler											*m_outputHandler = nullptr;


private:

	/*! Initializes/checks solver parameter.
		Override-optiones specified via command line are used to modify the
		m_project->m_solverParameter data structure.
	*/
	void initSolverParameter(const NANDRAD::ArgsParser & args);
	/*! Initializes/checks simulation parameter.*/
	void initSimulationParameter();
	/*! Loads climate data and creates Loads. */
	void initClimateData();
	/*! Loads schedule data and creates Schedules + Schedule parameters. */
	void initSchedules();
	/*! FMI import/export initialization. */
	void initFMI();
	/*! Loads BIM data and creates RoomBalanceModels, Summation and Splitting models. */
	void initZones();
	/*! Loads BIM data and creates ConstructionSolverModel, initialises WallSolver, creates
		ConstructionBCModels (only for active walls and interfaces).*/
	void initWallsAndInterfaces();
	/*! Initializes all models that are parametrized for several objects. */
	void initModels();
	/*! With the knowledge of all available zone, construction, interface etc. models, resolve
		object list ID groups and replace them with definite sets of IDs for the referenced
		model types.
		This modifies object lists in-place, yet does not change the vector. Previously
		obtained pointers to object lists are not invalidated.
		\warning Object list ID groups are updated such, that the m_ids set of IDs will contain afterwards all
			selected IDs of the given type (i.e. range and wildcards resolved) with matching and existing objects,
			and *only* IDs of existing objects. For example, if the object list requests objects with IDs 10,11 and 12
			but there are only objects of the requested type with IDs 1,2 and 4, then the object list's id set
			will be *empty*!!!
	*/
	void initObjectLists();
	/*! Initializes all hydraulic networks. */
	void initNetworks();
	/*! Connects implicit and explicit models according to input references, calls
		modelInputReference() of each model. .*/
	void initModelDependencies();
	/*! Creates the state model dependency graph, performs a sequencial or cyclic
		mdoel grouping and fills m_orderedTimeStateDependentSubModels, m_orderedStateDependentSubModels vectors
		with sorted parallel object groups.*/
	void initModelGraph();
	/*! Creates a list of all available output quantities. Also writes the variable-display name mapping table. */
	void initOutputReferenceList();
	/*! Initialises all outputs.
		\param restart If true, output files are re-opened for writing (and are expected to exist).
	*/
	void initOutputs(bool restart);
	/*! Resizes solution vectors and stores vector offset for wall solver and room solver results.*/
	void initSolverVariables();
	/*! Selects information about the discretisation matrix and provide optimal ordering for band matrices,
		index vector for sparse matrices.*/
	void initSolverMatrix();
	/*! Initializes model-specific statistics output files.
		\param restart If true, the statistics file is opened in append mode.
	*/
	void initStatistics(SOLFRA::ModelInterface * modelInterface, bool restart);
	/*! Depending on model's priorityOfModelEvaluation() (-1 for unordered, or a number for
		head/tail ordering) the model is added to m_orderedStateDependentSubModelsHead or
		m_orderedStateDependentSubModelsTail.
	*/
	void registerStateDependendModel(AbstractStateDependency * stateModel);


	/*! Vectors storing sparse matrix pattern (CSR format): number of nonzero elements: */
	unsigned int nnz() const  { return (unsigned int) m_ja.size(); }

	/*! Updates all time dependent models due to their order in state dependency graph. */
	int updateTimeDependentModels();
	/*! Updates all state dependent models due to their order in state dependency graph. */
	int updateStateDependentModels();


	// *** PRIVATE MEMBER VARIABLES ***

	/*! Cached project file path. */
	IBK::Path												m_projectFilePath;
	/*! Directories related to the project. */
	Directories												m_dirs;

	/*! The actual NANDRAD project data (owned). */
	NANDRAD::Project										*m_project;


	// *** Core Solver Variables ***

	/*! Cached total number of unknowns (number of rooms + all elements in all walls). */
	unsigned int											m_n = 0;
	/*! Cached number of thermal (active) zones. */
	unsigned int											m_nZones = 0;
	/*! Cached number of walls. */
	unsigned int											m_nWalls = 0;
	/*! Cached number of thermal networks. */
	unsigned int											m_nNetworks = 0;
	/*! Starting time point in [s]. */
	double													m_t0 = 0;
	/*! End time point in [s]. */
	double													m_tEnd = 0;
	/*! Vector with initial conditions.
		Unlike m_y and m_ydot the values in yo are stored in solver-y-variable order.
	*/
	std::vector<double>										m_y0;
	/*! Vector with cached solution, updated at last call to setY(). */
	std::vector<double>										m_y;
	/*! Vector with cached derivatives, updated at last call to updateAllDivergences(). */
	std::vector<double>										m_ydot;
	/*! Cached time point in [s], updated at last call to setTime(). */
	double													m_t;
	/*! Factor for the error weights for all zones, 1 per default. */
	double													m_weightsFactorZones = 1.0;
	/*! Flag that indicates the m_y has changed.
		This flag is queried in ydot() to determine if
		a re-calculation is necessary.
		The flag may also be set in stepCompleted() to indicate that some other input
		variable has changed.
	*/
	bool													m_yChanged;
	/*! Flag that indicates that the time point was recently changed.
		This flag is set in setTime() and evaluated in setY() in order
		to determine whether a full variable update is needed or not.
	*/
	bool													m_tChanged;


	// ***  Solver specification. ***

	/*! Linear equation solver. */
	SOLFRA::LESInterface									*m_lesSolver = nullptr;
	/*! Jacobian. */
	SOLFRA::JacobianInterface								*m_jacobian = nullptr;
	/*! Preconditioner. */
	SOLFRA::PrecondInterface								*m_preconditioner = nullptr;
	/*! Integrator. */
	SOLFRA::IntegratorInterface								*m_integrator = nullptr;

	/*! Sparse matrix indices. */
	std::vector<unsigned int>								m_ia;
	std::vector<unsigned int>								m_ja;
	std::vector<unsigned int>								m_iaT;
	std::vector<unsigned int>								m_jaT;


	// *** Parallelization ***

	/*! Number of parallel threads to use.
		Set in initSolverParameter().
	*/
	int														m_numThreads;
	/*! If true (the default for NUM_THREADS=1 or small m_n), the model evaluation uses
		serial code, otherwise parallel loops. This speeds up execution of small model problems.
		Set in initSolverVariables().
	*/
	bool													m_useSerialCode;


	// *** State model containers and organization data structures ***

	/*! This list contains and owns all models (at the end, all models in this container must
		be deleted from the heap).
	*/
	std::vector<AbstractModel*>								m_modelContainer;

	/*! Container for all pairs of ODE states and ODE balance models.
	*/
	std::vector<std::pair<AbstractODEStatesModel*, AbstractODEBalanceModel*> >
															m_ODEStatesAndBalanceModelContainer;

	/*! Vector with offsets for variable ranges of solution vector. */
	std::vector<unsigned int>								m_ODEVariableOffset;

	/*! Container for all time-dependent models created dynamically on the heap (owned).
		The models are owned by the NandradModelImpl class and released within the clear()
		function (and thus destructor).
		The evaluation models is sequential.
	*/
	std::vector<AbstractTimeDependency*>					m_timeModelContainer;

	/*! Container for all time-dependent models that need to complete an integrator step when
		output writing is requested (in order to update referenceable results).
	*/
	std::vector<AbstractTimeDependency*>					m_stepCompletedForOutputWriting;

	/*! Container for all staedy state models that inherit from SOLFRA::ModelInterface.
		The evaluation models is sequential.
	*/
	std::vector<SteadyStateSolver*>							m_steadyStateModelContainer;

	/*! Container for all state-dependent models created dynamically on the heap (owned).
	The models are owned by the NandradModelImpl class and released within the clear()
	function (and thus destructor).
	*/
	std::vector<AbstractStateDependency*>					m_stateModelContainer;

	/*! Graph that stores all elements and functions of the underlying graph*/
	ZEPPELIN::DependencyGraph								m_stateDependencyGraph;

	/*! Clusters from the DependencyGraph sortimg algorithm. Used to generate state model groups*/
	std::list<ZEPPELIN::DependencyGroup>					m_stateDependencyGroups;

	/*! Container for groups of models (that are in cyclic or sequential dependency). Only used
	 * for ownership of generated state model groups.
	*/
	std::set<StateModelGroup*>								m_stateModelGroups;

	/*! Vector containing several vectors with references to state-dependent sub-models
		in the order of evaluation.
		The pointers are only references to sub-models distributed throughout the data
		structure of the model. The objects in these containers are not owned,
		only referenced and must not be deleted.
		The function update() is called in all objects in this container.
		\note This container owns instances of AbstractStateModelGroup, which are
			  deleted in the destructor.
	*/
	std::vector<ParallelStateObjects>						m_orderedStateDependentSubModels;

	/*! Vector of unordered state dependencies.
		This container holds all models with undefined priority (-1). Their evaluation order
		is determined by the dependency pattern.
		The container is populated in registerStateDependendModel().
	*/
	std::vector<AbstractStateDependency*>					m_unorderedStateDependencies;
	/*! Vector of of already sorted state dependencies that are evaluated *before* the
		graph of unordered models (head). The models in this container have a priority != -1 but
		smaller than AbstractStateDependency::priorityOffsetTail.
		The container is populated in registerStateDependendModel().
	*/
	std::vector<ParallelStateObjects>						m_orderedStateDependentSubModelsHead;
	/*! Vector of of already sorted state dependencies that are evaluated *after* the graph of
		unordered models (tail). The models in this container have a priority larger than
		AbstractStateDependency::priorityOffsetTail.
		The container is populated in registerStateDependendModel().
	*/
	std::vector<ParallelStateObjects>						m_orderedStateDependentSubModelsTail;

	/*! Stores different dependency patterns.
		Index 0: ydot-y dependencies
		Index 1: ydot-FMU input dependencies (only if inputs and outputs exist)
		Index 2: FMU output-y dependencies (only if inputs and outputs exist)
		Index 3: FMU output-input dependencies (only if inputs and outputs exist)
	*/
	std::vector<IBKMK::SparseMatrixPattern>					m_dependencyPatterns;


	/*! Holds references to room state models (does not own the models), size m_nZones. */
	std::vector<RoomStatesModel*>							m_roomStatesModelContainer;
	/*! Holds references to room balance models (does not own the models), size m_nZones. */
	std::vector<RoomBalanceModel*>							m_roomBalanceModelContainer;
	/*! Vector with offsets for conserved quantities of the room models. */
	std::vector<unsigned int>								m_zoneVariableOffset;

	/*! Holds references to construction state models (does not own the models), size m_nWalls. */
	std::vector<ConstructionStatesModel*>					m_constructionStatesModelContainer;
	/*! Holds references to construction balance models (does not own the models), size m_nWalls. */
	std::vector<ConstructionBalanceModel*>					m_constructionBalanceModelContainer;
	/*! Vector with offsets for conserved quantities of the construction models.
		\code
		statesIndex = m_constructionVariableOffset[constructionIndex];
		y[statesIndex] = <first conserved quantity of construction>;
		\endcode
	*/
	std::vector<unsigned int>								m_constructionVariableOffset;

	/*! Holds references to thermal network state models (does not own the models). */
	std::vector<ThermalNetworkStatesModel*>					m_networkStatesModelContainer;
	/*! Holds references to thermal network balance models (does not own the models). */
	std::vector<ThermalNetworkBalanceModel*>					m_networkBalanceModelContainer;
	/*! Vector with offsets for conserved quantities of the network models.
	*/
	std::vector<unsigned int>								m_networkVariableOffset;

	/*! Used to translates generic variable names ('NetwokElement(id=3)') to displayNames ('HeatExchanger_id3') */
	std::map<std::string, std::string>						m_varSubstitutionMap;

	// *** Models and sub-models ***

	/*! Climatic loads calculation model (exists as single instance).
		This is merely a quick-access pointer to the loads model, but it does
		not own the model. The Loads model is inserted into m_modelContainer like
		any other model.
		\note The loads object is _not_ inserted in the m_stateModelContainer
			  because it is always evaluated first on each call to setTime().
	*/
	Loads													*m_loads = nullptr;

	/*! Single object/model providing schedules quantities. */
	Schedules												*m_schedules = nullptr;


	// *** STATISTICS ***

	/*! Log file containing realtime, simtime, and gliding average. */
	std::ostream											*m_progressLog = nullptr;
	/*! Holds seconds elapsed in last run in case solver was continued.
		This value is normally = 0, except when the solver was restarted. Then, the
		time needed in the previous run is stored in this variable.
	*/
	double													m_elapsedSecondsAtStart;
	/*! Holds simulation time in [s] already elapsed at first call of the model
		(sim time already computed when the solver was restarted).
		This value is normally = t0(), except when the solver was restarted. Then, the
		simulation time reached in the previous run is stored in this variable.
	*/
	double													m_elapsedSimTimeAtStart;

	SOLFRA::SolverFeedback									m_feedback;

protected:
	/*! FMI import/export model. */
	FMIInputOutput											*m_fmiInputOutput = nullptr;

};

} // namespace NANDRAD_MODEL

#endif // NM_NandradModelH
