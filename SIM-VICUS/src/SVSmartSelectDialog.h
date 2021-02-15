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

	void on_checkBoxMaxHeatingDemandBelow_clicked(bool checked);

	void on_checkBoxMaxHeatingDemandAbove_clicked(bool checked);

	void on_checkBoxLengthBelow_clicked(bool checked);

	void on_checkBoxLengthAbove_clicked(bool checked);

private:
	Ui::SVSmartSelectDialog *m_ui;
};

#endif // SVSmartSelectDialogH
