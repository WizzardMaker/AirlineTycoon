//============================================================================================
// CWerbung.cpp : Hier werden die Flugzeuge repariert
//============================================================================================
// Link: "Werbung.h"
//============================================================================================
#include "StdAfx.h"
#include "Werbung.h"
#include "glnowerb.h"
#include "glwerbe.h"

#define ROOM_LIMIT DIFF_NORMAL // Siehe auch StdRaum.cpp wegen dem Intro-Text beim ersten betreten

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//--------------------------------------------------------------------------------------------
// Die Schalter wird eröffnet:
//--------------------------------------------------------------------------------------------
CWerbung::CWerbung(BOOL qHandy, ULONG PlayerNum) : CStdRaum(qHandy, PlayerNum, "", 0) {
    if (bHandy == 0) {
        AmbientManager.SetGlobalVolume(40);
    }

    Sim.FocusPerson = -1;

    if (Sim.Difficulty < ROOM_LIMIT && Sim.Difficulty != DIFF_FREEGAME) {
        ReSize("nowerb.gli", GFX_WERBUNG);
    } else {
        SetRoomVisited(PlayerNum, ROOM_WERBUNG);
        ReSize("werbung.gli", GFX_WERBUNG);
        DiskBm.ReSize(pRoomLib, "OHNEDISK");
        NoSaboBm.ReSize(pRoomLib, "NOSABO");

        Talkers.Talkers[TALKER_WERBUNG].IncreaseReference();
        DefaultDialogPartner = TALKER_WERBUNG;

        SP_Mann.ReSize(8);
        SP_Mann.Clips[0].ReSize(0, "wa_copyc.smk", "wa_copyc.raw", XY(290, 240), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                SMACKER_CLIP_SET, 0, nullptr, // Kopieren
                                "A9A3A2E1", 0, 1, 2, 3);
        SP_Mann.Clips[1].ReSize(1, "wa_copy.smk", "wa_copy.raw", XY(290, 240), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                SMACKER_CLIP_SET, 0, nullptr, // Kopieren & Schauen
                                "A9E1", 0, 3);
        SP_Mann.Clips[2].ReSize(2, "wa_wait.smk", "", XY(290, 240), SPM_IDLE, CRepeat(1, 1), CPostWait(30, 60), SMACKER_CLIP_DONTCANCEL, nullptr,
                                SMACKER_CLIP_SET, 0, nullptr, // Warten
                                "A9E1", 0, 3);
        SP_Mann.Clips[3].ReSize(3, "wa_turn.smk", "", XY(290, 240), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                SMACKER_CLIP_SET, 0, nullptr, // Drehen
                                "A9E1E1", 4, 4, 5);
        SP_Mann.Clips[4].ReSize(4, "wa_redea.smk", "", XY(290, 240), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr,
                                SMACKER_CLIP_SET, 0, nullptr, // Reden
                                "A9E1E1", 4, 5, 7);
        SP_Mann.Clips[5].ReSize(5, "wa_redew.smk", "", XY(290, 240), SPM_LISTENING, CRepeat(1, 1), CPostWait(2, 2), SMACKER_CLIP_CANCANCEL, nullptr,
                                SMACKER_CLIP_SET, 0, nullptr, // Zuhören
                                "A9A1E1E1", 5, 6, 4, 7);
        SP_Mann.Clips[6].ReSize(6, "wa_redek.smk", "", XY(290, 240), SPM_LISTENING, CRepeat(1, 1), CPostWait(2, 2), SMACKER_CLIP_CANCANCEL, nullptr,
                                SMACKER_CLIP_SET, 0, nullptr, // Blinzeln
                                "A9E1E1", 5, 4, 7);
        SP_Mann.Clips[7].ReSize(7, "wa_turnz.smk", "", XY(290, 240), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                SMACKER_CLIP_SET, 0, nullptr, // Zurückdrehen
                                "A9", 0);

        // Raumanimationen
        CoffeeFx.ReInit("kaffee.raw");
        SpratzelFx.ReInit("spratzel.raw");

        BoyAnim.ReSize(pRoomLib, "BOY_01", 3, nullptr, TRUE, ANIMATION_MODE_RANDOM, 0, 25);
        KaffeeAnim.ReSize(pRoomLib, "CAFE01", 15, &CoffeeFx, FALSE, ANIMATION_MODE_REPEAT, 20 * 14, 3, 20 * 19);

        ScreenAnim.ReSize(pRoomLib, "SCREEN01", 6, nullptr, FALSE, ANIMATION_MODE_REPEAT, 250, 4);
        SpratzelAnim.ReSize(pRoomLib, "SPRATZL0", 5, &SpratzelFx, FALSE, ANIMATION_MODE_REPEAT, 20 * 14, 1, 10 * 19);
    }

    JahreszeitBms.ReSize(pRoomLib, "WERBUNG0", 4);

#ifdef DEMO
    MenuStart(MENU_REQUEST, MENU_REQUEST_NO_WERBUNG);
    MenuSetZoomStuff(XY(320, 220), 0.17, FALSE);
#endif

    SDL_ShowWindow(FrameWnd->m_hWnd);
    SDL_UpdateWindowSurface(FrameWnd->m_hWnd);
}

//--------------------------------------------------------------------------------------------
// Sehr destruktiv!
//--------------------------------------------------------------------------------------------
CWerbung::~CWerbung() {
    DiskBm.Destroy();
    NoSaboBm.Destroy();
    JahreszeitBms.Destroy();
    Talkers.Talkers[TALKER_WERBUNG].DecreaseReference();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CWerbung message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// void CWerbung::OnPaint()
//--------------------------------------------------------------------------------------------
void CWerbung::OnPaint() {
    SLONG NewTip = 0;
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    if (Sim.Date > 4) {
        Sim.GiveHint(HINT_WERBUNG);
    }

    if (bHandy == 0) {
        SetMouseLook(CURSOR_NORMAL, 0, ROOM_WERBUNG, 0);
    }

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    // Die Jahreszeit:
    if (Sim.Difficulty < ROOM_LIMIT && Sim.Difficulty != DIFF_FREEGAME) {
        RoomBm.BlitFrom(JahreszeitBms[SIM::GetSeason()], 491, 0); // nowerbung
    } else {
        RoomBm.BlitFrom(JahreszeitBms[SIM::GetSeason()], 495, 0); // werbung
    }

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips();

    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
        if (gMousePosition.IfIsWithin(0, 145, 70, 381)) {
            SetMouseLook(CURSOR_EXIT, 0, ROOM_WERBUNG, 999);
        } else if ((Sim.Difficulty >= ROOM_LIMIT || Sim.Difficulty == DIFF_FREEGAME) && gMousePosition.IfIsWithin(302, 257, 454, 419)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_WERBUNG, 10);
        } else if (gMousePosition.IfIsWithin(74, 274, 111, 310) && (qPlayer.HasItem(ITEM_DISKETTE) == 0) &&
                   !(Sim.Difficulty < ROOM_LIMIT && Sim.Difficulty != DIFF_FREEGAME)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_WERBUNG, 800);
        }
    }

#ifdef DEMO
    if (!IsDialogOpen() && !MenuIsOpen())
        SetMouseLook(CURSOR_EXIT, 0, ROOM_WERBUNG, 999);
#endif

    if (qPlayer.HasItem(ITEM_DISKETTE) != 0) {
        RoomBm.BlitFrom(DiskBm, 82, 281);
    }
    if ((qPlayer.SecurityFlags & 32) != 0U) {
        RoomBm.BlitFromT(NoSaboBm, 139, 241);
    }

    if (Sim.Difficulty >= ROOM_LIMIT || Sim.Difficulty == DIFF_FREEGAME) {
        // Die Raum-Animationen:
        BoyAnim.BlitAtT(RoomBm, 486, 139);
        ScreenAnim.BlitAtT(RoomBm, 287, 77);
        KaffeeAnim.BlitAt(RoomBm, 161, 30);
        SpratzelAnim.BlitAt(RoomBm, 388, 79);

        SP_Mann.Pump();
        SP_Mann.BlitAtT(RoomBm);
    }

    CStdRaum::PostPaint();
    CStdRaum::PumpToolTips();

    if (Sim.Difficulty >= ROOM_LIMIT || Sim.Difficulty == DIFF_FREEGAME) {
        if (CurrentMenu == MENU_ADROUTE && (gMousePosition - MenuPos).IfIsWithin(216, 6, 387, 212)) {
            NewTip = (gMousePosition.y - (MenuPos.y + 25)) / 13 + MenuPage;

            if (NewTip >= 0 && NewTip < MenuDataTable.LineIndex.AnzEntries() &&
                qPlayer.RentRouten.RentRouten.AnzEntries() > (MenuDataTable.LineIndex[NewTip]) &&
                (qPlayer.RentRouten.RentRouten[MenuDataTable.LineIndex[NewTip]].Rang != 0U)) {
                if (NewTip != CurrentTip) {
                    MenuRepaint();

                    DrawRouteTipContents(OnscreenBitmap, PlayerNum, MenuDataTable.LineIndex[NewTip], -2, 0, 0, 0, 0, 0, XY(6, 6), XY(6, 28), &FontSmallBlack,
                                         &FontSmallBlack);
                    CurrentTip = NewTip;
                }

                PrimaryBm.BlitFromT(OnscreenBitmap, WinP2.x - 10 - TipBm.Size.x, (gMousePosition.y - (MenuPos.y + 44)) / 13 * 13 + (MenuPos.y + 44) + 16);
            } else {
                NewTip = -1;
            }
        } else {
            NewTip = -1;
        }

        if ((MenuIsOpen() != 0) && NewTip != CurrentTip) {
            MenuRepaint();
            CurrentTip = NewTip;
        }
    }
}

//--------------------------------------------------------------------------------------------
// void CWerbung::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CWerbung::OnLButtonDown(UINT nFlags, CPoint point) {
    XY RoomPos;

    DefaultOnLButtonDown();

    if (ConvertMousePosition(point, &RoomPos) == 0) {
        CStdRaum::OnLButtonDown(nFlags, point);
        return;
    }

#ifndef DEMO
    if (PreLButtonDown(point) == 0) {
        if (Sim.Difficulty >= ROOM_LIMIT || Sim.Difficulty == DIFF_FREEGAME) {
            if (gMousePosition.IfIsWithin(287, 136, 322, 164) || gMousePosition.IfIsWithin(322, 131, 351, 156) ||
                gMousePosition.IfIsWithin(354, 126, 393, 150)) {
                ScreenAnim.Reset();
            }
        } else {
            if (gMousePosition.IfIsWithin(178, 72, 183, 88) || gMousePosition.IfIsWithin(447, 85, 470, 100)) {
                if (Sim.Options.OptionEffekte != 0) {
                    gUniversalFx.ReInit("fused.raw");
                    gUniversalFx.Play(0, Sim.Options.OptionPlaneVolume * 100 / 7 * AmbientManager.GlobalVolume / 100);
                }
            }
        }

        if (MouseClickArea == ROOM_WERBUNG && MouseClickId == 999) {
            Sim.Players.Players[PlayerNum].LeaveRoom();
        } else if (MouseClickArea == ROOM_WERBUNG && MouseClickId == 800) {
            StartDialog(TALKER_WERBUNG, MEDIUM_AIR, 800);
        } else if (MouseClickArea == ROOM_WERBUNG && MouseClickId == 10) {
            StartDialog(TALKER_WERBUNG, MEDIUM_AIR, 1);
        } else {
            CStdRaum::OnLButtonDown(nFlags, point);
        }
    }
#else
    if (!PreLButtonDown(point))
        if (MouseClickArea == ROOM_WERBUNG && MouseClickId == 999)
            Sim.Players.Players[(SLONG)PlayerNum].LeaveRoom();
#endif
}

//--------------------------------------------------------------------------------------------
// void CWerbung::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CWerbung::OnRButtonDown(UINT nFlags, CPoint point) {
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
