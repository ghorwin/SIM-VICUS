#ifndef VICUS_VIEWSETTINGS_H
#define VICUS_VIEWSETTINGS_H

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

namespace VICUS {

class ViewSettings {
public:

    // *** PUBLIC MEMBER FUNCTIONS ***

    VICUS_READWRITE

    // *** PUBLIC MEMBER VARIABLES ***



    /*! holds grid spacing in [m] */
    double                              m_gridSpacing;
    /*! holds width dimension of grid in [m] */
    double                              m_gridWidth;


};

} // namespace VICUS


#endif // VICUS_VIEWSETTINGS_H
