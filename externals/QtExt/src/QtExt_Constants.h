/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
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
