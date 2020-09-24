#ifndef SVPreferencesDialogH
#define SVPreferencesDialogH

#include <QDialog>

class SVPreferencesPageTools;

namespace Ui {
	class SVPreferencesDialog;
}

/*! Implementation of the preferences dialog. */
class SVPreferencesDialog : public QDialog {
	Q_OBJECT
public:
	/*! Constructor.*/
	SVPreferencesDialog(QWidget * parent);
	/*! Destructor. */
	~SVPreferencesDialog();

	/*! Spawns the dialog and returns when user has closed the dialog.
		\return Returns true if user has changed preferences (some of which may require an update of views).
	*/
	bool edit(int initialPage = 0);

protected:
	/*! QDialog::accept() re-implemented for input data checking (called indirectly from buttonBox). */
	void accept();

private:
	/*! Transfers values from Settings object to user interface (config pages).*/
	void updateUi();

	/*! Transfers the current settings from all configuration pages into
		the settings object.
		If one of the options was set wrong, the function will pop up a dialog
		asking the user to fix it.
		\return Returns true, if all settings were successfully stored. Otherwise
				 false which signals that the dialog must not be closed, yet.
	*/
	bool storeConfig();

	/*! GUI member. */
	Ui::SVPreferencesDialog		*m_ui;

	/*! The Tools page. */
	SVPreferencesPageTools			*m_pageTools;
};

#endif // SVPreferencesDialogH
