//============================================================================================
// Fracht.cpp : Der Fracht Schalter (neu im Add-On)
//============================================================================================
#include "StdAfx.h"
#include "AtNet.h"
#include "Fracht.h"
#include "glfracht.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Default-Position der Kiste im Frachtraum:
static const SLONG KistePos[6 * 2] = {
    103, 235, 286, 12, 268, 192, 30, -61, 26, 131, 181, 57,
};

// Offset von Zettel innerhalb der KistenBm
static const XY ZettelOffset[6] = {
    XY(19 - 26, 91 + 8), XY(19 - 26, 82 + 8), XY(26 - 26, 76 + 8), XY(15 - 26, 79 + 8), XY(22 - 26, 61 + 8), XY(28 - 26, 51 + 8),
};

// Offset des Seils relativ zur KistenBm
static const SLONG SeilXOffset[6] = {-10, -6, -4, -4, -4, -4};

extern SLONG PlayerMaxPassagiere;
extern SLONG PlayerMaxLength;
extern SLONG PlayerMinPassagiere;
extern SLONG PlayerMinLength;

void CalcPlayerMaximums(bool bForce = false);

CString ShortenLongCities(CString City);

//--------------------------------------------------------------------------------------------
// Die Schalter wird eröffnet:
//--------------------------------------------------------------------------------------------
CFrachtRaum::CFrachtRaum(BOOL bHandy, ULONG PlayerNum) : CStdRaum(bHandy, PlayerNum, "fracht.gli", GFX_FRACHT) {
    bCanPaint = false;

    SetRoomVisited(PlayerNum, ROOM_FRACHT);
    HandyOffset = 320;
    Sim.FocusPerson = -1;

    KommVar = -1;
    DropItNow = 0;

    if (bHandy == 0) {
        AmbientManager.SetGlobalVolume(60);
    }
    DefaultDialogPartner = TALKER_FRACHT;

    Sim.NetRefill(3);
    gFrachten.Refill();

    SP_Fracht.ReSize(13);
    SP_Fracht.Clips[0].ReSize(0, "fm1a.smk", "", XY(485, 50), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                              &KommVar, // Warten
                              "A4A4A9E1", 0, 1, 2, 4);
    SP_Fracht.Clips[1].ReSize(1, "fm1a.smk", "", XY(485, 50), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                              &KommVar, // Warten
                              "A4A4A9E1", 0, 1, 2, 4);
    SP_Fracht.Clips[2].ReSize(3, "fm3.smk", "", XY(485, 50), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                              &KommVar, // Warten
                              "A4A4A9E1", 0, 1, 2, 4);

    // Buzzer
    SP_Fracht.Clips[3].ReSize(2, "fm2.smk", "", XY(485, 50), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_DONTCANCEL, &DropItNow,
                              SMACKER_CLIP_SET | (SMACKER_CLIP_FRAME + 5 * SMACKER_CLIP_MULT), 1, nullptr, // Warten
                              "A4A4A9E1", 0, 1, 2, 4);

    // Nach vorne drehen:
    SP_Fracht.Clips[4].ReSize(4, "fm4.smk", "", XY(485, 50), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                              nullptr, // Warten->Speak/Listen
                              "A1E1", 5, 6);

    // Zuhören:
    SP_Fracht.Clips[5].ReSize(5, "fm4spas1.smk", "", XY(485, 50), SPM_LISTENING, CRepeat(1, 1), CPostWait(10, 30), SMACKER_CLIP_CANCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Listening
                              "A9A9A9E1E1", 5, 6, 7, 8, 12);
    SP_Fracht.Clips[6].ReSize(6, "fm4spas2.smk", "", XY(485, 50), SPM_LISTENING, CRepeat(1, 1), CPostWait(0, 3), SMACKER_CLIP_CANCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Listening
                              "A9E1E1", 5, 8, 12);
    SP_Fracht.Clips[7].ReSize(7, "fm4spas3.smk", "", XY(485, 50), SPM_LISTENING, CRepeat(1, 1), CPostWait(8, 20), SMACKER_CLIP_CANCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Listening
                              "A9E1E1", 5, 8, 12);

    // Babbeln:
    SP_Fracht.Clips[8].ReSize(8, "fmspa1.smk", "", XY(485, 50), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                              0, nullptr, // Talking
                              "A9A9A9E1E1", 9, 10, 11, 5, 12);
    SP_Fracht.Clips[9].ReSize(9, "fmspa2.smk", "", XY(485, 50), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                              0, nullptr, // Talking
                              "A9A9A9E1E1", 10, 10, 11, 5, 12);
    SP_Fracht.Clips[10].ReSize(10, "fmspa3.smk", "", XY(485, 50), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr,
                               SMACKER_CLIP_SET, 0, nullptr, // Talking
                               "A9A9A9E1E1", 9, 11, 11, 5, 12);
    SP_Fracht.Clips[11].ReSize(11, "fmspa4.smk", "", XY(485, 50), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr,
                               SMACKER_CLIP_SET, 0, nullptr, // Talking
                               "A9A9A9E1E1", 9, 10, 12, 5, 12);

    // Zurück zur Arbeit drehen:
    SP_Fracht.Clips[12].ReSize(12, "fm4back.smk", "", XY(485, 50), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                               0, nullptr, // Speak/Listen->Warten
                               "A1E1", 0, 6);

    SP_Stapler.ReSize(2);
    SP_Stapler.Clips[0].ReSize(0, "fa1.smk", "", XY(60, 240), SPM_IDLE, CRepeat(1, 1), CPostWait(10, 10), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                               nullptr, "A9", 1);
    SP_Stapler.Clips[1].ReSize(2, "fa2.smk", "", XY(0, 190), SPM_IDLE, CRepeat(1, 1), CPostWait(10, 10), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                               nullptr, "A9", 0);

    pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("tipau.gli", GliPath)), &pMenuLib, L_LOCMEM);
    TipBm.ReSize(pMenuLib, "BLOC1");
    MapPlaneBms[0].ReSize(pMenuLib, "PL_B00", 1 + 8);
    MapPlaneBms[1].ReSize(pMenuLib, "PL_V00", 1 + 8);
    MapPlaneBms[2].ReSize(pMenuLib, "PL_R00", 1 + 8);
    MapPlaneBms[3].ReSize(pMenuLib, "PL_J00", 1 + 8);
    MapPlaneBms[4].ReSize(pMenuLib, "PL_SW00", 1 + 8);

    NoGlueBm.ReSize(pRoomLib, "NOGLUE");
    StempelBm.ReSize(pRoomLib, "STEMPEL");
    KranMaskeBm.ReSize(pRoomLib, "MASK");

    SeileBms.ReSize(pRoomLib, "SEIL01", 3);
    KistenBms.ReSize(pRoomLib, "KISTE01", 6);

    KistenFx[0].ReInit("kiste1.raw");
    KistenFx[1].ReInit("kiste2.raw");
    WarningFx.ReInit("warning.raw");

    for (SLONG c = 0; c < gFrachten.AnzEntries(); c++) {
        RepaintZettel(c);
    }

    SetBackgroundFx(0, "pap3.raw", 25000); // Papierrascheln

    pZettelLib = nullptr;

    bCanPaint = true;
}

//--------------------------------------------------------------------------------------------
// Sehr destruktiv!
//--------------------------------------------------------------------------------------------
CFrachtRaum::~CFrachtRaum() {
    bCanPaint = false;

    NoGlueBm.Destroy();
    StempelBm.Destroy();
    KranMaskeBm.Destroy();

    TipBm.Destroy();
    MapPlaneBms[0].Destroy();
    MapPlaneBms[1].Destroy();
    MapPlaneBms[2].Destroy();
    MapPlaneBms[3].Destroy();
    MapPlaneBms[4].Destroy();

    for (SLONG c = 0; c < gFrachten.AnzEntries(); c++) {
        if (gFrachten[c].Praemie < -1) {
            gFrachten[c].Praemie = -1;
        }
    }

    Talkers.Talkers[TALKER_ARAB].DecreaseReference();

    if ((pMenuLib != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pMenuLib);
    }

    Sim.NetRefill(3);
    gFrachten.Refill();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CFrachtRaum message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// void CFrachtRaum::OnPaint()
//--------------------------------------------------------------------------------------------
void CFrachtRaum::OnPaint() {
    if (!bCanPaint) {
        return;
    }

    SLONG c = 0;
    XY RoomPos;
    BOOL RemoveTip = TRUE;
    BOOL IsOverPaper = FALSE;
    CPoint point = Sim.Players.Players[PlayerNum].CursorPos;

    static XY LastMouse;
    static SLONG LastTime;
    SLONG DeltaTime = AtGetTime() - LastTime;

    LastTime = AtGetTime();

    // Koordinaten für kleine Fenster konvertieren:
    ConvertMousePosition(point, &RoomPos);

    if (bHandy == 0) {
        SetMouseLook(CURSOR_NORMAL, 0, ROOM_FRACHT, 0);
    }

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    SP_Stapler.Pump();
    SP_Stapler.BlitAtT(RoomBm);

    SP_Fracht.Pump();
    SP_Fracht.BlitAtT(RoomBm);

    if (!(Sim.ItemGlue != 2 && (Sim.Players.Players[PlayerNum].HasItem(ITEM_GLUE) == 0))) {
        RoomBm.BlitFrom(NoGlueBm, 577, 396);
    }

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips();

    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
        if (gMousePosition.IfIsWithin(567, 386, 630, 429)) {
            if (Sim.ItemGlue != 2 && (Sim.Players.Players[PlayerNum].HasItem(ITEM_GLUE) == 0)) {
                SetMouseLook(CURSOR_HOT, 0, ROOM_FRACHT, 12);
            }
        } else if (gMousePosition.IfIsWithin(620, 0, 639, 439) || gMousePosition.IfIsWithin(440, 357, 639, 430)) {
            SetMouseLook(CURSOR_EXIT, 0, ROOM_FRACHT, 999);
        } else if (gMousePosition.IfIsWithin(465, 0, 639, 309)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_FRACHT, 11);
        }
        /*if (RoomPos.IfIsWithin (0,0,485,439))
          SetTip (&TipBm, MapPlaneBms, FALSE, XY (400,125), TIP_BUYFRACHT, -1, 0, 0); */
    }

    RoomBm.pBitmap->SetClipRect(CRect(0, 0, 640, 440));
    for (c = gFrachten.AnzEntries() - 1; c >= 0; c--) {
        if (gFrachten[c].Praemie >= 0) {
            if (RoomBm.BlitFromT(ZettelBms[c], KistePos[c * 2], KistePos[c * 2 + 1]) == 0) {
                RepaintZettel(c);
                RoomBm.BlitFromT(ZettelBms[c], KistePos[c * 2], KistePos[c * 2 + 1]);
            }
            RoomBm.BlitFromT(SeileBms[c / 2], KistePos[c * 2] + ZettelBms[c].Size.x / 2 + SeilXOffset[c], KistePos[c * 2 + 1] - SeileBms[c / 2].Size.y);
            RoomBm.BlitFromT(SeileBms[c / 2], KistePos[c * 2] + ZettelBms[c].Size.x / 2 + SeilXOffset[c], KistePos[c * 2 + 1] - SeileBms[c / 2].Size.y * 2);

            if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
                if (RoomPos.IfIsWithin(KistePos[c * 2], KistePos[c * 2 + 1] + 30, KistePos[c * 2] + ZettelBms[c].Size.x,
                                       KistePos[c * 2 + 1] + ZettelBms[c].Size.y)) {
                    RemoveTip = FALSE;
                    IsOverPaper = TRUE;

                    SetTip(&TipBm, MapPlaneBms, FALSE, XY(400, 125), TIP_BUYFRACHT, c, 0, 0);

                    SetMouseLook(CURSOR_HOT, 0, ROOM_TAFEL, 0);

                    if (c != LastTip) {
                        LastTip = c;

                        Sim.Players.Players[PlayerNum].CheckAuftragsBerater(gFrachten[c]);
                    }
                }
            }
        } else if (gFrachten[c].Praemie < -1) {
            if (gFrachten[c].Praemie != -1000 || (DropItNow != 0)) {
                if (gFrachten[c].Praemie == -1000 && (DropItNow != 0)) {
                    WarningFx.Stop();
                    WarningFx.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                    KommVar = -1;
                }

                gFrachten[c].Praemie += DeltaTime;
                if (gFrachten[c].Praemie > -1) {
                    gFrachten[c].Praemie = -1;
                }

                if (gFrachten[c].Praemie >= -1) {
                    SLONG r = rand() % 2;

                    KistenFx[r].Stop();
                    KistenFx[r].Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                }
            }

            XY Pos;
            SLONG p = 1000 + gFrachten[c].Praemie;

            Pos.x = KistePos[c * 2];
            Pos.y = SLONG(KistePos[c * 2 + 1] + __int64(p * p) * 600 / 1000000);

            RoomBm.BlitFromT(ZettelBms[c], Pos.x, Pos.y);
            RoomBm.BlitFromT(SeileBms[c / 2], Pos.x + ZettelBms[c].Size.x / 2 + SeilXOffset[c], Pos.y - SeileBms[c / 2].Size.y);
            RoomBm.BlitFromT(SeileBms[c / 2], Pos.x + ZettelBms[c].Size.x / 2 + SeilXOffset[c], Pos.y - SeileBms[c / 2].Size.y * 2);
            RoomBm.BlitFromT(StempelBm, Pos.x + 50 + 20 + 3 + ZettelOffset[c].x + ZettelBms[c].Size.x - 30 - 90 - 15,
                             Pos.y + 5 + 10 - 15 + 8 + ZettelOffset[c].y);
        }
    }
    if (IsOverPaper == 0) {
        LastTip = -1;
    }

    RoomBm.BlitFromT(KranMaskeBm, 333, 0);

    DropItNow = 0;

    RoomBm.pBitmap->SetClipRect(CRect(0, 0, 640, 480));

    if ((RemoveTip != 0) && !gMousePosition.IfIsWithin(0, 0, 485, 439) && LastMouse.IfIsWithin(0, 0, 485, 439)) {
        Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_AUFTRAG, "", MESSAGE_COMMENT);
    }

    LastMouse = gMousePosition;

    CStdRaum::PostPaint();
    CStdRaum::PumpToolTips();
}

//--------------------------------------------------------------------------------------------
// void CFrachtRaum::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CFrachtRaum::OnLButtonDown(UINT nFlags, CPoint point) {
    SLONG c = 0;
    XY RoomPos;
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    DefaultOnLButtonDown();

    if (ConvertMousePosition(point, &RoomPos) == 0) {
        CStdRaum::OnLButtonDown(nFlags, point);
        return;
    }

    if (PreLButtonDown(point) == 0) {
        if (MouseClickArea == ROOM_FRACHT && MouseClickId == 999) {
            qPlayer.LeaveRoom();
        }
        if (MouseClickArea == ROOM_FRACHT && MouseClickId == 11) {
            StartDialog(TALKER_FRACHT, MEDIUM_AIR, 1000);
        }
        if (MouseClickArea == ROOM_FRACHT && MouseClickId == 12) {
            if (Sim.ItemGlue == 0) {
                StartDialog(TALKER_FRACHT, MEDIUM_AIR, 1010);
            } else if (Sim.ItemGlue == 1 && (qPlayer.HasSpaceForItem() != 0)) {
                qPlayer.BuyItem(ITEM_GLUE);

                if (qPlayer.HasItem(ITEM_GLUE) != 0) {
                    Sim.ItemGlue = 2;
                    SIM::SendSimpleMessage(ATNET_TAKETHING, 0, ITEM_GLUE);
                }
            }
        }

        for (c = gFrachten.AnzEntries() - 1; c >= 0; c--) {
            if (gFrachten[c].Praemie >= 0) {
                if (RoomPos.IfIsWithin(KistePos[c * 2], KistePos[c * 2 + 1] + 30, KistePos[c * 2] + ZettelBms[c].Size.x,
                                       KistePos[c * 2 + 1] + ZettelBms[c].Size.y)) {
                    if (qPlayer.Frachten.GetNumFree() < 3) {
                        qPlayer.Frachten.ReSize(qPlayer.Frachten.AnzEntries() + 10);
                    }

                    gUniversalFx.Stop();
                    gUniversalFx.ReInit("paptake.raw");
                    gUniversalFx.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);

                    qPlayer.Frachten += gFrachten[c];
                    qPlayer.NetUpdateFreightOrder(gFrachten[c]);

                    KommVar = 3;

                    // Für den Statistikscreen:
                    qPlayer.Statistiken[STAT_FRACHTEN].AddAtPastDay(1);

                    gFrachten[c].Praemie = -1000;
                    qPlayer.NetUpdateTook(3, c);
                    break;
                }
            }
        }
    }

    CStdRaum::OnLButtonDown(nFlags, point);
}

//--------------------------------------------------------------------------------------------
// void CFrachtRaum::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CFrachtRaum::OnRButtonDown(UINT nFlags, CPoint point) {
    DefaultOnRButtonDown();

    // Außerhalb geklickt? Dann Default-Handler!
    if (point.x < WinP1.x || point.y < WinP1.y || point.x > WinP2.x || point.y > WinP2.y) {
        return;
    }

    if (MenuIsOpen() != 0) {
        MenuRightClick(point);
    } else {
        if ((IsDialogOpen() == 0) && point.y < 440) {
            Sim.Players.Players[PlayerNum].LeaveRoom();
        }

        CStdRaum::OnRButtonDown(nFlags, point);
    }
}

//--------------------------------------------------------------------------------------------
// void CFrachtRaum::RepaintZettel (SLONG n)
//--------------------------------------------------------------------------------------------
void CFrachtRaum::RepaintZettel(SLONG n) {
    ZettelBms[n].ReSize(KistenBms[n].Size);
    ZettelBms[n].BlitFrom(KistenBms[n]);

    ZettelBms[n].PrintAt(bprintf("%s-%s", (LPCTSTR)Cities[gFrachten[n].VonCity].Kuerzel, (LPCTSTR)Cities[gFrachten[n].NachCity].Kuerzel), FontSmallBlack,
                         TEC_FONT_CENTERED, XY(3, 10) + ZettelOffset[n], XY(ZettelBms[n].Size.x - 3, 29) + ZettelOffset[n]);

    ZettelBms[n].PrintAt(ShortenLongCities(Cities[gFrachten[n].VonCity].Name), FontSmallBlack, TEC_FONT_CENTERED, XY(3, 31) + ZettelOffset[n],
                         XY(ZettelBms[n].Size.x - 3, 102) + ZettelOffset[n]);
    ZettelBms[n].PrintAt("-", FontSmallBlack, TEC_FONT_CENTERED, XY(3, 41) + ZettelOffset[n], XY(ZettelBms[n].Size.x - 3, 102) + ZettelOffset[n]);
    ZettelBms[n].PrintAt(ShortenLongCities(Cities[gFrachten[n].NachCity].Name), FontSmallBlack, TEC_FONT_CENTERED, XY(3, 52) + ZettelOffset[n],
                         XY(ZettelBms[n].Size.x - 3, 102) + ZettelOffset[n]);
}

//--------------------------------------------------------------------------------------------
// Das Raster zum ausgrauen der ungültigen Tage zeichnen:
//--------------------------------------------------------------------------------------------
void CFracht::BlitGridAt(SBBM *pBitmap, XY Offset, BOOL Tagesansicht, SLONG Page) const {
    SLONG c = 0;

    if (Tagesansicht != 0) {
        if (Sim.Date + Page < Date || Sim.Date + Page > BisDate) {
            for (c = 0; c < 7; c++) {
                pBitmap->BlitFromT(FlugplanBms[51], Offset.x, Offset.y + FlugplanBms[51].Size.y * c);
            }
        }
    } else {
        for (c = 0; c < 7; c++) {
            if (Sim.Date + c < Date || Sim.Date + c > BisDate) {
                pBitmap->BlitFromT(FlugplanBms[51], Offset.x, Offset.y + FlugplanBms[51].Size.y * c);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Legt Städte fest: (AreaType: 0=Europa-Europa, 1=Region-gleicher Region, 2=alles
//--------------------------------------------------------------------------------------------
void CFracht::RandomCities(SLONG AreaType, SLONG HomeCity, TEAKRAND *pRand) {
    SLONG TimeOut = 0;

    do {
        switch (AreaType) {
        // Region-Gleiche Region:
        case 0: {
            SLONG Area = 1 + pRand->Rand(4);
            VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Area, pRand));
            NachCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Area, pRand));
        } break;

            // Europa:
        case 1:
            VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Cities[Sim.HomeAirportId].Areacode, pRand));
            NachCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Cities[Sim.HomeAirportId].Areacode, pRand));
            switch (pRand->Rand(2)) {
            case 0:
                VonCity = HomeCity;
                break;
            case 1:
                NachCity = HomeCity;
                break;
            default:
                hprintf("Fracht.cpp: Default case should not be reached.");
                DebugBreak();
            }
            break;

            // Irgendwas:
        case 2:
            VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(pRand));
            NachCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(pRand));
            break;

            // Die ersten Tage:
        case 4:
            VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Cities[Sim.HomeAirportId].Areacode, pRand));
            NachCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Cities[Sim.HomeAirportId].Areacode, pRand));
            switch (pRand->Rand(2)) {
            case 0:
                VonCity = HomeCity;
                break;
            case 1:
                NachCity = HomeCity;
                break;
            default:
                hprintf("Fracht.cpp: Default case should not be reached.");
                DebugBreak();
            }
            break;
        default:
            hprintf("Fracht.cpp: Default case should not be reached.");
            DebugBreak();
        }

        TimeOut++;

        if (VonCity < 0x1000000) {
            VonCity = Cities.GetIdFromIndex(VonCity);
        }
        if (NachCity < 0x1000000) {
            NachCity = Cities.GetIdFromIndex(NachCity);
        }

        if (TimeOut > 300 && VonCity != NachCity) {
            break;
        }
    } while (VonCity == NachCity || (AreaType == 4 && Cities.CalcDistance(VonCity, NachCity) > 10000000));
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag für den Spielbeginn ein:
//--------------------------------------------------------------------------------------------
void CFracht::RefillForBegin(SLONG AreaType, TEAKRAND *pRand) {
    SLONG TimeOut = 0;

    InPlan = 0;
    Okay = 1;

too_large:
    if (Sim.Difficulty == DIFF_ADDON03 && pRand->Rand(100) > 25) {
        NachCity = Sim.KrisenCity;

        do {
            VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(pRand));
        } while (VonCity == ULONG(Sim.KrisenCity));
    } else {
        do {
            RandomCities(AreaType, Sim.HomeAirportId, pRand);

            VonCity = Sim.HomeAirportId;

            if (Sim.Difficulty == DIFF_ADDON03 && pRand->Rand(100) > 25) {
                NachCity = Sim.KrisenCity;
            }
            if (Sim.Difficulty == DIFF_ADDON03 && VonCity == ULONG(Sim.KrisenCity)) {
                continue;
            }
        } while (VonCity == NachCity || Cities.CalcDistance(VonCity, NachCity) > 5000000);
    }

    Date = UWORD(Sim.Date);
    BisDate = UWORD(Sim.Date);
    Tons = 30;
    InPlan = 0;
    Praemie = ((CalculateFlightCost(VonCity, NachCity, 8000, 700, -1)) + 99) / 100 * 130;
    Strafe = Praemie / 2 * 100 / 100;

    SLONG Type = pRand->Rand(100);

    // Typ A = Normal, Gewinn möglich, etwas Strafe
    if (Type >= 0 && Type < 50) {
    }
    // Typ B = Hoffmann, Gewinn möglich, keine Strafe
    else if (Type >= 50 && Type < 60) {
        BisDate = UWORD(Sim.Date + 6);
    }
    // Typ C = Zeit knapp, viel Gewinn, viel Strafe
    else if (Type >= 70 && Type < 95) {
        Praemie *= 2;
        Strafe = Praemie * 2;
        BisDate = UWORD(Sim.Date + 1);
    }
    // Typ E = Glücksfall, viel Gewinn, keine Strafe
    else if (Type >= 95 && Type < 100) {
        Praemie *= 2;
        Strafe = 0;
    }

    if (AreaType == 1) {
        Praemie = Praemie * 3 / 2;
    }
    if (AreaType == 2) {
        Praemie = Praemie * 8 / 5;
    }
    if (Date != BisDate) {
        Praemie = Praemie * 4 / 5;
    }

    if (NachCity == ULONG(Sim.KrisenCity) && Sim.Difficulty == DIFF_ADDON03) {
        Praemie = 0;
        Strafe /= 2;
    }

    TimeOut++;

    if ((Cities.CalcDistance(VonCity, NachCity) > PlayerMaxLength) && TimeOut < 80) {
        goto too_large;
    }

    if ((Cities.CalcDistance(VonCity, NachCity) > PlayerMinLength) && TimeOut < 60) {
        goto too_large;
    }

    if ((Cities.CalcDistance(VonCity, NachCity) > PlayerMaxLength)) {
        Strafe = 0;
        Praemie = Praemie * 2;
        Date = static_cast<UWORD>(Sim.Date);
        BisDate = Date + 5;
    }

    TonsOpen = TonsLeft = Tons;
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag ein:
//--------------------------------------------------------------------------------------------
void CFracht::Refill(SLONG AreaType, TEAKRAND *pRnd) {
    SLONG TimeOut = 0;

    InPlan = 0;
    Okay = 1;

too_large:
    if (Sim.Difficulty == DIFF_ADDON03 && pRnd->Rand(100) > 25) {
        NachCity = Sim.KrisenCity;

        do {
            VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(pRnd));
        } while (VonCity == ULONG(Sim.KrisenCity));
    } else {
        RandomCities(AreaType, Sim.HomeAirportId, pRnd);
    }

    Tons = 30;
    Date = UWORD(Sim.Date + 1 + pRnd->Rand(3));
    BisDate = Date;
    InPlan = 0;

    // Kopie dieser Formel auch bei Last-Minute
    Praemie = ((CalculateFlightCost(VonCity, NachCity, 8000, 700, -1)) + 99) / 100 * 105;
    Strafe = Praemie / 2 * 100 / 100;

    if (pRnd->Rand(5) == 4) {
        BisDate += ((pRnd->Rand(5)));
    }

    SLONG Type = pRnd->Rand(100);

    // Typ A = Normal, Gewinn möglich, etwas Strafe
    if (Type >= 0 && Type < 50) {
    }
    // Typ B = Hoffmann, Gewinn möglich
    else if (Type >= 50 && Type < 60) {
        Date = UWORD(Sim.Date);
        BisDate = UWORD(Sim.Date + 4 + pRnd->Rand(3));
    }
    // Typ C = Zeit knapp, viel Gewinn, viel Strafe
    else if (Type >= 70 && Type < 80) {
        Praemie *= 2;
        Strafe = Praemie * 2;
        BisDate = Date = UWORD(Sim.Date + 1);
    }
    // Typ D = Betrug, kein Gewinn möglich, etwas Strafe
    else if (Type >= 80 && Type < 95) {
        Praemie /= 2;
    }
    // Typ E = Glücksfall, viel Gewinn, keine Strafe
    else if (Type >= 95 && Type < 100) {
        Praemie *= 2;
        Strafe = 0;
    }

    Type = pRnd->Rand(100);

    if (Type == 0) {
        Tons = 1;
        Praemie = Praemie * 4;
        Strafe = Praemie * 4;
    } else if (Type < 15 || (Sim.Date < 4 && Type < 30) || (Sim.Date < 8 && Type < 20) || (Sim.Difficulty == DIFF_TUTORIAL && Type < 70)) {
        Tons = 15;
        Praemie = Praemie * 3 / 4;
    } else if (Type < 40 || Sim.Difficulty == DIFF_TUTORIAL) {
        Tons = 30;
        /* Praemie bleibt gleich */
    } else if (Type < 70) {
        Tons = 40;
        Praemie = Praemie * 5 / 4;
    } else if (Type < 90) {
        Tons = 60;
        Praemie = Praemie * 6 / 4;
    } else {
        Tons = 100;
        Praemie = Praemie * 7 / 4;
    }

    if (AreaType == 1) {
        Praemie = Praemie * 3 / 2;
    }
    if (AreaType == 2) {
        Praemie = Praemie * 8 / 5;
    }
    if (Date != BisDate) {
        Date = static_cast<UWORD>(Sim.Date);
        Praemie = Praemie * 4 / 5;
    }

    if (NachCity == ULONG(Sim.KrisenCity) && Sim.Difficulty == DIFF_ADDON03) {
        Praemie = 0;
        Strafe /= 2;
    }

    if (Tons > 80) {
        Praemie *= 5;
    } else if (Tons > 60) {
        Praemie *= 4;
    } else if (Tons > 30) {
        Praemie *= 3;
    }

    if ((Cities.CalcDistance(VonCity, NachCity) > PlayerMaxLength) && TimeOut++ < 100) {
        goto too_large;
    }

    TonsOpen = TonsLeft = Tons;
}

//--------------------------------------------------------------------------------------------
// Passt der Auftrag in ein Flugzeug von der Sorte?
//--------------------------------------------------------------------------------------------
BOOL CFracht::FitsInPlane(const CPlane &Plane) const {
    if (Cities.CalcDistance(VonCity, NachCity) > Plane.ptReichweite * 1000) {
        return (FALSE);
    }
    if (Cities.CalcFlugdauer(VonCity, NachCity, Plane.ptGeschwindigkeit) >= 24) {
        return (FALSE);
    }

    return (TRUE);
}

//--------------------------------------------------------------------------------------------
// Ein CFracht-Datum speichern:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CFracht &Fracht) {
    File << Fracht.VonCity << Fracht.NachCity << Fracht.Tons << Fracht.TonsLeft << Fracht.TonsOpen << Fracht.Date << Fracht.InPlan << Fracht.Okay
         << Fracht.Praemie << Fracht.Strafe << Fracht.BisDate;
    return (File);
}

//--------------------------------------------------------------------------------------------
// Ein CFracht-Datum laden:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CFracht &Fracht) {
    File >> Fracht.VonCity >> Fracht.NachCity >> Fracht.Tons >> Fracht.TonsLeft >> Fracht.TonsOpen >> Fracht.Date >> Fracht.InPlan >> Fracht.Okay >>
        Fracht.Praemie >> Fracht.Strafe >> Fracht.BisDate;
    return (File);
}

//============================================================================================
// CFrachten::
//============================================================================================
// Fügt eine Reihe von neuen Aufträgen ein:
//============================================================================================
void CFrachten::Fill() {
    CalcPlayerMaximums();

    ReSize(6);

    for (auto &f : *this) {
        f.Refill(1, &Random);
    }

    if (Sim.Difficulty == DIFF_ATFS10 && Sim.Date >= 25 && Sim.Date <= 35) {
        for (auto &f : *this) {
            f.Praemie = -1;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag ein:
//--------------------------------------------------------------------------------------------
void CFrachten::Refill(SLONG Minimum) {
    SLONG c = 0;
    SLONG Anz = min(AnzEntries(), Sim.TickFrachtRefill);

    CalcPlayerMaximums();

    ReSize(6);

    for (auto &f : *this) {
        if (Anz <= 0) {
            break;
        }
        if (f.Praemie < 0) {
            if (Sim.Date < 5 && c < 5) {
                f.Refill(4, &Random);
            } else if (Sim.Date < 10 && c < 3) {
                f.Refill(4, &Random);
            } else {
                f.Refill(1, &Random);
            }

            Anz--;
        }
    }

    for (auto &f : *this) {
        if (Anz <= 0) {
            break;
        }
        if (f.Praemie != -1) {
            Minimum--;
        }
    }

    for (auto &f : *this) {
        if (Anz <= 0) {
            break;
        }
        if (f.Praemie < 0 && Minimum > 0) {
            if (Sim.Date < 5 && c < 5) {
                f.Refill(4, &Random);
            } else if (Sim.Date < 10 && c < 3) {
                f.Refill(4, &Random);
            } else {
                f.Refill(1, &Random);
            }

            Minimum--;
        }
    }

    Sim.TickFrachtRefill = 0;

    if (Sim.Difficulty == DIFF_ATFS10 && Sim.Date >= 25 && Sim.Date <= 35) {
        for (auto &f : *this) {
            f.Praemie = -1;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Returns the number of open Order flights which are due today:
//--------------------------------------------------------------------------------------------
SLONG CFrachten::GetNumDueToday() {
    SLONG Anz = 0;

    for (auto &f : *this) {
        if (f.BisDate >= Sim.Date) {
            if (f.TonsOpen > 0 && f.BisDate == Sim.Date) {
                Anz++;
            }
        }
    }

    return (Anz);
}

//--------------------------------------------------------------------------------------------
// Returns the number of open Order flights which still must be planned:
//--------------------------------------------------------------------------------------------
SLONG CFrachten::GetNumOpen() {
    SLONG Anz = 0;

    for (auto &f : *this) {
        if (f.BisDate >= Sim.Date) {
            if (f.TonsOpen > 0) {
                Anz++;
            }
        }
    }

    return (Anz);
}

//--------------------------------------------------------------------------------------------
// Speichert ein CFrachten Datum:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CFrachten &Frachten) {
    File << *((const ALBUM_V<CFracht> *)&Frachten);
    File << Frachten.Random;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Läd ein CFrachten Datum:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CFrachten &Frachten) {
    File >> *((ALBUM_V<CFracht> *)&Frachten);
    File >> Frachten.Random;

    return (File);
}

//============================================================================================
// Läßt nötigenfalls den Auftragsberater mit einem Spruch erscheien
//============================================================================================
void PLAYER::CheckAuftragsBerater(const CFracht &Fracht) {
    if (HasBerater(BERATERTYP_AUFTRAG) != 0) {
        SLONG d = 0;
        SLONG Okay = 0;

        for (d = 0, Okay = FALSE; d < Planes.AnzEntries(); d++) {
            if (Planes.IsInAlbum(d) != 0) {
                Okay |= Fracht.FitsInPlane(Planes[d]);
            }
        }
        // Okay|=Fracht.FitsInPlane (PlaneTypes[(SLONG)Planes[d].TypeId]);

        for (d = 0; d < Planes.AnzEntries(); d++) {
            if (Planes.IsInAlbum(d) != 0) {
                CPlane &qPlane = Planes[d];

                for (SLONG e = 0; e < qPlane.Flugplan.Flug.AnzEntries(); e++) {
                    if (qPlane.Flugplan.Flug[e].ObjectType == 3) {
                        CFlugplanEintrag &qFPE = qPlane.Flugplan.Flug[e];

                        if (qFPE.Startdate > Sim.Date || (qFPE.Startdate == Sim.Date && qFPE.Startzeit > Sim.GetHour() + 1)) {
                            if (Fracht.VonCity == qFPE.VonCity && Fracht.NachCity == qFPE.NachCity && Fracht.Date <= qFPE.Startdate &&
                                Fracht.BisDate >= qFPE.Startdate) {
                                // Fracht ersetzt Autoflug:
                                Messages.AddMessage(BERATERTYP_AUFTRAG, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 7113), (LPCTSTR)qPlane.Name), MESSAGE_COMMENT,
                                                    SMILEY_GREAT);
                                return;
                            }
                        }
                    }
                }
            }
        }

        if (Okay == 0) {
            // Kein Flugzeug, was die Entfernung schafft:
            Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7110), MESSAGE_COMMENT, SMILEY_BAD);
        } else {
            SLONG Cost = ((CalculateFlightCost(Fracht.VonCity, Fracht.NachCity, 8000, 700, -1)) + 99) / 100 * 100;

            // Bei den Kosten auch die wahrscheinliche Zahl der Flüge berücksichtigen:
            if (Fracht.Tons > 22) {
                Cost *= Fracht.Tons / 22;
            }

            if (Fracht.Praemie == 0) {
                Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7105), MESSAGE_COMMENT, SMILEY_GOOD);
            } else if (Fracht.Strafe == 0) {
                Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7100), MESSAGE_COMMENT, SMILEY_NEUTRAL);
            } else if (Cost <= Fracht.Praemie * 5 / 10) {
                Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7101), MESSAGE_COMMENT, SMILEY_GREAT);
            } else if (Cost <= Fracht.Praemie * 9 / 10) {
                Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7102), MESSAGE_COMMENT, SMILEY_GOOD);
            } else if (Cost <= Fracht.Praemie * 14 / 10) {
                Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7103), MESSAGE_COMMENT, SMILEY_NEUTRAL);
            } else {
                Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7104), MESSAGE_COMMENT, SMILEY_BAD);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
void CFracht::RefillForAusland(SLONG AreaType, SLONG CityNum, TEAKRAND *pRandom) {
    SLONG TimeOut = 0;

    InPlan = 0;
    Okay = 1;

    if (CityNum < 0x1000000) {
        CityNum = Cities.GetIdFromIndex(CityNum);
    }

too_large:
    do {
        VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(pRandom));
        NachCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(pRandom));
        switch (pRandom->Rand(2)) {
        case 0:
            VonCity = CityNum;
            break;
        case 1:
            NachCity = CityNum;
            break;
        default:
            hprintf("Fracht.cpp: Default case should not be reached.");
            DebugBreak();
        }
    } while (VonCity == NachCity || (AreaType == 4 && Cities.CalcDistance(VonCity, NachCity) > 10000000));

    Tons = 30;
    Date = UWORD(Sim.Date + 1 + pRandom->Rand(3));
    BisDate = Date;
    InPlan = 0;

    // Kopie dieser Formel auch bei Last-Minute
    Praemie = ((CalculateFlightCost(VonCity, NachCity, 8000, 700, -1)) + 99) / 100 * 105;
    Strafe = Praemie / 2 * 100 / 100;

    if (pRandom->Rand(5) == 4) {
        BisDate += ((pRandom->Rand(5)));
    }

    SLONG Type = pRandom->Rand(100);

    // Typ A = Normal, Gewinn möglich, etwas Strafe
    if (Type >= 0 && Type < 50) {
    }
    // Typ B = Hoffmann, Gewinn möglich
    else if (Type >= 50 && Type < 60) {
        Date = UWORD(Sim.Date);
        BisDate = UWORD(Sim.Date + 4 + pRandom->Rand(3));
    }
    // Typ C = Zeit knapp, viel Gewinn, viel Strafe
    else if (Type >= 70 && Type < 80) {
        Praemie *= 2;
        Strafe = Praemie * 2;
        BisDate = Date = UWORD(Sim.Date + 1);
    }
    // Typ D = Betrug, kein Gewinn möglich, etwas Strafe
    else if (Type >= 80 && Type < 95) {
        Praemie /= 2;
    }
    // Typ E = Glücksfall, viel Gewinn, keine Strafe
    else if (Type >= 95 && Type < 100) {
        Praemie *= 2;
        Strafe = 0;
    }

    Type = pRandom->Rand(100);

    if (Type == 0) {
        Tons = 1;
        Praemie = Praemie * 4;
        Strafe = Praemie * 4;
    } else if (Type < 15 || (Sim.Date < 4 && Type < 30) || (Sim.Date < 8 && Type < 20) || (Sim.Difficulty == DIFF_TUTORIAL && Type < 70)) {
        Tons = 15;
        Praemie = Praemie * 3 / 4;
    } else if (Type < 40 || Sim.Difficulty == DIFF_TUTORIAL) {
        Tons = 30;
        /* Praemie bleibt gleich */
    } else if (Type < 70) {
        Tons = 40;
        Praemie = Praemie * 5 / 4;
    } else if (Type < 90) {
        Tons = 60;
        Praemie = Praemie * 6 / 4;
    } else {
        Tons = 100;
        Praemie = Praemie * 7 / 4;
    }

    if (AreaType == 1) {
        Praemie = Praemie * 3 / 2;
    }
    if (AreaType == 2) {
        Praemie = Praemie * 8 / 5;
    }
    if (Date != BisDate) {
        Date = static_cast<UWORD>(Sim.Date);
        Praemie = Praemie * 4 / 5;
    }

    if (NachCity == ULONG(Sim.KrisenCity) && Sim.Difficulty == DIFF_ADDON03) {
        Praemie = 0;
        Strafe /= 2;
    }

    if (Tons > 80) {
        Praemie *= 5;
    } else if (Tons > 60) {
        Praemie *= 4;
    } else if (Tons > 30) {
        Praemie *= 3;
    }

    if ((Cities.CalcDistance(VonCity, NachCity) > PlayerMaxLength) && TimeOut++ < 100) {
        goto too_large;
    }

    TonsOpen = TonsLeft = Tons;
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
void CFrachten::RefillForAusland(SLONG CityNum, SLONG Minimum) {
    SLONG Anz = min(AnzEntries(), AuslandsFRefill[CityNum]);

    CalcPlayerMaximums();

    ReSize(6);
    FillAlbum();

    SLONG c = 0;
    for (auto &f : *this) {
        if (Anz <= 0) {
            break;
        }
        if (f.Praemie <= 0) {
            if (Sim.Date < 5 && c < 5) {
                f.RefillForAusland(4, CityNum, &Random);
            } else if (Sim.Date < 10 && c < 3) {
                f.RefillForAusland(4, CityNum, &Random);
            } else {
                f.RefillForAusland(1, CityNum, &Random);
            }

            Anz--;
        }
        c++;
    }

    for (auto &f : *this) {
        if (Anz <= 0) {
            break;
        }
        if (f.Praemie != 0) {
            Minimum--;
        }
    }

    c = 0;
    for (auto &f : *this) {
        if (Anz <= 0) {
            break;
        }
        if (f.Praemie <= 0 && Minimum > 0) {
            if (Sim.Date < 5 && c < 5) {
                f.RefillForAusland(4, CityNum, &Random);
            } else if (Sim.Date < 10 && c < 3) {
                f.RefillForAusland(4, CityNum, &Random);
            } else {
                f.RefillForAusland(1, CityNum, &Random);
            }

            Minimum--;
        }
    }

    AuslandsFRefill[CityNum] = 0;
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
void CFrachten::FillForAusland(SLONG CityNum) {
    SLONG c = 0;

    CalcPlayerMaximums();

    ReSize(6); // ex:10
    FillAlbum();

    for (auto &f : *this) {
        if (Sim.Date < 5 && c < 5) {
            f.RefillForAusland(4, CityNum, &Random);
        } else if (Sim.Date < 10 && c < 3) {
            f.RefillForAusland(4, CityNum, &Random);
        } else {
            f.RefillForAusland(1, CityNum, &Random);
        }
    }
}
