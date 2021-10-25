#ifndef SOLFRA_JacobianSparseCSRH
#define SOLFRA_JacobianSparseCSRH

#include "SOLFRA_JacobianSparse.h"

#include <vector>

#include <IBKMK_SparseMatrixCSR.h>

#include <IBK_StopWatch.h>

namespace SOLFRA {

/*! A sparse Jacobian implementation for use with iterative solvers.

	\todo Implement prover ellpack-itpack format with additional columnbased index table
		  which can be generated upon construction from the row-based index table.
		  The "symmetric" flag is still useful, because it reduces memory consumption about 1/3.
*/
class JacobianSparseCSR : public JacobianSparse, private IBKMK::SparseMatrixCSR {
public:

	/*! Initializes JacobianSparseCSR.
		\param n Dimension
		\param elementsPerRow Maximum number of non-zero elements per matrix row.
		\param indices Array of size n x elementsPerRow with column indices.
		\sa IBK::SparseMatrix
	*/
	JacobianSparseCSR(unsigned int n, unsigned int nnz, const unsigned int *ia, const unsigned int * ja,
					  const unsigned int *iaT = nullptr, const unsigned int *jaT = nullptr);

	/*! Initializes sparse matrix. */
	virtual void init(ModelInterface * model) override;

	/*! In this function, the preconditioner matrix is composed an LU-factorised.
		This function is called from the linear equation solver during iterations.
		\param y The current prediction of the solution.
		\param residuals The currentl prediction of the solution.
	*/
	virtual int setup(double t, const double * y, const double * ydot, const double * residuals,
		double gamma) override;

	/*! Implementation of the matrix-vector product function. */
	virtual int jacTimesVec(const double * v, double * Jv) const override {
		multiply(v, Jv);
		return 0;
	}

	/*! Holds number of RHS function evaluations (ydot()/residual() calls) used for generating
		the Jacobian matrix.
	*/
	virtual unsigned int nRHSEvals() const override { return m_nRhsEvals; }

	/*! Create and releases a copy of the internally stored sparse matrix holding the system jacobian.
		\note Caller takes ownership of allocated memory.
	*/
	virtual IBKMK::SparseMatrix * createAndReleaseJacobianCopy() const override;

	/*! Returns read-only access to currently stored system jacobian. */
	virtual const IBKMK::SparseMatrix * jacobian() const override { return this; }

	/*! Computes and returns serialization size, by default returns returns an invalid value (-1). */
	virtual std::size_t serializationSize() const override;

	/*! Stores content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void serialize(void* & dataPtr) const override;

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void deserialize(void* & dataPtr) override;

	/*! Restores content from memory at location pointed to by dataPtr and increases
	pointer afterwards to point just behind the memory occupied by the copied data.
	Resizes matrix to requested dimensions and populates matrix with stored content.
	*/
	void recreate(void* & dataPtr) override;

	/*! Relative tolerance to compute epsilon in difference-quotient approximation. */
	double									m_relToleranceDQ;
	/*! Absolute tolerance to compute epsilon in difference-quotient approximation. */
	double									m_absToleranceDQ;

protected:
	/*! Pointer to the underlying model. */
	ModelInterface							*m_model;

	/*! Coloring information, vector of vectors with maximum size m_n, that holds coloring information.
		The outer vector holds colors, whereas the inner vector holds the corresponding columns. The
		vector is generated in setColoringType(), when coloring type is automatic.
	*/
	std::vector<std::vector<unsigned int> >	m_colors;

	/*! Used to store individual y elements. */
	std::vector<double>						m_yMod;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotMod;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydiff;

	/*! Number of rhs evaluations for ILU preconditioner. */
	unsigned int							m_nRhsEvals;
};

} // namespace SOLFRA

#endif // SOLFRA_JacobianSparseCSR
