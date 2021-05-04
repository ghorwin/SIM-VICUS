#include "NANDRAD_FMIVariableDefinition.h"

#include "NANDRAD_KeywordList.h"
#include "NANDRAD_ModelInputReference.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

namespace NANDRAD {

void FMIVariableDefinition::checkParameters() {
	FUNCID(FMIVariableDefinition::checkParameters);

	try {
		// check name
		std::vector<std::string> tokens;
		size_t size = IBK::explode_in2(m_varName, tokens, '.');
		// invalid name
		if(size != 2) {
			throw IBK::Exception(IBK::FormatString("Malformed variable name '%1'. "
					"Expected '<referenceType>.<name>!")
				 .arg(m_varName),
				 FUNC_ID);
		}
		// check reference type
		if(!KeywordList::KeywordExists("ModelInputReference::referenceType_t", tokens[0])){
			throw IBK::Exception(IBK::FormatString("Malformed variable name '%1'. "
					"Reference type '%2' is undefined!")
				 .arg(m_varName).arg(tokens[0]),
				 FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for FMIVariableDefinition %1.")
				 .arg(m_fmiVarName),
				 FUNC_ID);
	}
}

}
