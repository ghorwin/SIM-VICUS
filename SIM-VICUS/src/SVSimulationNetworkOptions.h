#ifndef SVSIMULATIONETWORKOPTIONS_H
#define SVSIMULATIONETWORKOPTIONS_H

#include <QWidget>

#include <VICUS_Network.h>

namespace Ui {
class SVSimulationNetworkOptions;
}

class SVSimulationNetworkOptions : public QWidget
{
	Q_OBJECT

public:
	explicit SVSimulationNetworkOptions(QWidget *parent, std::vector<VICUS::Network> &networks);
	~SVSimulationNetworkOptions();

	void updateUi();

	void modify();

private slots:
	void on_comboBoxNetwork_activated(int index);

	void on_comboBoxModelType_activated(int index);

	void on_lineEditDefaultFluidTemperature_editingFinished();

	void on_lineEditReferencePressure_editingFinished();

	void on_lineEditMaxPipeDiscretization_editingFinished();

private:
	Ui::SVSimulationNetworkOptions *m_ui;

	std::vector<VICUS::Network>		*m_networks;

	unsigned int					m_currentId = VICUS::INVALID_ID;
};

#endif // SVSIMULATIONETWORKOPTIONS_H
