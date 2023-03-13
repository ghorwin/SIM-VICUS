#include "SVTimeSeriesPreviewDialog.h"
#include "ui_SVTimeSeriesPreviewDialog.h"

#include "SVSettings.h"
#include "SVProjectHandler.h"

#include <IBK_FileUtils.h>
#include <IBK_CSVReader.h>

#include <QtExt_Conversions.h>


SVTimeSeriesPreviewDialog::SVTimeSeriesPreviewDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVTimeSeriesPreviewDialog) {
	m_ui->setupUi(this);

	setWindowTitle(tr("Edit heat exchange spline"));

	QFont f = m_ui->listWidgetColumnSelection->font();
	int pointSize = int(f.pointSizeF()*0.8);
	f.setPointSize(pointSize);
	m_ui->listWidgetColumnSelection->setFont(f);

	m_ui->filepathDataFile->setup("", true, true, tr("Time-series data files (*.tsv *.csv);;All files (*.*)"),
										SVSettings::instance().m_dontUseNativeDialogs);
}

SVTimeSeriesPreviewDialog::~SVTimeSeriesPreviewDialog() {
	delete m_ui;
}


void SVTimeSeriesPreviewDialog::updateInput() {
	// now do all the updates

	m_ui->filepathDataFile->setFilename("");
	m_ui->labelFileNameReference->setText("");
	m_ui->pushButtonEditAnnualDataInTexteditor->setEnabled(false);

	m_ui->radioButtonInterpolationLinear->blockSignals(true);
	m_ui->radioButtonInterpolationLinear->setChecked(true);
	m_ui->radioButtonInterpolationLinear->blockSignals(false);

	m_ui->radioButtonRelativeFilePath->blockSignals(true);
	m_ui->radioButtonRelativeFilePath->setChecked(true);
	m_ui->radioButtonRelativeFilePath->blockSignals(false);

	// clear list widget
	m_ui->listWidgetColumnSelection->selectionModel()->blockSignals(true);
	m_ui->listWidgetColumnSelection->clear();
	m_ui->listWidgetColumnSelection->selectionModel()->blockSignals(false);
	m_ui->widgetColumnSelection->setEnabled(false);

	if (m_data.m_tsvFile.isValid()) {
		IBK::Path annualDataFile = m_data.m_tsvFile;
		try {
			annualDataFile = SVProjectHandler::instance().replacePathPlaceholders(m_data.m_tsvFile);
		} catch (...) {
			// in case we have no project loaded and "Project Directory" is used as placeholder
		}
		m_ui->filepathDataFile->setFilename(QString::fromStdString(annualDataFile.absolutePath().str()) );

		bool useLinearInterpolation = m_data.m_interpolationMethod == NANDRAD::LinearSplineParameter::I_LINEAR ||
									m_data.m_interpolationMethod == NANDRAD::LinearSplineParameter::NUM_I;
		m_ui->radioButtonInterpolationLinear->blockSignals(true);
		m_ui->radioButtonInterpolationLinear->setChecked(useLinearInterpolation);
		m_ui->radioButtonInterpolationConstant->setChecked(!useLinearInterpolation);
		m_ui->radioButtonInterpolationLinear->blockSignals(false);

		m_ui->radioButtonRelativeFilePath->blockSignals(true);
		if (m_data.m_tsvFile.hasPlaceholder()) {
			m_ui->radioButtonRelativeFilePath->setChecked(true);
		}
		else {
			m_ui->radioButtonAbsoluteFilePath->setChecked(true);
		}
		m_ui->radioButtonRelativeFilePath->blockSignals(false);
		// update relative path
		generateRelativeFilePath();
		m_ui->pushButtonEditAnnualDataInTexteditor->setEnabled(true);
		updateColumnIndexList();
	}
}


void SVTimeSeriesPreviewDialog::select(NANDRAD::LinearSplineParameter & data) {

	// copy spline data
	m_data = data;

	updateInput();

	// if we click ok, copy data back to reference
	if (exec() == QDialog::Accepted)
		data = m_data;
}


void SVTimeSeriesPreviewDialog::on_filepathDataFile_editingFinished() {

	// clear embedded spline data
	m_data.m_values.clear();

	// update text label with file reference
	// this also signals a modification of the m_current object
	on_radioButtonRelativeFilePath_toggled(m_ui->radioButtonRelativeFilePath->isChecked());

	QString dataFilePath = m_ui->filepathDataFile->filename();
	if (dataFilePath.trimmed().isEmpty()) {
		m_data.m_tsvFile.clear();
		return;
	}

	// allow editing of file in text editor
	m_ui->pushButtonEditAnnualDataInTexteditor->setEnabled(true);

	// initially disable column selection widget
	m_ui->widgetColumnSelection->setEnabled(false);

	updateColumnIndexList(); // if there are columns to be selected, the widget will be re-enabled here
}


void SVTimeSeriesPreviewDialog::on_radioButtonRelativeFilePath_toggled(bool) {
	// read path from
	generateRelativeFilePath();
	m_data.m_tsvFile = m_ui->labelFileNameReference->text().toStdString();
}


void SVTimeSeriesPreviewDialog::on_pushButtonEditAnnualDataInTexteditor_clicked() {
	// span editor if valid file name has been entered
	IBK::Path f(m_ui->filepathDataFile->filename().toStdString());
	IBK::Path adjustedFileName;
	int selectedColumn = 1;
	IBK::extract_number_suffix(f, adjustedFileName, selectedColumn);
	if (adjustedFileName.exists()) {
		SVSettings::instance().openFileInTextEditor(this, QString::fromStdString(adjustedFileName.str()));
	}
	else {
		QMessageBox::critical(this, tr("Invalid filename"), tr("Cannot edit file, file does not exist, yet."));
	}
}


void SVTimeSeriesPreviewDialog::generateRelativeFilePath() const{
	// if filename is empty, simply return
	if (m_ui->filepathDataFile->filename().trimmed().isEmpty()) {
		m_ui->labelFileNameReference->setText("");
		return;
	}

	// no project file saved yet? or absolute file path selected?
	if (SVProjectHandler::instance().projectFile().isEmpty() ||
		m_ui->radioButtonAbsoluteFilePath->isChecked() )
	{
		m_ui->labelFileNameReference->setText(m_ui->filepathDataFile->filename());
		return;
	}

	IBK::Path fname(QtExt::QString2Path(m_ui->filepathDataFile->filename()));

	// relative to project dir?
	if (m_ui->radioButtonRelativeFilePath->isChecked()) {
		// generate relative path
		try {
			IBK::Path relFname = fname.relativePath( QtExt::QString2Path(SVProjectHandler::instance().projectFile()).parentPath() );
			m_ui->labelFileNameReference->setText("${Project Directory}/" + QString::fromStdString(relFname.str()));
		}
		catch (...) {
			m_ui->labelFileNameReference->setText(m_ui->filepathDataFile->filename());
		}
	}
}


void SVTimeSeriesPreviewDialog::updateColumnIndexList() {
	// clear list widget
	m_ui->listWidgetColumnSelection->selectionModel()->blockSignals(true);
	m_ui->listWidgetColumnSelection->clear();
	m_ui->listWidgetColumnSelection->selectionModel()->blockSignals(false);

	QString dataFilePath = m_ui->filepathDataFile->filename();
	// parse tsv-file and if several data columns are in file, show the column selection list widget
	IBK::Path filePath(dataFilePath.toStdString()); // this is always an absolute path
	// check if we have a  csv/tsv file
	IBK::Path adjustedFileName;
	int selectedColumn = 1;
	IBK::extract_number_suffix(filePath, adjustedFileName, selectedColumn);
	std::string extension = IBK::tolower_string(adjustedFileName.extension());
	// read first line of file
	IBK::CSVReader reader;
	try {
		// read only header
		reader.read(adjustedFileName, true, true);
	}
	catch (...) {
		m_ui->widgetTimeSeriesPreview->setErrorMessage(tr("Error reading data file."));
		return;
	}

	// store x title
	if (reader.m_captions.size() > 0 && reader.m_units.size() > 0)
		m_xTitle = IBK::FormatString("%1 [%2]").arg(reader.m_captions[0]).arg(reader.m_units[0]).str();

	// special case: only two columns, just compose linear spline parameter and populate diagram
	if (reader.m_captions.size() == 2) {
		m_yTitle = IBK::FormatString("%1 [%2]").arg(reader.m_captions[1]).arg(reader.m_units[1]).str();
		updateAnnualDataDiagram();
		return;
	}

	m_ui->widgetColumnSelection->setEnabled(true);
	m_ui->listWidgetColumnSelection->selectionModel()->blockSignals(true);

	// process all columns past the first
	for (unsigned int i=1; i<reader.m_captions.size(); ++i) {

		// try to extract unit
		QListWidgetItem * item = nullptr;
		std::string ustr = reader.m_units[i];
		if (ustr.empty())
			ustr = "-";
		item = new QListWidgetItem(QString("%1 [%2]").arg(QString::fromStdString(reader.m_captions[i]),
														  QString::fromStdString(ustr)) );
		try {
			IBK::Unit u(ustr); // will throw in case of unknown unit

			item->setData(Qt::UserRole, i);
			item->setData(Qt::UserRole+1, QString::fromStdString(ustr));
			item->setData(Qt::UserRole+2, QString::fromStdString(reader.m_captions[i]));
			item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		}
		catch (...) {
			item->setData(Qt::UserRole, -2); // unrecognized unit
			item->setForeground(Qt::gray);
		}
		m_ui->listWidgetColumnSelection->addItem(item);
	}
	selectedColumn = qMin(selectedColumn, (int)reader.m_captions.size());

	m_ui->listWidgetColumnSelection->setCurrentRow(selectedColumn-1);
	m_ui->listWidgetColumnSelection->selectionModel()->blockSignals(false);

	// we now have a column selected, trigger update of diagram
	on_listWidgetColumnSelection_currentItemChanged(m_ui->listWidgetColumnSelection->currentItem(), nullptr);
}


void SVTimeSeriesPreviewDialog::updateAnnualDataDiagram() const{
	FUNCID(SVDBScheduleEditWidget::updateAnnualDataDiagram);
	// We compose a NANDRAD::LinearSplineParameter and afterwards update the diagram.
	// This function is called when we have at least valid meta-data.
	if (m_data.m_tsvFile.isValid()) {
		// tsv-file variant
		// get absolute path
		IBK::Path fname(IBK::Path(m_ui->filepathDataFile->filename().toStdString()));

		NANDRAD::LinearSplineParameter spl;
		spl.m_tsvFile = fname;
		try {
			IBK::IBK_Message(IBK::FormatString("Reading '%1'\n").arg(fname), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			spl.readTsv();
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			m_ui->widgetTimeSeriesPreview->setErrorMessage(tr("Error reading data file."));
			return;
		}
		m_ui->widgetTimeSeriesPreview->setData(spl, m_xTitle, m_yTitle);
	}
	else {
		m_ui->widgetTimeSeriesPreview->setErrorMessage(tr("No data, yet."));
		return;
	}
}



void SVTimeSeriesPreviewDialog::on_listWidgetColumnSelection_currentItemChanged(QListWidgetItem *current, QListWidgetItem */*previous*/) {
	if (current == nullptr)
		return; // do nothing

	// get column index
	int currentListItem = current->data(Qt::UserRole).toInt();
	if (currentListItem < 0) {
		// invalid unit in data column, cannot use this column
		m_ui->widgetTimeSeriesPreview->setErrorMessage(tr("Invalid/missing unit in column header of data file."));
		return;
	}
	QString unitName = current->data(Qt::UserRole+1).toString();
	m_yTitle = QString("%1 [%2]").arg(current->data(Qt::UserRole+2).toString()).arg(unitName).toStdString();

	// add suffix to file name
	IBK::Path fname(IBK::Path(m_ui->filepathDataFile->filename().toStdString()));
	IBK::Path adjustedFileName;
	int number;
	IBK::extract_number_suffix(fname, adjustedFileName, number);
	QString extendedFilename = QString("%1?%2")
			.arg(QString::fromStdString(adjustedFileName.str()))
			.arg(currentListItem);
	m_ui->filepathDataFile->setFilename( extendedFilename );
	on_radioButtonRelativeFilePath_toggled(m_ui->radioButtonRelativeFilePath->isChecked());

	updateAnnualDataDiagram();
}


void SVTimeSeriesPreviewDialog::on_radioButtonInterpolationLinear_toggled(bool checked) {
	if (checked)
		m_data.m_interpolationMethod = NANDRAD::LinearSplineParameter::I_LINEAR;
	else
		m_data.m_interpolationMethod = NANDRAD::LinearSplineParameter::I_CONSTANT;
}

