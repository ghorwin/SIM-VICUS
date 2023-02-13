#ifndef SVPROPRESULTSVIEWWIDGETH
#define SVPROPRESULTSVIEWWIDGETH

#include <QWidget>
#include <QDir>

#include "NANDRAD_LinearSplineParameter.h"

namespace Ui {
class SVPropResultsWidget;
}

class ModificationInfo;

class SVPropResultsWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVPropResultsWidget(QWidget *parent = nullptr);
	~SVPropResultsWidget();

	void clearAll();

private slots:

	void onModified(int modificationType, ModificationInfo *data);

	void readResultsDir();

	void onTimeSliderCutValueChanged(double currentTime);

	void on_pushButtonMaxColor_clicked();

	void on_pushButtonMinColor_clicked();

	void on_pushButtonSetGlobalMinMax_clicked();

	void on_pushButtonSetLocalMinMax_clicked();

	void on_tableWidgetAvailableResults_cellDoubleClicked(int row, int column);

	void on_toolButtonSetDefaultDirectory_clicked();

	void on_toolButtonUpdateAvailableOutputs_clicked();

	void on_pushButton_clicked();

	void on_lineEditMaxValue_editingFinishedSuccessfully();

	void on_lineEditMinValue_editingFinishedSuccessfully();

private:

	void readCurrentResult(bool forceToRead);

	void setCurrentMinMaxValues(bool localMinMax=false);

	void interpolateColor(const double &val, QColor &col);

	void updateColors(const double & currentTime);

	Ui::SVPropResultsWidget		*m_ui;

	QDir											m_resultsDir;

	std::map<QString, std::vector<unsigned int>>	m_availableOutputs;
	std::map<QString, QString>						m_availableOutputUnits;
	std::map<QString, QString>						m_outputFile;

	std::map<QString, unsigned int>					m_objectName2Id;

	std::map<QString, std::map<unsigned int, NANDRAD::LinearSplineParameter> >	m_allResults;

	QString											m_currentOutput;

	double											m_currentMin = 0;
	double											m_currentMax = 1;
	QColor											m_minColor;
	QColor											m_maxColor;
};

#endif // SVRESULTSVIEWWIDGETH
