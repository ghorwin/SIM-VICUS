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

#ifndef IntegratorRungeKutta45H
#define IntegratorRungeKutta45H

#include <string>
#include <vector>

#include "SOLFRA_IntegratorErrorControlled.h"

namespace SOLFRA {

class LESInterface;
class ModelInterface;

/*! \brief Declaration of Runge-Kutta 45 Integrator Engine.

	This is a RK implementation using the Dormand-Prince parameters.

	\todo Implement support for StopTime parameter.
*/
class IntegratorRungeKutta45 : public IntegratorErrorControlled {
public:
	virtual const char * identifier() const override { return "Runge-Kutta (RK45)"; }

	/*! Default constructor.
		\param dt0 Initial time step size in [s], if 0 (the default) time step is
					queried from model.
	*/
	IntegratorRungeKutta45(double dt0 = 0);

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
		Time step size is updated in a call to step().
	*/
	virtual double dt() const override { return m_dt; }

	/*! Returns a pointer to the memory array with interpolated states at the
		given output time.

		The interpolation is done linearly between this and the last output
		values.

		\param t_out A time point between the current time point and the
					 previous time point.
		\return A constant pointer to the memory array holding the interpolated data.
		\warning The lifetime of the pointer returned from this function may be limited.
				 At latest after a call to step() or another y_out() call, the pointer
				 is invalidated and has to be queried again.
	*/
	virtual const double * yOut(double t_out) const override;

	/*! Called from the framework to write create statistics file and write its header. */
	virtual void writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) override;

	/*! Writes currently collected statistics. */
	virtual void writeStatistics() override;

	/*! Writes currently collected solver metrics/statistics to output.
		\param simtime Totel elapsed wall clock time of simulation in [s] (needed for percentage calculation)
		\param metricsFile If not nullptr, computer-readible metrics are written to the file.
	*/
	virtual void writeMetrics(double simtime, std::ostream * metricsFile=nullptr) override;

	/*! Maximum allowed time step. */
	double m_dtMax;

private:
	// takes a trial step
	bool trialStep(double dt);
	// calculates error norm from m_err
	double errorNorm(unsigned int & i_max) const;
	// adjusts step size and returns whether true if step needs to be redone
	bool adjustStepSize(double err);

	int evaluateModel(double t, const double * y, double * ydot) const;


	ModelInterface * m_model;

	/*! Cached problem size. */
	unsigned int m_n;

	/*! Time point in [s]of current state of solver. */
	double m_t;
	/*! Time step size in [s] using in last step. */
	double m_dt;
	/*! Time step size in [s] to be attempted in next step. */
	double m_dtProposed;

	/*! Current solution at m_t. */
	std::vector<double> m_y;
	/*! Solution at m_t - m_dt. */
	std::vector<double> m_yn;

	/*! Estimated solution (only used during step()). */
	std::vector<double> m_ynext;

	/*! Holds interpolated values (updated in y_out()). */
	mutable std::vector<double> m_yout;

	/*! Current derivatives at m_dt. */
	std::vector<double> m_ydot;

	/*! Estimated derivatives (only used during step()). */
	std::vector<double> m_ydotnext;

	std::vector<double> m_alpha2;
	std::vector<double> m_alpha3;
	std::vector<double> m_alpha4;
	std::vector<double> m_alpha5;
	std::vector<double> m_alpha6;

	std::vector<double> m_err;

	/*! Statistics file stream. */
	std::ostream *m_statsFileStream;

	unsigned int m_statNumSteps;
	unsigned int m_statNumRHSEvals;
	unsigned int m_statNumFails;

	double m_lastErr;

}; // class IntegratorRungeKutta45

} // namespace SOLFRA

#endif // IntegratorRungeKutta45H

