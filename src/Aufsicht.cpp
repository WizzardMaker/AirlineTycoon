//============================================================================================
// Aufsicht.cpp : Das Büro der Flugaufsicht
//============================================================================================
#include "StdAfx.h"
#include "AtNet.h"
#include "Aufsicht.h"
#include "glauf.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern SB_CColorFX ColorFX;

//////////////////////////////////////////////////////////////////////////////////////////////
// Flugaufsicht Konstruktion, Initialisation, Destruction
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// Konstruktor
//--------------------------------------------------------------------------------------------
CAufsicht::CAufsicht(BOOL bHandy, ULONG PlayerNum) : CStdRaum(bHandy, PlayerNum, "aufsicht.gli", GFX_AUFSICHT) {
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;

    Sim.ShowExtrablatt = -1;
    Sim.FocusPerson = -1;
    Sim.b18Uhr = FALSE;

    bIsMorning = (Sim.Time == 9 * 60000);
    bExitASAP = false;
    bExited = false;

    if (bIsMorning) {
        NetGenericSync(0x4211014);
        PLAYER::NetSynchronizeMeeting();
        NetGenericSync(0x4211015);
    }

    KonstruktorFinished = 0;
    LeereZettelBms.ReSize(pRoomLib, "ZETTELK0", 3);
    PostcardBm.ReSize(pRoomLib, "NOCARD");

    // Morgends verhindern, daß jemand rausgeht, noch bevor alle da sind:
    bOkayToAct = TRUE;
    if (bIsMorning) {
        bOkayToAct = FALSE;
    }
    if (Sim.bNetwork == 0) {
        bOkayToAct = 1;
    }
    if (bOkayToAct == 0) {
        SetNetworkBitmap(3, 2); // Waitung for Players
    }

    Sim.Players.Players[Sim.localPlayer].bReadyForBriefing = 1;
    SIM::SendSimpleMessage(ATNET_READYFORBRIEFING, 0, Sim.localPlayer);

    // DaysWithoutSabotage aktualisieren:
    for (c = 0; c < Sim.SabotageActs.AnzEntries(); c++) {
        Sim.Players.Players[Sim.SabotageActs[c].Opfer].DaysWithoutSabotage = 0;
    }

    // Prüfen wer, welche Routen hat (wg. Spielziel):
    if (Sim.GetHour() == 9 && Sim.GetMinute() == 0 && Sim.Difficulty == DIFF_NORMAL) {
        std::array<SLONG, 6> cityIds;

        auto homeAirportId = Cities(Sim.HomeAirportId);
        for (SLONG i = 0; i < cityIds.size(); i++) {
            cityIds[i] = Cities(Sim.MissionCities[i]);
        }

        for (c = 0; c < Sim.Players.AnzPlayers; c++) {
            PLAYER &qPlayer = Sim.Players.Players[c];

            if (qPlayer.IsOut != 0) {
                continue;
            }

            qPlayer.NumMissionRoutes = 0;

            const auto &qRRouten = qPlayer.RentRouten.RentRouten;
            for (SLONG i = 0; i < qRRouten.AnzEntries(); i++) {
                const auto &rentRoute = qRRouten[i];
                if (rentRoute.Rang == 0 || rentRoute.RoutenAuslastung <= 20) {
                    continue;
                }

                ULONG a = Cities(Routen[i].VonCity);
                ULONG b = Cities(Routen[i].NachCity);

                if (a != homeAirportId) {
                    std::swap(a, b);
                    if (a != homeAirportId) {
                        continue;
                    }
                }

                for (const auto cityId : cityIds) {
                    if (b == cityId) {
                        qPlayer.NumMissionRoutes++;
                        break;
                    }
                }
            }
        }
    }

    SetRoomVisited(PlayerNum, ROOM_AUFSICHT);

    HandyOffset = 320;

    ExitFromMiddle = -1;
    ExitFromLeft = -1;
    ExitFromRight = -1;

    Talkers.Talkers[TALKER_BOSS].IncreaseReference();
    DefaultDialogPartner = TALKER_BOSS;

    for (c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
        IsOut[c] = Sim.Players.Players[c].IsOut;
        HaloKomm[c] = -1;
    }

    AirportRoomPos = Airport.GetRandomTypedRune(RUNE_2SHOP, ROOM_AUFSICHT);
    AirportRoomPos.y = AirportRoomPos.y - 5000 + 93;
    AirportRoomPos += XY(-22, -33);

    OpaqueBm.ReSize(pRoomLib, GFX_OPAQUE);
    TransBm.ReSize(pRoomLib, GFX_TRANS);

    if (bHandy == 0) {
        AmbientManager.SetGlobalVolume(30);
    }

    TimeClick = AtGetTime();

    SP_Boss.ReSize(17);

    // Raumanimationen
    MonitorAnim.ReSize(pRoomLib, "MONI01", 3, nullptr, FALSE, ANIMATION_MODE_RANDOM, 0, 3, 350);

    // Mitte:
    SP_Boss.Clips[0].ReSize(0, "bb_wait.smk", "", XY(344, 105), SPM_IDLE, CRepeat(1, 1), CPostWait(5, 5), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            &ExitFromMiddle, "A9E1E1", 0, 1, 11);
    SP_Boss.Clips[1].ReSize(1, "bb_base.smk", "", XY(344, 105), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, &ExitFromMiddle, "A9A1E1E1", 1, 3, 0, 11);
    SP_Boss.Clips[3].ReSize(3, "bb_hand.smk", "", XY(344, 105), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, nullptr, "A1", 1);

    // Für Sabotage:
    SP_Boss.Clips[2].ReSize(2, "bb_fing.smk", "", XY(344, 105), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &ExitFromMiddle,
                            SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, &ExitFromMiddle, "A1", 1);

    // Left:
    SP_Boss.Clips[4].ReSize(4, "bb_left.smk", "", XY(344, 105), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &ExitFromMiddle,
                            SMACKER_CLIP_SET | SMACKER_CLIP_POST, -1, nullptr, "A1", 5);
    SP_Boss.Clips[5].ReSize(5, "bb_leftr.smk", "", XY(344, 105), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, &ExitFromLeft, "A1E1", 5, 6);
    SP_Boss.Clips[6].ReSize(6, "bb_leftz.smk", "", XY(344, 105), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                            SMACKER_CLIP_SET, 0, nullptr, "A1", 0);

    // Right:
    SP_Boss.Clips[7].ReSize(7, "bb_righ.smk", "", XY(344, 105), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &ExitFromMiddle,
                            SMACKER_CLIP_SET | SMACKER_CLIP_POST, -1, nullptr, "A1", 8);
    SP_Boss.Clips[8].ReSize(8, "bb_righr.smk", "", XY(344, 105), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, &ExitFromRight, "A1E1", 8, 9);
    SP_Boss.Clips[9].ReSize(9, "bb_righz.smk", "", XY(344, 105), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                            SMACKER_CLIP_SET, 0, nullptr, "A1", 1);

    // Left-Right:
    SP_Boss.Clips[10].ReSize(10, "bb_turnr.smk", "", XY(344, 105), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                             SMACKER_CLIP_SET, 0, nullptr, "A1", 8);

    // Left-Right-Left (ermahnung wegen Sabotage):
    SP_Boss.Clips[12].ReSize(12, "bb_left.smk", "", XY(344, 105), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &ExitFromMiddle,
                             SMACKER_CLIP_SET | SMACKER_CLIP_POST, -1, nullptr, "A1", 13);
    SP_Boss.Clips[13].ReSize(13, "bb_turnr.smk", "", XY(344, 105), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                             0, nullptr, "A1", 14);
    SP_Boss.Clips[14].ReSize(14, "bb_turnl.smk", "", XY(344, 105), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                             0, nullptr, "A1", 15);
    SP_Boss.Clips[15].ReSize(15, "bb_turnr.smk", "", XY(344, 105), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                             0, nullptr, "A1", 16);
    SP_Boss.Clips[16].ReSize(16, "bb_righz.smk", "", XY(344, 105), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET,
                             0, nullptr, "A1", 0);

    // Listening
    SP_Boss.Clips[11].ReSize(0, "bb_wait.smk", "", XY(344, 105), SPM_LISTENING, CRepeat(1, 1), CPostWait(5, 5), SMACKER_CLIP_CANCANCEL, nullptr, 0, -1, nullptr,
                             "A9E1E1", 11, 1, 0);

    if (Sim.GetHour() == 9 && Sim.GetMinute() == 0) {
        PlayerStuff.ReSize(pRoomLib, "OBJ_PB OBJ_PV OBJ_PR OBJ_PJ");
        FrauFuss.ReSize(pRoomLib, "FRAUFUSS");

        if ((Sim.bNetwork != 0) && Sim.Date == 0) {
            for (c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
                Sim.Players.Players[c].bReadyForMorning = 0;
            }
        }

        if (IsOut[0] == 0) // Blau:
        {
            SP_Player[0].ReSize(9);
            SP_Player[0].Clips[0].ReSize(0, "pb_wait.smk", "", XY(70, 135), SPM_IDLE, CRepeat(1, 1), CPostWait(5, 5), SMACKER_CLIP_CANCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A6A1A1E1", 0, 1, 4, 1);
            SP_Player[0].Clips[8].ReSize(8, "pb_guckw.smk", "", XY(70, 135), SPM_HOLY, CRepeat(1, 1), CPostWait(9999, 9999), SMACKER_CLIP_CANCANCEL,
                                         &(HaloKomm[0]), SMACKER_CLIP_SET | SMACKER_CLIP_PRE, 1, nullptr, "A1E1", 8, 3);

            SP_Player[0].Clips[1].ReSize(1, "pb_guck.smk", "", XY(70, 135), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 2);
            SP_Player[0].Clips[2].ReSize(2, "pb_guckw.smk", "", XY(70, 135), SPM_IDLE, CRepeat(1, 1), CPostWait(30, 60), SMACKER_CLIP_CANCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A7A1E1", 3, 7, 8);
            SP_Player[0].Clips[3].ReSize(3, "pb_guckz.smk", "", XY(70, 135), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);

            SP_Player[0].Clips[4].ReSize(4, "pb_fing.smk", "", XY(70, 135), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 5);
            SP_Player[0].Clips[5].ReSize(5, "pb_fingw.smk", "", XY(70, 135), SPM_IDLE, CRepeat(1, 1), CPostWait(20, 80), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 6);
            SP_Player[0].Clips[6].ReSize(6, "pb_fingz.smk", "", XY(70, 135), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);
            SP_Player[0].Clips[7].ReSize(7, "pb_guckk.smk", "", XY(70, 135), SPM_IDLE, CRepeat(1, 1), CPostWait(1, 1), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0, 2);

            SP_Halo[0].ReSize(2);
            SP_Halo[0].Clips[0].ReSize(0, "bombew.smk", "", XY(94, 119), SPM_IDLE, CRepeat(1, 1), CPostWait(50, 50), SMACKER_CLIP_CANCANCEL, nullptr,
                                       SMACKER_CLIP_SET, 0, &(HaloKomm[0]), "A1", 0);
            SP_Halo[0].Clips[1].ReSize(1, "p1halo.smk", "", XY(94, 119), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, &(HaloKomm[0]),
                                       SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, nullptr, "A1", 0);
        }

        if (IsOut[1] == 0) // Grün:
        {
            SP_Player[1].ReSize(7);
            SP_Player[1].Clips[0].ReSize(0, "pv_wait.smk", "", XY(165, 110), SPM_IDLE, CRepeat(1, 1), CPostWait(15, 45), SMACKER_CLIP_CANCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A8A1A2A2E1", 0, 5, 1, 2, 6);
            SP_Player[1].Clips[6].ReSize(6, "pv_wait.smk", "", XY(165, 110), SPM_HOLY, CRepeat(1, 1), CPostWait(9999, 9999), SMACKER_CLIP_CANCANCEL,
                                         &(HaloKomm[1]), SMACKER_CLIP_SET | SMACKER_CLIP_PRE, 1, nullptr, "A1E1", 6, 0);

            SP_Player[1].Clips[1].ReSize(1, "pv_hand.smk", "", XY(165, 110), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);
            SP_Player[1].Clips[2].ReSize(2, "pv_handb.smk", "", XY(165, 110), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 3);
            SP_Player[1].Clips[3].ReSize(3, "pv_handw.smk", "", XY(165, 110), SPM_IDLE, CRepeat(1, 1), CPostWait(40, 90), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 4);
            SP_Player[1].Clips[4].ReSize(4, "pv_handz.smk", "", XY(165, 110), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);
            SP_Player[1].Clips[5].ReSize(5, "pv_waitk.smk", "", XY(165, 110), SPM_IDLE, CRepeat(1, 1), CPostWait(1, 1), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);

            SP_Halo[1].ReSize(2);
            SP_Halo[1].Clips[0].ReSize(0, "bombew.smk", "", XY(165, 93), SPM_IDLE, CRepeat(1, 1), CPostWait(50, 50), SMACKER_CLIP_CANCANCEL, nullptr,
                                       SMACKER_CLIP_SET, 0, &(HaloKomm[1]), "A1", 0);
            SP_Halo[1].Clips[1].ReSize(1, "p2halo.smk", "", XY(165, 93), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, &(HaloKomm[1]),
                                       SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, nullptr, "A1", 0);
        }

        if (IsOut[2] == 0) // Rot:
        {
            SP_Player[2].ReSize(6);
            SP_Player[2].Clips[0].ReSize(0, "pr_wait.smk", "", XY(422, 142), SPM_IDLE, CRepeat(1, 1), CPostWait(5, 5), SMACKER_CLIP_CANCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A2A1A1E1", 0, 1, 4, 5);
            SP_Player[2].Clips[5].ReSize(5, "pr_wait.smk", "", XY(422, 142), SPM_HOLY, CRepeat(1, 1), CPostWait(9999, 9999), SMACKER_CLIP_CANCANCEL,
                                         &(HaloKomm[2]), SMACKER_CLIP_SET | SMACKER_CLIP_PRE, 1, nullptr, "A1E1", 5, 0);

            SP_Player[2].Clips[1].ReSize(1, "pr_auf.smk", "", XY(422, 142), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 2);
            SP_Player[2].Clips[2].ReSize(2, "pr_aufw.smk", "", XY(422, 142), SPM_IDLE, CRepeat(1, 1), CPostWait(40, 120), SMACKER_CLIP_CANCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 3);
            SP_Player[2].Clips[3].ReSize(3, "pr_aufz.smk", "", XY(422, 142), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);
            SP_Player[2].Clips[4].ReSize(4, "pr_uhr.smk", "", XY(422, 142), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);

            SP_Halo[2].ReSize(2);
            SP_Halo[2].Clips[0].ReSize(0, "bombew.smk", "", XY(528, 130), SPM_IDLE, CRepeat(1, 1), CPostWait(50, 50), SMACKER_CLIP_CANCANCEL, nullptr,
                                       SMACKER_CLIP_SET, 0, &(HaloKomm[2]), "A1", 0);
            SP_Halo[2].Clips[1].ReSize(1, "p3halo.smk", "", XY(528, 130), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, &(HaloKomm[2]),
                                       SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, nullptr, "A1", 0);
        }

        if (IsOut[3] == 0) // Gelb:
        {
            SP_Player[3].ReSize(6);
            SP_Player[3].Clips[0].ReSize(0, "pj_wait.smk", "", XY(446, 186), SPM_IDLE, CRepeat(2, 2), CPostWait(15, 20), SMACKER_CLIP_CANCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A9A1A1A1A1E1", 0, 4, 1, 2, 3, 5);
            SP_Player[3].Clips[5].ReSize(5, "pj_wait.smk", "", XY(446, 186), SPM_HOLY, CRepeat(2, 2), CPostWait(9999, 9999), SMACKER_CLIP_CANCANCEL,
                                         &(HaloKomm[3]), SMACKER_CLIP_SET | SMACKER_CLIP_PRE, 1, nullptr, "A1E1", 5, 0);

            SP_Player[3].Clips[1].ReSize(1, "pj_arm.smk", "", XY(446, 186), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);
            SP_Player[3].Clips[2].ReSize(0, "pj_fing.smk", "", XY(446, 186), SPM_IDLE, CRepeat(1, 1), CPostWait(5, 5), SMACKER_CLIP_CANCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);
            SP_Player[3].Clips[3].ReSize(1, "pj_schu.smk", "", XY(446, 186), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);
            SP_Player[3].Clips[4].ReSize(1, "pj_k.smk", "", XY(446, 186), SPM_IDLE, CRepeat(1, 1), CPostWait(1, 1), SMACKER_CLIP_DONTCANCEL, nullptr,
                                         SMACKER_CLIP_SET, 0, nullptr, "A1", 0);

            SP_Halo[3].ReSize(2);
            SP_Halo[3].Clips[0].ReSize(0, "bombew.smk", "", XY(583, 164), SPM_IDLE, CRepeat(1, 1), CPostWait(50, 50), SMACKER_CLIP_CANCANCEL, nullptr,
                                       SMACKER_CLIP_SET, 0, &(HaloKomm[3]), "A1", 0);
            SP_Halo[3].Clips[1].ReSize(1, "p4halo.smk", "", XY(583, 164), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, &(HaloKomm[3]),
                                       SMACKER_CLIP_SET | SMACKER_CLIP_PRE, -1, nullptr, "A1", 0);
        }
    }

    // Das Briefing machen wir später & erlösen die Figuren sofort:
    if (Sim.DayState == 1 && (Sim.IsTutorial == 0)) {
#ifdef DEMO
        if (Sim.Date >= 100)
            StartDialog(TALKER_BOSS, MEDIUM_AIR, 30);
#endif

        auto &qPlayer = Sim.Players.Players[Sim.localPlayer];
        if ((CheatTestGame != 0 || CheatAutoSkip != 0) && qPlayer.Money < 500000) {
            qPlayer.Money = 1000000;
            // log: hprintf ("Event: localPlayer gets Money-Boost for testing reasons");
        }

        // Uhrig's Aufträge:
        if (Sim.Difficulty == DIFF_ADDON09) {
            for (SLONG c = 0; c < 4; c++) {
                PLAYER &qPlayer = Sim.Players.Players[c];

                if (qPlayer.IsOut == 0) {
                    if (Sim.Date == 0) {
                        qPlayer.NumOrderFlightsToday = 2 + (qPlayer.PlayerNum & 1);
                    } else {
                        qPlayer.NumOrderFlightsToday = 0;
                    }

                    qPlayer.NumOrderFlightsToday2 = qPlayer.NumOrderFlightsToday;
                    qPlayer.Statistiken[STAT_AUFTRAEGE].AddAtPastDay(5);

                    if (qPlayer.Owner != 1 || !qPlayer.RobotUse(ROBOT_UHRIG_FLIGHTS_AUTO)) {
                        qPlayer.Add5UhrigFlights();
                    }
                }
            }
        }

        if (Sim.Date == 0 || Sim.Options.OptionBriefBriefing == 0) {
            StartDialog(TALKER_BOSS, MEDIUM_AIR, 1);
            DontDisplayPlayer = Sim.localPlayer;
        } else {
            MenuStart(MENU_BRIEFING);
        }
    }

    if (Sim.GetHour() == 9 && Sim.GetMinute() == 0 && MouseWait > 0) {
        MouseWait--;
    }

    KonstruktorFinished = 1;
}

//--------------------------------------------------------------------------------------------
// Destruktor
//--------------------------------------------------------------------------------------------
CAufsicht::~CAufsicht() {
    SLONG c = 0;

    LeereZettelBms.Destroy();
    PostcardBm.Destroy();

    if ((Sim.bNetwork != 0) && bIsMorning) {
        PLAYER &qLocalPlayer = Sim.Players.Players[Sim.localPlayer];

        PLAYER::NetSynchronizeMoney();
        PLAYER::NetSynchronizeImage();
        PLAYER::NetSynchronizeRoutes();
        qLocalPlayer.NetUpdateWorkers();
    }

    Sim.Players.Players[Sim.localPlayer].Messages.KillBerater();

    if (CheatTestGame != 0) {
        Sim.Players.Players[PlayerNum].GameSpeed = 3;
    }

    Talkers.Talkers[TALKER_BOSS].DecreaseReference();

    if (bIsMorning) {
        Sim.SabotageActs.ReSize(0);

        for (c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
            if (Sim.Players.Players[c].IsOut == 0) {
                PERSON &qPerson = Sim.Persons[Sim.Persons.GetPlayerIndex(c)];

                if (Sim.Date != 0) {
                    qPerson.Position = Airport.GetRandomTypedRune(RUNE_PCREATION2, static_cast<UBYTE>((c + 1) * 10));
                } else {
                    qPerson.Position = Airport.GetRandomTypedRune(RUNE_PCREATION2, static_cast<UBYTE>((c + 1 + 4) * 10));
                }

                qPerson.MoodCountdown = 0;
                qPerson.ScreenPos.x = qPerson.Position.x - (qPerson.Position.y - 5000) / 2 + 86;
                qPerson.ScreenPos.y = (qPerson.Position.y - 5000) + 93;

                if (c == 0 || c == 1) {
                    qPerson.LookDir = 3;
                } else {
                    qPerson.LookDir = 1;
                }

                qPerson.Dir = qPerson.LookDir * 2;

                qPerson.StatePar = 0;
            }
        }

        bool bAnyBombs = false;
        for (c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
            if (Sim.Players.Players[c].IsOut == 0) {
                PLAYER &qPlayer = Sim.Players.Players[c];

                if (c != Sim.localPlayer && (Sim.bNetwork == 0 || (Sim.bIsHost != 0))) {
                    qPlayer.LeaveRoom();
                }

                qPlayer.NumFlights = 0;
                qPlayer.WaitWorkTill = -1;
                qPlayer.WorkCountdown = 1;
                qPlayer.WaitWorkTill = 0;

                if (qPlayer.Owner == 1) {
                    qPlayer.WalkToRoom(UBYTE(ROOM_BURO_A + c * 10));
                }

                bool bFremdsabotage = false;
                if (Sim.Players.Players[c].ArabMode2 < 0) {
                    Sim.Players.Players[c].ArabMode2 = -Sim.Players.Players[c].ArabMode2;
                    bFremdsabotage = true;
                }

                if (qPlayer.ArabMode2 != 0) {
                    PLAYER &qOpfer = Sim.Players.Players[qPlayer.ArabOpfer2];

                    if (!bFremdsabotage) {
                        Sim.Players.Players[c].Statistiken[STAT_SABOTIERT].AddAtPastDay(1);
                    }

                    switch (qPlayer.ArabMode2) {
                    case 1: // Bakterien im Kaffee
                        if (!bFremdsabotage) {
                            qPlayer.ArabHints += 8;
                        }
                        qOpfer.Sympathie[c] -= 10;
                        qOpfer.SickTokay = TRUE;
                        PLAYER::NetSynchronizeFlags();
                        break;

                    case 2: // Virus im Notepad
                        if ((qOpfer.HasItem(ITEM_LAPTOP) != 0) && qOpfer.LaptopVirus == 0) {
                            qOpfer.LaptopVirus = 1;
                        }
                        break;

                    case 3: // Bombe im Büro
                        bAnyBombs = true;
                        if (!bFremdsabotage) {
                            qPlayer.ArabHints += 25;
                        }
                        qOpfer.Sympathie[c] -= 50;
                        qOpfer.OfficeState = 1;
                        qOpfer.WalkToRoom(UBYTE(ROOM_BURO_A + qOpfer.PlayerNum * 10));
                        break;

                    case 4: // Streik provozieren
                        if (!bFremdsabotage) {
                            qPlayer.ArabHints += 40;
                        }
                        qOpfer.StrikePlanned = TRUE;
                        break;
                    default:
                        hprintf("Aufsicht.cpp: Default case should not be reached.");
                        DebugBreak();
                    }

                    // Für's nächste Briefing vermerken:
                    Sim.SabotageActs.ReSize(Sim.SabotageActs.AnzEntries() + 1);
                    Sim.SabotageActs[Sim.SabotageActs.AnzEntries() - 1].Player = bFremdsabotage ? -2 : c;
                    Sim.SabotageActs[Sim.SabotageActs.AnzEntries() - 1].ArabMode = 2075 + qPlayer.ArabMode2 - 1;
                    Sim.SabotageActs[Sim.SabotageActs.AnzEntries() - 1].Opfer = qPlayer.ArabOpfer2;

                    qPlayer.ArabMode2 = 0;
                }

                bFremdsabotage = false;
                if (Sim.Players.Players[c].ArabMode3 < 0) {
                    Sim.Players.Players[c].ArabMode3 = -Sim.Players.Players[c].ArabMode3;
                    bFremdsabotage = true;
                }

                if (qPlayer.ArabMode3 != 0) {
                    PLAYER &qOpfer = Sim.Players.Players[qPlayer.ArabOpfer3];

                    Sim.Players.Players[c].Statistiken[STAT_SABOTIERT].AddAtPastDay(1);

                    switch (qPlayer.ArabMode3) {
                    case 1: // Fremde Broschüren
                        if (!bFremdsabotage) {
                            qPlayer.ArabHints += 8;
                        }
                        qOpfer.WerbeBroschuere = qPlayer.PlayerNum;
                        PLAYER::NetSynchronizeFlags();
                        break;

                    case 2: // Telefone sperren
                        if (!bFremdsabotage) {
                            qPlayer.ArabHints += 15;
                        }
                        qOpfer.TelephoneDown = 1;
                        PLAYER::NetSynchronizeFlags();
                        break;

                    case 3: // Presseerklärung
                        if (!bFremdsabotage) {
                            qPlayer.ArabHints += 25;
                        }
                        qOpfer.Presseerklaerung = 1;
                        PLAYER::NetSynchronizeFlags();
                        Sim.Players.Players[Sim.localPlayer].Letters.AddLetter(
                            FALSE, bprintf(StandardTexte.GetS(TOKEN_LETTER, 509), (LPCTSTR)qOpfer.AirlineX, (LPCTSTR)qOpfer.NameX, (LPCTSTR)qOpfer.AirlineX),
                            "", "", 0);
                        if (qOpfer.PlayerNum == Sim.localPlayer) {
                            qOpfer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2020));
                        }

                        {
                            // Für alle Flugzeuge die er besitzt, die Passagierzahl aktualisieren:
                            for (SLONG d = 0; d < qOpfer.Planes.AnzEntries(); d++) {
                                if (qOpfer.Planes.IsInAlbum(d) != 0) {
                                    CPlane &qPlane = qOpfer.Planes[d];

                                    for (SLONG e = 0; e < qPlane.Flugplan.Flug.AnzEntries(); e++) {
                                        if (qPlane.Flugplan.Flug[e].ObjectType == 1) {
                                            qPlane.Flugplan.Flug[e].CalcPassengers(qOpfer.PlayerNum, qPlane);
                                        }
                                    }
                                    // qPlane.Flugplan.Flug[e].CalcPassengers (qPlane.TypeId, (LPCTSTR)qOpfer.PlayerNum, (LPCTSTR)qPlane);
                                }
                            }
                        }
                        break;

                    case 4: // Bankkonto hacken
                        qOpfer.ChangeMoney(-1000000, 3502, "");
                        if (!bFremdsabotage) {
                            qPlayer.ChangeMoney(1000000, 3502, "");
                        }
                        if (!bFremdsabotage) {
                            qPlayer.ArabHints += 30;
                        }
                        break;

                    case 5: // Flugzeug festsetzen
                        if (!bFremdsabotage) {
                            qPlayer.ArabHints += 50;
                        }
                        if (qOpfer.Planes.IsInAlbum(qPlayer.ArabPlane) != 0) {
                            qOpfer.Planes[qPlayer.ArabPlane].PseudoProblem = 15;
                        }
                        break;

                    case 6: // Route klauen
                        qPlayer.ArabHints += 70;
                        qOpfer.RouteWegnehmen(Routen(qPlayer.ArabPlane), qPlayer.PlayerNum);
                        {
                            for (SLONG d = 0; d < Routen.AnzEntries(); d++) {
                                if ((Routen.IsInAlbum(d) != 0) && Routen[d].VonCity == Routen[qPlayer.ArabPlane].NachCity &&
                                    Routen[d].NachCity == Routen[qPlayer.ArabPlane].VonCity) {
                                    qOpfer.RouteWegnehmen(d, qPlayer.PlayerNum);
                                    break;
                                }
                            }
                        }
                        if (qOpfer.PlayerNum == Sim.localPlayer) {
                            qOpfer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2021));
                        }
                        break;
                    default:
                        hprintf("Aufsicht.cpp: Default case should not be reached.");
                        DebugBreak();
                    }

                    // Für's nächste Briefing vermerken:
                    Sim.SabotageActs.ReSize(Sim.SabotageActs.AnzEntries() + 1);
                    Sim.SabotageActs[Sim.SabotageActs.AnzEntries() - 1].Player = bFremdsabotage ? -2 : c;
                    Sim.SabotageActs[Sim.SabotageActs.AnzEntries() - 1].ArabMode = 2090 + qPlayer.ArabMode3;
                    Sim.SabotageActs[Sim.SabotageActs.AnzEntries() - 1].Opfer = qPlayer.ArabOpfer3;

                    qPlayer.ArabMode3 = 0;
                }
            }
        }

        if (!bAnyBombs) {
            for (c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
                if (Sim.Players.Players[c].IsOut == 0) {
                    Sim.Players.Players[c].WalkToRoom(UBYTE(ROOM_BURO_A + c * 10));
                }
            }
        }
    }

    if (Sim.Players.Players[Sim.localPlayer].OfficeState != 1) {
        for (c = 0; c < 4; c++) {
            if (Sim.Players.Players[c].OfficeState == 1 && Sim.Players.Players[c].IsOut == 0) {
                Sim.Players.Players[Sim.localPlayer].WalkStop();
                Sim.Players.Players[Sim.localPlayer].WalkStopEx();
                Sim.FocusPerson = Sim.Persons.GetIdFromIndex(Sim.Persons.GetPlayerIndex(c));
            }
        }
    }

    // Flugzeuge von allen relevanten Spielern aktualisieren:
    for (c = 0; c < 4; c++) {
        if (Sim.Players.Players[c].IsOut == 0) {
            PLAYER &qPlayer = Sim.Players.Players[c];

            // Für alle Flugzeuge die er besitzt
            for (SLONG d = 0; d < qPlayer.Planes.AnzEntries(); d++) {
                if (qPlayer.Planes.IsInAlbum(d) != 0) {
                    CPlane &qPlane = qPlayer.Planes[d];

                    if (qPlane.Ort > 0) {
                        qPlane.Position = Cities[qPlane.Ort].GlobusPosition;
                        qPlane.UpdateGlobePos(Sim.Players.Players[Sim.localPlayer].EarthAlpha);
                    }
                }
            }
        }
    }

    if (Sim.Difficulty == DIFF_ATFS10) {
        PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];

        if (Sim.Date == 3) {
            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2400));
        }
        if (Sim.Date == 18) {
            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2401));
        }
        if (Sim.Date == 20) {
            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2402));
        }
        if (Sim.Date == 25) {
            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2403));
        }
        if (Sim.Date == 35) {
            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2404));
        }
        if (Sim.Date == 40) {
            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2405));
        }
        if (Sim.Date == 45) {
            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2406));
        }
        if (Sim.Date == 55) {
            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2407));
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CAufsicht message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// void CAufsicht::OnPaint()
//--------------------------------------------------------------------------------------------
void CAufsicht::OnPaint() {
    PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];
    UBYTE Painted = 0;

    if (bExitASAP && Sim.Time > 9 * 60000 && Sim.bWatchForReady == 0) {
        if (!bExited) {
            Sim.Players.Players[PlayerNum].LeaveRoom();
            bExited = true;
        }
    }

    if (Sim.Date > 5 && Sim.GetHour() > 9) {
        Sim.GiveHint(HINT_AUFSICHT);
    }

    if (KonstruktorFinished == 0) {
        return;
    }

    if (Sim.bNetwork == 0) {
        bOkayToAct = 1;
    }

    if (bOkayToAct == 0) {
        bOkayToAct = TRUE;

        for (SLONG c = 0; c < 4; c++) {
            if (!static_cast<bool>(Sim.Players.Players[c].bReadyForBriefing) && Sim.Players.Players[c].Owner == 2 && (Sim.Players.Players[c].IsOut == 0)) {
                bOkayToAct = FALSE;
            }
        }

        if (bOkayToAct != 0) {
            SetNetworkBitmap(0);
        }
    }

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    if (CurrentMenu != MENU_BRIEFING) {
        CStdRaum::OnPaint();
    }

    if (!((Sim.ItemPostcard != 0) && qPlayer.SeligTrust == 0 && Sim.Difficulty != DIFF_TUTORIAL)) {
        RoomBm.BlitFromT(PostcardBm, 260, 106);
    }

    if (CurrentMenu != MENU_BRIEFING || (gMouseLButton != 0) || (gMouseRButton != 0)) {
    do_the_painting_again:
        Painted++;
        for (SLONG c = 0; c < TafelData.ByPositions.size(); c++) {
            RoomBm.BlitFromT(LeereZettelBms[c % 3], (TafelData.ByPositions[c]->Position.x - 91) * 74 / 441 + 274,
                             (TafelData.ByPositions[c]->Position.y - 20) * 68 / 366 + 55);
        }

        // Draw Persons:
        RoomBm.pBitmap->SetClipRect(CRect(422, 0, 640, 174));
        for (SLONG d = 0; d < SLONG(Sim.Persons.AnzEntries()); d++) {
            // Entscheidung! Person malen:
            if ((Sim.Persons.IsInAlbum(d) != 0) && (Clans.IsInAlbum(Sim.Persons[d].ClanId) != 0)) {
                PERSON &qPerson = Sim.Persons[d];
                CLAN &qClan = Clans[static_cast<SLONG>(qPerson.ClanId)];
                UBYTE Dir = qPerson.LookDir;
                UBYTE Phase = qPerson.Phase;

                // if (Dir==1 || Dir==3) Dir = 4-Dir;
                if (Dir < 4) {
                    Dir = UBYTE((Dir + 1) & 3);
                }
                if (Dir == 8 && Phase < 4) {
                    Phase = UBYTE((Phase + 1) % 4);
                }
                if (Dir == 8 && Phase >= 4) {
                    Phase = UBYTE((Phase + 1) % 4 + 4);
                }

                XY p = qPerson.ScreenPos - AirportRoomPos;
                XY pp;

                if (abs(p.y) < 150) {
                    pp.x = 640 - p.y * 4 - p.x * 2;
                    pp.y = 220 + p.x;

                    if (pp.x > 380 && pp.x < 700 && abs(pp.y - 220) < 40) {
                        qClan.BlitLargeAt(RoomBm, Dir, Phase, pp);
                    }
                }
            } else {
                break;
            }
        }
        RoomBm.pBitmap->SetClipRect(CRect(0, 0, 640, 440));

        // DrawTransparency:
        ColorFX.BlitTrans(TransBm.pBitmap, RoomBm.pBitmap, XY(422, 0), nullptr, 2);
        RoomBm.BlitFromT(OpaqueBm, 422, 0);

        SP_Boss.Pump();
        SP_Boss.BlitAtT(RoomBm);

        // Draw Smacker Persons:
        if (Sim.GetHour() == 9 && Sim.GetMinute() == 0) {
            if (IsOut[1] == 0) {
                RoomBm.BlitFromT(PlayerStuff[1], 244 - 20, 192 + 9);
            }
            if (IsOut[0] == 0) {
                RoomBm.BlitFrom(FrauFuss, 138, 373);
                RoomBm.BlitFromT(PlayerStuff[0], 72, 245);
            }
        }

        // Die Raum-Animationen:
        MonitorAnim.BlitAtT(RoomBm, 329, 188);

        // Draw Smacker Persons:
        if (Sim.GetHour() == 9 && Sim.GetMinute() == 0) {
            if (IsOut[2] == 0) {
                RoomBm.BlitFromT(PlayerStuff[2], 403, 233);
            }
            if (IsOut[3] == 0) {
                RoomBm.BlitFromT(PlayerStuff[3], 242, 249);
            }

            if (IsOut[1] == 0) {
                SP_Player[1].Pump();
                SP_Player[1].BlitAtT(RoomBm);
                SP_Halo[1].Pump();
                SP_Halo[1].BlitAtT(RoomBm);
            }

            for (SLONG c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
                if ((IsOut[c] == 0) && c != 1) {
                    SP_Player[c].Pump();
                    SP_Player[c].BlitAtT(RoomBm);
                    SP_Halo[c].Pump();
                    SP_Halo[c].BlitAtT(RoomBm);
                }
            }
        }

        if (Painted == 4) {
            CStdRaum::PumpToolTips();
            return;
        }
    }

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips();

    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && Sim.bNoTime == FALSE) {
        if (gMousePosition.IfIsWithin(64, 18, 172, 240)) {
            SetMouseLook(CURSOR_EXIT, 0, ROOM_AUFSICHT, 999);
        } else if (gMousePosition.IfIsWithin(269, 41, 362, 137)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_AUFSICHT, 10);
        } else if (gMousePosition.IfIsWithin(400, 100, 505, 241)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_AUFSICHT, 11);
        }
    } else if (Sim.GetHour() == 9 && Sim.GetMinute() == 0) {
        if ((AtGetTime() - TimeClick > 40000 && Sim.Options.OptionTalking * Sim.Options.OptionDigiSound == 0) ||
            (((bTest != 0) || (CheatTestGame != 0)) && AtGetTime() - TimeClick > 5000)) {
            if ((bTest != 0) || (CheatTestGame != 0)) {
                OnRButtonDown(0, CPoint(160, 100));
            } else {
                CStdRaum::PreLButtonDown(XY(160, 100));
            }

            if (bTest != 0) {
                TimeClick = AtGetTime() - 39000;
            } else {
                TimeClick = AtGetTime() - 30000;
            }
        }
    }

    if (Sim.bPause != 0) {
        TimeClick = AtGetTime();
    }

    CStdRaum::PostPaint();

    if (CurrentMenu != MENU_BRIEFING && Painted == 0) {
        Painted = 3;
        goto do_the_painting_again;
    }

    CStdRaum::PumpToolTips();

    if (Sim.Date == gAutoQuitOnDay) {
        exit(0);
    }
    if (CheatAutoSkip != 0 && (gQuickTestRun > 0 || (Sim.Date % 100) != 99)) {
        OnRButtonDown(0, CPoint());
        qPlayer.CallItADay = TRUE;
    }
}

//--------------------------------------------------------------------------------------------
// void CAufsicht::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CAufsicht::OnLButtonDown(UINT nFlags, CPoint point) {
    XY RoomPos;

    if (bOkayToAct == 0) {
        return;
    }

    DefaultOnLButtonDown();

    TimeClick = AtGetTime();

    if (CurrentMenu == MENU_GAMEOVER && AtGetTime() - TimeAtStart < 3000) {
        return;
    }

    if (ConvertMousePosition(point, &RoomPos) == 0) {
        CStdRaum::OnLButtonDown(nFlags, point);
        return;
    }

    if (PreLButtonDown(point) == 0) {
        if (MouseClickArea == ROOM_AUFSICHT && MouseClickId == 999) {
            Sim.bNoTime = FALSE;
            Sim.DayState = 2;
            // Sim.Players.Players[(SLONG)PlayerNum].LeaveRoom();
            TryLeaveAufsicht();
        } else if (MouseClickArea == ROOM_AUFSICHT && MouseClickId == 10) {
            Sim.Players.Players[PlayerNum].EnterRoom(ROOM_TAFEL);
        } else if (MouseClickArea == ROOM_AUFSICHT && MouseClickId == 11) {
            StartDialog(TALKER_BOSS, MEDIUM_AIR, 2);
        } else {
            CStdRaum::OnLButtonDown(nFlags, point);
        }
    }
}

//--------------------------------------------------------------------------------------------
// void CAufsicht::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CAufsicht::OnRButtonDown(UINT nFlags, CPoint point) {
    BOOL CanCancel = TRUE;
    SLONG c = 0;

    if (bOkayToAct == 0) {
        return;
    }

    DefaultOnRButtonDown();

    if ((MenuIsOpen() != 0) && CurrentMenu == MENU_BRIEFING) {
        MenuStop();

        StartDialog(TALKER_BOSS, MEDIUM_AIR, 1);
        DontDisplayPlayer = Sim.localPlayer;
    }

    if (Sim.GetHour() == 9 && Sim.GetMinute() == 0) {
        for (c = 0; c < Sim.Players.AnzPlayers; c++) {
            if (Sim.Players.Players[c].IsOut == 0) {
                if (Sim.Players.Players[c].Image < -990 || Sim.Players.Players[c].Money < DEBT_GAMEOVER) {
                    CanCancel = FALSE;
                }
                if (Sim.Players.Players[c].HasWon() != 0) {
                    // Add-On Mission #3 Dauert immer 30 Tage:
                    if ((!(Sim.Difficulty == DIFF_ADDON03 && Sim.Date < TARGET_DAYS)) && (!(Sim.Difficulty == DIFF_ADDON04 && Sim.Date < TARGET_MILESDAYS)) &&
                        (!(Sim.Difficulty == DIFF_ADDON06 && Sim.Date < TARGET_VALUEDAYS)) && (!(Sim.Difficulty == DIFF_ATFS09 && Sim.Date < BTARGET_NDAYS9)) &&
                        (!(Sim.Difficulty == DIFF_ATFS10 && Sim.Date < BTARGET_NDAYS10))) {
                        CanCancel = FALSE;
                    }
                }

#ifdef DEMO
                if (Sim.Date >= 30)
                    CanCancel = FALSE;
#endif
            }
        }

        for (c = 0; c < Sim.SabotageActs.AnzEntries(); c++) {
            if (Sim.SabotageActs[c].Player != -1) {
                if (Sim.Players.Players[Sim.SabotageActs[c].Player].ArabHints >= 100) {
                    CanCancel = FALSE;
                }
            }
        }

        if (Sim.Overtake != 0) {
            CanCancel = FALSE;
        }
    }

    // Außerhalb geklickt? Dann Default-Handler!
    if (point.x < WinP1.x || point.y < WinP1.y || point.x > WinP2.x || point.y > WinP2.y) {
        return;
    }

    if (MenuIsOpen() != 0) {
        if (CanCancel != 0) {
            MenuRightClick(point);
        }
    } else {
        if ((IsDialogOpen() == 0) && (Sim.IsTutorial == FALSE || Sim.Tutorial > 1310)) {
            if (point.y < 440) {
                Sim.bNoTime = FALSE;
                Sim.DayState = 2;
                TryLeaveAufsicht();
            }
        }

        if ((CanCancel != 0) && (Sim.IsTutorial == FALSE || Sim.Tutorial > 1310)) {
            if (Sim.bNoTime != 0) {
                Sim.bNoTime = FALSE;
                Sim.DayState = 2;
                TryLeaveAufsicht();
            }
        }

        if (!((IsDialogOpen() != 0) && (Sim.IsTutorial != 0)) && (CanCancel != 0)) {
            CStdRaum::OnRButtonDown(nFlags, point);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Raum verlassen:
//--------------------------------------------------------------------------------------------
void CAufsicht::TryLeaveAufsicht() {
    if ((Sim.bNetwork != 0) && bIsMorning) {
        bExitASAP = true;
        Sim.bWatchForReady = TRUE;
        SIM::SendSimpleMessage(ATNET_READYFORMORNING, 0, Sim.localPlayer);
        Sim.Players.Players[Sim.localPlayer].bReadyForMorning = 1;
        SetNetworkBitmap(3, 1);
        FrameWnd->Invalidate();
        MessagePump();
    } else {
        Sim.Players.Players[Sim.localPlayer].LeaveRoom();
    }
}
