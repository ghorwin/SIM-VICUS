#ifndef SVAboutDialogH
#define SVAboutDialogH

#include <QDialog>

namespace Ui {
	class SVAboutDialog;
}

/*! The about dialog. */
class SVAboutDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVAboutDialog(QWidget *parent = nullptr);
	~SVAboutDialog();

private:
	Ui::SVAboutDialog *m_ui;
};

#endif // AboutDialogH
