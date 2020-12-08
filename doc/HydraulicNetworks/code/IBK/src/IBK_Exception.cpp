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

#include "IBK_Exception.h"
#include "IBK_messages.h"

namespace IBK {

Exception::Exception()
{
}

Exception::~Exception() throw() {
}

Exception::Exception(const std::string& what, const std::string& loc) {
	msgs_.push_back( IBK::Exception::MsgInfo(what, loc) );
}

Exception::Exception(const IBK::FormatString& what, const std::string& loc) {
	msgs_.push_back( IBK::Exception::MsgInfo(what.str(), loc) );
}

Exception::Exception(const Exception & old, const std::string& what, const std::string& loc) :
	msgs_(old.msgs_)
{
	msgs_.push_back( IBK::Exception::MsgInfo(what, loc) );
}

Exception::Exception(const Exception & old, const IBK::FormatString& what, const std::string& loc) :
	msgs_(old.msgs_)
{
	msgs_.push_back( IBK::Exception::MsgInfo(what.str(), loc) );
}

Exception::Exception(const std::exception & old, const IBK::FormatString& what, const std::string& loc){
	msgs_.push_back( IBK::Exception::MsgInfo( old.what(), loc ) );
	msgs_.push_back( IBK::Exception::MsgInfo(what.str(), loc) );
}


const char* Exception::what() 	const throw() {
	if (msgs_.empty()) return "";
	else return msgs_.back().what.c_str();
}


const char* Exception::location() const {
	if (msgs_.empty()) return "";
	else return msgs_.back().location.c_str();
}

void Exception::writeMsgStackToError() const {
	for (std::list<MsgInfo>::const_iterator it = msgs_.begin();
		it != msgs_.end(); ++it)
	{
		IBK::IBK_Message(it->what, MSG_ERROR, it->location.c_str(), VL_STANDARD);
	}
}

std::string Exception::msgStack() const {
	std::string allMsgs;
	for (std::list<MsgInfo>::const_iterator it = msgs_.begin();
		it != msgs_.end(); ++it)
	{
		allMsgs += std::string(it->what);
		std::list<MsgInfo>::const_iterator itNext = it;
		if (++itNext != msgs_.end())
			allMsgs +=" \n";
	}
	return allMsgs;
}

} // namespace IBK

