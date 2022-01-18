#ifndef SOLFRA_JacobianDenseH
#define SOLFRA_JacobianDenseH

#include "SOLFRA_JacobianInterface.h"

#include <vector>
#include <iosfwd>

#include <IBKMK_DenseMatrix.h>

namespace SOLFRA {

/*! An implementation of Jacobian matrix generation and J times v implementation.
	Currently only supports ODE-type models.
*/
class JacobianDense : public JacobianInterface, public IBKMK::DenseMatrix {
public:
	/*! Constructor */
	JacobianDense();

	/*! Initialize the Jacobian matrix (resizes matrix and vectors). */
	virtual void init(ModelInterface * model) override;

	/*! In this function the Jacobian matrix is generated using directional
		derivatives.
		\param t The current time point in [s].
		\param y The current prediction of the solution.
		\param ydot The current time derivatives of the solution variables.
		\param residuals The current prediction of the solution.
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

	/*! Relative tolerance to compute epsilon in difference-quotient approximation. */
	double									m_relToleranceDQ;
	/*! Absolute tolerance to compute epsilon in difference-quotient approximation. */
	double									m_absToleranceDQ;

	/*! Computes and returns serialization size. */
	std::size_t serializationSize() const override;

	/*! Stores content at memory location pointed to by dataPtr and increases
	pointer afterwards to point just behind the memory occupied by the copied data.
	Default implementation does nothing.
	*/
	void serialize(void* & dataPtr) const override;

	/*! Restores content from memory at location pointed to by dataPtr and increases
	pointer afterwards to point just behind the memory occupied by the copied data.
	Default implementation does nothing.
	*/
	void deserialize(void* & dataPtr) override;

	/*! Restores content from memory at location pointed to by dataPtr and increases
	pointer afterwards to point just behind the memory occupied by the copied data.
	Resizes matrix to requested dimensions and populates matrix with stored content.
	*/
	void recreate(void* & dataPtr);

private:
	/*! Pointer to the underlying model. */
	ModelInterface							*m_model;

	/*! Used to store individual y elements. */
	std::vector<double>						m_yMod;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotMod;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydiff;

	/*! Number of rhs evaluations for Band preconditioner. */
	unsigned int							m_nRhsEvals;
};

} // namespace SOLFRA

#endif // SOLFRA_JacobianDense
