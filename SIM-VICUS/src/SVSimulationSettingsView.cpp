#include "SVSimulationSettingsView.h"
#include "ui_SVSimulationSettingsView.h"

#include "SVSimulationLocationOptions.h"
#include "SVSimulationShadingOptions.h"
#include "SVSimulationOutputOptions.h"
#include "SVSimulationModelOptions.h"
#include "SVSimulationPerformanceOptions.h"
#include "SVSimulationNetworkOptions.h"
#include "SVSimulationStartOptions.h"
#include "SVSettings.h"
#include "SVProjectHandler.h"
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

	// disable LCA
	QListWidgetItem *item = m_ui->listWidget->item(3);
	item->setFlags(item->flags() & ~Qt::ItemIsEnabled);

	QFont fnt;
	fnt.setPixelSize(14);
	m_ui->listWidget->setFont(fnt);
	m_ui->listWidget->setIconSize(QSize(32,32));
	m_ui->listWidget->setSpacing(5);



	// *** add widgets to stacked widget

	SVSimulationLocationOptions *locationOptions = new SVSimulationLocationOptions(this);
	QHBoxLayout *lay1 = new QHBoxLayout;
	lay1->addWidget(locationOptions);
	lay1->setContentsMargins(10,0,10,0);
	m_ui->stackedWidget->widget(0)->setLayout(lay1);

	SVSimulationShadingOptions *shadingOptions = new SVSimulationShadingOptions(this);
	QHBoxLayout *lay2 = new QHBoxLayout;
	lay2->addWidget(shadingOptions);
	lay2->setContentsMargins(10,0,10,0);
	m_ui->stackedWidget->widget(1)->setLayout(lay2);

	SVLcaLccSettingsWidget *lcaSettings = new SVLcaLccSettingsWidget(this);
	QHBoxLayout *lay3 = new QHBoxLayout;
	lay3->addWidget(lcaSettings);
	lay3->setContentsMargins(10,0,10,0);
	m_ui->stackedWidget->widget(3)->setLayout(lay3);



	// sim start widget

	m_ui->pageSimulation->layout()->setContentsMargins(10,0,10,0);

	QHBoxLayout *laySimStart = new QHBoxLayout;
	m_simulationStartOptions = new SVSimulationStartOptions(this);
	laySimStart->addWidget(m_simulationStartOptions);
	laySimStart->setContentsMargins(10,10,10,10);
	m_ui->tabSimulationStart->setLayout(laySimStart);

	QHBoxLayout *layOutput = new QHBoxLayout;
	m_simulationOutputOptions = new SVSimulationOutputOptions(this);
	layOutput->addWidget(m_simulationOutputOptions);
	layOutput->setContentsMargins(10,10,10,10);
	m_ui->tabSimulationOutputs->setLayout(layOutput);

	QHBoxLayout *layPerf = new QHBoxLayout;
	SVSimulationPerformanceOptions *performanceOptions = new SVSimulationPerformanceOptions(this);
	layPerf->addWidget(performanceOptions);
	layPerf->setContentsMargins(10,10,10,10);
	m_ui->tabSimulationPerformanceOptions->setLayout(layPerf);

	QHBoxLayout *layModel = new QHBoxLayout;
	SVSimulationModelOptions *modelOptions = new SVSimulationModelOptions(this);
	layModel->addWidget(modelOptions);
	layModel->setContentsMargins(10,10,10,10);
	m_ui->tabSimulationModelOptions->setLayout(layModel);

	QHBoxLayout *layNetwork = new QHBoxLayout;
	SVSimulationNetworkOptions *networkOptions = new SVSimulationNetworkOptions(this);
	layNetwork->addWidget(networkOptions);
	layNetwork->setContentsMargins(10,10,10,10);
	m_ui->tabNetworkSettings->setLayout(layNetwork);

	m_ui->tabWidgetSimulation->setStyleSheet("QTabWidget QTabBar::tab {width: 200px;}");

	// set pointer
	m_simulationStartOptions->m_simulationOutputOptions = m_simulationOutputOptions;
	m_simulationOutputOptions->m_simulationStartOptions = m_simulationStartOptions;

	// should be equal counts in list widget and stacked widget!
	Q_ASSERT(m_ui->listWidget->count() == m_ui->stackedWidget->count());

	m_ui->listWidget->setCurrentRow(0);
}


SVSimulationSettingsView::~SVSimulationSettingsView() {
	delete m_ui;
}


void SVSimulationSettingsView::setCurrentPage(unsigned int index) {
	Q_ASSERT((int)index < m_ui->listWidget->count());
	m_ui->listWidget->setCurrentRow((int)index);
}


void SVSimulationSettingsView::on_listWidget_currentRowChanged(int currentRow) {
	m_ui->stackedWidget->setCurrentIndex(currentRow);
}


void SVSimulationSettingsView::addCustomWidgetToListWidget(const QString & text, const QString & pathToIcon){
	QListWidgetItem *item = new QListWidgetItem;
	item->setText(text);
	item->setIcon(QIcon(pathToIcon));
	m_ui->listWidget->addItem(item);
}

