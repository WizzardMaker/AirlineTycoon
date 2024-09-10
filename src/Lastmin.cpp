//============================================================================================
// LastMin.cpp : Der Last-Minute Schalter
//============================================================================================
// Link: "LastMin.h"
//============================================================================================
#include "StdAfx.h"
#include "AtNet.h"
#include "gllast.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const SLONG ZettelPos[10 * 2] = {66, 79, 126, 212, 145, 116, 33, 250, 128, 289, 15, 150, 346, 331, 40, 113, 449, 126, 245, 307};

extern SLONG timeLastClose;

//////////////////////////////////////////////////////////////////////////////////////////////
// Museum Konstruktion, Initialisation, Destruction
//////////////////////////////////////////////////////////////////////////////////////////////

CString ShortenLongCities(CString City);

//--------------------------------------------------------------------------------------------
// Konstruktor:
//--------------------------------------------------------------------------------------------
CLastMinute::CLastMinute(BOOL bHandy, ULONG PlayerNum) : CStdRaum(bHandy, PlayerNum, "lastmin.gli", GFX_LASTMIN) {
    SLONG c = 0;

    SetRoomVisited(PlayerNum, ROOM_LAST_MINUTE);

    Sim.FocusPerson = -1;

    if (bHandy == 0) {
        AmbientManager.SetGlobalVolume(60);
    }
    Sim.NetRefill(1);
    LastMinuteAuftraege.RefillForLastMinute();

    MoveKran = 0;
    NewKranDir = -1;
    KranArrived = -1;

    SP_Girl.ReSize(7);
    SP_Girl.Clips[0].ReSize(0, "lmwait.smk", "", XY(333, 225), SPM_IDLE, CRepeat(1, 1), CPostWait(25, 60), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, "A9A2A4A4A8", 0, 6, 1, 2, 5);
    SP_Girl.Clips[1].ReSize(1, "lmgrins.smk", "", XY(333, 225), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, "A1", 0);
    SP_Girl.Clips[2].ReSize(2, "lmknopf.smk", "kran1.raw", XY(333, 225), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr,
                            SMACKER_CLIP_SET, 0, nullptr, "A1", 3);
    SP_Girl.Clips[3].ReSize(3, "lmknopfw.smk", "", XY(333, 225), SPM_IDLE, CRepeat(1, 1), CPostWait(9999, 9999), SMACKER_CLIP_CANCANCEL, &MoveKran,
                            SMACKER_CLIP_SET | SMACKER_CLIP_PRE, 1, &KranArrived, "A1", 3);
    SP_Girl.Clips[4].ReSize(4, "lmknopfz.smk", "", XY(333, 225), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &KranArrived,
                            SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, nullptr, "A1", 0);
    SP_Girl.Clips[5].ReSize(5, "lmtaste.smk", "", XY(333, 225), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, &KommMonitor,
                            SMACKER_CLIP_SET | (SMACKER_CLIP_FRAME + 4 * SMACKER_CLIP_MULT), 0, nullptr, "A1", 0);
    SP_Girl.Clips[6].ReSize(6, "lmwaitk.smk", "", XY(333, 225), SPM_IDLE, CRepeat(1, 1), CPostWait(1, 1), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, "A9", 0);

    SP_Monitor.ReSize(3);
    //--------------------------------------------------------------------------------------------
    SP_Monitor.Clips[0].ReSize(0, "", "", XY(448, 318), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0, nullptr,
                               "A5A5", 1, 2);
    SP_Monitor.Clips[1].ReSize(1, "moni.smk", "", XY(448, 318), SPM_IDLE, CRepeat(1, 1), CPostWait(999, 999), SMACKER_CLIP_CANCANCEL, &KommMonitor,
                               SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &KommMonitor, "A1", 0);
    SP_Monitor.Clips[2].ReSize(2, "monia.smk", "", XY(448, 318), SPM_IDLE, CRepeat(1, 1), CPostWait(999, 999), SMACKER_CLIP_CANCANCEL, &KommMonitor,
                               SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &KommMonitor, "A1", 0);

    SP_Kran.ReSize(4);
    SP_Kran.Clips[0].ReSize(0, "kranrs.smk", "", XY(300, 129), SPM_IDLE, CRepeat(1, 1), CPostWait(9990, 9990), SMACKER_CLIP_CANCANCEL, &NewKranDir,
                            SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &NewKranDir, "A9", 0);
    SP_Kran.Clips[1].ReSize(1, "kranl.smk", "kran2.raw", XY(300, 129), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &KranArrived,
                            SMACKER_CLIP_SET | (SMACKER_CLIP_FRAME + 21 * SMACKER_CLIP_MULT), 4, nullptr, "A1", 2);
    SP_Kran.Clips[2].ReSize(2, "kranls.smk", "", XY(300, 129), SPM_IDLE, CRepeat(1, 1), CPostWait(9990, 9990), SMACKER_CLIP_CANCANCEL, &NewKranDir,
                            SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &NewKranDir, "A1", 2);
    SP_Kran.Clips[3].ReSize(3, "kranr.smk", "kran2.raw", XY(300, 129), SPM_IDLE, CRepeat(1, 1), CPostWait(1, 0), SMACKER_CLIP_DONTCANCEL, &KranArrived,
                            SMACKER_CLIP_SET | (SMACKER_CLIP_FRAME + 21 * SMACKER_CLIP_MULT), 4, nullptr, "A1", 0);

    pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("tipau.gli", GliPath)), &pMenuLib, L_LOCMEM);
    TipBm.ReSize(pMenuLib, "BLOC1");
    MapPlaneBms[0].ReSize(pMenuLib, "PL_B00", 1 + 8);
    MapPlaneBms[1].ReSize(pMenuLib, "PL_V00", 1 + 8);
    MapPlaneBms[2].ReSize(pMenuLib, "PL_R00", 1 + 8);
    MapPlaneBms[3].ReSize(pMenuLib, "PL_J00", 1 + 8);
    MapPlaneBms[4].ReSize(pMenuLib, "PL_SW00", 1 + 8);

    SetBackgroundFx(0, "pap3.raw", 25000); // Papierrascheln

    for (c = Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Planes.AnzEntries() - 1; c >= 0; c--) {
        if (Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Planes.IsInAlbum(c) != 0) {
            Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Planes[c].UpdateGlobePos(0);
        }
    }

    for (c = 0; c < LastMinuteAuftraege.AnzEntries(); c++) {
        RepaintZettel(c);
    }

#ifdef DEMO
    MenuStart(MENU_REQUEST, MENU_REQUEST_NO_LM);
    MenuSetZoomStuff(XY(320, 220), 0.17, FALSE);
#endif
}

//--------------------------------------------------------------------------------------------
// Destruktor:
//--------------------------------------------------------------------------------------------
CLastMinute::~CLastMinute() {
    SLONG c = 0;

    TipBm.Destroy();

    for (c = 0; c < 5; c++) {
        MapPlaneBms[c].Destroy();
    }

    if ((pMenuLib != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pMenuLib);
    }

    for (c = 0; c < LastMinuteAuftraege.AnzEntries(); c++) {
        if (LastMinuteAuftraege[c].Praemie < 0) {
            LastMinuteAuftraege[c].Praemie = 0;
        }
    }

    Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_AUFTRAG, "", MESSAGE_COMMENT);

    Sim.NetRefill(1);
    LastMinuteAuftraege.RefillForLastMinute();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// LastMinute message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// void CLastMinute::OnPaint()
//--------------------------------------------------------------------------------------------
void CLastMinute::OnPaint() {
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

    if (SLONG(Sim.Time) >= timeLastClose) {
        Sim.Players.Players[PlayerNum].LeaveRoom();
    }

    if (bHandy == 0) {
        SetMouseLook(CURSOR_NORMAL, 0, ROOM_LAST_MINUTE, 0);
    }

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    SP_Monitor.Pump();
    SP_Monitor.BlitAtT(RoomBm);
    SP_Girl.Pump();
    SP_Girl.BlitAtT(RoomBm);
    SP_Kran.Pump();
    SP_Kran.BlitAtT(RoomBm);

    if (MoveKran != 0) {
        if (SP_Kran.GetClip() == 0) {
            NewKranDir = 1;
        }
        if (SP_Kran.GetClip() == 2) {
            NewKranDir = 3;
        }
        MoveKran = FALSE;
    }

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips();

    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
        if (gMousePosition.IfIsWithin(560, 0, 640, 440)) {
            SetMouseLook(CURSOR_EXIT, 0, ROOM_LAST_MINUTE, 999);
        }

        if (RoomPos.IfIsWithin(17, 73, 237, 343)) {
            SetTip(&TipBm, MapPlaneBms, FALSE, XY(300, 125), TIP_BUYAUFTRAG, -1, 0, 0);
        }
    }

    RoomBm.pBitmap->SetClipRect(CRect(0, 0, 640, 440));
    for (c = 0; c < LastMinuteAuftraege.AnzEntries(); c++) {
        if (LastMinuteAuftraege[c].Praemie > 0) {
            if (RoomBm.BlitFromT(ZettelBms[c], ZettelPos[c * 2], ZettelPos[c * 2 + 1]) == 0) {
                RepaintZettel(c);
                RoomBm.BlitFromT(ZettelBms[c], ZettelPos[c * 2], ZettelPos[c * 2 + 1]);
            }

            if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
                if (RoomPos.IfIsWithin(ZettelPos[c * 2], ZettelPos[c * 2 + 1], ZettelPos[c * 2] + gZettelBms[c % 3].Size.x,
                                       ZettelPos[c * 2 + 1] + gZettelBms[c % 3].Size.y)) {
                    RemoveTip = FALSE;
                    IsOverPaper = TRUE;

                    SetTip(&TipBm, MapPlaneBms, FALSE, XY(300, 125), TIP_BUYAUFTRAG, c, 0, 0);

                    SetMouseLook(CURSOR_HOT, 0, ROOM_TAFEL, 0);

                    if (c != LastTip) {
                        LastTip = c;

                        Sim.Players.Players[PlayerNum].CheckAuftragsBerater(LastMinuteAuftraege[c]);
                    }
                }
            }
        }
    }
    if (IsOverPaper == 0) {
        LastTip = -1;
    }

    for (c = 0; c < LastMinuteAuftraege.AnzEntries(); c++) {
        if (LastMinuteAuftraege[c].Praemie < 0) {
            LastMinuteAuftraege[c].Praemie += DeltaTime * 4;
            if (LastMinuteAuftraege[c].Praemie > 0) {
                LastMinuteAuftraege[c].Praemie = 0;
            }

            XY Pos;
            SLONG p = -LastMinuteAuftraege[c].Praemie;

            Pos.x = (p * ZettelPos[c * 2] + 16 * (1000 - p)) / 1000;
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

    if ((RemoveTip != 0) && !gMousePosition.IfIsWithin(8, 69, 242, 347) && LastMouse.IfIsWithin(8, 69, 242, 347)) {
        Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_AUFTRAG, "", MESSAGE_COMMENT);
    }

    LastMouse = gMousePosition;

#ifdef DEMO
    if (!IsDialogOpen() && !MenuIsOpen())
        SetMouseLook(CURSOR_EXIT, 0, ROOM_LAST_MINUTE, 999);
#endif

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::PostPaint();
    CStdRaum::PumpToolTips();
}

//--------------------------------------------------------------------------------------------
// void CLastMinute::OnPaint()
//--------------------------------------------------------------------------------------------
void CLastMinute::RepaintZettel(SLONG n) {
    ZettelBms[n].ReSize(gZettelBms[n % 3].Size);
    ZettelBms[n].BlitFrom(gZettelBms[n % 3]);

    ZettelBms[n].PrintAt(bprintf("%s-%s", (LPCTSTR)Cities[LastMinuteAuftraege[n].VonCity].Kuerzel, (LPCTSTR)Cities[LastMinuteAuftraege[n].NachCity].Kuerzel),
                         FontSmallBlack, TEC_FONT_CENTERED, XY(3, 10), XY(ZettelBms[n].Size.x - 3, 29));

    ZettelBms[n].PrintAt(ShortenLongCities(Cities[LastMinuteAuftraege[n].VonCity].Name), FontSmallBlack, TEC_FONT_CENTERED, XY(3, 31),
                         XY(ZettelBms[n].Size.x - 3, 102));
    ZettelBms[n].PrintAt("-", FontSmallBlack, TEC_FONT_CENTERED, XY(3, 41), XY(ZettelBms[n].Size.x - 3, 102));
    ZettelBms[n].PrintAt(ShortenLongCities(Cities[LastMinuteAuftraege[n].NachCity].Name), FontSmallBlack, TEC_FONT_CENTERED, XY(3, 52),
                         XY(ZettelBms[n].Size.x - 3, 102));
}

//--------------------------------------------------------------------------------------------
// void CLastMinute::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CLastMinute::OnLButtonDown(UINT nFlags, CPoint point) {
    SLONG c = 0;
    XY RoomPos;

    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    DefaultOnLButtonDown();

    if (ConvertMousePosition(point, &RoomPos) == 0) {
        CStdRaum::OnLButtonDown(nFlags, point);
        return;
    }

#ifndef DEMO
    if (PreLButtonDown(point) == 0) {
        if (MouseClickArea == ROOM_LAST_MINUTE && MouseClickId == 999) {
            qPlayer.LeaveRoom();
        }

        for (c = LastMinuteAuftraege.AnzEntries() - 1; c >= 0; c--) {
            if (LastMinuteAuftraege[c].Praemie > 0) {
                if (RoomPos.IfIsWithin(ZettelPos[c * 2], ZettelPos[c * 2 + 1], ZettelPos[c * 2] + gZettelBms[c % 3].Size.x,
                                       ZettelPos[c * 2 + 1] + gZettelBms[c % 3].Size.y)) {
                    if (qPlayer.Auftraege.GetNumFree() < 3) {
                        qPlayer.Auftraege.ReSize(qPlayer.Auftraege.AnzEntries() + 10);
                    }

                    gUniversalFx.Stop();
                    gUniversalFx.ReInit("paptake.raw");
                    gUniversalFx.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);

                    qPlayer.Auftraege += LastMinuteAuftraege[c];
                    qPlayer.NetUpdateOrder(LastMinuteAuftraege[c]);

                    // Für den Statistikscreen:
                    qPlayer.Statistiken[STAT_AUFTRAEGE].AddAtPastDay(1);
                    qPlayer.Statistiken[STAT_LMAUFTRAEGE].AddAtPastDay(1);

                    SIM::SendSimpleMessage(ATNET_SYNCNUMFLUEGE, 0, Sim.localPlayer, static_cast<SLONG>(qPlayer.Statistiken[STAT_AUFTRAEGE].GetAtPastDay(0)),
                                           static_cast<SLONG>(qPlayer.Statistiken[STAT_LMAUFTRAEGE].GetAtPastDay(0)));

                    LastMinuteAuftraege[c].Praemie = -1000;
                    qPlayer.NetUpdateTook(1, c);
                    break;
                }
            }
        }
    }
#else
    if (!PreLButtonDown(point))
        if (MouseClickArea == ROOM_LAST_MINUTE && MouseClickId == 999)
            qPlayer.LeaveRoom();
#endif

    CStdRaum::OnLButtonDown(nFlags, point);
}

//--------------------------------------------------------------------------------------------
// void CLastMinute::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CLastMinute::OnRButtonDown(UINT nFlags, CPoint point) {
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
