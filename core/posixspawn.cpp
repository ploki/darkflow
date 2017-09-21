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
#include "ports.h"
#if !defined(DF_WINDOWS) && !defined(ANDROID)
#include "posixspawn.h"
#include "console.h"
#include <unistd.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <cstring>

#if defined(Q_OS_OSX)
#define __environ environ
  extern char **environ;
#define POSIX_SPAWN_USEVFORK 0
#endif

class PosixSpawnImpl {
    friend class PosixSpawn;
    PosixSpawnImpl() :
        pid(-1),
        channels(),
        error(false),
        started(false),
        rc(0)
    {
        channels[0] = channels[1] = -1;
    }
    ~PosixSpawnImpl() {
        if ( channels[0] ) ::close(channels[0]);
        if ( channels[1] ) ::close(channels[1]);
    }
    pid_t pid;
    int channels[2];
    bool error;
    bool started;
    int rc;
};

PosixSpawn::PosixSpawn() :
    QIODevice(),
    impl(new PosixSpawnImpl)
{
}

PosixSpawn::~PosixSpawn()
{
    delete impl;
}

int PosixSpawn::exitCode()
{
    return impl->rc;
}

QByteArray PosixSpawn::readAllStandardOutput()
{
    QByteArray array;
    if ( impl->error || !impl->started ) {
        dflError("read in error");
        return array;
    }
    char buf[4096];
    int ret;
    for ( ret = readData(buf, sizeof buf) ;
          ret > 0 ;
          ret = readData(buf, sizeof buf) ) {
        array += QByteArray(buf, ret);
    }
    if ( ret < 0 )
        impl->error=true;
    dflDebug(tr("Return array of size %0").arg(array.count()));
    return array;
}

bool PosixSpawn::waitForFinished(int)
{
    if ( impl->error || !impl->started )
        return false;
    pid_t w;
    int status;
    do {
        w = waitpid(impl->pid, &status, WUNTRACED | WCONTINUED);
        if ( w < 0 ) {
            dflError("waitpid() failed, errno=%s", strerror(errno));
            impl->error = true;
            return false;
        }
    } while( !WIFEXITED(status) && !WIFSIGNALED(status));
    impl->rc = WEXITSTATUS(status);
    if ( impl->rc ) {
        dflError(tr("Child process failed with exit code %0").arg(impl->rc));
    }
    return true;
}

bool PosixSpawn::waitForStarted(int )
{
    if (impl->error || !impl->started) {
        dflError(tr("Child process not started"));
        return false;
    }
    return true;
}

void PosixSpawn::start(const QString &program,
                       const QStringList &arguments,
                       QIODevice::OpenMode mode)
{
    int rc;
    char *file = strdup(program.toLocal8Bit().data());
    size_t sz = arguments.count()+2;
    char * argv[sz];
    memset(argv, 0, sizeof(*argv)*sz);
    posix_spawn_file_actions_t file_actions;
    posix_spawnattr_t attr;
    posix_spawn_file_actions_init(&file_actions);
    posix_spawnattr_init(&attr);
#ifndef Q_OS_OSX
    /* os X doesn't support posix spawn with vfork flag
     * I hope it's implementation is still based on it
     * since the syscall exists
     */
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_USEVFORK);
#endif
    int peer[2] = { -1, -1 };

    if ( mode & QIODevice::ReadOnly) {
        int p[2];
        rc = pipe(p);
        if ( rc < 0 ) {
            dflError("pipe(read) failed, errno=%s", strerror(errno));
            impl->error=true;
            return;
        }
        impl->channels[0] = p[0];
        rc = posix_spawn_file_actions_adddup2(&file_actions,
                                              p[1],
                                              STDOUT_FILENO);
        peer[1] = p[1];
        if ( rc < 0 ) {
            dflError("posix_spawn_file_actions_adddup2(stdout) failed, errno=%s", strerror(errno));
            impl->error=true;
            return;
        }
    }
    else {
        rc = posix_spawn_file_actions_addopen(&file_actions,
                                              STDOUT_FILENO,
                                              "/dev/null",
                                              O_WRONLY, 0);
        if ( rc < 0 ) {
            dflError("posix_spawn_file_actions_addopen(stdout) failed, errno=%s", strerror(errno));
            impl->error=true;
            return;
        }
    }
    if ( mode & QIODevice::WriteOnly) {
        int p[2];
        rc = pipe(p);
        if ( rc < 0 ) {
            dflError("pipe(write) failed, errno=%s", strerror(errno));
            impl->error=true;
            return;
        }
        impl->channels[1] = p[1];
        rc = posix_spawn_file_actions_adddup2(&file_actions,
                                              p[0],
                                              STDIN_FILENO);
        peer[0] = p[0];
        if ( rc < 0 ) {
            dflError("posix_spawn_file_actions_adddup2(stdin) failed, errno=%s", strerror(errno));
            impl->error=true;
            return;
        }
    }
    else {
        rc = posix_spawn_file_actions_addopen(&file_actions,
                                              STDIN_FILENO,
                                              "/dev/null",
                                              O_RDONLY, 0);
        if ( rc < 0 ) {
            dflError("posix_spawn_file_actions_addopen(stdin) failed, errno=%s", strerror(errno));
            impl->error=true;
            free(file);
            return;
        }
    }

    QVector<QString> args = arguments.toVector();

    argv[0] = strdup(file);
    for ( int i = 0 ; i < args.count() ; ++i ) {
        argv[i+1] = strdup(args[i].toLocal8Bit().data());
    }
    argv[args.count()+1] = 0;

    QString debugString = "cmd:";
    for ( char **p = argv ; *p ; ++p ) {
        debugString += " ";
        debugString += *p ;
    }
    dflDebug(debugString);
    debugString = "environ:";
    for ( char **p = __environ ; *p ; ++p ) {
        debugString += " ";
        debugString += *p ;
    }
    dflDebug(debugString);

    rc = posix_spawnp(&impl->pid, file,
                      &file_actions,
                      &attr,
                      (char*const*)argv,
                      __environ);
    for ( char **p = argv ; *p ; ++p )
        free(*p);
    free(file);

    if ( rc < 0 ) {
        dflError("posix_spawnp failed, errno=%s", strerror(errno));
        impl->error=true;
        return;
    }
    impl->started=true;
    posix_spawnattr_destroy(&attr);
    posix_spawn_file_actions_destroy(&file_actions);

    if ( peer[0] ) ::close(peer[0]);
    if ( peer[1] ) ::close(peer[1]);
    dflDebug(tr("PosixSpawn Success"));
    open(mode);
}

qint64 PosixSpawn::readData(char *data, qint64 maxSize)
{
    if (impl->channels[0] < 0 ) {
        dflError(tr("Read on closed file"));
        return -1;
    }
    qint64 ret = ::read(impl->channels[0], data, maxSize);
    return ret;
}

qint64 PosixSpawn::writeData(const char *data, qint64 maxSize)
{
    if ( impl->channels[1] < 0 )
        return -1;
    return ::write(impl->channels[1], data, maxSize);
}
#endif
