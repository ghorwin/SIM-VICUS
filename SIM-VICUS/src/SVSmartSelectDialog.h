#ifndef SVSmartSelectDialogH
#define SVSmartSelectDialogH

#include <QDialog>

namespace Ui {
class SVSmartSelectDialog;
}

/*! The dialog for selecting objects based on properties. */
class SVSmartSelectDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVSmartSelectDialog(QWidget *parent = nullptr);
	~SVSmartSelectDialog();

private slots:
	void onSelectClicked();

private:
	Ui::SVSmartSelectDialog *m_ui;
};

#endif // SVSmartSelectDialogH
