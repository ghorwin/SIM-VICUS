#ifndef SVSIMULATIONSETTINGSVIEWH
#define SVSIMULATIONSETTINGSVIEWH

#include <QWidget>

namespace Ui {
class SVSimulationSettingsView;
}


class SVSimulationStartOptions;
class SVSimulationOutputOptions;

class SVSimulationSettingsView : public QWidget
{
	Q_OBJECT

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

};

#endif // SVSIMULATIONSETTINGSVIEWH
