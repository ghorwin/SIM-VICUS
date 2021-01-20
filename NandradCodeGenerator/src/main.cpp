/*!

NandradCodeGenerator - creates NANDRAD_KeywordList.cpp and class-specific implementation files
This file is part of the SIM-VICUS project. See LICENSE file for details.

Authors: Andreas Nicolai @ IBK/TU-Dresden

Example syntax:

	> NandradCodeGenerator NANDRAD /path/to/NANDRAD/src 0 NANDRAD ncg

*/

#include <iostream>
#include <IBK_MessageHandler.h>
#include <IBK_MessageHandlerRegistry.h>

#include "CodeGenerator.h"

const char * const SYNTAX =
		"SYNTAX:  NandradCodeGenerator <namespace> <path/to/src> <generateQtSrc> <prefix> <ncg-dir>\n"
		"         <namespace> is usually NANDRAD (used also to compose file names).\n"
		"         <path/to/<lib>/src> is + separated list of input directories to read the header files from.\n"
		"         Keywordlist-source files are written into the first (or only) source directory.\n"
		"         <generateQtSrc> is 1 when Qt source should be generated, 0 otherwise.\n"
		"         <prefix> is the file prefix <prefix>_KeywordList.cpp.\n"
		"         <ncg-dir> is the path to the directory where ncg_xxx.cpp files are written to.\n"
		"         Example: NandradCodeGenerator NANDRAD ~/git/SIM-VICUS/externals/Nandrad/src 0 NANDRAD ncg";


// ******* MAIN ********

int main(int argc, char *argv[]) {
	std::cout << "-------------------------------------------------------------------------------" << std::endl;
	std::cout << "NandradCodeGenerator v1.1 - SIM-VICUS project (based on IBK-KeywordListCreator)" << std::endl;
	std::cout << "-------------------------------------------------------------------------------" << std::endl;

	if (argc != 6) {
		std::cerr << "Invalid syntax." << std::endl;
		std::cerr << argc-1 << " Arguments received" << std::endl;
		for (int i=1; i<argc; ++i)
			std::cerr << "  " << argv[i] << std::endl;
		std::cerr << std::endl;
		std::cerr << SYNTAX << std::endl;

		return EXIT_FAILURE;
	}

	IBK::MessageHandlerRegistry::instance().messageHandler()->setConsoleVerbosityLevel(IBK::VL_STANDARD);

	CodeGenerator cg;
	cg.handleArguments(argv);

	// the next function parses the input files and does all the error handling
	if (!cg.parseDirectories())
		return EXIT_FAILURE;  // error messages where already written to file

	std::cout << "--------------------------------------------------------------------------" << std::endl;
	try {
		cg.generateKeywordList();
	} catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		std::cerr << "Error generating keywordlist" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "--------------------------------------------------------------------------" << std::endl;
	try {
		cg.generateReadWriteCode();
	} catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		std::cerr << "Error generating read/write code" << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "--------------------------------------------------------------------------" << std::endl;

	return EXIT_SUCCESS;
}

