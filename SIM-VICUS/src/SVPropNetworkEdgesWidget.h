#ifndef SVPROPNETWORKEDGESWIDGETH
#define SVPROPNETWORKEDGESWIDGETH

#include <QWidget>


class QTableWidgetItem;

namespace Ui {
class SVPropNetworkEdgesWidget;
}

class SVPropNetworkEditWidget;

class SVPropNetworkEdgesWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVPropNetworkEdgesWidget(QWidget *parent = nullptr);
	~SVPropNetworkEdgesWidget();

	/*! Updates all widgets */
	void updateUi();

	void updateTableWidget();

	void clearUi();

	void setWidgetsEnabled(bool enabled);

private slots:
	void on_pushButtonRecalculateLength_clicked();

	void on_checkBoxSupplyPipe_clicked();

	void on_lineEditEdgeDisplayName_editingFinished();

	void on_pushButtonEditPipe_clicked();

	void on_pushButtonExchangePipe_clicked();

	void on_pushButtonSelectEdgesWithPipe_clicked();

	void on_pushButtonAssignPipe_clicked();

	void on_tableWidgetPipes_itemSelectionChanged();

	void on_tableWidgetPipes_itemDoubleClicked(QTableWidgetItem */*item*/);

private:

	Ui::SVPropNetworkEdgesWidget *m_ui;

	SVPropNetworkEditWidget	*m_pa;

};

#endif // SVPROPNETWORKEDGESWIDGETH
