#ifndef SVPROPRESULTSVIEWWIDGETH
#define SVPROPRESULTSVIEWWIDGETH

#include <QWidget>
#include <QDir>

#include "NANDRAD_LinearSplineParameter.h"

namespace Ui {
class SVPropResultsWidget;
}

class SVPropResultsWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVPropResultsWidget(QWidget *parent = nullptr);
	~SVPropResultsWidget();

	void clearAll();

private slots:

	void onModified(int modificationType);

	void on_pushButtonSetDefaultDirectory_clicked();

	void updateTableWidgetAvailableOutputs();

	void on_pushButtonUpdateAvailableOutputs_clicked();

	void on_pushButtonReadResults_clicked();

	void onTimeSliderCutValueChanged(double currentTime);

	void on_doubleSpinBoxMaxValue_valueChanged(double arg1);

	void on_doubleSpinBoxMinValue_valueChanged(double arg1);

	void on_pushButtonMaxColor_clicked();

	void on_pushButtonMinColor_clicked();

	void on_pushButtonSetGlobalMinMax_clicked();

	void on_pushButtonSetLocalMinMax_clicked();

	void on_tableWidgetAvailableResults_cellDoubleClicked(int row, int column);

private:

	void setCurrentMinMaxValues(bool localMinMax=false);

	void calculateColor(const double &val, QColor &col);

	void updateColors(const double & currentTime);

	Ui::SVPropResultsWidget		*m_ui;

	QDir											m_resultsDir;

	std::map<QString, std::vector<unsigned int>>	m_availableOutputs;
	std::map<QString, QString>						m_availableOutputUnits;
	std::map<QString, QString>						m_outputFile;

	std::map<QString, unsigned int>					m_objectName2Id;

	std::map<std::string, std::map<unsigned int, NANDRAD::LinearSplineParameter> >	m_allResults;

	std::map<unsigned int, NANDRAD::LinearSplineParameter> 	m_currentResults;

	double											m_currentMin;
	double											m_currentMax;
	QColor											m_minColor;
	QColor											m_maxColor;
};

#endif // SVRESULTSVIEWWIDGETH
