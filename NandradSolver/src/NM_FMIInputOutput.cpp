#include <NM_FMIInputOutput.h>

namespace NANDRAD_MODEL {


void FMIInputOutput::setup(const NANDRAD::Project & prj) {

}

int FMIInputOutput::setTime(double t) {

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
