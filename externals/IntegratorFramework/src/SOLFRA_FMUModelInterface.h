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

#ifndef SOLFRA_FMUModelInterfaceH
#define SOLFRA_FMUModelInterfaceH

#include <string>

namespace SOLFRA {

/*! Interface functions for models that can be used within FMU containers.
	Implement the functions declared in this interface for proper FMU for CoSimulation
	and ModelExchange functionality.
	This class interface must be inherited by models that are passed to the
	FMUCoSimFramework class in addition to the regular ModelInterface class interface.
*/
class FMUModelInterface {
public:
	typedef const char * ConstString;

	virtual ~FMUModelInterface() {}

	/*! Sets a new input parameter of type double. */
	virtual void setReal(int varID, double value) { (void)varID; (void)value; }
	/*! Sets a new input parameter of type int. */
	virtual void setInteger(int varID, int value) { (void)varID; (void)value; }
	/*! Sets a new input parameter of type string. */
	virtual void setString(int varID, ConstString  value) { (void)varID; (void)value; }
	/*! Sets a new input parameter of type bool. */
	virtual void setBoolean(int varID, bool value) { (void)varID; (void)value; }

	/*! Retrieves an output parameter of type double. */
	virtual void getReal(int varID, double & value) { (void)varID; (void)value; }
	/*! Retrieves an output parameter of type int. */
	virtual void getInteger(int varID, int & value) { (void)varID; (void)value; }
	/*! Retrieves an output parameter of type string.
		\code
		// at global scope
		std::string myStaticString;

		// store reference
		value = myStaticString.c_str();
		\endcode
	*/
	virtual void getString(int varID, ConstString & value) { (void)varID; value = ""; }
	/*! Retrieves an output parameter of type bool. */
	virtual void getBoolean(int varID, bool & value) { (void)varID; (void)value; }

	/*! This function is called by the master/control system whenever a communication
		interval is started or restarted.
		The model should implement all functionality related to resetting temporary
		outputs made already in previous iterations over this interval, that means
		clear all outputs past the given tStart value.

		For example, that store output data in memory this function should reset the index
		for the current output. Subsequently new outputs may overwrite existing outputs in memory.

		In solvers with temporary output files these files must be recreated/reset.
	*/
	virtual void startCommunicationInterval(double tStart, bool noSetFMUStatePriorToCurrentPoint) { (void)tStart; (void)noSetFMUStatePriorToCurrentPoint; }

	/*! This function is called by the master/control system whenever a communication
		interval has been completed.
		The model should implement all functionality to write/append (temporary) outputs
		collected so far into persistant output files.

		In models that store output data in memory this function does not need to be
		implemented. In solvers with temporary output files the content of these files must
		be appended to the persistant output files.
	*/
	virtual void completeCommunicationInterval() {}

	/*! Can be re-implemented to do something in the model just after an integration step
		has been completed.
		This function is only called in ModelExchange mode from InstanceDataCommon::completedIntegratorStep().
	*/
	virtual void completedIntegratorStep(double t, const double * y) { (void)t; (void)y; }
};

} // namespace SOLFRA


#endif // SOLFRA_FMUModelInterfaceH


