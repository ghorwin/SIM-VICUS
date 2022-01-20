#ifndef SVMEASUREMENTDATAVIEW_H
#define SVMEASUREMENTDATAVIEW_H

#include <QWidget>

namespace Ui {
class SVMeasurementDataView;
}

class SVMeasurementDataView : public QWidget
{
	Q_OBJECT

public:
	explicit SVMeasurementDataView(QWidget *parent = nullptr);
	~SVMeasurementDataView();

private:
	Ui::SVMeasurementDataView *ui;
};

#endif // SVMEASUREMENTDATAVIEW_H
