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

#ifndef IBK_NotificationHandlerH
#define IBK_NotificationHandlerH

namespace IBK {

/*! \brief This is an abstract base class for notification objects.

	It can be used to allow call backs from a library function to another
	library without the need of making one library dependend on the other.

	Re-implement this class and use it in lengthy functions to get
	feedback information during the loading/writing process.
	The following example shows a very simple implementation of a
	notification handler that writes up to a line of hash
	characters while reading the file:
	\code
	class Notify : public IBK::NotificationHandler {
	public:
		// Initializing constructor
		Notify() : counter(0) {}

		// For functions that do not support percentage notification, simply print a
		// twisting -\|/- image.
		void notify() {
			std::cout << '\r';
			switch (counter) {
				case 0 : cout << "-";
				case 1 : cout << "\";
				case 2 : cout << "|";
				case 3 : cout << "/";
				default : ; // just here to prevent compiler warnings
			}
			counter = ++counter % 4;
			std::cout.flush();
		}

		// For functions that support percentage notification, print a
		// progress bar and percentage information.
		virtual void notify(double percentage) {
			std::cout << '\r';
			std::cout << std::setw(3) << std::right << std::fixed << std::setprecision(0) << 100*percentage;
			int chars_so_far = (int)(70*percentage);
			std::cout << " " << std::string(chars_so_far, '#');
			std::cout.flush();
		}

		unsigned int counter;
	};
	\endcode
*/
class NotificationHandler {
public:
	/*! Virtual destructor. */
	virtual ~NotificationHandler() {}
	/*! Reimplement this function in derived child 'notification' objects
		to provide whatever notification operation you want to perform.
	*/
	virtual void notify() = 0;
	/*! Reimplement this function in derived child 'notification' objects
		to provide whatever notification operation you want to perform.

		The default implementation calls notify().
	*/
	virtual void notify(double ) { notify(); }
};

} // namespace IBK

#endif // IBK_NotificationHandlerH
