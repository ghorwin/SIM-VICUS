/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVConstantsH
#define SVConstantsH

#include <QAbstractItemDelegate>

/*! To be used as institution/organization in SVSettings. */
extern const char * const ORG_NAME;
/*! E-mail address for support requests. */
extern const char * const SUPPORT_EMAIL;

/*! URL for update info file. */
extern const char * const UPDATE_FILE_URL;
/*! URL for news file. */
extern const char * const NEWS_FILE_URL;
/*! URL for bug reports. */
extern const char * const BUG_REPORT_URL;
/*! URL for online manual. */
extern const char * const MANUAL_URL;
/*! URL for discussion forum. */
extern const char * const FORUM_URL;
/*! Third language for QtExt_lineEdits. */
extern const char * const THIRD_LANGUAGE;


/*! Fixed-size font family to be used in tables. */
extern const char * const FIXED_FONT_FAMILY;

/*! Custom data roles provided by the models.
	Note: not all models provide all roles. But when they do, they should use the
		  constants below
*/
enum DataRoles {
	Role_Id				= Qt::UserRole,
	/*! Role to be used to identify built-in database components. */
	Role_BuiltIn,
	/*! Role to be used when a file name (without path) is to be returned.*/
	Role_FileName,
	/*! Role to be used when a file path (with placeholder) is to be returned.*/
	Role_FilePath,
	/*! Role to be used when a file path is to be returned as absolute file path.*/
	Role_AbsoluteFilePath,
	/*! Role to use for double values needed for comparisions. */
	Role_Value,
	/*! Role to use for passing raw data pointers to whatever data structure
		the item corresponds to. */
	Role_RawPointer,
	/*! Role to be used to identify color column. */
	Role_Color,
	/*! Role to retrieve sub-template type, only used by SVDBZoneTemplateTreeModel all other
		models return QVariant(). */
	Role_SubTemplateType,
	/*! Role to identify local database components, which are not stored in user database. */
	Role_Local,
	/*! Role to identify referenced database components, which are currently used in the project. */
	Role_Referenced
};

/*! Different edit/coloring modes. */
enum BuildingPropertyType {
	/*! Assigned component. */
	BT_Components,
	/*! Assigned sub-surface components. */
	BT_SubSurfaceComponents,
	/*! Orientation of component (side A/B colors). */
	BT_ComponentOrientation,
	/*! Boundary conditions of component. */
	BT_BoundaryConditions,
	/*! Surface interlinking operations. */
	BT_SurfaceConnection,
	/*! Zone templates. */
	BT_ZoneTemplates,
	/*! Surface heating */
	BT_SurfaceHeating,
	/*! Supply Systems */
	BT_SupplySystems,
	/*! Zone property */
	BT_ZoneProperty,
	/*! Floor manager. */
	BT_FloorManager
};

/*! Different edit/coloring modes. */
enum NetworkPropertyTypes {
	/*! Assigned node properties. */
	NT_Geometry,
	/*! Assigned node properties. */
	NT_Node,
	/*! Assigned edge properties. */
	NT_Edge,
	/*! Assigned sub station properties. */
	NT_SubStation,
	/*! Assigned heat exchange properties. */
	NT_HeatExchange,
};

/*! \file SVConstants.h
	Contains constants which are used in several ui files. This is to
	avoid multiple specifications of constants or even worse, the use of
	C defines.
*/

#endif // SVConstantsH
