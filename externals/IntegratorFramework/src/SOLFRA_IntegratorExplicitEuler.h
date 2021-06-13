/*	Solver Control Framework
	Copyright (C) 2010  Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IntegratorExplicitEulerH
#define IntegratorExplicitEulerH

#include <string>
#include <vector>

#include "SOLFRA_IntegratorInterface.h"

namespace SOLFRA {

class LESInterface;
class ModelInterface;

/*!	\brief Declaration for class IntegratorExplictEuler

	This is a very simple example implementation that illustrates how
	integrators are built to match the given IntegratorInterface.
*/
class IntegratorExplicitEuler : public IntegratorInterface {
public:
	virtual const char * identifier() const override { return "Explicit Euler"; }

	/*! Default constructor. */
	IntegratorExplicitEuler();


	/*! Release allocated resources (statistics file stream etc). */
	~IntegratorExplicitEuler() override;

	/*! Initializes the Integrator.
		\param model The physical model instance.
		\param lesSolver Linear equation system solver, can be nullptr if (explicit) solver doesn't need LES solver functionality.
		\param precond Preconditioner, in case that an iterative linear equation system solver is used.
			If nullptr, no preconditioner is specified.
		\param t0 Starting time point.
		\param y0 Pointer to linear memory array of size model->n() holding the initial conditions.
	*/
	virtual void init(ModelInterface * model, double t0, const double * y0,
					  LESInterface * lesSolver,
					  PrecondInterface * precond,
					  JacobianInterface * jacobian) override;

	/*! Advances the solution from the current time to the next, thereby
		adjusting the time step.
		If the step was completed sucessfully, the functions t() and dt() can be
		used to query the new time point and the step size just used, respectively.
		\return Returns an enum value indicating the outcome of the step. For
				negative values, the state of the solver is undetermined.
	*/
	virtual IntegratorInterface::StepResultType step() override;

	/*! Returns the current time point in simulation time in [s].
		Current time point is updated in a call to step().
	*/
	virtual double t() const override { return m_t; }

	/*! Returns the time step in [s] that was used in the last step.
		The step size is taken from the model during init() or can be set directly in the integrator.
	*/
	virtual double dt() const override { return m_dt; }

	/*! Returns a pointer to the memory array with interpolated states at the
		given output time.
		\param t_out A time point between the current time point and the
					 previous time point.
		\return A constant pointer to the memory array holding the interpolated data.
		\warning The lifetime of the pointer returned from this function may be limited.
				 At latest after a call to step() or another y_out() call, the pointer
				 is invalidated and has to be queried again.
	*/
	virtual const double * yOut(double t_out) const override;

	/*! Writes the header of a statistics file. */
	virtual void writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) override;

	/*! Writes currently collected solver statistics. */
	virtual void writeStatistics() override;

	/*! Writes currently collected solver metrics/statistics to output.
		\param simtime Totel elapsed wall clock time of simulation in [s] (needed for percentage calculation)
		\param metricsFile If not nullptr, computer-readible metrics are written to the file.
	*/
	virtual void writeMetrics(double simtime, std::ostream * metricsFile=nullptr) override;

	/*! Constant time step size in [s] used in this integrator.
		By default the time step is taken from the model.
	*/
	double m_dt;

private:
	/*! Pointer to model. */
	ModelInterface * m_model;
	/*! Cached system size. */
	unsigned int m_n;
	/*! Current time point in [s]. */
	double m_t;
	/*! States at time point m_t. */
	std::vector<double> m_y;
	/*! States at time point m_t - m_dt. */
	std::vector<double> m_yn;
	/*! Time derivatives at time point m_t. */
	std::vector<double> m_ydot;
	/*! States interpolated in function y_out. */
	mutable std::vector<double> m_yout;
	/*! Residuals of balance equations at time point m_t. */
	std::vector<double> m_res;

	/*! Statistics file stream. */
	std::ostream *m_statsFileStream = nullptr;

	/*! Stores total number of steps. */
	unsigned int m_statNumSteps;
	/*! Stores number of RHS (system function) evaluations done. */
	unsigned int m_statNumRHSEvals;
};

} // namespace SOLFRA

#endif // IntegratorExplicitEulerH
