/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

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

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.

*/

#ifndef IBKMK_randomH
#define IBKMK_randomH

#include <ctime>

namespace IBKMK {

/*! Implementation (policy) class for Random.
	This is the simple, single-state random generator which is
	fast and can be used for all everyday purposes.
*/
class RandomGeneratorSimple {
protected:
	/*! Constructor, initializes the state of the generator. */
	RandomGeneratorSimple(unsigned long long  seed) :
		state(4101842887655102017LL)
	{
		state ^= seed;
		state = createInt64();
	}

	/*! Generates a 64 bit integer number. */
	inline unsigned long long createInt64() {
		// improve generator by some xor-shift operations: a1..a3 = 21, 35,4
		state ^= state >> 21;
		state ^= state << 35;
		state ^= state >> 4;
		// MLCG Modulo Generator - a = 2685821657736338717
		return state * 2685821657736338717LL;
	}

private:
	/*! The only state of the generator. */
	unsigned long long state;
};

/*! Implementation (policy) class for Random.
	This is the full power random number generator with three individual
	states.
*/
class RandomGeneratorFull {
protected:
	/*! Constructor, initializes the state of the generator. */
	RandomGeneratorFull(unsigned long long  seed) :
		y(4101842887655102017LL),
		z(1)
	{
		x = seed ^ y;
		y = x;
		createInt64();
		z = y;
		createInt64();
	}

	/*! Generates a 64 bit integer number. */
	inline unsigned long long createInt64() {
		// LCG Module generator (the old type)
		x = x * 2862933555777941757LL + 7046029254386353087LL;
		// xor-shift generator
		y ^= y >> 17;
		y ^= y << 31;
		y ^= y >> 8;
		// z - MWC Generator, multiply with Carry
		z = 4294957665U*(z & 0xffffffff) + (z >> 32);
		// combination of different generators
		unsigned long long tmp = x ^ ( x << 21 );
		tmp ^= tmp >> 35;
		tmp ^= tmp << 4;
		return (tmp + y) ^ z;
	}

private:
	/*! The state of the LCG Module generator. */
	unsigned long long x;
	/*! The state of the XOR-SHIFT generator. */
	unsigned long long y;
	/*!  The state of the MWC Module generator. */
	unsigned long long z;
};


/*! Common interface for Random Number Generators.
	Use the following interface when using the simple and fast generator:
	\code
	double val = Random<RandomGeneratorSimple>::double64();
	unsigned int i32 = Random<RandomGeneratorSimple>::int32();
	unsigned long long i64 = Random<RandomGeneratorSimple>::int64();
	\endcode
	or when using the more complex and detailed generator:
	\code
	double val = Random<RandomGeneratorFull>::double64();
	unsigned int i32 = Random<RandomGeneratorFull>::int32();
	unsigned long long i64 = Random<RandomGeneratorFull>::int64();
	\endcode
*/
template <typename Generator = RandomGeneratorSimple>
class Random : public Generator {
public:

	/*! Generates a 64 bit integer number. */
	static inline unsigned long long int64() {
		return instance().createInt64();
	}

	/*! Generates a 32 bit integer number. */
	static inline unsigned int int32() {
		return static_cast<unsigned int>(instance().createInt64());
	}

	/*! Generates a 64 bit double floating point number. */
	static inline double double64() {
		return 5.42101086242752217e-20 * instance().createInt64();
	}

private:

	/*! Creates an instance of the random number generator. */
	static Random & instance() {
		static Random my_rand(time(NULL));
		return my_rand;
	}

	/*! Initialize with any integer number. */
	Random(unsigned long long seed) : Generator(seed)
	{
	}
};

/*! Typedef for the simple but fast random number generator.
	Use as in the example below:
	\code
	double val = Ran::int64();
	\endcode
*/
typedef Random<RandomGeneratorSimple> Ran;

/*! Typedef for the simple but fast random number generator.
	Use as in the example below:
	\code
	double val = RanFull::int64();
	\endcode
*/
typedef Random<RandomGeneratorFull> RanFull;

} // namespace IBKMK

#endif // IBKMK_randomH
