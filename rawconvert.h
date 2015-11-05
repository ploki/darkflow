#ifndef RAWCONVERT_H
#define RAWCONVERT_H
#include <QString>
#include "operatorworker.h"

class OperatorLoadRaw;

class RawConvert : public OperatorWorker
{
    Q_OBJECT
public:
    RawConvert(QThread *thread, OperatorLoadRaw *op);

private slots:
    void play();

    Image *process(const Image *image) { Q_UNUSED(image); return NULL; }
private:
    QByteArray convert(const QString& filename);

signals:

public slots:
private:
    OperatorLoadRaw *m_loadraw;
};

#endif // RAWCONVERT_H
