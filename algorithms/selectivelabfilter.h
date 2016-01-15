#ifndef SELECTIVELABFILTER_H
#define SELECTIVELABFILTER_H

#include <QObject>
#include "algorithm.h"
#include "photo.h"

class SelectiveLabFilter : public Algorithm
{
    Q_OBJECT
public:
    explicit SelectiveLabFilter(int hue,
                                int coverage,
                                qreal saturation,
                                bool strict,
                                qreal exposure,
                                bool insideSelection,
                                bool exposureStrict,
                                QObject *parent = 0);

    void applyOnImage(Magick::Image& image, bool hdr);

private:
    int m_hue;
    int m_coverage;
    qreal m_saturation;
    bool m_strict;
    qreal m_exposure;
    bool m_insideSelection;
    bool m_exposureStrict;
};

#endif // SELECTIVELABFILTER_H
