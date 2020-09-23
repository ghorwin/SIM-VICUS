#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <qmainwindow.h>

class Plot;
class Panel;
class QLabel;
class Settings;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget *parent = NULL );
    virtual bool eventFilter( QObject *, QEvent * );

private Q_SLOTS:
    void applySettings( const Settings & );

private:
    Plot *d_plot;
    Panel *d_panel;
    QLabel *d_frameCount;
};

#endif
