#include "ports.h"
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
