#ifndef SVTIMESLIDERWIDGETH
#define SVTIMESLIDERWIDGETH

#include <QWidget>
#include <QTimer>

#include <IBK_UnitVector.h>


namespace Ui {
class SVTimeSliderWidget;
}

class SVTimeSliderWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVTimeSliderWidget(QWidget *parent = nullptr);
	~SVTimeSliderWidget();


	/*! Set the values to be used by the slider.
		\param values Vector of values to be used in the slider, may be empty in which case the
		cut slider widget will have disabled controls and calls to setCurrentIndex() and setCurrentValue()
		will be ignored.
	*/
	void setValues(const IBK::UnitVector& values);

	/*! Clears the internal members of the slider widget and hides the widget. */
	void clear();

	/*! Returns true, if no values are currently set in the slider or slider is disabled. */
	bool isEmpty() const;

	/*! Returns the current cut value (in m or s) in the slider widget,
		or std::numeric_limits<double>::quiet_NaN() if slider widget is disabled.
	*/
	double currentCutValue() const;

	/*! Returns the currently selected time unit (only display unit for slider, the cut value is always in s). */
	QString currentTimeUnit() const;

signals:
	/*! Will be emitted if a new cutvalue is set.
		\param value New cut value.
	*/
	void cutValueChanged(double value);

public slots:
	/*! Sets the current index to the given index.
		\param index New data index.
		\note Does emit the signal indexChanged() when the new index value differs from the old.
	*/
	void setCurrentIndex(unsigned int index);

	/*! Calculates the new index from the given value and set the current index.
		The resulting data value can be different if the given value doesn't exist in the value vector.
		\param value New data value.
		\note Does emit the signal indexChanged() when the new index value differs from the old.
	*/
	void setCurrentValue(double value);

private slots:
	/*! Reactes on all slider changes and gives the new index.*/
	void onSliderIndexChanged(unsigned int index);

	/*! Triggered by a press on the play/stop button. */
	void onPlayStop(bool checked);

	/*! Reacts on press on fast foreward button.*/
	void onFastForward();

	/*! Reacts on press on fast rewind button.*/
	void onFastRewind();

	/*! Triggered by reset button. */
	void onReset();

	/*! Triggered by reset button. */
	void onGoToEnd();

	/*! Update the widget state.*/
	void updateLabels();

	/*! Triggered when combo box changes value. */
	void onValueUnitChanged(int);

	/*! Triggered when move timer fires. */
	void onMoveTimerTimeOut();

	/*! Triggered when offset has been edited. */
	void on_lineEditOffsetFromStart_editingFinished();

	/*! Adjusts timer interval. */
	void on_horizontalSliderDisplaySpeed_sliderMoved(int position);


private:
	Ui::SVTimeSliderWidget		*m_ui;
	bool						m_spaceCoordinate;	///< Is true if the cut plane is a space otherwise it is time.
	QTimer						m_moveTimer;		///< Timer, that repeatedly calls onFastForward()

};

#endif // SVTIMESLIDERWIDGETH
