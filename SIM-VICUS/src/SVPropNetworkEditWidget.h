#ifndef SVPropNetworkEditWidgetH
#define SVPropNetworkEditWidgetH

#include <QWidget>
#include <QMap>

namespace VICUS {
	class Network;
	class Object;
}

namespace Ui {
	class SVPropNetworkEditWidget;
}

/*! A property widget for editing network properties. */
class SVPropNetworkEditWidget : public QWidget {
	Q_OBJECT
public:
	explicit SVPropNetworkEditWidget(QWidget *parent = nullptr);
	~SVPropNetworkEditWidget();

	/*! Called when widget is just shown, updates content to current project's data
		and selected node. */
	void updateUi();

	void updateSizingParams();

	void modifyStatus();

	void modifySizingParams();

	void modifyNodeProperties();

	void modifyEdgeProperties();

	void showNetworkProperties();

	void showNodeProperties();

	void showEdgeProperties();

	void setupComboboxPipeDB();

	void setupComboboxComponents();

private slots:
	void on_comboBoxNodeType_activated(int index);

	void on_doubleSpinBoxNodeNeatingDemand_editingFinished();

	void on_lineEditNodeX_editingFinished();

	void on_comboBoxPipeModel_activated(int index);

	void on_comboBoxPipeDB_activated(int index);

	void on_checkBoxSupplyPipe_clicked();

	void on_pushButtonSizePipeDimensions_clicked();

	void on_pushButtonGenerateIntersections_clicked();

	void on_pushButtonConnectBuildings_clicked();

	void on_pushButtonReduceDeadEnds_clicked();

	void on_pushButtonReduceRedundantNodes_clicked();

	void on_lineEditNodeY_editingFinished();

	void on_horizontalSliderScaleNodes_actionTriggered(int action);

	void on_horizontalSliderScaleEdges_actionTriggered(int action);

	void on_comboBoxComponent_activated(const QString &arg1);

private:

	void networkFromId();

	Ui::SVPropNetworkEditWidget *m_ui;

	const VICUS::Network *m_network = nullptr;

	const VICUS::Object * m_obj = nullptr;

	QMap<QString, unsigned> m_mapPipeModels;

	QMap<QString, unsigned> m_mapComponents;

	QMap<QString, unsigned> m_mapNodeTypes;

	QMap<QString, unsigned> m_mapDBPipes;

	QMap<QString, unsigned> m_mapHeatExchangeType;

	unsigned int m_treeItemId = 0;

};

#endif // SVPropNetworkEditWidgetH
