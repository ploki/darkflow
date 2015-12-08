#ifndef RAWCONVERT_H
#define RAWCONVERT_H
#include <QString>
#include "operatorworker.h"

class OpLoadRaw;
class Photo;

class WorkerLoadRaw : public OperatorWorker
{
    Q_OBJECT
public:
    WorkerLoadRaw(QThread *thread, OpLoadRaw *op);

    Photo process(const Photo &, int, int) { throw 0; }

private slots:
    void play();

private:
    QByteArray convert(const QString& filename);
    void setTags(const QString& filename, Photo& photo);

signals:

public slots:
private:
    OpLoadRaw *m_loadraw;
};

#endif // RAWCONVERT_H
