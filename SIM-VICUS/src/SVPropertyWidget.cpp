#include "SVPropertyWidget.h"

#include "SVGeometryView.h"

SVPropertyWidget::SVPropertyWidget(QWidget * parent) :
	QWidget(parent)
{

}

void SVPropertyWidget::setMode(int m) {
	switch ((SVGeometryView::GeometryEditMode)m) {
		case SVGeometryView::NUM_M :
		default : {
			// set maximum size to 0, to avoid widget being expandible
		}
	}
}
