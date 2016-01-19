#ifndef PORTS_H
#define PORTS_H

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
# if defined(__LP64__)
#  define DF_ARCH "64-bit"
# else
#  define DF_ARCH "32-bit"
#endif
# define DF_PRINTF_FORMAT(x,y) __attribute__((format(printf,x,y)))
# define DF_TRAP() do { __asm__("int3"); } while(0)
# define atomic_incr(ptr) do { __sync_fetch_and_add ((ptr), 1); } while(0)

#else /* not GCC */

# define DF_PRINTF_FORMAT(x,y)
# define DF_TRAP() __debugbreak()
# define atomic_incr(ptr) do { InterlockedIncrement ((ptr)); } while(0)

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
#endif /* __GNUC__ */

#ifdef DF_WINDOWS
extern "C" {
int vasprintf(char **strp, const char *fmt, va_list ap);
}
#define snprintf(buffer, size,fmt,...) \
    _snprintf_s(buffer, size, _TRUNCATE, fmt, __VA_ARGS__)
#endif


#define DF_EQUALS(x, y, epsilon) (fabs((x)-(y)) < (epsilon))
#define DF_ROUND(x) ((x) + 0.5)

#endif // PORTS_H
