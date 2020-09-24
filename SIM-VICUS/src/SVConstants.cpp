#include  "SVConstants.h"

const char * const ORG_NAME			= "IBK";

/*! WARNING: The PROGRAM_NAME is primarily used for writing registry/program config with the respective
			 activation information. Since this is specific for a <major>.<minor> version number,
			 the PROGRAM_NAME *must* contain both version strings.
			 It needs to be synchronized with the VERSION string in VICUS_Constants.h
*/
const char * const PROGRAM_NAME		= "SIM-VICUS 1.0";

const char * const UPDATE_FILE_URL	= "...";
const char * const NEWS_FILE_URL	= "...";
const char * const BUG_REPORT_URL	= "...";
const char * const FORUM_URL		= "...";

const char * const SUPPORT_EMAIL	= "...";

#if defined(Q_OS_MAC) // Q_OS_UNIX

const char * const FIXED_FONT_FAMILY = "Monaco";

#elif defined(Q_OS_UNIX)

const char * const FIXED_FONT_FAMILY = "Monospace";

#else

const char * const FIXED_FONT_FAMILY = "Courier New";

#endif

const char * const EditFieldBackground						= "#f9f6c8";
const char * const AlternativeEditFieldBackground			= "#f9ffd8";
const char * const ErrorEditFieldBackground					= "#ff7777";
const char * const AlternativeErrorEditFieldBackground		= "#ff7087";
const char * const ReadOnlyEditFieldBackground				= "#d6e9ff";

const char * const AlternativeBackgroundBright				= "#fff4b8";
const char * const AlternativeBackgroundDark				= "#ffe49d";
const char * const AlternativeBackgroundText				= "#760000";

