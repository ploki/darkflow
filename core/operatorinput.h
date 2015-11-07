#ifndef OPERATORINPUT_H
#define OPERATORINPUT_H

#include <QObject>
#include <QString>
#include <QSet>

class Operator;
class OperatorOutput;

class OperatorInput : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        Image,
        Set,
    } OperatorInputCompatibility;

    OperatorInput(const QString& name,
                  const QString& description,
                  OperatorInputCompatibility compatibility,
                  Operator *parent);

    QString name() const;

    QString description() const;

    OperatorInputCompatibility compatibility() const;

    QSet<OperatorOutput *> sources() const;
    void addSource(OperatorOutput *output);
    void removeSource(OperatorOutput *output);
public:
    Operator *m_operator;
private:
    QString m_name;
    QString m_description;
    OperatorInputCompatibility m_compatibility;
    QSet<OperatorOutput*> m_sources;
};

#endif // OPERATORINPUT_H
