#pragma once

#include <list>
#include <map>

typedef unsigned short word;
typedef unsigned int dword;

// Can you spot the bug? (x is executed two more times just to get the error codes)
// Bonus points if you spot that FAILED() should've been used to check the HRESULT.
#define DD_ERROR(x) if (!(x)) ODS("DDError in File: %s Line: %d Code: %d [%x]",__FILE__,__LINE__,x,x);

extern void ODS(const char *, ...);
extern int GetLowestSetBit(int mask);
extern int GetHighestSetBit(int mask);

#define CREATE_SYSMEM      0
#define CREATE_VIDMEM      1
#define CREATE_USECOLORKEY 2
#define CREATE_USEZBUFFER  4
#define CREATE_USEALPHA    8
#define CREATE_FULLSCREEN  16

class GfxLib
{
public:
    struct _GfxStruct* ReloadSurface(__int64);
    int Restore(void);
    void Release(void);
    class GfxLib* ReleaseSurface(int);
    class GfxLib* ReleaseSurface(__int64);
    SDL_Surface* GetSurface(__int64);
    SDL_Surface* GetSurface(int);
    int AddRef(__int64);
    int AddRef(int);
    __int64 LongName2Id(char*);
    char* Id2LongName(__int64);
    int GetGfxHeader(int, struct _GfxChunkInfo*);
    __int64 GetGfxShortId(int);

protected:
    GfxLib(void*, SDL_Renderer*, char*, int, int, int*);
    void ErrorProc(int);

    friend class GfxMain;

private:
    int CountGfxChunks(struct _uniChunk*, int);
    struct _GfxLibHeader* LoadHeader(SDL_RWops*);
    void ReadPaletteChunk(int, struct _PaletteInfo);
    void ReadNameChunk(int, struct _LongNameChunk);
    void* DeCompData(void*, struct _GfxChunkInfo, int);
    void* ConvertData(void*, int, char*, int, int, int, int, int);
    struct IDirectDrawSurface* FillSurface(int, struct _GfxChunkInfo, char*, struct IDirectDrawSurface*);
    struct IDirectDrawSurface* ReadPixelData(int, struct _GfxChunkInfo, char*, int);
    struct IDirectDrawPalette* ReadPalette(int, struct _GfxChunkInfo);
    void* ReadZBuffer(SDL_RWops*, struct _GfxChunkInfo);
    void* ReadAlphaBuffer(SDL_RWops*, struct _GfxChunkInfo);
    int ReadGfxChunk(SDL_RWops*, struct _GfxChunkHeader, int, int);
    int Load(SDL_RWops*, struct _GfxLibHeader*);
    int FindId(__int64);
    void RelSurface(int);

    std::map<__int64, SDL_Surface*> Surfaces;
};

#define L_LOCMEM 0

class GfxMain
{
public:
    GfxMain(SDL_Renderer*);
    ~GfxMain(void);
    int Restore(void);
    int LoadLib(char*, class GfxLib**, int);
    int ReleaseLib(class GfxLib*);
    void KillLib(class GfxLib*);
    int GetListId(class GfxLib*, __int64);
    struct _GfxStruct* GetSurface(int);
    int AddRef(int);
    class GfxLib* ReleaseSurface(int);

private:
    void ErrorProc(int);

    std::list<GfxLib> Libs;
};

class SB_CString
{
public:
    SB_CString(void);
    SB_CString(class SB_CString const&);
    SB_CString(int, int);
    SB_CString(char*);
    ~SB_CString(void);
    int operator==(class SB_CString const&);
	operator char*() { return Buffer; }
    class SB_CString const& operator=(class SB_CString const&);
    class SB_CString const& operator=(char const*);
    class SB_CString const& operator=(int b) { Empty(); *this += b; return *this; }
    class SB_CString const& operator=(char b) { Empty(); *this += b; return *this; }
    class SB_CString const& operator+=(char const*);
    class SB_CString const& operator+=(char);
    class SB_CString const& operator+=(class SB_CString const&);
    class SB_CString const& operator+=(int);
    class SB_CString& AttachChar(char);
    class SB_CString& DetachChar(char);
    class SB_CString& AddSlash(void);
    class SB_CString& RemoveSlash(void);
    void SplitPath(class SB_CString*, class SB_CString*, class SB_CString*, class SB_CString*);
    class SB_CString& MakePath(char*, char*, char*, char*);
    class SB_CString Mid(unsigned int) const;
    class SB_CString Mid(unsigned int, unsigned int) const;
    class SB_CString Right(unsigned int) const;
    class SB_CString Left(unsigned int) const;
    unsigned int ReverseFind(char) const;
    unsigned int Find(char const*) const;
    bool LoadStringA(unsigned int, struct HINSTANCE__*);
    bool LoadStringA(unsigned int, class SB_CDatabase*);
    void MakeUpper(void);
    void MakeLower(void);
    void Show(void) const;
	unsigned int Length() { return Anz; }

    static SB_CString& Format(char const*, ...);

protected:
    void AllocCopy(class SB_CString&, unsigned int, unsigned int, unsigned int) const;
    void AllocBuffer(unsigned int);
    void ConcatCopy(unsigned int, char const*, unsigned int, char const*);
    void ConcatInPlace(int, char const*);
    void AssignCopy(unsigned int, char const*);
    void Init(void);
    void Empty(void);
    void SafeDelete(char*);

private:
	unsigned int Anz;
	unsigned int Size;
	char* Buffer;
};

class SB_CHardwarecolorHelper
{
public:
   SB_CHardwarecolorHelper() : Color() { }
   SB_CHardwarecolorHelper(SB_CHardwarecolorHelper* color) : Color(reinterpret_cast<word&>(color)) { }

   operator word() { return Color; }

private:
   word Color;
};

typedef SB_CHardwarecolorHelper* SB_Hardwarecolor;

class SB_CBitmapCore
{
public:
    unsigned int AddAlphaMsk(void);
    unsigned int AddZBuffer(unsigned int, unsigned int);
    SB_Hardwarecolor GetHardwarecolor(unsigned int);
    unsigned int SetPixel(int, int, class SB_CHardwarecolorHelper*);
    unsigned int GetPixel(int, int);
    unsigned int Clear(class SB_CHardwarecolorHelper*, struct tagRECT const* = NULL);
    unsigned int Line(int, int, int, int, class SB_CHardwarecolorHelper*);
    unsigned int LineTo(int, int, class SB_CHardwarecolorHelper*);
    unsigned int Rectangle(const RECT*, class SB_CHardwarecolorHelper*);
    void InitClipRect(void);
    void SetClipRect(const RECT*);
    void SetClipRect(const CRect&);
    void SetColorKey(unsigned int);
    virtual unsigned int Release(void);
    unsigned int BlitFast(class SB_CBitmapCore*, int, int, const RECT* = NULL, unsigned short = 0);
    unsigned int BlitChar(SDL_Surface*, int, int, const RECT* = NULL, unsigned short = 0);
    unsigned int Blit(class SB_CBitmapCore*, int, int, const RECT* = NULL, unsigned short = 0, unsigned int = 0);
    int BlitA(class SB_CBitmapCore*, int, int, const RECT*, class SB_CHardwarecolorHelper*);
    int BlitA(class SB_CBitmapCore*, int, int, const RECT*);
    int BlitAT(class SB_CBitmapCore*, int, int, const RECT*, class SB_CHardwarecolorHelper*);
    int BlitAT(class SB_CBitmapCore*, int, int, const RECT*);

    unsigned int BlitT(class SB_CBitmapCore* bm, int x, int y, const RECT* rect = NULL, short flags = 16, unsigned int unk = 0) { return Blit(bm, x, y, rect, flags, unk); }
    unsigned int SetPixel(int x, int y, SLONG color) { return SetPixel(x, y, GetHardwarecolor(color)); }
    unsigned int Line(int x1, int y1, int x2, int y2, DWORD color) { return Line(x1, y1, x2, y2, GetHardwarecolor(color)); }
    SLONG GetXSize() { return Size.x; }
    SLONG GetYSize() { return Size.y; }
    RECT GetClipRect() { const SDL_Rect& r = lpDDSurface->clip_rect; return CRect(r.x, r.y, r.x + r.w, r.y + r.h); }
    SDL_Surface* GetSurface() { return lpDDSurface; }
    SDL_PixelFormat* GetPixelFormat(void) { return lpDDSurface->format; }

protected:
    virtual int Lock(struct _DDSURFACEDESC*) const;
    virtual int Unlock(struct _DDSURFACEDESC*) const;

    friend class SB_CBitmapMain;
    friend class SB_CBitmapKey;

    SDL_Renderer* lpDD;
    SDL_Surface* lpDDSurface;
    SDL_Texture* Texture;
    dword Unknown1[12];
    XY Size;
    dword Unknown2[5];
};

//static_assert(sizeof(SB_CBitmapCore) == 0x5Cu, "SB_CBitmapCore size check");

class SB_CCursor
{
public:
    SB_CCursor(class SB_CPrimaryBitmap*, class SB_CBitmapCore* = NULL);
    ~SB_CCursor(void);
    int Create(class SB_CBitmapCore*);
    int SetImage(class SB_CBitmapCore*);
    int MoveImage(int, int);
    int FlipBegin(void);
    int FlipEnd(void);
    int Show(bool);

private:
    int BlitImage(int, int);
    int RestoreBackground(struct SDL_Surface*);
    int SaveBackground(struct SDL_Surface*);
    int CreateBackground(void);
    int CreateSurface(struct SDL_Surface**, int, int);

    SB_CPrimaryBitmap* Primary;
    SB_CBitmapCore* Cursor;
    SDL_Surface* Background;
    XY Position;
    dword Unknown[22];
};

//static_assert(sizeof(SB_CCursor) == 0x6Cu, "SB_CCursor size check");

class SB_CPrimaryBitmap : public SB_CBitmapCore
{
public:
    SB_CPrimaryBitmap(void);
    ~SB_CPrimaryBitmap(void);

    int Create(SDL_Renderer**, SDL_Window*, unsigned short, int, int, unsigned char, unsigned short);
    virtual unsigned int Release(void);
    int Flip(void);
    void SetPos(struct tagPOINT&);
    struct IDirectDrawSurface* GetLastPage(void);

    void AssignCursor(SB_CCursor* c) { Cursor = c; }
    SDL_Window* GetPrimarySurface() { return Window; }
    bool FastClip(CRect clipRect, POINT* pPoint, RECT* pRect);

private:
    void Delete(void);

    SDL_Window* Window;
    dword Unknown[9];
    SB_CCursor* Cursor;
};

//static_assert(sizeof(SB_CPrimaryBitmap) == 0x88u, "SB_CPrimaryBitmap size check");

class SB_CBitmapMain
{
public:
    SB_CBitmapMain(SDL_Renderer*);
    ~SB_CBitmapMain(void);
    unsigned int Release(void);
    unsigned int CreateBitmap(SB_CBitmapCore**, GfxLib*, __int64, unsigned int);
    unsigned int CreateBitmap(SB_CBitmapCore**, int, int, unsigned int, unsigned int = 16, unsigned int = 0);
    unsigned int ReleaseBitmap(SB_CBitmapCore*);
    unsigned int DelEntry(SB_CBitmapCore*);

private:
    SDL_Renderer* Renderer;
    std::list<SB_CBitmapCore> Bitmaps;
    dword Unknown[3];
};

////static_assert(sizeof(SB_CBitmapMain) == 0x1Cu, "SB_CBitmapMain size check");

class SB_CBitmapKey
{
public:
    SB_CBitmapKey(class SB_CBitmapCore&);
    ~SB_CBitmapKey(void);

    SDL_Surface* Surface;
    dword Unknown[27];
    void* Bitmap;
    int lPitch;
};

//static_assert(sizeof(SB_CBitmapKey) == 0x78u, "SB_CBitmapKey size check");

typedef struct
{
    SB_CPrimaryBitmap* pBitmap;
    SB_CBitmapCore* pBitmapCore;
} CFRONTDATA;

#define TEC_FONT_LEFT      1
#define TEC_FONT_RIGHT     2
#define TEC_FONT_CENTERED  3

#define TAB_STYLE_LEFT     1
#define TAB_STYLE_DOT      2
#define TAB_STYLE_CENTER   3
#define TAB_STYLE_RIGHT    4

typedef struct tagTabs
{
    dword Style;
    dword Width;
} TABS;

//static_assert(sizeof(TABS) == 8, "TABS size check");

typedef struct tagTextStyle
{
    dword Unknown[3];
} TEXT_STYLE;

//static_assert(sizeof(TEXT_STYLE) == 12, "TEXT_STYLE size check");

class SB_CFont
{
public:
#pragma pack(push)
#pragma pack(1)
    struct FontHeader
    {
        word HeaderSize;
        word Unknown0;
        word Flags;
        word Width;
        word Height;
        dword Unknown1;
        dword BitDepth;
        word NumColors;
        word Unknown2;
        word LoChar;
        word HiChar;
        word Unknown3;
        dword szPixels;
        dword szColors;
        dword Unknown4;
        dword szFooter;
    };
#pragma pack(pop)

    SB_CFont(void);
    ~SB_CFont(void);
    void DrawTextA(class SB_CBitmapCore*, int, int, char*, int = 0, bool = false);
    void DrawTextWithTabs(class SB_CBitmapCore*, int, int, char*, int = 0, bool = false);
    int DrawTextBlock(class SB_CBitmapCore*, struct tagRECT*, char*, int = 0, int = 0, bool = false);
    int PreviewTextBlock(class SB_CBitmapCore*, struct tagRECT*, char*, int = 0, int = 0, bool = false);
    int DrawTextBlock(class SB_CBitmapCore*, int, int, int, int, char*, int = 0, int = 0, bool = false);
    int PreviewTextBlock(class SB_CBitmapCore*, int, int, int, int, char*, int = 0, int = 0, bool = false);
    int GetWidthAt(char*, int, char);
    int GetWordLength(char*, int);
    int GetWidth(char*, int);
    int GetWidth(char);
    bool Load(SDL_Renderer*, char*, struct HPALETTE__* = NULL);
    bool CopyMemToSurface(struct HPALETTE__*);
    void SetTabulator(struct tagTabs*, unsigned int);

    void SetLineSpace(float LineSpace) { this->LineSpace = LineSpace; }

protected:
    void Init(void);
    void Delete(void);
    bool GetSurface(struct _DDSURFACEDESC*);
    void ReleaseSurface(struct _DDSURFACEDESC*);
    bool DrawChar(char, bool);
    bool DrawWord(char*, int);
    unsigned char* GetDataPtr(void);
    bool CreateFontSurface(SDL_Renderer*);
    bool CopyBitmapToMem(struct tagCreateFont*);

private:
    FontHeader Header;
    dword Unknown0[4];
    SDL_Surface* Surface;
    SDL_Texture* Texture;
    BYTE* VarWidth;
    BYTE* VarHeight;
    bool Hidden;
    TABS* Tabulator;
    word NumTabs;
    XY Pos;
    XY Start;
    float LineSpace;
    SB_CBitmapCore* Bitmap;
};

//static_assert(sizeof(SB_CFont) == 0x70u, "SB_CFont size check");

class SB_CXList
{
public:
    SB_CXList(void);
    ~SB_CXList(void);
    unsigned short AddRef(void);
    unsigned short Release(void);
    unsigned short AddElementAtTop(void*);
    unsigned short AddElementAtBottom(void*);
    unsigned short AddElement(void*, unsigned short);
    bool DeleteElementAtTop(void);
    bool DeleteElementAtBottom(void);
    bool DeleteElement(unsigned short);
    void DeleteAllElements(void);
    bool DeleteElementAtValue(void*);
    bool Swap(unsigned short, unsigned short);
    bool SetElementOneBack(unsigned short);
    bool SetElementBack(unsigned short);
    bool SetElementOneAhead(unsigned short);
    bool SetElementAhead(unsigned short);

private:
    void Init(void);
    void Delete(void);

    dword Unknown[4];
};

//static_assert(sizeof(SB_CXList) == 0x10u, "SB_CXList size check");
