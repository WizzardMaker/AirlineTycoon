#include "StdAfx.h"

#define AT_Log(...) AT_Log_I("Rendering", __VA_ARGS__)

Uint16 get_pixel16(SDL_Surface *surface, SLONG x, SLONG y);

void put_pixel16(SDL_Surface *surface, SLONG x, SLONG y, Uint16 pixel);

SB_CBitmapMain::SB_CBitmapMain(SDL_Renderer *render) : Renderer(render) {}

SB_CBitmapMain::~SB_CBitmapMain() {
    for (auto &Bitmap : Bitmaps) {
        Bitmap.second.Release();
    }
}

ULONG SB_CBitmapMain::CreateBitmap(SB_CBitmapCore **out, GfxLib *lib, __int64 name, ULONG flags) {
    auto id = UniqueId++;
    auto res = Bitmaps.emplace(std::make_pair(id, id));
    if (!res.second) {
        assert(false);
        return 1;
    }
    auto it = res.first;
    SB_CBitmapCore *core = &(it->second);
    core->IncRef();
    SDL_Surface *surface = lib->GetSurface(name);
    if (surface != nullptr) {
        core->lpDD = Renderer;
        core->lpDDSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB565, 0);
        if ((flags & CREATE_USECOLORKEY) != 0U) {
            core->SetColorKey(0);
        }

        core->lpTexture = (Renderer != nullptr) && ((flags & CREATE_VIDMEM) != 0U) ? SDL_CreateTextureFromSurface(Renderer, core->lpDDSurface) : nullptr;
        core->Size.x = core->lpDDSurface->w;
        core->Size.y = core->lpDDSurface->h;
        core->InitClipRect();

        SDL_SetSurfaceRLE(core->lpDDSurface, SDL_TRUE);
    } else {
        core->lpDD = Renderer;
        core->lpDDSurface = nullptr;
        core->lpTexture = nullptr;
        core->Size.x = 0;
        core->Size.y = 0;
    }
    *out = core;
    return 0;
}

ULONG SB_CBitmapMain::CreateBitmap(SB_CBitmapCore **out, SLONG w, SLONG h, ULONG /*unused*/, ULONG flags, ULONG /*unused*/) {
    auto id = UniqueId++;
    auto res = Bitmaps.emplace(std::make_pair(id, id));
    if (!res.second) {
        assert(false);
        return 1;
    }
    auto it = res.first;
    SB_CBitmapCore *core = &(it->second);
    core->IncRef();
    core->lpDD = Renderer;

    int depth, format;
    if ((flags & CREATE_INDEXED) != 0U) {
        depth = 8;
        format = SDL_PIXELFORMAT_INDEX8;
    } else if ((flags & CREATE_USEALPHA) != 0U) {
        depth = 32;
        format = SDL_PIXELFORMAT_RGBA8888;
    } else {
        depth = 16;
        format = SDL_PIXELFORMAT_RGB565;
    }

    core->lpDDSurface = SDL_CreateRGBSurfaceWithFormat(0, w, h, depth, format);

    if ((flags & CREATE_USECOLORKEY) != 0U) {
        core->SetColorKey(0);
    }

    if (Renderer != nullptr && (flags & CREATE_VIDMEM) != 0U) {
        core->lpTexture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);

        if ((flags & (CREATE_USEALPHA | CREATE_USECOLORKEY)) != 0U) {
            SDL_SetTextureBlendMode(core->lpTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        core->lpTexture = nullptr;
    }

    core->Size.x = w;
    core->Size.y = h;
    core->InitClipRect();
    // SDL_SetSurfaceRLE(core->lpDDSurface, SDL_TRUE);
    *out = core;
    return 0;
}

ULONG SB_CBitmapMain::ReleaseBitmap(SB_CBitmapCore *core) {
    auto it = Bitmaps.find(core->getId());
    if (it == Bitmaps.end()) {
        assert(false);
        return 1;
    }
    assert(core == &(it->second));
    if (core->DecRef()) {
        core->Release();
        Bitmaps.erase(it);
    }
    return 0;
}

void SB_CBitmapCore::SetColorKey(ULONG key) { SDL_SetColorKey(lpDDSurface, SDL_TRUE, key); }

ULONG SB_CBitmapCore::Line(SLONG x1, SLONG y1, SLONG x2, SLONG y2, SB_Hardwarecolor hwcolor) {
    if (lpTexture != nullptr) {
        int access = 0;
        SDL_QueryTexture(lpTexture, nullptr, &access, nullptr, nullptr);
        if (access == SDL_TEXTUREACCESS_TARGET) {
            if (SDL_SetRenderTarget(lpDD, lpTexture) < 0) {
                return 1;
            }

            dword key = 0;
            auto color = (dword)hwcolor;
            SDL_GetColorKey(lpDDSurface, &key);
            SDL_SetRenderDrawColor(lpDD, (color & 0xFF0000) >> 16, (color & 0xFF00) >> 8, color & 0xFF,
                                   color == key ? SDL_ALPHA_TRANSPARENT : SDL_ALPHA_OPAQUE);
            SDL_RenderDrawLine(lpDD, x1, y1, x2, y2);
            return 0;
        }
    }

    // Bresenham's Line Algorithm
    int x = 0;
    int y = 0;
    int dx = 0;
    int dy = 0;
    int dx1 = 0;
    int dy1 = 0;
    int px = 0;
    int py = 0;
    int xe = 0;
    int ye = 0;
    int i = 0;
    dx = x2 - x1;
    dy = y2 - y1;
    dx1 = fabs(dx);
    dy1 = fabs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;
    if (dy1 <= dx1) {
        if (dx >= 0) {
            x = x1;
            y = y1;
            xe = x2;
        } else {
            x = x2;
            y = y2;
            xe = x1;
        }
        SetPixel(x, y, hwcolor);
        for (i = 0; x < xe; i++) {
            x = x + 1;
            if (px < 0) {
                px = px + 2 * dy1;
            } else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    y = y + 1;
                } else {
                    y = y - 1;
                }
                px = px + 2 * (dy1 - dx1);
            }
            SetPixel(x, y, hwcolor);
        }
    } else {
        if (dy >= 0) {
            x = x1;
            y = y1;
            ye = y2;
        } else {
            x = x2;
            y = y2;
            ye = y1;
        }
        SetPixel(x, y, hwcolor);
        for (i = 0; y < ye; i++) {
            y = y + 1;
            if (py <= 0) {
                py = py + 2 * dx1;
            } else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    x = x + 1;
                } else {
                    x = x - 1;
                }
                py = py + 2 * (dx1 - dy1);
            }
            SetPixel(x, y, hwcolor);
        }
    }
    return 0;
}

void SB_CBitmapCore::SetClipRect(const CRect &rect) {
    SDL_Rect clip = {rect.left, rect.top, rect.Width(), rect.Height()};
    SDL_SetClipRect(lpDDSurface, &clip);
}

SB_Hardwarecolor SB_CBitmapCore::GetHardwarecolor(ULONG color) {
#if 0
    SLONG r = GetHighestSetBit(Format.redMask) - GetHighestSetBit(0xFF0000);
    SLONG g = GetHighestSetBit(Format.greenMask) - GetHighestSetBit(0xFF00);
    SLONG b = GetHighestSetBit(Format.blueMask) - GetHighestSetBit(0xFF);

    SLONG result;
    if (r >= 0)
        result = Format.redMask & ((color & 0xFF0000) << r);
    else
        result = Format.redMask & ((color & 0xFF0000) >> -(char)r);
    if (g >= 0)
        result |= Format.greenMask & ((word)(color & 0xFF00) << g);
    else
        result |= Format.greenMask & ((color & 0xFF00) >> -(char)g);
    if (b >= 0)
        result |= Format.blueMask & ((unsigned char)color << b);
    else
        result |= Format.blueMask & ((dword)(unsigned char)color >> -(char)b);
    return (SB_Hardwarecolor)(result);
#else
    char r = (color & 0xFF0000) >> 16;
    char g = (color & 0xFF00) >> 8;
    char b = (color & 0xFF);
    return SB_Hardwarecolor(SDL_MapRGB(lpDDSurface->format, r, g, b));
#endif
}

SB_Hardwarecolor SB_CBitmapCore::GetHardwarecolor(char r, char g, char b) {
    return SB_Hardwarecolor(SDL_MapRGB(lpDDSurface->format, r, g, b));
}

ULONG SB_CBitmapCore::Clear(SB_Hardwarecolor hwcolor, const RECT *pRect) {
    auto color = (dword)hwcolor;
    if (SDL_MUSTLOCK(lpDDSurface) && SDL_LockSurface(lpDDSurface) < 0) {
        return 1;
    }
    if (lpTexture != nullptr) {
        if (SDL_SetRenderTarget(lpDD, lpTexture) < 0) {
            return 1;
        }

        dword key = 0;
        SDL_GetColorKey(lpDDSurface, &key);
        SDL_SetRenderDrawColor(lpDD, (color & 0xFF0000) >> 16, (color & 0xFF00) >> 8, color & 0xFF, color == key ? SDL_ALPHA_TRANSPARENT : SDL_ALPHA_OPAQUE);
    }

    if (pRect != nullptr) {
        const CRect &rect = *(const CRect *)pRect;
        SDL_Rect dst = {rect.left, rect.top, rect.Width(), rect.Height()};
        if (lpTexture != nullptr) {
            SDL_RenderFillRect(lpDD, &dst);
        }

        const int result = SDL_FillRect(lpDDSurface, &dst, color);

        if (SDL_MUSTLOCK(lpDDSurface)) {
            SDL_UnlockSurface(lpDDSurface);
        }

        return result;
    }

    if (lpTexture != nullptr) {
        SDL_RenderFillRect(lpDD, nullptr);
    }

    const int result = SDL_FillRect(lpDDSurface, nullptr, color);

    if (SDL_MUSTLOCK(lpDDSurface)) {
        SDL_UnlockSurface(lpDDSurface);
    }
    return result;
}

ULONG SB_CBitmapCore::SetPixel(SLONG x, SLONG y, SB_Hardwarecolor hwcolor) {
    if (SDL_MUSTLOCK(lpDDSurface) && SDL_LockSurface(lpDDSurface) < 0) {
        return 1;
    }
    Uint8 bpp = lpDDSurface->format->BytesPerPixel;
    Uint8 *p = static_cast<Uint8 *>(lpDDSurface->pixels) + y * lpDDSurface->pitch + x * bpp;

    if (lpDDSurface->format->format == SDL_PIXELFORMAT_INDEX8) {
        *static_cast<uint8_t *>(p) = (uint8_t)hwcolor;
    } else if (lpDDSurface->format->format == SDL_PIXELFORMAT_RGBA8888) {
        *reinterpret_cast<uint32_t *>(p) = (uint32_t)hwcolor;
    } else if (lpDDSurface->format->format == SDL_PIXELFORMAT_RGB565) {
        *reinterpret_cast<uint16_t *>(p) = (uint16_t)hwcolor;
    }

    if (SDL_MUSTLOCK(lpDDSurface)) {
        SDL_UnlockSurface(lpDDSurface);
    }
    return 0;
}

ULONG SB_CBitmapCore::GetPixel(SLONG x, SLONG y) {
    if (SDL_MUSTLOCK(lpDDSurface) && SDL_LockSurface(lpDDSurface) < 0) {
        return 1;
    }
    Uint8 bpp = lpDDSurface->format->BytesPerPixel;
    Uint8 bits = lpDDSurface->format->BitsPerPixel;
    Uint8 *p = static_cast<Uint8 *>(lpDDSurface->pixels) + y * lpDDSurface->pitch + x * bpp;
    dword result = *reinterpret_cast<Uint32 *>(p);
    if (SDL_MUSTLOCK(lpDDSurface)) {
        SDL_UnlockSurface(lpDDSurface);
    }
    return result & ((1 << bits) - 1);
}

Uint16 get_pixel16(SDL_Surface *surface, SLONG x, SLONG y) {
    // Convert the pixels to 32 bit
    auto *pixels = static_cast<Uint16 *>(surface->pixels);

    // Get the requested pixel
    return pixels[(y * surface->pitch / 2) + x];
}

void put_pixel16(SDL_Surface *surface, SLONG x, SLONG y, Uint16 pixel) {
    // Convert the pixels to 32 bit
    auto *pixels = static_cast<Uint16 *>(surface->pixels);

    // Set the pixel
    pixels[(y * surface->pitch / 2) + x] = pixel;
}

SDL_Surface *SB_CBitmapCore::GetFlippedSurface() {
    if (flippedBufferSurface != nullptr) {
        return flippedBufferSurface;
    }

    flippedBufferSurface =
        SDL_CreateRGBSurfaceWithFormat(lpDDSurface->flags, lpDDSurface->w, lpDDSurface->h, lpDDSurface->format->BitsPerPixel, lpDDSurface->format->format);

    if (SDL_MUSTLOCK(lpDDSurface)) {
        // Lock the surface
        SDL_LockSurface(lpDDSurface);
        SDL_LockSurface(flippedBufferSurface);
    }

    for (SLONG x = 0, rx = lpDDSurface->w - 1; x < lpDDSurface->w; x++, rx--) {
        // Go through rows
        for (SLONG y = 0, ry = lpDDSurface->h - 1; y < lpDDSurface->h; y++, ry--) {
            Uint16 pixel = get_pixel16(lpDDSurface, x, y);
            put_pixel16(flippedBufferSurface, rx, y, pixel);
        }
    }

    if (SDL_MUSTLOCK(lpDDSurface)) {
        // Lock the surface
        SDL_UnlockSurface(lpDDSurface);
        SDL_UnlockSurface(flippedBufferSurface);
    }

    Uint32 key = 0;
    if (SDL_GetColorKey(lpDDSurface, &key) == 0) {
        SDL_SetColorKey(flippedBufferSurface, 1, key);
    }

    return flippedBufferSurface;
}

ULONG SB_CBitmapCore::Blit(class SB_CBitmapCore *core, SLONG x, SLONG y) {
    if (!lpDDSurface || !core->lpDDSurface) {
        return 0;
    }

    SDL_Rect dst = {x, y, Size.x, Size.y};
    return SDL_BlitSurface(lpDDSurface, nullptr, core->lpDDSurface, &dst);
}

ULONG SB_CBitmapCore::Blit(class SB_CBitmapCore *core, SLONG x, SLONG y, const CRect &rect) {
    if (!lpDDSurface || !core->lpDDSurface) {
        return 0;
    }

    SDL_Rect src = {rect.left, rect.top, rect.Width(), rect.Height()};
    SDL_Rect dst = {x, y, rect.Width(), rect.Height()};
    return SDL_BlitSurface(lpDDSurface, &src, core->lpDDSurface, &dst);
}

ULONG SB_CBitmapCore::BlitFast(class SB_CBitmapCore* core, SLONG x, SLONG y) {
    if (!lpDDSurface || !core->lpDDSurface) {
        return 0;
    }

    // Ignore source color key
    Uint32 key = 0;
    int result = SDL_GetColorKey(lpDDSurface, &key);
    if (result != -1) {
        SDL_SetColorKey(lpDDSurface, SDL_FALSE, key);
    }

    SDL_Rect dst = {x, y, Size.x, Size.y};
    SDL_BlitSurface(lpDDSurface, nullptr, core->lpDDSurface, &dst);

    // Restore color key
    if (result != -1) {
        SDL_SetColorKey(lpDDSurface, SDL_TRUE, key);
    }
    return 0;
}

ULONG SB_CBitmapCore::BlitFast(class SB_CBitmapCore* core, SLONG x, SLONG y, const CRect& rect) {
    if(!lpDDSurface) {
        return 0;
    }

    // Ignore source color key
    Uint32 key = 0;
    int result = SDL_GetColorKey(lpDDSurface, &key);
    if (result != -1) {
        SDL_SetColorKey(lpDDSurface, SDL_FALSE, key);
    }

    SDL_Rect src = {rect.left, rect.top, rect.Width(), rect.Height()};
    SDL_Rect dst = {x, y, rect.Width(), rect.Height()};
    SDL_BlitSurface(lpDDSurface, &src, core->lpDDSurface, &dst);

    // Restore color key
    if (result != -1) {
        SDL_SetColorKey(lpDDSurface, SDL_TRUE, key);
    }
    return 0;
}

ULONG SB_CBitmapCore::BlitChar(SDL_Surface *font, SLONG x, SLONG y, const SDL_Rect &rect) {
    SDL_Rect dst = {x, y, rect.w, rect.h};
    return SDL_BlitSurface(font, &rect, lpDDSurface, &dst);
}

void SB_CBitmapCore::InitClipRect() { SDL_SetClipRect(lpDDSurface, nullptr); }

ULONG SB_CBitmapCore::Release() {
    if (lpDDSurface != nullptr) {
        SDL_FreeSurface(lpDDSurface);
    }
    if (flippedBufferSurface != nullptr) {
        SDL_FreeSurface(flippedBufferSurface);
    }
    if (lpTexture != nullptr) {
        SDL_DestroyTexture(lpTexture);
    }
    return 0;
}

bool SB_CPrimaryBitmap::FastClip(CRect clipRect, POINT *pPoint, RECT *pRect) {
    POINT offset;
    offset.x = 0;
    if (pRect->top <= 0) {
        offset.y = 0;
    } else {
        offset.y = pRect->top;
    }
    if ((offset.x != 0) || (offset.y != 0)) {
        OffsetRect(pRect, -offset.x, -offset.y);
    }
    if (pRect->right + pPoint->x >= clipRect.right) {
        pRect->right = clipRect.right - pPoint->x;
    }
    if (pPoint->x < clipRect.left) {
        pRect->left += clipRect.left - pPoint->x;
        pPoint->x = clipRect.left;
    }
    if (pRect->bottom + pPoint->y > clipRect.bottom) {
        pRect->bottom = clipRect.bottom - pPoint->y;
    }
    if (pPoint->y < clipRect.top) {
        pRect->top += clipRect.top - pPoint->y;
        pPoint->y = clipRect.top;
    }
    if ((offset.x != 0) || (offset.y != 0)) {
        OffsetRect(pRect, offset.x, offset.y);
    }
    return pRect->right - pRect->left > 0 && pRect->bottom - pRect->top > 0;
}

SLONG SB_CPrimaryBitmap::Flip() {
    if (lpDD != nullptr) {
        /*
         * None of the SDL renderers actually lock the GPU resource,
         * they all use either staging memory or a staging texture.
         * Thus we can still use the texture while it's locked and
         * we simply cycle through lock/unlock to update the texture.
         */
        SDL_UnlockTexture(lpTexture);
        if (SDL_LockTextureToSurface(lpTexture, nullptr, &lpDDSurface) < 0) {
            return -1;
        }
    } else {
        if (Cursor != nullptr) {
            Cursor->FlipBegin();
        }

        SDL_Rect target = SDL_Rect{TargetOffset.x, TargetOffset.y, TargetSize.x, TargetSize.y};
        if (SDL_BlitScaled(lpDDSurface, nullptr, SDL_GetWindowSurface(Window), &target) < 0) {
            return -2;
        }

        if (Cursor != nullptr) {
            Cursor->FlipEnd();
        }
    }

    return Present();
}

SLONG SB_CPrimaryBitmap::Present() {
    if (lpDD != nullptr) {
        SDL_SetRenderDrawColor(lpDD, 0, 0, 0, 255);
        SDL_RenderClear(lpDD);

        // Set the backbuffer as the render target
        if (SDL_SetRenderTarget(lpDD, nullptr) < 0) {
            return -1;
        }

        const SDL_Rect target = SDL_Rect{TargetOffset.x, TargetOffset.y, TargetSize.x, TargetSize.y};
        // Copy our primary texture to the backbuffer
        if (SDL_RenderCopy(lpDD, lpTexture, nullptr, &target) < 0) {
            return -2;
        }

        // Render the cursor onto the backbuffer
        if (Cursor != nullptr) {
            Cursor->Render(lpDD);
        }

        SDL_RenderPresent(lpDD);
    } else {
        if (SDL_UpdateWindowSurface(Window) < 0) {
            return -3;
        }
        SDL_Delay(10); // Ensure we don't run too fast without v-sync
    }
    return 0;
}

void SB_CPrimaryBitmap::SetTarget(XY offset, XY size) {
    this->TargetOffset = offset;
    this->TargetSize = size;
}

SLONG SB_CPrimaryBitmap::Create(SDL_Renderer **out, SDL_Window *Wnd, unsigned short /*flags*/, SLONG w, SLONG h, unsigned char /*unused*/,
                                unsigned short /*unused*/) {
    SDL_ClearError();

    Window = Wnd;
    lpDD = SDL_CreateRenderer(Window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    if (lpDD != nullptr) {
        AT_Log("Using hardware accelerated presentation");
        lpTexture = SDL_CreateTexture(lpDD, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, w, h);

        if (SDL_LockTextureToSurface(lpTexture, nullptr, &lpDDSurface) < 0) {
            AT_Log("Unable to lock backbuffer to surface");
            return -1;
        }
    } else {
        AT_Log("Falling back to software presentation");
        AT_Log("Reason for fallback: %s", SDL_GetError());
        SDL_ClearError();

        lpTexture = nullptr;
        lpDDSurface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 16, SDL_PIXELFORMAT_RGB565);
    }

    Size.x = w;
    Size.y = h;
    Cursor = nullptr;
    InitClipRect();
    *out = lpDD;
    return 0;
}

ULONG SB_CPrimaryBitmap::Release() {
    if (lpDD == nullptr) {
        if (lpDDSurface != nullptr) {
            SDL_FreeSurface(lpDDSurface);
            lpDDSurface = nullptr;
        }
        assert(lpTexture == nullptr);
    } else {
        if (lpTexture != nullptr) {
            SDL_DestroyTexture(lpTexture);
            lpTexture = nullptr;
        }
        SDL_DestroyRenderer(lpDD);
        lpDD = nullptr;
        lpDDSurface = nullptr;
    }
    return 0;
}

SB_CBitmapKey::SB_CBitmapKey(class SB_CBitmapCore &core) : Surface(core.lpDDSurface) {
    if (Surface == nullptr) {
        return;
    }
    if (SDL_MUSTLOCK(Surface)) {
        SDL_LockSurface(Surface);
    }
    Bitmap = Surface->pixels;
    lPitch = Surface->pitch;
}

SB_CBitmapKey::~SB_CBitmapKey() {
    if (Surface == nullptr) {
        return;
    }
    if (SDL_MUSTLOCK(Surface)) {
        SDL_UnlockSurface(Surface);
    }
}
