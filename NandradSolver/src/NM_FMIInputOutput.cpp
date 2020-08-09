#include <NM_FMIInputOutput.h>

namespace NANDRAD_MODEL {


void FMIInputOutput::setup(const NANDRAD::Project & prj) {

}

int FMIInputOutput::setTime(double t) {
	// if interpolation of input variables is enabled, calculate
	// value in integration interval based on Taylor series expansion rule
	// and store in m_results
}


const double * FMIInputOutput::resultValueRef(const NANDRAD_MODEL::InputReference & valueRef) const {

	/// \todo implement

	return nullptr;
}


void FMIInputOutput::inputReferences(std::vector<InputReference> & inputRefs) const {

	/// \todo implement
}


void FMIInputOutput::initInputReferences(const std::vector<AbstractModel *> &) {

}


void FMIInputOutput::setInputValueRef(const InputReference & inputRef, const double * resultValueRef) {

}



} // namespace NANDRAD_MODEL
