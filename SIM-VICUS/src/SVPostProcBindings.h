#ifndef SVPostProcBindingsH
#define SVPostProcBindingsH

#include <QString>
#include <IBK_Path.h>

/*! This class encapsulates everything related to PostProc interaction functionality. */
class SVPostProcBindings {
public:

	/*! This function generates and returns the path to the default session file. */
	static IBK::Path defaultSessionFilePath(const QString & projectFile);

	/*! This function generates the default post-proc session file using information from
		the project.
	*/
	static void generateDefaultSessionFile(const QString & projectFile);

};

#endif // SVPostProcBindingsH
