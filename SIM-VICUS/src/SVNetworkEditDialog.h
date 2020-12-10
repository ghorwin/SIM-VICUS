#ifndef SVNetworkEditDialogH
#define SVNetworkEditDialogH

#include <QDialog>
#include <QMap>

#include "VICUS_Network.h"

namespace Ui {
class SVNetworkEditDialog;
}

class SVNetworkEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVNetworkEditDialog(QWidget *parent = nullptr);
	~SVNetworkEditDialog();

	void edit();

private slots:
	void on_pushButtonGenerateIntersections_clicked();

	void on_comboBoxSelectNetwork_activated(const QString &arg1);

	void on_pushButtonConnectBuildings_clicked();

	void on_pushButtonReduceDeadEnds_clicked();

	void on_pushButtonSizePipeDimensions_clicked();

	void on_pushButtonCopy_clicked();

	void on_pushButtonDelete_clicked();

	void on_pushButtonReduceRedundantNodes_clicked();

	void on_pushButtonSimplify_clicked();

	void on_checkBoxVisible_clicked();

private:
	Ui::SVNetworkEditDialog *m_ui;

	void updateStatus() const;

	void updateSizingParams();

	void modifyStatus();

	void setNetwork();

	void setupComboBox();

	void copyNetwork(const std::string & appendName);

	QMap<QString, unsigned> m_existingNetworksMap;

	VICUS::Network m_network;
};

#endif // SVNetworkEditDialogH
