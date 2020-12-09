#include "SVUndoAddFluid.h"


SVUndoAddFluid::SVUndoAddFluid(const QString & label, const VICUS::NetworkFluid & fluid):
m_fluid(fluid)
{
setText( label );
}

void SVUndoAddFluid::undo() {
// remove last fluid
Q_ASSERT(!theProject().m_networkFluids.empty());

theProject().m_networkFluids.pop_back();

// TODO Andreas:  a modified flag for all project DB elements?

// tell project that the fluid DB has changed
//SVProjectHandler::instance().setModified( SVProjectHandler::???);

}

void SVUndoAddFluid::redo() {
// append fluid
theProject().m_networkFluids.push_back(m_fluid);
// tell project that the fluid DB has changed
//SVProjectHandler::instance().setModified( SVProjectHandler::???);
}
