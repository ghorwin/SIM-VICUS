#ifndef SOLFRA_PrecondADISparseH
#define SOLFRA_PrecondADISparseH

#include "SOLFRA_PrecondInterface.h"

#include <vector>

namespace IBKMK {
	class SparseMatrixEID;
}

namespace SOLFRA {

class IntegratorInterface;
class ModelInterfaceADI;

/*! An ADI-based pre-conditioner for use with Sundials solvers.
	For now it only supports x and y direction ADI.
*/
class PrecondADISparse : public SOLFRA::PrecondInterface {
public:
	/*! Type of the ADI splitting. */
	enum PrecondSolveType {
		SolveLeft = 1,
		SolveRight = 2
	};

	/*! Initializes PrecondADISparse.
		\param indicesX Index mapping array, size n*m.
		\param indicesY Index mapping array, size n*m.
		\param n Number of blocks.
		\param m Block size.
		\param precondType Defines the type of the preconditioner.

		The number of unknowns is typically based on CMK numbering, that means
		it is independent of x or y direction. The indicesX and indicesY vectors
		now map a regular index to the index as if the grid was numbered in X or Y
		direction, respectively.
		\code
		// convert from x-numbering index to index in vector with solution variables
		unsigned int valueIdx = indicesX[xBasedIndex];
		\endcode
	*/
	PrecondADISparse(unsigned int n, PreconditionerType precondType);

	/*! Destructor, releases band matrix memory. */
	~PrecondADISparse() override;

	/*! Returns type of precondition (where it should be applied in context of the iteration linear equation solver). */
	virtual PreconditionerType preconditionerType() const override { return m_precondType; }

	/*! Initialize the preconditioner, called from the
		framework before integration is started. */
	virtual void init(SOLFRA::ModelInterface * model, SOLFRA::IntegratorInterface * integrator,
					  const JacobianInterface * jacobianInterface) override;

	/*! In this function, the preconditioner matrix is composed an LU-factorised.
		This function is called from the linear equation solver during iterations.
		\param y The current prediction of the solution.
		\param residuals The currentl prediction of the solution.
	*/
	virtual int setup(double t, const double * y, const double * ydot, const double * residuals,
		bool jacOk, bool & jacUpdated, double gamma) override;

	virtual int solve(double t, const double * y, const double * ydot, const double * residuals,
		const double * r, double * z, double gamma, double delta, int lr) override;

	/*! Called from the framework to write create statistics file and write its header. */
	virtual void writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) override;

	/*! Writes currently collected statistics. */
	virtual void writeStatistics(double t) override;

	/*! Holds number of RHS function evaluations (ydot()/residual() calls) used for generating
		the preconditioner.
	*/
	virtual unsigned int nRHSEvals() const override { return m_nRhsEvals; }

private:

	/*! Preconditioning type: Left or Both. */
	PrecondInterface::PreconditionerType	m_precondType;

	/*! Statistics file stream. */
	std::ostream							*m_statsFileStream;

	SOLFRA::IntegratorInterface				*m_integrator;

	/*! Pointer to the underlying model. */
	SOLFRA::ModelInterfaceADI				*m_modelADI;

	/*! Number of matrix bocks = number of discretisation elements. */
	unsigned int							m_n;
	/*! Sparse indices for x-step. */
	std::vector<unsigned int>				m_indicesX;
	/*! Sparse indices for y-step. */
	std::vector<unsigned int>				m_indicesY;

	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotX;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotY;
	/*! Global solution vector of two splitting steps. */
	std::vector<double>						m_residualsX;
	/*! Global rhs vector. */
	std::vector<double>						m_residualsY;

	/*! ID numbers for ODE equations (size n). */
	std::vector<double>						m_odeIDs;

	/*! Used to store individual ydot elements. */
	std::vector<double>						m_y;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydot;
	/*! Used to store individual y elements. */
	std::vector<double>						m_yMod;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotMod;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydiff;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydotDiff;
	/*! Stores the modified residuals/ydot values during the FD algorithm. */
	std::vector<double>						m_residualsMod;
	/*! Stores the modified JX * v values during the FD algorithm. */
	std::vector<double>						m_JvX;
	/*! Stores the modified JY * v values during the FD algorithm. */
	std::vector<double>						m_JvY;

	/*! Global solution vector of two splitting steps. */
	std::vector<double>						m_z;
	/*! Used to store solution vector for splitting step 1. */
	std::vector<double>						m_zX;
	/*! Used to store solution vector for splitting step 2. */
	std::vector<double>						m_zY;
	/*! Used to store any scalar vector for product JX*v. */
	std::vector<double>						m_vX;
	/*! Used to store any scalar vector for product JY*v. */
	std::vector<double>						m_vY;
	/*! Band matrix implementation for x-splitting (owned). */
	IBKMK::SparseMatrixEID					*m_jacobianX;
	/*! Band matrix implementation for y-splitting (owned). */
	IBKMK::SparseMatrixEID					*m_jacobianY;
	/*! Jacobian matrix (not LU factorized) with respect to df/dy (part of J = I - gamma * df/dy). */
	IBKMK::SparseMatrixEID					*m_partialJacobianCopyX;
	/*! Jacobian matrix (not LU factorized) with respect to df/dy (part of J = I - gamma * df/dy). */
	IBKMK::SparseMatrixEID					*m_partialJacobianCopyY;
	/*! Bandwidth of the Sparse Matrix (needed for DQ algorithm). */
	unsigned int							m_bandWidthX;
	/*! Bandwidth of the Sparse Matrix (needed for DQ algorithm). */
	unsigned int							m_bandWidthY;

	/*! Relative tolerance to compute epsilon in difference-quotient approximation. */
	double									m_relToleranceDQ;
	/*! Absolute tolerance to compute epsilon in difference-quotient approximation. */
	double									m_absToleranceDQ;
	/*! Number of jacobian evaluations for implicit Euler integrator. */
	unsigned int							m_nJacEvals;
	/*! Number of rhs evaluations for implicit Euler integrator. */
	unsigned int							m_nRhsEvals;
};

} // namespace SOLFRA

#endif // SOLFRA_PrecondADISparse
