//============================================================================================
// Ricks.cpp : Der Schalter und das Hinterzimmer von Ricks!
//============================================================================================
// Link: "Ricks.h"
//============================================================================================
#include "StdAfx.h"
#include "Ricks.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Zum debuggen:
static const char FileId[] = "Arab";

#define GFX_RICK			 (0x000000004b434952)

//--------------------------------------------------------------------------------------------
//Die Schalter wird erˆffnet:
//--------------------------------------------------------------------------------------------
CRicks::CRicks(BOOL bHandy, ULONG PlayerNum) : CStdRaum (bHandy, PlayerNum, "rick.gli", GFX_RICK)
{
    SetRoomVisited (PlayerNum, ROOM_RICKS);
    HandyOffset = 320;

    Sim.InvalidateHint(HINT_RICK);
    Sim.FocusPerson=-1;

    if (bHandy == 0) { AmbientManager.SetGlobalVolume (50);
}

    Talkers.Talkers[TALKER_RICK].IncreaseReference ();
    DefaultDialogPartner=TALKER_RICK;

    SP_Rick.ReSize (7);
    SP_Rick.Clips[0].ReSize (0, "bar_wait.smk", "", XY (157, 130), SPM_IDLE,       CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
            "A9A1E1", 0, 1, 2);
    SP_Rick.Clips[1].ReSize (1, "bar_aug.smk", "", XY (157, 130), SPM_IDLE,       CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_DONTCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
            "A9A1E1", 0, 1, 2);
    SP_Rick.Clips[2].ReSize (2, "bar_rede.smk", "", XY (157, 130), SPM_TALKING,    CRepeat(1,1), CPostWait(0,0), SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Speak
            "A9A2A2E1E1", 2, 3, 4, 5, 0);
    SP_Rick.Clips[3].ReSize (3, "bar_fing.smk", "", XY (157, 130), SPM_TALKING,    CRepeat(1,1), CPostWait(0,0), SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Speak
            "A9A2E1E1", 2, 4, 5, 0);
    SP_Rick.Clips[4].ReSize (4, "bar_arm.smk", "",  XY (157, 130), SPM_TALKING,    CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Speak
            "A9A2E1E1", 2, 3, 5, 0);
    SP_Rick.Clips[5].ReSize (5, "bar_wait.smk", "", XY (157, 130), SPM_LISTENING,  CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Listen
            "A9A2E1E1", 5, 6, 0, 2);
    SP_Rick.Clips[6].ReSize (6, "bar_aug.smk", "", XY (157, 130), SPM_LISTENING,  CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_DONTCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Listen
            "A9A2E1E1", 5, 6, 0, 2);

    SP_Neon.ReSize (1);
    SP_Neon.Clips[0].ReSize (0, "neon.smk", "", XY (514,0),     SPM_IDLE,       CRepeat(99,99), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
            "A9E1", 0, 1);

    SP_TrinkerAugen.ReSize (2);
    SP_TrinkerAugen.Clips[0].ReSize (0, "augw.smk", "", XY (479,199),  SPM_IDLE, CRepeat(1,1), CPostWait(10,90), SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
            "A9A9", 0, 1);
    SP_TrinkerAugen.Clips[1].ReSize (1, "aug.smk",  "", XY (479,199),  SPM_IDLE, CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
            "A9A9", 0, 1);

    SP_TrinkerMund.ReSize (7);
    SP_TrinkerMund.Clips[0].ReSize (0, "mund_w.smk", "", XY (483,230),  SPM_IDLE, CRepeat(1,1), CPostWait(10,90), SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
            "A9E1E1", 0, 5, 6);
    SP_TrinkerMund.Clips[1].ReSize (1, "mund.smk",  "", XY (483,230),  SPM_IDLE, CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
            "A9E1E1", 2, 5, 6);
    SP_TrinkerMund.Clips[2].ReSize (2, "munda.smk", "", XY (483,230),  SPM_IDLE, CRepeat(1,1), CPostWait(0,30), SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
            "A9", 3);
    SP_TrinkerMund.Clips[3].ReSize (3, "mundaw.smk",  "", XY (483,230),  SPM_IDLE, CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
            "A9", 4);
    SP_TrinkerMund.Clips[4].ReSize (4, "mundz.smk", "", XY (483,230),  SPM_IDLE, CRepeat(1,1), CPostWait(10,90), SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Warten
            "A9", 0);
    SP_TrinkerMund.Clips[5].ReSize (5, "mund.smk",  "", XY (483,230),  SPM_TALKING, CRepeat(1,1), CPostWait(0,0),   SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Speak
            "A9E1E1", 5,6,0);
    SP_TrinkerMund.Clips[6].ReSize (6, "mund_w.smk", "", XY (483,230),  SPM_LISTENING, CRepeat(1,1), CPostWait(10,90), SMACKER_CLIP_CANCANCEL,
            NULL, SMACKER_CLIP_SET, 0, NULL,  //Listen
            "A9E1E1", 6,5,0);

    //Raumanimationen
    if (Sim.Options.OptionEffekte != 0)
    {
        SetBackgroundFx (0, "kaffee.raw",   40000, 27000);
        SetBackgroundFx (2, "toilet.raw",  120000, 80000, 40);
    }

    SDL_ShowWindow(FrameWnd->m_hWnd);
    SDL_UpdateWindowSurface(FrameWnd->m_hWnd);
}

//--------------------------------------------------------------------------------------------
//Sehr destruktiv!
//--------------------------------------------------------------------------------------------
CRicks::~CRicks()
{
    Talkers.Talkers[TALKER_RICK].DecreaseReference ();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CRicks message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
//void CRicks::OnPaint()
//--------------------------------------------------------------------------------------------
void CRicks::OnPaint()
{
    if (bHandy == 0) { SetMouseLook (CURSOR_NORMAL, 0, ROOM_RICKS, 0);
}

    if (Sim.Date>4) { Sim.GiveHint (HINT_RICKS);
}

    //Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint ();

    SP_Neon.Pump ();
    SP_Neon.BlitAtT (RoomBm);
    SP_Rick.Pump ();
    SP_Rick.BlitAtT (RoomBm);
    SP_TrinkerMund.Pump ();
    SP_TrinkerMund.BlitAtT (RoomBm);
    SP_TrinkerAugen.Pump ();
    SP_TrinkerAugen.BlitAtT (RoomBm);

    //Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips ();

    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0))
    {
        if (gMousePosition.IfIsWithin (0, 0, 70, 439) || gMousePosition.IfIsWithin (570, 0, 640, 225)) { SetMouseLook (CURSOR_EXIT, 0, ROOM_RICKS, 999);
        } else if (gMousePosition.IfIsWithin (159,131,302,346)) { SetMouseLook (CURSOR_HOT, 0, ROOM_RICKS, 10);
        } else if (gMousePosition.IfIsWithin (466,141,639,439)) { SetMouseLook (CURSOR_HOT, 0, ROOM_RICKS, 11);
}
    }

    CStdRaum::PostPaint ();
    CStdRaum::PumpToolTips ();
}

//--------------------------------------------------------------------------------------------
//void CRicks::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CRicks::OnLButtonDown(UINT nFlags, CPoint point)
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
        if (MouseClickArea==ROOM_RICKS && MouseClickId==999) { Sim.Players.Players[(SLONG)PlayerNum].LeaveRoom();
        } else if (MouseClickArea==ROOM_RICKS && MouseClickId==10) { StartDialog (TALKER_RICK, MEDIUM_AIR, 0); }
        else if (MouseClickArea==ROOM_RICKS && MouseClickId==11) { StartDialog (TALKER_TRINKER, MEDIUM_AIR, 0); }
        else { CStdRaum::OnLButtonDown(nFlags, point);
}
    }
}

//--------------------------------------------------------------------------------------------
//void CRicks::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CRicks::OnRButtonDown(UINT nFlags, CPoint point)
{
    DefaultOnRButtonDown ();

    //Auﬂerhalb geklickt? Dann Default-Handler!
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
                Sim.Players.Players[(SLONG)PlayerNum].LeaveRoom();

            CStdRaum::OnRButtonDown(nFlags, point);
        }
   
}
