#ifndef SOLFRA_LESGMRESH
#define SOLFRA_LESGMRESH

#include "SOLFRA_LESInterfaceIterative.h"

#include <vector>
#include <iosfwd>

namespace IBKMK {
	class BandMatrix;
}

namespace SOLFRA {


class JacobianInterface;
class PrecondInterface;
class ModelInterface;
class IntegratorInterface;

/*! A custom LESGMRES implementation to be used with ImplicitEuler for testing purposes. If used in
	conjunction with CVODE integrator, the GMRES implementation from Sundials is being used.
*/
class LESGMRES : public LESInterfaceIterative {
public:

	/*! Constructor */
	LESGMRES();

	/*! Destructor. */
	~LESGMRES() override;

	/*! Initialize the linear equation solver, called from the
		framework before integration is started.
	*/
	virtual void init(ModelInterface * model, IntegratorInterface * integrator, PrecondInterface * precond,
					  JacobianInterface * jacobian) override;

	/*! Setup of equation system.
		\sa LESInterface::setup
	*/
	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma) override;

	/*! Solves equation system.
		\sa LESInterface::solve
	*/
	virtual void solve(double * rhs) override;

	/*! Called from the framework to write create statistics file and write its header. */
	virtual void writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) override;

	/*! Writes currently collected statistics. */
	virtual void writeStatistics(double t) override;

	/*! Computes and returns serialization size. */
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

private:
	/*! Implementation of the matrix-vector product function. */
	int ATimesVec(const double * v, double * Av, double t, double * y, double * ydot, double* residuals);
	int ClassicalGramSchmidt(const double *Avl, unsigned int l);
	int ModifiedGramSchmidt(const double *Avl, unsigned int l);
	int QRfactorisation(unsigned int l);
	int QRsolve(unsigned int lmax, double* rhs);
	double dotProduct(const double *v, const double *w, unsigned int n);
	double norm2(const double *v, unsigned int n);

	/*! Hessian matrix for owned GMRES calculation. */
	IBKMK::BandMatrix						*m_hessian;
	/*! Matrix of base vectors for Krylov subspace. */
	std::vector<std::vector<double> >		m_V;
	/*! Vector for the product A*v. */
	std::vector<double>						m_W;
	/*! Givens rotation: size 2*(kryldim+1). */
	std::vector<double>						m_givens;
	/*! Used to store individual y elements. */
	std::vector<double>						m_y;
	/*! Correction vector for the given solution. */
	std::vector<double>						m_yCorr;
	/*! Correction vector for the given solution. */
	std::vector<double>						m_yCorrTemp;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydot;
	/*! Stores the modified residuals/ydot values during the FD algorithm. */
	std::vector<double>						m_residuals;
	/*! Used to store individual weighting function for scaling the residuals. */
	std::vector<double>						m_weights;

	/*! Used to store individual y elements. */
	std::vector<double>						m_yMod;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotMod;
	/*! Used to store individual right hand side of ODE models. */
	std::vector<double>						m_FMod;
	/*! ID numbers for ODE equations (size n). */
	std::vector<double>						m_odeIDs;
	/*! Vector for temporary results, size = problem dimension */
	std::vector<double>						m_temp;
	/*! Vector for temporary results, size = problem dimension */
	std::vector<double>						m_res;
	/*! Vector for temporary results, size = krylov space dimension. */
	std::vector<double>						m_tempKryl;
	/*! Time step size. */
	double									m_gamma;
	/*! Error limit. */
	double									m_delta;
#ifdef DEBUG_EIGENVALUES
	/*! Flag indicating whether a calculation of eigenvalues is requested. */
	bool									m_eigenValueUpdateNeeded;
	/*! Estimated Eigenvalues auf PL^(-1) A PR^(-1). */
	std::vector<double>						m_eigenValueEstH;
#endif

};

} // namespace SOLFRA


#endif // SOLFRA_SundialsBandLESH
