#include "plot.h"
#include <qwt_color_map.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qslider.h>
#include <qlabel.h>
#include <qcheckbox.h>

class MainWindow: public QMainWindow
{
public:
    MainWindow( QWidget * = NULL );

private:
    Plot *d_plot;
};

MainWindow::MainWindow( QWidget *parent ):
    QMainWindow( parent )
{
    d_plot = new Plot( this );
    d_plot->setContentsMargins( 0, 5, 0, 10 );

    setCentralWidget( d_plot );

    QToolBar *toolBar = new QToolBar( this );

#ifndef QT_NO_PRINTER
    QToolButton *btnPrint = new QToolButton( toolBar );
    btnPrint->setText( "Print" );
    btnPrint->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( btnPrint );
    connect( btnPrint, SIGNAL( clicked() ),
        d_plot, SLOT( printPlot() ) );

    toolBar->addSeparator();
#endif

    toolBar->addWidget( new QLabel("Color Map " ) );

    QComboBox *mapBox = new QComboBox( toolBar );
    mapBox->addItem( "RGB" );
    mapBox->addItem( "Hue" );
    mapBox->addItem( "Saturation" );
    mapBox->addItem( "Value" );
    mapBox->addItem( "Sat.+Value" );
    mapBox->addItem( "Alpha" );
    mapBox->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    toolBar->addWidget( mapBox );

    connect( mapBox, SIGNAL( currentIndexChanged( int ) ),
             d_plot, SLOT( setColorMap( int ) ) );

    toolBar->addWidget( new QLabel("Table " ) );

    QComboBox *colorTableBox = new QComboBox( toolBar );
    colorTableBox->addItem( "None" );
    colorTableBox->addItem( "256" );
    colorTableBox->addItem( "1024" );
    colorTableBox->addItem( "16384" );
    toolBar->addWidget( colorTableBox );

    connect( colorTableBox, SIGNAL( currentIndexChanged( int ) ),
             d_plot, SLOT( setColorTableSize( int ) ) );

    toolBar->addWidget( new QLabel( " Opacity " ) );
    QSlider *slider = new QSlider( Qt::Horizontal );
    slider->setRange( 0, 255 );
    slider->setValue( 255 );
    connect( slider, SIGNAL( valueChanged( int ) ), 
        d_plot, SLOT( setAlpha( int ) ) );

    toolBar->addWidget( slider );
    toolBar->addWidget( new QLabel("   " ) );

    QCheckBox *btnSpectrogram = new QCheckBox( "Spectrogram", toolBar );
    toolBar->addWidget( btnSpectrogram );
    connect( btnSpectrogram, SIGNAL( toggled( bool ) ),
        d_plot, SLOT( showSpectrogram( bool ) ) );

    QCheckBox *btnContour = new QCheckBox( "Contour", toolBar );
    toolBar->addWidget( btnContour );
    connect( btnContour, SIGNAL( toggled( bool ) ),
        d_plot, SLOT( showContour( bool ) ) );

    addToolBar( toolBar );

    btnSpectrogram->setChecked( true );
    btnContour->setChecked( false );

    connect( d_plot, SIGNAL( rendered( const QString& ) ),
        statusBar(), SLOT( showMessage( const QString& ) ),
        Qt::QueuedConnection );
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    a.setStyle( "Windows" );

    MainWindow mainWindow;
    mainWindow.resize( 600, 400 );
    mainWindow.show();

    return a.exec();
}
