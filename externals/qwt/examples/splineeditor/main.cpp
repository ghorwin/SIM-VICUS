#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include "plot.h"

class ToolButton: public QToolButton
{
public:
    ToolButton( const char *text, QToolBar *toolBar ):
        QToolButton( toolBar )
    {
        setText( text );
        setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    }
};

class PlotTab: public QMainWindow
{
public:
    PlotTab( bool parametric,  QWidget *parent ):
        QMainWindow( parent )
    {
        Plot *plot = new Plot( parametric, this );
        setCentralWidget( plot );
    
        QToolBar *toolBar = new QToolBar( this );

#ifndef QT_NO_PRINTER
        ToolButton *btnPrint = new ToolButton( "Print", toolBar );
        toolBar->addWidget( btnPrint );
        QObject::connect( btnPrint, SIGNAL( clicked() ),
            plot, SLOT( printPlot() ) );
#endif
    
        ToolButton *btnOverlay = new ToolButton( "Overlay", toolBar );
        btnOverlay->setCheckable( true );
        toolBar->addWidget( btnOverlay );
        QObject::connect( btnOverlay, SIGNAL( toggled( bool ) ),
            plot, SLOT( setOverlaying( bool ) ) );
    
        if ( parametric )
        {
            QComboBox *parameterBox = new QComboBox( toolBar );

            parameterBox->addItem( "Uniform" );
            parameterBox->addItem( "Centripetral" );
            parameterBox->addItem( "Chordal" );
            parameterBox->addItem( "Manhattan" );
            toolBar->addWidget( parameterBox );
            connect( parameterBox, SIGNAL( activated( const QString & ) ),
                plot, SLOT( setParametric( const QString & ) ) );

            parameterBox->setCurrentIndex( 2 ); // chordal
            plot->setParametric( parameterBox->currentText() );

            ToolButton *btnClosed = new ToolButton( "Closed", toolBar );
            btnClosed->setCheckable( true );
            toolBar->addWidget( btnClosed );
            QObject::connect( btnClosed, SIGNAL( toggled( bool ) ),
                plot, SLOT( setClosed( bool ) ) );
        }

        QComboBox *boundaryBox = new QComboBox( toolBar );

        boundaryBox->addItem( "Natural" );
        boundaryBox->addItem( "Linear Runout" );
        boundaryBox->addItem( "Parabolic Runout" );
        boundaryBox->addItem( "Cubic Runout" );
        boundaryBox->addItem( "Not a Knot" );

        toolBar->addWidget( boundaryBox );
        connect( boundaryBox, SIGNAL( activated( const QString & ) ),
            plot, SLOT( setBoundaryCondition( const QString & ) ) );
    
        addToolBar( toolBar );
    }
};

int main ( int argc, char **argv )
{
    QApplication a( argc, argv );
    a.setStyle( "Windows" );

    QTabWidget w;
    w.addTab( new PlotTab( false, &w ), "Non Parametric" );
    w.addTab( new PlotTab( true, &w ), "Parametric" );

    const QSize sz = 0.6 * QApplication::desktop()->size();
    w.resize( sz.boundedTo( QSize( 800, 600 ) ) );
    w.show();

    return a.exec();
}
