#ifndef OPERATORPATHTHROUGH_H
#define OPERATORPATHTHROUGH_H

#include "operator.h"
#include <QObject>

class OperatorPathThrough : public Operator
{
    Q_OBJECT
public:
    OperatorPathThrough(Process *parent);
    ~OperatorPathThrough();
    OperatorPathThrough *newInstance();
    QString getClassIdentifier();

signals:

protected:
    Image *process(const Image *);

};

#endif // OPERATORPATHTHROUGH_H
