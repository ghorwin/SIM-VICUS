#ifndef QtExt_ClickableLabelH
#define QtExt_ClickableLabelH

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
	explicit ClickableLabel( QWidget* parent = Q_NULLPTR);
	explicit ClickableLabel(const QString &text="", QWidget* parent = Q_NULLPTR);
	explicit ClickableLabel(int id, const QString &text="", QWidget* parent = Q_NULLPTR);

	void setId(unsigned int id)  {m_id = id; }
	unsigned int id() const { return m_id; }

	void setStyleSheet(const QString & normalStyleSheet, const QString & hoverStyleSheet);
	void setStyleSheet(const QString & normalStyleSheet, const QString & hoverStyleSheet, const QString & activeStyleSheet);

	/*! Switches button to active mode and set the "hovered" style sheet, even if mouse leaves label. */
	void setActive(bool active);

signals:
	/*! Emitted when user left-clicks in label.
		\code
		// retrieve ID of clicked label in slot
		int labelId = qobject_cast<ClickableLabel*>(sender())->id();
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

	void updateStyleSheet();

	unsigned int		m_id		= -1;

	bool				m_active	= false;

	/*! Regular style-sheet. */
	QString				m_normalStyleSheet;
	/*! Style-sheet to be used when hovering. */
	QString				m_hoverStyleSheet;
	/*! Style-sheet to be used when hovering. */
	QString				m_activeStyleSheet;
};

} // namespace QtExt


#endif // QtExt_ClickableLabelH
