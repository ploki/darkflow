#ifndef SLIDER_H
#define SLIDER_H

#include <QDialog>

namespace Ui {
class Slider;
}

class Slider : public QDialog
{
    Q_OBJECT
public:
    typedef enum {
        Value,
        Percent,
        ExposureValue,
        Magnitude
    } Unit;
    typedef enum {
        Linear,
        Logarithmic
    } Scale;
    explicit Slider(const QString& windowCaption,
                    Unit unit,
                    Scale scale,
                    qreal min,
                    qreal max,
                    qreal value,
                    bool hardMin,
                    bool hardMax,
                    QWidget *parent = 0);
    ~Slider();

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

private:
    Ui::Slider *ui;
    Unit m_unit;
    Scale m_scale;
    qreal m_min;
    qreal m_max;
    qreal m_value;
    bool m_hardMin;
    bool m_hardMax;
};

#endif // SLIDER_H
