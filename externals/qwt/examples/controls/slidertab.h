#ifndef SLIDER_TAB_H
#define SLIDER_TAB_H

#include <qwidget.h>

class QBoxLayout;

class SliderTab: public QWidget
{
public:
    SliderTab( QWidget *parent = NULL );

private:
    QBoxLayout *createLayout( Qt::Orientation,
        QWidget *widget = NULL );
};

#endif
