#ifndef SVUNDOMODIFYCLIMATEH
#define SVUNDOMODIFYCLIMATEH


#include "SVUndoCommandBase.h"

#include <NANDRAD_Location.h>


class SVUndoModifyClimate : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifySolverParams)
public:
	SVUndoModifyClimate(const QString & label, const NANDRAD::Location & location, bool climateFileModified);

	virtual void undo();
	virtual void redo();

private:
	/*! Location and climate settings for NANDRAD. */
	NANDRAD::Location									m_location;
	bool												m_climateFileModified = false;

};

#endif // SVUNDOMODIFYCLIMATEH
