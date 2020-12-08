/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#ifndef IBK_StopWatchH
#define IBK_StopWatchH

#include <string>

namespace IBK {

class StopWatchImpl;

/*! The StopWatch class can be used to measure the time used during execution of
	certain program parts.
	Every StopWatch object remembers its time and state. So you can use several
	stop watches simultaneously (for instance a global watch and a watch for
	certain subroutines).

	\code
	StopWatch w;
	someLengthyFunction();
	std::cout << w.diff_str() << " needed for execution of someLengthyFunction()" << std::endl;
	\endcode

	\note This stopwatch can be used also for OpenMP parallelized code. On Unix-Systems
	it uses gettimeofday().

	We use the P-impl pattern to hide the plattform specific includes and code from
	users.

	Interval functionality:
	\code
	StopWatch w;
	// set notification interval in seconds
	w.setIntervalLength(10);
	// start lengthy function
	while (true) {
		if (w.intervalCompleted())
			std::cout << "Still busy..." << std::endl;

		// ...
	}

	\endcode

*/
class StopWatch {
public:
	/*! Default constructor, creates a stop watch and starts it. */
	StopWatch();
	/*! Cleanup of p-impl object. */
	~StopWatch();
	/*! Restarts the clock. */
	void start();
	/*! Stops the clock and remembers the elapsed time.
		\return Returns the elapsed time from start in [ms].
	*/
	double stop();
	/*! Returns the time so far in milli seconds [ms]. */
	double difference() const;
	/*! Returns the time so far as a string with unit (either in milli seconds or seconds).
		\code
		StopWatch w;
		// ...
		std::string s = w.diff_str(); // gives a string of format "1.23 s";
		\endcode
	*/
	const std::string diff_str() const;
	/*! Returns the time so far as a string but allows the specification of the width of
		the resulting string. The time value will be align to the right.
		\param width The field with of the number part.
		\code
		StopWatch w;
		// ...
		std::string s = w.diff_str(6);
		// gives a string of format "  1.23 s";
		\endcode
	*/
	const std::string diff_str(std::size_t width) const;

	/*! Sets interval length.
		\param intervalLength Interval length in [s].
	*/
	void setIntervalLength(double intervalLengthInSeconds);

	/*! Returns true when interval has completed and increases the interval counter.
		Call this function repeatedly in your code to check if the selected interval has
		passed.
	*/
	bool intervalCompleted();

protected:
	/*! Timer check interval in  [s]. */
	double m_intervalLengthInSeconds;

	/*! Time elapsed since start of timer and until last interval in [s]. */
	double m_lastIntervalDiff;

private:
	/*! Pointer to private implementation. */
	StopWatchImpl	*m_p;
};

} // namespace IBK

/*! \file IBK_StopWatch.h
	\brief Contains the declaration of the class StopWatch.

	\example StopWatch.cpp
	This is an example of how to use the class IBK::StopWatch.
*/

#endif // IBK_StopWatchH
