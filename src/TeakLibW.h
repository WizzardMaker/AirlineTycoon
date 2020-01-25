#pragma once

extern void memswap(void*, void*, ULONG);
extern char* bprintf(char const*, ...);
extern char* bitoa(int, int = 10);
extern void here(char*, int);

extern const char* ExcAssert;
extern const char* ExcGuardian;
extern const char* ExcImpossible;
extern const char* ExcNotImplemented;
extern const char* ExcOutOfMem;
extern const char* ExcStrangeMem;

#define FNL 0, 0

extern int TeakLibW_Exception(char*, int, const char*, ...);
extern char* TeakStrRemoveCppComment(char*);
extern char* TeakStrRemoveEndingCodes(char*, char const*);
extern unsigned char GerToLower(unsigned char);
extern unsigned char GerToUpper(unsigned char);
extern unsigned char* RecapizalizeString(unsigned char*);
extern const char* GetSuffix(const char*);

template <typename T>
inline void Limit(T min, T& value, T max)
{
   if (value < min) value = min;
   if (value > max) value = max;
}

template <typename T>
inline void Swap(T& a, T& b)
{
   T c(a);
   a = b;
   b = c;
}

inline SLONG min(SLONG a, SLONG b)
{
   return (b < a) ? b : a;
}

inline SLONG max(SLONG a, SLONG b)
{
   return (a < b) ? b : a;
}

template <typename A, typename B>
inline const A Min(const A& a, const B& b)
{
   return (b < a) ? b : a;
}

template <typename A, typename B>
inline const A Max(const A& a, const B& b)
{
   return (a < b) ? b : a;
}

inline void ReferTo(...) {}

inline void MB() {}

template <typename T>
class BUFFER
{
public:
    BUFFER(SLONG anz)
    {
        if (anz == 0)
        {
            MemPointer = DelPointer = NULL;
        }
        else
        {
            MemPointer = DelPointer = new T[anz]();
            if (!MemPointer)
                TeakLibW_Exception(NULL, 0, ExcOutOfMem);
        }

        Size = anz;
    }

    /*BUFFER(BUFFER& rhs)
    {
        ::Swap(MemPointer, rhs.MemPointer);
        ::Swap(DelPointer, rhs.DelPointer);
        ::Swap(Size, rhs.Size);
    }*/

    BUFFER(void) : MemPointer(NULL), DelPointer(NULL), Size(0) {}

    ~BUFFER()
    {
        if (MemPointer)
            delete[] MemPointer;
        MemPointer = NULL;
    }

    void ReSize(SLONG anz)
    {
        if (anz == Size)
            return;

        if (anz > 0)
        {
            T* m = new T[anz]();
            if (!m)
                TeakLibW_Exception(NULL, 0, ExcOutOfMem);

            if (MemPointer)
            {
                SLONG num;
                if (anz < Size)
                    num = anz;
                else
                    num = Size;

                // This is *will* break self-referencing pointers
                // ... please don't resize anything that uses ALBUM
                memswap(m, MemPointer, sizeof(T) * num);
                delete[] MemPointer;

                DelPointer = m + ((DelPointer - MemPointer) / sizeof(T));
            }
            else
            {
                DelPointer = m;
            }
            MemPointer = m;
        }
        else
        {
            if (MemPointer)
                delete[] MemPointer;

            MemPointer = DelPointer = NULL;
        }

        Size = anz;
    }

    void ReSize(SLONG anz, T* memory)
    {
        if (MemPointer)
            delete[] MemPointer;

        DelPointer = &memory[DelPointer - MemPointer];
        Size = anz;
        MemPointer = memory;
    }

    int AnzEntries() const { return Size; }

    void Clear()
    {
        if (Size > 0)
        {
            if (MemPointer)
                delete [] MemPointer;
            MemPointer = DelPointer = NULL;
            Size = 0;
        }
    }

    void FillWith(T value)
    {
        for (int i = 0; i < Size; i++)
            MemPointer[i] = value;
    }

    operator T* () const
    {
        return DelPointer;
    }

    void operator+=(int rhs)
    {
        DelPointer += rhs;
    }

    /*void operator=(BUFFER<T>& rhs)
    {
        ::Swap(MemPointer, rhs.MemPointer);
        ::Swap(DelPointer, rhs.DelPointer);
        ::Swap(Size, rhs.Size);
    }*/

    T* MemPointer;
    T* DelPointer;
    SLONG Size;
};

#define TEAKFILE_READ	1
#define TEAKFILE_WRITE	2

class TEAKFILE
{
public:
    TEAKFILE(void);
    TEAKFILE(char const*, int);
    ~TEAKFILE(void);

    void ReadLine(char*, int);
    int IsEof(void);
    void Close(void);
    int GetFileLength(void);
    int GetPosition(void);
    void Open(char const*, int);
    int IsOpen(void);
    unsigned char* Read(int);
    void Read(unsigned char*, int);
    char* ReadLine(void);
    void ReadTrap(int);
    void WriteTrap(int);
    void SetPasswort(char*);
    void SetPosition(int);
    void Skip(int);
    void Write(unsigned char*, int);
    void WriteLine(char*);
    void Announce(int);

    SDL_RWops* Ctx;
    SLONG Unknown[3];
    char* Path;
    SLONG Unknown1[7];
    BUFFER<UBYTE> MemBuffer;
    int MemPointer;
    ULONG MemBufferUsed;

    friend TEAKFILE& operator << (TEAKFILE& File, const BOOL& b) { File.Write((UBYTE*)& b, sizeof(b)); return File; }
    friend TEAKFILE& operator >> (TEAKFILE& File, BOOL& b) { File.Read((UBYTE*)& b, sizeof(b)); return File; }

    friend TEAKFILE& operator << (TEAKFILE& File, const UBYTE& b) { File.Write((UBYTE*)& b, sizeof(b)); return File; }
    friend TEAKFILE& operator >> (TEAKFILE& File, UBYTE& b) { File.Read((UBYTE*)& b, sizeof(b)); return File; }

    friend TEAKFILE& operator << (TEAKFILE& File, const SBYTE& b) { File.Write((UBYTE*)& b, sizeof(b)); return File; }
    friend TEAKFILE& operator >> (TEAKFILE& File, SBYTE& b) { File.Read((UBYTE*)& b, sizeof(b)); return File; }

    friend TEAKFILE& operator << (TEAKFILE& File, const UWORD& b) { File.Write((UBYTE*)& b, sizeof(b)); return File; }
    friend TEAKFILE& operator >> (TEAKFILE& File, UWORD& b) { File.Read((UBYTE*)& b, sizeof(b)); return File; }

    friend TEAKFILE& operator << (TEAKFILE& File, const SWORD& b) { File.Write((UBYTE*)& b, sizeof(b)); return File; }
    friend TEAKFILE& operator >> (TEAKFILE& File, SWORD& b) { File.Read((UBYTE*)& b, sizeof(b)); return File; }

    friend TEAKFILE& operator << (TEAKFILE& File, const ULONG& b) { File.Write((UBYTE*)& b, sizeof(b)); return File; }
    friend TEAKFILE& operator >> (TEAKFILE& File, ULONG& b) { File.Read((UBYTE*)& b, sizeof(b)); return File; }

    friend TEAKFILE& operator << (TEAKFILE& File, const SLONG& b) { File.Write((UBYTE*)& b, sizeof(b)); return File; }
    friend TEAKFILE& operator >> (TEAKFILE& File, SLONG& b) { File.Read((UBYTE*)& b, sizeof(b)); return File; }

    friend TEAKFILE& operator << (TEAKFILE& File, const __int64& b) { File.Write((UBYTE*)& b, sizeof(b)); return File; }
    friend TEAKFILE& operator >> (TEAKFILE& File, __int64& b) { File.Read((UBYTE*)& b, sizeof(b)); return File; }

    friend TEAKFILE& operator << (TEAKFILE& File, const double& b) { File.Write((UBYTE*)& b, sizeof(b)); return File; }
    friend TEAKFILE& operator >> (TEAKFILE& File, double& b) { File.Read((UBYTE*)& b, sizeof(b)); return File; }

    friend TEAKFILE& operator << (TEAKFILE& File, const float& b) { File.Write((UBYTE*)& b, sizeof(b)); return File; }
    friend TEAKFILE& operator >> (TEAKFILE& File, float& b) { File.Read((UBYTE*)& b, sizeof(b)); return File; }

    friend TEAKFILE& operator << (TEAKFILE& File, const CPoint& b) { File.Write((UBYTE*)& b, sizeof(CPoint)); return (File); }
    friend TEAKFILE& operator >> (TEAKFILE& File, CPoint& b) { File.Read((UBYTE*)& b, sizeof(CPoint)); return (File); }

    friend TEAKFILE& operator << (TEAKFILE& File, const CString& b)
    {
        File << b.GetLength() + 1;
        File.Write((UBYTE*)(PCSTR)b, b.GetLength() + 1);
        return File;
    }

    friend TEAKFILE& operator >> (TEAKFILE& File, CString& b)
    {
        ULONG size;
        File >> size;
        BUFFER<BYTE> str(size);
        File.Read(str, size);
        b = (PCSTR)(BYTE*)str;
        return File;
    }

    template <class T>
    friend class TEAKFILE& operator << (class TEAKFILE& File, const BUFFER<T>& buffer)
    {
        File << buffer.Size;
        File << int(buffer.DelPointer - buffer.MemPointer);
        for (SLONG i = 0; i < buffer.Size; i++)
            File << buffer.MemPointer[i];
        return File;
    }

    template <class T>
    friend class TEAKFILE& operator >> (class TEAKFILE& File, BUFFER<T>& buffer)
    {
        SLONG size, offset;
        File >> size;
        buffer.ReSize(0);
        buffer.ReSize(size);
        File >> offset;
        for (SLONG i = 0; i < buffer.Size; i++)
            File >> buffer.MemPointer[i];
        buffer.DelPointer = buffer.MemPointer + offset;
        return File;
    }

private:
    void CodeBlock(unsigned char*, int, int);
};

//static_assert(sizeof(TEAKFILE) == 68, "TEAKFILE_size_check");

class CRLEReader
{
public:
    CRLEReader(const char* path);
    ~CRLEReader(void);

    bool Close(void);
    bool Buffer(void*, int);
    bool NextSeq(void);
    bool Read(BYTE*, int, bool);

    int GetSize() { return Size; }

private:
    SDL_RWops* Ctx;
    char SeqLength;
    char SeqUsed;
    bool IsSeq;
    BYTE Sequence[132];

    bool IsRLE;
    int Size;
    int Key;
};

template <typename T>
class FBUFFER : public BUFFER<T>
{
public:
    FBUFFER(void) : BUFFER<T>(0) {}

    //FBUFFER(FBUFFER& buffer) : BUFFER<T>(buffer) {}

    FBUFFER(SLONG anz) : BUFFER<T>(anz) {}
};

class TEAKRAND
{
public:
    TEAKRAND(void);
    TEAKRAND(ULONG _Seed);

    void SRand(ULONG _Seed);
    void SRandTime(void);
    void Reset(void);

    UWORD Rand(void);
    UWORD Rand(SLONG Max);
    UWORD Rand(SLONG Min, SLONG Max);
    ULONG GetSeed(void);

    friend TEAKFILE& operator << (TEAKFILE& File, const TEAKRAND& r);
    friend TEAKFILE& operator >> (TEAKFILE& File, TEAKRAND& r);

private:
    ULONG Seed;
    ULONG Value;
};

template <typename T>
class TXY
{
public:
    T x, y;

    TXY() : x(), y() {}
    TXY(T s) : x(s), y(s) {}
    TXY(T x, T y) : x(x), y(y) {}
    TXY(POINT& point) : x(point.x), y(point.y) {}

    TXY operator+(const TXY& b) const
    {
        return TXY(x + b.x, y + b.y);
    }

    TXY operator-(const TXY& b) const
    {
        return TXY(x - b.x, y - b.y);
    }

    TXY operator*(const TXY& b) const
    {
        return TXY(x * b.x, y * b.y);
    }

    TXY operator/(const TXY& b) const
    {
        return TXY(x / b.x, y / b.y);
    }

    TXY operator*(const T& b) const
    {
        return TXY(x * b, y * b);
    }

    TXY operator/(const T& b) const
    {
        return TXY(x / b, y / b);
    }

    TXY operator-() const
    {
        return TXY(-x, -y);
    }

    bool operator==(const TXY& b) const
    {
        return x == b.x
            && y == b.y;
    }

    bool operator!=(const TXY& b) const
    {
        return x != b.x
            || y != b.y;
    }

    bool operator<(const TXY& b) const
    {
        return x < b.x
            && y < b.y;
    }

    bool operator>(const TXY& b) const
    {
        return x > b.x
            && y > b.y;
    }

    TXY& operator-=(const TXY& b)
    {
        x -= b.x;
        y -= b.y;
        return *this;
    }

    TXY& operator+=(const TXY& b)
    {
        x += b.x;
        y += b.y;
        return *this;
    }

    TXY& operator/=(const TXY& b)
    {
        x /= b.x;
        y /= b.y;
        return *this;
    }

    TXY& operator*=(const TXY& b)
    {
        x *= b.x;
        y *= b.y;
        return *this;
    }

    operator CPoint& ()
    {
        return reinterpret_cast<CPoint&>(*this);
    }

    double abs() const
    {
        return sqrt(x * x + y * y);
    }

    double operator*(const double& b) const
    {
        return (x + y) * b;
    }

    double operator/(const double& b) const
    {
        return (x + y) / b;
    }

    bool IfIsWithin(T _x1, T _y1, T _x2, T _y2) const
    {
        return x >= _x1 && y >= _y1 && x <= _x2 && y <= _y2;
    }

    bool IfIsWithin(const TXY<T>& a, const TXY<T>& b) const
    {
        return a < *this && *this < b;
    }

    friend TEAKFILE& operator << (TEAKFILE& File, const TXY<T>& b) { File.Write((UBYTE*)& b, sizeof(T) * 2); return (File); }
    friend TEAKFILE& operator >> (TEAKFILE& File, TXY<T>& b) { File.Read((UBYTE*)& b, sizeof(T) * 2); return (File); }
};

template <typename T>
class TXYZ
{
public:
    T x, y, z;

    TXYZ() : x(), y(), z() {}
    TXYZ(T s) : x(s), y(s), z(s) {}
    TXYZ(T x, T y, T z) : x(x), y(y), z(z) {}

    TXYZ operator+(const TXYZ& b) const
    {
        return TXYZ(x + b.x, y + b.y, z + b.z);
    }

    TXYZ operator-(const TXYZ& b) const
    {
        return TXYZ(x - b.x, y - b.y, z - b.z);
    }

    TXYZ operator*(const TXYZ& b) const
    {
        return TXYZ(x * b.x, y * b.y, z * b.z);
    }

    TXYZ operator/(const TXYZ& b) const
    {
        return TXYZ(x / b.x, y / b.y, z / b.z);
    }

    TXYZ operator*(const T& b) const
    {
        return TXYZ(x * b, y * b, z * b);
    }

    TXYZ operator/(const T& b) const
    {
        return TXYZ(x / b, y / b, z / b);
    }

    TXYZ operator-() const
    {
        return TXYZ(-x, -y, -z);
    }

    bool operator==(const TXYZ& b) const
    {
        return x == b.x
            && y == b.y
            && z == b.z;
    }

    bool operator!=(const TXYZ& b) const
    {
        return x != b.x
            || y != b.y
            || z != b.z;
    }

    bool operator<(const TXYZ& b) const
    {
        return x < b.x
            && y < b.y
            && z < b.z;
    }

    bool operator>(const TXYZ& b) const
    {
        return x > b.x
            && y > b.y
            && z > b.z;
    }

    TXYZ& operator-=(const TXYZ& b)
    {
        x -= b.x;
        y -= b.y;
        z -= b.z;
        return *this;
    }

    TXYZ& operator+=(const TXYZ& b)
    {
        x += b.x;
        y += b.y;
        z += b.z;
        return *this;
    }

    TXYZ& operator/=(const TXYZ& b)
    {
        x /= b.x;
        y /= b.y;
        z /= b.z;
        return *this;
    }

    TXYZ& operator*=(const TXYZ& b)
    {
        x *= b.x;
        y *= b.y;
        z *= b.z;
        return *this;
    }

    double abs() const
    {
        return sqrt(x * x + y * y + z * z);
    }

    double operator*(const double& b) const
    {
        return (x + y + z) * b;
    }

    double operator/(const double& b) const
    {
        return (x + y + z) / b;
    }

    friend TEAKFILE& operator << (TEAKFILE& File, const TXYZ<T>& b) { File.Write((UBYTE*)& b, sizeof(T) * 3); return (File); }
    friend TEAKFILE& operator >> (TEAKFILE& File, TXYZ<T>& b) { File.Read((UBYTE*)& b, sizeof(T) * 3); return (File); }
};

typedef TXY<SLONG> XY;
typedef TXYZ<SLONG> XYZ;

typedef TXY<FLOAT> FXY;
typedef TXYZ<FLOAT> FXYZ;

class PALETTE
{
public:
    PALETTE(void);

    void Blend(int, int) const;
    void BlendIn(void) const;
    void BlendOut(void) const;
    void RefreshDD(int) const;
    void RefreshDD(int, int) const;
    void RefreshPalFromLbm(CString const&);
    void RefreshPalFromPcx(CString const&);
    void ConvertToBlackWhite(void);
    unsigned char FindColorClosestToRGB(unsigned char, unsigned char, unsigned char) const;
    void RotateArea(int, int);
    void CopyArea(int, int, int);
    void CopyAreaFrom(PALETTE const&, int, int, int);

    BUFFER<SDL_Color> Pal;
    SLONG Unknown;
};

//static_assert(sizeof(PALETTE) == 16, "PALETTE size check");

struct TEXTRES_CACHE_ENTRY
{
    SLONG Group, Id;
    char* Text;
};

//static_assert(sizeof(TEXTRES_CACHE_ENTRY) == 12, "TEXTRES_CACHE_ENTRY size check");

#define TEXTRES_CACHED (void*)1

class TEXTRES
{
public:
    TEXTRES();
    TEXTRES(char const*, void*);
    ~TEXTRES(void);

    void Open(char const*, void*);
    BUFFER<char>& GetB(unsigned int, unsigned int);
    char* GetP(unsigned int, unsigned int);
    char* GetS(unsigned int, unsigned int);
    //char* GetS(unsigned int, char const*);
    char* GetS(char const* c, unsigned int i) { return GetS(*(unsigned int*)c, i); }

private:
    BUFFER<char> Path;
    BUFFER<char> Strings;
    BUFFER<TEXTRES_CACHE_ENTRY> Entries;
};

//static_assert(sizeof(TEXTRES) == 36, "TEXTRES size check");

class CRegistration
{
public:
    CRegistration(void);
    CRegistration(CString const&, unsigned int);
    void ReSize(CString const&, unsigned int);
    CString GetDisplayString(void);
    int GetMode(void);
    CString GetSomeString(char*);
    unsigned int CalcChecksum(CString);
    int IsMaster(void);
    void CheckIfIsMaster(void);
};

#define VIDRAMBM (void*)1
#define SYSRAMBM (void*)2

class TECBM
{
public:
    TECBM(void);
    TECBM(CString const&, void*);
    TECBM(CString const&, int, void*);
    TECBM(int, int, void*);
    TECBM(TXY<int>, void*);
    ~TECBM(void);

    void Destroy(void);
    TECBM& operator=(TECBM&);
    virtual int Refresh(void);
    int TextOutA(int, int, unsigned int, unsigned int, CString const&);
    int IsOk(void) const;
    static int IsMemCritical(void);
    static int IsHardwareCritical(void);
    static int IsEitherCritical(void);
    static void SetCriticalMem(int);
    void ReSize(CString const&, void*);
    void ReSize(CString const&, int, void*);
    void ReSizeLbm(CString const&, void*);
    void ReSizeLbm(CString const&, int, void*);
    void ReSize(TXY<int>, void*);
    void ReSizePcx(CString const&, int, void*);
    void ReSizePcx(CString const&, void*);
    int SavePCX(CString const&, PALETTE const&) const;
    void ShiftColors(int);
    void RemapColor(unsigned char, unsigned char);
    int BlitFrom(TECBM const&, TXY<int>, TXY<int>);
    int BlitFromT(TECBM const&, TXY<int>, TXY<int>, unsigned char);
    void InterleaveBitmaps(TECBM const&, TECBM const&, int);
    int UniversalClip(TXY<int>*, CRect*);
    void GetClipRegion(TXY<int>*, TXY<int>*);
    void SetClipRegion(void);
    void SetClipRegion(TXY<int>, TXY<int>);
    int IsLost(void) const;
    int SetPixel(TXY<int>, unsigned char);
    unsigned char GetPixel(TXY<int>) const;
    int FillWith(unsigned char);
    int FillWith(TECBM const&);
    int FillWith(TECBM const&, TXY<int>);
    int Line(TXY<int> const&, TXY<int> const&, unsigned char);
    int HLine(int, int, int, unsigned char);
    int VLine(int, int, int, unsigned char);
    int DotLine(TXY<int> const&, TXY<int> const&, unsigned char);
    int Rectangle(TXY<int> const&, TXY<int> const&, unsigned char);
    int Box(TXY<int>, TXY<int>, unsigned char);
    int Box(TXY<int> const&, TXY<int> const&, TECBM const&);
    int Box(TXY<int> const&, TXY<int> const&, TECBM const&, TXY<int>);
    int Circle(TXY<int> const&, int, unsigned char);
    int BlitFrom(TECBM&, TXY<int>);
    int BlitFromT(TECBM&, TXY<int>);
    int BlitPartFrom(TECBM&, TXY<int>, TXY<int> const&, TXY<int> const&);
    int BlitPartFromT(TECBM&, TXY<int>, TXY<int> const&, TXY<int> const&);
    int GetAnzSubBitmaps(void) const;
    TECBM* ParseNextVertikalSubBitmap(void);
    TECBM* ParseNextHorizontalSubBitmap(void);
    FBUFFER<TECBM>* ParseVertikalSubBitmaps(void);
    void ParseHorizontalSubBitmapsInto(FBUFFER<TECBM>&);
    FBUFFER<TECBM>* ParseHorizontalSubBitmaps(void);
    TECBM* ParseVertikalSubBitmapNumberX(int);
    TECBM* ParseHorizontalSubBitmapNumberX(int);
    int ParseVertikalSubBitmapNumberXInto(int, TECBM&);
    int ParseHorizontalSubBitmapNumberXInto(int, TECBM&);

    SDL_Surface* Surface;
    SLONG Unknown[6];

    XY Size;

private:
    static bool BltSupport;
    static int CriticalVidMem;
    static int TotalVidMem;
};

//static_assert(sizeof(TECBM) == 40, "TECBM size check");

class TECBMKEY
{
public:
    TECBMKEY(TECBM&);
    ~TECBMKEY(void);

    SDL_Surface* Surface;
    UBYTE* Bitmap;
    SLONG lPitch;
};

//static_assert(sizeof(TECBMKEY) == 12, "TECBMKEY size check");

class TECBMKEYC
{
public:
    TECBMKEYC(const TECBM&);
    ~TECBMKEYC(void);

    SDL_Surface* Surface;
    const UBYTE* Bitmap;
    SLONG lPitch;
};

//static_assert(sizeof(TECBMKEYC) == 12, "TECBMKEYC size check");


class HDU
{
public:
    HDU();
    ~HDU();

    void Close();
    void Disable();
    void ClearScreen();
    void HercPrintf(int, char* Format, ...);
    void HercPrintf(char* Format, ...);
    void LogPosition(const char*, int);

private:
    FILE* Log;
};

extern HDU Hdu;

#define hprintf Hdu.HercPrintf
#define hprintvar(x) Hdu.HercPrintf("%d\n", x)

class XID
{
public:
    XID(void);
    void SetValue(unsigned int);

    friend TEAKFILE& operator<<(TEAKFILE&, XID const&);
    friend TEAKFILE& operator>>(TEAKFILE&, XID&);

    int Value;
    int Index;
};

extern void TeakAlbumRemoveT(FBUFFER<unsigned int>&, unsigned int, CString const&, unsigned int);
extern void TeakAlbumRefresh(FBUFFER<unsigned int>&, unsigned int);
extern int TeakAlbumSearchT(FBUFFER<unsigned int>&, unsigned int, CString const&, unsigned int);
extern int TeakAlbumXIdSearchT(FBUFFER<unsigned int>&, unsigned int, CString const&, XID&);
extern int TeakAlbumIsInAlbum(FBUFFER<unsigned int>&, unsigned int, unsigned int);
extern unsigned int TeakAlbumAddT(FBUFFER<unsigned int>&, unsigned int, CString const&, unsigned int);
extern unsigned int TeakAlbumFrontAddT(FBUFFER<unsigned int>&, unsigned int, CString const&, unsigned int);
extern unsigned int TeakAlbumGetNumFree(FBUFFER<unsigned int>&, unsigned int);
extern unsigned int TeakAlbumGetNumUsed(FBUFFER<unsigned int>&, unsigned int);
extern unsigned int TeakAlbumRandom(FBUFFER<unsigned int>&, unsigned int, CString const&, TEAKRAND*);

template <typename T>
class ALBUM
{
public:
    ALBUM(BUFFER<T>& buffer, CString str)
        : LastId(0xFFFFFF)
        , Values((FBUFFER<T>*)(&buffer))
        , Name(str)
    {
    }

    void Repair(BUFFER<T>& buffer)
    {
        Values = (FBUFFER<T>*)&buffer;
    }

    int IsInAlbum(unsigned int id)
    {
        return TeakAlbumIsInAlbum(Ids, Values->AnzEntries(), id);
    }

    int AnzEntries()
    {
        return Values->AnzEntries();
    }

    int GetNumFree()
    {
        return TeakAlbumGetNumFree(Ids, Values->AnzEntries());
    }

    int GetNumUsed()
    {
        return TeakAlbumGetNumUsed(Ids, Values->AnzEntries());
    }

    int GetRandomUsedIndex(TEAKRAND* rand = NULL)
    {
        return TeakAlbumRandom(Ids, Values->AnzEntries(), Name, rand);
    }

    int GetUniqueId()
    {
        return ++LastId;
    }

    unsigned int GetIdFromIndex(int i)
    {
        return Ids[i];
    }

    void ClearAlbum()
    {
        TeakAlbumRefresh(Ids, Values->AnzEntries());
        for (int i = Ids.AnzEntries() - 1; i >= 0; --i)
            Ids[i] = 0;
    }

    void Swap(int a, int b)
    {
        TeakAlbumRefresh(Ids, Values->AnzEntries());
        if (a >= Ids.Size)
            a = (*this)(a);
        if (b >= Ids.Size)
            b = (*this)(b);

        UBYTE* tmp = new UBYTE[sizeof(T)];
        ::Swap(Ids[a], Ids[b]);
        memcpy(tmp, &Values->MemPointer[a], sizeof(T));
        memcpy(&Values->MemPointer[a], &Values->MemPointer[b], sizeof(T));
        memcpy(&Values->MemPointer[b], tmp, sizeof(T));
        delete [] tmp;
    }

    void ResetNextId()
    {
        LastId = 0xFFFFFF;
    }

    void Sort()
    {
        UBYTE* tmp = new UBYTE[sizeof(T)];
        if (!tmp)
            TeakLibW_Exception(FNL, ExcOutOfMem);
        TeakAlbumRefresh(Ids, Values->AnzEntries());
        for (SLONG i = 0; i < Values->AnzEntries() - 1; i++)
        {
            if (Ids[i] && Ids[i + 1] && Values->MemPointer[i] > Values->MemPointer[i + 1])
            {
                ::Swap(Ids[i], Ids[i + 1]);
                memcpy(tmp, &Values->MemPointer[i], sizeof(T));
                memcpy(&Values->MemPointer[i], &Values->MemPointer[i + 1], sizeof(T));
                memcpy(&Values->MemPointer[i + 1], tmp, sizeof(T));
                i -= 2;
                if ( i < -1 )
                    i = -1;
            }
            else if (!Ids[i])
            {
                if (Ids[i + 1])
                {
                    ::Swap(Ids[i], Ids[i + 1]);
                    memcpy(tmp, &Values->MemPointer[i], sizeof(T));
                    memcpy(&Values->MemPointer[i], &Values->MemPointer[i + 1], sizeof(T));
                    memcpy(&Values->MemPointer[i + 1], tmp, sizeof(T));
                    i -= 2;
                    if (i < -1)
                        i = -1;
                }
            }
        }
        if (tmp)
            delete [] tmp;
    }

    unsigned int operator*=(unsigned int id)
    {
        return TeakAlbumFrontAddT(Ids, Values->AnzEntries(), Name, id);
    }

    unsigned int operator+=(unsigned int id)
    {
        return TeakAlbumAddT(Ids, Values->AnzEntries(), Name, id);
    }

    void operator-=(unsigned int id)
    {
        TeakAlbumRemoveT(Ids, Values->AnzEntries(), Name, id);
    }

    unsigned int operator*=(T& rhs)
    {
       unsigned int Id = TeakAlbumFrontAddT(Ids, Values->AnzEntries(), Name, GetUniqueId());
       (*this)[Id] = rhs;
       return Id;
    }

    unsigned int operator*=(T&& rhs)
    {
        unsigned int Id = TeakAlbumFrontAddT(Ids, Values->AnzEntries(), Name, GetUniqueId());
        (*this)[Id] = rhs;
        return Id;
    }

    unsigned int operator+=(T& rhs)
    {
       unsigned int Id = TeakAlbumAddT(Ids, Values->AnzEntries(), Name, GetUniqueId());
       (*this)[Id] = rhs;
       return Id;
    }

    unsigned int operator+=(T&& rhs)
    {
        unsigned int Id = TeakAlbumAddT(Ids, Values->AnzEntries(), Name, GetUniqueId());
        (*this)[Id] = rhs;
        return Id;
    }

    int operator()(unsigned int id)
    {
        return TeakAlbumSearchT(Ids, Values->AnzEntries(), Name, id);
    }

    T& operator[](unsigned int id)
    {
        unsigned int i = TeakAlbumSearchT(Ids, Values->AnzEntries(), Name, id);
        return (*Values)[i];
    }

    friend TEAKFILE& operator<< (TEAKFILE& File, const ALBUM<T>& r)
    {
        File << r.LastId;
        File << r.Ids;
        return File;
    }

    friend TEAKFILE& operator>> (TEAKFILE& File, ALBUM<T>& r)
    {
        File >> r.LastId;
        File >> r.Ids;
        return File;
    }

private:
    unsigned int LastId;
    FBUFFER<unsigned int> Ids;

    // This self-reference could be stored as an offset to survive reallocations,
    // but instead Spellbound implemented a Repair() function.
    FBUFFER<T>* Values;
    CString Name;
};

extern int DoesFileExist(char const*);
extern BUFFER<BYTE>* LoadCompleteFile(char const*);
extern int CalcInertiaVelocity(int, int);
extern int Calc1nSum(int);
