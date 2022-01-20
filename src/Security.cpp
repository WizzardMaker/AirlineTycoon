//============================================================================================
// Security.cpp : Der Security-Raum
//============================================================================================
#include "StdAfx.h"
#include "AtNet.h"
#include "Security.h"
#include "glsecurity.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DSBVOLUME_MIN (-10000)
#define DSBVOLUME_MAX 0

// Zum debuggen:
static const char FileId[] = "Secu";

//--------------------------------------------------------------------------------------------
// Die Schalter wird erˆffnet:
//--------------------------------------------------------------------------------------------
CSecurity::CSecurity(BOOL bHandy, ULONG PlayerNum) : CStdRaum(bHandy, PlayerNum, "security.gli", GFX_SECURITY) {
    SetRoomVisited(PlayerNum, ROOM_SECURITY);
    HandyOffset = 320;
    Sim.FocusPerson = -1;

    if (bHandy == 0) {
        AmbientManager.SetGlobalVolume(60);
    }

    Talkers.Talkers[TALKER_SECURITY].IncreaseReference();
    DefaultDialogPartner = TALKER_SECURITY;

    KommVar = -1;

    SP_Secman.ReSize(15);
    SP_Secman.Clips[0].ReSize(0, "so_normal.smk", "", XY(240, 79), SPM_IDLE, CRepeat(2, 3), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                              0, nullptr, // Warten
                              "A1A9A1A1E1", 0, 14, 7, 10, 1);
    SP_Secman.Clips[1].ReSize(1, "so_zum_reden.smk", "", XY(240, 79), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Warten->Speak
                              "E1E1E1", 2, 4, 6);
    SP_Secman.Clips[2].ReSize(2, "so_reden_mund_zu.smk", "", XY(240, 79), SPM_LISTENING, CRepeat(1, 1), CPostWait(50, 50), SMACKER_CLIP_CANCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Listen
                              "A9E5E5", 2, 4, 5);
    SP_Secman.Clips[3].ReSize(3, "arabp.smk", "", XY(240, 79), SPM_TALKING, CRepeat(1, 1), CPostWait(20, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                              0, nullptr, // Psst!, danach Speaking
                              "A9", 4);
    SP_Secman.Clips[4].ReSize(4, "so_reden.smk", "", XY(240, 79), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Speak
                              "A9E1E1", 4, 2, 5);
    SP_Secman.Clips[5].ReSize(5, "so_reden_zurueck.smk", "", XY(240, 79), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Speak->Wait
                              "A9", 0);

    SP_Secman.Clips[6].ReSize(6, "so_erschrecken.smk", "", XY(240, 79), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Speak->Wait
                              "A9", 0);
    SP_Secman.Clips[7].ReSize(7, "so_hindrehen.smk", "", XY(240, 79), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Speak->Wait
                              "A2A1E1", 8, 9, 9);
    SP_Secman.Clips[8].ReSize(8, "so_druecken.smk", "", XY(256, 63), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Speak->Wait
                              "A9A9E1", 8, 9, 9);
    SP_Secman.Clips[9].ReSize(9, "so_herdrehen.smk", "", XY(240, 79), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Speak->Wait
                              "A1", 0);

    SP_Secman.Clips[10].ReSize(10, "so_tasse_nehmen.smk", "", XY(240, 79), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                               SMACKER_CLIP_SET, 0, nullptr, // Speak->Wait
                               "A1A1E1", 11, 12, 13);
    SP_Secman.Clips[11].ReSize(11, "so_tasse_trinken.smk", "", XY(240, 79), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                               SMACKER_CLIP_SET, 0, nullptr, // Speak->Wait
                               "A1A1E1", 12, 13, 13);
    SP_Secman.Clips[12].ReSize(12, "so_tasse_zittern.smk", "", XY(240, 79), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                               SMACKER_CLIP_SET, 0, nullptr, // Speak->Wait
                               "A1A1A1E1", 11, 12, 13, 13);
    SP_Secman.Clips[13].ReSize(13, "so_tasse_hinsetzen.smk", "", XY(240, 79), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                               SMACKER_CLIP_SET, 0, nullptr, // Speak->Wait
                               "A1", 0);

    SP_Secman.Clips[14].ReSize(14, "so_normal.smk", "", XY(240, 79), SPM_IDLE, CRepeat(1, 1), CPostWait(120, 120), SMACKER_CLIP_CANCANCEL, &KommVar,
                               SMACKER_CLIP_SET | SMACKER_CLIP_PRE, 1, nullptr, // Speak->Wait
                               "A1E1", 6, 1);

    SP_Tuer.ReSize(1);
    SP_Tuer.Clips[0].ReSize(0, "so_tuer.smk", "", XY(38, 145), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 0);

    SP_Tresor.ReSize(2);
    SP_Tresor.Clips[0].ReSize(0, "so_tresor_pause.smk", "", XY(528, 198), SPM_IDLE, CRepeat(1, 1), CPostWait(20, 20), SMACKER_CLIP_CANCANCEL, nullptr,
                              SMACKER_CLIP_SET, 0, nullptr, // Warten
                              "A9A2", 0, 1);
    SP_Tresor.Clips[1].ReSize(1, "so_tresor.smk", "", XY(528, 198), SPM_IDLE, CRepeat(1, 3), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                              0, nullptr, // Warten
                              "A9A3", 1, 0);

    SP_Spion.ReSize(2);
    SP_Spion.Clips[0].ReSize(0, "so_spion_pause.smk", "", XY(85, 327), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr,
                             SMACKER_CLIP_SET, 0, &KommVar, // Warten
                             "A9A0", 0, 1);
    SP_Spion.Clips[1].ReSize(1, "so_spion.smk", "", XY(85, 327), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, &KommVar,
                             SMACKER_CLIP_SET | SMACKER_CLIP_PRE, 0, nullptr, // Warten
                             "A9", 0);

    SP_Gasmaske.ReSize(2);
    SP_Gasmaske.Clips[0].ReSize(0, "so_gasmask_pause.smk", "", XY(468, 89), SPM_IDLE, CRepeat(1, 1), CPostWait(80, 80), SMACKER_CLIP_CANCANCEL, nullptr,
                                SMACKER_CLIP_SET, 0, nullptr, // Warten
                                "A9A4", 0, 1);
    SP_Gasmaske.Clips[1].ReSize(1, "so_gasmask.smk", "", XY(468, 89), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr,
                                SMACKER_CLIP_SET, 0, nullptr, // Warten
                                "A9", 0);

    SP_Cam1.ReSize(14);
    SP_Cam1.Clips[0].ReSize(0, "kama12.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 1, 2);
    SP_Cam1.Clips[1].ReSize(1, "kama2.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 2);
    SP_Cam1.Clips[2].ReSize(2, "kama23.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 3, 4);
    SP_Cam1.Clips[3].ReSize(3, "kama3.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 4);
    SP_Cam1.Clips[4].ReSize(4, "kama34.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 5, 6);
    SP_Cam1.Clips[5].ReSize(5, "kama4.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 6);
    SP_Cam1.Clips[6].ReSize(6, "kama45.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 7, 8);
    SP_Cam1.Clips[7].ReSize(7, "kama5.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 8);
    SP_Cam1.Clips[8].ReSize(8, "kama56.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 9, 10);
    SP_Cam1.Clips[9].ReSize(9, "kama6.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 10);
    SP_Cam1.Clips[10].ReSize(10, "kama67.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                             nullptr, // Warten
                             "A9A9", 11, 12);
    SP_Cam1.Clips[11].ReSize(11, "kama7.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                             nullptr, // Warten
                             "A9", 12);
    SP_Cam1.Clips[12].ReSize(12, "kama71.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                             nullptr, // Warten
                             "A9A9", 13, 0);
    SP_Cam1.Clips[13].ReSize(13, "kama1.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                             nullptr, // Warten
                             "A9", 0);

    SP_Cam2.ReSize(14);
    SP_Cam2.Clips[0].ReSize(0, "kamb12.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 1, 2);
    SP_Cam2.Clips[1].ReSize(1, "kamb2.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 2);
    SP_Cam2.Clips[2].ReSize(2, "kamb23.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 3, 4);
    SP_Cam2.Clips[3].ReSize(3, "kamb3.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 4);
    SP_Cam2.Clips[4].ReSize(4, "kamb34.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 5, 6);
    SP_Cam2.Clips[5].ReSize(5, "kamb4.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 6);
    SP_Cam2.Clips[6].ReSize(6, "kamb45.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 7, 8);
    SP_Cam2.Clips[7].ReSize(7, "kamb5.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 8);
    SP_Cam2.Clips[8].ReSize(8, "kamb56.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 9, 10);
    SP_Cam2.Clips[9].ReSize(9, "kamb6.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 10);
    SP_Cam2.Clips[10].ReSize(10, "kamb67.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                             nullptr, // Warten
                             "A9A9", 11, 12);
    SP_Cam2.Clips[11].ReSize(11, "kamb7.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                             nullptr, // Warten
                             "A9", 12);
    SP_Cam2.Clips[12].ReSize(12, "kamb71.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                             nullptr, // Warten
                             "A9A9", 13, 0);
    SP_Cam2.Clips[13].ReSize(13, "kamb1.smk", "", XY(125, 85), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                             nullptr, // Warten
                             "A9", 0);

    SP_Cam3.ReSize(6);
    SP_Cam3.Clips[0].ReSize(0, "kamc12.smk", "", XY(362, 83), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 1, 2);
    SP_Cam3.Clips[1].ReSize(1, "kamc2.smk", "", XY(362, 83), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 2);
    SP_Cam3.Clips[2].ReSize(2, "kamc23.smk", "", XY(362, 83), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 3, 4);
    SP_Cam3.Clips[3].ReSize(3, "kamc3.smk", "", XY(362, 83), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 4);
    SP_Cam3.Clips[4].ReSize(4, "kamc31.smk", "", XY(362, 83), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 5, 0);
    SP_Cam3.Clips[5].ReSize(5, "kamc1.smk", "", XY(362, 83), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 0);

    SP_Cam4.ReSize(6);
    SP_Cam4.Clips[0].ReSize(0, "kamd12.smk", "", XY(348, 277), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 1, 2);
    SP_Cam4.Clips[1].ReSize(1, "kamd2.smk", "", XY(348, 277), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 2);
    SP_Cam4.Clips[2].ReSize(2, "kamd23.smk", "", XY(348, 277), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 3, 4);
    SP_Cam4.Clips[3].ReSize(3, "kamd3.smk", "", XY(348, 277), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 4);
    SP_Cam4.Clips[4].ReSize(4, "kamd31.smk", "", XY(348, 277), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 5, 0);
    SP_Cam4.Clips[5].ReSize(5, "kamd1.smk", "", XY(348, 277), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 0);

    SP_Cam5.ReSize(6);
    SP_Cam5.Clips[0].ReSize(0, "kame12.smk", "", XY(509, 47), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 1, 2);
    SP_Cam5.Clips[1].ReSize(1, "kame2.smk", "", XY(509, 47), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 2);
    SP_Cam5.Clips[2].ReSize(2, "kame23.smk", "", XY(509, 47), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 3, 4);
    SP_Cam5.Clips[3].ReSize(3, "kame3.smk", "", XY(509, 47), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 4);
    SP_Cam5.Clips[4].ReSize(4, "kame31.smk", "", XY(509, 47), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9A9", 5, 0);
    SP_Cam5.Clips[5].ReSize(5, "kame1.smk", "", XY(509, 47), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 20), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 0);

    /*SP_Cam1.ReSize (1);
      SP_Cam1.Clips[0].ReSize (0, "so_kam_hili.smk", "", XY (125, 85), SPM_IDLE,  CRepeat(1,1), CPostWait(0,20),   SMACKER_CLIP_CANCANCEL,
      NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
      "A9", 0);

      SP_Cam3.ReSize (1);
      SP_Cam3.Clips[0].ReSize (0, "so_kam_hire.smk", "", XY (362, 83), SPM_IDLE,  CRepeat(1,1), CPostWait(0,20),   SMACKER_CLIP_CANCANCEL,
      NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
      "A9", 0);
      SP_Cam4.ReSize (1);
      SP_Cam4.Clips[0].ReSize (0, "so_kam_pult.smk", "", XY (348, 277), SPM_IDLE,  CRepeat(1,1), CPostWait(0,20),   SMACKER_CLIP_CANCANCEL,
      NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
      "A9", 0);
      SP_Cam5.ReSize (1);
      SP_Cam5.Clips[0].ReSize (0, "so_kam_vore.smk", "", XY (509, 47), SPM_IDLE,  CRepeat(1,1), CPostWait(0,20),   SMACKER_CLIP_CANCANCEL,
      NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
      "A9", 0);*/

    SDL_ShowWindow(FrameWnd->m_hWnd);
    SDL_UpdateWindowSurface(FrameWnd->m_hWnd);

    // Hintergrundsounds:
    if (Sim.Options.OptionEffekte != 0) {
        BackFx.ReInit("secure.raw");
        BackFx.Play(DSBPLAY_NOSTOP | DSBPLAY_LOOPING, Sim.Options.OptionEffekte * 100 / 7);
    }
}

//--------------------------------------------------------------------------------------------
// Sehr destruktiv!
//--------------------------------------------------------------------------------------------
CSecurity::~CSecurity() {
    BackFx.SetVolume(DSBVOLUME_MIN);
    BackFx.Stop();
    Talkers.Talkers[TALKER_SECURITY].DecreaseReference();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CSecurity message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// void CSecurity::OnPaint()
//--------------------------------------------------------------------------------------------
void CSecurity::OnPaint() {
    if (bHandy == 0) {
        SetMouseLook(CURSOR_NORMAL, 0, ROOM_SECURITY, 0);
    }

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    SP_Tuer.Pump();
    SP_Gasmaske.Pump();
    SP_Cam1.Pump();
    SP_Cam3.Pump();
    SP_Cam4.Pump();
    SP_Cam5.Pump();
    SP_Spion.Pump();
    SP_Tresor.Pump();
    SP_Secman.Pump();

    SP_Tuer.BlitAtT(RoomBm);
    SP_Gasmaske.BlitAtT(RoomBm);
    SP_Cam1.BlitAtT(RoomBm);
    SP_Cam3.BlitAtT(RoomBm);
    SP_Cam4.BlitAtT(RoomBm);
    SP_Cam5.BlitAtT(RoomBm);
    SP_Spion.BlitAtT(RoomBm);
    SP_Tresor.BlitAtT(RoomBm);
    SP_Secman.BlitAtT(RoomBm);

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips();

    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
        if (gMousePosition.IfIsWithin(4, 70, 112, 412)) {
            SetMouseLook(CURSOR_EXIT, 0, ROOM_SECURITY, 999);
        } else if (gMousePosition.IfIsWithin(288, 144, 401, 267)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_SECURITY, 10);
        }
    }

    CStdRaum::PostPaint();
    CStdRaum::PumpToolTips();
}

//--------------------------------------------------------------------------------------------
// void CSecurity::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CSecurity::OnLButtonDown(UINT nFlags, CPoint point) {
    XY RoomPos;

    DefaultOnLButtonDown();

    if (ConvertMousePosition(point, &RoomPos) == 0) {
        CStdRaum::OnLButtonDown(nFlags, point);
        return;
    }

    if (PreLButtonDown(point) == 0) {
        if (MouseClickArea == ROOM_SECURITY && MouseClickId == 999) {
            Sim.Players.Players[PlayerNum].LeaveRoom();
        } else if (MouseClickArea == ROOM_SECURITY && MouseClickId == 10) {
            StartDialog(TALKER_SECURITY, MEDIUM_AIR, 1000);
        } else if (MouseClickArea == ROOM_SECURITY && MouseClickId == 12) {
            Sim.Players.Players[PlayerNum].BuyItem(ITEM_GLOVE);

            if (Sim.Players.Players[PlayerNum].HasItem(ITEM_GLOVE) != 0) {
                Sim.ItemGlove = 0;
                SIM::SendSimpleMessage(ATNET_TAKETHING, 0, ITEM_GLOVE);
            }
        } else {
            CStdRaum::OnLButtonDown(nFlags, point);
        }
    }
}

//--------------------------------------------------------------------------------------------
// void CSecurity::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CSecurity::OnRButtonDown(UINT nFlags, CPoint point) {
    DefaultOnRButtonDown();

    // Auﬂerhalb geklickt? Dann Default-Handler!
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
