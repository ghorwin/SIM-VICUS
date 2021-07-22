#ifndef DM_LESADIH
#define DM_LESADIH

#include "SOLFRA_LESInterfaceDirect.h"

#include <vector>

namespace IBKMK {
	class BlockTridiagMatrix;
	class TridiagMatrix;
};

namespace SOLFRA {

class IntegratorInterface;
class ModelInterfaceADI;

/*! An ADI-based linear equation system solver.
	For now it only supports x and y direction ADI.
*/
class LESADI : public SOLFRA::LESInterfaceDirect {
public:
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
		\todo move indices to ModelInterfaceADI
	*/
	LESADI(unsigned int n, unsigned int m);

	/*! Destructor, releases band matrix memory. */
	~LESADI();

	/*! Re-implemented from LESInterface::init(). 	*/
	void init(ModelInterface * model, IntegratorInterface * integrator,
			  PrecondInterface * precond, JacobianInterface * jacobian);

	/*! Re-implemented from LESInterface::setup(). 	*/
	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma);

	/*! Re-implemented from LESInterface::solve(). */
	virtual void solve(double * rhs);

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
	/*! Used to store individual ydot elements computed by the right-hand-side function of ODE models. */
	std::vector<double>						m_ydotMod;
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
};

} // namespace SOLFRA

#endif // SOLFRA_LESADI
