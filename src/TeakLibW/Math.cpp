#include "stdafx.h"

void memswap(void* dst, void* src, ULONG size)
{
    for (unsigned int i = size; i > 0; --i)
        Swap(((BYTE*)dst)[i - 1], ((BYTE*)src)[i - 1]);
}

int CalcInertiaVelocity(int a, int b)
{
    if ( a <= b )
    {
        if ( a >= b )
            return 0;
        else
            return -(int)sqrt((double)(b - a + 1) / 2.0 + 0.5);
    }
    else
    {
        return (int)sqrt((double)(a - b + 1) / 2.0 + 0.5);
    }
}

int Calc1nSum(int a)
{
    return a * (a + 1) / 2;
}

double GetFrameRate()
{
    TeakLibW_Exception(0, 0, ExcNotImplemented);
    return 42.0;
}
