#ifndef OPERATORPARAMETER_H
#define OPERATORPARAMETER_H

#include <QObject>

class OperatorParameter : public QObject
{
    Q_OBJECT
public:
    explicit OperatorParameter(QObject *parent = 0);

signals:
    void parameterChanged();

public slots:
};

#endif // OPERATORPARAMETER_H
