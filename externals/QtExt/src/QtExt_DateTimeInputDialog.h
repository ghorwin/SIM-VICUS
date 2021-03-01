#ifndef QtExt_DateTimeInputDialogH
#define QtExt_DateTimeInputDialogH

#include <QDialog>

namespace Ui {
	class DateTimeInputDialog;
}

namespace QtExt {

/*! A dialog to request date or date/time inputs from user.

	TODO : configure to request only date, only time, a mix, add date/time limits, ....

*/
class DateTimeInputDialog : public QDialog {
	Q_OBJECT

public:
	explicit DateTimeInputDialog(QWidget *parent = nullptr);
	~DateTimeInputDialog();

	/*! Creates the dialog
		\code
		QDate dt = requestDate("Enter start date");
		QDate dt2 = requestDate("Enter start date", "MM/dd");
		\endcode

		\return Returns a valid date/time object if entered, otherwise invalid object when dialog was cancelled.
	*/
	static QDate requestDate(const QString & title, const QString & label, const QString & dateFormat = "dd.MM.", QDate * initialValue = nullptr);

private:
	::Ui::DateTimeInputDialog *m_ui;
};


} // namespace QtExt

#endif // QtExt_DateTimeInputDialogH
