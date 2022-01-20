#include "StdAfx.h"

void memswap(void *dst, void *src, ULONG size) {
    for (unsigned int i = size; i > 0; --i) {
        Swap((static_cast<BYTE *>(dst))[i - 1], (static_cast<BYTE *>(src))[i - 1]);
    }
}

SLONG CalcInertiaVelocity(SLONG a, SLONG b) {
    if (a <= b) {
        if (a >= b) {
            return 0;
        }
        return -static_cast<SLONG>(sqrt(static_cast<double>(b - a + 1) / 2.0 + 0.5));
    }

    return static_cast<SLONG>(sqrt(static_cast<double>(a - b + 1) / 2.0 + 0.5));
}

SLONG Calc1nSum(SLONG a) { return a * (a + 1) / 2; }

double GetFrameRate() {
    TeakLibW_Exception(nullptr, 0, ExcNotImplemented);
    return 42.0;
}
