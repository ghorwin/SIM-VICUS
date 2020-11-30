/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef QtExt_ConstantsH
#define QtExt_ConstantsH

#include <QColor>
#include <Qt>

namespace QtExt {

/*! New variables defition */
extern unsigned int PANEL_BUTTON_SIZE;
extern unsigned int SOUTH_PANEL_HEIGHT;
extern unsigned int EAST_PANEL_WIDTH;
extern unsigned int WEST_PANEL_WIDTH;

/*! The font size for the tables. */
extern unsigned int TABLE_VIEW_FONT_SIZE;

extern QColor WINDOW_COLOR_DARKGRAY;
extern QColor WINDOW_COLOR_GRAY;

/*! Custom item roles supported by the various models. */
enum CustomItemRoles {
	// item data roles
	DataSourceIndexRole		= Qt::UserRole,
	MaterialIndexRole		= Qt::UserRole + 1,
	CategoryRole			= Qt::UserRole + 2,
	DensityRole				= Qt::UserRole + 3,
	AssignmentRole			= Qt::UserRole + 4,
	MaterialIsUsedRole		= Qt::UserRole + 5,
	MaterialIsValidRole		= Qt::UserRole + 6,
	MaterialErrorReasonRole = Qt::UserRole + 7,
	WidthRole				= Qt::UserRole + 8,
	ConstructionsIndexRole  = Qt::UserRole + 9,
	ColorEditRole			= Qt::UserRole + 10,

	// display related roles
	HighlightRole			= Qt::UserRole + 100,

	// climate data base roles
	ClimateDepthRole		= Qt::UserRole + 200,
	ClimateDataSourceRole	= Qt::UserRole + 201,

	//
	OrientationRole			= Qt::UserRole + 300,
	WTypeRole				= Qt::UserRole + 301,
	ToggleRole				= Qt::UserRole + 302,
	ComboIndexRole			= Qt::UserRole + 303,
	ActiveRole				= Qt::UserRole + 304,
	PointerRole				= Qt::UserRole + 305,

	// limit roles
	UpperLimit				= Qt::UserRole + 400,
	LowerLimit				= Qt::UserRole + 401

};


/*! This enum gives directions for different views */
enum Directions{
	Left		= 0x1,
	Right		= 0x2,
	Top			= 0x4,
	Bottom		= 0x8,
	ZweiDMask	= 0xF,
	XDir		= 0x80000000,
	YDir		= 0x40000000,
	DreiDMask	= 0xC000000F,
	XDirMask	= 0x7FFFFFFF,
	YDirMask	= 0xBFFFFFFF,
	LeftMask	= 0xFFFFFFFE,
	RightMask	= 0xFFFFFFFD,
	TopMask		= 0xFFFFFFFB,
	BottomMask	= 0xFFFFFFF7,
	MatInterfaceLeft	= 0x0100,
	MatInterfaceRight	= 0x0200,
	MatInterfaceTop		= 0x0400,
	MatInterfaceBottom	= 0x0800,
	MatInterfaceMask	= 0x0F00
};

/*! Possible hatching types. Used in class GraphicsRectItemWithHatch.*/
enum HatchingType {
	HT_NoHatch,
	HT_LinesHorizontal,
	HT_LinesVertical,
	HT_LinesObliqueLeftToRight,
	HT_LinesObliqueRightToLeft,
	HT_CrossHatchOblique,
	HT_CrossHatchStraight,
	HT_InsulationHatch
};


/*! User roles for material model*/
enum UserRoles {
	IdRole		=	Qt::UserRole + 1,	///< Identifier of user role.
	BuiltInRole	=	Qt::UserRole + 2,	///< Marker for database material.
	ParaRole =		Qt::UserRole + 3  ///< Role for parameter type (\sa MaterialBase::parameter_t).

};

extern QColor ColorHeat;
extern QColor ColorVapor;
extern QColor ColorWater;
extern QColor ColorAir;
extern QColor ColorVOC;
extern QColor ColorSalt;


/*! \file QtExt_Constants.h
	Contains constants which are used for GUI components like default sizes and
	geometry settings.
*/

} // namespace QtExt

#endif // QtExt_ConstantsH
