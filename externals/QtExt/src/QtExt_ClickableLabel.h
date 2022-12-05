#ifndef QTEXT_CLICKABLELABELH
#define QTEXT_CLICKABLELABELH

#include <QWidget>
#include <QLabel>

namespace QtExt{

/*!
 * \brief The ClickableLabel class re-implements QLabel, with the feature that it emits a signal when clicked on.
 * The label can be provided with a custom id, which may be used for identification when receiving the clicked(id) signal.
 */

class ClickableLabel : public QLabel
{
	Q_OBJECT

public:
	explicit ClickableLabel(const QString &text="", QWidget* parent = Q_NULLPTR);
	explicit ClickableLabel(int id, const QString &text="", QWidget* parent = Q_NULLPTR);

	QString m_hoverWeight = "bold";
	QString m_normalWeight = "normal";

	bool	m_active = false;

signals:
	void clicked();
	void clicked(int m_id);

protected:
	void mousePressEvent(QMouseEvent* event) override;

	void enterEvent(QEvent */*ev*/) override {
		setStyleSheet("QLabel { font-weight: bold }");
	}

	void leaveEvent(QEvent *ev) override {
		if (!m_active)
			setStyleSheet("QLabel { font-weight: normal }");
	}

	int		m_id = -1;

};

} // namespace QtExt


#endif // QTEXT_CLICKABLELABELH
