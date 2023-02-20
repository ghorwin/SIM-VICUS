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

#ifndef SVViewStateH
#define SVViewStateH

#include <VICUS_Constants.h>

/*! This class defines the current state of the user interface.
	This includes:

	- the current display and input options for the 3D scene.
	- the content of the property widget
	- the actions allowed on the navigation pane

	When the user interface changes its state (for example, when user
	executes an action, presses a trigger key etc.)
	the state of this object is changed and all parts of the user-interface
	are signaled to adjust their state according to the content of this object.

	Lokal coordinate system (for vertex placement/translation) may have fixed axes
	(properties of the local coordinate system).

	In a way this is similar to the project's onModified() function, yet independent
	of the project itself.
*/
class SVViewState {
public:

	/*! Returns true if we are editing properties (for networks or buildings) and
	 *  we want to use different coloring of the objects. */
	bool inPropertyEditingMode() const {
		return m_propertyWidgetMode == PM_NetworkProperties || m_propertyWidgetMode == PM_BuildingProperties;
	}


	/*! The different operation modes the scene can be in. NUM_OM means "none" and indicates simple
		navigation.
	*/
	enum SceneOperationMode {
		/*! The scene is in passive geometry edit mode and has at least one selected surface/element.
			User can navigate and click on object to change selection.
			In this mode the local coordinate system is shown permanently at a fixed location and
			can be moved/rotated as needed.
			When the selection changes, the local coordinate system is translated to the center point
			of the selected geometry.
			When the last selection was de-selected, the scene goes back to passive move NUM_OM.
		*/
		OM_SelectedGeometry,
		/*! Place vertex mode.
			In this mode, the local coordinate system is shown, and the user can click on any
			snap point to place a vertex. Also, movement of the local coordinate system is
			communicated to the NewGeometryObject.
		*/
		OM_PlaceVertex,
		/*! In this mode, the local coordinate system is shown, and the user can align the coordinate
			system by clicking on any surface.
			Typically, when this operation is complete, the view state switches back to the previous view state.
		*/
		OM_AlignLocalCoordinateSystem,
		/*! In this mode, the local coordinate system is shown, and the user can move the coordinate
			system by clicking on any surface.
			Typically, when this operation is complete, the view state switches back to the previous view state.
		*/
		OM_MoveLocalCoordinateSystem,
		/*! In this mode, the local coordinate system is shown and the user can click anywhere - with, or without snap.
			When the user clicks, the starting point is adjusted. When the user moves the coordinate system,
			a line between starting point and current coordinate system's location is drawn and the distance is displayed.
		*/
		OM_MeasureDistance,
		/*! In this mode, surfaces can be selected via an interactive rubberband inside the scene.
		*/
		OM_RubberbandSelection,
		/*! In this mode, the user can place three points after another (by snapping the local coordinate system
			to the points). When the third point was placed, all selected objects are rotated accordingly.
		*/
		OM_ThreePointRotation,
		/*! The scene is in passive mode - user can navigate and click on object to change selection. */
		NUM_OM
	};


	/*! Defines which view should be active in the property widget.
		This also determines the current operation, we are in, i.e. PM_VertexList = we are constructing geometry,
		PM_AddSubSurfaceGeometry = we add windows, the rest is just selection stuff.
	*/
	enum PropertyWidgetMode {
		/*! Shows the "Add geometry" widget and tool page. */
		PM_AddGeometry,
		/*! Shows the "Edit geometry" widget and tool page (requires selection). */
		PM_EditGeometry,
		/*! Shows the "Widget with list of newly placed vertexes" */
		PM_VertexList,
		/*! Shows the widget with global site (and view) properties. */
		PM_SiteProperties,
		/*! Shows the widget for adding sub-surface geometries. */
		PM_AddSubSurfaceGeometry,
		/*! Shows the widget with building properties. */
		PM_BuildingProperties,
		/*! Shows the widget with building acoustic properties. */
		PM_BuildingAcousticProperties,
		/*! Shows the widget with network properties. */
		PM_NetworkProperties
	};

	/*! These enum values indicate what kind of coloring/highlighting shall be applied
		when drawing opaque building/network geometry.
		The coloring will only be applied when in property-edit mode, otherwise the default
		surface color will be used.

		Note: when a "building property" color mode is selected, the network geometry (nodes/edges) will
			  be shown using standard colors. Similarly, when a network coloring mode is selected, the building
			  geometry is shown in standard colors. To distinguish easily between building/network color modes,
			  the enumerations for node properties start with 0x1000.
	*/
	enum ObjectColorMode {
		/*! Use default colors (whatever that means). This is used when property edit mode "Site" is active,
			or in geometry edit mode.
		*/
		OCM_None					=	0x0000,
		/*! In this mode the surfaces are colored based on the color assigned to their associated component, or a default
			gray value, if they are not yet associated with a component.
		*/
		OCM_Components,
		/*! In this mode the sub-surfaces are colored based on the color assigned to their associated sub-surface component,
			or a default gray value, if they are not yet associated with a component.
		*/
		OCM_SubSurfaceComponents,
		/*! All surfaces that have a specific component assigned (m_propertyHighlightID) are colored based on whether they
			are mapped to side A or B of the component, with a given color. All other surfaces are drawn semi-transparent
			gray.
		*/
		OCM_ComponentOrientation,
		/*! All surfaces that have a component assigned which has a boundary condition ID are colored based on the
			boundary condition color.
		*/
		OCM_BoundaryConditions,
		/*! All surfaces of rooms with associated zone template are colored based on that zone template color.
		*/
		OCM_ZoneTemplates,
		/*! All surfaces of rooms with associated zone template are colored based on that zone template color.
		*/
		OCM_SurfaceHeating,
		/*! All surfaces of rooms with associated supply system are colored based on that supply system color.
		*/
		OCM_SupplySystems,
		/*! All surfaces are drawn in transparent light gray, linked surfaces are drawn in transparent gray and are
			connected by red boxes.
		*/
		OCM_InterlinkedSurfaces,
		/*! When this mode is active, all but the selected surfaces are shown in dark gray, but the selected surfaces
			are shown in orange.
		*/
		OCM_SelectedSurfacesHighlighted,
		OCM_AcousticRoomType,
		OCM_Network			=	0x1000,
		OCM_NetworkNode,
		OCM_NetworkEdge,
		OCM_NetworkHeatExchange,
		OCM_NetworkSubNetworks,
	};

	/*! Snapping/navigation locks, apply to movement of the
		local coordinate system.
	*/
	enum Locks {
		/*! Only movement along local X axis is allowed. */
		L_LocalX,
		/*! Only movement along local Y axis is allowed. */
		L_LocalY,
		/*! Only movement along local Z axis is allowed. */
		L_LocalZ,
		/*! No axis lock. */
		NUM_L
	};

	/*! The different snap options.
		\warning Do not change the order/enum values. For any bitmask value larger
		than Snap_GridPlane a surface will be required.
	*/
	enum SnapOptions {
		Snap_GridPlane			= 0x0001,
		Snap_ObjectCenter		= 0x0002,
		Snap_ObjectVertex		= 0x0004,
		Snap_ObjectEdgeCenter	= 0x0008
	};

	bool operator!=(const SVViewState &other) const;

	SceneOperationMode		m_sceneOperationMode	= NUM_OM;
	PropertyWidgetMode		m_propertyWidgetMode	= PM_AddGeometry;
	/*! Indicates which color mode shall be used to color opaque geometry. */
	ObjectColorMode			m_objectColorMode		= OCM_None;
	/*! Some color modes require an additional ID property. */
	unsigned int			m_colorModePropertyID	= VICUS::INVALID_ID;
	/*! Bitmask with selected snap options. */
	int						m_snapOptionMask		= Snap_GridPlane | Snap_ObjectVertex | Snap_ObjectCenter | Snap_ObjectEdgeCenter;
	/*! Whether snapping is enabled or not. */
	bool					m_snapEnabled			= true;
	/*! Coordinate system movement locks. */
	Locks					m_locks					= NUM_L;
};

#endif // SVViewStateH
