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

	In a way this is similar to the project's onModified() function, yet independent
	of the project itself.
*/
class SVViewState {
public:
	SVViewState();

	/*! The different operation modes the scene can be in. NUM_OM means "none" and indicates simple
		navigation.
	*/
	enum SceneOperationMode {
		/*! Place vertex mode. */
		OM_PlaceVertex,
		/*! The scene is in passive mode - user can navigate and click on object to change selection. */
		NUM_OM
	};

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
		VM_EditGeometry,
		NUM_VM
	};

	/*! Different operation modes within geometry mode - switching the mode will
		change the appearance of the property widget and functionality of the scene.
	*/
	enum GeometryEditMode {
		/*! A new polygon is beeing added.
			The movable local coordinate system is being shown and snaps to
			the selected snap position.
		*/
		M_AddPolygon,
		NUM_M
	};

	SceneOperationMode	m_sceneOperationMode		= NUM_OM;
};

#endif // SVViewStateH
