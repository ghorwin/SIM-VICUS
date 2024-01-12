#include "SVNetworkSimultaneityDialog.h"
#include "ui_SVNetworkSimultaneityDialog.h"

#include "SVChartUtils.h"
#include "SVStyle.h"

#include <IBK_CSVReader.h>

#include <QtExt_Locale.h>
#include <QClipboard>
#include <QMimeData>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

SVNetworkSimultaneityDialog::SVNetworkSimultaneityDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkSimultaneityDialog)
{
	m_ui->setupUi(this);

	setWindowTitle(tr("Edit simultaneity"));

	configureChart(m_ui->widgetPlot);

	// table widget
	m_ui->tableWidget->blockSignals(true);
	m_ui->tableWidget->setColumnCount(2);
	m_ui->tableWidget->verticalHeader()->setVisible(false);
	m_ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Number of buildings [-]")));
	m_ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Simultaneity [-]")));
		SVStyle::formatDatabaseTableView(m_ui->tableWidget);
	m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_ui->tableWidget->setSortingEnabled(false);
	m_ui->tableWidget->blockSignals(false);
}

SVNetworkSimultaneityDialog::~SVNetworkSimultaneityDialog()
{
	delete m_ui;
}

void SVNetworkSimultaneityDialog::edit(IBK::LinearSpline & simultaneity) {

	m_tmpSimultaneity = simultaneity;

	updateTableWidget();

	on_tableWidget_itemSelectionChanged();

	if (exec() == QDialog::Accepted)
		simultaneity = m_tmpSimultaneity;
}

void SVNetworkSimultaneityDialog::updatePlot() {

	m_ui->widgetPlot->detachItems( QwtPlotItem::Rtti_PlotCurve );
	m_ui->widgetPlot->detachItems( QwtPlotItem::Rtti_PlotMarker );
	m_ui->widgetPlot->replot();
	m_ui->widgetPlot->setEnabled(false);

	// now do all the plotting
	m_ui->widgetPlot->setEnabled(true);

	m_curve = addConfiguredCurve(m_ui->widgetPlot);
	// adjust styling based on current theme's settings
	configureCurveTheme(m_curve);

	// heating curve
	m_curve->setRawSamples(m_tmpSimultaneity.x().data(), m_tmpSimultaneity.y().data(), (int)m_tmpSimultaneity.size());

	QFont ft;
	ft.setPointSize(10);
	QwtText xl(tr("Number of buildings [-]"));
	xl.setFont(ft);
	m_ui->widgetPlot->setAxisTitle(QwtPlot::xBottom, xl);
	QwtText yl(tr("Simultaneity [-]"));
	yl.setFont(ft);
	m_ui->widgetPlot->setAxisTitle(QwtPlot::yLeft, yl);
	m_ui->widgetPlot->replot();
}


void SVNetworkSimultaneityDialog::updateTableWidget() {
	// populate table widget with properties
	m_ui->tableWidget->blockSignals(true);
	m_ui->tableWidget->clearContents();
	m_ui->tableWidget->setRowCount((int)m_tmpSimultaneity.size());

	for (unsigned int i=0; i<m_tmpSimultaneity.size(); ++i) {
		QTableWidgetItem * item = new QTableWidgetItem(QString("%L1").arg(m_tmpSimultaneity.x()[i]));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		m_ui->tableWidget->setItem((int)i, 0, item);
		QTableWidgetItem * item2 = new QTableWidgetItem(QString("%L1").arg(m_tmpSimultaneity.y()[i]));
		item2->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		m_ui->tableWidget->setItem((int)i, 1, item2);
	}
	m_ui->tableWidget->blockSignals(false);

	updatePlot();
}


void SVNetworkSimultaneityDialog::modifyValues() {

	std::vector<double> x, y;
	bool ok = false;
	for (int row=0; row<m_ui->tableWidget->rowCount(); ++row) {
		QTableWidgetItem *item = m_ui->tableWidget->item(row, 0);
		int xVal = QtExt::Locale().toInt(item->text(), &ok);
		if (ok)
			x.push_back(xVal);
		else break;
		QTableWidgetItem *item2 = m_ui->tableWidget->item(row, 1);
		double yVal = QtExt::Locale().toDoubleWithFallback(item2->text(), &ok);
		if (ok)
			y.push_back(yVal);
		else break;
	}

	// we need to check if the spline can be set
	if (ok) {
		bool splineOk = true;
		try {
			IBK::LinearSpline spl;
			spl.setValues(x,y);
		} catch (...) {
			QMessageBox::critical(this, tr("Input error"), tr("Number of buildings must be monotonic increasing."));
			updateTableWidget();
			return;
		}

		m_tmpSimultaneity.setValues(x, y);
		updatePlot();
	}
}


void SVNetworkSimultaneityDialog::on_toolButtonAddPoint_clicked() {

	m_ui->tableWidget->blockSignals(true);

	int rows = m_ui->tableWidget->rowCount();


	m_ui->tableWidget->setRowCount(rows+1);
	QTableWidgetItem * item = new QTableWidgetItem(QString("%1").arg(m_tmpSimultaneity.x().back() + 5));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	m_ui->tableWidget->setItem(rows,0,item);
	QTableWidgetItem * item2 = new QTableWidgetItem(QString("%L1").arg(m_tmpSimultaneity.y().back()*0.9));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	m_ui->tableWidget->setItem(rows,1,item2);

	m_ui->tableWidget->blockSignals(false);

	modifyValues();

	on_tableWidget_itemSelectionChanged();
}


void SVNetworkSimultaneityDialog::on_toolButtonRemovePoint_clicked() {
	QList<QTableWidgetSelectionRange> ranges = m_ui->tableWidget->selectedRanges();
	if (ranges.empty())
		return;

	if (m_ui->tableWidget->rowCount() > 2) {
		QList<QTableWidgetSelectionRange> ranges = m_ui->tableWidget->selectedRanges();
		std::vector<double> x,y;
		for (int row=0; row < (int)m_tmpSimultaneity.size(); ++row){
			bool skip = false;
			for (const QTableWidgetSelectionRange &range : ranges) {
				if (row >= range.topRow() && row <= range.bottomRow())
					skip = true;
			}
			if (!skip) {
				x.push_back( m_tmpSimultaneity.x()[(unsigned int)row]);
				y.push_back( m_tmpSimultaneity.y()[(unsigned int)row]);
			}
		}
		m_tmpSimultaneity.setValues(x, y);
		updateTableWidget();

		on_tableWidget_itemSelectionChanged();
//		if (ranges[0].bottomRow() < m_ui->tableWidget->rowCount())
//			m_ui->tableWidget->selectRow(ranges[0].bottomRow());
//		else
//			m_ui->tableWidget->selectRow(m_ui->tableWidget->rowCount() - 1);
	}
}


void SVNetworkSimultaneityDialog::on_tableWidget_cellChanged(int row, int column) {

	QTableWidgetItem *item = m_ui->tableWidget->item(row, column);
	QString text = item->text();
	bool ok = false;

	// check number
	if (!text.isEmpty()) {
		if (column==0) {
			unsigned int val = QtExt::Locale().toUInt(text, &ok);
			if (!ok)
				val = text.toUInt(&ok);
			ok = val > 0;
			if (ok)
				item->setText(QString("%1").arg(val));
			else
				QMessageBox::critical(this, QString(), tr("Wrong input in cell at row %1. Only integers > 0 allowed.").arg(row));
		}
		else {
			double val = QtExt::Locale().toDoubleWithFallback(text, &ok);
			if (!ok)
				val = text.toDouble(&ok);
			// simultaneity must be >0 and <=1
			ok = val>0 && val<=1;
			if (ok)
				item->setText(QString("%L1").arg(val));
			else
				QMessageBox::critical(this, QString(), tr("Wrong input in cell at row %1. Only values >0 and <=1 allowed.").arg(row));
		}
	}

	if (ok)
		modifyValues();
	else
		updateTableWidget(); 	// set value back
}

void SVNetworkSimultaneityDialog::on_tableWidget_itemSelectionChanged() {
	bool enable = m_ui->tableWidget->selectedItems().size()/2 < m_tmpSimultaneity.size() - 1
				  && !m_ui->tableWidget->selectedItems().empty()
				  && m_ui->tableWidget->rowCount() > 1;
	m_ui->toolButtonRemovePoint->setEnabled(enable);
}


void SVNetworkSimultaneityDialog::on_pushButtonSetDefault_clicked() {
	int res = QMessageBox::question(this, tr("Set default"), tr("Delete current values and set to default?"));
	if (res == QMessageBox::Yes) {
		VICUS::Network::setDefaultSimultaneity(m_tmpSimultaneity);
		updateTableWidget();
	}
}


void SVNetworkSimultaneityDialog::on_pushButtonCopyToClipboard_clicked()
{
	QClipboard *clipboard = QApplication::clipboard();
	QString table = "";
	for(unsigned int i = 0; i < m_tmpSimultaneity.size(); i++){
		QString newData = QString::number(m_tmpSimultaneity.x()[i]) + "	" + QString::number(m_tmpSimultaneity.y()[i]) + "\n";
		table.append(newData);
	}

	QMimeData *mimeData = new QMimeData();
	mimeData->setText(table);
	clipboard->setMimeData(mimeData);

}

void SVNetworkSimultaneityDialog::on_pushButtonPasteFromClipboard_clicked() {
	// get content of clip board
	QString data = qApp->clipboard()->text();
	if (data.isEmpty()) {
		QMessageBox::critical(this, tr("Cannot paste data"), tr("No data on clipboard"));
		return;
	}
	// first replace all , with .; this may also result in header name changes, but using , as part of a column
	// name is bad practice anyway
	std::replace(data.begin(), data.end(), ',', '.');

	// now use the CSV-Reader to read the data into memory
	IBK::CSVReader reader;
	try {
		reader.parse(data.toStdString(), false, true);
		if (reader.m_nColumns != 2) {
			QMessageBox::critical(this, tr("Cannot paste data"), tr("Expected exactly 2 columns of data."));
			return;
		}
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		QMessageBox::critical(this, tr("Cannot paste data"), tr("Invalid format of table. Requires exactly 2 columns"));
		return;
	}

	if (reader.m_nRows == 0) {
		QMessageBox::critical(this, tr("Cannot paste data"), tr("Missing data."));
		return;
	}


	IBK::LinearSpline spl;
	try {
		std::vector<double> x,y;

		//Check if the header contains values
		QString headerColumn0 = QString::fromStdString(reader.m_captions[0]);
		QString headerColumn1 = QString::fromStdString(reader.m_captions[1]);
		bool headerColumn0IsNumber;
		bool headerColumn1IsNumber;
		double x0 = headerColumn0.toDouble(&headerColumn0IsNumber);
		double y0 = headerColumn1.toDouble(&headerColumn1IsNumber);

		// if header contains values, add them at the beginning
		if(headerColumn0IsNumber && headerColumn1IsNumber){
			x.reserve(reader.m_nRows + 1);
			y.reserve(reader.m_nRows + 1);
			x.push_back(x0);
			y.push_back(y0);
		} else {
			x.reserve(reader.m_nRows);
			y.reserve(reader.m_nRows);
		}
		for (unsigned int i=0; i<reader.m_nRows; ++i) {
			x.push_back(reader.m_values[i][0]);
			y.push_back(reader.m_values[i][1]);
		}

		spl.setValues(x,y);
		if (!spl.valid())
			throw std::runtime_error("...");
	} catch (...) {
		QMessageBox::critical(this, tr("Cannot paste schedule data"), tr("Invalid spline data in table."));
		return;
	}


	// store spline data
	m_tmpSimultaneity = spl;
	updatePlot();
	updateTableWidget();

}
