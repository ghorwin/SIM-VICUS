#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <qevent.h>
#include <qmainwindow.h>

class FormulaView;
class TreeView;

class QCheckBox;
class QComboBox;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private Q_SLOTS:
    void load();
    void loadFormula( const QString & );
    void updateFontSize( const QString & );
    void updateTransformation( const bool & );
    void updateScaling( const bool & );
    void updateRotation( const QString & );
    void updateDrawFrames( const bool & );
    void updateColors( const bool & );

private:
    FormulaView *d_formulaView;
    TreeView *d_treeView;

    QCheckBox *d_checkScale;
    QComboBox *d_comboRotations;
};

#endif
