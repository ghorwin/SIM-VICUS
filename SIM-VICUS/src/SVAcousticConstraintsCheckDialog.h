#ifndef SVACOUSTICCONSTRAINTSCHECKDIALOG_H
#define SVACOUSTICCONSTRAINTSCHECKDIALOG_H

#include <QDialog>

namespace Ui {
class SVAcousticConstraintsCheckDialog;
}

class SVAcousticConstraintsCheckDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SVAcousticConstraintsCheckDialog(QWidget *parent = nullptr);
    ~SVAcousticConstraintsCheckDialog();

	/*! Always start the dialog with this function.
		\return Returns true if dialog was confirmed and data can be added to project.
	*/
	bool edit();

private:
	Ui::SVAcousticConstraintsCheckDialog *m_ui;
};

#endif // SVACOUSTICCONSTRAINTSCHECKDIALOG_H
