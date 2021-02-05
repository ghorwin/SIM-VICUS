#ifndef SVDIALOGSELECTNETWORKPIPES_H
#define SVDIALOGSELECTNETWORKPIPES_H

#include <QDialog>

namespace VICUS {
	class Network;
}

namespace Ui {
class SVDialogSelectNetworkPipes;
}

class SVDialogSelectNetworkPipes : public QDialog
{
	Q_OBJECT

public:
	explicit SVDialogSelectNetworkPipes(QWidget *parent = nullptr);
	~SVDialogSelectNetworkPipes();

	void edit(VICUS::Network &network);

private:
	Ui::SVDialogSelectNetworkPipes *m_ui;
};

#endif // SVDIALOGSELECTNETWORKPIPES_H
