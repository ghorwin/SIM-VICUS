#ifndef SVPROPEDITCOPYDIALOG_H
#define SVPROPEDITCOPYDIALOG_H

#include <QDialog>

namespace Ui {
class SVPropEditCopyDialog;
}

class SVPropEditCopyDialog : public QDialog
{
	Q_OBJECT


public:
	explicit SVPropEditCopyDialog(QWidget *parent = nullptr);
	~SVPropEditCopyDialog();

	/*! Creates the dialog
		\code
		QString buttonTitle1 = "All Subsurfaces."
		QString buttonTitle2 = "Only selected Subsurfaces."
		\endcode

		\return Returns true if buttonAccepted is clicked ohterwise false.
	*/
	static int requestCopyMethod(const QString & title, const QString & buttonTitle1, const QString & buttonTitle2);

private slots:


	void on_pushButton1_clicked();

	void on_pushButton2_clicked();

private:
	Ui::SVPropEditCopyDialog *m_ui;

	/*! Cached Method */
	bool				m_copyMethod = true;
};

#endif // SVPROPEDITCOPYDIALOG_H
