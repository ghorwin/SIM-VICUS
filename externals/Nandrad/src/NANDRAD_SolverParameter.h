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

#ifndef NANDRAD_SolverParameterH
#define NANDRAD_SolverParameterH

#include <string>
#include <IBK_Parameter.h>
#include <IBK_IntPara.h>
#include <IBK_Flag.h>

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	Solver parameters define options/flags related to the numerical engine, and are
	typically independent of the physical model evaluation.

	\sa SimulationParameter
*/
class SolverParameter {
public:

	/*! Parameter. */
	enum para_t {
		/*! Relative tolerance for solver error check,  default 1e-5. */
		P_RelTol,							// Keyword: RelTol							[---]	'Relative tolerance for solver error check.'
		/*! Absolute tolerance for solver error check, default 1e-10. */
		P_AbsTol,							// Keyword: AbsTol							[---]	'Absolute tolerance for solver error check.'
		/*! Maximum permitted time step for integration, default 1 h. */
		P_MaxTimeStep,						// Keyword: MaxTimeStep						[min]	'Maximum permitted time step for integration.'
		/*! Minimum accepted time step, before solver aborts with error, default 1e-12 s. */
		P_MinTimeStep,						// Keyword: MinTimeStep						[s]		'Minimum accepted time step, before solver aborts with error.'
		/*! Initial time step size (or constant step size for ExplicitEuler integrator), default 0.1 s. */
		P_InitialTimeStep,					// Keyword: InitialTimeStep					[s]		'Initial time step size (or constant step size for ExplicitEuler integrator).'
		P_NonlinSolverConvCoeff,			// Keyword: NonlinSolverConvCoeff			[---]	'Coefficient reducing nonlinear equation solver convergence limit.'
		P_IterativeSolverConvCoeff,			// Keyword: IterativeSolverConvCoeff		[---]	'Coefficient reducing iterative equation solver convergence limit.'
		/*! Minimum element width for wall discretization, default 2 mm. */
		P_DiscMinDx,						// Keyword: DiscMinDx						[m]		'Minimum element width for wall discretization.'
		/*! Stretch factor for variable wall discretizations (0-no disc, 1-equidistance, larger than 1 - variable), default 4. */
		P_DiscStretchFactor,				// Keyword: DiscStretchFactor				[---]	'Stretch factor for variable wall discretizations (0-no disc, 1-equidistance, larger than 1 - variable).'
		P_ViewfactorTileWidth,				// Keyword: ViewfactorTileWidth				[m]		'Maximum dimension of a tile for calculation of view factors.'
		P_SurfaceDiscretizationDensity,		// Keyword: SurfaceDiscretizationDensity	[---]	'Number of surface discretization elements of a wall in each direction.'
		P_ControlTemperatureTolerance,		// Keyword: ControlTemperatureTolerance		[K]		'Temperature tolerance for ideal heating or cooling.'
		P_KinsolRelTol,						// Keyword: KinsolRelTol					[---]	'Relative tolerance for Kinsol solver.'
		P_KinsolAbsTol,						// Keyword: KinsolAbsTol					[---]	'Absolute tolerance for Kinsol solver.'
		P_HydraulicNetworkAbsTol,			// Keyword: HydraulicNetworkAbsTol			[---]	'Absolute tolerance (WRMS threshold) for Newton method in hydraulic network.'
		P_HydraulicNetworkMassFluxScale,	// Keyword: HydraulicNetworkMassFluxScale	[---]	'Scale factor for mass fluxes in solution vector of hydraulic network.'
		NUM_P
	};

	/*! Integer parameters. */
	enum intPara_t {
		IP_PreILUWidth,						// Keyword: PreILUWidth								'Maximum level of fill-in to be used for ILU preconditioner.'
		IP_MaxKrylovDim,					// Keyword: MaxKrylovDim							'Maximum dimension of Krylov subspace.'
		IP_MaxNonlinIter,					// Keyword: MaxNonlinIter							'Maximum number of nonlinear iterations.'
		IP_MaxOrder,						// Keyword: MaxOrder								'Maximum order allowed for multi-step solver.'
		IP_KinsolMaxNonlinIter,				// Keyword: KinsolMaxNonlinIter						'Maximum nonlinear iterations for Kinsol solver.'
		IP_DiscMaxElementsPerLayer,			// Keyword: DiscMaxElementsPerLayer					'Maximum number of elements per layer.'
		NUM_IP
	};

	/*! Flags. */
	enum flag_t {
		F_DetectMaxTimeStep,				// Keyword: DetectMaxTimeStep			'Check schedules to determine minimum distances between steps and adjust MaxTimeStep.'
		F_KinsolDisableLineSearch,			// Keyword: KinsolDisableLineSearch		'Disable line search for steady state cycles.'
		F_KinsolStrictNewton,				// Keyword: KinsolStrictNewton			'Enable strict Newton for steady state cycles.'
		NUM_F
	};

	/*! Enumeration of available integrators. */
	enum integrator_t {
		I_CVODE,						// Keyword: CVODE						'CVODE based solver'
		I_ExplicitEuler,				// Keyword: ExplicitEuler				'Explicit Euler solver'
		I_ImplicitEuler,				// Keyword: ImplicitEuler				'Implicit Euler solver'
		NUM_I							// Keyword: auto						'Automatic selection of integrator'
	};

	/*! Enumeration of available linear equation system (LES) solvers, to be used with
		implicit integrators.
	*/
	enum lesSolver_t {
		LES_Dense,						// Keyword: Dense						'Dense solver'
		LES_KLU,						// Keyword: KLU							'KLU sparse solver'
		LES_GMRES,						// Keyword: GMRES						'GMRES iterative solver'
		LES_BiCGStab,					// Keyword: BiCGStab					'BICGSTAB iterative solver'
		NUM_LES							// Keyword: auto						'Automatic selection of linear equation system solver'
	};

	/*! Enumeration of available preconditioners, to be used with iterative LES solvers. */
	enum precond_t {
		PRE_ILU,						// Keyword: ILU							'Incomplete LU preconditioner'
		NUM_PRE							// Keyword: auto						'Automatic selection of preconditioner'
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Init default values (called before readXML()).
		\note These values will be overwritten in readXML() when the respective property is set
			  in the project file.
	*/
	void initDefaults();

	NANDRAD_READWRITE
	NANDRAD_COMP(SolverParameter)

	/*! To be called after readXML() and mainly used to check whether user-provided parameters are in the valid ranges. */
	void checkParameters() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! List of parameters. */
	IBK::Parameter		m_para[NUM_P];					// XML:E
	/*! List of integer value parameters. */
	IBK::IntPara		m_intPara[NUM_IP];				// XML:E
	/*! List of flags. */
	IBK::Flag			m_flag[NUM_F];					// XML:E

	/*! Selected integrator engine.
		\sa integrator_t
	*/
	integrator_t		m_integrator = NUM_I;			// XML:E

	/*! Selected LES solver.
		\sa lesSolver_t
	*/
	lesSolver_t			m_lesSolver = NUM_LES;			// XML:E

	/*! Selected preconditioner.
		\sa precond_t
	*/
	precond_t			m_preconditioner = NUM_PRE;		// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_SolverParameterH
