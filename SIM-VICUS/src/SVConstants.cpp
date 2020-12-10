#include  "SVConstants.h"

const char * const ORG_NAME			= "IBK";

/*! WARNING: The PROGRAM_NAME is primarily used for writing registry/program config with the respective
			 activation information. Since this is specific for a <major>.<minor> version number,
			 the PROGRAM_NAME *must* contain both version strings.
			 It needs to be synchronized with the VERSION string in VICUS_Constants.h
*/
const char * const PROGRAM_NAME		= "SIM-VICUS 0.2";

const char * const UPDATE_FILE_URL	= "...";
const char * const NEWS_FILE_URL	= "...";
const char * const BUG_REPORT_URL	= "...";
const char * const FORUM_URL		= "...";
const char * const MANUAL_URL		= "https://ghorwin.github.io/SIM-VICUS";

const char * const SUPPORT_EMAIL	= "...";

#if defined(Q_OS_MAC) // Q_OS_UNIX

const char * const FIXED_FONT_FAMILY = "Monaco";

#elif defined(Q_OS_UNIX)

const char * const FIXED_FONT_FAMILY = "Monospace";

#else

const char * const FIXED_FONT_FAMILY = "Courier New";

#endif

unsigned int USER_ID_SPACE = 100000;
