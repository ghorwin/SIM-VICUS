#include "SVShadingCalculationDialog.h"
#include "ui_SVShadingCalculationDialog.h"

#include <QDate>
#include <QElapsedTimer>
#include <QTimer>

#include <QtExt_DateTimeInputDialog.h>

#include <vector>
#include <fstream>

#include <IBK_physics.h>

#include <VICUS_Surface.h>
#include <VICUS_Project.h>

#include <SH_StructuralShading.h>

#include <SVProjectHandler.h>
#include <SVSettings.h>
#include <SVMainWindow.h>



SVShadingCalculationDialog::SVShadingCalculationDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVShadingCalculationDialog)
{
	m_ui->setupUi(this);

	m_localProject = project();
	m_simulationParameter = &m_localProject.m_simulationParameter; // readability improvements

	m_ui->lineEditDuration->setup(m_ui->comboBoxUnitDuration, IBK::Unit("s"),
								  0, std::numeric_limits<double>::max(), tr("Duration of the simulation.") );

	m_ui->lineEditGridSize->setText(QString::number(0.1));
	m_ui->lineEditSunCone->setText(QString::number(3));

}

SVShadingCalculationDialog::~SVShadingCalculationDialog()
{
	delete m_ui;
}

int SVShadingCalculationDialog::edit() {

	// create a copy of the current project
	m_localProject = project(); // Mind: addresses to member variables m_solverParameters, m_location etc. do not change here!
	m_localProject.updatePointers();

	// initialize defaults
	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvement

	// initialize simulation parameters with meaningful defaults and fix possibly wrong values
	// in project (wherever they come from)
	if (simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].name.empty() ||
		simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value < 0)
	{
		simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", 2019);
	}

	if (simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].name.empty() ||
		simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].value < 0 ||
		simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].value > 365*24*3600)
	{
		simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].set("Start", 0, IBK::Unit("d"));
	}

	if (simParas.m_interval.m_para[ NANDRAD::Interval::P_End].name.empty() ||
		simParas.m_interval.m_para[ NANDRAD::Interval::P_End].value < 0)
	{
		simParas.m_interval.m_para[ NANDRAD::Interval::P_End].set("End", 1, IBK::Unit("a"));
	}


	updateTimeFrameEdits();

	return exec();
	// if dialog was confirmed, data is transfered into project
}

void SVShadingCalculationDialog::setProgressBar(double progress) {

	if (m_elapsedTime->elapsed() > 100) {
		m_progressDialog->setValue((int)(progress*100));
		qApp->processEvents();
	}
}

void SVShadingCalculationDialog::evaluateResults() {
	FUNCID(SVShadingCalculationDialog::evaluateResults);

	m_progressDialog->setValue(0);
	m_progressDialog->setLabelText(tr("Load shading factors") );

	unsigned int surfCounter = 0;

	for ( const VICUS::Surface *s : m_selSurfaces) {

		VICUS::Surface *surf = const_cast<VICUS::Surface *>(s);

		for ( SH::Polygon &s : m_shading.m_surfaces) {
			if ( s.m_id == surf->m_id ) {
				surf->m_shadingFactor = s.m_shadingFactors;
				break;
			}
		}

		std::ofstream out( "../../data/validation/SimQuality/TF09/ShadingFactor_" + s->m_displayName.toStdString() + "_" +
							QString::number(m_gridSize, 'f', 2).toStdString() + "_" + QString::number(m_sunCone, 'f', 2).toStdString() + ".tsv" );

		if( !out.is_open() )
			throw IBK::Exception(IBK::FormatString(), FUNC_ID);
		out << "Time [h]\tAzimut [Deg]\tAltitude [Deg]\tShading Factor [-]\n";

		for (unsigned int i=0; i<8760*2; ++i) {
			out << (double)i/2 << "\t" << m_shading.sunPositions()[i].m_azimuth / IBK::DEG2RAD << "\t" << m_shading.sunPositions()[i].m_altitude / IBK::DEG2RAD <<
				   "\t" << s->m_shadingFactor.value(i * 3600) << "\n";

			if ( i%300 == 0 ) {
				double progress = (double)( ( (double)surfCounter + ((double)i / 8760.0) ) / (double)m_selSurfaces.size() );
				m_progressDialog->setValue( (int)( progress*100 ) );
				qApp->processEvents();
			}
		}

		out.close();

		++surfCounter;
	}

	m_progressDialog->hide();

}

void SVShadingCalculationDialog::stopCalculation() {

	m_timer->stop();
}

void SVShadingCalculationDialog::updateTimeFrameEdits() {

	m_ui->lineEditStartDate->blockSignals(true);
	m_ui->lineEditEndDate->blockSignals(true);
	m_ui->lineEditDuration->blockSignals(true);

	// Note: we can be sure that all the parameters are set, though possibly to invalid values

	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	int startYear = simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	// fall-back to zero, if not specified
	double startOffset = simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].value;

	IBK::Time t(startYear, startOffset);
	m_ui->lineEditStartDate->setText( QString::fromStdString(t.toDateTimeFormat()) );

	double endTime = simParas.m_interval.m_para[ NANDRAD::Interval::P_End].value;
	if (simParas.m_interval.m_para[ NANDRAD::Interval::P_End].name.empty())
		endTime = startOffset + 365*24*3600; // fallback to 1 year
	double simDuration = endTime - startOffset;
	t += simDuration;

	m_ui->lineEditEndDate->setText( QString::fromStdString(t.toDateTimeFormat()) );

	IBK::Parameter durationPara;
	// use unit from end
	durationPara = IBK::Parameter("Duration", 0, simParas.m_interval.m_para[ NANDRAD::Interval::P_End].IO_unit);
	durationPara.value = simDuration; // set value in seconds
	m_ui->lineEditDuration->setFromParameter(durationPara);

	m_ui->lineEditStartDate->blockSignals(false);
	m_ui->lineEditEndDate->blockSignals(false);
	m_ui->lineEditDuration->blockSignals(false);
}

void SVShadingCalculationDialog::on_lineEditStartDate_editingFinished() {
	IBK::Time startTime = IBK::Time::fromDateTimeFormat(m_ui->lineEditStartDate->text().toStdString());

	// update date time
	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", startTime.year());
	simParas.m_interval.m_para[NANDRAD::Interval::P_Start].set("Start", startTime.secondsOfYear(), IBK::Unit("s"));
	updateTimeFrameEdits();
}


void SVShadingCalculationDialog::on_lineEditEndDate_editingFinished() {
	IBK::Time endTime = IBK::Time::fromDateTimeFormat(m_ui->lineEditEndDate->text().toStdString());

	// compose start time (startYear and offset are given and well defined, we ensure that)
	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	int startYear = simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	double offset = simParas.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Time startTime(startYear, offset);

	// compute difference between dates
	IBK::Time diff = endTime - startTime; // Might be negative!
	if (!diff.isValid()) {
		m_ui->lineEditDuration->setValue(0); // set zero duration to indicate that something is wrong!
		return;
	}

	// end date is the offset from start, so we first need the start date
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].set("End", diff.secondsOfYear(), IBK::Unit("s"));
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = m_ui->lineEditDuration->currentUnit();

	updateTimeFrameEdits();
}


void SVShadingCalculationDialog::on_lineEditDuration_editingFinishedSuccessfully() {
	// we always update the end time and let the end time signal do the undo action stuff
	IBK::Parameter durPara = m_ui->lineEditDuration->toParameter("Duration");
	if (durPara.name.empty())
		return; // invalid input in parameter edit

	if (durPara.value <= 0)
		return; // invalid input in parameter edit

	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	int startYear = simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	double offset = simParas.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Time startTime(startYear, offset);

	// add duration
	startTime += durPara.value;
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].set("End", startTime.secondsOfYear(), IBK::Unit("s"));
	// set duration unit in parameter - this will be used to select matching unit in combo box
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = durPara.IO_unit;
	updateTimeFrameEdits();
}

void SVShadingCalculationDialog::on_pushButtonCalculate_clicked(){

	// Start calculation

	FUNCID(SVShadingCalculation::calculateShadingFactors);

	std::vector<std::vector<IBKMK::Vector3D> > selSurf;
	std::vector<std::vector<IBKMK::Vector3D> > selObst;

	// We take all our selected surfaces
	project().selectedSurfaces(m_selSurfaces,VICUS::Project::SG_Building);
	project().selectedSurfaces(m_selObstacles,VICUS::Project::SG_Obstacle);

	NANDRAD::Location loc = project().m_location;

	NANDRAD::KeywordList::setParameter(loc.m_para, "Location::para_t", NANDRAD::Location::P_Latitude, 50 );
	NANDRAD::KeywordList::setParameter(loc.m_para, "Location::para_t", NANDRAD::Location::P_Longitude, 14.27 );

	loc.m_timeZone = 1;

	m_shading.setLocation(loc.m_timeZone, loc.m_para[NANDRAD::Location::P_Longitude].get_value("Deg"),
														 loc.m_para[NANDRAD::Location::P_Latitude].get_value("Deg") );

	m_shading.setShadingParameters(m_ui->lineEditGridSize->text().toDouble(), m_ui->lineEditSunCone->text().toDouble() );

	for (const VICUS::Surface *s: m_selObstacles) {
		selObst.push_back( s->m_geometry.vertexes() );
	}

	m_shading.initializeShadingCalculation(selObst);

	if ( m_progressDialog == nullptr )
		m_progressDialog = new QProgressDialog (tr("Calculate shading factors"), tr("Abort"), 0, m_selSurfaces.size(), this);

	m_progressDialog->setMaximum(100);
	m_progressDialog->setWindowModality(Qt::WindowModal);
	m_progressDialog->setValue(0);
	m_progressDialog->show();
	qApp->processEvents();

	QElapsedTimer progressTimer;
	progressTimer.start();

	int counter = 0;

	m_timer = new QTimer;
	m_elapsedTime = new QElapsedTimer;

	connect( m_progressDialog, &QProgressDialog::canceled, this, &SVShadingCalculationDialog::stopCalculation );

//	connect( &m_shading, SIGNAL(progress(double)), this, SLOT(setProgressBar(double) ) );
//	connect( &m_shading, &SH::StructuralShading::finished, this, &SVShadingCalculationDialog::evaluateResults );

	for (const VICUS::Surface *s: m_selSurfaces) {
		VICUS::Surface *surf = const_cast<VICUS::Surface *>(s);
		VICUS::Surface surfInverted = *s;

		m_shading.m_surfaces.push_back( SH::Polygon(surf->m_id, surfInverted.m_geometry.vertexes() ) );
	}

//	m_timer->setSingleShot(true);
//	connect(m_timer, &QTimer::timeout, &m_shading, &SH::StructuralShading::calculateShadingFactors);
//	m_timer->start();


}

void SVShadingCalculationDialog::on_lineEditGridSize_editingFinished() {
	if (m_ui->lineEditGridSize->isValid() )
		m_gridSize = m_ui->lineEditGridSize->value();
	else {
		m_ui->lineEditGridSize->setValue(m_gridSize);
	}
}

void SVShadingCalculationDialog::on_lineEditSunCone_editingFinished() {
	if (m_ui->lineEditSunCone->isValid() )
		m_sunCone = m_ui->lineEditSunCone->value();
	else {
		m_ui->lineEditGridSize->setValue(m_sunCone);
	}
}

void SVShadingCalculationDialog::on_pushButtonChancel_clicked() {
	this->reject();
}
