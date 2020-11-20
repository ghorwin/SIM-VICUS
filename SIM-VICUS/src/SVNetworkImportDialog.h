#ifndef SVNETWORKIMPORTDIALOG_H
#define SVNETWORKIMPORTDIALOG_H

#include <QDialog>

namespace Ui {
	class SVNetworkImportDialog;
}

namespace VICUS {
	class Network;
}

/*! A dialog for importing pipe network data. */
class SVNetworkImportDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVNetworkImportDialog(QWidget *parent = nullptr);
	~SVNetworkImportDialog();

	/*! Always start the dialog with this function.
		\return Returns true if dialog was confirmed and data can be added to project.
	*/
	bool edit(VICUS::Network & n);

private slots:
	void on_pushButtonGISNetwork_clicked();

private:
	Ui::SVNetworkImportDialog *m_ui;

	VICUS::Network *	m_network;
};

#endif // SVNETWORKIMPORTDIALOG_H
