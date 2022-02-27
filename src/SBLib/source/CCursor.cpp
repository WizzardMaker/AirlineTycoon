#include "StdAfx.h"

SB_CCursor::SB_CCursor(class SB_CPrimaryBitmap* primary, class SB_CBitmapCore* cursor)
    : Primary(primary)
    , Cursor(cursor)
      , Background(nullptr)
{
    CreateBackground();
}

SB_CCursor::~SB_CCursor()
{
    if (Background != nullptr) {
        SDL_FreeSurface(Background);
}
}

SLONG SB_CCursor::MoveImage(SLONG x, SLONG y)
{
    Position.x = x;
    Position.y = y;
    return 0;
}

SLONG SB_CCursor::SetImage(class SB_CBitmapCore* cursor)
{
    Cursor = cursor;
    CreateBackground();
    return 0;
}

SLONG SB_CCursor::FlipBegin()
{
    if (SaveBackground(Background) == 0) {
        return BlitImage(Position.x, Position.y);
}
    return -1;
}

SLONG SB_CCursor::FlipEnd()
{
    return RestoreBackground(Background);
}

SLONG SB_CCursor::Show(bool show)
{
    if (Cursor == nullptr) {
        return -1;
}

    return SDL_ShowCursor(!show ? SDL_ENABLE : SDL_DISABLE);
}

SLONG SB_CCursor::Render(SDL_Renderer* renderer)
{
    SDL_Rect dst = { Position.x, Position.y, Cursor->GetXSize(), Cursor->GetYSize() };
    if (renderer != nullptr) {
        return SDL_RenderCopy(renderer, Cursor->GetTexture(), nullptr, &dst);
}
    return -1;
}

SLONG SB_CCursor::BlitImage(SLONG x, SLONG y)
{
    if (Cursor == nullptr) {
        return -1;
}

    return Cursor->Blit(Primary, x, y);
}

SLONG SB_CCursor::RestoreBackground(struct SDL_Surface* surf)
{
    if (Cursor == nullptr) {
        return -1;
}

    SDL_Rect rect = { Position.x, Position.y, Cursor->GetXSize(), Cursor->GetYSize() };
    return SDL_BlitSurface(surf, nullptr, Primary->GetSurface(), &rect);
}

SLONG SB_CCursor::SaveBackground(struct SDL_Surface* surf)
{
    if (Cursor == nullptr) {
        return -1;
}

    SDL_Rect src = { Position.x, Position.y, Cursor->GetXSize(), Cursor->GetYSize() };
    SDL_Rect dst = { 0, 0, Cursor->GetXSize(), Cursor->GetYSize() };
    return SDL_BlitSurface(Primary->GetSurface(), &src, surf, &dst);
}

SLONG SB_CCursor::CreateBackground()
{
    if (Cursor == nullptr) {
        return -1;
}

    SLONG w = Cursor->GetXSize();
    SLONG h = Cursor->GetYSize();
    if (Background == nullptr)
    {
        return CreateSurface(&Background, w, h);
    }
    if (w != Background->w || h != Background->h)
    {
        SDL_FreeSurface(Background);
        return CreateSurface(&Background, w, h);
    }
    return 0;
}

SLONG SB_CCursor::CreateSurface(struct SDL_Surface** out, SLONG w, SLONG h)
{
    *out = SDL_CreateRGBSurfaceWithFormat(0, w, h, 16, SDL_PIXELFORMAT_RGB565);
    return *out != nullptr ? 0 : -1;
}
