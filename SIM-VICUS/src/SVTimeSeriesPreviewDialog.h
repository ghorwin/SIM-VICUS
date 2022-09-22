#ifndef SVTIMESERIESPREVIEWDIALOGH
#define SVTIMESERIESPREVIEWDIALOGH

#include <QDialog>

#include <NANDRAD_LinearSplineParameter.h>


class QListWidgetItem;


namespace Ui {
class SVTimeSeriesPreviewDialog;
}

class SVTimeSeriesPreviewDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVTimeSeriesPreviewDialog(QWidget *parent = nullptr);
	~SVTimeSeriesPreviewDialog();

	void select(NANDRAD::LinearSplineParameter &data);

private slots:
	void on_filepathDataFile_editingFinished();

	void on_radioButtonRelativeFilePath_toggled(bool checked);

	void on_listWidgetColumnSelection_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

	void on_pushButtonEditAnnualDataInTexteditor_clicked();

	void on_radioButtonInterpolationLinear_toggled(bool checked);

private:
	void updateAnnualDataDiagram() const;

	void updateColumnIndexList();

	void updateInput();

	void updateName();

	void generateRelativeFilePath() const;



	Ui::SVTimeSeriesPreviewDialog	*m_ui;

	NANDRAD::LinearSplineParameter  m_data;
};

#endif // SVTIMESERIESPREVIEWDIALOGH
