//============================================================================================
// World.cpp : Telefonat mit den Niederlassungen
//============================================================================================
#include "StdAfx.h"
#include "World.h"

#define GFX_SAT1 (0x0000000031544153)
#define GFX_WORLD (0x000000444c524f57)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern SB_CColorFX ColorFX;

//--------------------------------------------------------------------------------------------
// Telefonat mit der Niederlassung
//--------------------------------------------------------------------------------------------
CWorld::CWorld(BOOL bHandy, ULONG PlayerNum, SLONG CityId) : CStdRaum(bHandy, PlayerNum, "world.gli", GFX_WORLD) {
    CWorld::CityId = CityId;

    SLONG EarthAlpha = UWORD((Cities[CityId].GlobusPosition.x + 170) * (3200 / 18) - 16000 + 1300);

#ifdef WIN32
    EarthBm.ReSize(const_cast<char *>((LPCTSTR)FullFilename("earthall.lbm", GliPath)), SYSRAMBM);
#else
    EarthBm.ReSize(const_cast<char *>((LPCTSTR)FullFilename("earthall.tga", GliPath)), SYSRAMBM);
#endif

    Satellite.ReSize(pRoomLib, GFX_SAT1);
    LightAnim.ReSize(pRoomLib, "SATLICHT", 1, nullptr, FALSE, ANIMATION_MODE_REPEAT, 100, 2, 100);
    SendingAnim.ReSize(pRoomLib, "SENDER1", 3, nullptr, FALSE, ANIMATION_MODE_REPEAT, 0, 5);

    HandyOffset = 170;

    XY tmp = Cities[CityId].GlobusPosition;
    EarthProjectize(tmp, static_cast<UWORD>(EarthAlpha), &BubblePos);

    BubblePos += XY(100 + HandyOffset, 20);

    if (BubblePos.y > 200) {
        BubbleStyle = 2;
        BubblePos += XY(20, -30);
    } else {
        BubbleStyle = 1;
        BubblePos += XY(35, 0);
    }

    ::PaintGlobe(EarthBm, &PicBitmap, static_cast<UWORD>(EarthAlpha), XY(100, 20));

    for (SLONG c = Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Planes.AnzEntries() - 1; c >= 0; c--) {
        if (Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Planes.IsInAlbum(c) != 0) {
            Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Planes[c].UpdateGlobePos(0);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Sehr destruktiv!
//--------------------------------------------------------------------------------------------
CWorld::~CWorld() { Sim.Players.Players[Sim.localPlayer].Messages.AddMessage(BERATERTYP_AUFTRAG, "", MESSAGE_COMMENT); }

//////////////////////////////////////////////////////////////////////////////////////////////
// CWorld message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// void CWorld::OnPaint()
//--------------------------------------------------------------------------------------------
void CWorld::OnPaint() {
    if (Sim.Gamestate == GAMESTATE_BOOT) {
        return;
    }

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    RoomBm.BlitFromT(Satellite, 30, 285);

    LightAnim.BlitAtT(RoomBm, 30 + 153, 285 + 104);
    SendingAnim.BlitAtT(RoomBm, 30 + 173, 285 + 5 - 127 - 16);

    CStdRaum::PostPaint();
}

//--------------------------------------------------------------------------------------------
// void CWorld::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CWorld::OnLButtonDown(UINT nFlags, CPoint point) {
    XY RoomPos;

    if (Sim.Players.Players[Sim.localPlayer].ArabMode == 6) {
        return;
    }

    DefaultOnLButtonDown();

    if (ConvertMousePosition(point, &RoomPos) == 0) {
        CStdRaum::OnLButtonDown(nFlags, point);
        return;
    }
}

//--------------------------------------------------------------------------------------------
// void CWorld::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CWorld::OnRButtonDown(UINT nFlags, CPoint point) {
    if (Sim.Players.Players[Sim.localPlayer].ArabMode == 6) {
        return;
    }

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
