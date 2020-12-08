// IBK Library - Collection of classes, functions and algorithms
//               for development of numeric simulations
// 
// Copyright (C) 2009 Andreas Nicolai, Andreas.Nicolai@gmx.net
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.


#include "IBK_configuration.h"

#include "IBK_htmlhelp.h"

namespace IBK {

bool HtmlHelp::loadDll() {
	unloadDll();
	dllInstance = LoadLibrary( "HHCTRL.OCX" );
	if (!dllInstance) return false;
	(FARPROC&) htmlHelpFunction = GetProcAddress( dllInstance, "HtmlHelpA" );
	if (!htmlHelpFunction)	return false;
	else					return true;
}

void HtmlHelp::unloadDll() {
	if (dllInstance != 0) {
		FreeLibrary(dllInstance);
		dllInstance = 0;
		htmlHelpFunction = 0;
	}
}

HWND HtmlHelp::htmlHelp(HWND parent, LPCSTR topic, UINT command, DWORD q) const {
	// ensure that DLL was properly loaded before continuing
	if (!dllInstance) return 0;

	return htmlHelpFunction(parent, topic, command, q);
}

HWND HtmlHelp::displayHelp(unsigned short kind, const std::string& helpfile, const std::string& topic, HWND parent) const {
	// ensure that DLL was properly loaded before continuing
	if (!dllInstance) return 0;

	HH_FTS_QUERY q;
	q.cbStruct         = sizeof(HH_FTS_QUERY);
	q.fUniCodeStrings  = false;
	q.pszSearchQuery   = "";
	q.iProximity       = HH_FTS_DEFAULT_PROXIMITY;
	q.fStemmedSearch   = false;
	q.fTitleOnly       = false;
	q.fExecute         = false;
	q.pszWindow        = NULL;

	HWND hwnd;
	switch(kind)
	{
		case HH_DISPLAY_INDEX:  	hwnd = htmlHelpFunction( parent, helpfile.c_str(), HH_DISPLAY_INDEX, NULL ); break;
		case HH_DISPLAY_TOC:		hwnd = htmlHelpFunction( parent, helpfile.c_str(), HH_DISPLAY_TOC, ( DWORD )&q ); break;
		case HH_DISPLAY_SEARCH: 	hwnd = htmlHelpFunction( parent, helpfile.c_str(), HH_DISPLAY_SEARCH, ( DWORD )&q ); break;
		case HH_DISPLAY_TOPIC:
		{
			std::string top = helpfile + "::/" + topic + ".htm";
			hwnd = htmlHelpFunction( parent, top.c_str(), HH_DISPLAY_TOPIC, NULL );
			break;
		}
// not implemented yet, this flag needs additional values
//			case HH_DISPLAY_TEXT_POPUP:	hwnd = htmlHelpFunction( parent, helpfile.c_str(), HH_DISPLAY_TEXT_POPUP, NULL ); break;
		default: return 0;
	}
	return hwnd;
}
//---------------------------------------------------------------------------


const HtmlHelp&  HtmlHelper() {
	static HtmlHelp h;
	if (h.dllInstance == 0) {
		h.loadDll();
	}
	return h;
}

} // namespace IBK
