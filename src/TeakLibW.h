#pragma once

extern void memswap(void *, void *, ULONG);
extern char *bprintf(char const *, ...);
extern char *bitoa(int);
extern void here(char *, SLONG);

extern const char *ExcAssert;
extern const char *ExcGuardian;
extern const char *ExcImpossible;
extern const char *ExcNotImplemented;
extern const char *ExcOutOfMem;
extern const char *ExcStrangeMem;

#define FNL 0, 0

extern const char *ExcAlbumInsert;
extern const char *ExcAlbumFind;
extern const char *ExcAlbumDelete;
extern const char *ExcXIDUnrecoverable;
extern const char *ExcAlbumNotConsistent;
extern const char *ExcAlbumInvalidArg;

extern SLONG TeakLibW_Exception(char *, SLONG, const char *, ...);
extern char *TeakStrRemoveCppComment(char *);
extern char *TeakStrRemoveEndingCodes(char *, char const *);
extern unsigned char GerToLower(unsigned char);
extern unsigned char GerToUpper(unsigned char);
extern void RecapizalizeString(CString &str);
extern const char *GetSuffix(const char *);

template <typename T> inline void Limit(T min, T &value, T max) {
    if (value < min)
        value = min;
    if (value > max)
        value = max;
}

template <typename T> inline void Swap(T &a, T &b) {
    T c;
    c = a;
    a = b;
    b = c;
}

template <typename T> inline const T &Min(const T &a, const T &b) { return (b < a) ? b : a; }

template <typename T> inline const T &Max(const T &a, const T &b) { return (a < b) ? b : a; }

template <typename A, typename B> inline A min(const A &a, const B &b) { return (b < a) ? b : a; }

template <typename A, typename B> inline A max(const A &a, const B &b) { return (a < b) ? b : a; }

// inline void ReferTo(...) {}

inline void MB() {}

template <typename T> class BUFFER_V : public std::vector<T> {
  public:
    BUFFER_V() = default;
    BUFFER_V(int size) : std::vector<T>(size) {}
    void ReSize(SLONG anz) {
        Offset = 0;
        std::vector<T>::resize(anz);
    }
    SLONG AnzEntries() const { return std::vector<T>::size(); }
    void Clear() {
        Offset = 0;
        std::vector<T>::clear();
    }
    void FillWith(T value) {
        for (int i = 0; i < std::vector<T>::size(); i++)
            std::vector<T>::at(i) = value;
    }

    // operator T *() const { return std::vector<T>::data(); }
    const T *getData() const { return std::vector<T>::data() + Offset; }
    T *getData() { return std::vector<T>::data() + Offset; }

    // void operator+=(int rhs) { DelPointer += rhs; }
    void incIter(int i) { Offset += i; }
    SLONG getIter() const { return Offset; }

#ifdef DEBUG_ALBUM
    T &operator[](size_t pos) { return std::vector<T>::at(pos); }
    const T &operator[](size_t pos) const { return std::vector<T>::at(pos); };
#endif

  private:
    SLONG Offset{0};
};

template <typename T> class BUFFER {
  public:
    BUFFER(SLONG anz) {
        if (anz == 0) {
            MemPointer = DelPointer = NULL;
        } else {
            MemPointer = DelPointer = new T[anz]();
            if (!MemPointer)
                TeakLibW_Exception(NULL, 0, ExcOutOfMem);
        }

        Size = anz;
    }

    BUFFER(BUFFER &rhs) {
        ::Swap(MemPointer, rhs.MemPointer);
        ::Swap(DelPointer, rhs.DelPointer);
        ::Swap(Size, rhs.Size);
    }

    BUFFER(BUFFER &&rhs) : MemPointer(std::move(rhs.MemPointer)), DelPointer(std::move(rhs.DelPointer)), Size(std::move(rhs.Size)) {}

    BUFFER(void) : MemPointer(NULL), DelPointer(NULL), Size(0) {}

    ~BUFFER() {
        if (MemPointer)
            delete[] MemPointer;
        MemPointer = NULL;
    }

    void ReSize(SLONG anz) {
        if (anz == Size)
            return;

        if (anz > 0) {
            T *m = new T[anz]();
            if (!m)
                TeakLibW_Exception(NULL, 0, ExcOutOfMem);

            if (MemPointer) {
                SLONG num;
                if (anz < Size)
                    num = anz;
                else
                    num = Size;

                // This is *will* break self-referencing pointers
                // ... please don't resize anything that uses ALBUM
                // memswap(m, MemPointer, sizeof(T) * num);
                for (SLONG i = 0; i < num; i++) {
                    // use aligned char buffer to prevent destructor calls that crash the game
                    alignas(T) unsigned char buf[sizeof(T)];
                    T *tmp = new (buf) T;

                    m[i] = MemPointer[i];
                    MemPointer[i] = *tmp;
                }
                DelPointer = m + ((DelPointer - MemPointer) / sizeof(T));
                delete[](MemPointer);
            } else {
                DelPointer = m;
            }
            MemPointer = m;
        } else {
            if (MemPointer)
                delete[] MemPointer;

            MemPointer = DelPointer = NULL;
        }

        Size = anz;
    }

    void ReSize(SLONG anz, T *memory) {
        if (MemPointer)
            delete[] MemPointer;

        DelPointer = &memory[DelPointer - MemPointer];
        Size = anz;
        MemPointer = memory;
    }

    SLONG AnzEntries() const { return Size; }

    void Clear() {
        if (Size > 0) {
            if (MemPointer)
                delete[] MemPointer;
            MemPointer = DelPointer = NULL;
            Size = 0;
        }
    }

    void FillWith(T value) {
        for (int i = 0; i < Size; i++)
            MemPointer[i] = value;
    }

    operator T *() const { return DelPointer; }

    void operator+=(int rhs) { DelPointer += rhs; }

    void operator=(BUFFER<T> &rhs) {
        ::Swap(MemPointer, rhs.MemPointer);
        ::Swap(DelPointer, rhs.DelPointer);
        ::Swap(Size, rhs.Size);
    }

    void operator=(BUFFER<T> &&rhs) {
        MemPointer = std::move(rhs.MemPointer);
        DelPointer = std::move(rhs.DelPointer);
        Size = std::move(rhs.Size);
    }

    T *MemPointer;
    T *DelPointer;
    SLONG Size;
};
#define TEAKFILE_READ 1
#define TEAKFILE_WRITE 2

class TEAKFILE {
  public:
    TEAKFILE(void);
    TEAKFILE(char const *, SLONG);
    ~TEAKFILE(void);

    void ReadLine(char *, SLONG) const;
    int IsEof(void) const;
    void Close(void);
    SLONG GetFileLength(void) const;
    SLONG GetPosition(void) const;
    void Open(char const *, SLONG);
    int IsOpen(void) const;
    unsigned char *Read(SLONG);
    void Read(unsigned char *, SLONG);
    char *ReadLine(void);
    void ReadTrap(SLONG) const;
    void WriteTrap(SLONG) const;
    void SetPasswort(char *);
    void SetPosition(SLONG) const;
    void Skip(SLONG);
    void Write(const unsigned char *, SLONG);
    void WriteLine(char *);
    void Announce(SLONG);

    SDL_RWops *Ctx;
    char *Path;
    BUFFER<UBYTE> MemBuffer;
    SLONG MemPointer;
    ULONG MemBufferUsed;

    friend TEAKFILE &operator<<(TEAKFILE &File, const bool &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, bool &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const signed char &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, signed char &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const unsigned char &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, unsigned char &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const unsigned short &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, unsigned short &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const signed short &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, signed short &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const unsigned int &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, unsigned int &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const signed int &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, signed int &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const unsigned long &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, unsigned long &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const signed long &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, signed long &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const __int64 &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, __int64 &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const double &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, double &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const float &b) {
        File.Write((const UBYTE *)&b, sizeof(b));
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, float &b) {
        File.Read((UBYTE *)&b, sizeof(b));
        return File;
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const CPoint &b) {
        File.Write((const UBYTE *)&b, sizeof(CPoint));
        return (File);
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, CPoint &b) {
        File.Read((UBYTE *)&b, sizeof(CPoint));
        return (File);
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const CString &b) {
        File << b.GetLength() + 1;
        File.Write((const UBYTE *)(PCSTR)b, b.GetLength() + 1);
        return File;
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, CString &b) {
        ULONG size;
        File >> size;
        BUFFER_V<BYTE> str(size);
        File.Read(str.getData(), size);
        b = (PCSTR)(BYTE *)str.getData();
        return File;
    }

    template <typename T, unsigned int N> friend TEAKFILE &operator<<(TEAKFILE &File, const std::array<T, N> &buffer) {
        for (SLONG i = 0; i < buffer.size(); i++) {
            File << buffer[i];
        }
        return File;
    }

    template <typename T, unsigned int N> friend TEAKFILE &operator>>(TEAKFILE &File, std::array<T, N> &buffer) {
        for (SLONG i = 0; i < buffer.size(); i++) {
            File >> buffer[i];
        }
        return File;
    }

    template <typename T> friend TEAKFILE &operator<<(TEAKFILE &File, const BUFFER_V<T> &buffer) {
        File << buffer.AnzEntries();
        File << buffer.getIter();
        for (SLONG i = 0; i < buffer.AnzEntries(); i++) {
            File << buffer[i];
        }
        return File;
    }

    template <typename T> friend TEAKFILE &operator>>(TEAKFILE &File, BUFFER_V<T> &buffer) {
        SLONG size, offset;
        File >> size >> offset;
        buffer.ReSize(size);
        buffer.incIter(offset);
        for (SLONG i = 0; i < buffer.AnzEntries(); i++) {
            File >> buffer[i];
        }
        return File;
    }

    template <typename T> friend TEAKFILE &operator<<(TEAKFILE &File, const BUFFER<T> &buffer) {
        File << buffer.Size;
        File << SLONG(buffer.DelPointer - buffer.MemPointer);
        for (SLONG i = 0; i < buffer.Size; i++)
            File << buffer.MemPointer[i];
        return File;
    }

    template <typename T> friend TEAKFILE &operator>>(TEAKFILE &File, BUFFER<T> &buffer) {
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
    void CodeBlock(unsigned char *, SLONG, SLONG);
};

// static_assert(sizeof(TEAKFILE) == 68, "TEAKFILE_size_check");

class CRLEReader {
  public:
    CRLEReader(const char *path);
    ~CRLEReader(void);

    bool Close(void);
    bool Buffer(void *, SLONG);
    bool NextSeq(void);
    bool Read(BYTE *, SLONG, bool);

    SLONG GetSize() { return Size; }

    bool getIsRLE() { return IsRLE; }
    void SaveAsPlainText();

  private:
    SDL_RWops *Ctx;
    int8_t SeqLength;
    int8_t SeqUsed;
    bool IsSeq;
    BYTE Sequence[132];

    bool IsRLE;
    SLONG Size;
    int Key;

    const char *Path;
};

class TEAKRAND {
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

    friend TEAKFILE &operator<<(TEAKFILE &File, const TEAKRAND &r);
    friend TEAKFILE &operator>>(TEAKFILE &File, TEAKRAND &r);

  private:
    ULONG Seed{};
    ULONG Value;
};

template <typename T> class TXY {
  public:
    T x, y;

    TXY() : x(), y() {}
    TXY(T s) : x(s), y(s) {}
    TXY(T x, T y) : x(x), y(y) {}
    TXY(POINT &point) : x(point.x), y(point.y) {}

    TXY operator+(const TXY &b) const { return TXY(x + b.x, y + b.y); }

    TXY operator-(const TXY &b) const { return TXY(x - b.x, y - b.y); }

    TXY operator*(const TXY &b) const { return TXY(x * b.x, y * b.y); }

    TXY operator/(const TXY &b) const { return TXY(x / b.x, y / b.y); }

    TXY operator*(const T &b) const { return TXY(x * b, y * b); }

    TXY operator/(const T &b) const { return TXY(x / b, y / b); }

    TXY operator-() const { return TXY(-x, -y); }

    bool operator==(const TXY &b) const { return x == b.x && y == b.y; }

    bool operator!=(const TXY &b) const { return x != b.x || y != b.y; }

    bool operator<(const TXY &b) const { return x < b.x && y < b.y; }

    bool operator>(const TXY &b) const { return x > b.x && y > b.y; }

    TXY &operator-=(const TXY &b) {
        x -= b.x;
        y -= b.y;
        return *this;
    }

    TXY &operator+=(const TXY &b) {
        x += b.x;
        y += b.y;
        return *this;
    }

    TXY &operator/=(const TXY &b) {
        x /= b.x;
        y /= b.y;
        return *this;
    }

    TXY &operator*=(const TXY &b) {
        x *= b.x;
        y *= b.y;
        return *this;
    }

    operator CPoint &() { return reinterpret_cast<CPoint &>(*this); }

    double abs() const { return sqrt(x * x + y * y); }

    double operator*(const double &b) const { return (x + y) * b; }

    double operator/(const double &b) const { return (x + y) / b; }

    bool IfIsWithin(T _x1, T _y1, T _x2, T _y2) const { return x >= _x1 && y >= _y1 && x <= _x2 && y <= _y2; }

    bool IfIsWithin(const TXY<T> &a, const TXY<T> &b) const { return a < *this && *this < b; }

    friend TEAKFILE &operator<<(TEAKFILE &File, const TXY<T> &b) {
        File.Write((const UBYTE *)&b, sizeof(T) * 2);
        return (File);
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, TXY<T> &b) {
        File.Read((UBYTE *)&b, sizeof(T) * 2);
        return (File);
    }
};

template <typename T> class TXYZ {
  public:
    T x, y, z;

    TXYZ() : x(), y(), z() {}
    TXYZ(T s) : x(s), y(s), z(s) {}
    TXYZ(T x, T y, T z) : x(x), y(y), z(z) {}

    TXYZ operator+(const TXYZ &b) const { return TXY<T>(x + b.x, y + b.y, z + b.z); }

    TXYZ operator-(const TXYZ &b) const { return TXY<T>(x - b.x, y - b.y, z - b.z); }

    TXYZ operator*(const TXYZ &b) const { return TXY<T>(x * b.x, y * b.y, z * b.z); }

    TXYZ operator/(const TXYZ &b) const { return TXY<T>(x / b.x, y / b.y, z / b.z); }

    TXYZ operator*(const T &b) const { return TXY<T>(x * b, y * b, z * b); }

    TXYZ operator/(const T &b) const { return TXY<T>(x / b, y / b, z / b); }

    TXYZ operator-() const { return TXY<T>(-x, -y, -z); }

    bool operator==(const TXYZ &b) const { return x == b.x && y == b.y && z == b.z; }

    bool operator!=(const TXYZ &b) const { return x != b.x || y != b.y || z != b.z; }

    bool operator<(const TXYZ &b) const { return x < b.x && y < b.y && z < b.z; }

    bool operator>(const TXYZ &b) const { return x > b.x && y > b.y && z > b.z; }

    TXYZ &operator-=(const TXYZ &b) {
        x -= b.x;
        y -= b.y;
        z -= b.z;
        return *this;
    }

    TXYZ &operator+=(const TXYZ &b) {
        x += b.x;
        y += b.y;
        z += b.z;
        return *this;
    }

    TXYZ &operator/=(const TXYZ &b) {
        x /= b.x;
        y /= b.y;
        z /= b.z;
        return *this;
    }

    TXYZ &operator*=(const TXYZ &b) {
        x *= b.x;
        y *= b.y;
        z *= b.z;
        return *this;
    }

    double abs() const { return sqrt(x * x + y * y + z * z); }

    double operator*(const double &b) const { return (x + y + z) * b; }

    double operator/(const double &b) const { return (x + y + z) / b; }

    friend TEAKFILE &operator<<(TEAKFILE &File, const TXYZ<T> &b) {
        File.Write((const UBYTE *)&b, sizeof(T) * 3);
        return (File);
    }
    friend TEAKFILE &operator>>(TEAKFILE &File, TXYZ<T> &b) {
        File.Read((UBYTE *)&b, sizeof(T) * 3);
        return (File);
    }
};

typedef TXY<SLONG> XY;
typedef TXYZ<SLONG> XYZ;

typedef TXY<FLOAT> FXY;
typedef TXYZ<FLOAT> FXYZ;

class PALETTE {
  public:
    PALETTE(void);

    void Blend(SLONG, SLONG) const;
    void BlendIn(void) const;
    void BlendOut(void) const;
    void RefreshDD(int) const;
    void RefreshDD(SLONG, int) const;
    void RefreshPalFromLbm(CString const &);
    void RefreshPalFromPcx(CString const &);
    void RefreshPalFromTga(CString const &);
    void ConvertToBlackWhite(void);
    unsigned char FindColorClosestToRGB(unsigned char, unsigned char, unsigned char) const;
    void RotateArea(SLONG, SLONG);
    void CopyArea(SLONG, SLONG, SLONG);
    void CopyAreaFrom(PALETTE const &, SLONG, SLONG, SLONG);

    BUFFER_V<SDL_Color> Pal;
};

// static_assert(sizeof(PALETTE) == 16, "PALETTE size check");

struct TEXTRES_CACHE_ENTRY {
    SLONG Group{};
    SLONG Id{};
    char *Text{nullptr};
};

// static_assert(sizeof(TEXTRES_CACHE_ENTRY) == 12, "TEXTRES_CACHE_ENTRY size check");

#define TEXTRES_CACHED (void *)1

class TEXTRES {
  public:
    TEXTRES();
    TEXTRES(char const *, void *);
    ~TEXTRES(void);

    void Open(char const *, void *);
    BUFFER_V<char> &GetB(ULONG, ULONG);
    char *GetP(ULONG, ULONG);
    char *GetS(ULONG, ULONG);
    // char* GetS(ULONG, char const*);
    char *GetS(char const *c, ULONG i) { return GetS(*(const ULONG *)c, i); }

  private:
    BUFFER_V<char> Path;
    BUFFER_V<char> Strings;
    BUFFER_V<TEXTRES_CACHE_ENTRY> Entries;
};

// static_assert(sizeof(TEXTRES) == 36, "TEXTRES size check");

class CRegistration {
  public:
    CRegistration(void);
    CRegistration(CString const &, ULONG);
    void ReSize(CString const &, ULONG);
    CString GetDisplayString(void);
    SLONG GetMode(void);
    CString GetSomeString(char *);
    ULONG CalcChecksum(CString);
    int IsMaster(void);
    void CheckIfIsMaster(void);
};

#define VIDRAMBM (void *)1
#define SYSRAMBM (void *)2

class TECBM {
  public:
    TECBM(void);
    TECBM(CString const &, void *);
    TECBM(CString const &, SLONG, void *);
    TECBM(SLONG, SLONG, void *);
    TECBM(TXY<SLONG>, void *);
    ~TECBM(void);

    void Destroy(void);
    TECBM &operator=(TECBM &);
    virtual int Refresh(void);
    int TextOutA(SLONG, SLONG, ULONG, ULONG, CString const &);
    int IsOk(void) const;
    static int IsMemCritical(void);
    static int IsHardwareCritical(void);
    static int IsEitherCritical(void);
    static void SetCriticalMem(SLONG);
    void ReSize(CString const &, void *);
    void ReSize(CString const &, SLONG, void *);
    void ReSizeLbm(CString const &, void *);
    void ReSizeLbm(CString const &, SLONG, void *);
    void ReSize(TXY<SLONG>, void *);
    void ReSizePcx(CString const &, SLONG, void *);
    void ReSizePcx(CString const &, void *);
    void ReSizeTga(CString const &, void *);
    int SavePCX(CString const &, PALETTE const &) const;
    void ShiftColors(SLONG);
    void RemapColor(unsigned char, unsigned char);
    int BlitFrom(TECBM const &, TXY<SLONG>, TXY<SLONG>);
    int BlitFromT(TECBM const &, TXY<SLONG>, TXY<SLONG>, unsigned char);
    void InterleaveBitmaps(TECBM const &, TECBM const &, SLONG);
    int UniversalClip(TXY<SLONG> *, CRect *);
    void GetClipRegion(TXY<SLONG> *, TXY<SLONG> *);
    void SetClipRegion(void);
    void SetClipRegion(TXY<SLONG>, TXY<SLONG>);
    int IsLost(void) const;
    int SetPixel(TXY<SLONG>, unsigned char);
    unsigned char GetPixel(TXY<SLONG>) const;
    int FillWith(unsigned char);
    int FillWith(TECBM const &);
    int FillWith(TECBM const &, TXY<SLONG>);
    int Line(TXY<SLONG> const &, TXY<SLONG> const &, unsigned char);
    int HLine(SLONG, SLONG, SLONG, unsigned char);
    int VLine(SLONG, SLONG, SLONG, unsigned char);
    int DotLine(TXY<SLONG> const &, TXY<SLONG> const &, unsigned char);
    int Rectangle(TXY<SLONG> const &, TXY<SLONG> const &, unsigned char);
    int Box(TXY<SLONG>, TXY<SLONG>, unsigned char);
    int Box(TXY<SLONG> const &, TXY<SLONG> const &, TECBM const &);
    int Box(TXY<SLONG> const &, TXY<SLONG> const &, TECBM const &, TXY<SLONG>);
    int Circle(TXY<SLONG> const &, SLONG, unsigned char);
    int BlitFrom(TECBM &, TXY<SLONG>);
    int BlitFromT(TECBM &, TXY<SLONG>);
    int BlitPartFrom(TECBM &, TXY<SLONG>, TXY<SLONG> const &, TXY<SLONG> const &);
    int BlitPartFromT(TECBM &, TXY<SLONG>, TXY<SLONG> const &, TXY<SLONG> const &);
    SLONG GetAnzSubBitmaps(void) const;
    TECBM *ParseNextVertikalSubBitmap(void);
    TECBM *ParseNextHorizontalSubBitmap(void);
    BUFFER_V<TECBM> *ParseVertikalSubBitmaps(void);
    void ParseHorizontalSubBitmapsInto(BUFFER_V<TECBM> &);
    BUFFER_V<TECBM> *ParseHorizontalSubBitmaps(void);
    TECBM *ParseVertikalSubBitmapNumberX(SLONG);
    TECBM *ParseHorizontalSubBitmapNumberX(SLONG);
    int ParseVertikalSubBitmapNumberXInto(SLONG, TECBM &);
    int ParseHorizontalSubBitmapNumberXInto(SLONG, TECBM &);

    SDL_Surface *Surface;

    XY Size;

  private:
    static bool BltSupport;
    static SLONG CriticalVidMem;
    static SLONG TotalVidMem;
};

// static_assert(sizeof(TECBM) == 40, "TECBM size check");

class TECBMKEY {
  public:
    TECBMKEY(TECBM &);
    ~TECBMKEY(void);

    SDL_Surface *Surface;
    UBYTE *Bitmap;
    SLONG lPitch;
};

// static_assert(sizeof(TECBMKEY) == 12, "TECBMKEY size check");

class TECBMKEYC {
  public:
    TECBMKEYC(const TECBM &);
    ~TECBMKEYC(void);

    SDL_Surface *Surface;
    const UBYTE *Bitmap;
    SLONG lPitch;
};

// static_assert(sizeof(TECBMKEYC) == 12, "TECBMKEYC size check");

class HDU {
  public:
    HDU();
    ~HDU();

    void Close();
    void Disable();
    void ClearScreen();
    void HercPrintf(int, const char *Format, ...);
    void HercPrintf(const char *Format, ...);
    void LogPosition(const char *, int);

  private:
    FILE *Log;
};

extern HDU Hdu;

#define hprintf Hdu.HercPrintf
#define hprintvar(x) Hdu.HercPrintf("%d\n", x)

class XID {
  public:
    XID(void);
    void SetValue(ULONG);

    friend TEAKFILE &operator<<(TEAKFILE &, XID const &);
    friend TEAKFILE &operator>>(TEAKFILE &, XID &);

    SLONG Value;
    SLONG Index;
};

extern void TeakAlbumRemoveT(BUFFER<ULONG> &, ULONG, CString const &, ULONG);
extern void TeakAlbumRefresh(BUFFER<ULONG> &, ULONG);
extern SLONG TeakAlbumSearchT(BUFFER<ULONG> &, ULONG, CString const &, ULONG);
extern SLONG TeakAlbumXIdSearchT(BUFFER<ULONG> &, ULONG, CString const &, XID &);
extern int TeakAlbumIsInAlbum(BUFFER<ULONG> &, ULONG, ULONG);
extern ULONG TeakAlbumAddT(BUFFER<ULONG> &, ULONG, CString const &, ULONG);
extern ULONG TeakAlbumFrontAddT(BUFFER<ULONG> &, ULONG, CString const &, ULONG);
extern ULONG TeakAlbumGetNumFree(BUFFER<ULONG> &, ULONG);
extern ULONG TeakAlbumGetNumUsed(BUFFER<ULONG> &, ULONG);
extern ULONG TeakAlbumRandom(BUFFER<ULONG> &, ULONG, CString const &, TEAKRAND *);

template <typename T> class ALBUM {
  public:
    ALBUM(BUFFER<T> &buffer, CString str) : LastId(0xFFFFFF), Values((BUFFER<T> *)(&buffer)), Name(str) {}

    void Repair(BUFFER<T> &buffer) { Values = (BUFFER<T> *)&buffer; }

    int IsInAlbum(ULONG id) { return TeakAlbumIsInAlbum(Ids, Values->AnzEntries(), id); }

    SLONG AnzEntries() { return Values->AnzEntries(); }

    SLONG GetNumFree() { return TeakAlbumGetNumFree(Ids, Values->AnzEntries()); }

    SLONG GetNumUsed() { return TeakAlbumGetNumUsed(Ids, Values->AnzEntries()); }

    SLONG GetRandomUsedIndex(TEAKRAND *rand = NULL) { return TeakAlbumRandom(Ids, Values->AnzEntries(), Name, rand); }

    SLONG GetUniqueId() { return ++LastId; }

    ULONG GetIdFromIndex(SLONG i) { return Ids[i]; }

    void ClearAlbum() {
        TeakAlbumRefresh(Ids, Values->AnzEntries());
        for (SLONG i = Ids.AnzEntries() - 1; i >= 0; --i)
            Ids[i] = 0;
    }

    void Swap(SLONG a, SLONG b) {
        TeakAlbumRefresh(Ids, Values->AnzEntries());
        if (a >= Ids.Size)
            a = (*this)(a);
        if (b >= Ids.Size)
            b = (*this)(b);

        ::Swap(Ids[a], Ids[b]);
        ::Swap(Values->MemPointer[a], Values->MemPointer[b]);
    }

    void ResetNextId() { LastId = 0xFFFFFF; }

    void Sort() {
        TeakAlbumRefresh(Ids, Values->AnzEntries());
        for (SLONG i = 0; i < Values->AnzEntries() - 1; i++) {
            if (Ids[i] && Ids[i + 1] && Values->MemPointer[i] > Values->MemPointer[i + 1]) {
                ::Swap(Ids[i], Ids[i + 1]);
                ::Swap(Values->MemPointer[i], Values->MemPointer[i + 1]);
                i -= 2;
                if (i < -1)
                    i = -1;
            } else if (!Ids[i]) {
                if (Ids[i + 1]) {
                    ::Swap(Ids[i], Ids[i + 1]);
                    ::Swap(Values->MemPointer[i], Values->MemPointer[i + 1]);
                    i -= 2;
                    if (i < -1)
                        i = -1;
                }
            }
        }
    }

    ULONG operator*=(ULONG id) { return TeakAlbumFrontAddT(Ids, Values->AnzEntries(), Name, id); }

    ULONG operator+=(ULONG id) { return TeakAlbumAddT(Ids, Values->AnzEntries(), Name, id); }

    void operator-=(ULONG id) { TeakAlbumRemoveT(Ids, Values->AnzEntries(), Name, id); }

    ULONG operator*=(T &rhs) {
        ULONG Id = TeakAlbumFrontAddT(Ids, Values->AnzEntries(), Name, GetUniqueId());
        (*this)[Id] = rhs;
        return Id;
    }

    ULONG operator*=(T &&rhs) {
        ULONG Id = TeakAlbumFrontAddT(Ids, Values->AnzEntries(), Name, GetUniqueId());
        (*this)[Id] = rhs;
        return Id;
    }

    ULONG operator+=(T &rhs) {
        ULONG Id = TeakAlbumAddT(Ids, Values->AnzEntries(), Name, GetUniqueId());
        (*this)[Id] = rhs;
        return Id;
    }

    ULONG operator+=(T &&rhs) {
        ULONG Id = TeakAlbumAddT(Ids, Values->AnzEntries(), Name, GetUniqueId());
        (*this)[Id] = rhs;
        return Id;
    }

    SLONG operator()(ULONG id) { return TeakAlbumSearchT(Ids, Values->AnzEntries(), Name, id); }

    T &operator[](ULONG id) {
        ULONG i = TeakAlbumSearchT(Ids, Values->AnzEntries(), Name, id);
        return (*Values)[i];
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const ALBUM<T> &r) {
        File << r.LastId;
        File << r.Ids;
        return File;
    }

    friend TEAKFILE &operator>>(TEAKFILE &File, ALBUM<T> &r) {
        File >> r.LastId;
        File >> r.Ids;
        return File;
    }

  private:
    ULONG LastId;
    BUFFER<ULONG> Ids;

    // This self-reference could be stored as an offset to survive reallocations,
    // but instead Spellbound implemented a Repair() function.
    BUFFER<T> *Values;
    CString Name;
};

extern int DoesFileExist(char const *);
extern BUFFER_V<BYTE> *LoadCompleteFile(char const *);
extern SLONG CalcInertiaVelocity(SLONG, SLONG);
extern SLONG Calc1nSum(SLONG);

extern bool run_regression();

template <typename T> class ALBUM_V {
  public:
    class Iter {
      public:
        using difference_type = int;
        using value_type = T;
        using pointer = T *;
        using reference = T &;
        using iterator_category = std::random_access_iterator_tag;

        Iter(typename std::vector<T>::iterator it, std::vector<int>::iterator it2, std::unordered_map<ULONG, int> *h) : It(it), It2(it2), Hash(h) {}
        inline Iter &operator++() {
            It++;
            It2++;
            return (*this);
        }
        inline Iter &operator--() {
            It--;
            It2--;
            return (*this);
        }
        inline Iter &operator+=(int i) {
            It += i;
            It2 += i;
            return (*this);
        }
        inline Iter &operator-=(int i) {
            It -= i;
            It2 -= i;
            return (*this);
        }
        friend Iter operator+(Iter it, int i) { return Iter(it.It + i, it.It2 + i, it.Hash); }
        friend Iter operator-(Iter it, int i) { return Iter(it.It - i, it.It2 - i, it.Hash); }
        friend Iter operator+(int i, Iter it) { return Iter(it.It + i, it.It2 + i, it.Hash); }
        friend Iter operator-(int i, Iter it) { return Iter(it.It - i, it.It2 - i, it.Hash); }
        inline difference_type operator-(Iter it) { return It - it.It; }
        inline reference operator*() const { return *It; }
        inline bool operator==(const Iter &i) const { return It == i.It; }
        inline bool operator!=(const Iter &i) const { return It != i.It; }

        inline bool operator<(const Iter &i) const { return It < i.It; }
        inline bool operator<=(const Iter &i) const { return It <= i.It; }
        inline bool operator>(const Iter &i) const { return It > i.It; }
        inline bool operator>=(const Iter &i) const { return It >= i.It; }

        static void swap(Iter &a, Iter &b) {
            if (a == b) {
                return;
            }
            auto *h = a.Hash;
            if (h->end() != h->find(*a.It2)) {
                h->at(*a.It2) += (b.It2 - a.It2);
            }
            if (h->end() != h->find(*b.It2)) {
                h->at(*b.It2) -= (b.It2 - a.It2);
            }
            std::iter_swap(a.It, b.It);
            std::iter_swap(a.It2, b.It2);
        }

        bool IsInAlbum() const { return (*It2) != 0; }

      private:
        typename std::vector<T>::iterator It;
        std::vector<int>::iterator It2;
        std::unordered_map<ULONG, int> *Hash;
    };
    Iter begin() { return Iter(List.begin(), ListInit.begin(), &Hash); }
    Iter end() { return Iter(List.end(), ListInit.end(), &Hash); }

    ALBUM_V(CString str) : Name(str) {}

    /* query capacity and resize */

    SLONG AnzEntries() const {
#ifdef DEBUG_ALBUM
        assert(List.size() == ListInit.size());
#endif
        return List.size();
    }
    SLONG GetNumFree() const { return std::count(ListInit.begin(), ListInit.end(), 0); }
    SLONG GetNumUsed() const { return AnzEntries() - GetNumFree(); }

    void ReSize(SLONG anz) {
        for (int i = anz; i < AnzEntries(); i++) {
            Hash.erase(ListInit[i]);
            ListInit[i] = 0;
        }
        List.resize(anz);
        ListInit.resize(anz);
    }

    void ClearAlbum() {
        ListInit = std::vector<int>(AnzEntries(), 0);
        Hash = {};
    }

    /* accessing elements */

    ULONG GetIdFromIndex(SLONG i) const { return ListInit[i]; }

    int IsInAlbum(ULONG id) const {
        if (id >= 0x1000000) {
            return (Hash.end() != Hash.find(id));
        } else if (id < AnzEntries() && (ListInit[id] != 0)) {
            return 1;
        }
        return 0;
    }

    SLONG operator()(ULONG id) const { return find(id); }
    SLONG find(ULONG id) const {
        if (id >= 0x1000000) {
            auto it = Hash.find(id);
            if (it != Hash.end()) {
                return it->second;
            }
        } else if (id < AnzEntries()) {
            return id;
        }
        TeakLibW_Exception(nullptr, 0, ExcAlbumFind, Name.c_str());
        return 0;
    }

#ifdef DEBUG_ALBUM
    T &operator[](ULONG id) { return List.at(find(id)); }
#else
    T &operator[](ULONG id) { return List[find(id)]; }
#endif
    T &at(ULONG id) { return List.at(find(id)); }

    /* comparison */

    bool operator==(const ALBUM_V<T> &l) const {
        return (LastId == l.LastId) && (Name == l.Name) && (List == l.List) && (ListInit == l.ListInit) && (Hash == l.Hash);
    }
    bool operator!=(const ALBUM_V<T> &l) const { return !operator==(l); }

    bool operator==(const std::vector<T> &l) const {
        if (AnzEntries() != l.size()) {
            return false;
        }
        return std::equal(List.begin(), List.end(), l.begin());
    }
    bool operator!=(const std::vector<T> &l) const { return !operator==(l); }

    /* modifiers */

    void ResetNextId() { LastId = 0xFFFFFF; }

    ULONG push_front(ULONG id, T rhs) {
        if (id >= 0x1000000 && (Hash.end() == Hash.find(id))) {
            for (int i = 0; i < AnzEntries(); ++i) {
                if (ListInit[i] == 0) {
                    std::swap(List[i], rhs);
                    ListInit[i] = id;
                    Hash[id] = i;
                    return id;
                }
            }
        }
        TeakLibW_Exception(nullptr, 0, ExcAlbumInsert, Name.c_str());
        return 0;
    }

    ULONG push_back(ULONG id, T rhs) {
        if (id >= 0x1000000 && (Hash.end() == Hash.find(id))) {
            for (int i = AnzEntries() - 1; i >= 0; --i) {
                if (ListInit[i] == 0) {
                    std::swap(List[i], rhs);
                    ListInit[i] = id;
                    Hash[id] = i;
                    return id;
                }
            }
        }
        TeakLibW_Exception(nullptr, 0, ExcAlbumInsert, Name.c_str());
        return 0;
    }

    ULONG operator*=(T rhs) {
        auto id = GetUniqueId();
        return push_front(id, std::move(rhs));
    }

    ULONG operator+=(T rhs) {
        auto id = GetUniqueId();
        return push_back(id, std::move(rhs));
    }

    void operator-=(ULONG id) {
        if (id >= 0x1000000) {
            auto it = Hash.find(id);
            if (it != Hash.end()) {
                ListInit[it->second] = 0;
            }
            Hash.erase(id);
            return;
        } else if (id < AnzEntries() && (ListInit[id] != 0)) {
            Hash.erase(ListInit[id]);
            ListInit[id] = 0;
            return;
        }
        TeakLibW_Exception(nullptr, 0, ExcAlbumDelete, Name.c_str());
    }

    friend TEAKFILE &operator<<(TEAKFILE &File, const ALBUM_V<T> &buffer) {
#ifdef DEBUG_ALBUM
        assert(buffer.AnzEntries() == buffer.List.size());
        assert(buffer.AnzEntries() == buffer.ListInit.size());
#endif

        SLONG filler = 0;

        File << buffer.List.size();
        File << filler;
        for (auto &i : buffer.List) {
            File << i;
        }

        File << buffer.LastId;

        File << buffer.ListInit.size();
        File << filler;
        for (auto &i : buffer.ListInit) {
            File << i;
        }

        /*
        File << buffer.Hash.size();
        for (auto &i : buffer.Hash) {
            File << i.first << i.second;
        }
        */

        return File;
    }

    friend TEAKFILE &operator>>(TEAKFILE &File, ALBUM_V<T> &buffer) {
        SLONG size, filler;
        File >> size;
        File >> filler;
        buffer.ReSize(size);
        for (SLONG i = 0; i < size; i++) {
            File >> buffer.List[i];
        }

        File >> buffer.LastId;

        File >> size;
        File >> filler;
#ifdef DEBUG_ALBUM
        assert(buffer.AnzEntries() == size);
#endif
        for (SLONG i = 0; i < size; i++) {
            File >> buffer.ListInit[i];
        }

        buffer.Hash.clear();
        /*
        for (SLONG i = 0; i < size; i++) {
            ULONG key;
            int value;
            File >> key;
            File >> value;
            buffer.Hash[key] = value;
        }
        */
        for (SLONG i = 0; i < size; i++) {
            auto id = buffer.ListInit[i];
            if (id >= 0x1000000) {
                buffer.Hash[id] = i;
            }
        }

        return File;
    }

    SLONG GetRandomUsedIndex(TEAKRAND *random = NULL) const {
        SLONG used = GetNumUsed();
        if (used == 0) {
            TeakLibW_Exception(nullptr, 0, ExcAlbumFind, Name.c_str());
        }

        SLONG target = (random != nullptr) ? random->Rand(used) : rand() % 5;
        SLONG index = 0;
        for (int i = AnzEntries() - 1; i >= 0; --i) {
            if (ListInit[i] == 0) {
                continue;
            }
            if (++index > target) {
                return ListInit[i];
            }
        }
        TeakLibW_Exception(nullptr, 0, ExcAlbumFind, Name.c_str());
        return 0;
    }

    void Sort() {
        auto a = begin();
        auto b = end() - 1;
        while (true) {
            while (a.IsInAlbum() && a < b) {
                ++a;
            }
            while (!b.IsInAlbum() && a < b) {
                --b;
            }
            if (a >= b) {
                break;
            }
            Iter::swap(a, b);
        }
#ifdef DEBUG_ALBUM
        assert(a == b);
#endif
        qSort(begin(), a - 1);
    }

    void Swap(int a, int b) {
        if (a == b) {
            return;
        }
        if (a >= 0x1000000 && b >= 0x1000000) {
            auto idxA = find(a);
            auto idxB = find(b);
            Hash.at(a) = idxB;
            Hash.at(b) = idxA;
            std::swap(List[idxA], List[idxB]);
            std::swap(ListInit[idxA], ListInit[idxB]);
            return;
        } else if (a < 0x1000000 && b < 0x1000000) {
            auto idA = ListInit[a];
            auto idB = ListInit[b];
            if (Hash.end() != Hash.find(idA)) {
                Hash.at(idA) = b;
            }
            if (Hash.end() != Hash.find(idB)) {
                Hash.at(idB) = a;
            }
            std::swap(List[a], List[b]);
            std::swap(ListInit[a], ListInit[b]);
            return;
        }
        TeakLibW_Exception(nullptr, 0, ExcAlbumInvalidArg, Name.c_str());
    }

    void check_consistent_index() {
        for (int i = 0; i < AnzEntries(); i++) {
            auto id = GetIdFromIndex(i);
            if ((id != 0) && find(id) != i) {
                TeakLibW_Exception(nullptr, 0, ExcAlbumNotConsistent, Name.c_str());
            }
        }
    }

  private:
    void qSort(Iter start, Iter end) {
        if (start >= end)
            return;
        Iter pivotIter = (end - start) / 2 + start;
        Iter i = start;
        Iter j = end;
        while (i < j) {
            while (*i < *pivotIter) {
                ++i;
            }
            while (*pivotIter < *j) {
                --j;
            }
            if (i >= j) {
                break;
            }
            Iter::swap(i, j);
            ++i;
            --j;
        }
        qSort(start, j);
        qSort(j + 1, end);
    }

  private:
    SLONG GetUniqueId() { return ++LastId; }

    ULONG LastId{0xFFFFFF};
    std::vector<T> List;
    std::vector<int> ListInit;
    std::unordered_map<ULONG, int> Hash;
    CString Name;
};
