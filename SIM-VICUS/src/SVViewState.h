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
	SVViewState();

	/*! Different basic view modes.
		Depending on the view mode, different actions are available and
		the rendering works differently.
	*/
	enum ViewMode {
		/*! Standard mode - allows scene navigation and selection of
			elements. Rendering is only done when viewport changes or
			when something is selected.
		*/
		VM_Standard,
		NUM_VM
	};


	/*! The different operation modes the scene can be in. NUM_OM means "none" and indicates simple
		navigation.
	*/
	enum SceneOperationMode {
		/*! Place vertex mode.
			In this mode, the local coordinate system is shown, and the user can click on any
			snap point to place a vertex.
		*/
		OM_PlaceVertex,
		/*! In this mode, the local coordinate system is shown, and the user can align the coordinate
			system by clicking on any surface.
			Typically, when this operation is complete, the view state switches back to the previous view state.
		*/
		OM_AlignLocalCoordinateSystem,
		/*! The scene is in passive mode - user can navigate and click on object to change selection. */
		NUM_OM
	};


	/*! Defines which view should be active in the property widget. */
	enum PropertyWidgetMode {
		/*! Shows the "Add geometry" widget and tool page. */
		PM_ADD_GEOMETRY,
		/*! Shows the "Add geometry" widget and transform tool page. */
		PM_EDIT_GEOMETRY,
		NUM_PM
	};

	SceneOperationMode	m_sceneOperationMode		= NUM_OM;
	PropertyWidgetMode	m_propertyWidgetMode		= PM_EDIT_GEOMETRY;
};

#endif // SVViewStateH
