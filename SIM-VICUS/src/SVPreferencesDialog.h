#ifndef SVPreferencesDialogH
#define SVPreferencesDialogH

#include <QDialog>

class SVPreferencesPageTools;
class SVPreferencesPageStyle;

namespace Ui {
	class SVPreferencesDialog;
}

/*! Implementation of the preferences dialog. */
class SVPreferencesDialog : public QWidget {
	Q_OBJECT
public:
	/*! Constructor.*/
	SVPreferencesDialog(QWidget * parent);
	/*! Destructor. */
	~SVPreferencesDialog();

	/*! Spawns the dialog and returns when user has closed the dialog.
		\param initialPage The index of the page to be shown initially.
	*/
	void edit(int initialPage = 0);

	/*! Provides read-only access to pageStyle() so that signals can be connected. */
	const SVPreferencesPageStyle * pageStyle() const { return m_pageStyle; }

private:
	/*! Transfers values from Settings object to user interface (config pages).*/
	void updateUi();

	/*! GUI member. */
	Ui::SVPreferencesDialog			*m_ui;

	/*! The Tools page. */
	SVPreferencesPageTools			*m_pageTools;
	/*! The Style page. */
	SVPreferencesPageStyle			*m_pageStyle;
};

#endif // SVPreferencesDialogH
