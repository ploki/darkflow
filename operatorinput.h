#ifndef OPERATORINPUT_H
#define OPERATORINPUT_H

#include <QString>

class OperatorInput
{
public:
    typedef enum {
        Image,
        Set,
    } OperatorInputCompatibility;

    OperatorInput(const QString& name,
                  const QString& description,
                  OperatorInputCompatibility compatibility);

    QString name() const;

    QString description() const;

    OperatorInputCompatibility compatibility() const;

private:
    QString m_name;
    QString m_description;
    OperatorInputCompatibility m_compatibility;
};

#endif // OPERATORINPUT_H
