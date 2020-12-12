#ifndef SVSIMULATIONSTARTNANDRAD_H
#define SVSIMULATIONSTARTNANDRAD_H

#include <QDialog>

namespace Ui {
class SVSimulationStartNandrad;
}

class SVSimulationStartNandrad : public QDialog
{
	Q_OBJECT

public:
	explicit SVSimulationStartNandrad(QWidget *parent = 0);
	~SVSimulationStartNandrad();

private:
	Ui::SVSimulationStartNandrad *ui;
};

#endif // SVSIMULATIONSTARTNANDRAD_H
