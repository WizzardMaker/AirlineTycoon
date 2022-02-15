#include "StdAfx.h"
#include <SDL_image.h>

const char *ExcPrimarySurface = "CreatePrimarySurface failed";
const char *ExcAttachSurface = "GetAttachedSurface failed";
const char *ExcBackbuffer = "Create Backbuffer failed";
const char *ExcReresize = "Can't reresize TECPRIMARYBM";
const char *ExcAttachPal = "Can't attach Palette";
const char *ExcNotPcx = "%s is not a valid pcx-file!";
const char *ExcNotLbm = "%s is not a valid lbm-file!";
const char *ExcPicUnknown = "%s is not a known picture file!";

XYZ DetectCurrentDisplayResolution() {
    SDL_DisplayMode current;

    if (SDL_GetCurrentDisplayMode(0, &current) != 0) {
        TeakLibW_Exception(nullptr, 0, ExcPrimarySurface);
    }

    return XYZ(current.w, current.h, SDL_BITSPERPIXEL(current.format));
}

TECBM::TECBM() : Surface(nullptr), Size(0, 0) {}

TECBM::TECBM(CString const &path, void *flags) : Surface(nullptr), Size(0, 0) { ReSize(path, flags); }

TECBM::~TECBM() {
    if (Surface != nullptr) {
        SDL_FreeSurface(Surface);
    }
}

int TECBM::Refresh() {
    TeakLibW_Exception(nullptr, 0, ExcNotImplemented);
    return 0;
}

void TECBM::ReSize(CString const &path, void *flags) {
    const char *suffix = GetSuffix(path);
    if (!stricmp(suffix, "lbm") || !stricmp(suffix, "bbm")) {
        ReSizeLbm(path, flags);
    } else if (!stricmp(suffix, "pcx") || !stricmp(suffix, "pcc")) {
        ReSizePcx(path, flags);
    } else if (!stricmp(suffix, "tga")) {
        ReSizeTga(path, flags);
    } else {
        TeakLibW_Exception(nullptr, 0, ExcPicUnknown, path.c_str());
    }
}

void TECBM::ReSizeLbm(CString const &path, void * /*flags*/) {
    SDL_RWops *file = SDL_RWFromFile(path, "rb");
    if (file == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcPicUnknown, path.c_str());
    }
    if (IMG_isLBM(file) == 0) {
        SDL_RWclose(file);
        TeakLibW_Exception(nullptr, 0, ExcNotLbm, path.c_str());
    }

    Surface = IMG_LoadLBM_RW(file);
    SDL_RWclose(file);

    if (Surface == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcNotLbm, path.c_str());
    }

    Size.x = Surface->w;
    Size.y = Surface->h;
}

void TECBM::ReSizePcx(CString const &path, void * /*flags*/) {
    SDL_RWops *file = SDL_RWFromFile(path, "rb");
    if (file == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcPicUnknown, path.c_str());
    }
    if (IMG_isPCX(file) == 0) {
        SDL_RWclose(file);
        TeakLibW_Exception(nullptr, 0, ExcNotPcx, path.c_str());
    }

    Surface = IMG_LoadPCX_RW(file);
    SDL_RWclose(file);

    if (Surface == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcNotPcx, path.c_str());
    }

    Size.x = Surface->w;
    Size.y = Surface->h;
}

void TECBM::ReSizeTga(CString const &path, void * /*flags*/) {
    SDL_RWops *file = SDL_RWFromFile(path, "rb");
    if (file == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcPicUnknown, path.c_str());
    }

    Surface = IMG_LoadTGA_RW(file);
    SDL_RWclose(file);

    if (Surface == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcNotPcx, path.c_str());
    }

    Size.x = Surface->w;
    Size.y = Surface->h;
}

TECBMKEY::TECBMKEY(TECBM &bm) : Surface(bm.Surface), Bitmap(static_cast<UBYTE *>(bm.Surface->pixels)), lPitch(bm.Surface->pitch) {
    if (SDL_LockSurface(Surface) != 0) {
        TeakLibW_Exception(nullptr, 0, ExcAttachSurface);
    }
}

TECBMKEY::~TECBMKEY() { SDL_UnlockSurface(Surface); }

TECBMKEYC::TECBMKEYC(const TECBM &bm) : Surface(bm.Surface), Bitmap(static_cast<const UBYTE *>(bm.Surface->pixels)), lPitch(bm.Surface->pitch) {
    if (SDL_LockSurface(Surface) != 0) {
        TeakLibW_Exception(nullptr, 0, ExcAttachSurface);
    }
}

TECBMKEYC::~TECBMKEYC() { SDL_UnlockSurface(Surface); }

PALETTE::PALETTE() = default;

void PALETTE::RefreshPalFromLbm(CString const &path) {
    SDL_RWops *file = SDL_RWFromFile(path, "rb");
    if (file == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcPicUnknown, path.c_str());
    }
    if (IMG_isLBM(file) == 0) {
        SDL_RWclose(file);
        TeakLibW_Exception(nullptr, 0, ExcNotLbm, path.c_str());
    }

    SDL_Surface *surface = IMG_LoadLBM_RW(file);
    SDL_RWclose(file);

    if (surface == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcNotLbm, path.c_str());
    }

    SDL_Palette *palette = surface->format->palette;
    Pal.ReSize(palette->ncolors);
    for (int i = 0; i < palette->ncolors; ++i) {
        Pal[i] = (palette->colors)[i];
    }
    SDL_FreeSurface(surface);
}

void PALETTE::RefreshPalFromPcx(CString const &path) {
    SDL_RWops *file = SDL_RWFromFile(path, "rb");
    if (file == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcPicUnknown, path.c_str());
    }
    if (IMG_isPCX(file) == 0) {
        SDL_RWclose(file);
        TeakLibW_Exception(nullptr, 0, ExcNotPcx, path.c_str());
    }

    SDL_Surface *surface = IMG_LoadPCX_RW(file);
    SDL_RWclose(file);

    if (surface == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcNotPcx, path.c_str());
    }

    SDL_Palette *palette = surface->format->palette;
    Pal.ReSize(palette->ncolors);
    for (int i = 0; i < palette->ncolors; ++i) {
        Pal[i] = (palette->colors)[i];
    }
    SDL_FreeSurface(surface);
}

void PALETTE::RefreshPalFromTga(CString const &path) {
    SDL_RWops *file = SDL_RWFromFile(path, "rb");
    if (file == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcPicUnknown, path.c_str());
    }

    SDL_Surface *surface = IMG_LoadTGA_RW(file);
    SDL_RWclose(file);

    if (surface == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcNotPcx, path.c_str());
    }

    SDL_Palette *palette = surface->format->palette;
    Pal.ReSize(palette->ncolors);
    for (int i = 0; i < palette->ncolors; ++i) {
        Pal[i] = (palette->colors)[i];
    }
    SDL_FreeSurface(surface);
}
