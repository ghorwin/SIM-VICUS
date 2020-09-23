#ifndef QUOTE_FACTORY_H
#define QUOTE_FACTORY_H

#include <qwt_series_data.h>

class QuoteFactory
{
public:
    enum Stock
    {
        BMW,
        Daimler,
        Porsche,

        NumStocks
    };

    static QVector<QwtOHLCSample> samples2010( Stock );
    static QString title( Stock );
};

#endif
