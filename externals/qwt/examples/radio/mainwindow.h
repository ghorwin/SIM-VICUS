#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <qwidget.h>

class MainWindow : public QWidget
{
public:
    MainWindow();

protected:
    virtual void resizeEvent( QResizeEvent * );

private:
    void updateGradient();
};

#endif
