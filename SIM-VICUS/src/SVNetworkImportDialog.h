#ifndef SVNetworkImportDialogH
#define SVNetworkImportDialogH

#include <QDialog>
#include <QMap>

#include <VICUS_Network.h>

namespace Ui {
	class SVNetworkImportDialog;
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
	bool edit();

private slots:
	void on_pushButtonGISNetwork_clicked();

	void on_radioButtonNewNetwork_clicked(bool checked);

	void on_radioButtonAddToExistingNetwork_clicked(bool checked);

	void on_radioButtonEdges_clicked(bool checked);

	void on_radioButtonNodes_clicked(bool checked);

private:

	void toggleReadEdges(bool readEdges);

	void toggleReadExistingNetwork(bool readExisting);

	Ui::SVNetworkImportDialog *m_ui;

	VICUS::Network m_network;

	QMap<QString, unsigned> m_existingNetworksMap;

	void readNetworkData(const IBK::Path &fname, VICUS::Network &network) const;

	unsigned generateId();

	std::string uniqueName(const std::string &name);
};

#endif // SVNetworkImportDialogH
