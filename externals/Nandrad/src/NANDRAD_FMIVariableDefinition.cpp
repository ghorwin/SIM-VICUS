#include "NANDRAD_FMIVariableDefinition.h"

#include <IBK_Exception.h>

namespace NANDRAD {

void FMIVariableDefinition::checkParameters() {
	FUNCID(FMIVariableDefinition::checkParameters);

	try {
		// check string 'causality'
		if(m_causality == "input") {
			m_inputVariable = true;
		}
		else if(m_causality == "output") {
			m_inputVariable = false;
		}
		else {
			throw IBK::Exception(IBK::FormatString("Invalid causality '%1'.")
				 .arg(m_causality),
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
