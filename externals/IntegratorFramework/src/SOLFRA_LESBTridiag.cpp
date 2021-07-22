#include "SOLFRA_LESBTridiag.h"

#include <cstring>
#include <cmath>
#include <fstream>
#include <iomanip>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_IntegratorImplicitEuler.h"
#include "SOLFRA_ModelInterface.h"

#include <cvode/cvode_btridiag.h>

#include <sundials/sundials_direct.h>

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>

//#define DUMP_BTRIDIAG_JACOBIAN

namespace SOLFRA {

LESBTridiag::LESBTridiag(unsigned int nblocks, unsigned int blocksize) :
	m_nblocks(nblocks),
	m_blocksize(blocksize),
	m_modelODE(nullptr),
	m_jacobian(nullptr)
{
}


LESBTridiag::~LESBTridiag() {
	if (m_jacobian != nullptr)
		DestroyMat(reinterpret_cast<DlsMat>(m_jacobian));
}


void LESBTridiag::init(ModelInterface * model, IntegratorInterface * integrator, PrecondInterface * precond, JacobianInterface * /*jacobian*/) {
	const char * const FUNC_ID = "[LESBTridiag::init]";

	m_integrator = integrator;
	m_modelODE = model;

	// determine which preconditioner we are using
	if (precond != nullptr)
		IBK::IBK_Message( "LESBTridiag solver is chosen: Preconditioner will be ignored.", IBK::MSG_WARNING, FUNC_ID);
	// determine which integrator we are using
	if (IntegratorSundialsCVODE * intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(integrator)) {
		// create and register CVBTridiag with integrator
		int result = CVBTridiag(intCVODE->cvodeMem(), m_nblocks, m_blocksize);
		if (result != CVDLS_SUCCESS) {
			if (result == CVDLS_MEM_FAIL)
				IBK::IBK_Message( "CVBTridiag memory initialization error (problem too large?)", IBK::MSG_ERROR, FUNC_ID);
			else if (result == CVDLS_ILL_INPUT)
				IBK::IBK_Message( "CVBTridiag init error (wrong input?)", IBK::MSG_ERROR, FUNC_ID);
			else
				IBK::IBK_Message( "CVBTridiag init error", IBK::MSG_ERROR, FUNC_ID);
			throw IBK::Exception("Error initializing linear equation solver.", FUNC_ID);
		}
	}
	else if (IntegratorImplicitEuler * intImpEuler = dynamic_cast<IntegratorImplicitEuler*>(integrator)) {
		(void)intImpEuler;
		// for now, only resize matrix and vectors when using implicit Euler (so that we are not wasting memory)
		m_n = model->n();
		// create new BTridiag matrix
		m_jacobian = NewBTridiagMat(m_nblocks, m_blocksize);
		// resize all vectors
		m_yMod.resize(m_n);
		m_ydotMod.resize(m_n);
		m_FMod.resize(m_n);
		m_ydiff.resize(m_n);
		m_pivots.resize(m_nblocks*m_blocksize);
	}
	else {
		throw IBK::Exception("Error initializing BlockTridiag linear solver: "
			"solver is only defined for CVODE, IDA or implicit Euler integrator.",FUNC_ID);
	}
}


void LESBTridiag::setup(const double * y, const double * ydot, const double * residuals, double gamma) {

	(void) residuals;

	// only setup linear equation system explicitely if integrator implicit Euler is chosen
	IBK_ASSERT(dynamic_cast<IntegratorImplicitEuler *>(m_integrator) != nullptr);
	IBK_ASSERT(m_modelODE != nullptr);
	// store current solution guess
	std::memcpy(&m_yMod[0], y, m_yMod.size()*sizeof(double));

	DlsMat M = reinterpret_cast<DlsMat>(m_jacobian);

	// compose jacobian
	/* Loop over block-based column groups (=3 for any tridiag matrix) . */
	for (unsigned int  column = 0; column < 3; ++column) {
		/* Loop over all columns in block */
		for (unsigned int j = 0; j< m_blocksize ; ++j) {

			/* Increment all y_i in all blocks k of the group */
			for (unsigned int k = column; k < m_nblocks; k += 3) {
				/* determine global column or row i in y vector */
				unsigned int i = k * m_blocksize + j;
				// modify all y value at column j
				m_ydiff[i] = std::fabs(y[i]) * m_relToleranceDQ + m_absToleranceDQ;
				m_yMod[i] += m_ydiff[i];
			}

			/* Update model with incremented y */
			// calculate modified right hand side
			m_modelODE->setY(&m_yMod[0]);
			// calculate modified right hand side of the model, and store f(t,y) in m_FMod
			m_modelODE->ydot(&m_FMod[0]);
			// F = y - yn - dt*ydot,
			// derivative: 1 - dt * dydot/dy
			++m_statNumRhsEvals; // increase function evaluation counter

			/* Restore ytemp, then form and load difference quotients
				k is the block-column index
			*/
			for (unsigned int k = column; k < m_nblocks; k += 3) {
				/* i = index of modified y value (y_data[i]) */
				unsigned int i = k * m_blocksize + j ;
				// reset y vector
				m_yMod[i] = y[i];

				/* we have to fill-in all the DQ elements in
					all rows of the blocks k-1, k and k+1 */
				if ( k > 0 ) {
					/* set values in upper band */
					double *block = BTRIDIAG_UPPER(M, k - 1);
					/* i2 = index of first equation affected by y_data[i] */
					unsigned int i2 = ( k - 1 ) * m_blocksize;
					// fill block according to model type
					if (m_modelODE != nullptr) {
						for (unsigned int i1 = 0; i1 < m_blocksize; ++i1, ++i2) {
							BTRIDIAG_BLOCK_ELEM(M, block, i1, j) = - gamma * ( m_FMod[i2] - ydot[i2] )/ m_ydiff[i];
						}
					}
				}

				/*! special case, block L[0] = J_{0,2} */
				if (k==2) {
					/* set values in first block in lower band */
					double *block = BTRIDIAG_LOWER(M, 0);
					unsigned int i2 = 0;
					// fill block according to model type
					if(m_modelODE != nullptr) {
						for (unsigned int i1 = 0; i1 < m_blocksize; ++i1, ++i2) {
							BTRIDIAG_BLOCK_ELEM(M, block, i1, j) = - gamma * ( m_FMod[i2] - ydot[i2] )/ m_ydiff[i];
						}
					}
				}

				/* set values in main band */
				double *block = BTRIDIAG_MAIN(M, k);
				unsigned int i2 = k* m_blocksize;
				if (m_modelODE != nullptr) {
					for (unsigned int i1 = 0; i1 < m_blocksize; ++i1, ++i2) {
						BTRIDIAG_BLOCK_ELEM(M, block, i1, j) = - gamma * ( m_FMod[i2] - ydot[i2] )/ m_ydiff[i];
						// add identity matrix
						if( i2 == i )
							BTRIDIAG_BLOCK_ELEM(M, block, i1, j) += 1.0;
					}
				}

				if ( k < m_nblocks - 1) {
					/* set values in lower band */
					double *block = BTRIDIAG_LOWER(M, k + 1);
					unsigned int i2 = ( k + 1 ) * m_blocksize;
					// fill block according to model type
					if(m_modelODE != nullptr) {
						for (unsigned int i1 = 0; i1 < m_blocksize; ++i1, ++i2) {
							BTRIDIAG_BLOCK_ELEM(M, block, i1, j) = - gamma * ( m_FMod[i2] - ydot[i2] )/ m_ydiff[i];
						}
					}
				}

				/*! special case, block U[n] = J_{n-1,n-3} */
				if (k == m_nblocks - 3) {
					/* set values in last block in upper band */
					double *block = BTRIDIAG_UPPER(M, m_nblocks - 1);
					unsigned int i2 = ( m_nblocks - 1 ) * m_blocksize;
					// fill block according to model type
					if(m_modelODE != nullptr) {
						for (unsigned int i1 = 0; i1 < m_blocksize; ++i1, ++i2) {
							BTRIDIAG_BLOCK_ELEM(M, block, i1, j) = - gamma * ( m_FMod[i2] - ydot[i2] )/ m_ydiff[i];
						}
					}
				}
			} /* for (k = column; ; k < m_nblocks; k += 3) */
		} /* for (j = 0; j < m_ blocksize; ++j) */
	} /* for (column = 0; column < 3; ++column) */

#ifdef DUMP_JACOBIAN_TEXT
	SaveMat(M, "jacobian_btridiag.txt");
	throw IBK::Exception("Done with test-dump of Jacobian", "[LESBTridiag::setup]");
#endif
	// Do LU factorization of jacobian
	int ier = BTridiagBTTRF(M, &m_pivots[0]);
	/// \todo error handling
	(void)ier;
	++m_statNumJacEvals;
}


void LESBTridiag::solve(double * rhs) {
	// only solve linear equation system if integrator implicit Euler is chosen
	IBK_ASSERT(dynamic_cast<IntegratorImplicitEuler *>(m_integrator) != nullptr);

	DlsMat M = reinterpret_cast<DlsMat>(m_jacobian);
	BTridiagBTTRS(M, &m_pivots[0], rhs);
}


} // namespace SOLFRA

