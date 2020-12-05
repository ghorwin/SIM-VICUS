#ifndef SVPreferencesPageStyleH
#define SVPreferencesPageStyleH

#include <QWidget>

namespace Ui {
	class SVPreferencesPageStyle;
}

/*! The configuration page with style settings. */
class SVPreferencesPageStyle : public QWidget {
	Q_OBJECT
	Q_DISABLE_COPY(SVPreferencesPageStyle)
public:
	/*! Default constructor. */
	explicit SVPreferencesPageStyle(QWidget *parent = nullptr);
	/*! Destructor. */
	~SVPreferencesPageStyle();

	/*! Updates the user interface with values in Settings object.*/
	void updateUi();

	/*! Transfers the current settings from the style page into
		the settings object.
		If one of the options was set wrong, the function will pop up a dialog
		asking the user to fix it.
		\return Returns true, if all settings were successfully stored. Otherwise
				 false which signals that the dialog must not be closed, yet.
	*/
	bool storeConfig();

	/*! */
	bool rejectConfig();
signals:
	/*! Emitted, when user has changed the style. */
	void styleChanged();

protected:

private slots:

	void on_comboBoxTheme_activated(const QString &theme);

	void on_pushButtonSceneBackgroundColor_colorChanged();
	void on_pushButtonMajorGridColor_colorChanged();
	void on_pushButtonMinorGridColor_colorChanged();
	void on_pushButtonSelectedSurfaceColor_colorChanged();

	void on_pushButtonDefault_clicked();

private:
	Ui::SVPreferencesPageStyle *m_ui;
};


#endif // SVPreferencesPageStyleH
