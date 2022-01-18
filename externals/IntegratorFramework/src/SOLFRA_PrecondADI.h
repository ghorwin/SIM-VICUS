#ifndef SOLFRA_PrecondADIH
#define SOLFRA_PrecondADIH

#include "SOLFRA_PrecondInterface.h"
#include "SOLFRA_JacobianInterface.h"

#include <vector>

namespace IBKMK {
	class BlockTridiagMatrix;
	class TridiagMatrix;
};

namespace SOLFRA {

class IntegratorInterface;
class ModelInterfaceADI;

/*! An ADI-based pre-conditioner for use with Sundials solvers.
	For now it only supports x and y direction ADI.
*/
class PrecondADI : public SOLFRA::PrecondInterface, public SOLFRA::JacobianInterface {
public:
	/*! Type of the ADI splitting. */
	enum PrecondSolveType {
		SolveLeft = 1,
		SolveRight = 2
	};

	/*! Initializes PrecondADI.
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
	PrecondADI(unsigned int n, unsigned int m,
			PreconditionerType precondType);

	/*! Destructor, releases band matrix memory. */
	~PrecondADI() override;

	/*! Returns type of precondition (where it should be applied in context of the iteration linear equation solver). */
	virtual PreconditionerType preconditionerType() const override { return m_precondType; }

	/*! Initialize the preconditioner, called from the
		framework before integration is started.
		Derived from PrecondInterface*/
	virtual void init(SOLFRA::ModelInterface * model, SOLFRA::IntegratorInterface * integrator,
					  const JacobianInterface * jacobianInterface) override;

	/*! Initialize the Jacobian matrix generator, called from the
		framework before integration is started.
		Derived from JacobianInterface.*/
	virtual void init(ModelInterface * model) override;

	/*! This function is derived from PreconditionerInterface.
		This function is called from the linear equation solver during iterations.
		\param t The current time step.
		\param y The current prediction of the solution.
		\param ydot The corresponding result of f(y).
		\param residuals The current prediction of the solution.
		\param gamma The factor in front of the derivative df/dy (dt in case of Implicit Euler).
	*/
	virtual int setup(double t, const double * y, const double * ydot, const double * residuals,
		bool jacOk, bool & jacUpdated, double gamma) override;

	/*! This setup function is inherited from JacobianInterface.
		The Jacobian matrix is composed an LU-factorised.
		This function is called from the linear equation solver during iterations.
	*/
	virtual int setup(double t, const double * y, const double * ydot, const double * residuals,
		double gamma) override;

	/*! This function solves the system P^(-1) v = r. */
	virtual int solve(double t, const double * y, const double * ydot, const double * residuals,
		const double * r, double * z, double gamma, double delta, int lr) override;

	/*! Function implementing J*v operation. */
	virtual int jacTimesVec(const double * v, double * Jv) const override;

	/*! Called from the framework to write create statistics file and write its header. */
	virtual void writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) override;

	/*! Writes currently collected statistics. */
	virtual void writeStatistics(double t) override;

	/*! Holds number of RHS function evaluations (ydot()/residual() calls) used for generating
		the preconditioner.
	*/
	virtual unsigned int nRHSEvals() const override { return m_nRhsEvals; }

private:

	/*! Pointer to the underlying model. */
	SOLFRA::ModelInterfaceADI				*m_modelADI;

	/*! Number of matrix bocks = number of discretisation elements. */
	unsigned int							m_n;
	/*! Block size = number of equations. */
	unsigned int							m_m;

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
	/*! Used to store individual right hand side of ODE models. */
	std::vector<double>						m_FMod;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydiff;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydotDiff;
	/*! Stores the modified residuals/ydot values during the FD algorithm. */
	std::vector<double>						m_residualsMod;

	/*! Global solution vector of two splitting steps. */
	std::vector<double>						m_z;
	/*! Used to store solution vector for splitting step 1. */
	std::vector<double>						m_zX;
	/*! Used to store solution vector for splitting step 2. */
	std::vector<double>						m_zY;
	/*! Band matrix implementation for x-splitting (owned). */
	IBKMK::BlockTridiagMatrix				*m_jacobianX;
	/*! Band matrix implementation for y-splitting (owned). */
	IBKMK::BlockTridiagMatrix				*m_jacobianY;
	/*! Jacobian matrix (not LU factorized) with respect to df/dy (part of J = I - gamma * df/dy). */
	IBKMK::BlockTridiagMatrix				*m_partialJacobianCopyX;
	/*! Jacobian matrix (not LU factorized) with respect to df/dy (part of J = I - gamma * df/dy). */
	IBKMK::BlockTridiagMatrix				*m_partialJacobianCopyY;

	/*! Factor psi defining the method type: split via P = 1/(2*psi)(A1 - psi * gamma I)(A2 - psi * gamma I). */
	double									m_psi;
	/*! Relative tolerance to compute epsilon in difference-quotient approximation. */
	double									m_relToleranceDQ;
	/*! Absolute tolerance to compute epsilon in difference-quotient approximation. */
	double									m_absToleranceDQ;
	/*! Number of jacobian evaluations for ADI preconditioner. */
	unsigned int							m_nJacEvals;
	/*! Number of rhs evaluations for ADI preconditioner. */
	unsigned int							m_nRhsEvals;
};

} // namespace SOLFRA

#endif // SOLFRA_PrecondADI
