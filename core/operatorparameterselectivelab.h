#ifndef OPERATORPARAMETERSELECTIVELAB_H
#define OPERATORPARAMETERSELECTIVELAB_H

#include "operatorparameter.h"
#include "selectivelab.h"

class OperatorParameterSelectiveLab : public OperatorParameter
{
    Q_OBJECT
public:
    OperatorParameterSelectiveLab(
            const QString &name,
            const QString &caption,
            const QString &windowCaption,
            int hue,
            int coverage,
            bool strict,
            int level,
            bool displayGuide,
            bool previewEffect,
            Operator *op);

    QJsonObject save();
    void load(const QJsonObject &obj);

    QString currentValue() const;

    QString windowCaption() const;
    void setWindowCaption(const QString &windowCaption);

    int hue() const;
    void setHue(int hue);

    int coverage() const;
    void setCoverage(int coverage);

    bool strict() const;
    void setStrict(bool strict);

    int level() const;
    void setLevel(int level);

    bool displayGuide() const;
    void setDisplayGuide(bool displayGuide);

    bool previewEffect() const;
    void setPreviewEffect(bool previewEffect);

signals:
    void updated();
private:
    QString m_windowCaption;
    int m_hue;
    int m_coverage;
    bool m_strict;
    int m_level;
    bool m_displayGuide;
    bool m_previewEffect;

};

#endif // OPERATORPARAMETERSELECTIVELAB_H
