//============================================================================================
// AskBrick.Cpp : Auswahlmenü für die Bricks des Editors
//============================================================================================
#include "StdAfx.h"
#include "AskBrick.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Anmerkung:
// Askbrick immer und nur dynamisch allokieren und nie de-allokieren. Das wird automatisch
// erledigt. Also immer nur:
// "new AskBrick (this, Groupm, &EditObject, &EditObjects[1]);"

//--------------------------------------------------------------------------------------------
// AskBrick
//--------------------------------------------------------------------------------------------
AskBrick::AskBrick(BOOL bHandy, SLONG PlayerNum, ULONG Group, ULONG *rc1) : CStdRaum(bHandy, PlayerNum, "", 0) {
    AskBrick::rc1 = rc1;
    AskBrick::Group = Group;

    /*RECT rect;

    rect.top = 10;
    rect.left = 10;
    rect.right = 620;
    rect.bottom = 460;

      if (!Create(NULL, "AskBrick", WS_VISIBLE|WS_CHILD, rect, ParentWnd, 42))
      {
      ::MessageBox (NULL, "Create failed", "ERROR", MB_OK );
      return;
      }
      if (bFullscreen) SetWindowPos (&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOREDRAW|SWP_NOSIZE);*/

    pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("askbrick.gli", GliPath)), &pMenuLib, L_LOCMEM);
    AskBrickBm.ReSize(pMenuLib, "ASKBRICK");
}

//--------------------------------------------------------------------------------------------
// ~AskBrick
//--------------------------------------------------------------------------------------------
AskBrick::~AskBrick() {
    AskBrickBm.Destroy();
    if ((pGfxMain != nullptr) && (pMenuLib != nullptr)) {
        pGfxMain->ReleaseLib(pMenuLib);
    }

    TopWin = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// AskBrick message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

void AskBrick::OnPaint() {
    SLONG g = 0;
    SLONG x = 0;
    SLONG y = 0;

    if (bActive != 0) {
        PrimaryBm.BlitFrom(AskBrickBm, 10, 10);

        g = Group;

        for (y = 0; y < 4; y++) {
            for (x = 0; x < 5; x++) {
                while ((Bricks.IsInAlbum(g) == 0) && g - Group < 100) {
                    g++;
                }

                if (Bricks.IsInAlbum(g) != 0) {
                    /*if (Bricks[g].GetBitmapDimension().x<120 && Bricks[g].GetBitmapDimension().y<110)
                      { */
                    Bricks[g].BlitAt(PrimaryBm, 0, 124 * x + 10 + (124 - Bricks[g].GetBitmapDimension().x) / 2,
                                     115 * y + 10 + (115 - Bricks[g].GetBitmapDimension().y) / 2, 0);
                    /*}
                      else
                      {
                      DOUBLE f=min(119.0/Bricks[g].GetBitmapDimension().x, 109.0/Bricks[g].GetBitmapDimension().y);

                      Bricks[g].BlitAt (PrimaryBm,
                      0,
                      SLONG(124*x+10+(124-Bricks[g].GetBitmapDimension().x*f)/2),
                      SLONG(115*y+10+(115-Bricks[g].GetBitmapDimension().y*f)/2),
                      SLONG(124*x+10+(124-Bricks[g].GetBitmapDimension().x*f)/2+Bricks[g].GetBitmapDimension().x*f),
                      SLONG(115*y+10+(115-Bricks[g].GetBitmapDimension().y*f)/2+Bricks[g].GetBitmapDimension().y*f));
                      } */
                }

                g++;
            }
        }
    }
}

void AskBrick::OnLButtonDown(UINT /*nFlags*/, CPoint point) {
    SLONG g = 0;
    SLONG x = 0;
    SLONG y = 0;

    g = Group;

    for (y = 0; y < 5; y++) {
        for (x = 0; x < 5; x++) {
            while ((Bricks.IsInAlbum(g) == 0) && g - Group < 100) {
                g++;
            }

            if (Bricks.IsInAlbum(g) != 0) {
                if (point.x >= 124 * x + 10 && point.x <= 124 * (x + 1) + 10 && point.y >= 112 * y + 10 && point.y <= 112 * (y + 1) + 10) {
                    *rc1 = g;
                }
            }

            g++;
        }
    }

    // ReferTo (nFlags);
    // ReferTo (point);
}

void AskBrick::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/) {
    this->OnClose();
    // ReferTo (nFlags);
    // ReferTo (point);
}

void AskBrick::OnClose() {
    // Self-Destruct (zerstört Fenster und gibt anschließend Speicher frei):
    delete this;
}

void AskBrick::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/) {
    switch (nChar) {
    case ATKEY_UP:
        Group -= 5;
        break;

    case ATKEY_DOWN:
        Group += 5;
        break;

    case ATKEY_LEFT:
        Group--;
        break;

    case ATKEY_RIGHT:
        Group++;
        break;

    case ATKEY_PRIOR:
        Group -= 20;
        break;

    case ATKEY_NEXT:
        Group += 20;
        break;
    default:
        break;
    }
}

//--------------------------------------------------------------------------------------------
// BOOL CStdRaum::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) : AG:
//--------------------------------------------------------------------------------------------
BOOL AskBrick::OnSetCursor(void *pWnd, UINT nHitTest, UINT message) { return (FrameWnd->OnSetCursor(pWnd, nHitTest, message)); }

//--------------------------------------------------------------------------------------------
// void CStdRaum::OnMouseMove(UINT nFlags, CPoint point): AG:
//--------------------------------------------------------------------------------------------
void AskBrick::OnMouseMove(UINT nFlags, CPoint point) { FrameWnd->OnMouseMove(nFlags, point); }
