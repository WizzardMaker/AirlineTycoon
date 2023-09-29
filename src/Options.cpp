//============================================================================================
// Options.cpp : Das Optionsmenü
//============================================================================================
// Link: "Options.h"
//============================================================================================
#include "StdAfx.h"
#include "AtNet.h"
#include "SbLib.h"
#include "glstart.h"
#include "network.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern SLONG OptionsShortcut;
extern SLONG NewgameWantsToLoad;
extern SLONG NewgameToOptions;
extern SLONG gNetworkSavegameLoading;

extern SBNetwork gNetwork;

static SLONG nLocalOptionsOption = 0;

//--------------------------------------------------------------------------------------------
// ULONG PlayerNum
//--------------------------------------------------------------------------------------------
Options::Options(BOOL bHandy, SLONG PlayerNum) : CStdRaum(bHandy, PlayerNum, "startup.gli", GFX_HAUPTMNU) {
    nLocalOptionsOption++;

    ChangedDisplay = 0;

    // Das Optionen-Fenster ist offen! Alles anhalten!
    nOptionsOpen++;
    if (Sim.bNetwork != 0) {
        SIM::SendSimpleMessage(ATNET_OPTIONS, 0, 1, Sim.localPlayer);
    }

    if (bHandy == 0) {
        AmbientManager.SetGlobalVolume(0);
    }

    AmbientFX.ReInit("raunen.raw");
    EffektFX.ReInit("kaffee.raw");
    PlaneFX.ReInit("prop.raw");
    TalkFX.ReInit("opt_say.raw");
    DurchsagenFX.ReInit("opt_hinw.raw");
    ClickFx.ReInit("change.raw");

    AT_Log_Generic("Loaded font: stat_1.mcf");
    VersionFont.Load(lpDD, const_cast<char*>((LPCTSTR)FullFilename("stat_1.mcf", MiscPath)));

    Options::PageNum = 1;
    Options::PlayerNum = PlayerNum;
    Options::CursorY = -1;
    Options::BlinkState = 0;

    if (NewgameWantsToLoad != 0) {
        UpdateSavegameNames();
        Options::PageNum = 5;
    }

    if (OptionsShortcut != -1) {
        UpdateSavegameNames();
        Options::PageNum = static_cast<UBYTE>(OptionsShortcut);
    }

    if ((Sim.Gamestate & 31) != GAMESTATE_INIT) {
        gKlackerPlanes.Reset();
    }

    RefreshKlackerField();

    // Create a timer to 'klacker'
    TimerId = SDL_AddTimer(50, TimerFunc, this);
    if (TimerId == 0) {
        TimerFailure = 1;
    } else {
        TimerFailure = 0;
    }
}

//--------------------------------------------------------------------------------------------
// Options-Fenster zerstören:
//--------------------------------------------------------------------------------------------
Options::~Options() {
    SLONG c = 0;
    if (TimerId != 0) {
        SDL_RemoveTimer(TimerId);
    }

    nLocalOptionsOption--;

    if (Sim.bNetwork != 0) {
        if (Sim.bIsHost && Sim.ServerGameSpeed != Sim.GameSpeed) {
            SIM::SendSimpleMessage(ATNET_SETGAMESPEED, 0, Sim.GameSpeed, Sim.localPlayer);
            Sim.ServerGameSpeed = Sim.GameSpeed;
            DisplayBroadcastMessage(bprintf("GameSpeed changed to %i / 7\n", static_cast<int>(std::floor(Sim.ServerGameSpeed / 5)) + 1), Sim.localPlayer);
              
        }

        SIM::SendSimpleMessage(ATNET_OPTIONS, 0, -1, Sim.localPlayer);
    }

    Sim.Options.WriteOptions();

    if (bLeaveGameLoop == 0) {
        for (c = 0; c < Sim.Players.AnzPlayers; c++) {
            if (Sim.Players.Players[c].IsOut == 0) {
                Sim.Players.Players[c].CalcRoom();
            }
        }
    }

    gDisablePauseKey = FALSE;
    if (nOptionsOpen > 0) {
        nOptionsOpen--;
    }
    if (Sim.bNetwork != 0) {
        SetNetworkBitmap(static_cast<SLONG>(nOptionsOpen > 0) * 1);
    }

    Sim.SaveOptions();

    if (NewgameWantsToLoad == 1) {
        NewgameWantsToLoad = FALSE;
    }
    NewgameToOptions = FALSE;

    StatusCount = 60;
    OptionsShortcut = -1;
}

//--------------------------------------------------------------------------------------------
// Welche Savegame Names haben wir auf der Pladde?
//--------------------------------------------------------------------------------------------
void Options::UpdateSavegameNames() {
    SLONG c = 0;
    CString Filename;

    const char *pNamebaseStr = nullptr;

    if (Sim.bNetwork != 0) {
        pNamebaseStr = "net%li.dat";
    } else {
        pNamebaseStr = "game%li.dat";
    }

    for (c = 0; c < 12; c++) {
        Filename = FullFilename((LPCTSTR)bprintf(pNamebaseStr, c), SavegamePath);

        if (DoesFileExist(Filename) != 0) {
            SLONG SaveVersion = 0;
            SLONG SaveVersionSub = 0;
            TEAKFILE InputFile(Filename, TEAKFILE_READ);

            InputFile >> SavegameNames[c];
            SavenamesValid[c] = TRUE;

            InputFile >> SaveVersion >> SaveVersionSub;

            if (SaveVersionSub > 0) {
                // Info-String ignorieren
                InputFile >> SavegameInfos[c];
            }
        } else {
            SavegameNames[c] = StandardTexte.GetS(TOKEN_MISC, 4073);
            SavenamesValid[c] = FALSE;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Aktualisiert die Text-Daten im Klacker-Feld:
//--------------------------------------------------------------------------------------------
void Options::RefreshKlackerField() {
    SLONG c = 0;

    KlackerTafel.Clear();

    KlackerTafel.PrintAt(0, 1, "========================");

    if (PageNum == 1) {
        KlackerTafel.PrintAt(0, 0, StandardTexte.GetS(TOKEN_MISC, 4000));
        KlackerTafel.PrintAt(0, 2, StandardTexte.GetS(TOKEN_MISC, 4001));
        KlackerTafel.PrintAt(0, 3, StandardTexte.GetS(TOKEN_MISC, 4002));
        KlackerTafel.PrintAt(0, 4, StandardTexte.GetS(TOKEN_MISC, 4003));

        if ((Sim.Gamestate & 31) != GAMESTATE_INIT) {
            KlackerTafel.PrintAt(0, 6, StandardTexte.GetS(TOKEN_MISC, 4004));
            KlackerTafel.PrintAt(0, 7, StandardTexte.GetS(TOKEN_MISC, 4005));
        }

        if ((Sim.Gamestate & 31) == GAMESTATE_INIT) {
            KlackerTafel.PrintAt(0, 6, StandardTexte.GetS(TOKEN_MISC, 4007));
        } else {
            KlackerTafel.PrintAt(0, 9, StandardTexte.GetS(TOKEN_MISC, 4006));
            KlackerTafel.PrintAt(0, 11, StandardTexte.GetS(TOKEN_MISC, bFirstClass != 0 ? 4007 : 4008));
        }

        // KlackerTafel.PrintAt (0, 15, VersionString);
    } else if (PageNum == 2) // Grafik-Optionen
    {
        KlackerTafel.PrintAt(0, 0, StandardTexte.GetS(TOKEN_MISC, 4010));
        KlackerTafel.PrintAt(0, 2, StandardTexte.GetS(TOKEN_MISC, 4011 + Sim.Options.OptionPlanes));
        KlackerTafel.PrintAt(0, 3, StandardTexte.GetS(TOKEN_MISC, 4013 + Sim.Options.OptionPassengers));
        KlackerTafel.PrintAt(0, 4, StandardTexte.GetS(TOKEN_MISC, 4015 + Sim.Options.OptionBlenden));
        KlackerTafel.PrintAt(0, 5, StandardTexte.GetS(TOKEN_MISC, 4020 + Sim.Options.OptionThinkBubbles));
        KlackerTafel.PrintAt(0, 6, StandardTexte.GetS(TOKEN_MISC, 4022 + Sim.Options.OptionFlipping));
        KlackerTafel.PrintAt(0, 7, StandardTexte.GetS(TOKEN_MISC, 4024 + Sim.Options.OptionTransparenz));
        KlackerTafel.PrintAt(0, 8, StandardTexte.GetS(TOKEN_MISC, 4026 + Sim.Options.OptionSchatten));

        KlackerTafel.PrintAt(0, 10,
                             Sim.Options.OptionFullscreen == 0   ? "# Display : Fullscreen"
                             : Sim.Options.OptionFullscreen == 1 ? "# Display : Windowed"
                             : Sim.Options.OptionFullscreen == 2 ? "# Display : Borderless"
                                                                 : "???");
        KlackerTafel.PrintAt(0, 11, ((static_cast<bool>(Sim.Options.OptionKeepAspectRatio)) ? "# Aspect Ratio: Keep" : "# Aspect Ratio: Stretch"));

        KlackerTafel.PrintAt(0, 13, StandardTexte.GetS(TOKEN_MISC, 4099));
    } else if (PageNum == 3) // Musik-Optionen
    {
        KlackerTafel.PrintAt(0, 0, StandardTexte.GetS(TOKEN_MISC, 4120));

        const bool usesMidi = Sim.Options.OptionMusicType == 1;
        const bool usesOgg = Sim.Options.OptionMusicType == 2;
        SLONG musicShift = usesMidi ? -1 : 0;

        KlackerTafel.PrintAt(0, 2, usesMidi ? "Music: Midi" : usesOgg ? "Music: Ogg" : "Music: Off");

        if (Sim.Options.OptionMusicType != 0) {
            if (!usesMidi) {
                KlackerTafel.PrintAt(1, 3, StandardTexte.GetS(TOKEN_MISC, 4121));
                KlackerTafel.PrintVolumeAt(15, 3, 8, Sim.Options.OptionMusik);
            }
            KlackerTafel.PrintAt(1, 4 + musicShift, StandardTexte.GetS(TOKEN_MISC, 4150 + Sim.Options.OptionLoopMusik));
            if (Sim.Options.OptionLoopMusik == 0) {
                KlackerTafel.PrintAt(1, 5 + musicShift, StandardTexte.GetS(TOKEN_MISC, 4140));
            }
        }

        KlackerTafel.PrintAt(0, 7, "Sound:");
        KlackerTafel.PrintAt(1, 8, StandardTexte.GetS(TOKEN_MISC, 4127));
        KlackerTafel.PrintAt(1, 9, StandardTexte.GetS(TOKEN_MISC, 4122));
        KlackerTafel.PrintAt(1, 10, StandardTexte.GetS(TOKEN_MISC, 4123));
        KlackerTafel.PrintAt(1, 11, StandardTexte.GetS(TOKEN_MISC, 4124));
        KlackerTafel.PrintAt(1, 12, StandardTexte.GetS(TOKEN_MISC, 4125));
        KlackerTafel.PrintAt(1, 13, StandardTexte.GetS(TOKEN_MISC, 4126));
        KlackerTafel.PrintVolumeAt(15, 8, 8, Sim.Options.OptionMasterVolume);
        KlackerTafel.PrintVolumeAt(15, 9, 8, Sim.Options.OptionAmbiente);
        KlackerTafel.PrintVolumeAt(15, 10, 8, Sim.Options.OptionDurchsagen);
        KlackerTafel.PrintVolumeAt(15, 11, 8, Sim.Options.OptionTalking);
        KlackerTafel.PrintVolumeAt(15, 12, 8, Sim.Options.OptionEffekte);
        KlackerTafel.PrintVolumeAt(15, 13, 8, Sim.Options.OptionPlaneVolume);

        KlackerTafel.PrintAt(0, 15, StandardTexte.GetS(TOKEN_MISC, 4099));
    } else if (PageNum == 4) // Sonstiges
    {
        KlackerTafel.PrintAt(0, 0, StandardTexte.GetS(TOKEN_MISC, 4030));
        KlackerTafel.PrintAt(0, 2, StandardTexte.GetS(TOKEN_MISC, 4031 + Sim.Options.OptionGirl));
        KlackerTafel.PrintAt(0, 3, StandardTexte.GetS(TOKEN_MISC, 4033 + Sim.Options.OptionBerater));
        KlackerTafel.PrintAt(0, 4, StandardTexte.GetS(TOKEN_MISC, 4038 + Sim.Options.OptionAutosave));
        KlackerTafel.PrintAt(0, 5, StandardTexte.GetS(TOKEN_MISC, 4040 + Sim.Options.OptionFax));
        KlackerTafel.PrintAt(0, 6, StandardTexte.GetS(TOKEN_MISC, 4042 + Sim.Options.OptionRealKuerzel));
        KlackerTafel.PrintAt(0, 7, StandardTexte.GetS(TOKEN_MISC, 4044 + Sim.Options.OptionSpeechBubble));
        KlackerTafel.PrintAt(0, 8, StandardTexte.GetS(TOKEN_MISC, 4048 + Sim.Options.OptionBriefBriefing));
        KlackerTafel.PrintAt(0, 9, StandardTexte.GetS(TOKEN_MISC, 4050 + Sim.Options.OptionRandomStartday));
        KlackerTafel.PrintAt(0, 10, bprintf("# %s" , StandardTexte.GetS(TOKEN_PLANE, 1002)));
        KlackerTafel.PrintVolumeAt(17, 10, 7, Sim.GameSpeed == 1 ? 0 : Sim.GameSpeed / 5);
        KlackerTafel.PrintAt(0, 12, StandardTexte.GetS(TOKEN_MISC, 4099));
    } else if (PageNum == 5) // Laden
    {
        KlackerTafel.PrintAt(0, 0, StandardTexte.GetS(TOKEN_MISC, 4070));

        for (c = 0; c < 12; c++) {
            KlackerTafel.PrintAt(1, 2 + c, bprintf("%2li:%s", c + 1, (LPCTSTR)SavegameNames[c]));
        }

        KlackerTafel.PrintAt(0, 15, StandardTexte.GetS(TOKEN_MISC, 4096));
    } else if (PageNum == 6) // Speichern
    {
        KlackerTafel.PrintAt(0, 0, StandardTexte.GetS(TOKEN_MISC, 4071));

        for (c = 0; c < 11; c++) {
            KlackerTafel.PrintAt(1, 2 + c, bprintf("%2li:%s", c + 1, (LPCTSTR)SavegameNames[c]));
        }

        if (CursorY != -1) {
            KlackerTafel.PrintAt(0, 15, StandardTexte.GetS(TOKEN_MISC, 4097));
            KlackerTafel.PrintAt(22, 15, StandardTexte.GetS(TOKEN_MISC, 4098));
        } else {
            KlackerTafel.PrintAt(0, 15, StandardTexte.GetS(TOKEN_MISC, 4096));
        }
    } else if (PageNum == 7) // Quit
    {
        KlackerTafel.PrintAt(0, 0, StandardTexte.GetS(TOKEN_MISC, 4060));
        KlackerTafel.PrintAt(0, 2, StandardTexte.GetS(TOKEN_MISC, 4061));
        KlackerTafel.PrintAt(0, 4, StandardTexte.GetS(TOKEN_MISC, 4062));
        KlackerTafel.PrintAt(0, 5, StandardTexte.GetS(TOKEN_MISC, 4063));
    } else if (PageNum == 8) // New Game
    {
        KlackerTafel.PrintAt(0, 0, StandardTexte.GetS(TOKEN_MISC, 4065));
        KlackerTafel.PrintAt(0, 2, StandardTexte.GetS(TOKEN_MISC, 4061));
        KlackerTafel.PrintAt(0, 4, StandardTexte.GetS(TOKEN_MISC, 4062));
        KlackerTafel.PrintAt(0, 5, StandardTexte.GetS(TOKEN_MISC, 4063));
    }
}

/////////////////////////////////////////////////////////////////////////////
// Options message handlers

//--------------------------------------------------------------------------------------------
// void Options::OnPaint():
//--------------------------------------------------------------------------------------------
void Options::OnPaint() {
    static SLONG x;
    static SLONG y;
    static SLONG py;
    x++;
    static SLONG LastLine = -1;

    if (nLocalOptionsOption == 0) {
        return;
    }

    SLONG Line = (gMousePosition.y - 63) / 22;
    SLONG Column = (gMousePosition.x - 128) / 16;

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips();

    if (Column < 0 || Column > 24) {
        Line = -1;
        Column = -1;
    }

    if (PageNum == 3 && (gMouseLButton != 0)) // Sound Slide
    {
        if (Column >= 15 && Column < 23 && Line >= 3 && Line <= 13) {
            if (Line == 9 && Sim.Options.OptionAmbiente != Column - 15) {
                Sim.Options.OptionAmbiente = Column - 15;
                AmbientManager.RecalcVolumes();
                AmbientFX.SetVolume(Prozent2Dezibel(Sim.Options.OptionAmbiente * 100 / 8));
                AmbientFX.Play(DSBPLAY_NOSTOP | DSBPLAY_LOOPING);
            }
            if (Line == 10 && Sim.Options.OptionDurchsagen != Column - 15) {
                Sim.Options.OptionDurchsagen = Column - 15;
                DurchsagenFX.SetVolume(Prozent2Dezibel(Sim.Options.OptionDurchsagen * 100 / 8));
                DurchsagenFX.Play();
            }

#if !defined(NO_D_VOICES) || !defined(NO_E_VOICES) || !defined(NO_N_VOICES)
            if (!bVoicesNotFound) {
                if (Line == 11 && Sim.Options.OptionTalking != Column - 15) {
                    Sim.Options.OptionTalking = Column - 15;
                    TalkFX.SetVolume(Prozent2Dezibel(Sim.Options.OptionTalking * 100 / 8));
                    TalkFX.Play();
                }
            }
#endif

            if (Line == 12 && Sim.Options.OptionEffekte != Column - 15) {
                Sim.Options.OptionEffekte = Column - 15;
                EffektFX.SetVolume(Prozent2Dezibel(Sim.Options.OptionEffekte * 100 / 8));
                EffektFX.Play();
            }
            if (Line == 13 && Sim.Options.OptionPlaneVolume != Column - 15) {
                Sim.Options.OptionPlaneVolume = Column - 15;
                PlaneFX.SetVolume(Prozent2Dezibel(Sim.Options.OptionPlaneVolume * 100 / 8));
                PlaneFX.Play();
            }
            RefreshKlackerField();
            KlackerTafel.Warp();
        }
    }

    if (TimerFailure != 0) {
        KlackerTafel.Klack(); // Tafel notfalls asynchron aktualisieren
    }

    static SLONG cnt = 0;
    cnt++;

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    if (bActive != 0) {
        // Klacker-Felder:
        if (PageNum == 5 || PageNum == 6) {
            BOOL SavenameValid = 0;

            for (py = 63, y = 0; y < 16; y++, py += 22) {
                SavenameValid = TRUE;

                if (y >= 2 && y < 14 && (SavenamesValid[y - 2] == 0) && CursorY != y - 2) {
                    SavenameValid = FALSE;
                }

                for (x = 0; x < 24; x++) {
                    if (KlackerTafel.Haben[x + y * 24] != 0) {
                        RoomBm.BlitFrom(KlackerTafel.KlackerBms[static_cast<SLONG>(KlackerTafel.Haben[x + y * 24]) +
                                                                static_cast<SLONG>(SavenameValid == 0) * (73 + 8 + 3 + 3)],
                                        128 + x * 16, py);
                    }
                }
            }
        } else {
            for (py = 63, y = 0; y < 16; y++, py += 22) {
                for (x = 0; x < 24; x++) {
                    if (KlackerTafel.Haben[x + y * 24] != 0) {
                        RoomBm.BlitFrom(KlackerTafel.KlackerBms[static_cast<SLONG>(KlackerTafel.Haben[x + y * 24])], 128 + x * 16, py);
                    }
                }
            }
        }

        if (CursorY != -1) {
            RoomBm.BlitFromT(KlackerTafel.Cursors[BlinkState % 8], (CursorX + 4) * 16 + 129, (CursorY + 1) * 22 + 85);
        }

        // Cursor highlighting:
        switch (PageNum) {
        case 1: // Startseite:
            if (Line == 2 || Line == 3 || Line == 4) {
                SetMouseLook(CURSOR_HOT, 0, -100, 0);
            }

            if ((Sim.Gamestate & 31) == GAMESTATE_INIT) {
                if (Line == 6) {
                    SetMouseLook(CURSOR_HOT, 0, -100, 0);
                }
            } else {
                if (Line == 6 || Line == 7 || Line == 9 || Line == 11) {
                    SetMouseLook(CURSOR_HOT, 0, -100, 0);
                }
            }
            break;

        case 2: // Grafik:
            if ((Line >= 2 && Line <= 8) || Line == 10 || Line == 11 || Line == 13) {
                SetMouseLook(CURSOR_HOT, 0, -100, 0);
            }
            break;

        case 3: // Sound:
        {
            const bool usesMusic = Sim.Options.OptionMusicType != 0;
            const bool usesMidi = Sim.Options.OptionMusicType == 1;
            const SLONG musicShift = usesMidi ? -1 : 0;

            if (Column >= 15 && Column < 23 && Line >= 3 && Line <= 15) {
                if ((Line >= 8 && Line <= 13) || (Line == 3 && !usesMidi && usesMusic)) {
                    SetMouseLook(CURSOR_HOT, 0, -100, 0);
                }
            }
            if (Line == 2 || (Line == 4 + musicShift && usesMusic) || Line == 15) {
                SetMouseLook(CURSOR_HOT, 0, -100, 0);
            }
            if (Line == 5 + musicShift && Sim.Options.OptionLoopMusik == 0 && usesMusic) {
                SetMouseLook(CURSOR_HOT, 0, -100, 0);
            }
            break;
        }
        case 4: // Sonstiges:
            if ((Line >= 2 && Line <= 9) || Line == 12) {
                SetMouseLook(CURSOR_HOT, 0, -100, 0);
            }
            if (Line == 10) 
            {
                SetMouseLook(Column >= 17 && Column < 23 ? CURSOR_HOT : CURSOR_NORMAL, 1003, -100, 0);
            }
            break;

        case 5: // Laden
            if (Line >= 2 && Line <= 13) {
                if (SavegameInfos[Line - 2].GetLength() > 0 && (MenuIsOpen() == 0)) {
                    SetMouseLook(CURSOR_HOT, 5000 + Line, SavegameInfos[Line - 2], ROOM_OPTIONS, 0);
                    if (ToolTipState == FALSE) {
                        ToolTipTimer = AtGetTime() - 601;
                    }

                    if (Line != LastLine) {
                        ToolTipState = FALSE;
                    }
                }
            }
            if (Line == 15 && Column < 10) {
                SetMouseLook(CURSOR_HOT, 0, -100, 0);
            }
            break;

        case 6: // Speichern
            if (Line >= 2 && Line <= 12 && CursorY == -1) {
                if (SavegameInfos[Line - 2].GetLength() > 0) {
                    SetMouseLook(CURSOR_HOT, 5000 + Line, SavegameInfos[Line - 2], ROOM_OPTIONS, 0);
                    if (ToolTipState == FALSE) {
                        ToolTipTimer = AtGetTime() - 601;
                    }
                }
            }
            if (Line == 15 && Column < 10) {
                SetMouseLook(CURSOR_HOT, 0, -100, 0);
            }
            if (Line == 15 && Column >= 22 && Column < 24 && CursorY != -1) {
                SetMouseLook(CURSOR_HOT, 0, -100, 0);
            }
            break;

        case 7: // Quit:
            if (Line == 4 || Line == 5) {
                SetMouseLook(CURSOR_HOT, 0, -100, 0);
            }
            break;

        case 8: // Leave Game:
            if (Line == 4 || Line == 5) {
                SetMouseLook(CURSOR_HOT, 0, -100, 0);
            }
            break;
        default:
            hprintf("Options.cpp: Default case should not be reached.");
            DebugBreak();
        }

        RoomBm.PrintAt(VersionString, FontSmallRed, TEC_FONT_RIGHT, XY(0, 429), XY(519, 480));
        gKlackerPlanes.PostPaint(RoomBm);

        if (CursorY != -1) {
            gKlackerPlanes.Pump(XY((CursorX + 4) * 16 + 129, (CursorY + 1) * 44 + 85));
        } else {
            gKlackerPlanes.Pump(gMousePosition);
        }
    }

    LastLine = Line;

    CStdRaum::PostPaint();
}

//--------------------------------------------------------------------------------------------
// void Options::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void Options::OnLButtonDown(UINT /*nFlags*/, CPoint point) {
    if (CursorY != -1 && PageNum != 6) {
        return;
    }

    if (MenuIsOpen() != 0) {
        MenuRightClick(point);
    } else if (PreLButtonDown(point) == 0) {
        ClickFx.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
        DefaultOnLButtonDown();

        SLONG Line = (point.y - 63) / 22;
        SLONG Column = (point.x - 128) / 16;

        switch (PageNum) {
        case 1: // Startseite:
            if (Line == 2) {
                PageNum = 2; // Grafik-Optionen
            }
            if (Line == 3) {
                PageNum = 3; // Musik-Optionen
            }
            if (Line == 4) {
                PageNum = 4; // Sonstiges
            }

            if ((Sim.Gamestate & 31) == GAMESTATE_INIT) {
                if (Line == 6) {
                    KlackerTafel.Warp();
                    FrameWnd->Invalidate();
                    MessagePump();
                    FrameWnd->Invalidate();
                    MessagePump();
                    Sim.Gamestate = GAMESTATE_BOOT;
                }
            } else {
                if (Line == 6) {
                    UpdateSavegameNames();
                    PageNum = 5;
                } // Speichern
                if (Line == 7) {
                    UpdateSavegameNames();
                    PageNum = 6;
                } // Laden

                if (Line == 9) {
                    PageNum = 8; // New Game
                }
                if (Line == 11) {
                    KlackerTafel.Warp();
                    FrameWnd->Invalidate();
                    MessagePump();
                    FrameWnd->Invalidate();
                    MessagePump();
                    Sim.Players.Players[static_cast<SLONG>(PlayerNum)].LeaveRoom();
                }
            }

            RefreshKlackerField();
            break;

        case 2: // Grafik:
            if (Line == 2) {
                Sim.Options.OptionPlanes ^= 1;
            }
            if (Line == 3) {
                Sim.Options.OptionPassengers ^= 1;
            }
            if (Line == 4) {
                Sim.Options.OptionBlenden ^= 1;
                if (Sim.Options.OptionBlenden != 0) {
                    gBlendState = -2;
                    if (FrameWnd != nullptr) {
                        GameFrame::PrepareFade();
                    }
                }
            }
            if (Line == 5) {
                Sim.Options.OptionThinkBubbles ^= 1;
            }
            if (Line == 6) {
                Sim.Options.OptionFlipping ^= 1;
            }
            if (Line == 7) {
                Sim.Options.OptionTransparenz ^= 1;
            }
            if (Line == 8) {
                Sim.Options.OptionSchatten ^= 1;
            }

            if (Line == 10) {
                ChangedDisplay = 1;

                Sim.Options.OptionFullscreen++;
                if (Sim.Options.OptionFullscreen > 2) {
                    Sim.Options.OptionFullscreen = 0;
                }
            } // Fullscreen Option

            if (Line == 11) {
                Sim.Options.OptionKeepAspectRatio = static_cast<BOOL>(static_cast<BOOL>(Sim.Options.OptionKeepAspectRatio) == 0);
                FrameWnd->UpdateFrameSize();
            } // Aspect Ratio Option

            if (Line == 13) {
                if (ChangedDisplay != 0) {
                    FrameWnd->UpdateWindow();
                }
                PageNum = 1;
            }
            RefreshKlackerField();
            break;

        case 3: // Sound:
        {
            if (Line == 2) {
                if (++Sim.Options.OptionMusicType > 2) {
                    Sim.Options.OptionMusicType = 0;
                }
                gpMidi->SetMode(Sim.Options.OptionMusicType);
                if (Sim.Options.OptionMusicType != 0 && (Sim.Options.OptionMusik != 0)) {
                    NextMidi();
                } else {
                    StopMidi();
                }

                RefreshKlackerField();
                // KlackerTafel.Warp();
                KlackerTafel.Klack();
            }
            // else if (Line == 7)
            //{
            //	Sim.Options.OptionEnableDigi ^= 1;
            //	if (Sim.Options.OptionEnableDigi) gpSSE->EnableDS(); else gpSSE->DisableDS();
            //	RefreshKlackerField();
            //	KlackerTafel.Warp();
            //}

            const bool usesMusic = Sim.Options.OptionMusicType != 0;
            const bool usesMidi = Sim.Options.OptionMusicType == 1;
            const SLONG musicShift = usesMidi ? -1 : 0;

            if (Column >= 15 && Column < 23 && Line >= 2 && Line <= 13) {
                if (Line == 3 && !usesMidi && usesMusic) {
                    Sim.Options.OptionMusik = Column - 15;
                    SetMidiVolume(Sim.Options.OptionMusik);
                }
                if (Line == 8) {
                    Sim.Options.OptionMasterVolume = Column - 15;
                    SetWaveVolume(Sim.Options.OptionMasterVolume);
                }
                if (Line == 9) {
                    Sim.Options.OptionAmbiente = Column - 15;
                    AmbientManager.RecalcVolumes();
                    AmbientFX.SetVolume(Prozent2Dezibel(Sim.Options.OptionAmbiente * 100 / 8));
                    AmbientFX.Play(DSBPLAY_NOSTOP | DSBPLAY_LOOPING);
                }
                if (Line == 10) {
                    Sim.Options.OptionDurchsagen = Column - 15;
                    DurchsagenFX.SetVolume(Prozent2Dezibel(Sim.Options.OptionDurchsagen * 100 / 8));
                    DurchsagenFX.Play();
                }

#if !defined(NO_D_VOICES) || !defined(NO_E_VOICES) || !defined(NO_N_VOICES)
                if (!bVoicesNotFound) {
                    if (Line == 11) {
                        Sim.Options.OptionTalking = Column - 15;
                        TalkFX.SetVolume(Prozent2Dezibel(Sim.Options.OptionTalking * 100 / 8));
                        TalkFX.Play();
                    }
                }
#endif

                if (Line == 12) {
                    Sim.Options.OptionEffekte = Column - 15;
                    EffektFX.SetVolume(Prozent2Dezibel(Sim.Options.OptionEffekte * 100 / 8));
                    EffektFX.Play();
                }
                if (Line == 13) {
                    Sim.Options.OptionPlaneVolume = Column - 15;
                    PlaneFX.SetVolume(Prozent2Dezibel(Sim.Options.OptionPlaneVolume * 100 / 8));
                    PlaneFX.Play();
                }
                RefreshKlackerField();
                KlackerTafel.Warp();
            }

            if (Line == 4 + musicShift && usesMusic) {
                Sim.Options.OptionLoopMusik = (Sim.Options.OptionLoopMusik + 1) % (9);
                if (Sim.Options.OptionLoopMusik != 0) {
                    NextMidi();
                }
                RefreshKlackerField();
            }
            if (Line == 5 + musicShift && usesMusic) {
                if (Sim.Options.OptionLoopMusik == 0) {
                    NextMidi();
                }
            } // Skip
            if (Line == 15) {
                PageNum = 1;
                AmbientFX.Stop();
            } // Back
            RefreshKlackerField();
            break;
        }
        case 4: // Sonstiges:
            if (Line == 2) {
                Sim.Options.OptionGirl ^= 1;
            }
            if (Line == 3) {
                Sim.Options.OptionBerater ^= 1;
            }
            if (Line == 4) {
                Sim.Options.OptionAutosave ^= 1;
            }
            if (Line == 5) {
                Sim.Options.OptionFax ^= 1;
            }
            if (Line == 6) {
                Sim.Options.OptionRealKuerzel ^= 1;
                Cities.UseRealKuerzel(Sim.Options.OptionRealKuerzel);
            }
            if (Line == 7) {
                Sim.Options.OptionSpeechBubble ^= 1;
            }
            if (Line == 8) {
                Sim.Options.OptionBriefBriefing ^= 1;
            }
            if (Line == 9) {
                Sim.Options.OptionRandomStartday ^= 1;
            }

            if (Line == 10 && Column >= 17 && Column < 24) {
                auto speed = (Column - 17) * 5;
                if (speed <= 0) {
                    speed = 1;
                }
                Sim.GameSpeed = speed;
            }
            if (Line == 12) {
                PageNum = 1;
            }
            RefreshKlackerField();
            break;

        case 5: // Laden:
            if (Line == 15) {
                if (NewgameWantsToLoad != 0) {
                    gNetworkSavegameLoading = -1;
                    KlackerTafel.Warp();
                    FrameWnd->Invalidate();
                    MessagePump();
                    FrameWnd->Invalidate();
                    MessagePump();
                    Sim.Gamestate = GAMESTATE_BOOT;
                } else if (OptionsShortcut != 0) {
                    KlackerTafel.Warp();
                    FrameWnd->Invalidate();
                    MessagePump();
                    FrameWnd->Invalidate();
                    MessagePump();
                    Sim.Players.Players[static_cast<SLONG>(PlayerNum)].LeaveRoom();
                } else {
                    PageNum = 1;
                }
            }
            if (Line >= 2 && Line <= 13 && SavegameInfos[Line - 2].GetLength() > 0) {
                if (NewgameWantsToLoad == 2) {
                    // Netzwerk-Laden vom Hauptmenü aus:
                    gNetworkSavegameLoading = Line - 2;
                    KlackerTafel.Warp();
                    FrameWnd->Invalidate();
                    MessagePump();
                    FrameWnd->Invalidate();
                    MessagePump();
                    Sim.Gamestate = GAMESTATE_BOOT;
                } else if (Sim.bNetwork != 0) {
                    // Laden während des Spiels: Kommt das mit der aktuellen Anzahl der Spieler hin?
                    if (Sim.GetSavegameNumHumans(Line - 2) != Sim.Players.GetAnzHumanPlayers()) {
                        if (Sim.Players.Players[Sim.localPlayer].LocationWin != nullptr) {
                            (Sim.Players.Players[Sim.localPlayer].LocationWin)->MenuStart(MENU_REQUEST, MENU_REQUEST_NET_NUM);
                        }
                    } else {
                        // Laden im Netzwerk? Erst einmal bei den anderen Spielern nachfragen, ob das geht!
                        for (SLONG c = 0; c < 4; c++) {
                            Sim.Players.Players[c].bReadyForBriefing = 0;
                        }

                        nOptionsOpen++;
                        SIM::SendSimpleMessage(ATNET_OPTIONS, 0, 1, Sim.localPlayer);
                        SIM::SendSimpleMessage(ATNET_IO_LOADREQUEST, 0, Sim.localPlayer, Line - 2, Sim.GetSavegameUniqueGameId(Line - 2, true));
                    }
                } else {
                    Sim.LoadGame(Line - 2);
                    return;
                }
            }
            RefreshKlackerField();
            break;

        case 6: // Speichern:
            if (Line == 15 && Column < 10) {
                if (OptionsShortcut != 0) {
                    KlackerTafel.Warp();
                    FrameWnd->Invalidate();
                    MessagePump();
                    FrameWnd->Invalidate();
                    MessagePump();
                    Sim.Players.Players[static_cast<SLONG>(PlayerNum)].LeaveRoom();
                } else {
                    PageNum = 1;
                    CursorY = -1;
                }
            }
            if (Line >= 2 && Line <= 12 && CursorY == -1) {
                gDisablePauseKey = TRUE;
                CursorX = 0;
                CursorY = Line - 2;
            }
            if (Line == 15 && Column >= 22 && Column < 24 && CursorY != -1) {
                CursorX = 0;
                gDisablePauseKey = FALSE;

                if (Sim.bNetwork != 0) {
                    Sim.UniqueGameId = ((AtGetTime() ^ DWORD(rand() % 30000) ^ gMousePosition.x ^ gMousePosition.y) & 0x7fffffff);
                    Sim.Players.Players[static_cast<SLONG>(PlayerNum)].NetSave(Sim.UniqueGameId, CursorY, (LPCTSTR)SavegameNames[CursorY]);
                }
                Sim.SaveGame(CursorY, (LPCTSTR)SavegameNames[CursorY]);

                if ((Sim.Gamestate & 31) == GAMESTATE_INIT) {
                    Sim.Gamestate = GAMESTATE_BOOT;
                } else {
                    if ((Sim.Gamestate & 31) == GAMESTATE_INIT) {
                        Sim.Gamestate = GAMESTATE_BOOT;
                        // PostMessage(WM_CLOSE);
                    } else {
                        Sim.Players.Players[static_cast<SLONG>(PlayerNum)].LeaveRoom();
                    }
                }
            }
            RefreshKlackerField();
            break;

        case 7: // Quit:
            if (Line == 4) {
                bLeaveGameLoop = 1;
            }
            if (Line == 5) {
                PageNum = 1;
                RefreshKlackerField();
            }
            break;

        case 8: // Leave Game:
            if (Line == 4) {
                if (Sim.bNetwork != 0) {
                    SIM::SendChatBroadcast(bprintf(StandardTexte.GetS(TOKEN_MISC, 7022), (LPCTSTR)Sim.Players.Players[Sim.localPlayer].NameX));
                    gNetwork.DisConnect();
                    Sim.bNetwork = 0;
                }

                nOptionsOpen--;

                Sim.Gamestate = GAMESTATE_BOOT;
                if (NewgameToOptions != 0) {
                    KeepRoomLib();
                }
            }
            if (Line == 5) {
                PageNum = 1;
                RefreshKlackerField();
            }
            break;
        default:
            hprintf("Options.cpp: Default case should not be reached.");
            DebugBreak();
        }
    }
}

//--------------------------------------------------------------------------------------------
// void Options::OnRButtonDown(UINT nFlags, CPoint point):
//--------------------------------------------------------------------------------------------
void Options::OnRButtonDown(UINT /*nFlags*/, CPoint point) {
    DefaultOnRButtonDown();

    if (MenuIsOpen() != 0) {
        MenuRightClick(point);
    } else {
        AmbientFX.Stop();

        if (CursorY != -1) {
            gDisablePauseKey = FALSE;
            UpdateSavegameNames();
            RefreshKlackerField();
            CursorY = -1;
            return;
        }
        if (PageNum == 1) {
            if ((Sim.Gamestate & 31) == GAMESTATE_INIT) {
                KlackerTafel.Warp();
                FrameWnd->Invalidate();
                MessagePump();
                FrameWnd->Invalidate();
                MessagePump();
                Sim.Gamestate = GAMESTATE_BOOT;
            } else {
                KlackerTafel.Warp();
                FrameWnd->Invalidate();
                MessagePump();
                FrameWnd->Invalidate();
                MessagePump();
                Sim.Players.Players[static_cast<SLONG>(PlayerNum)].LeaveRoom();
            }
        } else {
            if (PageNum == 5 && (NewgameWantsToLoad != 0)) {
                KlackerTafel.Warp();
                FrameWnd->Invalidate();
                MessagePump();
                FrameWnd->Invalidate();
                MessagePump();
                Sim.Gamestate = GAMESTATE_BOOT;
            } else if ((OptionsShortcut != 0) && (PageNum == 5 || PageNum == 6)) {
                KlackerTafel.Warp();
                FrameWnd->Invalidate();
                MessagePump();
                FrameWnd->Invalidate();
                MessagePump();
                Sim.Players.Players[static_cast<SLONG>(PlayerNum)].LeaveRoom();
            } else {
                PageNum = 1;
                RefreshKlackerField();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// void Options::OnTimer(UINT nIDEvent):
//--------------------------------------------------------------------------------------------
void Options::OnTimer(UINT nIDEvent) {
    // Mit 10 FPS die Anzeige rotieren lassen:
    if (nIDEvent == 1) {
        KlackerTafel.Klack();
    }

    BlinkState++;
}

//--------------------------------------------------------------------------------------------
// void Options::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
//--------------------------------------------------------------------------------------------
void Options::OnChar(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/) {
    if (CursorY != -1) {

        nChar = KeycodeToUpper(nChar);

        if (nChar == ' ' || nChar == '-' || nChar == '+' || nChar == '.' || (nChar >= 'A' && nChar <= 'Z') || nChar == '\xC4' || nChar == '\xD6' ||
            nChar == '\xDC' || (nChar >= '0' && nChar <= '9')) {
            if ((SavenamesValid[CursorY] == 0) && strncmp(SavegameNames[CursorY], StandardTexte.GetS(TOKEN_MISC, 4073), 6) == 0) {
                SavegameNames[CursorY] = "      ";
            }

            while (SavegameNames[CursorY].GetLength() < CursorX + 1) {
                SavegameNames[CursorY] += " ";
            }
            SavegameNames[CursorY].SetAt(CursorX, UBYTE(nChar));
            RefreshKlackerField();

            if (CursorX < 19) {
                CursorX++;
            }
        }

        if (nChar == ATKEY_RETURN) {
            CursorX = 0;
            gDisablePauseKey = FALSE;

            if (Sim.bNetwork != 0) {
                Sim.UniqueGameId = ((AtGetTime() ^ DWORD(rand() % 30000) ^ gMousePosition.x ^ gMousePosition.y) & 0x7fffffff);
                Sim.Players.Players[static_cast<SLONG>(PlayerNum)].NetSave(Sim.UniqueGameId, CursorY, (LPCTSTR)SavegameNames[CursorY]);
            }
            Sim.SaveGame(CursorY, (LPCTSTR)SavegameNames[CursorY]);

            if ((Sim.Gamestate & 31) == GAMESTATE_INIT) {
                Sim.Gamestate = GAMESTATE_BOOT;
            } else {
                if ((Sim.Gamestate & 31) == GAMESTATE_INIT) {
                    Sim.Gamestate = GAMESTATE_BOOT;
                } else {
                    Sim.Players.Players[static_cast<SLONG>(PlayerNum)].LeaveRoom();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// void Options::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
//--------------------------------------------------------------------------------------------
void Options::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/) {
    if (CursorY != -1) {
        if (nChar == ATKEY_LEFT && CursorX > 0) {
            CursorX--;
        }
        if (nChar == ATKEY_RIGHT && CursorX < 19) {
            CursorX++;
        }

        if (nChar == ATKEY_BACK) {
            while (SavegameNames[CursorY].GetLength() < CursorX + 1) {
                SavegameNames[CursorY] += " ";
            }
            SavegameNames[CursorY].SetAt(CursorX, ' ');
            RefreshKlackerField();

            if (CursorX > 0) {
                CursorX--;
            }
        }

        if (nChar == ATKEY_ESCAPE) {
            gDisablePauseKey = FALSE;
            UpdateSavegameNames();
            RefreshKlackerField();
            CursorY = -1;
        }

    } else {
        if (nChar == ATKEY_F3) {
            UpdateSavegameNames();
            PageNum = 5;
            RefreshKlackerField();
        }
        if (nChar == ATKEY_F4) {
            UpdateSavegameNames();
            PageNum = 6;
            RefreshKlackerField();
        }

        if (nChar == ATKEY_ESCAPE) {
            Sim.Players.Players[Sim.localPlayer].LeaveRoom();
        }
    }
}
