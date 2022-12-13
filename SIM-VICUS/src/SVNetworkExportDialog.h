#ifndef SVNETWORKEXPORTDIALOG_H
#define SVNETWORKEXPORTDIALOG_H

#include <QDialog>
#include <QMap>

#include <VICUS_Network.h>

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

	void on_checkBoxExportPipeline_toggled(/*bool checked*/);

	void on_checkBoxExportSubStation_toggled(/*bool checked*/);

	/*! triggers a check if the export button should be enabled*/
	void on_lineEditExportFileName_editingFinished();

	/*! triggers exportToGeoJson and a file will be written*/
	void on_buttonBox_accepted();

private:
	Ui::SVNetworkExportDialog * m_ui;

	/*! reads the given network and saves its features in geoJson format*/
	void exportToGeoJson(unsigned int networkId);

	/*! checks if a file name is set and if at least one checkbox is checked*/
	void updateOkButtonEnableState();

};

#endif // SVNETWORKEXPORTDIALOG_H
