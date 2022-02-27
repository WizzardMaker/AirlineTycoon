//============================================================================================
// Sabotage.cpp : Der Raum für die Sabotage
//============================================================================================
// Link: "Sabotage.h"
//============================================================================================
#include "StdAfx.h"
#include "glsabo.h"
#include "Sabotage.h"
#include "AtNet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Zum debuggen:
static const char FileId[] = "Sabo";

//--------------------------------------------------------------------------------------------
//Der Sabotageraum
//--------------------------------------------------------------------------------------------
CSabotage::CSabotage(BOOL bHandy, ULONG PlayerNum) : CStdRaum (bHandy, PlayerNum, "sabotage.gli", GFX_SABOTAGE)
{
    SetRoomVisited (PlayerNum, ROOM_SABOTAGE);
    HandyOffset = 320;

    if (bHandy == 0) { AmbientManager.SetGlobalVolume (40);
}
    PlayEyeAnim = FALSE;

    Sim.FocusPerson=-1;

    //Hintergrundsounds:
    if (Sim.Options.OptionEffekte != 0)
    {
        BackFx.ReInit("saboback.raw");
        BackFx.Play(DSBPLAY_NOSTOP|DSBPLAY_LOOPING, Sim.Options.OptionEffekte*100/7);
    }

    SP_Araber.ReSize (12);
    SP_Araber.Clips[0].ReSize (0, "s_clic.smk", "", XY (413, 255), SPM_IDLE,       CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,  //Warten
            "A9A4E1", 0, 10, 1);
    SP_Araber.Clips[10].ReSize (10, "s_clicb.smk", "", XY (413, 255), SPM_IDLE,    CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,  //Warten
            "A9E1", 0, 1);

    SP_Araber.Clips[1].ReSize (1, "s_turn.smk", "s_turn.raw", XY (275, 101), SPM_IDLE,       CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_DONTCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,  //Dreht sich um
            "E1E1", 2, 3);
    SP_Araber.Clips[2].ReSize (2, "s_rede.smk", "", XY (155, 102), SPM_TALKING,    CRepeat(8,8), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,
            "A9E1E1", 2, 3, 6);
    SP_Araber.Clips[3].ReSize (3, "s_wait.smk", "", XY (155, 102), SPM_LISTENING,  CRepeat(2,2), CPostWait(20,20), SMACKER_CLIP_CANCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,
            "A9A1A1E1E1", 3, 4, 5, 2, 6);
    SP_Araber.Clips[4].ReSize (4, "s_left.smk", "", XY (155, 102), SPM_LISTENING,  CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_DONTCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,
            "A9E1E1", 3, 2, 6);
    SP_Araber.Clips[5].ReSize (5, "s_rigt.smk", "", XY (155, 102), SPM_LISTENING,  CRepeat(1,1), CPostWait(0,0),    SMACKER_CLIP_DONTCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,
            "A9E1E1", 3, 2, 6);
    SP_Araber.Clips[6].ReSize (6, "s_raus.smk", "", XY (155, 102), SPM_IDLE,       CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_DONTCANCEL,
            &PlayEyeAnim, SMACKER_CLIP_SET|SMACKER_CLIP_PRE, 1, nullptr,
            "A9E1E1", 7, 2, 3);
    SP_Araber.Clips[7].ReSize (3, "s_wait.smk", "", XY (155, 102), SPM_IDLE,       CRepeat(1,1), CPostWait(20,20), SMACKER_CLIP_CANCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,
            "A9A1A1A1A1E1E1", 7, 11, 6, 8, 9, 2, 3);
    SP_Araber.Clips[11].ReSize (11, "s_waitk.smk", "", XY (155, 102), SPM_IDLE,    CRepeat(1,1), CPostWait(1,1),   SMACKER_CLIP_CANCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,  //Blinzeln
            "A9", 7);

    SP_Araber.Clips[8].ReSize (4, "s_left.smk", "", XY (155, 102), SPM_IDLE,       CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_DONTCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,
            "A9E1E1", 7, 2, 3);
    SP_Araber.Clips[9].ReSize (5, "s_rigt.smk", "", XY (155, 102), SPM_IDLE,       CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_DONTCANCEL,
            nullptr, SMACKER_CLIP_SET, 0, nullptr,
            "A9E1E1", 7, 2, 3);

    AraberBm.ReSize (pRoomLib, "S_BASE16");
    DartBm.ReSize (pRoomLib, "OHNEDART");
    ZangeBm.ReSize (pRoomLib, "PINCERS");

    //Raumanimationen
    ZischFx.ReInit ("zisch.raw");
    LunteFx.ReInit ("lunte.raw");

    if (rand()%10==0 || (CheatAnimNow != 0)) {
        DynamitAnim.ReSize (pRoomLib, "FEUER00", 3, &LunteFx,  FALSE, ANIMATION_MODE_REPEAT, 0,   2);
}

    DampfAnim.ReSize   (pRoomLib, "DAMPF00",  8, &ZischFx, FALSE, ANIMATION_MODE_REPEAT, 300, 3, 300, 1);
    KamelAnim.ReSize   (pRoomLib, "AUGE00",   7, nullptr,     TRUE,  ANIMATION_MODE_NEVER,  0  , 7, 300, 1);
    LampeAnim.ReSize   (pRoomLib, "RAUCH00", 11, nullptr,     TRUE,  ANIMATION_MODE_REPEAT, 0,   5);

    Talkers.Talkers[TALKER_SABOTAGE].IncreaseReference ();
    DefaultDialogPartner=TALKER_SABOTAGE;

    SDL_ShowWindow(FrameWnd->m_hWnd);
    SDL_UpdateWindowSurface(FrameWnd->m_hWnd);

#ifdef DEMO
    MenuStart (MENU_REQUEST, MENU_REQUEST_NO_SABOTAGE);
    MenuSetZoomStuff (XY(320,220), 0.17, FALSE);
#endif

    PlayUniversalFx ("morse.raw", Sim.Options.OptionEffekte);
}

//--------------------------------------------------------------------------------------------
//Sehr destruktiv!
//--------------------------------------------------------------------------------------------
CSabotage::~CSabotage()
{
    AraberBm.Destroy();
    DartBm.Destroy();
    ZangeBm.Destroy();
    Talkers.Talkers[TALKER_SABOTAGE].DecreaseReference ();

    if (Sim.Players.Players[Sim.localPlayer].ArabMode==6) {
        Sim.Players.Players[Sim.localPlayer].EnterRoom (ROOM_WORLD);
}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CSabotage message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
//void CSabotage::OnPaint()
//--------------------------------------------------------------------------------------------
void CSabotage::OnPaint()
{
    SLONG   NewTip = 0;
    PLAYER &qPlayer = Sim.Players.Players[(SLONG)PlayerNum];

    if (bHandy == 0) { SetMouseLook (CURSOR_NORMAL, 0, ROOM_SABOTAGE, 0);
}

    //Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint ();

    DampfAnim.BlitAt   (RoomBm, 423, 0);
    DynamitAnim.BlitAt (RoomBm, 287, 386);
    KamelAnim.BlitAt   (RoomBm,  20, 37);
    LampeAnim.BlitAt   (RoomBm, 344, 299);

    if (!((qPlayer.HasItem (ITEM_DART) == 0) && (qPlayer.HasItem (ITEM_DISKETTE) == 0) && qPlayer.WerbungTrust==FALSE)) {
        RoomBm.BlitFrom (DartBm, 148, 42);
}

    if (Sim.ItemZange != 0) { RoomBm.BlitFrom (ZangeBm, 369, 188);
}

    if (SP_Araber.GetClip()!=0 && SP_Araber.GetClip()!=1 && SP_Araber.GetClip()!=-1 && SP_Araber.GetClip()!=10) {
        RoomBm.BlitFrom (AraberBm, 380-3, 194);
}

    SP_Araber.Pump ();
    SP_Araber.BlitAtT (RoomBm);

    if (PlayEyeAnim != 0)
    {
        PlayEyeAnim=FALSE;
        KamelAnim.StartNow ();
    }

    //Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips ();

    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0))
    {
        if (gMousePosition.IfIsWithin (86, 98, 209, 439)) { SetMouseLook (CURSOR_EXIT, 0, ROOM_SABOTAGE, 999);
        } else if (gMousePosition.IfIsWithin (138,9,211,120) && (qPlayer.HasItem (ITEM_DART) == 0) && (qPlayer.HasItem (ITEM_DISKETTE) == 0) && qPlayer.WerbungTrust==FALSE) {
            SetMouseLook (CURSOR_HOT, 0, ROOM_SABOTAGE, 800);
        } else
        {
            if (SP_Araber.GetClip()!=0 && SP_Araber.GetClip()!=1 && SP_Araber.GetClip()!=-1)
            {
                if (gMousePosition.IfIsWithin (376,106,546,440)) { SetMouseLook (CURSOR_HOT, 0, ROOM_SABOTAGE, 10);
}
                if (gMousePosition.IfIsWithin (288,227,631,323)) { SetMouseLook (CURSOR_HOT, 0, ROOM_SABOTAGE, 10);
}
            }
            else
            {
                if (gMousePosition.IfIsWithin (422,265,548,439)) { SetMouseLook (CURSOR_HOT, 0, ROOM_SABOTAGE, 10);
}
            }
        }
    }
    else
    {
        if (CurrentMenu==MENU_SABOTAGEPLANE && (gMousePosition-MenuPos).IfIsWithin (216,6, 387,212))
        {
            NewTip = (gMousePosition.y-(MenuPos.y+25))/13 + MenuPage;

            if (NewTip>=0 && NewTip-MenuPage<13 && NewTip<MenuDataTable.LineIndex.AnzEntries() && (Sim.Players.Players[(SLONG)qPlayer.ArabOpfer].Planes.IsInAlbum (MenuDataTable.LineIndex[NewTip]) != 0))
            {
                if (NewTip != CurrentTip)
                {
                    MenuRepaint ();
                    //DrawPlaneTipContents (OnscreenBitmap, &PlaneTypes[Sim.Players.Players[(SLONG)qPlayer.ArabOpfer].Planes[MenuDataTable.LineIndex[NewTip]].TypeId], &Sim.Players.Players[(SLONG)qPlayer.ArabOpfer].Planes[MenuDataTable.LineIndex[NewTip]],
                    DrawPlaneTipContents (OnscreenBitmap, nullptr, &Sim.Players.Players[(SLONG)qPlayer.ArabOpfer].Planes[MenuDataTable.LineIndex[NewTip]],
                            XY(6,6), XY(6,28), &FontSmallBlack, &FontSmallBlack, FALSE, TRUE);
                }

                if (MenuDataTable.ValueFlags[0+NewTip*MenuDataTable.AnzColums] != 0u) {
                    CheckCursorHighlight (ReferenceCursorPos, CRect (MenuPos.x+216, MenuPos.y+(NewTip-MenuPage)*13+25-2, MenuPos.x+387, MenuPos.y+(NewTip-MenuPage)*13+25+12), ColorOfFontRed, CURSOR_HOT);
                } else {
                    CheckCursorHighlight (ReferenceCursorPos, CRect (MenuPos.x+216, MenuPos.y+(NewTip-MenuPage)*13+25-2, MenuPos.x+387, MenuPos.y+(NewTip-MenuPage)*13+25+12), ColorOfFontBlack, CURSOR_HOT);
}

                CurrentTip = NewTip;
            }
            else { NewTip = -1;
}
        }
        else { NewTip = -1;
}

        if (NewTip != CurrentTip)
        {
            MenuRepaint ();
            CurrentTip = NewTip;
        }
    }

#ifdef DEMO
    if (!IsDialogOpen() && !MenuIsOpen()) SetMouseLook (CURSOR_EXIT, 0, ROOM_SABOTAGE, 999);
#endif

    CStdRaum::PostPaint ();
    CStdRaum::PumpToolTips ();
}

//--------------------------------------------------------------------------------------------
//void CSabotage::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CSabotage::OnLButtonDown(UINT nFlags, CPoint point)
{
    XY RoomPos;

    DefaultOnLButtonDown ();

    if (ConvertMousePosition (point, &RoomPos) == 0)
    {
        CStdRaum::OnLButtonDown(nFlags, point);
        return;
    }

    if (PreLButtonDown (point) == 0)
    {
        if (MouseClickArea==ROOM_SABOTAGE && MouseClickId==999) { Sim.Players.Players[(SLONG)PlayerNum].LeaveRoom();
        } else if (MouseClickArea==ROOM_SABOTAGE && MouseClickId==800)
        {
            StartDialog (TALKER_SABOTAGE, MEDIUM_AIR, 800);
        }
        else if (MouseClickArea==ROOM_SABOTAGE && MouseClickId==10) { StartDialog (TALKER_SABOTAGE, MEDIUM_AIR, 0);
        } else if (gMousePosition.IfIsWithin (298,395, 337,423)) { DynamitAnim.Remove();
        } else if (gMousePosition.IfIsWithin (384,204, 426,263))
        {
            if (Sim.ItemZange != 0)
            {
                Sim.Players.Players[(SLONG)PlayerNum].BuyItem (ITEM_ZANGE);
                if (Sim.Players.Players[(SLONG)PlayerNum].HasItem (ITEM_ZANGE) != 0)
                {
                    Sim.ItemZange=0;
                    SIM::SendSimpleMessage (ATNET_TAKETHING, 0, ITEM_ZANGE);
                }
            }
        }
        else { CStdRaum::OnLButtonDown(nFlags, point);
}
    }
}

//--------------------------------------------------------------------------------------------
//void CSabotage::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CSabotage::OnRButtonDown(UINT nFlags, CPoint point)
{
    DefaultOnRButtonDown ();

    //Außerhalb geklickt? Dann Default-Handler!
    if (point.x<WinP1.x || point.y<WinP1.y || point.x>WinP2.x || point.y>WinP2.y)
    {
        return;
    }
    
            if (MenuIsOpen())
        {
            MenuRightClick (point);
        }
        else
        {
            if (!IsDialogOpen() && point.y<440)
            {
                Sim.Players.Players[(SLONG)PlayerNum].LeaveRoom();
            }
            CStdRaum::OnRButtonDown(nFlags, point);
        }
   
}

//--------------------------------------------------------------------------------------------
//Speichert ein CSabotageAct-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator << (TEAKFILE &File, const CSabotageAct &SabotageAct)
{
    File << SabotageAct.Player << SabotageAct.ArabMode << SabotageAct.Opfer;

    return (File);
}

//--------------------------------------------------------------------------------------------
//Lädt ein CSabotageAct-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator >> (TEAKFILE &File, CSabotageAct &SabotageAct)
{
    File >> SabotageAct.Player >> SabotageAct.ArabMode >> SabotageAct.Opfer;

    return (File);
}
