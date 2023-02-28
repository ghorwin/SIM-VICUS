#include "SVTimeSliderWidget.h"
#include "ui_SVTimeSliderWidget.h"

#include <QLayout>

#include <algorithm>

#include <IBK_Time.h>
#include <IBK_assert.h>
#include <IBK_UnitList.h>


SVTimeSliderWidget::SVTimeSliderWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVTimeSliderWidget),
	m_spaceCoordinate(true)
{
	m_ui->setupUi(this);
	// update Icons ... we have to wait for judith here
	m_ui->widgetEnd->setNormalIcons(
		QPixmap(":/gfx/simulation/goToEnd_24x24/03_GoEnd_enabled_24x24.png"),
		QPixmap(":/gfx/simulation/goToEnd_24x24/04_GoEnd_hover_24x24.png"),
		QPixmap(":/gfx/simulation/goToEnd_24x24/01_GoEnd_disabled_24x24.png")
		);
	m_ui->widgetEnd->setCheckedIcons(
		QPixmap(":/gfx/simulation/goToEnd_24x24/06_GoEnd_checked_24x24.png"),
		QPixmap(":/gfx/simulation/goToEnd_24x24/05_GoEnd_enabled_checked_24x24.png"),
		QPixmap(":/gfx/simulation/goToEnd_24x24/02_GoEnd_disabled_checked_24x24.png")
		);

	m_ui->widgetFastForeward->setNormalIcons(
		QPixmap(":/gfx/simulation/fforeward_24x24/03_FFW_enabled_24x24.png"),
		QPixmap(":/gfx/simulation/fforeward_24x24/04_FFW_hover_24x24.png"),
		QPixmap(":/gfx/simulation/fforeward_24x24/01_FFW_disabled_24x24.png")
		);
	m_ui->widgetFastForeward->setCheckedIcons(
		QPixmap(":/gfx/simulation/fforeward_24x24/06_FFW_hover_checked_24x24.png"),
		QPixmap(":/gfx/simulation/fforeward_24x24/05_FFW_enabled_checked_24x24.png"),
		QPixmap(":/gfx/simulation/fforeward_24x24/02_FFW_disabled_checked_24x24.png")
		);

	m_ui->widgetFastRewind->setNormalIcons(
		QPixmap(":/gfx/simulation/fbackward_24x24/03_FBW_enabled_24x24.png"),
		QPixmap(":/gfx/simulation/fbackward_24x24/04_FBW_hover_24x24.png"),
		QPixmap(":/gfx/simulation/fbackward_24x24/01_FBW_disabled_24x24.png")
		);
	m_ui->widgetFastRewind->setCheckedIcons(
		QPixmap(":/gfx/simulation/fbackward_24x24/06_FBW_hover_checked_24x24.png"),
		QPixmap(":/gfx/simulation/fbackward_24x24/05_FBW_enabled_checked_24x24.png"),
		QPixmap(":/gfx/simulation/fbackward_24x24/02_FBW_disabled_checked_24x24.png")
		);

	m_ui->widgetPlay->setNormalIcons(
		QPixmap(":/gfx/simulation/play_24x24/03_Play_enabled_24x24.png"),
		QPixmap(":/gfx/simulation/play_24x24/04_Play_hover_24x24.png"),
		QPixmap(":/gfx/simulation/play_24x24/01_Play_disabled_24x24.png")
		);
	m_ui->widgetPlay->setCheckedIcons(
		QPixmap(":/gfx/simulation/pause_24x24/06_Pause_hover_checked_24x24.png"),
		QPixmap(":/gfx/simulation/pause_24x24/05_Pause_enabled_checked_24x24.png"),
		QPixmap(":/gfx/simulation/pause_24x24/02_Pause_disabled_checked_24x24.png")
		);

	m_ui->widgetPlay->setCheckable(true);

	m_ui->widgetReset->setNormalIcons(
		QPixmap(":/gfx/simulation/goToStart_24x24/03_GoStart_enabled_24x24.png"),
		QPixmap(":/gfx/simulation/goToStart_24x24/04_GoStart_hover_24x24.png"),
		QPixmap(":/gfx/simulation/goToStart_24x24/01_GoStart_disabled_24x24.png")
		);
	m_ui->widgetReset->setCheckedIcons(
		QPixmap(":/gfx/simulation/goToStart_24x24/06_GoStart_hover_checked_24x24.png"),
		QPixmap(":/gfx/simulation/goToStart_24x24/05_GoStart_enabled_checked_24x24.png"),
		QPixmap(":/gfx/simulation/goToStart_24x24/02_GoStart_disabled_checked_24x24.png")
		);

	m_ui->widgetRecord->setNormalIcons(
		QPixmap(":/gfx/simulation/record_24x24/03_enabled.png"),
		QPixmap(":/gfx/simulation/record_24x24/04_hover.png"),
		QPixmap(":/gfx/simulation/record_24x24/01_disabled.png")
		);
	m_ui->widgetRecord->setCheckedIcons(
		QPixmap(":/gfx/simulation/record_24x24/06_hover_checked.png"),
		QPixmap(":/gfx/simulation/record_24x24/05_enabled_checked.png"),
		QPixmap(":/gfx/simulation/record_24x24/02_disabled_checked.png")
		);

	m_ui->widgetRecord->setCheckable(true);
	connect(m_ui->widgetSlider, SIGNAL(indexChanged(unsigned int)),
			this, SLOT(onSliderIndexChanged(unsigned int)));
	connect(m_ui->widgetFastForeward, SIGNAL(clicked()),
			this, SLOT(onFastForward()));
	connect(m_ui->widgetFastRewind, SIGNAL(clicked()),
			this, SLOT(onFastRewind()));
	connect(m_ui->widgetPlay, SIGNAL(clicked(bool)),
			this, SLOT(onPlayStop(bool)));
	connect(m_ui->widgetReset, SIGNAL(clicked()),
			this, SLOT(onReset()));
	connect(m_ui->widgetEnd, SIGNAL(clicked()),
			this, SLOT(onGoToEnd()));

	/// \todo Connect other buttons.

	QLayout * lay = dynamic_cast<QLayout *>(layout());
	Q_ASSERT(lay != NULL);
	lay->setSpacing(1);
	lay->setContentsMargins(0,0,0,0);

	connect(m_ui->comboBoxUnit, SIGNAL(currentIndexChanged(int)),
			this, SLOT(onValueUnitChanged(int)));

	connect(&m_moveTimer, SIGNAL(timeout()), this,
			SLOT(onMoveTimerTimeOut()));
	m_moveTimer.setInterval(500);
}

SVTimeSliderWidget::~SVTimeSliderWidget() {
	delete m_ui;
}

void SVTimeSliderWidget::clear() {
	m_spaceCoordinate = true;
	m_ui->lineEditCurrentTime->clear();
	m_ui->lineEditOffsetFromStart->clear();
	m_moveTimer.stop();
	m_ui->widgetPlay->setChecked(false);
	m_ui->widgetSlider->setValues(nullptr, 0);
	setEnabled(false);
}

bool SVTimeSliderWidget::isEmpty() const {
	return !isEnabled() || m_ui->widgetSlider->values().empty();
}

double SVTimeSliderWidget::currentCutValue() const {
	return m_ui->widgetSlider->currentValue(); // slider widget handles "empty" situation
}

QString SVTimeSliderWidget::currentTimeUnit() const {
	return m_ui->comboBoxUnit->currentText();
}

void SVTimeSliderWidget::setValues(const IBK::UnitVector& values) {
	// in case of animation, stop it
	if (m_moveTimer.isActive()) {
		m_moveTimer.stop();
		m_ui->widgetPlay->setChecked(false);
	}
	if (values.empty()) {
		clear();
		return;
	}
	int currentComboBoxIndex = m_ui->comboBoxUnit->currentIndex();

	// populate combo box but prevent unit changed signals
	m_ui->comboBoxUnit->blockSignals(true);
	if (values.m_unit.id() == IBK::Unit("s").id()) {
		m_spaceCoordinate = false;
		std::vector<IBK::Unit> timeUnits;
		IBK::UnitList::instance().convertible_units(timeUnits, IBK::Unit("s"));
		m_ui->comboBoxUnit->clear();
		for (unsigned int i=0; i<timeUnits.size(); ++i) {
			m_ui->comboBoxUnit->addItem( QString::fromStdString(timeUnits[i].name()), timeUnits[i].id());
		}
		if (currentComboBoxIndex != -1)
			m_ui->comboBoxUnit->setCurrentIndex(currentComboBoxIndex);
		else
			m_ui->comboBoxUnit->setCurrentIndex(3); // hours are the default
	}
	m_ui->comboBoxUnit->blockSignals(false);

	// values are always in seconds or meter
	m_ui->widgetSlider->setValues(&values.m_data[0], (unsigned int)values.size()); // this will emit a change signal when the current index changes

	setEnabled(true);
	updateLabels();
}


void SVTimeSliderWidget::updateLabels() {
	// no slider values, no label update - won't be drawn anyway
	if (m_ui->widgetSlider->values().empty()) {
		m_ui->lineEditCurrentTime->setText(QString());
		m_ui->lineEditOffsetFromStart->setText(QString());
		return;
	}
	double offset;
	double currentValue = m_ui->widgetSlider->currentValue();
	std::pair<double,double> minMax = m_ui->widgetSlider->valueLeftRight();
	if (!m_ui->widgetSlider->isReversed())
		offset = currentValue - minMax.first;
	else
		offset = currentValue - minMax.second;

	if( !m_spaceCoordinate) {
		IBK::Time time;
		time.set(currentValue); // currentValue = offset from midnight January 1.
		m_ui->lineEditCurrentTime->setText(QString::fromStdString(time.toFullDateFormat()));
		double val = currentValue;
		int currentUnitIndex = m_ui->comboBoxUnit->currentIndex();
		IBK::Unit unit(m_ui->comboBoxUnit->itemData(currentUnitIndex).toUInt());
		IBK::UnitList::instance().convert(IBK::Unit("s"), unit, val);
		m_ui->lineEditOffsetFromStart->setText( QString("%L1").arg(val));
	}
	else {
		m_ui->lineEditCurrentTime->setText(QString("%1 m").arg(currentValue));
		m_ui->lineEditOffsetFromStart->setText(QString("%1 m").arg(offset));
	}
}

void SVTimeSliderWidget::setCurrentIndex(unsigned int index) {
	m_ui->widgetSlider->setCurrentIndex(index); // slider handles situation of empty values vector
	updateLabels();
}

void SVTimeSliderWidget::setCurrentValue(double value) {
	m_ui->widgetSlider->setCurrentValue(value); // slider handles situation of empty values vector
	updateLabels();
}

void SVTimeSliderWidget::onSliderIndexChanged(unsigned int index) {
	if (m_ui->widgetSlider->values().empty())
		return; // no values, no change in slider
	m_ui->widgetSlider->setCurrentIndex(index);
	updateLabels();
	emit cutValueChanged(m_ui->widgetSlider->currentValue());
}

void SVTimeSliderWidget::onValueUnitChanged(int index) {
	if (index != -1)
		updateLabels();
}

void SVTimeSliderWidget::onPlayStop(bool checked) {
	if (checked)
		m_moveTimer.start();
	else if (m_moveTimer.isActive())
		m_moveTimer.stop();
}

void SVTimeSliderWidget::onFastForward() {
	m_ui->widgetSlider->moveIndex(1);
}

void SVTimeSliderWidget::onFastRewind() {
	m_ui->widgetSlider->moveIndex(-1);
}

void SVTimeSliderWidget::onGoToEnd() {
	m_ui->widgetSlider->setCurrentIndex( m_ui->widgetSlider->maxIndex() );
}

void SVTimeSliderWidget::onReset() {
	m_ui->widgetSlider->setCurrentIndex(0);
}

void SVTimeSliderWidget::onMoveTimerTimeOut() {
	m_moveTimer.stop();
	onFastForward();
	// only restart timer if we haven't reached the end yet
	if (m_ui->widgetSlider->atEnd()) {
		m_ui->widgetPlay->setChecked(false);
	}
	else {
		m_moveTimer.start();
	}
}


void SVTimeSliderWidget::on_lineEditOffsetFromStart_editingFinished() {
	// try to convert value to seconds
	QString currentVal = m_ui->lineEditOffsetFromStart->text();
	bool ok;
	QLocale loc;
	double val = loc.toDouble(currentVal, &ok);
	if (ok) {
		// convert value to seconds
		int currentUnitIndex = m_ui->comboBoxUnit->currentIndex();
		IBK::Unit unit(m_ui->comboBoxUnit->itemData(currentUnitIndex).toUInt());
		IBK::UnitList::instance().convert(unit, IBK::Unit("s"), val);
		m_ui->widgetSlider->setCurrentValue(val);
	}
	updateLabels();
}


void SVTimeSliderWidget::on_horizontalSliderDisplaySpeed_sliderMoved(int position) {
	double relPosition = position;
	relPosition /= m_ui->horizontalSliderDisplaySpeed->maximum();

	// relPosition = 0..1
	// interval = 1500..100
	m_moveTimer.setInterval(50 + 1500*(1-relPosition));
}
