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

#include "IBK_configuration.h"

#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <iostream>

#include "IBK_StopWatch.h"

#if defined(_OPENMP)

	// in OpenMP code, use omp_get_wtime()
	#include <omp.h>

#else // _OPENMP

/// \todo Add code for Intel Compiler on Unix
#if defined(__GNUC__) && !defined(__MINGW32__)

	// on Unix systems, use gettimeofday()
	#include <sys/time.h> // for gettimeofday()

#else // Unix

	// on Windows systems use std::clock()
	#include <ctime>    // for std::clock()

#define USE_QPC_STOPWATCH
#ifdef USE_QPC_STOPWATCH
	#include <windows.h> // for QueryPerformanceCounter
#endif // USE_QPC_STOPWATCH

#endif // Unix

#endif // _OPENMP


namespace IBK {


#if defined(_OPENMP)

/*! OpenMP version of the StopWatch implementation. */
class StopWatchImpl {
public:
	/*! Starts the stop watch. */
	void start() {
		start_ = omp_get_wtime();
		stop_  = 0;
	}

	/*! Stops the stop watch. */
	double stop() {
		stop_ = omp_get_wtime();
		return (stop_-start_)*1000.0;
	}

	/*! Returns the current elapsed time. */
	double difference() const {
		if (stop_!=0) {
			return (stop_-start_)*1000.0;
		}
		else {
			double ct = omp_get_wtime();
			return (ct-double(start_))*1000.0;
		}
	}

	/*! Starting wall clock time. */
	double start_;
	/*! End wall clock time, if stopwatch was stopped, otherwise 0. */
	double stop_;
};


#else // _OPENMP


/// \todo Add code for Intel Compiler on Unix
#if defined(__GNUC__) && !defined(__MINGW32__)

/*! gettimeofday() version of the StopWatch implementation. */
class StopWatchImpl {
public:
	/*! Starts the stop watch. */
	void start() {
		gettimeofday(&start_, NULL);
		stop_.tv_sec  = 0;
	}

	/*! Stops the stop watch. */
	double stop() {
		gettimeofday(&stop_, NULL);
		return ((stop_.tv_sec  - start_.tv_sec) * 1000000u + stop_.tv_usec - start_.tv_usec) / 1.e3;
	}

	/*! Returns the current elapsed time. */
	double difference() const {
		if (stop_.tv_sec != 0) {
			return ((stop_.tv_sec  - start_.tv_sec) * 1000000u + stop_.tv_usec - start_.tv_usec) / 1.e3;
		}
		else {
			struct timeval ct;
			gettimeofday(&ct, NULL);
			return ((ct.tv_sec  - start_.tv_sec) * 1000000u + ct.tv_usec - start_.tv_usec) / 1.e3;
		}
	}

	/*! Starting wall clock time. */
	struct timeval start_;
	/*! End wall clock time, if stopwatch was stopped, otherwise 0. */
	struct timeval stop_;
};

#else // Unix

#ifdef USE_QPC_STOPWATCH

/*! Windows QPC version of the StopWatch implementation. */
class StopWatchImpl {
public:
	/*! Starts the stop watch. */
	void start() {
		QueryPerformanceCounter(&start_);
		stop_.QuadPart = 0;
	}

	/*! Stops the stop watch. */
	double stop() {
		QueryPerformanceCounter(&stop_);
		LARGE_INTEGER freq, misecs;
		QueryPerformanceFrequency(&freq);
		misecs.QuadPart = stop_.QuadPart - start_.QuadPart;
		double secs = (double)misecs.QuadPart*1000; // convert to seconds
		secs /= freq.QuadPart;
		return secs;
	}

	/*! Returns the current elapsed time. */
	double difference() const {
		LARGE_INTEGER freq, misecs;
		QueryPerformanceFrequency(&freq);
		if (stop_.QuadPart == 0) {
			LARGE_INTEGER nostop;
			QueryPerformanceCounter(&nostop);
			misecs.QuadPart = nostop.QuadPart - start_.QuadPart;
		}
		else {
			misecs.QuadPart = stop_.QuadPart - start_.QuadPart;
		}
		double secs = (double)misecs.QuadPart*1000; // convert to seconds
		secs /= freq.QuadPart;
		return secs;
	}

	/*! Starting wall clock time. */
	LARGE_INTEGER start_;
	/*! End wall clock time, if stopwatch was stopped, otherwise 0. */
	LARGE_INTEGER stop_;
};

#else // USE_QPC_STOPWATCH

/*! Windows version of the StopWatch implementation. */
class StopWatchImpl {
public:
	/*! Starts the stop watch. */
	void start() {
		start_=std::clock();
		stop_=0;
	}

	/*! Stops the stop watch. */
	double stop() {
		stop_ = std::clock();
		return (stop_-start_)*1000.0 / CLOCKS_PER_SEC;
	}

	/*! Returns the current elapsed time. */
	double difference() const {
		if (stop_!=0) {
			return (stop_-start_)*1000.0 / CLOCKS_PER_SEC;
		}
		else {
			std::clock_t c = std::clock();
			double ct = c;
			return (ct-double(start_))*1000.0 / CLOCKS_PER_SEC;
		}
	}

	/*! Starting wall clock time. */
	std::clock_t start_;
	/*! End wall clock time, if stopwatch was stopped, otherwise 0. */
	std::clock_t stop_;
};

#endif // USE_QPC_STOPWATCH

#endif // Unix

#endif // _OPENMP




// *** Implementation of StopWatch, for all plattforms and versions ***

StopWatch::StopWatch() :
	m_intervalLengthInSeconds(15),
	m_lastIntervalDiff(0),
	m_p(new StopWatchImpl)
{
	m_p->start();
}

StopWatch::~StopWatch() {
	delete m_p;
}

void StopWatch::start() {
	m_p->start();
	m_lastIntervalDiff = 0;
}

double StopWatch::stop() {
	return m_p->stop();
}

double StopWatch::difference() const {
	return m_p->difference();
}

const std::string StopWatch::diff_str() const {
	std::stringstream strm;
	double diff=m_p->difference();
	if (diff<1000)
		strm << static_cast<int>(diff) << " ms";
	else if (diff < 60000)
		strm << std::fixed << std::setprecision(3) << diff/1000.0 << " s";
	else if (diff < 3600000)
		strm << std::fixed << diff/60000.0 << " min";
	else
		strm << std::fixed << diff/3600000.0 << " h";
	return strm.str();
}

const std::string StopWatch::diff_str(std::size_t width) const {
	std::stringstream strm;
	double diff=m_p->difference();
	if (diff<1000)
		strm << std::setw(static_cast<int>(width)-3) << static_cast<int>(diff) << " ms";
	else if (diff < 60000)
		strm << std::setw(static_cast<int>(width)-2) << std::fixed << std::setprecision(3) << diff/1000.0 << " s";
	else if (diff < 3600000)
		strm << std::setw(static_cast<int>(width)-4) << std::fixed << std::setprecision(3) << diff/60000.0 << " min";
	else
		strm << std::setw(static_cast<int>(width)-2) << std::fixed << std::setprecision(3) << diff/3600000.0 << " h";
	return strm.str();
}


void StopWatch::setIntervalLength(double intervalLengthInSeconds) {
	m_intervalLengthInSeconds = intervalLengthInSeconds;
}

bool StopWatch::intervalCompleted() {
	if (m_lastIntervalDiff + m_intervalLengthInSeconds < difference()/1000) {
		m_lastIntervalDiff += m_intervalLengthInSeconds;
		return true;
	}
	return false;
}

}  // namespace IBK
