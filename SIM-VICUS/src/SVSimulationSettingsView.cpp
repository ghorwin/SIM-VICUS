#include "SVSimulationSettingsView.h"
#include "ui_SVSimulationSettingsView.h"

#include "SVSimulationLocationOptions.h"
#include "SVSimulationShadingOptions.h"
#include "SVSimulationOutputOptions.h"
#include "SVSimulationModelOptions.h"
#include "SVSimulationPerformanceOptions.h"
#include "SVSimulationNetworkOptions.h"
#include "SVSimulationStartOptions.h"
#include "SVLcaLccSettingsWidget.h"


SVSimulationSettingsView::SVSimulationSettingsView(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationSettingsView)
{
	m_ui->setupUi(this);
	layout()->setContentsMargins(10,10,10,10);

	// populate list widget
	// Note: Whitespace before label looks better, no simple workaround possible,
	// only if separate QLabels and QIcons are added, but that brings other problems...
	addCustomWidgetToListWidget(tr(" Climate & Location"), ":/gfx/actions/Location_Climate_32x32.svg");
	addCustomWidgetToListWidget(tr(" Shading"), ":/gfx/actions/shades.png");
	addCustomWidgetToListWidget(tr(" Thermal Simulation"), ":/gfx/actions/simulation-temperature.png");
	addCustomWidgetToListWidget(tr(" Life Cycle Assessment"), ":/gfx/actions/leaf.png");
	addCustomWidgetToListWidget(tr(" Acoustic"), ":/gfx/actions/sound-wave.png");

	// check if list widget and stacked widget have same count
	Q_ASSERT(m_ui->stackedWidget->count() == m_ui->listWidget->count());

	// disable LCA & Acoustic
	QListWidgetItem *item = m_ui->listWidget->item(P_LCA);
	item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
	item = m_ui->listWidget->item(P_Acoustic);
	item->setFlags(item->flags() & ~Qt::ItemIsEnabled);

	QFont fnt;
	fnt.setPixelSize(14);
	m_ui->listWidget->setFont(fnt);
	m_ui->listWidget->setIconSize(QSize(32,32));
	m_ui->listWidget->setSpacing(5);

	m_ui->tabWidgetSimulation->setStyleSheet("QTabWidget QTabBar::tab {width: 200px;}");

	m_ui->listWidget->setCurrentRow(P_Location);
}


SVSimulationSettingsView::~SVSimulationSettingsView() {
	delete m_ui;
}


void SVSimulationSettingsView::setCurrentPage(unsigned int index) {
	Q_ASSERT((int)index < m_ui->listWidget->count());

	m_ui->listWidget->setCurrentRow(index);
}


void SVSimulationSettingsView::on_listWidget_currentRowChanged(int currentRow) {

	switch (Page(currentRow)) {

		case P_Location: {
			if (m_locationOptions==nullptr) {
				m_locationOptions = new SVSimulationLocationOptions(this);
				QHBoxLayout *lay1 = new QHBoxLayout;
				lay1->addWidget(m_locationOptions);
				lay1->setContentsMargins(10,0,10,0);
				m_ui->stackedWidget->widget(P_Location)->setLayout(lay1);
			}
		} break;

		case P_Shading: {
			if (m_shadingOptions==nullptr) {
				m_shadingOptions = new SVSimulationShadingOptions(this);
				QHBoxLayout *lay2 = new QHBoxLayout;
				lay2->addWidget(m_shadingOptions);
				lay2->setContentsMargins(10,0,10,0);
				m_ui->stackedWidget->widget(P_Shading)->setLayout(lay2);
			}
		} break;

		case P_ThermalSimulation: {
			if (m_simulationStartOptions == nullptr) {
				m_simulationStartOptions = new SVSimulationStartOptions(this);
				m_simulationStartOptions->updateUi();
				QHBoxLayout *laySimStart = new QHBoxLayout;
				laySimStart->addWidget(m_simulationStartOptions);
				laySimStart->setContentsMargins(10,10,10,10);
				m_ui->tabSimulationStart->setLayout(laySimStart);

				m_simulationOutputOptions = new SVSimulationOutputOptions(this);
				m_simulationOutputOptions->updateUi();
				QHBoxLayout *layOutput = new QHBoxLayout;
				layOutput->addWidget(m_simulationOutputOptions);
				layOutput->setContentsMargins(10,10,10,10);
				m_ui->tabSimulationOutputs->setLayout(layOutput);

				QHBoxLayout *layPerf = new QHBoxLayout;
				SVSimulationPerformanceOptions *performanceOptions = new SVSimulationPerformanceOptions(this);
				performanceOptions->updateUi();
				layPerf->addWidget(performanceOptions);
				layPerf->setContentsMargins(10,10,10,10);
				m_ui->tabSimulationPerformanceOptions->setLayout(layPerf);

				QHBoxLayout *layModel = new QHBoxLayout;
				SVSimulationModelOptions *modelOptions = new SVSimulationModelOptions(this);
				modelOptions->updateUi();
				layModel->addWidget(modelOptions);
				layModel->setContentsMargins(10,10,10,10);
				m_ui->tabSimulationModelOptions->setLayout(layModel);

				QHBoxLayout *layNetwork = new QHBoxLayout;
				SVSimulationNetworkOptions *networkOptions = new SVSimulationNetworkOptions(this);
				networkOptions->updateUi();
				layNetwork->addWidget(networkOptions);
				layNetwork->setContentsMargins(10,10,10,10);
				m_ui->tabNetworkSettings->setLayout(layNetwork);

				m_ui->pageSimulation->layout()->setContentsMargins(10,0,10,0);

				// set pointer
				m_simulationStartOptions->m_simulationOutputOptions = m_simulationOutputOptions;
				m_simulationOutputOptions->m_simulationStartOptions = m_simulationStartOptions;
			}
		} break;

		case P_LCA: {
			if (m_lcaSettings == nullptr) {
				m_lcaSettings = new SVLcaLccSettingsWidget(this);
				QHBoxLayout *lay3 = new QHBoxLayout;
				lay3->addWidget(m_lcaSettings);
				lay3->setContentsMargins(10,0,10,0);
				m_ui->stackedWidget->widget(P_LCA)->setLayout(lay3);
			}
		} break;

		case P_Acoustic:
			break;

		default:
			Q_ASSERT(false);
	}

	m_ui->stackedWidget->setCurrentIndex(currentRow);
}


void SVSimulationSettingsView::addCustomWidgetToListWidget(const QString & text, const QString & pathToIcon){
	QListWidgetItem *item = new QListWidgetItem;
	item->setText(text);
	item->setIcon(QIcon(pathToIcon));
	m_ui->listWidget->addItem(item);
}

