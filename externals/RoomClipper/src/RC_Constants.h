/*	The RoomClipper data model library.

    Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

    Primary authors:
      Stephan Hirth     <stephan.hirth -[at]- tu-dresden.de>
      Dirk Weiß         <dirk.weis     -[at]- tu-dresden.de>

    This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#ifndef RC_ConstantsH
#define RC_ConstantsH

namespace RC {

/*! Version number of the data model and project file. */
extern const char * const VERSION;
/*! Long version number of the data model and project file. */
extern const char * const LONG_VERSION;
/*! Release Date. */
extern const char * const RELEASE_DATE;

/*! Scale factor for FLOAT / INT point conversion. */
extern const unsigned int SCALE_FACTOR;
/*! Minimum area definition for export */
extern const double MIN_AREA;
/*! Epsilon for distance clipping. */
extern const double EPSILON;

// When defined writes detailled output for debugging
//#define DETAILED_INFO

} // namespace RC
#endif // RC_ConstantsH
