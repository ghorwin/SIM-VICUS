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

	void on_comboBoxNodeType_currentIndexChanged(int index);

	void on_checkBoxMaxHeatingDemandBelow_stateChanged(int arg1);

	void on_checkBoxMaxHeatingDemandAbove_stateChanged(int arg1);

	void on_checkBoxLengthBelow_stateChanged(int arg1);

	void on_checkBoxLengthAbove_stateChanged(int arg1);

private:
	Ui::SVSmartSelectDialog *m_ui;
};

#endif // SVSmartSelectDialogH
