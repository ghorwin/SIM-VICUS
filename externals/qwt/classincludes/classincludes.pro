################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

TEMPLATE          = subdirs

QWT_ROOT = $${PWD}/..
include( $${QWT_ROOT}/qwtconfig.pri )
include( $${QWT_ROOT}/qwtbuild.pri )
include( $${QWT_ROOT}/qwtfunctions.pri )

CLASSHEADERS = \
    QwtAbstractScaleDraw \
    QwtAlphaColorMap \
    QwtClipper \
    QwtColorMap \
    QwtColumnRect \
    QwtColumnSymbol \
    QwtDate \
    QwtDateScaleDraw \
    QwtDateScaleEngine \
    QwtDynGridLayout \
    QwtGraphic \
    QwtHueColorMap \
    QwtInterval \
    QwtIntervalSymbol \
    QwtLinearColorMap \
    QwtLinearScaleEngine \
    QwtLogScaleEngine \
    QwtLogTransform \
    QwtMagnifier \
    QwtNullPaintDevice \
    QwtNullTransform \
    QwtPainter \
    QwtPainterCommand \
    QwtPanner \
    QwtPicker \
    QwtPickerClickPointMachine \
    QwtPickerClickRectMachine \
    QwtPickerDragLineMachine \
    QwtPickerDragPointMachine \
    QwtPickerDragRectMachine \
    QwtPickerMachine \
    QwtPickerPolygonMachine \
    QwtPickerTrackerMachine \
    QwtPixelMatrix \
    QwtPlainTextEngine \
    QwtPoint3D \
    QwtPointPolar \
    QwtPowerTransform \
    QwtRichTextEngine \
    QwtRoundScaleDraw \
    QwtSaturationValueColorMap \
    QwtScaleArithmetic \
    QwtScaleDiv \
    QwtScaleDraw \
    QwtScaleEngine \
    QwtScaleMap \
    QwtSimpleCompassRose \
    QwtSplineBasis \
    QwtSpline \
    QwtSplineC1 \
    QwtSplineC2 \
    QwtSplineCubic \
    QwtSplineG1 \
    QwtSplineInterpolating \
    QwtSplineLocal \
    QwtSplineParameter \
    QwtSplinePleasing \
    QwtSplinePolynomial \
    QwtSymbol \
    QwtSystemClock \
    QwtText \
    QwtTextEngine \
    QwtTextLabel \
    QwtTransform \
    QwtWidgetOverlay

contains(QWT_CONFIG, QwtPlot) {

    CLASSHEADERS += \
        QwtAbstractLegend \
        QwtCurveFitter \
        QwtEventPattern \
        QwtIntervalSample \
        QwtLegend \
        QwtLegendData \
        QwtLegendLabel \
        QwtPointMapper \
        QwtMatrixRasterData \
        QwtOHLCSample \
        QwtPlot \
        QwtPlotAbstractBarChart \
        QwtPlotBarChart \
        QwtPlotCanvas \
        QwtPlotCurve \
        QwtPlotDict \
        QwtPlotDirectPainter \
        QwtPlotGrid \
        QwtPlotHistogram \
        QwtPlotIntervalCurve \
        QwtPlotItem \
        QwtPlotLayout \
        QwtPlotLegendItem \
        QwtPlotMagnifier \
        QwtPlotMarker \
        QwtPlotMultiBarChart \
        QwtPlotPanner \
        QwtPlotPicker \
        QwtPlotRasterItem \
        QwtPlotRenderer \
        QwtPlotRescaler \
        QwtPlotScaleItem \
        QwtPlotSeriesItem \
        QwtPlotShapeItem \
        QwtPlotSpectroCurve \
        QwtPlotSpectrogram \
        QwtPlotSvgItem \
        QwtPlotTextLabel \
        QwtPlotTradingCurve \
        QwtPlotVectorField \
        QwtPlotZoneItem \
        QwtPlotZoomer \
        QwtScaleWidget \
        QwtRasterData \
        QwtSetSample \
        QwtSamplingThread \
        QwtSplineCurveFitter \
        QwtWeedingCurveFitter \
        QwtIntervalSeriesData \
        QwtPoint3DSeriesData \
        QwtPointSeriesData \
        QwtSetSeriesData \
        QwtSyntheticPointData \
        QwtPointArrayData \
        QwtTradingChartData \
        QwtCPointerData
}

contains(QWT_CONFIG, QwtOpenGL) {

    CLASSHEADERS += \
        QwtPlotAbstractCanvas \
        QwtPlotGLCanvas

    greaterThan(QT_MAJOR_VERSION, 4) {

        greaterThan(QT_MINOR_VERSION, 3) {

            CLASSHEADERS += \
                QwtPlotOpenGLCanvas
        }
    }
}

contains(QWT_CONFIG, QwtWidgets) {

    CLASSHEADERS += \
        QwtAbstractSlider \
        QwtAbstractScale \
        QwtArrowButton \
        QwtAnalogClock \
        QwtCompass \
        QwtCompassMagnetNeedle \
        QwtCompassRose \
        QwtCompassScaleDraw \
        QwtCompassWindArrow \
        QwtCounter \
        QwtDial \
        QwtDialNeedle \
        QwtDialSimpleNeedle \
        QwtKnob \
        QwtSlider \
        QwtThermo \
        QwtWheel
}

    
target.path    = $${QWT_INSTALL_HEADERS}
target.files   = $${CLASSHEADERS}
INSTALLS       = target

