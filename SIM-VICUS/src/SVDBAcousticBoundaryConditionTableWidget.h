#ifndef SVDBACOUSTICBOUNDARYCONDITIONTABLEWIDGET_H
#define SVDBACOUSTICBOUNDARYCONDITIONTABLEWIDGET_H

#include <QTableWidget>

class SVDBAcousticBoundaryConditionTableWidget : public QTableWidget
{
	Q_OBJECT
public:
	explicit SVDBAcousticBoundaryConditionTableWidget(QWidget *parent = 0);

	/* checks if column size ColName is lower than allowed, sets column then to minimum size
	*	resizes widget inside column immediately to fit the new size. Necessary because otherwise the widget would only be resized
	*	when the mouse button is released*/
	void onSectionResized();

	/* adds new widget with button and text, connects to onLayerChosen slot in SVDBAcousticBoundaryConditionEditWidget */
	void setButtonAndText(int row, int column, QString text);

	void setButton(int row, int column);

signals:


};

#endif // SVDBACOUSTICBOUNDARYCONDITIONTABLEWIDGET_H
