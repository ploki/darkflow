#ifndef OPERATORPARAMETERSLIDER_H
#define OPERATORPARAMETERSLIDER_H

#include "operatorparameter.h"
#include "slider.h"

class OperatorParameterSlider : public OperatorParameter
{
    Q_OBJECT
public:
    OperatorParameterSlider(
            const QString &name,
            const QString &caption,
            const QString &windowCaption,
            Slider::Unit unit,
            Slider::Scale scale,
            Slider::NumberSet numberSet,
            qreal min,
            qreal max,
            qreal value,
            qreal hardMin,
            qreal hardMax,
            uint parametersFilter=Slider::FilterAll,
            QObject *parent = 0);

    QString windowCaption() const;
    Slider::Unit unit() const;
    Slider::Scale scale() const;
    Slider::NumberSet numberSet() const;
    qreal min() const;
    qreal max() const;
    qreal value() const;
    qreal hardMin() const;
    qreal hardMax() const;
    uint parametersFilter() const;
    QString currentValue() const;

    QJsonObject save();
    void load(const QJsonObject &obj);

public slots:
    void setUnit(const Slider::Unit &unit);
    void setScale(const Slider::Scale &scale);
    void setMin(const qreal &min);
    void setMax(const qreal &max);
    void setValue(const qreal &value);

signals:
    void updated();

private:
    QString m_windowCaption;
    Slider::Unit m_unit;
    Slider::Scale m_scale;
    Slider::NumberSet m_numberSet;
    qreal m_min;
    qreal m_max;
    qreal m_value;
    qreal m_hardMin;
    qreal m_hardMax;
    uint m_parametersFilter;
};

#endif // OPERATORPARAMETERSLIDER_H
