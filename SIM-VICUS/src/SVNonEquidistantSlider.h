#ifndef SVNONEQUIDISTANTSLIDERH
#define SVNONEQUIDISTANTSLIDERH

#include <QWidget>

namespace Ui {
class SVNonEquidistantSlider;
}

class SVNonEquidistantSlider : public QWidget
{
	Q_OBJECT

public:
	explicit SVNonEquidistantSlider(QWidget *parent = nullptr);

	/*! Returns the current value.
		\note Do not call this function when slider is empty!
	*/
	double currentValue() const;

	/*! Returns the value for the given index.
		\param index Index for internal data array.
	*/
	double valueFromIndex(unsigned int index) const;

	/*! Returns the current index.
		\note If slider is empty, the index returned will be -1.
	*/
	unsigned int currentIndex() const { return m_currentIndex; }

	/*! Sets a new internal data set.
		If the same values are set again, the current index won't be changed and no signal
		will be emitted.
		\param values New data array, copied into local array.
		\param size Size of the data array, may be 0.
	*/
	void setValues(const double* values, unsigned int size);

	/*! Returns cached values, can be used to check if slider has some values. */
	const std::vector<double> & values() const { return m_values; }

	/*! Returns true if current index has reached the end of the slider range.
		If slider is empty (no values), this function returns true.
	*/
	bool atEnd() const { return m_currentIndex == maxIndex(); }

	/*! Returns the maximum possible index value.
		If slider is empty (no values), this function returns -1.
	*/
	unsigned int maxIndex() const { return static_cast<unsigned int>(m_values.size()-1); }

public slots:
	/*! Sets the slider to a new value.
		The resulting current value can be different because it must be a member in the data array.
		The nearest existing value will be setted.
		\param val Some value, if outside the range the min or max value is set, respectively.
		\note If slider is empty (no values), this function simply returns.
	*/
	void setCurrentValue(double val);

	/*! Move the current index by the given offset. Can be positive or negative.
		Moving cannot exceed the maximum and minimum indices.
		If slider is empty (no values), this function simply returns.
		\param move Index offset.
		\note Emits the signal indexChanged() when the index has changed (may not be the case
			  when minimum or maximum have been reached).
	*/
	void moveIndex(int move);

	/*! Sets a new current index.
		\param index Index for internal data array.
	*/
	void setCurrentIndex(unsigned int index);

	/*! Returns true if values are monotonically decreasing. */
	bool isReversed() const { return m_reversed; }

	/*! Returns the left and right position of the slider.*/
	std::pair<unsigned int,unsigned int> leftRight() const;

	/*! Returns the first (left) and last (right) value of the value array.*/
	std::pair<double,double> valueLeftRight() const;

signals:
	/*! Will be emitted if a new index is set.
		\param index New index.
	*/
	void indexChanged(unsigned int);

protected:
	/*! Paint event paints the slider with marker for the position.*/
	void paintEvent(QPaintEvent *event);

	/*! For moving marker with the mouse.*/
	void mouseMoveEvent(QMouseEvent *e);

	/*! For signaling begin of marker movement.*/
	void mousePressEvent(QMouseEvent *e);

private:

	/*! Calculates the position of the given value.
		Must not be called for empty slider.
	*/
	unsigned int xForValue(double);

	/*! Calculates the value at a given position.
		Must not be called for empty slider.
	*/
	double valueForX(unsigned int);

	/*! Calculates the array index of the value nearest to the given x position.
		Must not be called for empty slider.
	*/
	unsigned int indexForX(unsigned int x);

	/*! Calculates the nearest value in value array for the given arbitrary value.
		Must not be called for empty slider.
		\param val A value, if outside the range of the slider, the first or last index, respectively, is returned.
	*/
	unsigned int nearestIndex(double val);

	std::vector<double>		m_values;			///< Value array
	unsigned int			m_currentIndex;		///< Index for current value in value array, must be in the range 0 < m_values.size()
	bool					m_reversed;			///< If true the maximum value is on left side.
	unsigned int			m_leftMargin;		///< Margin between left side of slider and left side of widget.
	unsigned int			m_rightMargin;		///< Margin between right side of slider and right side of widget.
	unsigned int			m_topMargin;		///< Y position for slider.
	unsigned int			m_markerHeight;		///< Height of the marker triangle.
	unsigned int			m_ticHeight;		///< Height of a value tick.
	qreal					m_dpiFact;			///< Factor for dpi correction.
	unsigned int			m_minTickDist;		///< Minimum tick distance

};

#endif // SVNONEQUIDISTANTSLIDERH
