#ifndef SOLFRA_JacobianSparseH
#define SOLFRA_JacobianSparseH

#include "SOLFRA_JacobianInterface.h"

#include <vector>

#include <IBKMK_SparseMatrix.h>

#include <IBK_StopWatch.h>

namespace SOLFRA {

/*! An abstract interface for sparse Jacobian implementations for use with iterative solvers.
*/
class JacobianSparse : public JacobianInterface {
public:

	/*! Virtual destructor, so that destructors of derived classes will be called. */
	virtual ~JacobianSparse() {}

	/*! Create and releases a copy of the internally stored sparse matrix holding the system jacobian.
		\note Caller takes ownership of allocated memory.
	*/
	virtual IBKMK::SparseMatrix * createAndReleaseJacobianCopy() const = 0;

	/*! Returns read-only access to currently stored system jacobian. */
	virtual const IBKMK::SparseMatrix * jacobian() const = 0;
};

} // namespace SOLFRA

#endif // SOLFRA_JacobianSparse
