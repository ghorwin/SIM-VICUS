/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#ifndef NANDRAD_SolverParameterH
#define NANDRAD_SolverParameterH

#include <string>
#include <IBK_Parameter.h>
#include <IBK_Flag.h>

#include "NANDRAD_CodeGenMacros.h"

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class SolverParameters

	Solver parameters define options/flags related to the numerical engine, and are
	typically independent of the physical model evaluation.

	\sa SimulationParameter
*/
class SolverParameter {
public:

	/*! Parameter. */
	enum para_t {
		P_RELTOL,							// Keyword: RelTol							[---]	'Relative tolerance for solver error check.'
		P_ABSTOL,							// Keyword: AbsTol							[---]	'Absolute tolerance for solver error check.'
		P_MAX_DT,							// Keyword: MaxTimeStep						[min]	'Maximum permitted time step for integration.'
		P_MIN_DT,							// Keyword: MinTimeStep						[s]		'Minimum accepted time step, before solver aborts with error.'
		P_INITIAL_DT,						// Keyword: InitialTimeStep					[s]		'Initial time step'
		P_NONLINSOLVERCONVCOEFF,			// Keyword: NonlinSolverConvCoeff			[---]	'Coefficient reducing nonlinear equation solver convergence limit.'
		P_ITERATIVESOLVERCONVCOEFF,			// Keyword: IterativeSolverConvCoeff		[---]	'Coefficient reducing iterative equation solver convergence limit.'
		P_MAX_ORDER,						// Keyword: MaxOrder						[---]	'Maximum order allowed for multi-step solver.'
		P_MAX_KRYLOV_DIM,					// Keyword: MaxKrylovDim					[---]	'Maximum dimension of Krylov subspace.'
		P_MAX_NONLIN_ITER,					// Keyword: MaxNonlinIter					[---]	'Maximum number of nonlinear iterations.'
		P_LES_BANDWIDTH,					// Keyword: LESBandWidth					[---]	'Maximum band width to be used for band LES solver.'
		P_PRE_BANDWIDTH,					// Keyword: PreBandWidth					[---]	'Maximum band width to be used for banded preconditioner.'
		P_PRE_ILUWIDTH,						// Keyword: PreILUWidth						[---]	'Maximum level of fill-in to be used for ILU preconditioner.'
		P_DISCRETIZATION_MIN_DX,			// Keyword: DiscMinDx						[m]		'Minimum element width for wall discretization.'
		P_DISCRETIZATION_DETAIL,			// Keyword: DiscDetailLevel					[---]	'Level of detail for wall discretization.'
		P_VIEW_FACTOR_TILE_WIDTH,			// Keyword: ViewfactorTileWidth				[m]		'Maximum dimension of a tile for calculation of view factors.'
		P_SURFACE_DISCRETIZATION_DENSITY,	// Keyword: SurfaceDiscretizationDensity	[---]	'Number of surface discretization elements of a wall in each direction.'
		P_CONTROL_TEMPERATURE_TOLERANCE,	// Keyword: ControlTemperatureTolerance		[K]		'Temperature tolerance for ideal heating or cooling.'
		P_KINSOL_RELTOL,					// Keyword: KinsolRelTol					[---]	'Relative tolerance for Kinsol solver.'
		P_KINSOL_ABSTOL,					// Keyword: KinsolAbsTol					[---]	'Absolute tolerance for Kinsol solver.'
		P_KINSOL_MAX_NONLIN_ITER,			// Keyword: KinsolMaxNonlinIter				[---]	'Maximum nonlinear iterations for Kinsol solver.'
		P_INTEGRAL_WEIGHTS_FACTOR,			// Keyword: IntegralWeightsFactor			[---]	'Optional weighting factor for integral outputs.'
		NUM_P
	};

	/*! Flags.
		\note A flag that is not set is defined as off.
	*/
	enum flag_t {
		F_DETECT_MAX_DT,				// Keyword: DetectMaxTimeStep			'Check schedules to determine minimum distances between steps and adjust MaxTimeStep.'
		F_KINSOL_DISABLE_LINE_SEARCH,	// Keyword: KinsolDisableLineSearch		'Disable line search for steady state cycles.'
		F_KINSOL_STRICT_NEWTON,			// Keyword: KinsolStrictNewton			'Enable strict Newton for steady state cycles.'
		NUM_F
	};

	/*! Enumeration of available integrators. */
	enum integrator_t {
		I_CVODE,						// Keyword: CVODE						'CVODE based solver'
		I_EXPLICIT_EULER,				// Keyword: ExplicitEuler				'Explicit Euler solver'
		I_IMPLICIT_EULER,				// Keyword: ImplicitEuler				'Implicit Euler solver'
		NUM_I							// Keyword: auto						'System selects integrator automatically.'
	};

	/*! Enumeration of available linear equation system (LES) solvers, to be used with
		implicit integrators.
	*/
	enum lesSolver_t {
		LES_DENSE,						// Keyword: Dense						'Dense solver'
		LES_KLU,						// Keyword: KLU							'KLU sparse solver'
		LES_GMRES,						// Keyword: GMRES						'GMRES iterative solver'
		LES_BICGSTAB,					// Keyword: BiCGStab					'BICGSTAB iterative solver'
		NUM_LES							// Keyword: auto						'System selects les solver automatically.'
	};

	/*! Enumeration of available preconditioners, to be used with iterative LES solvers. */
	enum precond_t {
		PRE_BAND,						// Keyword: Band						'Band preconditioner'
		PRE_ILU,						// Keyword: ILU							'Incomplete LU preconditioner'
		NUM_PRE							// Keyword: auto						'System selects preconditioner automatically.'
	};


	/*! Default constructor. */
	SolverParameter();

	/*! Init default values (called before readXML()).
		\note These values will be overwritten in readXML() when the respective property is set
			  in the project file.
	*/
	void initDefaults();

	NANDRAD_READWRITE

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const SolverParameter & other) const;
	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const SolverParameter & other) const { return ! operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***

	/*! List of parameters. */
	IBK::Parameter		m_para[NUM_P];			// XML:E
	/*! List of flags. */
	IBK::Flag			m_flag[NUM_F];			// XML:E

	/*! Selected integrator engine.
		\sa integrator_t
	*/
	integrator_t		m_integrator;			// XML:E

	/*! Selected LES solver.
		\sa lesSolver_t
	*/
	lesSolver_t			m_lesSolver;			// XML:E

	/*! Selected preconditioner.
		\sa precond_t
	*/
	precond_t			m_preconditioner;		// XML:E
};

} // namespace NANDRAD

#endif // SolverParameterH
