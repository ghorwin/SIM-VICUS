# QwtPlot exportieren und drucken

This section discusses various aspects related to exporting a QwtPlot to images
(or image files) and printing charts with good quality to different sources.

## The basics

The process of drawing the actual QwtPlot is called *rendering* the plot. There are
several different functions available for that, which we discuss later.

When a QwtPlot is rendered on a raster-based paint device (see QPaintDevice), the
result will be a pixel buffer (e.g. image/pixmap), which can be stored, for example,
in PNG files.

When the plot is rendered on a vector paint device (PDF, SVG, ...) the result will
be a vector graphic and it depends on the viewer (or printer) to show/print it. As
a vector graphic, all items are drawn within some coordinate system which can
represent either real distances (mm or inch) or pixels or anything. Conversion
between these coordinate systems is typically done by some factor.

## Resolution

A typical way to relate between real distances to pixels/points is the resolution,
expressed as DotsPerInch (DPI). Printers have typically a resolution of 600 DPI or
1200 DPI. The resolution of a monitor depends on the screen resolution
(e.g. FullHD 1920x1080) and the monitor size, with typical values between 72 and
120 DPI. Above that (for 4K laptop screens) such displays are sometimes termed
HighDPI displays.

It is important to keep this resolution in mind, because viewers of the plot have
different expectations when whatching a plot on the monitor or on paper or in
a PDF. Here, you can zoom in to see details (same as using magnifying glass for the
printed plot).

The equation for converting from distance in mm to pixel is:

    resolution / 25.4 mm/inch = pixel / mm
    sizePixel = sizeMM * resolution / 25.4

For example, on a monitor with resolution of 85 DPI a single pixel has a width of:

    1 pixel * 25.4 / 85 = 0.3 mm

## Line widths and the 'cosmetic' property

For example, if you have a 15'' screen with full HD resolution, you may want to
use exactly 1 pixel-wide lines for the plot grid or the curves. Such a single
pixel line then has about 0.2...0.3 mm width (see above) - good to see without
too much effort.

Using the same screen size but with 4k display, a single-pixel line may be already
too thin to see. Still, 1 pixel wide lines are typically good for drawing on
computer screens. Since drawing single-pixel lines is very common, Qt such
lines can be drawn using a QPen with pen width of 0 (Note: in previous Qt versions
this was the default, since Qt 5 the default is 1 pixel wide). A pen with 0 width
is also termed a *cosmetic* pen, meaning that regardless on the resolution of the
paint device, always only 1 pixel is used for drawing.

However, the cosmetic property can be also set for pens with other widths:

```c++
    QPen::setCosmetic(bool)
```

Generally, all lines with cosmetic pens shall be drawn with exactly the
specified number of pixels, regardless of target paint device resolution and
possible **transformations**.

## Transformations and their impact on plot items

When rendering the plot to a painting device, the plot items are drawn with
coordinates which are transformed to the target coordinate system. Part of this
transformation is also the scaling of plot item properties, such as font sizes
and line widths. Normally, all plot items are drawn scaled to the coordinate
system (and resolution) of the target paint device. The scale factor is
determined by various factors, as we will see below.

Only cosmetic pens are exempt from the scaling. This may lead to surprising
results, when a cosmetic pen with width of 1 is rendered onto a 1200 DPI
printer page. Indeed, the printer will only print a single dot as line width,
leading to a tiny and hard to see line. Apparently, the cosmetic pen size
should somehow be aligned with the target resolution to get an expected
physical width. Different options on how to do this are shown below.

## Exporting bitmaps

Rendering a QwtPlot onto a bitmap is fairly easy. However, there are three
different options to consider:

- hardcopy of the plot as shown on the screen
- rendering on different pixel size yet same scaling
- rendering on different pixel size yet keep appearance and simply scale chart

### Bitmap hardcopy

Basically, the same as a screenshot, good for copying the QwtPlot to the
clipboard, simply use:

```c++
    pixmap = plot->grab();              // Qt 5 or newer
    pixmap = QPixmap::grabWidget(plot); // Qt 4
```

### Bitmap with different size yet same scaling

This is basically the same as resizing a QwtPlot widget - there will be
more space for the coordinate axes, there will be more canvas area for the plot
items and more space for labels and text.

Use:

```c++
  QwtPlotRenderer renderer; // the renderer
	QRect imageRect( 0.0, 0.0, imageWidth, imageHeight ); // target size in pixels
	QImage image( imageRect.size(), QImage::Format_ARGB32 );
	image.fill( Qt::white ); // fill with uniform background color, usually white

	QPainter painter( &image );
	renderer.render( plot, &painter, imageRect );
	painter.end();
```

For this variant, all pens and fonts will remain exactly the same as in the
QwtPlot widget.

### Export bitmap with proportional scaling

This type of export can be thought of as method to shrink/expand the current
plot to a new size. Major difference to the method above is that the proportions
within the chart stay the same. For example, the ratio of grid lines distance to
label text lengths and curve distances stays (approximately) the same.

Use:

```c++
    QwtPlotRenderer renderer; // the renderer

	QRect imageRect( 0.0, 0.0, imageWidth, imageHeight ); // target size in pixels
	QImage image( imageRect.size(), QImage::Format_ARGB32 );

	int resolution = 96; // in DPI, 24'' Monitor with 1920x1080 screen resolution
	const double mmToInch = 1.0 / 25.4;
	const int dotsPerMeter = qRound( resolution * mmToInch * 1000.0 );
	image.setDotsPerMeterX( dotsPerMeter );
	image.setDotsPerMeterY( dotsPerMeter );

	image.fill( Qt::white ); // fill with uniform background color, usually white

	QPainter painter( &image );
	renderer.render( plot, &painter, imageRect );
	painter.end();
```

It is now possible to generate a bitmap with the same dimensions, yet different
level of detail, by adjusting the resolution. _Higher resolution_ means effectively
_more pixels per item_, e.g larger fonts, larger line width etc.

> **Note:** The scaling properties (cosmetic!) have already an impact. When curve pens
and grid pens have a cosmetic pen assigned (e.g. 1 or 2 pixels), they will be drawn
with exactly this amount of pixels onto the bitmap.

To summarize: _when exporting bitmaps, use cosmetic pens_

### Bitmap export for screen presentation

When exporting bitmaps for use in a presentation and the resolution is
known (e.g. 1024x786 on old projectors), typically the task is to generate
a plot with a given maximum resolution (to fit on screen), without any
scaling in the presentation software (pixel = pixel) when the presentation
is shown on screen, and yet enough detail of the plot.

Suppose your plot is configured to look nicely
in the QwtPlot widget with a resolution of 1980x1080 pixels (full HD).
In your presentation you have only about 600x450 pixels. When exporting,
select the target size as image size and adjust the DPI value until you have a good
compromise between level of detail and size of fonts in the diagram. Thanks
to the cosmetic pens (if used), the lines and grid will always be drawn
nicely with good contrast and without blurring.

## Exporting vector images/printing

Exporting a vector image or printing a plot works essentially the same. The
plot is simply rendered on a vector-based paint device. Such a paint device has
properties such as size and resolution, something
that is, for example, provided by a QPrinter.

### Expectations on plot results/differences to screen view

When exporting a plot to a vector graphic, some expectations can be formulated:

1. when looking at the vector graphic in a viewer (e.g. in a PDF viewer) it
  should look similar to the plot in the QwtPlot widget

2. when zooming in, there should be more details visible; for example, when the
  original 100% view shows high frequency oscillations in the diagram such that
  many lines are drawn next to/over each other, zooming in into the vector graphic
  should reveal more details

3. when _looking_ at a PDF/Vector graphic in a viewer, some lines (grid,
   coordinates and label ticks) should be drawn with 1-pixel-wide lines, yet
   when _printing_ the plot, the same lines should be drawn with
   a meaningful thickness (e.g. 0.2 mm thick lines), so that they appear
   nicely on the page

4. exporting a plot with 40x30 mm size or 80x60 mm size should result in the
   same line ticknesses to be used (e.g. curve line thickness should be 0.6 mm
   as selected)

Apparently, there is no single correct solution and different options have to be
used depending on the desired scenario.

> **Note:** A general misconception may be, that a vector graphic output format
can be used to emulate the zooming features of QwtPlot. Instead, the general
purpose of a vector plot is to _maintain the general appearance_ (incl. line
thicknesses) regardless of target resolution. For example, when exploiting
a PDF viewer to zoom into a chart, one may _not_ expect the same functionality as
zooming into a QwtPlot, i.e. more samples being drawn with line widths kept
the same regardless of zoom level (see discussion of cosmetic property treatment
below).

### Export formats

#### PDF specifics

A common understanding of the PDF requirement is, that zooming into the PDF
scales _all elements_ in correct
proportions. Hence, if a plot shown at 100% zoom hides some details (because curve
line thickness is too large), zooming in will not help at all, since curve line
thicknesses scale just the same.

For cosmetic pens the property has only an impact on the _initial
composition_ of the vector graphic. A viewer will nonetheless scale line width
during zoom.

Tricky is the situation for PDFs with cosmetic, 1-pixel wide lines (pen-width = 0).
For viewing PDFs in a pdf viewer this may be quite neat, yet when printing such
a PDF with 600 DPI or higher, the extremely thin lines will almost vanish. So when
exporting, one should distinguish between "screen viewing" and "printing", which
may be quite confusing to the user of the PDF.

Therefore, for _generally_ usable PDFs, it is recommended to _avoid cosmetic 1-pixel
lines (i.e. pen-width = 0) in PDFs_, altogether.

> **Note:** for Qt 5.10 and probably other versions as well there appears to be a bug
that prevents the pdf paint engine of Qt to consider the cosmetic property when
a solid line with full opaque color is plotted with pen width != 0. Workaround
is to set the alpha value of the curve pen to 254. However, there is generally
a better way to handle line widths, see below. Do not use cosmetic pens for
PDF export.

#### SVG features

SVG files (at least with SVG version 1.2 or newer) have a property for path elements
`vector-effect="non-scaling-stroke"`

In a viewer that supports this feature (and with certain additional requirements:
viewport attribute must exist, width and height attribute must _not_ exist),
the cosmetic lines will be shown indeed with constant width regardless of the zoom
level. However, this feature is far from being universally supported. At the
current state, using cosmetic pens will not even work for SVG export.

**Do not rely on the cosmetic property to work when exporting SVG images.**

#### PS/EPS export

Postscript/Encapsulated Postscript export has been removed from Qt with version 5,
so there is no paint device available anylonger that generates this. However,
using a postscript printer driver and storing the result to file will
generally work (see printing section below).

#### EMF export

EMF export is important for Windows users, but relies on translating
Qt paint engine drawing commands into Windows drawing API commands. To export
the plot to emf format (either cliboard or *.emf file), a third-party
library (e.g. EmfLibary) is needed that generates EMF commands from
paint device commands.

> **Note:** It is important that the EMF library implements a paint device, that is
recognized by its type as a vector-based paint device (greater or equal to
User-type). QwtPlot distinguishes between raster and vector based paint devices,
so if characteristic steps appear in lines within the EMF output, it is likely
that the paint device type is reported/recognized wrong.

### Target paint device resolution

The target paint device resolution is usually given. A printer will report
the resolution selected by the printer driver. For PDF or SVG export, the resolution
can be arbitrarily selected. However, it should be selected such, that enough
points remain to render a plot without data loss. For example, if 10000 samples/data
points are equally spaced over the canvas area, the resulting width of
generated plot (=total number of points) should be well above 10000,
if chart should be plotted without data loss. If the line thickness is selected
fine enough, zooming into the vector graphic will eventually yield

### Adjusting line width

When configuring a QwtPlot for viewing a diagram in the widget, the curve pen widths
are typically selected based on pixels on the screen. For FullHD displays, pen widths
of 1-3 pixels are common, for 3K and 4K displays (HighDPI displays), pen widths up
to 6 pixels are used.

However, when printing charts, typical pen widths should in the magnitude
of 0.2 ... 1 mm (where 1 mm is already a fairly thick line). Marker symbols are often
drawn with less than 0.3 mm thick lines.

There are generally two approaches on how to achieve this:

1. select export DPI such, that the desired mm widths are achieved based on the current pen widths already used for the widget (requires _non-cosmetic_ pens to be used)

2. modify pen widths of all plot items prior to printing to get the desired width (this works with cosmetic and non-cosmetic pens, though the calculation of the
actual pen width to use will be different)

### Adjusting DPI

It is important to note that a change of the DPI settings will affect not only
line width but also font heights.

The required DPI can be calculated based on the equation shown above in
section [Print Resolution](#print-resolution).


\subsubsection exportingVectorLineWidthPenAdjustment Adjusting pen width

The advantage of this approach is, that the overall appearance of the vector image
looks very close to that rendered on the QwtPlot widget.

Suppose the resolution (in DPI) of the target device is known and a given, i.e.
the plot should be rendered with exactly this resolution. This avoids print-time
scaling and may give best quality (though I wonder if someone can really see
the difference between a 300 DPI plot upscaled to 1200 DPI or a natively
rendered 1200 DPI plot. The effect on antialiasing will be minor, but maybe
worth a try).


Again, we rearrage the equation from section \ref printingResolution.




## Additional topics


    renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales, true);


### Sample reduction

...

*/
