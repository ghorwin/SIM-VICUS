#ifndef SVPROPNETWORKNODESWIDGETH
#define SVPROPNETWORKNODESWIDGETH

#include <QWidget>

class SVPropNetworkEditWidget;

namespace Ui {
class SVPropNetworkNodesWidget;
}

class SVPropNetworkNodesWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVPropNetworkNodesWidget(QWidget *parent = nullptr);
	~SVPropNetworkNodesWidget();

	void updateUi();

	void clearUi();

	void setWidgetsEnabled(bool enabled);

private slots:
	void on_comboBoxNodeType_activated(int index);

	void on_lineEditNodeName_editingFinished();

	void on_lineEditNodeXPosition_editingFinishedSuccessfully();

	void on_lineEditNodeYPosition_editingFinishedSuccessfully();

	void on_lineEditNodeZPosition_editingFinishedSuccessfully();

	void on_lineEditNodeMaximumHeatingDemand_editingFinishedSuccessfully();

private:
	Ui::SVPropNetworkNodesWidget	*m_ui;

	SVPropNetworkEditWidget	*m_pa;
};

#endif // SVPROPNETWORKNODESWIDGETH
