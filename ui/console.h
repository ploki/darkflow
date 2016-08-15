/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
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
