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

private slots:
	void on_filepathPostProc_editingFinished();
	void on_filepathPostProc_returnPressed();

	void on_pushButtonAutoDetectPP2_clicked();

	void on_filepathTextEditor_editingFinished();
	void on_filepathTextEditor_returnPressed();

private:
	Ui::SVPreferencesPageTools *m_ui;
};


#endif // SVPreferencesPageToolsH
