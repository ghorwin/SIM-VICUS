#ifndef SVNETWORKEXPORTDIALOG_H
#define SVNETWORKEXPORTDIALOG_H

#include <QDialog>
#include <QMap>

namespace Ui {
class SVNetworkExportDialog;
}

class SVNetworkExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SVNetworkExportDialog(QWidget *parent = nullptr);
    ~SVNetworkExportDialog();
	bool edit();

private slots:
	void on_pushButtonExport_clicked();

	void on_pushButtonSelectFileLocation_clicked();

private:
	Ui::SVNetworkExportDialog * m_ui;

	QMap<QString, unsigned> m_existingNetworksMap;
};

#endif // SVNETWORKEXPORTDIALOG_H
