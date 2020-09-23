#ifndef COMPASS_GRID
#define COMPASS_GRID

#include <qframe.h>

class QwtCompass;

class CompassGrid: public QFrame
{
public:
    CompassGrid( QWidget *parent = NULL );

private:
    QwtCompass *createCompass( int pos );
};

#endif
