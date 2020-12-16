#ifndef SVPropNetworkEditWidgetH
#define SVPropNetworkEditWidgetH

#include <QWidget>

namespace VICUS {
	class Network;

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

	void showNetworkProperties();

	void showNodeProperties();

	void showEdgeProperties();

private:
	Ui::SVPropNetworkEditWidget *m_ui;

	VICUS::Network *m_network;
};

#endif // SVPropNetworkEditWidgetH
