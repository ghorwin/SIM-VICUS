#ifndef SVNETWORKCONTROLLERDIALOG_H
#define SVNETWORKCONTROLLERDIALOG_H

#include <QDialog>

#include <VICUS_NetworkController.h>

namespace Ui {
class SVNetworkControllerDialog;
}

class SVNetworkControllerDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVNetworkControllerDialog(QWidget *parent = nullptr);
	~SVNetworkControllerDialog();

	unsigned int select(unsigned int networkId, unsigned int controllerId);

	void update();

private slots:
	void on_lineEditName_editingFinished();

	void on_lineEditSetpoint_editingFinished();

	void on_lineEditKp_editingFinished();

	void on_lineEditKi_editingFinished();

	void on_comboBoxProperty_currentIndexChanged(int index);

	void on_comboBoxControllerType_currentIndexChanged(int index);

private:
	Ui::SVNetworkControllerDialog	*m_ui;

	VICUS::NetworkController		m_controller;

};

#endif // SVNETWORKCONTROLLERDIALOG_H
