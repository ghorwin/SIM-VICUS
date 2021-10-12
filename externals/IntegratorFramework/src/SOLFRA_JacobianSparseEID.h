#ifndef SOLFRA_JacobianSparseEIDH
#define SOLFRA_JacobianSparseEIDH

#include "SOLFRA_JacobianSparse.h"

#include <vector>

#include <IBKMK_SparseMatrixEID.h>

#include <IBK_StopWatch.h>

namespace SOLFRA {

/*! A sparse Jacobian implementation for use with iterative solvers.

	\todo Implement prover ellpack-itpack format with additional columnbased index table
		  which can be generated upon construction from the row-based index table.
		  The "symmetric" flag is still useful, because it reduces memory consumption about 1/3.
*/
class JacobianSparseEID : public JacobianSparse, private IBKMK::SparseMatrixEID {
public:

	/*! Type of coloring method for Jacobian generation to use. */
	enum ColoringType {
		/*! Dense generation, requires n RHS evaluations. */
		Dense,
		/*! Curtis-Powell-Reid group algorithm, requires m_bandWidth parameter to be set, requires
			n/m_bandWidth + 1 RHS evaluations. */
		CurtisPowellReid,
		/*! Determine coloring method automatically based on index array. */
		Automatic
	};

	/*! Initializes JacobianSparseEID.
		\param n Dimension
		\param elementsPerRow Maximum number of non-zero elements per matrix row.
		\param indices Array of size n x elementsPerRow with column indices.
		\sa IBK::SparseMatrix
	*/
	JacobianSparseEID(unsigned int n, unsigned int elementsPerRow, const unsigned int * indices, bool symmetric);

	/*! Specify coloring type to use for Jacobian generation.
		\param coloringType The method to use for generating sparse matrix Jacobian data
							via directional derivatives.
		This function generates required data automatically based on the selected coloring type.
		If coloring type is CurtisPowellReid, m_bandwidth is computed. For automatic, the
		m_colors vector is populated.
	*/
	void setColoringType(ColoringType coloringType);

	/*! Initializes sparse matrix. */
	virtual void init(ModelInterface * model);

	/*! In this function, the preconditioner matrix is composed an LU-factorised.
		This function is called from the linear equation solver during iterations.
		\param y The current prediction of the solution.
		\param residuals The currentl prediction of the solution.
	*/
	virtual int setup(double t, const double * y, const double * ydot, const double * residuals,
		double gamma);

	/*! Implementation of the matrix-vector product function. */
	virtual int jacTimesVec(const double * v, double * Jv) const {
		multiply(v, Jv);
		return 0;
	}

	/*! Holds number of RHS function evaluations (ydot()/residual() calls) used for generating
		the Jacobian matrix.
	*/
	virtual unsigned int nRHSEvals() const { return m_nRhsEvals; }

	/*! Create and releases a copy of the internally stored sparse matrix holding the system jacobian.
		\note Caller takes ownership of allocated memory.
	*/
	virtual IBKMK::SparseMatrix * createAndReleaseJacobianCopy() const;

	/*! Returns read-only access to currently stored system jacobian. */
	virtual const IBKMK::SparseMatrix * jacobian() const { return this; }

	/*! Computes and returns serialization size, by default returns returns an invalid value (-1). */
	std::size_t serializationSize() const;

	/*! Stores content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	void serialize(void* & dataPtr) const;

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	void deserialize(void* & dataPtr);

	/*! Restores content from memory at location pointed to by dataPtr and increases
	pointer afterwards to point just behind the memory occupied by the copied data.
	Resizes matrix to requested dimensions and populates matrix with stored content.
	*/
	void recreate(void* & dataPtr);

	/*! Relative tolerance to compute epsilon in difference-quotient approximation. */
	double									m_relToleranceDQ;
	/*! Absolute tolerance to compute epsilon in difference-quotient approximation. */
	double									m_absToleranceDQ;

protected:
	/*! Pointer to the underlying model. */
	ModelInterface							*m_model;

	/*! Coloring method to use for computing sparse Jacobian. */
	ColoringType							m_coloringType;

	/*! If true, a symmetric structure (not symmetric matrix) is used (improves Jacobian generation algorithm). */
	bool									m_symmetric;

	/*! Bandwidth of the Sparse Matrix (needed for DQ algorithm when coloring type is set to CurtisPowellReid). */
	unsigned int							m_bandWidth;

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

#endif // SOLFRA_JacobianSparseEID
