#ifndef SVPreferencesPageToolsH
#define SVPreferencesPageToolsH

#include <QWidget>

namespace Ui {
	class SVPreferencesPageTools;
}

/*! The configuration page with external tool settings. */
class SVPreferencesPageTools : public QWidget {
	Q_OBJECT
	Q_DISABLE_COPY(SVPreferencesPageTools)
public:
	/*! Default constructor. */
	explicit SVPreferencesPageTools(QWidget *parent = nullptr);
	/*! Destructor. */
	~SVPreferencesPageTools();

	/*! Updates the user interface with values in Settings object.*/
	void updateUi();

	/*! Transfers the current settings from the configuration page into
		the settings object.
		If one of the options was set wrong, the function will pop up a dialog
		asking the user to fix it.
		\return Returns true, if all settings were successfully stored. Otherwise
				 false which signals that the dialog must not be closed, yet.
	*/
	bool storeConfig();

private slots:
	void on_filepathPostProc_editingFinished();
	void on_filepathPostProc_returnPressed();

	void on_pushButtonAutoDetectPP2_clicked();

private:
	Ui::SVPreferencesPageTools *m_ui;
};


#endif // SVPreferencesPageToolsH
