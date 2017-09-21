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
#ifndef PORTS_H
#define PORTS_H

#include <QJsonValue>

#if defined(Q_OS_OSX) && !defined(_OPENMP) &&  !defined(DFL_USE_GCD)
# define DFL_USE_GCD 1
#endif

#if defined(DFL_USE_GCD)
#include <dispatch/dispatch.h>
#endif

#if defined(WIN32) || defined(WIN64)
# if defined(WIN64)
#  define DF_ARCH "64-bit"
# else
#  define DF_ARCH "32-bit"
#endif
# include <cstdio>
# include <Windows.h>
# define DF_WINDOWS
typedef unsigned int u_int32_t;
typedef unsigned long long u_int64_t;
typedef long long int64_t;
#endif

#ifdef __GNUC__
# include <signal.h>
# if defined(__LP64__)
#  define DF_ARCH "64-bit"
# else
#  define DF_ARCH "32-bit"
# endif
# define DF_PRINTF_FORMAT(x,y) __attribute__((format(printf,x,y)))
# define DF_TRAP() do { ::raise(SIGTRAP); } while(0)
# define atomic_incr(ptr) do { __sync_fetch_and_add ((ptr), 1); } while(0)
# define atomic_decr(ptr) do { __sync_fetch_and_add ((ptr), -1); } while(0)

#else /* not GCC */

# define DF_PRINTF_FORMAT(x,y)
# define DF_TRAP() __debugbreak()
# define atomic_incr(ptr) do { InterlockedIncrement ((ptr)); } while(0)
# define atomic_decr(ptr) do { InterlockedDecrement ((ptr)); } while(0)
#endif /* __GNUC__ */

# ifndef M_PI
#  define M_PI 3.141592654L
# endif

# ifndef M_SQRT1_2l
#  define M_SQRT1_2l 0.70710678L
# endif

# ifndef M_SQRT2l
#  define M_SQRT2l 1.4142136L
# endif

# ifndef M_LN2
#  define M_LN2 0.69314718L
# endif

#if defined(ANDROID)
# define log2(x) (log(x)*(1.L/M_LN2))
#endif

#ifdef DF_WINDOWS
extern "C" {
int vasprintf(char **strp, const char *fmt, va_list ap);
}
#define snprintf(buffer, size,fmt,...) \
    _snprintf_s(buffer, size, _TRUNCATE, fmt, __VA_ARGS__)
#endif


#define DF_EQUALS(x, y, epsilon) (fabs((x)-(y)) < (epsilon))
#define DF_ROUND(x) ((x) + 0.5)

#include "ordinary.h"

void init_platform();

#endif // PORTS_H
