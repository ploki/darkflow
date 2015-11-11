#ifndef RAWCONVERT_H
#define RAWCONVERT_H
#include <QString>
#include "operatorworker.h"

class OperatorLoadRaw;
class Photo;

class RawConvert : public OperatorWorker
{
    Q_OBJECT
public:
    RawConvert(QThread *thread, OperatorLoadRaw *op);

    Photo process(const Photo &, int, int) { throw 0; }

private slots:
    void play();

private:
    QByteArray convert(const QString& filename);
    void setTags(const QString& filename, Photo& photo);

signals:

public slots:
private:
    OperatorLoadRaw *m_loadraw;
};

#endif // RAWCONVERT_H
