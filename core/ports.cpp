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
#include <QFile>
#include <QStandardPaths>
#include "ports.h"
#include "console.h"
#include "preferences.h"
#include <cstdlib>

#ifdef DF_WINDOWS
int vasprintf(char **res, char const *fmt, va_list args)
{
    int	sz, r;
        sz = _vscprintf(fmt, args);

#if (defined(__STDC__) && __STDC__ >= 199901L) \
    || (defined(_XOPEN_VERSION) && (_XOPEN_VERSION >= 600))
        if (sz < 0)
                return sz;
        if (sz >= 0) {
#else
        if (sz >= 1) {
#endif
                if ((*res = (char*)malloc(sz + 1)) == NULL)
                        return -1;

                if ((sz = vsprintf(*res, fmt, args)) < 0) {
                        free(*res);
                        *res = NULL;
                }

                return sz;
        }

#define MAXLN 65535
        *res = NULL;
        for (sz = 128; sz <= MAXLN; sz *= 2) {
                if ((*res = (char*)realloc(*res, sz)) == NULL)
                        return -1;
                r = vsnprintf(*res, sz, fmt, args);
                if (r > 0 && r < sz)
                        return r;
        }

        if (*res) {
                free(*res);
                *res = NULL;
        }

        return -1;
}
#endif

#if defined(Q_OS_OSX)
#include <QDir>
#include <QApplication>

static void
init_osx()
{
    QString path = getenv("PATH")?:"";
    QDir dir(QApplication::applicationDirPath());
    if(!path.isEmpty())
        path += ":";
    path += dir.absolutePath();
    setenv("PATH", path.toLocal8Bit(), 1);

    if ( !dir.cdUp() ) return;
    if ( !dir.cd("Library/ImageMagick/modules-Q16/coders/") ) return;
    setenv("MAGICK_CODER_MODULE_PATH", dir.absolutePath().toLocal8Bit().data(), 1);

    if ( !dir.cdUp() ) return;
    if ( !dir.cd("filters/") ) return;
    setenv("MAGICK_CODER_FILTER_PATH", dir.absolutePath().toLocal8Bit().data(), 1);

    if ( !dir.cdUp() ) return;
    if ( !dir.cdUp() ) return;
    if ( !dir.cd("config-Q16/") ) return;
    setenv("MAGICK_CONFIGURE_PATH", dir.absolutePath().toLocal8Bit().data(), 1);
}

#endif

static void
install_imagemagick_policy()
{
    QString configurationDirectory = Preferences::getAppConfigLocation();
    QString destFilename = configurationDirectory + "/policy.xml";
    QFile destFile(destFilename);
    if (!destFile.exists()) {
        if (!QFile::copy(":/setup/policy.xml", destFilename)) {
            dflError("Could not create ImageMagick policy.xml file");
            return;
        }
    }
    setenv("MAGICK_CONFIGURE_PATH", configurationDirectory.toLocal8Bit().data(), 1);
}

void init_platform()
{
    install_imagemagick_policy();
#if defined(Q_OS_OSX)
    init_osx();
#endif
}
