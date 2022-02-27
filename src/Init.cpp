//============================================================================================
// Init.cpp : Routinen zur initialisierung:
//============================================================================================
#include "StdAfx.h"
#include "Checkup.h"
#include "Editor.h"
#include "cd_prot.h"
#include <algorithm>
#include <fstream>

extern SLONG IconsPos[]; // Referenziert globe.cpp

extern BOOL gSpawnOnly;

extern CPlaneBuild gPlaneBuilds[37];
extern CPlanePartRelation gPlanePartRelations[307];

SB_CFont FontVerySmall;

extern char chRegKeyOld[];

DWORD GetKey();

//--------------------------------------------------------------------------------------------
// Initialisierung der Pfade:
//--------------------------------------------------------------------------------------------
void InitPathVars() {
    gSpawnOnly = FALSE;

    BitmapPath = "bitmaps\\%s";
    BrickPath = "brick\\%s";
    CityPath = "city\\%s";
    ClanPath = "clan\\%s";
    ExcelPath = "data\\%s";
    GliPath = "gli\\%s";
    MiscPath = "misc\\%s";
    ScanPath = "scans\\%s";
    SoundPath = "sound\\%s";
    RoomPath = "room\\%s";
    PlanePath = "planes\\%s";
    SmackerPath = "video\\%s";
    IntroPath = "intro\\%s";
    VoicePath = "voice\\%s";
    MyPlanePath = "myplanes\\%s";

    if (SavegamePath.GetLength() == 0) {
        SavegamePath = "Savegame\\%s";
    }
}

//--------------------------------------------------------------------------------------------
// Initialisiert die Digital-Sache:
//--------------------------------------------------------------------------------------------
void InitSoundSystem(SDL_Window *AppWnd) {
    gpSSE = new SSE(AppWnd, 44100, 1, 16);

    if (Sim.Options.OptionDigiSound != 0) {
        if (Sim.Options.OptionEnableDigi != 0) {
            gpSSE->EnableDS();
        }

        gpSSE->CreateFX(&gpClickFx);
        gpClickFx->Load(const_cast<char *>((LPCTSTR)FullFilename("click.raw", SoundPath)));
        gDoorOpen.ReInit("tuerauf.raw");
        gDoorClose.ReInit("tuerzu.raw");
        gMovePaper.ReInit("paper.raw");
        gArabDoorFx.ReInit("sabotage.raw");

        gMovePaper.Play();
        gMovePaper.Stop();

        // Die Ambiente-Sounds:
        AmbientManager.AmbientFx.ReSize(3);
        AmbientManager.SetFx(AMBIENT_PEOPLE, "raunen.raw");
        AmbientManager.SetFx(AMBIENT_JET_OUTSIDE, "jetfield.raw");
        AmbientManager.SetFx(AMBIENT_JET_FIELD, "jetout.raw");

        SetWaveVolume(Sim.Options.OptionMasterVolume);
    }

    if (gpSSE == nullptr || !gpSSE->IsSoundEnabled()) {
        Sim.Options.OptionMasterVolume = 0;
    }
}

//--------------------------------------------------------------------------------------------
// Sorgt für Screen Refresh und für Ablauf der Simulation:
//--------------------------------------------------------------------------------------------
void InitFonts() {
    FontCash.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("status.mcf", MiscPath)));
    FontSmallBlack.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("norm_bl.mcf", MiscPath)));
    FontSmallGrey.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("norm_gr.mcf", MiscPath)));
    FontSmallRed.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("norm_rt.mcf", MiscPath)));
    FontSmallWhite.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("norm_wh.mcf", MiscPath)));
    FontSmallWhiteX.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("norm_x.mcf", MiscPath)));
    FontSmallPlastic.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("norm_pl.mcf", MiscPath)));

    FontVerySmall.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("small_bl.mcf", MiscPath)));

    FontCondensedBlack.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("cond_bl.mcf", MiscPath)));

    FontDialog.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("dlgfont1.mcf", MiscPath)));
    FontDialogLight.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("dlgfont2.mcf", MiscPath)));
    FontDialogPartner.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("dlgfont1.mcf", MiscPath)));
    FontDialogInk.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("inkfont.mcf", MiscPath)));
    FontBigGrey.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("dlgfont3.mcf", MiscPath)));
    FontBigWhite.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("dlgfont3.mcf", MiscPath))); // ex:4

    FontNormalGreen.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("norm_ve.mcf", MiscPath)));
    FontNormalGrey.Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("norm_drk.mcf", MiscPath)));

    FontCash.SetLineSpace(1);
    FontSmallBlack.SetLineSpace(1);
    FontSmallGrey.SetLineSpace(1);
    FontSmallRed.SetLineSpace(1);
    FontSmallWhite.SetLineSpace(1);
    FontSmallWhiteX.SetLineSpace(1);
    FontSmallPlastic.SetLineSpace(1);
    FontDialog.SetLineSpace(1);
    FontDialogLight.SetLineSpace(1);
    FontDialogPartner.SetLineSpace(1);
    FontBigGrey.SetLineSpace(1);
    FontNormalGrey.SetLineSpace(1);

    // Determine Font colors on this video card:
    SBBM TestBm(20, 20);
    SLONG x = 0;
    SLONG y = 0;

    TestBm.Clear();
    TestBm.PrintAt("X", FontSmallBlack, TEC_FONT_LEFT, 0, 0, 20, 20);
    for (x = 0; x < 20; x++) {
        for (y = 0; y < 20; y++) {
            if (TestBm.GetPixel(x, y) != 0U) {
                ColorOfFontBlack = static_cast<UWORD>(TestBm.GetPixel(x, y));
            }
        }
    }

    TestBm.Clear();
    TestBm.PrintAt("X", FontSmallGrey, TEC_FONT_LEFT, 0, 0, 20, 20);
    for (x = 0; x < 20; x++) {
        for (y = 0; y < 20; y++) {
            if (TestBm.GetPixel(x, y) != 0U) {
                ColorOfFontGrey = static_cast<UWORD>(TestBm.GetPixel(x, y));
            }
        }
    }

    TestBm.Clear();
    TestBm.PrintAt("X", FontSmallRed, TEC_FONT_LEFT, 0, 0, 20, 20);
    for (x = 0; x < 20; x++) {
        for (y = 0; y < 20; y++) {
            if (TestBm.GetPixel(x, y) != 0U) {
                ColorOfFontRed = static_cast<UWORD>(TestBm.GetPixel(x, y));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erzeugt die Statuszeilen:
//--------------------------------------------------------------------------------------------
void InitStatusLines() {
    StatusLineBms.ReSize(pGLibStd, GFX_STATLIN0, GFX_STATLIN1, GFX_STATLIN2, GFX_STATLIN3, GFX_STATLIN4, GFX_STATLIN5, GFX_STATLIN6, NULL);

    gClockBms.ReSize(pGLibStd, GFX_CLOCK0, GFX_CLOCK1, GFX_CLOCK2, GFX_CLOCK3, GFX_OCLOCK0, GFX_OCLOCK1, GFX_OCLOCK2, GFX_OCLOCK3, GFX_OCLOCK4, GFX_OCLOCK5,
                     GFX_OCLOCK6, GFX_OCLOCK7, NULL);
}

//--------------------------------------------------------------------------------------------
// Lädt die Inventar-Bilder:
//--------------------------------------------------------------------------------------------
void InitItems() {
    gItemBms.ReSize(pGLibStd, "ITEM00", 26);
    LogoBms.ReSize(pGLibStd, "LOGO0", 7);
    SmallLogoBms.ReSize(pGLibStd, "SLOGO0", 4);
    TinyLogoBms.ReSize(pGLibStd, "TLOGO0", 4);
    MoodBms.ReSize(pGLibStd, GFX_MOOD00, GFX_MOOD01, GFX_MOOD02, GFX_MOOD03, GFX_MOOD04, GFX_MOOD10, GFX_MOOD11, GFX_MOOD12, GFX_MOOD13, GFX_MOOD14, GFX_MOOD15,
                   GFX_MOOD16, GFX_MOOD17, GFX_MOOD18, GFX_MOOD19, GFX_MOOD20, GFX_MOOD21, GFX_MOOD22, GFX_MOOD23, GFX_MOOD24, GFX_MOOD25, GFX_MOOD26,
                   GFX_MOOD27, GFX_MOOD28, GFX_MOOD29, GFX_MOOD30, GFX_MOOD31, GFX_MOOD32, GFX_MOOD33, GFX_MOOD34, GFX_MOOD35, GFX_MOOD36, GFX_MOOD37,
                   GFX_MOOD38, GFX_MOOD39, GFX_MOOD40, GFX_MOOD41, GFX_MOOD42, NULL);
    SmileyBms.ReSize(pGLibStd, "SMILEY0", 4);
    gTutoriumBms.ReSize(pGLibStd, "TUT01", 3);
    gRepeatMessageBms.ReSize(pGLibStd, "REPEAT01", 3);
    gStatisticBms.ReSize(pGLibStd, "STAT01", 3);
    gStatLineHigh.ReSize(pGLibStd, "TIMEHIGH GELDHIGH");

    // Tabellen importieren:
    if (bFirstClass == 0) {
        CString str;

        // Relations:
        {
            std::ifstream file(FullFilename("relation.csv", ExcelPath), std::ios_base::in);

            file >> str;

            for (auto &gPlanePartRelation : gPlanePartRelations) {
                file >> str;
                long id = atol(str);

                if (gPlanePartRelation.Id != id) {
                    hprintf(0, "Id mismatch: %li vs %li!", gPlanePartRelation.Id, id);
                }
                gPlanePartRelation.FromString(str);
            }
        }

        // Planebuilds:
        {
            std::ifstream file(FullFilename("builds.csv", ExcelPath), std::ios_base::in);

            file >> str;

            for (auto &gPlaneBuild : gPlaneBuilds) {
                file >> str;
                long id = atol(str);

                if (gPlaneBuild.Id != id) {
                    hprintf(0, "Id mismatch: %li vs %li!", gPlaneBuild.Id, id);
                }
                gPlaneBuild.FromString(str);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Lädt die Tip-Bitmaps:
//--------------------------------------------------------------------------------------------
void InitTipBms() {
    SLONG c = 0;

    gCursorFeetBms[0].ReSize(pGLibStd, "CWALKL01", 9, CREATE_VIDMEM);
    gCursorFeetBms[1].ReSize(pGLibStd, "CWALKR01", 9, CREATE_VIDMEM);

    gNotepadButtonL.ReSize(pGLibStd, GFX_NOTEB1);
    gNotepadButtonM.ReSize(pGLibStd, GFX_NOTEB3);
    gNotepadButtonR.ReSize(pGLibStd, GFX_NOTEB2);
    gZettelBms.ReSize(pGLibStd, "ZETTEL01", 3);

    gPostItBms.ReSize(pGLibStd, "POST_SR POST_SG POST_SR2 POST_SG2 POST_SB POST_SB2");

    FlugplanBms.ReSize(pGLibStd, "SCHED01", 52 + 5 + 2);
    gInfoBms.ReSize(pGLibStd, "INFO01", 2);

    FlugplanIconBms.ReSize(14);
    for (c = 0; c < 14; c++) {
        FlugplanIconBms[c].ReSize(pGLibStd, bprintf("I%c_00", "123456789ABCDEF"[c]), 2);
    }

    BeraterBms.ReSize(12 + 4);
    BeraterBms[0].ReSize(pGLibBerater, "BERA1_01", 4);
    BeraterBms[1].ReSize(pGLibBerater, "BERA2_01", 4);
    BeraterBms[8].ReSize(pGLibBerater, "BERA8_01", 4);
    BeraterBms[9].ReSize(pGLibBerater, "BERA9_01", 4);
    BeraterBms[12].ReSize(pGLibBerater, "BER91_01", 8);
    BeraterBms[13].ReSize(pGLibBerater, "BER92_01", 8);
    BeraterBms[14].ReSize(pGLibBerater, "BER93_01", 8);
    BeraterBms[15].ReSize(pGLibBerater, "BER94_01", 8);

    SprechblasenBms.ReSize(pGLibStd, "BLASE0", 6);
    XBubbleBms.ReSize(pGLibStd, "XBLASE01", 14);

    gCityMarkerBm.ReSize(pGLibStd, GFX_CITYCUR);
    gDialogBarBm.ReSize(pGLibStd, GFX_DIALOG);

    gToolTipBms.ReSize(pGLibStd, "TOOLTIP1", 3);
    gUniversalPlaneBms.ReSize(pGLibPlanes, "999SB 999SV 999SR 999SG "
                                           "FLUG07 FLUG08 FLUG09 FLUG10 FLUG11 "
                                           "FLUGB01 FLUGB02 FLUGB03 FLUGB06 FLUGB07 "
                                           "FLUGV01 FLUGV02 FLUGV03 FLUGV06 FLUGV07 "
                                           "FLUGR01 FLUGR02 FLUGR03 FLUGR06 FLUGR07 "
                                           "FLUGG01 FLUGG02 FLUGG03 FLUGG06 FLUGG07 FLUGB08 FLUGB99 FLUGB98 FLUG12");
    gEditorPlane2dBms.ReSize(pGLibStd, "P2D_B1 P2D_B2 P2D_B3 P2D_B4 P2D_B5 "
                                       "P2D_C1 P2D_C2 P2D_C3 P2D_C4 P2D_C5 "
                                       "P2D_H1 P2D_H2 P2D_H3 P2D_H4 P2D_H5 P2D_H6 P2D_H7 "
                                       "P2D_R1 P2D_R2 P2D_R3 P2D_R4 P2D_R5 P2D_R6 "
                                       "P2D_M1 P2D_M2 P2D_M3 P2D_M4 P2D_M5 P2D_M6 P2D_M7 P2D_M8 "
                                       "P2D_L1 P2D_L2 P2D_L3 P2D_L4 P2D_L5 P2D_L6 ");

    gSmokeBms.ReSize(pGLibStd, "SMOKE01", 1);
    gStenchBms.ReSize(pGLibStd, "STENCH01", 1);
}

//--------------------------------------------------------------------------------------------
// Initialisiert die Erdkugel:
//--------------------------------------------------------------------------------------------
void InitGlobeMapper() {
    int x = 0;
    int y = 0;
    int xs = 0;
    dword Color = 0;

    // Dafault-Positionen der Laptop-Fenster:
    GlobeWindows.ReSize(5);

    for (x = 0; x < 5; x++) {
        GlobeWindows[x] = XY(IconsPos[(x + 1) * 2 + 24] + 60, IconsPos[(x + 1) * 2 + 1 + 24] - 200 / 6);
    }

    SBBM TmpBm(10, 10);

    PALETTE EarthPal;
    EarthPal.RefreshPalFromLbm(const_cast<char *>((LPCTSTR)FullFilename("earthall.lbm", GliPath)));

    TECBM ShadeBm(const_cast<char *>((LPCTSTR)FullFilename("shade.pcx", GliPath)), SYSRAMBM);

    GlobeMixTab.ReSize(256 * 64);

    for (x = 0; x < 256; x++) {
        for (y = 0; y < 64; y++) {
            Color = std::min(EarthPal.Pal[x].b * (y + 5) / 40, 255);
            Color += std::min(EarthPal.Pal[x].r * (y + 5) / 40 + (std::max(EarthPal.Pal[x].b * (y + 5) / 40 - 255, 0)), 255) << 16;
            Color += std::min(EarthPal.Pal[x].g * (y + 5) / 40 + (std::max(EarthPal.Pal[x].b * (y + 5) / 40 - 255, 0)), 255) << 8;

            GlobeMixTab[x + (y << 8)] = (UWORD)TmpBm.pBitmap->GetHardwarecolor(Color);
            if (GlobeMixTab[x + (y << 8)] == 0) {
                GlobeMixTab[x + (y << 8)] = (UWORD)TmpBm.pBitmap->GetHardwarecolor(0x000008);
            }
        }
    }

    GlobeLight.ReSize(369);
    GlobeMapper.ReSize(185);
    GlobeMapperY.ReSize(369);

    TECBMKEYC ShadeKey(ShadeBm);
    const UBYTE *s = ShadeKey.Bitmap;

    for (y = 0; y < 369; y++) {
        xs = static_cast<SLONG>(sqrt(184 * 184 - (y - 184) * (y - 184)));

        GlobeLight[y].ReSize(xs * 2 + 1);
        if (y <= 184) {
            GlobeMapper[y].ReSize(xs * 2 + 1);
        }
        GlobeMapperY[y] = SLONG(asin((y - 184) / 184.0) / 3.14159 * 2 * 250 + 250);

        for (x = -xs; x <= xs; x++) {
            (GlobeLight[y])[x + xs] = s[184 + x];
            //(GlobeLight[y])[x+xs]=UBYTE(150-sqrt((x+80)*(x+80)+(y-80)*(y-80))/2.8);

            if (y <= 184) {
                (GlobeMapper[y])[x + xs] = UWORD(asin(x / double(xs)) / 3.14159 * 2 * 128 + 128);
            }
        }

        s += ShadeKey.lPitch;
    }
}
