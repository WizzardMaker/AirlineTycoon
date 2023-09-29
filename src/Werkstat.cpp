//============================================================================================
// CWerkstat.cpp : Hier werden die Flugzeuge repariert
//============================================================================================
// Link: "Werkstat.h"
//============================================================================================
#include "StdAfx.h"
#include "glwerk.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static XY SlimePositions[] = {XY(93, 441), XY(111, 418), XY(137, 430), XY(54, 427), XY(14, 427)};

//--------------------------------------------------------------------------------------------
// Eine Animation initialisieren:
// * GfxLib  *gfxLib       Grafikbibliothek für die Animation
// * CString &graphicIDs   Name der Startphase
// * SLONG    Anzahl       Anzahl der Bilder in der Animation
// * SBFX    *Fx           Effekt, der beim Abspielen gespielt wird
// * BOOL     StartPic     Soll beim warten auf den Animationsbeginn ein Startbild gezeigt werden?
// * SLONG    Mode         ONCE/REPEAT/RANDOM PHASE
// * SLONG    Prelude      Soviel x/20 Sekunden bis zum Animationsbeginn warten
// * SLONG    Speed        Geschwindigkeit der Animation in x/20 Sekunden pro Frame
// * SLONG    CyclycWait   (optional) Wartepausen zwischen Folgedurchgängen der Animation
// * SLONG    RepeatCore   (optional) Sooft wird die Animation am Stück sofort wiederholt
//--------------------------------------------------------------------------------------------
void CAnimation::ReSize(GfxLib *gfxLib, const CString &graphicIDs, SLONG Anzahl, SBFX *Fx, BOOL StartPic, SLONG Mode, SLONG Prelude, SLONG Speed,
                        SLONG CyclycWait, SLONG RepeatCore) {
    CAnimation::Frames.ReSize(gfxLib, graphicIDs, Anzahl);
    CAnimation::pSoundFx = Fx;
    CAnimation::Mode = Mode;         // ANIMATION_MODE_ONCE / _REPEAT / _RANDOM
    CAnimation::StartPic = StartPic; // Bild zeigen, wenn Animation noch nicht begonnen?
    CAnimation::Prelude = Prelude;   // Wartezeit bis zur ersten Animation
    CAnimation::CounterStart = Sim.TickerTime + Prelude;
    CAnimation::Speed = Speed;
    CAnimation::CyclycWait = CyclycWait; // Wartezeit nach jedem kompletten Animationsdurchlauf
    CAnimation::RepeatCore = RepeatCore;

    CAnimation::JustPlayed = FALSE;
}

//--------------------------------------------------------------------------------------------
// Setzt die Animation zurück (z.B. für den Bildschirmschoner)
//--------------------------------------------------------------------------------------------
void CAnimation::Reset() { CAnimation::CounterStart = Sim.TickerTime + Prelude; }

//--------------------------------------------------------------------------------------------
// Gibt den aktuell gezeigten Frame zurück:
//--------------------------------------------------------------------------------------------
SLONG CAnimation::GetFrame() const {
    if (Frames.AnzEntries() != 0) {
        if (CAnimation::Mode == ANIMATION_MODE_NEVER) {
            if (StartPic != 0) {
                return (0);
            }
        } else {
            SLONG Phase = (SLONG(Sim.TickerTime) - CounterStart) / Speed;

            if (Phase < 0) {
                return (0);
            }

            if (Mode == ANIMATION_MODE_RANDOM) {
                Phase = SLONG(((sin(Phase) + 1) / 2.001) * Frames.AnzEntries());
            }

            if (Phase >= Frames.AnzEntries() * RepeatCore) {
                if (Mode == ANIMATION_MODE_REPEAT && (CyclycWait != 0)) {
                    return (0);
                }

                if (Mode == ANIMATION_MODE_REPEAT) {
                    Phase %= Frames.AnzEntries();
                }
                if (Mode == ANIMATION_MODE_ONCE) {
                    Phase = Frames.AnzEntries() - 1;
                }
            }

            return (Phase % Frames.AnzEntries());
        }
    }
    return (0);
}

//--------------------------------------------------------------------------------------------
// Entfernt die Animation vom Bildschirm:
//--------------------------------------------------------------------------------------------
void CAnimation::Remove() {
    if (pSoundFx != nullptr) {
        pSoundFx->Stop();
    }
    pSoundFx = nullptr;
    Frames.Destroy();
}

//--------------------------------------------------------------------------------------------
// Startet einen Animation JETZT:
//--------------------------------------------------------------------------------------------
void CAnimation::StartNow() {
    Mode = ANIMATION_MODE_ONCE;
    CounterStart = Sim.TickerTime + Prelude;
}

//--------------------------------------------------------------------------------------------
// Blittet eine Phase an eine bestimmte Stelle:
//--------------------------------------------------------------------------------------------
void CAnimation::BlitAt(SBBM &RoomBm, SLONG x, SLONG y) {
    if (Frames.AnzEntries() != 0) {
        if (CAnimation::Mode == ANIMATION_MODE_NEVER) {
            if (StartPic != 0) {
                RoomBm.BlitFrom(Frames[0], x, y);
            }
        } else {
            SLONG Phase = (SLONG(Sim.TickerTime) - CounterStart) / Speed;

            if (Phase < 0) {
                if (StartPic == 0) {
                    return;
                }
                Phase = 0;
            }

            if (Mode == ANIMATION_MODE_RANDOM) {
                Phase = SLONG(((sin(Phase) + 1) / 2.001) * Frames.AnzEntries());
            }

            if (Phase >= Frames.AnzEntries() * RepeatCore) {
                if (Mode == ANIMATION_MODE_REPEAT && (CyclycWait != 0)) {
                    CounterStart = Sim.TickerTime + CyclycWait;
                    if (StartPic == 0) {
                        return;
                    }
                    Phase = 0;
                }

                if (Mode == ANIMATION_MODE_REPEAT) {
                    Phase %= Frames.AnzEntries();
                }
                if (Mode == ANIMATION_MODE_ONCE) {
                    Phase = Frames.AnzEntries() - 1;
                }
            }

            if (Mode == ANIMATION_MODE_REPEAT && Prelude == 0 && (JustPlayed == 0)) {
                if ((pSoundFx != nullptr) && (Sim.Options.OptionDigiSound != 0)) {
                    pSoundFx->Play(DSBPLAY_NOSTOP | DSBPLAY_LOOPING, Sim.Options.OptionEffekte * 100 / 7);
                    JustPlayed = TRUE;
                }
            } else {
                if ((pSoundFx != nullptr) && (Sim.Options.OptionDigiSound != 0) && (JustPlayed == 0) && Phase % Frames.AnzEntries() == 0) {
                    pSoundFx->Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                    JustPlayed = TRUE;
                } else {
                    JustPlayed = FALSE;
                }
            }

            RoomBm.BlitFrom(Frames[Phase % Frames.AnzEntries()], x, y);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Blittet eine Phase an eine bestimmte Stelle:
//--------------------------------------------------------------------------------------------
void CAnimation::BlitAtT(SBBM &RoomBm, SLONG x, SLONG y) {
    if (Frames.AnzEntries() != 0) {
        SLONG Phase = (SLONG(Sim.TickerTime) - CounterStart) / Speed;

        if (Phase < 0) {
            if (StartPic == 0) {
                return;
            }
            Phase = 0;
        }

        if (Mode == ANIMATION_MODE_RANDOM) {
            Phase = SLONG(((sin(Phase) + 1) / 2.001) * Frames.AnzEntries());
        }

        if (Phase >= Frames.AnzEntries() * RepeatCore) {
            if (Mode == ANIMATION_MODE_REPEAT && (CyclycWait != 0)) {
                CounterStart = Sim.TickerTime + CyclycWait;
                if (StartPic == 0) {
                    return;
                }
                Phase = 0;
            }

            if (Mode == ANIMATION_MODE_REPEAT) {
                Phase %= Frames.AnzEntries();
            }
            if (Mode == ANIMATION_MODE_ONCE) {
                Phase = Frames.AnzEntries() - 1;
            }
        }

        if (Mode == ANIMATION_MODE_REPEAT && Prelude == 0 && (JustPlayed == 0)) {
            if ((pSoundFx != nullptr) && (Sim.Options.OptionDigiSound != 0)) {
                pSoundFx->Play(DSBPLAY_NOSTOP | DSBPLAY_LOOPING, Sim.Options.OptionEffekte * 100 / 7);
                JustPlayed = TRUE;
            }
        } else {
            if ((pSoundFx != nullptr) && (Sim.Options.OptionDigiSound != 0) && (JustPlayed == 0) && Phase % Frames.AnzEntries() == 0) {
                pSoundFx->Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                JustPlayed = TRUE;
            } else {
                JustPlayed = FALSE;
            }
        }

        RoomBm.BlitFromT(Frames[Phase % Frames.AnzEntries()], x, y);
    }
}

//--------------------------------------------------------------------------------------------
// Die Schalter wird eröffnet:
//--------------------------------------------------------------------------------------------
CWerkstatt::CWerkstatt(BOOL bHandy, ULONG PlayerNum) : CStdRaum(bHandy, PlayerNum, "werkstat.gli", GFX_WERKSTAT) {
    SetRoomVisited(PlayerNum, ROOM_WERKSTATT);

    Sim.FocusPerson = -1;

    if (bHandy == 0) {
        AmbientManager.SetGlobalVolume(40);
    }

    KommVar = -1;

    Talkers.Talkers[TALKER_MECHANIKER].IncreaseReference();
    DefaultDialogPartner = TALKER_MECHANIKER;

    OilCanBm.ReSize(pRoomLib, GFX_OIL);
    DoorBm.ReSize(pRoomLib, GFX_DOOR);

    SawFx.ReInit("saw.raw");

    SP_Mann.ReSize(9);
    //--------------------------------------------------------------------------------------------
    SP_Mann.Clips[0].ReSize(0, "wk_wait.smk", "", XY(190, 209), SPM_IDLE, CRepeat(1, 1), CPostWait(10, 10), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, nullptr, // Warten
                            "A9A1E1", 0, 4, 1);

    SP_Mann.Clips[4].ReSize(4, "wk_tuch.smk", "", XY(190, 209), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 5);
    SP_Mann.Clips[5].ReSize(5, "wk_putz.smk", "", XY(190, 209), SPM_IDLE, CRepeat(1, 3), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 6);
    SP_Mann.Clips[6].ReSize(6, "wk_tuchz.smk", "", XY(190, 209), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, nullptr, // Warten
                            "A9", 0);

    SP_Mann.Clips[1].ReSize(1, "wk_turn.smk", "", XY(190, 209), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, "E1E1", 2, 3);

    SP_Mann.Clips[2].ReSize(2, "wk_rede.smk", "", XY(190, 209), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, nullptr, "A9E1E1", 2, 3, 7);
    SP_Mann.Clips[3].ReSize(3, "wk_redew.smk", "", XY(190, 209), SPM_LISTENING, CRepeat(1, 1), CPostWait(10, 10), SMACKER_CLIP_CANCANCEL, nullptr,
                            SMACKER_CLIP_SET, 0, nullptr, // Warten
                            "A9E1E1A3", 3, 2, 7, 8);
    SP_Mann.Clips[8].ReSize(8, "wk_rauch.smk", "", XY(173, 209), SPM_LISTENING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &KommVar,
                            SMACKER_CLIP_SET | (SMACKER_CLIP_FRAME + 18 * SMACKER_CLIP_MULT), 1, nullptr, // Warten
                            "A9E1E1", 3, 2, 7);

    SP_Mann.Clips[7].ReSize(7, "wk_turnz.smk", "", XY(190, 209), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, nullptr, // Warten
                            "A9", 0);

    SP_Wer.ReSize(4);
    //--------------------------------------------------------------------------------------------
    SP_Wer.Clips[0].ReSize(0, "werstand.smk", "", XY(535, 72), SPM_IDLE, CRepeat(1, 1), CPostWait(100, 210), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                           0, nullptr, "A9", 1);
    SP_Wer.Clips[1].ReSize(1, "werl.smk", "", XY(535, 72), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                           nullptr, "A9", 2);
    SP_Wer.Clips[2].ReSize(2, "werstand.smk", "", XY(535, 72), SPM_IDLE, CRepeat(1, 1), CPostWait(100, 210), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                           0, nullptr, "A9", 3);
    SP_Wer.Clips[3].ReSize(3, "werr.smk", "", XY(535, 72), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                           nullptr, "A9", 0);

    SP_Schleim.ReSize(3);
    //--------------------------------------------------------------------------------------------
    if (Sim.Slimed == -1) {
        if (rand() % 10 == 0 || (CheatAnimNow != 0)) {
            SP_Schleim.Clips[0].ReSize(0, "slimew.smk", "", XY(0, 275), SPM_IDLE, CRepeat(1, 1), CPostWait(30, 30), SMACKER_CLIP_CANCANCEL, nullptr,
                                       SMACKER_CLIP_SET, 0, &Sim.Slimed, "A8A1", 0, 1);
        } else {
            SP_Schleim.Clips[0].ReSize(0, "slimew.smk", "", XY(0, 275), SPM_IDLE, CRepeat(1, 1), CPostWait(30, 30), SMACKER_CLIP_CANCANCEL, nullptr,
                                       SMACKER_CLIP_SET, 0, &Sim.Slimed, "A4", 0);
        }
    } else {
        SP_Schleim.Clips[0].ReSize(0, "slime02.smk", "", XY(0, 275), SPM_IDLE, CRepeat(99, 99), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                   SMACKER_CLIP_SET, 0, nullptr, "A9", 2);
    }
    SP_Schleim.Clips[1].ReSize(1, "slime01.smk", "slime01.raw", XY(0, 275), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &Sim.Slimed,
                               SMACKER_CLIP_SET | SMACKER_CLIP_POST, 2, nullptr, "A9", 2);
    SP_Schleim.Clips[2].ReSize(2, "slime02.smk", "", XY(0, 275), SPM_IDLE, CRepeat(99, 99), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                               0, nullptr, "A9", 2);

    for (SLONG c = 0; c < 5; c++) {
        SP_Blase[c].ReSize(3);
        SP_Blase[c].Clips[0].ReSize(0, "blasew.smk", "", SlimePositions[c], SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr,
                                    SMACKER_CLIP_SET, 0, &Sim.Slimed, "A9", 0);
        SP_Blase[c].Clips[1].ReSize(1, bprintf("blase%02li.smk", c % 3 + 1), "", SlimePositions[c], SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0),
                                    SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0, nullptr, "A9", 0);
        SP_Blase[c].Clips[2].ReSize(2, "blasew.smk", "", SlimePositions[c], SPM_IDLE, CRepeat(1, 1), CPostWait(30 + rand() % 80, 130 + rand() % 80),
                                    SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0, nullptr, "A9", 1);
    }

    SP_Bombe.ReSize(2);
    //--------------------------------------------------------------------------------------------
    SP_Bombe.Clips[0].ReSize(0, "bombew.smk", "", XY(180, 386), SPM_IDLE, CRepeat(1, 1), CPostWait(100, 210), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                             0, &KommVar, "A9", 0);
    SP_Bombe.Clips[1].ReSize(1, "bombe.smk", "bombe.raw", XY(180, 386), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &KommVar,
                             SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, nullptr, "A9", 0);

    SP_Hund.ReSize(1);
    //--------------------------------------------------------------------------------------------
    SP_Hund.Clips[0].ReSize(0, "spike.smk", "", XY(529, 115), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, "A9", 0);

    // Hintergrundsounds:
    if (Sim.Options.OptionEffekte != 0) {
        SetBackgroundFx(1, "crash.raw", 100000, 10000);
        SetBackgroundFx(2, "crash2.raw", 90000, 40000);
        SetBackgroundFx(3, "crash3.raw", 120000, 75000);
    }

    // Raumanimationen
    if (rand() % 6 == 0 || (CheatAnimNow != 0)) {
        SoudAnim.ReSize(pRoomLib, "SOUD0000", 21, &SawFx, FALSE, ANIMATION_MODE_REPEAT, 150, 2, 100);
    }
    FlameAnim.ReSize(pRoomLib, "FLAME01", 3, nullptr, TRUE, ANIMATION_MODE_REPEAT, 0, 1);
    LightAnim.ReSize(pRoomLib, "LIGHT01", 2, nullptr, FALSE, ANIMATION_MODE_REPEAT, 400, 2, 300, 30);

#ifdef DEMO
    MenuStart(MENU_REQUEST, MENU_REQUEST_NO_WERKSTATT);
    MenuSetZoomStuff(XY(320, 220), 0.17, FALSE);
#endif
}

//--------------------------------------------------------------------------------------------
// Sehr destruktiv!
//--------------------------------------------------------------------------------------------
CWerkstatt::~CWerkstatt() { Talkers.Talkers[TALKER_MECHANIKER].DecreaseReference(); }

//////////////////////////////////////////////////////////////////////////////////////////////
// CWerkstatt message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// void CWerkstatt::OnPaint()
//--------------------------------------------------------------------------------------------
void CWerkstatt::OnPaint() {
    SLONG NewTip = 0;

    if (bHandy == 0) {
        SetMouseLook(CURSOR_NORMAL, 0, ROOM_ARAB_AIR, 0);
    }

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips();

    // Der Mann im Flugzeug:
    if ((Sim.Players.Players[PlayerNum].SecurityFlags & (1 << 7)) != 0U) {
        SP_Hund.Pump();
        SP_Hund.BlitAtT(RoomBm);
    } else {
        SP_Wer.Pump();
        SP_Wer.BlitAtT(RoomBm);
    }

    SP_Schleim.Pump();
    SP_Schleim.BlitAtT(RoomBm);

    if (Sim.Slimed != -1) {
        for (auto &c : SP_Blase) {
            c.Pump();
            c.BlitAtT(RoomBm);
        }
    }

    if ((Sim.Players.Players[PlayerNum].SecurityFlags & (1 << 7)) == 0U) {
        RoomBm.BlitFromT(DoorBm, 534, 73);
    }

    if (Sim.Players.Players[PlayerNum].HasItem(ITEM_OEL) == 0) {
        RoomBm.BlitFrom(OilCanBm, 72, 294);
    }

    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
        if (gMousePosition.IfIsWithin(0, 0, 50, 439)) {
            SetMouseLook(CURSOR_EXIT, 0, ROOM_WERKSTATT, 999);
        } else if (gMousePosition.IfIsWithin(195, 212, 263, 437)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_WERKSTATT, 10);
        } else if ((Sim.Players.Players[PlayerNum].HasItem(ITEM_OEL) == 0) && gMousePosition.IfIsWithin(72, 294, 101, 315)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_WERKSTATT, 20);
        }
    }

    SP_Mann.Pump();
    SP_Mann.BlitAtT(RoomBm);
    if (Sim.Slimed == -1) {
        KommVar = -1;
    }

    SP_Bombe.Pump();
    SP_Bombe.BlitAtT(RoomBm);

    // Die Raum-Animationen:
    SoudAnim.BlitAt(RoomBm, 470, 290);
    FlameAnim.BlitAt(RoomBm, 104, 314);
    LightAnim.BlitAtT(RoomBm, 244, 89);

    if (MenuIsOpen() != 0) {
        if (CurrentMenu == MENU_PLANECOSTS && (gMousePosition - MenuPos).IfIsWithin(216, 6, 387, 212)) {
            NewTip = (gMousePosition.y - (MenuPos.y + 25)) / 13 + MenuPage;

            if (NewTip >= 0 && NewTip - MenuPage < 13 && NewTip < MenuDataTable.LineIndex.AnzEntries() &&
                (Sim.Players.Players[PlayerNum].Planes.IsInAlbum(MenuDataTable.LineIndex[NewTip]) != 0)) {
                if (NewTip != CurrentTip) {
                    MenuRepaint();
                    // DrawPlaneTipContents (OnscreenBitmap, &PlaneTypes[Sim.Players.Players[(SLONG)PlayerNum].Planes[MenuDataTable.LineIndex[NewTip]].TypeId],
                    // &Sim.Players.Players[(SLONG)PlayerNum].Planes[MenuDataTable.LineIndex[NewTip]],
                    DrawPlaneTipContents(OnscreenBitmap, nullptr, &Sim.Players.Players[PlayerNum].Planes[MenuDataTable.LineIndex[NewTip]], XY(6, 6), XY(6, 28),
                                         &FontSmallBlack, &FontSmallBlack, TRUE);
                }

                if (MenuDataTable.ValueFlags[0 + NewTip * MenuDataTable.AnzColums] != 0U) {
                    CheckCursorHighlight(
                        ReferenceCursorPos,
                        CRect(MenuPos.x + 216, MenuPos.y + (NewTip - MenuPage) * 13 + 25 - 2, MenuPos.x + 387, MenuPos.y + (NewTip - MenuPage) * 13 + 25 + 12),
                        ColorOfFontRed, CURSOR_HOT);
                } else {
                    CheckCursorHighlight(
                        ReferenceCursorPos,
                        CRect(MenuPos.x + 216, MenuPos.y + (NewTip - MenuPage) * 13 + 25 - 2, MenuPos.x + 387, MenuPos.y + (NewTip - MenuPage) * 13 + 25 + 12),
                        ColorOfFontBlack, CURSOR_HOT);
                }

                CurrentTip = NewTip;
            } else {
                NewTip = -1;
            }
        } else {
            NewTip = -1;
        }

        if (NewTip != CurrentTip) {
            MenuRepaint();
            CurrentTip = NewTip;
        }
    }

#ifdef DEMO
    if (!IsDialogOpen() && !MenuIsOpen())
        SetMouseLook(CURSOR_EXIT, 0, ROOM_WERKSTATT, 999);
#endif

    CStdRaum::PostPaint();
    CStdRaum::PumpToolTips();
}

//--------------------------------------------------------------------------------------------
// void CWerkstatt::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CWerkstatt::OnLButtonDown(UINT nFlags, CPoint point) {
    XY RoomPos;
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    DefaultOnLButtonDown();

    if (ConvertMousePosition(point, &RoomPos) == 0) {
        CStdRaum::OnLButtonDown(nFlags, point);
        return;
    }

    if (PreLButtonDown(point) == 0) {
        if (MouseClickArea == ROOM_WERKSTATT && MouseClickId == 999) {
            Sim.Players.Players[PlayerNum].LeaveRoom();
        } else if (MouseClickArea == ROOM_WERKSTATT && MouseClickId == 10) {
            StartDialog(TALKER_MECHANIKER, MEDIUM_AIR, 1);
        } else if (MouseClickArea == ROOM_WERKSTATT && MouseClickId == 20) {
            if (qPlayer.MechTrust == 0) {
                StartDialog(TALKER_MECHANIKER, MEDIUM_AIR, 20);
            } else {
                if (qPlayer.HasSpaceForItem() != 0) {
                    StartDialog(TALKER_MECHANIKER, MEDIUM_AIR, 21);
                    qPlayer.BuyItem(ITEM_OEL);
                }
            }
        } else {
            CStdRaum::OnLButtonDown(nFlags, point);
        }
    }
}

//--------------------------------------------------------------------------------------------
// void CWerkstatt::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CWerkstatt::OnRButtonDown(UINT nFlags, CPoint point) {
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
