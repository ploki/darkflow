#ifndef CONSOLE_H
#define CONSOLE_H

#include "ports.h"
#include <QMainWindow>

namespace Ui {
class Console;
}

class Console : public QMainWindow
{
    Q_OBJECT
public:
    typedef enum {
        Debug,
        Info,
        Warning,
        Error,
        Critical,
        LastLevel
    } Level;
    static void init();
    static void fini();
    static void show();
    static void close();
    static Level getLevel();
    static void setLevel(Level level);
    static void setTrapLevel(Level level);
    static void setRaiseLevel(Level level);
    static void trap(Level level);

private slots:
    void recvMessage(Level level, QString message);

signals:
    void message(Level level, QString message);

private:
    Level m_level;
    Level m_raiseLevel;
    Level m_trapLevel;
    explicit Console(QWidget *parent = 0);
    Ui::Console *ui;
    ~Console();
};

#define DF_NULL_PIXELS Console::tr("Could not get pixels from cache, memory exhausted?")

void dflMessage(Console::Level level, char *fmt, ...) DF_PRINTF_FORMAT(2,3);
void dflDebug(const char* fmt, ...) DF_PRINTF_FORMAT(1,2);
void dflInfo(const char* fmt, ...) DF_PRINTF_FORMAT(1,2);
void dflWarning(const char* fmt, ...) DF_PRINTF_FORMAT(1,2);
void dflError(const char* fmt, ...) DF_PRINTF_FORMAT(1,2);
void dflCritical(const char* fmt, ...) DF_PRINTF_FORMAT(1,2);

void dflMessage(Console::Level, const QString& msg);
void dflDebug(const QString& msg);
void dflInfo(const QString& msg);
void dflWarning(const QString& msg);
void dflError(const QString& msg);
void dflCritical(const QString& msg);

#endif // CONSOLE_H
