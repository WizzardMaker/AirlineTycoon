//============================================================================================
// Reise.cpp : Der Reisebuero Schalter
//============================================================================================
// Link: "reise.h"
//============================================================================================
#include "StdAfx.h"
#include "AtNet.h"
#include "Reise.h"
#include "glreiseb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const SLONG ZettelPos[10 * 2] = {965 - 466, 79,  965 - 526, 212, 965 - 545, 116, 965 - 433, 250, 965 - 512, 289,
                                        965 - 430, 150, 965 - 346, 331, 965 - 40,  113, 965 - 449, 126, 965 - 245, 307};

extern SLONG timeReisClose;

//////////////////////////////////////////////////////////////////////////////////////////////
// Museum Konstruktion, Initialisation, Destruction
//////////////////////////////////////////////////////////////////////////////////////////////


//--------------------------------------------------------------------------------------------
// Bei Cities mit zwei Leerzeichen (Palm de Mallorca und Rio de Janeiro) wird nur das erste Wort genommen:
//--------------------------------------------------------------------------------------------
CString ShortenLongCities(CString City);

CString ShortenLongCities(CString City) {
    City.TrimLeft();
    City.TrimRight();

    if (strchrcount(City, ' ') == 2) {
        for (SLONG c = 0; c < City.GetLength(); c++) {
            if (City[c] == ' ') {
                return City.Left(c);
            }
        }
    }

    return (City);
}

//--------------------------------------------------------------------------------------------
// Konstruktor:
//--------------------------------------------------------------------------------------------
CReisebuero::CReisebuero(BOOL bHandy, ULONG PlayerNum) : CStdRaum(bHandy, PlayerNum, "reiseb.gli", GFX_REISEB) {
    SLONG c = 0;

    SetRoomVisited(PlayerNum, ROOM_REISEBUERO);

    Sim.FocusPerson = -1;

    KommVar = KommVar2 = -1;
    KommMonitor = -1;

    AmbientManager.SetGlobalVolume(60);

    Sim.NetRefill(2);
    ReisebueroAuftraege.RefillForReisebuero();

    SP_Girl.ReSize(4);
    //--------------------------------------------------------------------------------------------
    SP_Girl.Clips[0].ReSize(0, "rbwait.smk", "", XY(200, 227), SPM_IDLE, CRepeat(1, 1), CPostWait(40, 80), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, "A3A4A4A9", 0, 1, 2, 3);
    SP_Girl.Clips[1].ReSize(1, "rbdrehh.smk", "", XY(200, 227), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, "A1", 0);
    SP_Girl.Clips[2].ReSize(2, "rbdrehf.smk", "", XY(200, 227), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, "A1", 0);
    SP_Girl.Clips[3].ReSize(3, "rbtaste.smk", "", XY(200, 227), SPM_IDLE, CRepeat(1, 1), CPostWait(10, 30), SMACKER_CLIP_CANCANCEL, &KommMonitor,
                            SMACKER_CLIP_SET | (SMACKER_CLIP_FRAME + 4 * SMACKER_CLIP_MULT), 0, nullptr, "A1", 0);

    SP_Monitor.ReSize(5);
    //--------------------------------------------------------------------------------------------
    SP_Monitor.Clips[0].ReSize(0, "", "", XY(170, 318), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0, nullptr,
                               "A5A5A5A5", 1, 2, 3, 4);
    SP_Monitor.Clips[1].ReSize(1, "monir.smk", "", XY(170, 318), SPM_IDLE, CRepeat(1, 1), CPostWait(999, 999), SMACKER_CLIP_CANCANCEL, &KommMonitor,
                               SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &KommMonitor, "A1", 0);
    SP_Monitor.Clips[2].ReSize(2, "monire.smk", "", XY(170, 318), SPM_IDLE, CRepeat(1, 1), CPostWait(999, 999), SMACKER_CLIP_CANCANCEL, &KommMonitor,
                               SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &KommMonitor, "A1", 0);
    SP_Monitor.Clips[3].ReSize(3, "monirc.smk", "", XY(170, 318), SPM_IDLE, CRepeat(1, 1), CPostWait(999, 999), SMACKER_CLIP_CANCANCEL, &KommMonitor,
                               SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &KommMonitor, "A1", 0);
    SP_Monitor.Clips[4].ReSize(4, "monird.smk", "", XY(170, 318), SPM_IDLE, CRepeat(1, 1), CPostWait(999, 999), SMACKER_CLIP_CANCANCEL, &KommMonitor,
                               SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &KommMonitor, "A1", 0);

    // Change: Jetzt auch im freien Spiel:
    if (Sim.Difficulty > 0 || Sim.Difficulty == DIFF_FREEGAME) {
        SP_Schlange.ReSize(3);
        //--------------------------------------------------------------------------------------------
        SP_Schlange.Clips[0].ReSize(0, "snakw.smk", "", XY(564, 213), SPM_IDLE, CRepeat(1, 1), CPostWait(10, 30), SMACKER_CLIP_CANCANCEL, &KommVar,
                                    SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &KommVar, "A9A2", 0, 1);
        SP_Schlange.Clips[1].ReSize(1, "snakzung.smk", "snakzung.raw", XY(564, 213), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                    SMACKER_CLIP_SET, 0, &KommVar, "A1", 0);
        SP_Schlange.Clips[2].ReSize(2, "snakeat.smk", "snakeat.raw", XY(494, 194), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, &KommVar,
                                    SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &KommVar, "A1", 0);

        if (Sim.Players.Players[Sim.localPlayer].HasItem(ITEM_SPINNE) == 0) {
            SP_Spinne.ReSize(3);
            //--------------------------------------------------------------------------------------------
            SP_Spinne.Clips[0].ReSize(0, "blasew.smk", "", XY(380, 320), SPM_IDLE, CRepeat(3, 3), CPostWait(10, 30), SMACKER_CLIP_CANCANCEL, nullptr,
                                      SMACKER_CLIP_SET, 0, nullptr, "A9A1A2", 0, 1, 2);
            SP_Spinne.Clips[1].ReSize(1, "spider.smk", "", XY(380, 320), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, &KommVar,
                                      SMACKER_CLIP_SET | (SMACKER_CLIP_FRAME + 11 * SMACKER_CLIP_MULT), 2, &KommVar2, "A1", 2);
            SP_Spinne.Clips[2].ReSize(2, "blasew.smk", "", XY(380, 320), SPM_IDLE, CRepeat(9, 9), CPostWait(99, 99), SMACKER_CLIP_CANCANCEL, &KommVar2,
                                      SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, nullptr, "A9A2A1", 0, 1, 2);
        }
    }

    pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("tipau.gli", GliPath)), &pMenuLib, L_LOCMEM);
    TipBm.ReSize(pMenuLib, "BLOC1");
    MapPlaneBms[0].ReSize(pMenuLib, "PL_B00", 1 + 8);
    MapPlaneBms[1].ReSize(pMenuLib, "PL_V00", 1 + 8);
    MapPlaneBms[2].ReSize(pMenuLib, "PL_R00", 1 + 8);
    MapPlaneBms[3].ReSize(pMenuLib, "PL_J00", 1 + 8);
    MapPlaneBms[4].ReSize(pMenuLib, "PL_SW00", 1 + 8);

    pZettelLib = nullptr;

    // Hintergrundsounds:
    if (Sim.Options.OptionEffekte != 0) {
        SetBackgroundFx(0, "pap3.raw", 25000); // Papierrascheln
        SetBackgroundFx(2, "moeve.raw", 50000, 25000);

        WellenFx.ReInit("wellen.raw");
        WellenFx.Play(DSBPLAY_NOSTOP | DSBPLAY_LOOPING, Sim.Options.OptionEffekte * 100 / 7);
    }

    for (c = Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Planes.AnzEntries() - 1; c >= 0; c--) {
        if (Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Planes.IsInAlbum(c) != 0) {
            Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Planes[c].UpdateGlobePos(0);
        }
    }

    for (c = ReisebueroAuftraege.AnzEntries() - 1; c >= 0; c--) {
        RepaintZettel(c);
    }
}

//--------------------------------------------------------------------------------------------
// Destruktor:
//--------------------------------------------------------------------------------------------
CReisebuero::~CReisebuero() {
    SLONG c = 0;

    TipBm.Destroy();

    for (c = 0; c < 5; c++) {
        MapPlaneBms[c].Destroy();
    }

    if ((pMenuLib != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pMenuLib);
    }

    for (c = 0; c < ReisebueroAuftraege.AnzEntries(); c++) {
        if (ReisebueroAuftraege[c].Praemie < 0) {
            ReisebueroAuftraege[c].Praemie = 0;
        }
    }

    Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_AUFTRAG, "", MESSAGE_COMMENT);

    Sim.NetRefill(2);
    ReisebueroAuftraege.RefillForReisebuero();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// LastMinute message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// void CReisebuero::OnPaint()
//--------------------------------------------------------------------------------------------
void CReisebuero::OnPaint() {
    SLONG c = 0;
    CPoint point = Sim.Players.Players[PlayerNum].CursorPos;
    XY RoomPos;
    BOOL RemoveTip = TRUE;
    BOOL IsOverPaper = FALSE;
    static XY LastMouse;

    static SLONG LastTime;
    SLONG DeltaTime = AtGetTime() - LastTime;

    LastTime = AtGetTime();

    // Koordinaten für kleine Fenster konvertieren:
    ConvertMousePosition(point, &RoomPos);

    if (bHandy == 0) {
        SetMouseLook(CURSOR_NORMAL, 0, ROOM_REISEBUERO, 0);
    }

    if (SLONG(Sim.Time) >= timeReisClose) {
        Sim.Players.Players[PlayerNum].LeaveRoom();
    }

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    SP_Monitor.Pump();
    SP_Monitor.BlitAtT(RoomBm);
    SP_Girl.Pump();
    SP_Girl.BlitAtT(RoomBm);

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips();

    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
        if (gMousePosition.IfIsWithin(0, 0, 80, 440)) {
            SetMouseLook(CURSOR_EXIT, 0, ROOM_REISEBUERO, 999);
        } else if (KommVar == -1 && SP_Spinne.GetClip() == 1 && (Sim.Players.Players[Sim.localPlayer].HasItem(ITEM_SPINNE) == 0)) {
            if ((gMousePosition.IfIsWithin(327, 322, 415, 389) && SP_Spinne.GetFrame() < 10) ||
                (gMousePosition.IfIsWithin(407, 365, 455, 410) && SP_Spinne.GetFrame() >= 10 && SP_Spinne.GetFrame() < 15)) {
                SetMouseLook(CURSOR_HOT, 0, ROOM_REISEBUERO, 20);
            }
        }

        if (RoomPos.IfIsWithin(403, 73, 623, 343)) {
            SetTip(&TipBm, MapPlaneBms, FALSE, XY(160, 125), TIP_BUYAUFTRAGR, -1, 0, 0);
        }
    }

    RoomBm.pBitmap->SetClipRect(CRect(0, 0, 640, 440));
    for (c = 0; c < ReisebueroAuftraege.AnzEntries(); c++) {
        if (ReisebueroAuftraege[c].Praemie > 0) {
            RoomBm.BlitFromT(ZettelBms[c], ZettelPos[c * 2], ZettelPos[c * 2 + 1]);

            if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
                if (RoomPos.IfIsWithin(ZettelPos[c * 2], ZettelPos[c * 2 + 1], ZettelPos[c * 2] + gZettelBms[c % 3].Size.x,
                                       ZettelPos[c * 2 + 1] + gZettelBms[c % 3].Size.y)) {
                    IsOverPaper = TRUE;
                    RemoveTip = FALSE;

                    SetTip(&TipBm, MapPlaneBms, FALSE, XY(160, 125), TIP_BUYAUFTRAGR, c, 0, 0);

                    SetMouseLook(CURSOR_HOT, 0, ROOM_TAFEL, 0);

                    if (c != LastTip) {
                        LastTip = c;

                        Sim.Players.Players[PlayerNum].CheckAuftragsBerater(ReisebueroAuftraege[c]);
                    }
                }
            }
        }
    }
    if (IsOverPaper == 0) {
        LastTip = -1;
    }

    for (c = 0; c < ReisebueroAuftraege.AnzEntries(); c++) {
        if (ReisebueroAuftraege[c].Praemie < 0) {
            ReisebueroAuftraege[c].Praemie += DeltaTime * 3;
            if (ReisebueroAuftraege[c].Praemie > 0) {
                ReisebueroAuftraege[c].Praemie = 0;
            }

            XY Pos;
            SLONG p = -ReisebueroAuftraege[c].Praemie;

            Pos.x = (p * ZettelPos[c * 2] + 590 * (1000 - p)) / 1000;
            Pos.y = (p * ZettelPos[c * 2 + 1] + 440 * (1000 - p)) / 1000;

            // RoomBm.BlitFromT (ZettelBms[c], Pos);

            {
                SDL_Rect SrcRect = {0, 0, ZettelBms[c].Size.x, ZettelBms[c].Size.y};
                SDL_Rect DestRect;

                DestRect.x = Pos.x;
                DestRect.y = Pos.y;
                DestRect.w = SLONG(ZettelBms[c].Size.x * (p + 400) / 1400);
                DestRect.h = SLONG(ZettelBms[c].Size.y * (p + 400) / 1400);

                SDL_BlitScaled(ZettelBms[c].pBitmap->GetSurface(), &SrcRect, RoomBm.pBitmap->GetSurface(), &DestRect);
            }
        }
    }
    RoomBm.pBitmap->SetClipRect(CRect(0, 0, 640, 480));

    // if (Sim.Players.Players[(SLONG)PlayerNum].Messages.IsSilent()) LastTip=-1;

    if ((RemoveTip != 0) && !gMousePosition.IfIsWithin(401, 69, 630, 354) && LastMouse.IfIsWithin(401, 69, 630, 354)) {
        Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_AUFTRAG, "", MESSAGE_COMMENT);
    }

    LastMouse = gMousePosition;

    // Change: Jetzt auch im freien Spiel:
    if (Sim.Difficulty > 0 || Sim.Difficulty == DIFF_FREEGAME) {
        SP_Schlange.Pump();
        SP_Spinne.Pump();
        SP_Spinne.BlitAtT(RoomBm);
        SP_Schlange.BlitAtT(RoomBm);
    }

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::PostPaint();
    CStdRaum::PumpToolTips();
}

//--------------------------------------------------------------------------------------------
// void CReisebuero::OnPaint()
//--------------------------------------------------------------------------------------------
void CReisebuero::RepaintZettel(SLONG n) {
    ZettelBms[n].ReSize(gZettelBms[n % 3].Size);
    ZettelBms[n].BlitFrom(gZettelBms[n % 3]);

    ZettelBms[n].PrintAt(bprintf("%s-%s", (LPCTSTR)Cities[ReisebueroAuftraege[n].VonCity].Kuerzel, (LPCTSTR)Cities[ReisebueroAuftraege[n].NachCity].Kuerzel),
                         FontSmallBlack, TEC_FONT_CENTERED, XY(3, 10), XY(ZettelBms[n].Size.x - 3, 29));

    ZettelBms[n].PrintAt(ShortenLongCities(Cities[ReisebueroAuftraege[n].VonCity].Name), FontSmallBlack, TEC_FONT_CENTERED, XY(3, 31),
                         XY(ZettelBms[n].Size.x - 3, 102));
    ZettelBms[n].PrintAt("-", FontSmallBlack, TEC_FONT_CENTERED, XY(3, 42), XY(ZettelBms[n].Size.x - 3, 102));
    ZettelBms[n].PrintAt(ShortenLongCities(Cities[ReisebueroAuftraege[n].NachCity].Name), FontSmallBlack, TEC_FONT_CENTERED, XY(3, 54),
                         XY(ZettelBms[n].Size.x - 3, 102));
}

//--------------------------------------------------------------------------------------------
// void CReisebuero::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CReisebuero::OnLButtonDown(UINT nFlags, CPoint point) {
    SLONG c = 0;
    XY RoomPos;

    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    DefaultOnLButtonDown();

    if (ConvertMousePosition(point, &RoomPos) == 0) {
        CStdRaum::OnLButtonDown(nFlags, point);
        return;
    }

    if (PreLButtonDown(point) == 0) {
        if (MouseClickArea == ROOM_REISEBUERO && MouseClickId == 999) {
            qPlayer.LeaveRoom();
        } else if (MouseClickArea == ROOM_REISEBUERO && MouseClickId == 20) {
            if ((qPlayer.HasItem(ITEM_SPINNE) == 0) && (qPlayer.HasSpaceForItem() != 0)) {
                qPlayer.BuyItem(ITEM_SPINNE);
                KommVar2 = 2;
                KommVar = 0;
            }
        }

        for (c = ReisebueroAuftraege.AnzEntries() - 1; c >= 0; c--) {
            if (ReisebueroAuftraege[c].Praemie > 0) {
                if (RoomPos.IfIsWithin(ZettelPos[c * 2], ZettelPos[c * 2 + 1], ZettelPos[c * 2] + gZettelBms[c % 3].Size.x,
                                       ZettelPos[c * 2 + 1] + gZettelBms[c % 3].Size.y)) {
                    if (qPlayer.Auftraege.GetNumFree() < 3) {
                        qPlayer.Auftraege.ReSize(qPlayer.Auftraege.AnzEntries() + 10);
                    }

                    PlayUniversalFx("paptake.raw", Sim.Options.OptionEffekte);

                    qPlayer.Auftraege += ReisebueroAuftraege[c];
                    qPlayer.NetUpdateOrder(ReisebueroAuftraege[c]);

                    // Für den Statistikscreen:
                    qPlayer.Statistiken[STAT_AUFTRAEGE].AddAtPastDay(1);

                    SIM::SendSimpleMessage(ATNET_SYNCNUMFLUEGE, 0, Sim.localPlayer, static_cast<SLONG>(qPlayer.Statistiken[STAT_AUFTRAEGE].GetAtPastDay(0)),
                                           static_cast<SLONG>(qPlayer.Statistiken[STAT_LMAUFTRAEGE].GetAtPastDay(0)));

                    ReisebueroAuftraege[c].Praemie = -1000;
                    qPlayer.NetUpdateTook(2, c);
                    break;
                }
            }
        }
    }
    CStdRaum::OnLButtonDown(nFlags, point);
}

//--------------------------------------------------------------------------------------------
// void CReisebuero::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CReisebuero::OnRButtonDown(UINT nFlags, CPoint point) {
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
