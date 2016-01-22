#ifndef SLIDER_H
#define SLIDER_H

#include <QDialog>

#define MAGNITUDE_POW (2.512L)

namespace Ui {
class Slider;
}

class QAbstractButton;

class Slider : public QDialog
{
    Q_OBJECT
public:
    typedef enum {
        Value         = (1<<0),
        Percent       = (1<<1),
        ExposureValue = (1<<2),
        Magnitude     = (1<<3)
    } Unit;
    typedef enum {
        Linear        = (1<<4),
        Logarithmic   = (1<<5)
    } Scale;
    typedef enum {
        Integer,
        Real
    } NumberSet;
    enum {
        FilterNothing = 0,
        FilterAll = -1,
        FilterExposureFromOne = Logarithmic|ExposureValue|Magnitude|Percent,
        FilterExposure = Logarithmic|ExposureValue|Magnitude,
        FilterPixels = Linear|Value,
        FilterPercent = Linear|Percent
    };
    explicit Slider(const QString& windowCaption,
                    Unit unit,
                    Scale scale,
                    NumberSet numberSet,
                    qreal min,
                    qreal max,
                    qreal value,
                    qreal hardMin,
                    qreal hardMax,
                    uint parametersFilter= FilterAll,
                    QWidget *parent = 0);
    ~Slider();

    Unit getUnit() const;

    Scale getScale() const;

    qreal getMin() const;

    qreal getMax() const;

    qreal getValue() const;

    QString currentValue() const;

public slots:
    void selectValue(bool);
    void selectPercent(bool);
    void selectExposureValue(bool);
    void selectMagnitude(bool);
    void selectLinear(bool);
    void selectLogarithmic(bool);

    void changeMin(double);
    void changeMax(double);
    void changeValue(double);

    void sliderChanged();

    void loadValues(Unit unit,
                    Scale scale,
                    qreal min,
                    qreal max,
                    qreal value);

    static const char* unitToString(Unit unit);
    static QString unitToLocalizedString(Unit unit);
    static Unit unitFromString(const QString& unit);
    static QString scaleToString(Scale scale);
    static Scale scaleFromString(const QString& scale);

signals:
    void updated();


private:
    Ui::Slider *ui;
    Unit m_unit;
    Scale m_scale;
    NumberSet m_numberSet;
    qreal m_min;
    qreal m_max;
    qreal m_value;
    qreal m_hardMin;
    qreal m_hardMax;
    qreal m_range;
    int m_pvDecimals;

    void setSlider();
    double getSlider();

    void updateInputs();

    qreal fromValue_(qreal v);
    qreal fromPercent_(qreal v);
    qreal fromExposureValue_(qreal v);
    qreal fromMagnitude_(qreal v);
    qreal toValue_(qreal v);
    qreal toPercent_(qreal v);
    qreal toExposureValue_(qreal v);
    qreal toMagnitude_(qreal v);

    qreal fromUnit(qreal v);
    qreal toUnit(qreal v);
    void setUnit(Slider::Unit u);
    void clickUnit(Slider::Unit u);
    void clickScale(Slider::Scale s);
};

#endif // SLIDER_H
