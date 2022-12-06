#ifndef QTEXT_CLICKABLELABELH
#define QTEXT_CLICKABLELABELH

#include <QWidget>
#include <QLabel>

namespace QtExt{

/*! The ClickableLabel class re-implements QLabel, with the feature that it emits a signal when clicked on.
	The label can be provided with a custom id, which may be used for identification when receiving the clicked() signal,
	see documentation of signal clicked().
*/
class ClickableLabel : public QLabel {
	Q_OBJECT

public:
	explicit ClickableLabel(const QString &text="", QWidget* parent = Q_NULLPTR);
	explicit ClickableLabel(int id, const QString &text="", QWidget* parent = Q_NULLPTR);

	unsigned int id() const { return m_id; }
	bool isHovered() const { return m_hovered; }

	void setStyleSheet(const QString & normalStyleSheet, const QString & hoverStyleSheet);

signals:
	/*! Emitted when user left-clicks in label.
		\code
		// retrieve ID of clicked label
		int labelId = qobject_cast<ClickableLabel>(sender())->id();
		\endcode
	*/
	void clicked();

protected:
	void mousePressEvent(QMouseEvent* event) override;

	/*! Alters stylesheet and sets m_hovered flag. */
	void enterEvent(QEvent *ev) override;
	/*! Alters stylesheet and clears m_hovered flag. */
	void leaveEvent(QEvent *ev) override;

private:
	unsigned int		m_id		= -1;

	bool				m_hovered	= false;

	/*! Regular style-sheet. */
	QString				m_normalStyleSheet;
	/*! Style-sheet to be used when hovering. */
	QString				m_hoverStyleSheet;
};

} // namespace QtExt


#endif // QTEXT_CLICKABLELABELH
