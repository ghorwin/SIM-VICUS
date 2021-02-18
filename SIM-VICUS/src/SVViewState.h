#ifndef SVViewStateH
#define SVViewStateH

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

	/*! Different basic view modes.
		Depending on the view mode, different actions are available and
		the rendering works differently.
	*/
	enum ViewMode {
		/*! Standard mode - allows scene navigation and selection of
			elements. Allows adding new geometry.
			Property widgets related to geometry creation/modification are shown.
		*/
		VM_GeometryEditMode,
		/*! Property edit mode - items in navigation tree view can be made active
			and property widgets show physical properties/attributes.
		*/
		VM_PropertyEditMode,
		NUM_VM
	};


	/*! The different operation modes the scene can be in. NUM_OM means "none" and indicates simple
		navigation.
	*/
	enum SceneOperationMode {
		/*! The scene is in passive mode and has at least one selected surface/element.
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
		/*! The scene is in passive mode - user can navigate and click on object to change selection. */
		NUM_OM
	};


	/*! Defines which view should be active in the property widget. */
	enum PropertyWidgetMode {
		/*! Shows the "Add geometry" widget and tool page. */
		PM_AddGeometry,
		/*! Shows the "Edit geometry" widget and transform tool page. */
		PM_EditGeometry,
		/*! Shows the "Widget with list of newly placed vertexes" */
		PM_VertexList,
		/*! Shows the widget with global site (and view) properties. */
		PM_SiteProperties,
		/*! Shows the widget with building properties. */
		PM_BuildingProperties,
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
		/*! All surfaces that have a specific component assigned (m_propertyHighlightID) are colored based on whether they
			are mapped to side A or B of the component, with a given color. All other surfaces are drawn semi-transparent
			gray.
		*/
		OCM_ComponentOrientation,
		/*! All surfaces that have a component assigned which has a boundary condition ID are colored based on the
			boundary condition color.
		*/
		OCM_BoundaryConditions,
		OCM_Network			=	0x1000,
		OCM_NetworkNode,
		OCM_NetworkEdge,
		OCM_NetworkComponents
	};

	/*! Snapping/navigation locks, apply to movement of the
		local coordinate system when in "place vertex" mode.
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

	ViewMode				m_viewMode				= VM_GeometryEditMode;
	SceneOperationMode		m_sceneOperationMode	= NUM_OM;
	PropertyWidgetMode		m_propertyWidgetMode	= PM_EditGeometry;
	/*! Indicates which color mode shall be used to color opaque geometry. */
	ObjectColorMode			m_objectColorMode		= OCM_None;
	/*! Some color modes require an additional ID property. */
	int						m_colorModePropertyID	= 0;
	/*! Bitmask with selected snap options. */
	int						m_snapOptionMask		= Snap_GridPlane | Snap_ObjectVertex | Snap_ObjectCenter | Snap_ObjectEdgeCenter;
	/*! Whether snapping is enabled or not. */
	bool					m_snapEnabled			= true;
	/*! Coordinate system movement locks. */
	Locks					m_locks					= NUM_L;
};

#endif // SVViewStateH
