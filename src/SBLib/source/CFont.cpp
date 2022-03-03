#include "StdAfx.h"

SB_CFont::SB_CFont()
    : Surface(nullptr), Texture(nullptr), VarWidth(nullptr), VarHeight(nullptr), Hidden(false), Tabulator(nullptr), LineSpace(1.5F), Bitmap(nullptr) {}

SB_CFont::~SB_CFont() {
    if (Texture != nullptr) {
        SDL_DestroyTexture(Texture);
    }

    if (Surface != nullptr) {
        SDL_FreeSurface(Surface);
    }

    { delete[] VarWidth; }

    { delete[] VarHeight; }
}

bool SB_CFont::Load(SDL_Renderer * /*renderer*/, const char *path, struct HPALETTE__ * /*unused*/) {
    SDL_RWops *file = SDL_RWFromFile(path, "rb");
    if (SDL_RWread(file, &Header, sizeof(Header), 1) != 1) {
        SDL_RWclose(file);
        return false;
    }

    BYTE *pixels = new BYTE[Header.szPixels];
    if (SDL_RWread(file, pixels, 1, Header.szPixels) != Header.szPixels) {
        SDL_RWclose(file);
        return false;
    }

    auto *colors = new SDL_Color[Header.NumColors + 1];
    if (SDL_RWread(file, colors, 1, Header.szColors) != Header.szColors) {
        SDL_RWclose(file);
        return false;
    }

    // Some fonts will refer beyond the palette, set those pixels to transparent
    memset(colors + Header.NumColors, 0, sizeof(SDL_Color));

    VarWidth = new BYTE[0x100];
    if (SDL_RWread(file, VarWidth, 0x100, 1) != 1) {
        SDL_RWclose(file);
        return false;
    }

    VarHeight = new BYTE[0x100];
    if (SDL_RWread(file, VarHeight, 0x100, 1) != 1) {
        SDL_RWclose(file);
        return false;
    }

    word chars = Header.HiChar - Header.LoChar + 1;
    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(pixels, Header.Width, Header.Height * chars, 8, Header.Width, SDL_PIXELFORMAT_INDEX8);
    for (int i = 0; i < Header.NumColors; i++) {
        Swap(colors[i].r, colors[i].b); // Convert BGR to RGB
    }
    SDL_SetPaletteColors(surf->format->palette, colors, 0, Header.NumColors + 1);
    Surface = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGB565, 0);
    SDL_SetColorKey(Surface, SDL_TRUE, 0);
    SDL_FreeSurface(surf);
    delete[] colors;
    delete[] pixels;
    SDL_RWclose(file);
    return true;
}

SLONG SB_CFont::DrawTextBlock(class SB_CBitmapCore *bmp, RECT *block, const char *str, SLONG length, SLONG offset, bool hidden) {
    this->Bitmap = bmp;
    this->Hidden = hidden;
    if (length == 0) {
        length = strlen(str);
    }
    if (offset < length) {
        str = str + offset;
        length = length - offset;
        this->Pos.x = block->left;
        this->Start.x = this->Pos.x;
        this->Pos.y = block->top;
        while (length != 0) {
            SLONG i = GetWordLength(str, length);
            if (i > 0) {
                if (block->right < this->Pos.x + GetWidth(str, i)) {
                    this->Pos.y = this->Pos.y + SLONG(this->Header.Height * LineSpace);
                    this->Pos.x = block->left;
                }
                if (block->bottom < this->Pos.y + this->Header.Height) {
                    return offset;
                }
                if (!DrawWord(str, i)) {
                    return offset;
                }
            }
            switch (str[i]) {
            case '\0':
                break;
            case '\n':
                this->Pos.x = this->Start.x;
                i++;
                break;
            case '\r':
                this->Pos.y = this->Pos.y + this->Header.Height;
                i++;
                break;
            case ' ':
                this->Pos.x = this->Pos.x + GetWidth(' ');
                i++;
                break;
            case -0x4b:
                this->Pos.y = this->Pos.y + SLONG(this->Header.Height * LineSpace);
                this->Pos.x = this->Start.x;
                i++;
                break;
            default:
                i++;
            }
            length = length - i;
            if (length < 1) {
                return 0xffff;
            }
            str = str + i;
            offset = offset + i;
        }
    } else {
        offset = 0xffff;
    }
    return offset;
}

void SB_CFont::DrawTextWithTabs(class SB_CBitmapCore *bmp, SLONG x, SLONG y, const char *str, SLONG length, bool hidden) {
    int i = 0;
    this->Bitmap = bmp;
    this->Hidden = hidden;
    this->Pos.x = x;
    this->Start.x = x;
    this->Pos.y = y;
    if (length == 0) {
        length = strlen(str);
    }
    while (0 < length) {
        switch (*str) {
        case '\t':
            if (i < this->NumTabs && (this->Tabulator != nullptr)) {
                str++;
                switch (this->Tabulator[i].Style) {
                case 1:
                    this->Pos.x = this->Tabulator[i].Width;
                    break;
                case 2:
                    this->Pos.x = this->Tabulator[i].Width - GetWidthAt(str, length - 1, '.');
                    break;
                case 3:
                    this->Pos.x = this->Tabulator[i].Width - (GetWidthAt(str, length - 1, '\t') / 2);
                    break;
                case 4:
                    this->Pos.x = this->Tabulator[i].Width - GetWidthAt(str, length - 1, '\t');
                    break;
                default:
                    break;
                }
                i++;
            }
            break;
        case '\n':
            this->Pos.y = this->Pos.y + SLONG(this->Header.Height * LineSpace);
            str++;
            break;
        case '\r':
            i = 0;
            this->Pos.x = this->Start.x;
            str++;
            break;
        case -0x4b:
            i = 0;
            this->Pos.y = this->Pos.y + SLONG(this->Header.Height * LineSpace);
            this->Pos.x = this->Start.x;
            str++;
            break;
        default:
            DrawChar(*str++, true);
        }
        length--;
    }
}

void SB_CFont::SetTabulator(TABS *pTabs, ULONG szTabs) {
    if (this->Tabulator != nullptr) {
        delete[] this->Tabulator;
        this->Tabulator = nullptr;
    }
    this->NumTabs = static_cast<word>(szTabs / sizeof(TABS));
    this->Tabulator = new TABS[this->NumTabs];
    memcpy(this->Tabulator, pTabs, szTabs);
}

SLONG SB_CFont::PreviewTextBlock(class SB_CBitmapCore *bmp, RECT *block, const char *str, SLONG length, SLONG offset, bool hidden) {
    this->Bitmap = bmp;
    this->Hidden = hidden;
    if (length == 0) {
        length = strlen(str);
    }
    if (offset < length) {
        str = str + offset;
        length = length - offset;
        this->Pos.x = block->left;
        this->Start.x = this->Pos.x;
        this->Pos.y = block->top;
        while (length != 0) {
            int i = GetWordLength(str, length);
            if (0 < i) {
                SLONG width = GetWidth(str, i);
                if (block->right < this->Pos.x + width) {
                    this->Pos.y = this->Pos.y + SLONG(this->Header.Height * LineSpace);
                    this->Pos.x = block->left;
                    if (block->right < this->Pos.x + width) {
                        return -1;
                    }
                }
                this->Pos.x += width;
            }
            switch (str[i]) {
            case '\0':
                break;
            case '\n':
                this->Pos.y = this->Pos.y + SLONG(this->Header.Height * LineSpace);
                i++;
                break;
            case '\r':
                this->Pos.x = this->Start.x;
                i++;
                break;
            case ' ':
                this->Pos.x = this->Pos.x + GetWidth(' ');
                i++;
                break;
            case -0x4b:
                this->Pos.y = this->Pos.y + SLONG(this->Header.Height * LineSpace);
                this->Pos.x = this->Start.x;
                i++;
                break;
            default:
                i++;
            }
            length -= i;
            if (length < 1) {
                break;
            }
            str += i;
        }
        return (this->Pos.y + this->Header.Height) - block->top;
    }
    return 0xffff;
}

SLONG SB_CFont::DrawTextBlock(class SB_CBitmapCore *bmp, SLONG l, SLONG t, SLONG r, SLONG b, const char *str, SLONG length, SLONG offset, bool hidden) {
    RECT rect{l, t, r, b};
    return DrawTextBlock(bmp, &rect, str, length, offset, hidden);
}

SLONG SB_CFont::PreviewTextBlock(class SB_CBitmapCore *bmp, SLONG l, SLONG t, SLONG r, SLONG b, const char *str, SLONG length, SLONG offset, bool hidden) {
    RECT rect{l, t, r, b};
    return PreviewTextBlock(bmp, &rect, str, length, offset, hidden);
}

SLONG SB_CFont::GetWidthAt(const char *str, SLONG /*offset*/, char ch) {
    int i = 0;
    while (true) {
        switch (*str) {
        case -0x4b:
        case '\0':
        case '\t':
        case '\n':
        case '\r':
            return i;
        default:
            break;
        }
        if (*str == ch) {
            break;
        }
        i += GetWidth(*str++);
    }
    return i;
}

SLONG SB_CFont::GetWordLength(const char *str, SLONG offset) {
    dword length = 0;
    while (offset != 0) {
        switch (str[length]) {
        case -0x4b:
        case '\0':
        case '\t':
        case '\n':
        case '\r':
        case ' ':
            return length;
        default:
            length++;
            offset--;
        }
    }
    return length;
}

SLONG SB_CFont::GetWidth(unsigned char ch) {
    if ((this->Header.Flags & 1) == 1) {
        return *(this->VarWidth + ch);
    }
    return this->Header.Width;
}

SLONG SB_CFont::GetWidth(const char *str, SLONG offset) {
    SLONG width = 0;
    if (offset == 0) {
        offset = strlen(str);
    }
    while (offset != 0) {
        switch (*str) {
        case -0x4b:
        case '\0':
        case '\t':
        case '\n':
        case '\r':
            offset++;
            break;
        default:
            width = width + GetWidth(*str++);
            offset--;
        }
    }
    return width;
}

bool SB_CFont::DrawChar(unsigned char ch, bool /*unused*/) {
    if (this->VarHeight != nullptr) {
        if (this->Bitmap != (SB_CBitmapCore *)nullptr) {
            SDL_Rect srcRect;
            srcRect.x = 0;
            srcRect.y = (*(this->VarHeight + ch) - this->Header.LoChar) * this->Header.Height;
            srcRect.w = this->Header.Width;
            srcRect.h = this->Header.Height;
            if (!this->Hidden) {
                this->Bitmap->BlitChar(Surface, Pos.x, Pos.y, srcRect);
            }
        }
        this->Pos.x = this->Pos.x + GetWidth(ch);
        return true;
    }
    return false;
}

bool SB_CFont::DrawWord(const char *str, SLONG offset) {
    do {
        if (offset == 0) {
            return true;
        }
        if (*str == -0x4b) {
            this->Pos.y = this->Pos.y + SLONG(this->Header.Height * LineSpace);
            this->Pos.x = this->Start.x;
        } else {
            if (*str == '\n') {
                this->Pos.y = this->Pos.y + SLONG(this->Header.Height * LineSpace);
            } else {
                if (*str == '\r') {
                    this->Pos.x = this->Start.x;
                } else {
                    if (!DrawChar(*str, true)) {
                        return false;
                    }
                }
            }
        }
        str++;
        offset--;
    } while (true);
}
