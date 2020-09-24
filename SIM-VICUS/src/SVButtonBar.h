#ifndef SVButtonBarH
#define SVButtonBarH

#include <QWidget>

class QToolButton;
class QMenu;

/*!	The button bar is the the container of the master control buttons to
	the left of the main window.
*/
class SVButtonBar : public QWidget {
	Q_OBJECT
public:
	/*! Default constructor. */
	SVButtonBar(QWidget * parent);
	/*! Default destructor. */
	~SVButtonBar();

	// *** PUBLIC MEMBER VARIABLES ***
	QToolButton  * toolButtonAbout;
	QToolButton  * toolButtonNew;
	QToolButton  * toolButtonSave;
	QToolButton  * toolButtonLoad;

	QToolButton  * toolButtonUndo;
	QToolButton  * toolButtonRedo;

	QToolButton  * toolButtonViewPostProc;
	QToolButton  * toolButtonQuit;

	/*! Different views. */
	enum Views {
		ProjectView,
		GeometryView,
		SimulationView
	};

	/*! Returns index of currently selected view. */
	Views currentView() const { return m_currentView; }

	/*! Sets another view index.
		\note Calling this function is the correct way to switch views.
	*/
	void setCurrentView(Views view);

signals:
	/*! Emitted when user selects a different view.
		Argument is one of the views defined in Views.
	*/
	void currentViewChanged(int view);

private slots:

	void onToolButtonSwitchLanguageClicked();

private:
	Views	m_currentView;
};

#endif // ButtonBarH
