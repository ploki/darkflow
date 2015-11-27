#ifndef OPLOADVIDEO_H
#define OPLOADVIDEO_H

#include "operator.h"
#include <QObject>

class OperatorParameterFilesCollection;
class Process;

class OpLoadVideo : public Operator
{
    Q_OBJECT
public:
    OpLoadVideo(Process *process);
    ~OpLoadVideo();
    OpLoadVideo *newInstance();
    OperatorWorker *newWorker();

    QStringList getCollection() const;

public slots:
    void filesCollectionChanged();

private:
    OperatorParameterFilesCollection *m_filesCollection;
};

#endif // OPLOADVIDEO_H
