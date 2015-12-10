#ifndef OPLOADVIDEO_H
#define OPLOADVIDEO_H

#include "operator.h"
#include <QObject>

class OperatorParameterFilesCollection;
class OperatorParameterSlider;
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
    int getSkip() const;
    int getCount() const;

public slots:
    void filesCollectionChanged();

private:
    OperatorParameterFilesCollection *m_filesCollection;
    OperatorParameterSlider *m_skip;
    OperatorParameterSlider *m_count;
};

#endif // OPLOADVIDEO_H
