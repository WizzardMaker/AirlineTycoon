//============================================================================================
// Intro.cpp : Der Render-Intro
//============================================================================================
#include "StdAfx.h"
#include "CVideo.h"
#include "Intro.h"
#include "gltitel.h"
#include <cmath>
#include <smacker.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern SB_CColorFX ColorFX;

//--------------------------------------------------------------------------------------------
// ULONG PlayerNum
//--------------------------------------------------------------------------------------------
CIntro::CIntro() : CVideo("intro.smk") {
    mWasIntroPlayed = FALSE;

    pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("titel.gli", RoomPath)), &mRoomLib, L_LOCMEM);

    mFadeFrom.ReSize(640, 480);
    mFadeFrom.FillWith(0);
    mFadeTo.ReSize(mRoomLib, GFX_TITEL);
}

//--------------------------------------------------------------------------------------------
// CIntro-Fenster zerstören:
//--------------------------------------------------------------------------------------------
CIntro::~CIntro() {
    Sim.Options.OptionViewedIntro = TRUE;
    mFadeFrom.Destroy();
    mFadeTo.Destroy();
}

/////////////////////////////////////////////////////////////////////////////
// CIntro message handlers
//--------------------------------------------------------------------------------------------
// void CIntro::OnPaint():
//--------------------------------------------------------------------------------------------
void CIntro::OnPaint() {
    if (pSmack != nullptr) {
        CVideo::OnPaint();

        if (State != SMK_MORE) {
            if (Sim.Options.OptionViewedIntro == 0) {
                mFadeFrom.BlitFrom(Bitmap, 320 - Width / 2, 240 - Height / 2);

                mFadeCount = AtGetTime();

                smk_close(pSmack);
                pSmack = nullptr;
            } else {
                Sim.Gamestate = GAMESTATE_BOOT;
            }
        }
    } else {
        if (AtGetTime() - mFadeCount < 1000) {
            SLONG Level = min(1000, AtGetTime() - mFadeCount) * 8 / 1000;
            ColorFX.ApplyOn2(Level, mFadeTo.pBitmap, 8 - Level, mFadeFrom.pBitmap, &PrimaryBm.PrimaryBm);
        } else {
            PrimaryBm.BlitFrom(mFadeTo);

            if (AtGetTime() - mFadeCount > 3000) {
                Sim.Gamestate = GAMESTATE_BOOT;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// void CIntro::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CIntro::OnVideoCancel() {
    if ((pSmack != nullptr) && Sim.Options.OptionViewedIntro == 0) {
        mFadeFrom.BlitFrom(Bitmap, 320 - Width / 2, 240 - Height / 2);

        mFadeCount = AtGetTime();

        smk_close(pSmack);
        pSmack = nullptr;
    } else {
        Sim.Gamestate = GAMESTATE_BOOT;
    }
}
