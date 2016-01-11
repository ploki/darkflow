#ifndef POSIXSPAWN_H
#define POSIXSPAWN_H
#include "ports.h"
#ifndef DF_WINDOWS
#include <QIODevice>

class PosixSpawnImpl;

class PosixSpawn : public QIODevice
{
public:
    PosixSpawn();
    ~PosixSpawn();
    int exitCode();
    QByteArray readAllStandardOutput();
    bool	waitForFinished(int msecs = 30000);
    bool	waitForStarted(int msecs = 30000);
    void	start(const QString & program, const QStringList & arguments, OpenMode mode = ReadWrite);

protected:
    virtual qint64	readData(char * data, qint64 maxSize);
    virtual qint64	writeData(const char * data, qint64 maxSize);

private:
    PosixSpawnImpl *impl;
};
#endif // !DF_WINDOWS
#endif // POSIXSPAWN_H
