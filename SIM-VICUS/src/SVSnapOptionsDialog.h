#ifndef SVSNAPOPTIONSDIALOGH
#define SVSNAPOPTIONSDIALOGH

#include <QDialog>

namespace Ui {
class SVSnapOptionsDialog;
}

class SVSnapOptionsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVSnapOptionsDialog(QWidget *parent = nullptr);
	~SVSnapOptionsDialog();

	void updateUi();

	void setPosition(const QPoint & position);

	void setExpanded(bool expanded);

private slots:
	void on_checkBoxGridPlane_clicked(bool checked);

	void on_checkBoxDrawing_clicked(bool checked);

	void on_checkBoxDrawingLine_clicked(bool checked);

	void on_toolButton_clicked();

	void on_checkBoxObjectVertex_clicked(bool checked);

	void on_checkBoxObjectCenter_clicked(bool checked);

	void on_checkBoxObjectEdge_clicked(bool checked);

	void on_lineEditSnapDistance_editingFinished();

private:
	Ui::SVSnapOptionsDialog *m_ui;

	bool					m_expanded = false;
};

#endif // SVSNAPOPTIONSDIALOGH
