#ifndef IBK_htmlhelpH
#define IBK_htmlhelpH

#include <windows.h>
#include <htmlhelp.h>
#include <string>

namespace IBK {

/// The class HtmlHelp is a utility class for showing HTML based help files.
/// It loads the necessary HHCTRL.OCX as late as possible in order to avoid the Microsoft Windows bug KB935448
/// http://support.microsoft.com/kb/935448
/// This class is not a component of IBK_lib. Please include this code directly in your application project.
///
/// Usage of class:
/// @code
/// IBK::HtmlHelper().displayHelp(HH_DISPLAY_TOC, "myhelpfile.chm"); // shows the table of contents
/// IBK::HtmlHelper().displayHelp(HH_DISPLAY_TOPIC, "myhelpfile.chm", "mytopic"); // shows a topic
/// @endcode
class HtmlHelp {
public:
	/// Typedef for winapi function
	typedef HWND (WINAPI *FPHH) (HWND, LPCSTR, UINT, DWORD);

	/// Destructor, unloads DLL.
	~HtmlHelp() { unloadDll(); }

	/// Shows a help dialog based on the given parameters
	/// @param kind One of the display kinds defined in HtmlHelp.h
	///             HH_DISPLAY_INDEX        opens the index
	///             HH_DISPLAY_TOC          opens the table of contents
	///             HH_DISPLAY_SEARCH       opens a search dialog
	///             HH_DISPLAY_TOPIC        opens the topic given in (/a topic)
	/// @param helpfile Name of the helpfile (should be with path)
	/// @param topic A string with the help topic (used for HH_DISPLAY_TOPIC)
	/// @return Returns handle to created help window, or zero, if there was a problem with the help system.
	HWND displayHelp(unsigned short kind, const std::string& helpfile, const std::string& topic = "", HWND parent = 0) const;

	/// Allows direct call to help system with full control over all parameters.
	/// @return Returns handle to created help window, or zero, if there was a problem with the help system.
	HWND htmlHelp(HWND parent, LPCSTR topic, UINT command, DWORD q) const;

private:
	/// Private constructor (ensures Singleton)
	HtmlHelp() : dllInstance(0), htmlHelpFunction(0) {}
	/// Private copy constructor (ensures Singleton)
	HtmlHelp(const HtmlHelp&) {}

	/// Public helper function to call help.
	/// @see HtmlHelp for usage examples.
	friend const HtmlHelp& HtmlHelper();

	/// Initialization function, loads Dll, called from HtmlHelper() on first call.
	bool loadDll();
	/// Removes DLL from memory.
	void unloadDll();

	/// Internal pointer to dllInstance in memory, = 0 if DLL is not loaded.
	HWND dllInstance;
	/// Internal pointer to htmlHelpFunction in memory, = 0 if DLL is not loaded.
	FPHH htmlHelpFunction;
};

}

#endif //  IBK_htmlhelpH
