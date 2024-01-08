#ifndef SVNETWORKSIMULTANEITYDIALOGH
#define SVNETWORKSIMULTANEITYDIALOGH

#include <QDialog>

#include <IBK_LinearSpline.h>

class QwtPlotCurve;

namespace Ui {
class SVNetworkSimultaneityDialog;
}

class SVNetworkSimultaneityDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVNetworkSimultaneityDialog(QWidget *parent = nullptr);
	~SVNetworkSimultaneityDialog();

	void edit(IBK::LinearSpline &simultaneity);

	void updatePlot();

	void updateTableWidget();

private slots:
	void on_toolButtonAddPoint_clicked();

	void on_toolButtonRemovePoint_clicked();

	void on_tableWidget_cellChanged(int row, int column);

	void on_tableWidget_itemSelectionChanged();

	void on_pushButtonSetDefault_clicked();

private:

	void modifyValues();

	Ui::SVNetworkSimultaneityDialog		*m_ui;

	IBK::LinearSpline					m_tmpSimultaneity;
	IBK::LinearSpline					m_simultaneity;

	/*! The curve used to plot */
	QwtPlotCurve						*m_curve;
};

#endif // SVNETWORKSIMULTANEITYDIALOGH
