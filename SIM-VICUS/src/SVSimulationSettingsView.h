#ifndef SVSIMULATIONSETTINGSVIEWH
#define SVSIMULATIONSETTINGSVIEWH

#include <QWidget>

namespace Ui {
class SVSimulationSettingsView;
}


class SVSimulationStartOptions;
class SVSimulationOutputOptions;
class SVSimulationLocationOptions;
class SVSimulationShadingOptions;
class SVLcaLccSettingsWidget;

class SVSimulationSettingsView : public QWidget
{
	Q_OBJECT

	enum Page {
		P_Location,
		P_Shading,
		P_ThermalSimulation,
		P_LCA,
		P_Acoustic,
		NUM_P
	};

public:
	explicit SVSimulationSettingsView(QWidget *parent = nullptr);
	~SVSimulationSettingsView();

	/*! Function for external access */
	void setCurrentPage(unsigned int index);

private slots:
	void on_listWidget_currentRowChanged(int currentRow);

private:
	void addCustomWidgetToListWidget(const QString &text, const QString & pathToIcon);

	Ui::SVSimulationSettingsView		*m_ui;

	SVSimulationStartOptions			*m_simulationStartOptions = nullptr;

	SVSimulationOutputOptions			*m_simulationOutputOptions = nullptr;

	SVSimulationLocationOptions			*m_locationOptions = nullptr;

	SVSimulationShadingOptions			*m_shadingOptions = nullptr;

	SVLcaLccSettingsWidget				*m_lcaSettings = nullptr;
};

#endif // SVSIMULATIONSETTINGSVIEWH
