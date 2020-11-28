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


};

#endif // SVViewStateH
