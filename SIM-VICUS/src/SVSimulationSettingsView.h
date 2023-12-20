#ifndef SVSIMULATIONSETTINGSVIEWH
#define SVSIMULATIONSETTINGSVIEWH

#include "SVAcousticConstraintsCheckWidget.h"
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

	/*! Enum for simulation page. */
	enum SimulationPage {
		SP_Location,
		SP_Shading,
		SP_ThermalSimulation,
		SP_LifeCycle,
		SP_Acoustic,
		NUM_SP
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

	SVAcousticConstraintsCheckWidget	*m_acousticWidget = nullptr;

	SVSimulationOutputOptions			*m_simulationOutputOptions = nullptr;

	SVSimulationLocationOptions			*m_locationOptions = nullptr;

	SVSimulationShadingOptions			*m_shadingOptions = nullptr;

	SVLcaLccSettingsWidget				*m_lcaSettings = nullptr;
};

#endif // SVSIMULATIONSETTINGSVIEWH
