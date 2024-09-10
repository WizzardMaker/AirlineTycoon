//============================================================================================
// StdRaum.cpp : Die Statuszeile
//============================================================================================
// Link: "StdRaum.h"
//============================================================================================
#include "StdAfx.h"

#include "ArabAir.h"
#include "Aufsicht.h"
#include "Bank.h"
#include "Buero.h"
#include "Designer.h"
#include "DutyFree.h"
#include "Editor.h"
#include "Fracht.h"
#include "Kiosk.h"
#include "Makler.h"
#include "Museum.h"
#include "Nasa.h"
#include "Ricks.h"
#include "RouteBox.h"
#include "Sabotage.h"
#include "Security.h"
#include "WeltAll.h"
#include "Werbung.h"
#include "World.h"

#include "AtNet.h"
#include "SbLib.h"
#include "network.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Abstand zwischen zwei Optionen im Dialog:
#define LINE_DIST 5

extern SLONG BeraterSlideY[];
extern BOOL IgnoreNextLButtonUp;
extern SBNetwork gNetwork;

extern SB_CColorFX ColorFX;

// static SLONG NumPlayerWins = 0;

static XY BigPlayerOffset[4] = {XY(210, 235), XY(220, 240), XY(220, 240), XY(220, 240)};

static GfxLib *pRoomLibStatic = nullptr;

XY BeraterSprechblasenOffset[] = {
    XY(26, 56),       //  0 = BERATERTYP_GIRL
    XY(10, 84),       //  1 = BERATERTYP_PERSONAL (Universell)
    XY(0, 0),         //  2 = BERATERTYP_KEROSIN
    XY(0, 0),         //  3 = BERATERTYP_ROUTE
    XY(0, 0),         //  4 = BERATERTYP_AUFTRAG
    XY(0, 0),         //  5 = BERATERTYP_GELD
    XY(0, 0),         //  6 = BERATERTYP_INFO
    XY(0, 0),         //  7 = BERATERTYP_FLUGZEUG
    XY(26, 56),       //  8 = BERATERTYP_FITNESS  (Fem. Sicherheit)
    XY(26, 56),       //  9 = BERATERTYP_SICHERHEIT
    XY(0, 0),         // 10 = WORKER_PILOT
    XY(0, 0),         // 11 = WORKER_STEWARDESS
    XY(73, 389 - 12), // 12 = Player 1
    XY(73, 389),      // 13 = Player 2
    XY(82, 385),      // 14 = Player 3
    XY(82, 385)       // 15 = Player 4
};

// Für den Taschenrechner
XY CalcOffsets[] = {
    XY(0, 0),                                                                                                                  // Rechner
    XY(82, 19),  XY(82, 19), XY(82, 19),  XY(82, 19),  XY(82, 19), XY(82, 19), XY(82, 19), XY(82, 19), XY(82, 19), XY(82, 19), // Digit ganz rechts
    XY(6, 125),  XY(6, 105), XY(36, 105), XY(66, 105), XY(6, 85),  XY(36, 85), XY(66, 85), XY(6, 65),  XY(36, 65), XY(66, 65), // Tasten
    XY(36, 125), XY(67, 46), XY(5, 45)};

static UBYTE FilofaxRoomRemapper[] = {ROOM_REISEBUERO, ROOM_ARAB_AIR, ROOM_SABOTAGE, ROOM_BANK,        ROOM_BURO_A, ROOM_SHOP1, ROOM_AUFSICHT,
                                      ROOM_FRACHT,     ROOM_MAKLER,   ROOM_KIOSK,    ROOM_LAST_MINUTE, ROOM_MUSEUM, ROOM_NASA,  ROOM_PERSONAL_A,
                                      ROOM_RICKS,      ROOM_ROUTEBOX, ROOM_WERBUNG,  ROOM_WERKSTATT,   255};

static UBYTE HandyRoomRemapper[] = {
    ROOM_ARAB_AIR, ROOM_BANK,      ROOM_BANK, ROOM_AUFSICHT, ROOM_MAKLER, ROOM_MUSEUM, ROOM_NASA, ROOM_PERSONAL_A, ROOM_PERSONAL_A,
    ROOM_WERBUNG,  ROOM_WERKSTATT, 0,         254,           ROOM_WORLD,  255};

static DWORD RoomTipTable[] = {ROOM_BURO_A,
                               1,
                               ROOM_BURO_B,
                               1,
                               ROOM_BURO_C,
                               1,
                               ROOM_BURO_D,
                               1,
                               ROOM_PERSONAL_A,
                               2,
                               ROOM_PERSONAL_B,
                               2,
                               ROOM_PERSONAL_C,
                               2,
                               ROOM_PERSONAL_D,
                               2,
                               ROOM_SHOP1,
                               3,
                               ROOM_BANK,
                               4,
                               ROOM_KIOSK,
                               5,
                               ROOM_MUSEUM,
                               6,
                               ROOM_LAST_MINUTE,
                               7,
                               ROOM_ARAB_AIR,
                               8,
                               ROOM_MAKLER,
                               9,
                               ROOM_AUFSICHT,
                               10,
                               ROOM_TAFEL,
                               11,
                               ROOM_WERBUNG,
                               12,
                               ROOM_WERKSTATT,
                               13,
                               ROOM_REISEBUERO,
                               14,
                               ROOM_SABOTAGE,
                               15,
                               ROOM_NASA,
                               16,
                               ROOM_INSEL,
                               17,
                               ROOM_RUSHMORE,
                               18,
                               ROOM_STATISTICS,
                               19,
                               ROOM_PLANEPROPS,
                               20,
                               ROOM_RICKS,
                               21,
                               ROOM_ROUTEBOX,
                               22,
                               ROOM_FRACHT,
                               23,
                               ROOM_WELTALL,
                               24,
                               ROOM_EDITOR,
                               25,
                               ROOM_SECURITY,
                               26,
                               ROOM_DESIGNER,
                               27,
                               0,
                               0};

// Bei Dialogen sind zwei Fenster offen und es ist zufall, wer den Klick bekommt...
// Also wird gepacht und über diese Variablen der Klick ans erste Fenster geleitet...
BOOL WasLButtonDown = FALSE;
CPoint WasLButtonDownPoint;
SLONG WasLButtonDownMouseClickArea; // In Statusleiste/Raum
SLONG WasLButtonDownMouseClickId;   // Der Id
SLONG WasLButtonDownMouseClickPar1;
SLONG WasLButtonDownMouseClickPar2;

extern CString gHostIP;

// Öfnungszeiten:
extern SLONG timeArabOpen;
extern SLONG timeMuseOpen;
extern SLONG timeMaklClose;
extern SLONG timeWerbOpen;

class CWaitCursorNow {
  public:
    CWaitCursorNow() {
        MouseWait++;
        pCursor->SetImage(gCursorSandBm.pBitmap);
    };

    ~CWaitCursorNow() {
        MouseWait--;
        if (MouseWait == 0) {
            pCursor->SetImage(gCursorBm.pBitmap);
        }
    };
};

static SLONG IgnoreNextPostPaintPump;
static bool bDestructorCalledInMeantime = false;

//--------------------------------------------------------------------------------------------
// Sort dafür, daß die Statusanzeige erneuert wird:
//--------------------------------------------------------------------------------------------
void UpdateStatusBar() {
    if (Sim.localPlayer != -1 && (Sim.Players.Players[Sim.localPlayer].LocationWin != nullptr)) {
        (Sim.Players.Players[Sim.localPlayer].LocationWin)->StatusCount = max(3, (Sim.Players.Players[Sim.localPlayer].LocationWin)->StatusCount);
    }
}

//--------------------------------------------------------------------------------------------
// Gibt die Nummer anhand des Id's zurück:
//--------------------------------------------------------------------------------------------
void SetRoomVisited(SLONG PlayerNum, UBYTE RoomId) {
    SLONG c = 0;
    BOOL WasAlreadyHere = FALSE;

    for (c = 0; c < RoomTipTable[c]; c += 2) {
        if (RoomTipTable[c] == RoomId) {
            WasAlreadyHere = (Sim.Options.OptionRoomDescription & (1 << RoomTipTable[c + 1]));
            Sim.Options.OptionRoomDescription |= (1 << RoomTipTable[c + 1]);
        }
    }

    if (RoomId == ROOM_BURO_A || RoomId == ROOM_BURO_B || RoomId == ROOM_BURO_C || RoomId == ROOM_BURO_D) {
        if (WasAlreadyHere == 0) {
            if ((Sim.Players.Players[PlayerNum].WasInRoom[ROOM_BURO_A] == 0) && (Sim.Difficulty != DIFF_TUTORIAL || Sim.Tutorial == 9999)) {
                Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 10000 + 10));
            }
        }

        Sim.Players.Players[PlayerNum].WasInRoom[ROOM_BURO_A] = TRUE;
    }

    if (RoomId == ROOM_PERSONAL_A || RoomId == ROOM_PERSONAL_B || RoomId == ROOM_PERSONAL_C || RoomId == ROOM_PERSONAL_D) {
        if (WasAlreadyHere == 0) {
            if ((Sim.Players.Players[PlayerNum].WasInRoom[ROOM_PERSONAL_A] == 0) && (Sim.Difficulty != DIFF_TUTORIAL || Sim.Tutorial == 9999)) {
                Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 10000 + 11));
            }
        }

        Sim.Players.Players[PlayerNum].WasInRoom[ROOM_PERSONAL_A] = TRUE;
    }

    if (RoomId == ROOM_PERSONAL_A || RoomId == ROOM_PERSONAL_B || RoomId == ROOM_PERSONAL_C || RoomId == ROOM_PERSONAL_D) {
        RoomId = ROOM_PERSONAL_A;
    }

    if (RoomId == ROOM_BURO_A || RoomId == ROOM_BURO_B || RoomId == ROOM_BURO_C || RoomId == ROOM_BURO_D) {
        RoomId = ROOM_BURO_A;
    }

    if (WasAlreadyHere == 0) {
        if ((Sim.Players.Players[PlayerNum].WasInRoom[static_cast<SLONG>(RoomId)] == 0) && (Sim.Difficulty != DIFF_TUTORIAL || Sim.Tutorial == 9999)) {
            if (RoomId == ROOM_WERBUNG && Sim.Difficulty < DIFF_NORMAL && Sim.Difficulty != DIFF_FREEGAME) {
                Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 20000 + RoomId));
            } else if (RoomId != ROOM_AIRPORT && RoomId != ROOM_LAPTOP && RoomId != ROOM_ABEND && RoomId != ROOM_WORLD && RoomId != 254) {
                Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 10000 + RoomId));
            }
        }
    }

    Sim.Players.Players[PlayerNum].WasInRoom[static_cast<SLONG>(RoomId)] = TRUE;
}

//--------------------------------------------------------------------------------------------
// ohne Worte
//--------------------------------------------------------------------------------------------
BOOL WasRoomVisited(SLONG PlayerNum, UBYTE RoomId) {
    if (RoomId == ROOM_BURO_A || RoomId == ROOM_BURO_B || RoomId == ROOM_BURO_C || RoomId == ROOM_BURO_D) {
        return (Sim.Players.Players[PlayerNum].WasInRoom[ROOM_BURO_A]);
    }

    if (RoomId == ROOM_PERSONAL_A || RoomId == ROOM_PERSONAL_B || RoomId == ROOM_PERSONAL_C || RoomId == ROOM_PERSONAL_D) {
        return (Sim.Players.Players[PlayerNum].WasInRoom[ROOM_PERSONAL_A]);
    }

    return (Sim.Players.Players[PlayerNum].WasInRoom[static_cast<SLONG>(RoomId)]);
}

//--------------------------------------------------------------------------------------------
// Ein Standardfenster mit optionaler Raumbildverwaltung wird erzeugt:
//--------------------------------------------------------------------------------------------
CStdRaum::CStdRaum(BOOL handy, ULONG playerNum, const CString &GfxLibName, __int64 graficId) {
    gFramesToDrawBeforeFirstBlend = 0;

    CWaitCursorNow wc; // CD-Cursor anzeigen
    UpdateStatusBar();

    if (handy == 0) {
        Sim.Players.Players[static_cast<SLONG>(playerNum)].Messages.StopDialog();
    }

    IsInBuro = FALSE;

    CStdRaum::KonstruktorFinished = 0;
    CStdRaum::bHandy = handy;
    CStdRaum::PlayerNum = playerNum;
    CStdRaum::HandyOffset = 160;

    CStdRaum::ForceRedrawTip = FALSE;
    CStdRaum::CurrentTipType = TIP_NONE;
    CStdRaum::LastTipType = CStdRaum::CurrentTipId = CStdRaum::LastTipId = CStdRaum::CurrentTipIdPar1 = CStdRaum::LastTipIdPar1 = CStdRaum::CurrentTipIdPar2 =
        CStdRaum::LastTipIdPar2 = -1;
    CStdRaum::TipPos = XY(0, 0);
    CStdRaum::AirportRoomPos = XY(0, 0);
    CStdRaum::TalkingSpeechFx = FALSE;
    CStdRaum::ReadyToStartSpeechFx = 0;

    CStdRaum::LastMoney = -1;
    CStdRaum::CurrentMenu = MENU_NONE;

    CStdRaum::DialogPartner = TALKER_NONE;
    CStdRaum::DefaultDialogPartner = TALKER_NONE;

    CStdRaum::MenuDialogReEntryB = -1;

    CStdRaum::BubblePos = XY(378, 200);
    CStdRaum::BubbleStyle = 2; // Von Süden
    CStdRaum::StatusCount = 20;

    CStdRaum::ZoomCounter = 0;
    CStdRaum::MinimumZoom = 1.0;

    CStdRaum::pMenuLib1 = nullptr;
    CStdRaum::pMenuLib2 = nullptr;
    CStdRaum::UsingHandy = FALSE;
    CStdRaum::Ferngespraech = FALSE;

    CStdRaum::CalculatorIsOpen = FALSE;
    CStdRaum::pCalculatorLib = nullptr;

    CStdRaum::pSmackerPartner = nullptr;
    CStdRaum::pSmackerPlayer = nullptr;

    CStdRaum::TalkedToA = FALSE;
    CStdRaum::TalkedToB = FALSE;

    IgnoreNextPostPaintPump = 0;

    if (pRoomLibStatic != nullptr) {
        pRoomLib = pRoomLibStatic;
        pRoomLibStatic = nullptr;
    } else {
        if (GfxLibName.GetLength() > 0) {
            pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename(GfxLibName, RoomPath)), &pRoomLib, L_LOCMEM);
        } else {
            pRoomLib = nullptr;
        }
    }

    if (bHandy != 0) {
        TempScreenScroll = 800;
        TempScreenScrollV = -1;
        LastScrollTime = -1;
    }

    // Kein Text aktiv:
    CurrentTextGroupId = 0;
    LastTextGroupId = *reinterpret_cast<const ULONG *>("none");

    if (graficId == 0) {
        PicBitmap.ReSize(SLONG(0), SLONG(0));
    } else {
        PicBitmap.ReSize(pRoomLib, graficId, CREATE_SYSMEM);
    }

    RoomBm.ReSize(PicBitmap.Size, CREATE_SYSMEM);

    WindowRect = CRect(0, 0, 640, 480);
    StatusLineSizeY = 40;

    WinP1 = XY(WindowRect.left, WindowRect.top);
    WinP2 = XY(WindowRect.right, WindowRect.bottom);

    StatusBm.ReSize(230, StatusLineSizeY);

    // Und noch 'ne Anomalie. Der Player hält 'ne Kopie der Fenster-Koordinaten
    if (PlayerNum != -1) {
        Sim.Players.Players[static_cast<SLONG>(PlayerNum)].WinP1 = WinP1;
        Sim.Players.Players[static_cast<SLONG>(PlayerNum)].WinP2 = WinP2;
    }

    // if (!Create(NULL, "PlayerWin", WS_VISIBLE|WS_CHILD, WindowRect, FrameWnd, NumPlayerWins++))
    //   TeakLibW_Exception (FNL, ExcCreateWindow);
    // if (bFullscreen) SetWindowPos (&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOREDRAW|SWP_NOSIZE);

    // SetActiveWindow();

    for (SLONG c = 0; c < 3; c++) {
        BackgroundWait[c] = -1;
    }
}

//--------------------------------------------------------------------------------------------
// Die Welt geht unter:
//--------------------------------------------------------------------------------------------
CStdRaum::~CStdRaum() {
    pSmackerPartner = nullptr;

    bDestructorCalledInMeantime = true;

    CalculatorFX.Stop();
    SpeechFx.Stop();

    gRoomJustLeft = TRUE;
    if (CurrentMenu != MENU_NONE) {
        MenuStop();
    }

    CalculatorBms.Destroy();
    if ((pCalculatorLib != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pCalculatorLib);
        pCalculatorLib = nullptr;
    }

    if (PlayerNum >= 0 && PlayerNum <= 3 && Sim.Players.Players.AnzEntries() == 4) {
        CMessages &qMessages = Sim.Players.Players[PlayerNum].Messages;

        // Spieler ggf. wegzaubern
        if (qMessages.AktuellerBeraterTyp >= 100) {
            qMessages.IsMonolog = FALSE;
            qMessages.IsDialogTalking = FALSE;
            qMessages.TalkPhase = 0;
            qMessages.TalkCountdown = 0;
        }

        qMessages.NoComments();
    }

    if (PlayerNum >= 0 && Sim.Players.Players.AnzEntries() > 0 && (Sim.Players.Players[PlayerNum].DialogWin != nullptr) &&
        Sim.Players.Players[PlayerNum].DialogWin != this) {
        delete Sim.Players.Players[PlayerNum].DialogWin;
        Sim.Players.Players[PlayerNum].DialogWin = nullptr;
    }

    PlayerNum = -1;

    PicBitmap.Destroy();

    if (pRoomLibStatic == nullptr && (pRoomLib != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pRoomLib);
        pRoomLib = nullptr;
    }
}

SLONG UTF8Toisolat1(unsigned char *out, SLONG outlen, const unsigned char *in, SLONG inlen);

SLONG UTF8Toisolat1(unsigned char *out, SLONG outlen, const unsigned char *in, SLONG inlen) {
    unsigned char *outstart = out;
    unsigned char *outend = out + outlen;
    const unsigned char *inend = in + inlen;
    unsigned char c = 0;

    while (in < inend) {
        c = *in++;
        if (c < 0x80) {
            if (out >= outend) {
                return -1;
            }
            *out++ = c;
        } else if (((c & 0xFE) == 0xC2) && in < inend) {
            if (out >= outend) {
                return -1;
            }
            *out++ = ((c & 0x03) << 6) | (*in++ & 0x3F);
        } else {
            return -2;
        }
    }
    return out - outstart;
}

void CStdRaum::ProcessEvent(const SDL_Event &event, const CPoint &position) {
    // SDL_Log("%d",event.type);
    switch (event.type) {
    case SDL_MOUSEMOTION: {
        OnMouseMove(0, position);
    } break;
    case SDL_TEXTINPUT:
    case SDL_TEXTEDITING: {
        unsigned char testValue = '\xE4';
        UTF8Toisolat1(&testValue, 1, reinterpret_cast<const unsigned char *>(&event.text.text), 2);
        OnChar(testValue, 1, (SDL_GetModState() & KMOD_LALT) << 5);
    } break;
    case SDL_KEYDOWN: {
        UINT nFlags = event.key.keysym.scancode | ((SDL_GetModState() & KMOD_LALT) << 5);
        OnKeyDown(KeycodeToUpper(event.key.keysym.sym), event.key.repeat, nFlags);

        // bool upper = SDL_GetModState() & KMOD_SHIFT || SDL_GetModState() & KMOD_CAPS;
        // OnChar(upper ? toupper(test) : event.key.keysym.sym,
        //   event.key.repeat, nFlags);
    } break;
    case SDL_MOUSEBUTTONDOWN: {
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (event.button.clicks == 2) {
                OnLButtonDblClk(WM_LBUTTONDBLCLK, position);
            } else {
                OnLButtonDown(WM_LBUTTONDOWN, position);
            }
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            OnRButtonDown(WM_RBUTTONDOWN, position);
        }
    } break;
    case SDL_MOUSEBUTTONUP: {
        if (event.button.button == SDL_BUTTON_LEFT) {
            OnLButtonUp(WM_LBUTTONUP, position);
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            OnRButtonUp(WM_RBUTTONUP, position);
        }
    } break;
    default:
        break;
    }
}

//--------------------------------------------------------------------------------------------
// Ein neues Hintergrundbild einbinden:
//--------------------------------------------------------------------------------------------
void CStdRaum::ReSize(const CString &GfxLibName, __int64 graficId) {
    PicBitmap.Destroy();

    if ((pRoomLib != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pRoomLib);
        pRoomLib = nullptr;
    }

    if (GfxLibName.GetLength() > 0) {
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename(GfxLibName, RoomPath)), &pRoomLib, L_LOCMEM);
    } else {
        pRoomLib = nullptr;
    }

    if (graficId == 0) {
        PicBitmap.ReSize(SLONG(0), SLONG(0));
    } else {
        PicBitmap.ReSize(pRoomLib, graficId, CREATE_SYSMEM);
    }

    RoomBm.ReSize(PicBitmap.Size, CREATE_SYSMEM);
}

//--------------------------------------------------------------------------------------------
// Ein neues Hintergrundbild einbinden:
//--------------------------------------------------------------------------------------------
void CStdRaum::ReSize(__int64 graficId) {
    if (graficId == 0) {
        PicBitmap.ReSize(SLONG(0), SLONG(0));
    } else {
        PicBitmap.ReSize(pRoomLib, graficId, CREATE_SYSMEM);
    }

    RoomBm.ReSize(PicBitmap.Size, CREATE_SYSMEM);
}

//--------------------------------------------------------------------------------------------
// Setzt ambiente Hintergrundsounds
//--------------------------------------------------------------------------------------------
void CStdRaum::SetBackgroundFx(SLONG Number, const CString &Filename, SLONG AvgWait, SLONG StartWait, SLONG Lautstaerke) {
    BackgroundFX[Number].ReInit(Filename);
    BackgroundWait[Number] = AvgWait;
    BackgroundCount[Number] = AtGetTime() + AvgWait * (rand() % 100 + 50) / 100 - StartWait;
    BackgroundLautstaerke[Number] = Lautstaerke;
}

//--------------------------------------------------------------------------------------------
// Gibt zurück, ob zur Zeit ein Dialog offen ist:
//--------------------------------------------------------------------------------------------
BOOL CStdRaum::IsDialogOpen() const { return static_cast<BOOL>(CurrentTextGroupId != 0 || DialogPartner != TALKER_NONE || (bHandy != 0)); }

//--------------------------------------------------------------------------------------------
// Läßt ein neues Text-Fenster auf dem Schirm erscheinen:
//--------------------------------------------------------------------------------------------
void CStdRaum::MakeSayWindow(BOOL TextAlign, ULONG SubId, const CString &String, SB_CFont *Normal) {
    CanCancelEmpty = FALSE;
    TimeAtStart = AtGetTime();

    CurrentHighlight = 0;
    if (CurrentTextGroupId == 0) {
        CurrentTextGroupId = LastTextGroupId;
    }
    CurrentTextSubIdVon = SubId;
    CurrentTextSubIdBis = 0;
    CStdRaum::TextAlign = TextAlign;

    TimeBubbleDisplayed = AtGetTime();
    DisplayThisBubble =
        static_cast<BOOL>((static_cast<BOOL>(Sim.Options.OptionSpeechBubble != 0) != 0) || Sim.Options.OptionTalking * Sim.Options.OptionMasterVolume == 0 ||
                          Sim.Options.OptionDigiSound == 0 || Sim.Options.OptionEnableDigi == 0);

    if (TalkingSpeechFx != 0) {
        SpeechFx.Stop();
        TalkingSpeechFx = FALSE;
    }

    // Styles kopieren:
    pFontNormal = Normal;
    pFontHighlight = Normal;

    // Daten zuweisen:
    Optionen[0] = RemoveSpeechFilename(String);
    OrgOptionen[0] = String;

    if (TextAlign != 0) {
        SLONG RightBorder = 5;
        if (DialogPartner == TALKER_COMPETITOR) { // Bei Dialog mit anderen Spielern rechts mehr Platz lassen
            RightBorder = 70;
        }

        CurrentTextSubIdBis = CurrentTextSubIdVon;

        if (Sim.Players.Players[PlayerNum].GetRoom() == ROOM_BURO_A + PlayerNum * 10 && TextAlign != 0) { // Im Büro?
            TextAreaSizeY[0] =
                OnscreenBitmap.TryPrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, BigPlayerOffset[Sim.localPlayer].x + 48, 0, 620 - RightBorder, 400);
        } else {
            TextAreaSizeY[0] = OnscreenBitmap.TryPrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, BeraterSprechblasenOffset[12 + Sim.localPlayer].x + 48, 0,
                                                         620 - RightBorder, 400);
        }
    }

    // Die Bitmap aktualisieren:
    RepaintText(TRUE);

    if (((Sim.Options.OptionTalking * Sim.Options.OptionMasterVolume) != 0) && Sim.Options.OptionDigiSound == 1) {
        SLONG pl = 0;
        if (TextAlign == 1) {
            pl = Sim.localPlayer;
        } else {
            pl = DialogPar1;
        }

        BOOL bAnyMissing = 0;
        ReadyToStartSpeechFx = 0;
        if (CreateSpeechSBFX(String, &SpeechFx, pl, &bAnyMissing) != 0) {
            if ((pSmackerPartner != nullptr) && TextAlign == 0) {
                ReadyToStartSpeechFx = 3;
            } else {
                SpeechFx.Play(0, Sim.Options.OptionTalking * 100 / 7);
            }

            TalkingSpeechFx = TRUE;
            if (bAnyMissing != 0) {
                TalkingSpeechFx = FALSE;
            }
        } else {
            TalkingSpeechFx = FALSE;
        }
    } else {
        TalkingSpeechFx = FALSE;
    }
}

//--------------------------------------------------------------------------------------------
// Läßt ein neues Text-Fenster auf dem Schirm erscheinen:
//--------------------------------------------------------------------------------------------
void CStdRaum::MakeSayWindow(BOOL TextAlign, const char *GroupId, ULONG SubId, SB_CFont *Normal, ...) {
    char TmpString[4096];

    CanCancelEmpty = FALSE;
    TimeAtStart = AtGetTime();

    DisplayThisBubble = static_cast<BOOL>((static_cast<BOOL>(Sim.Options.OptionSpeechBubble != 0) != 0) ||
                                          (Sim.Options.OptionTalking * Sim.Options.OptionMasterVolume == 0 || Sim.Options.OptionDigiSound == 0));

    if (TalkingSpeechFx != 0) {
        SpeechFx.Stop();
        TalkingSpeechFx = FALSE;
    }

    TimeBubbleDisplayed = AtGetTime();

    // Hilfskonstruktion für beliebige viele Argumente deklarieren:
    va_list Vars;

    // Tabelle initialisieren:
    va_start(Vars, Normal);

    // Die gesammten Parameter "reinvestieren":
    vsnprintf(TmpString, sizeof(TmpString), DialogTexte.GetS(GroupId, SubId), Vars);

    // Daten bereinigen:
    va_end(Vars);

    // Daten zuweisen:
    MakeSayWindow(TextAlign, SubId, CString(TmpString), Normal);
    CurrentTextGroupId = *(reinterpret_cast<const ULONG *>(GroupId));
    LastTextGroupId = CurrentTextGroupId;
}

//--------------------------------------------------------------------------------------------
// Läßt ein neues Auswahl Text-Fenster auf dem Schirm erscheinen:
//--------------------------------------------------------------------------------------------
void CStdRaum::MakeSayWindow(BOOL TextAlign, const char *GroupId, ULONG SubIdVon, ULONG SubIdBis, SLONG ParameterIndiziert, SB_CFont *Normal,
                             SB_CFont *Highlight, ...) {
    SLONG c = 0;

    CanCancelEmpty = FALSE;
    TimeAtStart = AtGetTime();

    DisplayThisBubble = static_cast<BOOL>((static_cast<BOOL>(Sim.Options.OptionSpeechBubble != 0) != 0) || (SubIdVon != SubIdBis && SubIdBis != 0) ||
                                          (Sim.Options.OptionTalking * Sim.Options.OptionMasterVolume == 0 || Sim.Options.OptionDigiSound == 0));

    if (TalkingSpeechFx != 0) {
        SpeechFx.Stop();
        TalkingSpeechFx = FALSE;
    }

    TimeBubbleDisplayed = AtGetTime();

    CurrentHighlight = 0;
    CurrentTextGroupId = *(reinterpret_cast<const ULONG *>(GroupId));
    LastTextGroupId = CurrentTextGroupId;
    CurrentTextSubIdVon = SubIdVon;
    CurrentTextSubIdBis = SubIdBis;
    CStdRaum::TextAlign = TextAlign;

    // Styles kopieren:
    pFontNormal = Normal;
    pFontHighlight = Highlight;

    // Delete old strings:
    for (c = 0; c < 10; c++) {
        Optionen[c].Empty();
    }

    char TmpString[4096];

    // Hilfskonstruktion für beliebige viele Argumente deklarieren:
    va_list Vars;

    // Tabelle initialisieren:
    va_start(Vars, Highlight);

    // Die gesammten Parameter "reinvestieren":
    for (c = CurrentTextSubIdVon; c <= static_cast<SLONG>(CurrentTextSubIdBis); c++) {
        if (c - CurrentTextSubIdVon < 0 || c - CurrentTextSubIdVon >= 10) {
            DebugBreak();
        }

        // Sind die Paramter eine Liste für die Antworten oder jeweils alle für alle?
        if (ParameterIndiziert == 1) {
            // MP: 1 string pro Antwort
            LPCTSTR tmp = va_arg(Vars, LPCTSTR);

            snprintf(TmpString, sizeof(TmpString), DialogTexte.GetS(CurrentTextGroupId, c), tmp);
        } else if (ParameterIndiziert == 2) {
            // MP: 2 string pro Antwort
            LPCTSTR tmp1 = va_arg(Vars, LPCTSTR);
            LPCTSTR tmp2 = va_arg(Vars, LPCTSTR);

            snprintf(TmpString, sizeof(TmpString), DialogTexte.GetS(CurrentTextGroupId, c), tmp1, tmp2);
        } else if (ParameterIndiziert == 3) {
            // MP: 1 int pro Antwort
            SLONG tmp = va_arg(Vars, SLONG);

            snprintf(TmpString, sizeof(TmpString), DialogTexte.GetS(CurrentTextGroupId, c), tmp);
        } else {
            // Alle für alle:
            vsnprintf(TmpString, sizeof(TmpString), DialogTexte.GetS(CurrentTextGroupId, c), Vars);
        }

        // Daten zuweisen:
        Optionen[c - CurrentTextSubIdVon] = RemoveSpeechFilename(CString(TmpString));
        OrgOptionen[c - CurrentTextSubIdVon] = TmpString;

        SLONG RightBorder = 5;
        if (DialogPartner == TALKER_COMPETITOR) { // Bei Dialog mit anderen Spielern rechts mehr Platz lassen
            RightBorder = 70;
        }

        if (Sim.Players.Players[PlayerNum].GetRoom() == ROOM_BURO_A + PlayerNum * 10 && TextAlign != 0) { // Im Büro?
            if (DialogPartner == TALKER_COMPETITOR) {
                TextAreaSizeY[c - CurrentTextSubIdVon] = OnscreenBitmap.TryPrintAt(Optionen[c - CurrentTextSubIdVon], *pFontNormal, TEC_FONT_LEFT,
                                                                                   BigPlayerOffset[Sim.localPlayer].x + 48 - 15, 0, 540, 400);
            } else {
                TextAreaSizeY[c - CurrentTextSubIdVon] = OnscreenBitmap.TryPrintAt(Optionen[c - CurrentTextSubIdVon], *pFontNormal, TEC_FONT_LEFT,
                                                                                   BigPlayerOffset[Sim.localPlayer].x + 48, 0, 620, 400);
            }
        } else {
            TextAreaSizeY[c - CurrentTextSubIdVon] =
                OnscreenBitmap.TryPrintAt(Optionen[c - CurrentTextSubIdVon], *pFontNormal, TEC_FONT_LEFT,
                                          BeraterSprechblasenOffset[12 + Sim.localPlayer].x + 48, 0, 620 - RightBorder, 400);
        }
    }

    // Daten bereinigen:
    va_end(Vars);

    // Die Bitmap aktualisieren:
    RepaintText(TRUE);

    if (((Sim.Options.OptionTalking * Sim.Options.OptionMasterVolume) != 0) && SubIdVon == SubIdBis && Sim.Options.OptionDigiSound == 1) {
        SLONG pl = 0;
        if (TextAlign == 1) {
            pl = Sim.localPlayer;
        } else {
            pl = DialogPar1;
        }

        ReadyToStartSpeechFx = 0;
        if (CreateSpeechSBFX(CString(TmpString), &SpeechFx, pl) != 0) {
            if ((pSmackerPartner != nullptr) && TextAlign == 0) {
                ReadyToStartSpeechFx = 3;
            } else {
                SpeechFx.Play(0, Sim.Options.OptionTalking * 100 / 7);
            }

            TalkingSpeechFx = TRUE;
        } else {
            TalkingSpeechFx = FALSE;
        }
    } else {
        TalkingSpeechFx = FALSE;
    }
}

//--------------------------------------------------------------------------------------------
// Zeigt ein Fenster mit einer Info zu den (unausgesprochenen) Zahlen an:
//--------------------------------------------------------------------------------------------
void CStdRaum::MakeNumberWindow(CString Text) {
    static GfxLib *pGLib = nullptr;

    for (SLONG c = 0; c < Text.GetLength(); c++) {
        if (Text.GetAt(c) == ' ') {
            Text.SetAt(c, '\1');
        }
    }

    if (Text.GetLength() == 0) {
        NumberBitmap.Destroy();

        if (pGLib != nullptr) {
            gNumberTemplate.Destroy();
            pGfxMain->ReleaseLib(pGLib);
            pGLib = nullptr;
        }
    } else {
        SLONG c = 0;
        SLONG suby = 0;

        if (pGLib == nullptr) {
            pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("glnumber.gli", GliPath)), &pGLib, L_LOCMEM);
            gNumberTemplate.ReSize(pGLib, "NUMBERS");
        }

        for (c = 60; c < 640; c += 10) {
            NumberBitmap.ReSize(c, c * 2 / 3);

            SLONG sizey = NumberBitmap.TryPrintAt(Text, FontDialogPartner, TEC_FONT_LEFT, XY(0, 0), NumberBitmap.Size);
            if (sizey < NumberBitmap.Size.y && sizey != -1) {
                for (suby = c * 2 / 3 / 10 * 10; suby >= 0; suby -= 10) {
                    NumberBitmap.ReSize(c, c * 2 / 3 - suby);

                    SLONG sizey = NumberBitmap.TryPrintAt(Text, FontDialogPartner, TEC_FONT_LEFT, XY(0, 0), NumberBitmap.Size);
                    if (sizey < NumberBitmap.Size.y && sizey != -1) {
                        NumberBitmap.ReSize(c + 20, c * 2 / 3 - suby + 20);

                        SDL_Rect SrcRect = {0, 0, gNumberTemplate.Size.x, gNumberTemplate.Size.y};
                        SDL_Rect DestRect = {0, 0, NumberBitmap.Size.x, NumberBitmap.Size.y};

                        SDL_BlitScaled(gNumberTemplate.pBitmap->GetSurface(), &SrcRect, NumberBitmap.pBitmap->GetSurface(), &DestRect);

                        SLONG sizey = NumberBitmap.TryPrintAt(Text, FontDialogPartner, TEC_FONT_LEFT, XY(10, 10), NumberBitmap.Size - XY(10, 10));

                        NumberBitmap.PrintAt(Text, FontDialogPartner, TEC_FONT_LEFT, XY(10, 10 + (NumberBitmap.Size.y - 20 - sizey) / 2),
                                             NumberBitmap.Size - XY(10, 10));
                        break;
                    }
                }

                break;
            }
        }

        // Auf dem Bildschirm Platz für das Zahlenfeld suchen:
        if (DisplayThisBubble != 0) {
            NumberBitmapPos = XY(320 - NumberBitmap.Size.x / 2, 440 - NumberBitmap.Size.y - 1);
        } else {
            NumberBitmapPos = XY(320 - NumberBitmap.Size.x / 2, 440 - NumberBitmap.Size.y - 1);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Zeigt ein Fenster mit einer Info zu den (unausgesprochenen) Zahlen an:
//--------------------------------------------------------------------------------------------
void CStdRaum::MakeNumberWindow(const char *GroupId, ULONG SubId, ...) {
    char TmpString[4096];

    // Hilfskonstruktion für beliebige viele Argumente deklarieren:
    va_list Vars;

    // Tabelle initialisieren:
    va_start(Vars, SubId);

    // Die gesammten Parameter "reinvestieren":
    vsnprintf(TmpString, sizeof(TmpString), DialogTexte.GetS(GroupId, SubId), Vars);

    // Daten bereinigen:
    va_end(Vars);

    // Daten zuweisen:
    MakeNumberWindow(CString(TmpString));
}

//--------------------------------------------------------------------------------------------
// Frischt den gemalten Text wieder auf:
//--------------------------------------------------------------------------------------------
void CStdRaum::RepaintText(BOOL RefreshAll) {
    static SLONG LastCurrentHighlight = 0;

    if (LastCurrentHighlight != SLONG(CurrentHighlight)) {
        LastCurrentHighlight = CurrentHighlight;
        RefreshAll = 1;
    }

    if (CurrentTextGroupId != 0U) {
        if (RefreshAll != 0) {
            if (OnscreenBitmap.Size != WinP2 - WinP1) {
                OnscreenBitmap.ReSize(WinP2 - WinP1);
            } else {
                OnscreenBitmap.Clear(0);
            }
        }

        if (CurrentTextSubIdBis == 0) {
            SmackerTimeToTalk = AtGetTime() + Optionen[0].GetLength() * 2 * 50;
            if (pSmackerPartner != nullptr) {
                pSmackerPartner->SetDesiredMood(SPM_TALKING);
                pSmackerPartner->Pump();
                pSmackerPartner->Pump();
            }
            TextAlign = 0;

            if (BubbleStyle == 2) {
                SLONG Summe = OnscreenBitmap.TryPrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, 90, 0, 560, 400);
                if (RefreshAll != 0) {
                    TextRect = BubbleRect = PaintTextBubble(OnscreenBitmap, XY(50, BubblePos.y - Summe - 8), XY(600, BubblePos.y + 8), BubblePos + XY(0, 8));
                }
                OnscreenBitmap.PrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, 80, BubblePos.y - Summe, 560, BubblePos.y);
            } else if (BubbleStyle == 0) {
                SLONG Summe = OnscreenBitmap.TryPrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, 90, 0, 560, 400);
                if (RefreshAll != 0) {
                    TextRect = BubbleRect =
                        PaintTextBubble(OnscreenBitmap, XY(50, BubblePos.y - 8), XY(600, BubblePos.y + Summe + 8), BubblePos + XY(0, -8 - 1));
                }
                OnscreenBitmap.PrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, 80, BubblePos.y, 560, BubblePos.y + Summe);
            } else if (BubbleStyle == 1) {
                SLONG LeftBorder = 5;

                if (DialogPartner == TALKER_COMPETITOR) { // Bei Dialog mit anderen Spielern rechts mehr Platz lassen
                    LeftBorder = 80;
                }

                SLONG Summe = OnscreenBitmap.TryPrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, LeftBorder + 45, 0, BubblePos.x - 40, 400);

                if (RefreshAll != 0) {
                    TextRect = BubbleRect = PaintTextBubble(OnscreenBitmap, XY(LeftBorder, BubblePos.y - Summe / 2 - 8),
                                                            XY(BubblePos.x - 10, BubblePos.y + Summe / 2 + 8), XY(BubblePos.x - 10, BubblePos.y));
                }

                OnscreenBitmap.PrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, LeftBorder + 45, BubblePos.y - Summe / 2, BubblePos.x - 40,
                                       BubblePos.y + Summe / 2);
            } else if (BubbleStyle == 3) {
                SLONG Summe = OnscreenBitmap.TryPrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, BubblePos.x + 40, 0, 600, 400);
                BubbleRect = PaintTextBubble(OnscreenBitmap, XY(BubblePos.x + 10, BubblePos.y - Summe / 2 - 8), XY(630, BubblePos.y + Summe / 2 + 8),
                                             XY(BubblePos.x + 10, BubblePos.y));
                TextRect = CRect(BubblePos.x + 10, BubblePos.y - Summe / 2 - 8, 620, BubblePos.y + Summe / 2 + 8);
                OnscreenBitmap.PrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, BubblePos.x + 50, BubblePos.y - Summe / 2, 600, BubblePos.y + Summe / 2);
            }
        } else {
            ULONG c = 0;
            ULONG Summe = 1;
            ULONG Size = 0;

            if (TextAlign != 0) // unten, vom Spieler
            {
                for (c = CurrentTextSubIdVon; c <= CurrentTextSubIdBis; c++) {
                    Summe += TextAreaSizeY[c - CurrentTextSubIdVon] + LINE_DIST;
                }

                Summe -= LINE_DIST;
                Size = Summe;

                if (SLONG(430 - Summe / 2) > BeraterSprechblasenOffset[12 + Sim.localPlayer].y) {
                    Summe = BeraterSprechblasenOffset[12 + Sim.localPlayer].y - Summe / 2;
                } else {
                    Summe = 430 - Summe;
                }

                if (RefreshAll != 0) {
                    if (DialogPartner == TALKER_COMPETITOR) { // Bei Dialog mit anderen Spielern rechts mehr Platz lassen
                        if (Sim.Players.Players[PlayerNum].GetRoom() == ROOM_BURO_A + PlayerNum * 10) // Im Büro?
                        {
                            BubbleRect =
                                PaintTextBubble(OnscreenBitmap, XY(BigPlayerOffset[Sim.localPlayer].x, BigPlayerOffset[Sim.localPlayer].y - Size / 2 - 10),
                                                XY(560, BigPlayerOffset[Sim.localPlayer].y + Size / 2 + 10), BigPlayerOffset[Sim.localPlayer]);
                            TextRect = CRect(BigPlayerOffset[Sim.localPlayer].x + 48 - 15 - 48, BigPlayerOffset[Sim.localPlayer].y - Size / 2 - 10, 540,
                                             BigPlayerOffset[Sim.localPlayer].y + Size / 2 + 10);
                        } else {
                            BubbleRect = PaintTextBubble(
                                OnscreenBitmap, XY(BeraterSprechblasenOffset[12 + Sim.localPlayer].x, Summe - 10), XY(560, Summe + Size + 10),
                                XY(BeraterSprechblasenOffset[12 + Sim.localPlayer].x, BeraterSprechblasenOffset[12 + Sim.localPlayer].y)); // normal
                            TextRect = CRect(BeraterSprechblasenOffset[12 + Sim.localPlayer].x, Summe - 10, 550, Summe + Size + 10);
                        }
                    } else                                                                            // Spieler
                        if (Sim.Players.Players[PlayerNum].GetRoom() == ROOM_BURO_A + PlayerNum * 10) // Im Büro?
                        {
                            BubbleRect =
                                PaintTextBubble(OnscreenBitmap, XY(BigPlayerOffset[Sim.localPlayer].x - 15, BigPlayerOffset[Sim.localPlayer].y - Size / 2 - 10),
                                                XY(630, BigPlayerOffset[Sim.localPlayer].y + Size / 2 + 10), BigPlayerOffset[Sim.localPlayer]);
                            TextRect = CRect(BigPlayerOffset[Sim.localPlayer].x - 15 /*!new*/, BigPlayerOffset[Sim.localPlayer].y - Size / 2 - 10, 620,
                                             BigPlayerOffset[Sim.localPlayer].y + Size / 2 + 10);
                        } else // normal
                        {
                            BubbleRect =
                                PaintTextBubble(OnscreenBitmap, XY(BeraterSprechblasenOffset[12 + Sim.localPlayer].x, Summe - 10), XY(630, Summe + Size + 10),
                                                XY(BeraterSprechblasenOffset[12 + Sim.localPlayer].x, BeraterSprechblasenOffset[12 + Sim.localPlayer].y));
                            TextRect = CRect(BeraterSprechblasenOffset[12 + Sim.localPlayer].x, Summe - 10, 615, Summe + Size + 10);
                        }
                }

                //!
                Summe = TextRect.top + 10;

                Sim.Players.Players[PlayerNum].Messages.IsDialogTalking = 0;
                for (c = CurrentTextSubIdVon; c <= CurrentTextSubIdBis; c++) {
#if 0
                    SLONG RightBorder = 5;
                    if (DialogPartner == TALKER_COMPETITOR) { // Bei Dialog mit anderen Spielern rechts mehr Platz lassen
                        RightBorder = 80;
                    }
#endif

                    if (CurrentTextSubIdVon == CurrentTextSubIdBis || Sim.Options.OptionTalking * Sim.Options.OptionMasterVolume == 0 ||
                        Sim.Options.OptionDigiSound == 0) {
                        Sim.Players.Players[PlayerNum].Messages.IsDialogTalking = Optionen[c - CurrentTextSubIdVon].GetLength() * 2;
                    }

                    if ((c == CurrentHighlight || CurrentTextSubIdVon == CurrentTextSubIdBis) && (IsInBuro != 0)) {
                        (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_TALKING, SPM_LISTENING);
                    }

                    if (c == CurrentHighlight) {
                        OnscreenBitmap.PrintAt(Optionen[c - CurrentTextSubIdVon], *pFontHighlight, TEC_FONT_LEFT, TextRect.left + 18 + 30, Summe,
                                               TextRect.right, TextRect.bottom);
                    } else {
                        OnscreenBitmap.PrintAt(Optionen[c - CurrentTextSubIdVon], *pFontNormal, TEC_FONT_LEFT, TextRect.left + 18 + 30, Summe, TextRect.right,
                                               TextRect.bottom);
                    }

                    if (CurrentTextSubIdVon != CurrentTextSubIdBis) {
                        if (c == CurrentHighlight) {
                            OnscreenBitmap.PrintAt("#", *pFontHighlight, TEC_FONT_LEFT, TextRect.left + 30, Summe, TextRect.right, TextRect.bottom);
                        } else {
                            OnscreenBitmap.PrintAt("#", *pFontNormal, TEC_FONT_LEFT, TextRect.left + 30, Summe, TextRect.right, TextRect.bottom);
                        }
                    }

                    Summe += TextAreaSizeY[c - CurrentTextSubIdVon];
                    Summe += LINE_DIST;
                }
                if (Sim.Players.Players[PlayerNum].Messages.IsDialogTalking == 0) {
                    Sim.Players.Players[PlayerNum].Messages.TalkPhase = 0;
                }

                if (pSmackerPartner != nullptr) {
                    pSmackerPartner->SetDesiredMood(SPM_LISTENING);
                }
                SmackerTimeToTalk = -1;
            } else // oben, vom gesprächspartner
            {
                Summe += TextAreaSizeY[0];
                if (RefreshAll != 0) {
                    TextRect = PaintTextBubble(OnscreenBitmap, XY(55, Summe), XY(585, BubblePos.y), BubblePos);
                }
                OnscreenBitmap.PrintAt(Optionen[0], *pFontNormal, TEC_FONT_LEFT, 85, Summe, 555, 440);

                SmackerTimeToTalk = AtGetTime() + Optionen[0].GetLength() * 2 * 50;
                if (pSmackerPartner != nullptr) {
                    pSmackerPartner->SetDesiredMood(SPM_TALKING);
                }
            }
        }
    } else {
        // Dadurch bricht die Transparenzroutine (falls sie gerade läuft) ab.
        gRoomJustLeft = TRUE;

        OnscreenBitmap.Destroy();
    }
}

//--------------------------------------------------------------------------------------------
// Schließt das TextFenster:
//--------------------------------------------------------------------------------------------
void CStdRaum::CloseTextWindow() {
    MakeNumberWindow("");

    Sim.Players.Players[PlayerNum].Messages.IsDialogTalking = FALSE;
    Sim.Players.Players[PlayerNum].Messages.TalkPhase = 0;
    Sim.Players.Players[PlayerNum].Messages.TalkCountdown = 0;

    MenuDialogReEntryA = CurrentTextGroupId;
    CurrentTextGroupId = 0;

    // Alles löschen:
    for (SLONG c = 0; c < 10; c++) {
        Optionen[c].Empty();
        TextAreaSizeY[c] = SLONG(0);
    }

    // Die Bitmap aktualisieren:
    RepaintText(TRUE);
}

//--------------------------------------------------------------------------------------------
// Startet einen Dialog mit einer Person:
//--------------------------------------------------------------------------------------------
void CStdRaum::StartDialog(SLONG DialogPartner, BOOL Medium, SLONG DialogPar1, SLONG DialogPar2, SLONG DialogPar3) {
    CStdRaum::DialogPartner = DialogPartner;
    CStdRaum::DialogMedium = Medium;
    CStdRaum::DialogPar1 = DialogPar1;
    CStdRaum::DialogPar2 = DialogPar2;
    CStdRaum::DialogPar3 = DialogPar3;
    CStdRaum::ZoomCounter = 0;
    CStdRaum::pSmackerPartner = nullptr;
    CStdRaum::pSmackerPlayer = nullptr;
    CStdRaum::DontDisplayPlayer = -1;

    CStdRaum::PayingForCall = TRUE;

    PERSON &qPerson = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)];

    if (qPerson.LookDir == 9) {
        qPerson.Dir = 8;
        qPerson.LookDir = UBYTE(Clans[static_cast<SLONG>(qPerson.ClanId)].GimmickArt2);
        qPerson.Phase = 0;
    }

    Talkers.Talkers[DialogPartner].StartDialog(DialogMedium);
    Sim.Players.Players[PlayerNum].Messages.StartDialog(PlayerNum);

    if (IsInBuro != 0) {
        (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_LISTENING, SPM_IDLE);
    }

    // Die Farbe des Dialogpartners:
    switch (DialogPartner) {
    case TALKER_FRACHT:
        pFontPartner = &FontDialogPartner;

        pSmackerPartner = &(dynamic_cast<CFrachtRaum *>(this))->SP_Fracht;

        BubblePos = XY(556, 143 - 17);
        BubbleStyle = 1;

        pSmackerPartner->SetDesiredMood(SPM_LISTENING);

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_FRACHT, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 1000) {
            MakeSayWindow(0, TOKEN_FRACHT, 1000, pFontPartner);
        } else if (DialogPar1 == 1010) {
            MakeSayWindow(0, TOKEN_FRACHT, 1010, pFontPartner);
        } else if (DialogPar1 == 1020) {
            MakeSayWindow(0, TOKEN_FRACHT, 1020, pFontPartner);
        }
        break;

    case TALKER_SECURITY:
        pFontPartner = &FontDialogPartner;

        pSmackerPartner = &(dynamic_cast<CSecurity *>(this))->SP_Secman;

        BubblePos = XY(328, 100);
        BubbleStyle = 2;

        pSmackerPartner->SetDesiredMood(SPM_LISTENING);

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_SECURITY, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 1000) {
            MakeSayWindow(0, TOKEN_SECURITY, 1000, pFontPartner);
        }
        break;

    case TALKER_DESIGNER:
        pFontPartner = &FontDialogPartner;

        pSmackerPartner = &(dynamic_cast<CDesigner *>(this))->SP_Stan;

        BubblePos = XY(268, 126);
        BubbleStyle = 0;

        pSmackerPartner->SetDesiredMood(SPM_LISTENING);

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_DESIGNER, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 1000 || DialogPar1 == 6000) {
            MakeSayWindow(0, TOKEN_DESIGNER, DialogPar1, pFontPartner);
        }
        break;

    case TALKER_ARAB:
        pFontPartner = &FontDialogPartner;

        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_ARAB_AIR]++;
            if (DialogMedium == MEDIUM_HANDY && (Sim.Players.Players[PlayerNum].DialogWin != nullptr)) {
                pSmackerPartner = &(dynamic_cast<CArabAir *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Araber;
            }

            BubblePos = XY(526, 183);
            BubbleStyle = 1;
        } else {
            pSmackerPartner = &(dynamic_cast<CArabAir *>(this))->SP_Araber;

            BubblePos = XY(486, 183);
            BubbleStyle = 1;
        }

        pSmackerPartner->SetDesiredMood(SPM_LISTENING);

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_ARAB, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 0) {
            if (Medium != 0) {
                MakeSayWindow(0, TOKEN_ARAB, 101, pFontPartner);
            } else if (TalkedToA != 0) {
                MakeSayWindow(0, TOKEN_ARAB, 102, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_ARAB, 100, pFontPartner);
            }

            TalkedToA = TRUE;
        } else if (DialogPar1 == 1) {
            MakeSayWindow(0, TOKEN_ARAB, 1010, pFontPartner);
        } else if (DialogPar1 == 2) {
            if (Sim.Players.Players[PlayerNum].ArabTrust == 0) {
                MakeSayWindow(0, TOKEN_ARAB, 1000, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_ARAB, 1020, pFontPartner);
            }
        } else if (DialogPar1 == 100) {
            MakeSayWindow(1, TOKEN_ARAB, 400, 403, 3, &FontDialog, &FontDialogLight, Sim.HoleKerosinPreis(0), Sim.HoleKerosinPreis(1), Sim.HoleKerosinPreis(2));
        }

        break;

    case TALKER_SABOTAGE:
        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_SABOTAGE]++;
            if (Sim.Players.Players[PlayerNum].DialogWin != nullptr) {
                pSmackerPartner = &(dynamic_cast<CSabotage *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Araber;
                (Sim.Players.Players[PlayerNum].DialogWin)->HandyOffset = 367;
            }

            BubblePos = XY(479 - 79, 157 + 31);
            BubbleStyle = 1;
        } else {
            pSmackerPartner = &(dynamic_cast<CSabotage *>(this))->SP_Araber;

            BubblePos = XY(396, 198);
            BubbleStyle = 1;
        }

        pFontPartner = &FontDialogPartner;

        if (DialogPar1 == 2000 || DialogPar1 == 1070 || DialogPar1 == 1090 || DialogPar1 == 1091) {
            MakeSayWindow(0, TOKEN_SABOTAGE, DialogPar1, pFontPartner);
        } else if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_SABOTAGE, DialogPar1, pFontPartner);
        } else if (Sim.Players.Players[PlayerNum].ArabTrust == 0) {
            MakeSayWindow(0, TOKEN_SABOTAGE, 1021, pFontPartner);
        } else if (DialogPar1 == 800) {
            if (Sim.Players.Players[PlayerNum].SpiderTrust != 0) {
                MakeSayWindow(0, TOKEN_SABOTAGE, 3002, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_SABOTAGE, 3001, pFontPartner);
            }
        } else if (DialogPar1 == 3000) {
            MakeSayWindow(0, TOKEN_SABOTAGE, 3000, pFontPartner);
        } else if (Sim.Players.Players[PlayerNum].ArabMode == 0 && Sim.Players.Players[PlayerNum].ArabMode2 == 0) {
            if (TalkedToA != 0) {
                MakeSayWindow(0, TOKEN_SABOTAGE, 1022, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_SABOTAGE, 1020, pFontPartner);
            }

            TalkedToA = TRUE;
        } else {
            MakeSayWindow(0, TOKEN_SABOTAGE, 1025, pFontPartner);
        }
        break;

    case TALKER_BANKER1:
        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_BANK]++;
            if (Sim.Players.Players[PlayerNum].DialogWin != nullptr) {
                pSmackerPartner = &(dynamic_cast<Bank *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Mann;
                (Sim.Players.Players[PlayerNum].DialogWin)->HandyOffset = 367;
            }

            BubblePos = XY(479, 157);
            BubbleStyle = 2;
        } else {
            pSmackerPartner = &(dynamic_cast<Bank *>(this))->SP_Mann;

            BubblePos = XY(477, 157);
            BubbleStyle = 2;
        }

        pFontPartner = &FontDialogPartner;

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_BANK, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 20) {
            MakeSayWindow(0, TOKEN_BANK, 920, pFontPartner, (LPCTSTR)Sim.Players.Players[Sim.OvertakenAirline].AirlineX,
                          (LPCTSTR)Sim.Players.Players[Sim.OvertakerAirline].AirlineX);
        } else if (Sim.Overtake != 0) {
            MakeSayWindow(0, TOKEN_BANK, 905 + DialogMedium, pFontPartner, (LPCTSTR)Sim.Players.Players[Sim.OvertakenAirline].AirlineX,
                          (LPCTSTR)Sim.Players.Players[Sim.OvertakerAirline].AirlineX);
        } else {
            MakeSayWindow(0, TOKEN_BANK, 900 + DialogMedium, pFontPartner);
        }
        break;

    case TALKER_BANKER2:
        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_BANK]++;
            if (Sim.Players.Players[PlayerNum].DialogWin != nullptr) {
                pSmackerPartner = &(dynamic_cast<Bank *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Frau;
                (Sim.Players.Players[PlayerNum].DialogWin)->HandyOffset = 50 + 103;
            }

            BubblePos = XY(482 - 15, 183 + 19);
            BubbleStyle = 1;
        } else {
            pSmackerPartner = &(dynamic_cast<Bank *>(this))->SP_Frau;

            BubblePos = XY(240, 147);
            BubbleStyle = 2;
        }
        pFontPartner = &FontDialogPartner;

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_BANK, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 100) {
            MakeSayWindow(0, TOKEN_BANK, 160, pFontPartner);
        } else if (DialogPar1 == 101) {
            MakeSayWindow(0, TOKEN_BANK, 161, pFontPartner);
        } else if (DialogPar1 == 102) {
            MakeSayWindow(0, TOKEN_BANK, 162, pFontPartner);
        } else if (Medium == MEDIUM_AIR) {
            MakeSayWindow(1, TOKEN_BANK, 101, 103, FALSE, &FontDialog, &FontDialogLight);
        } else {
            MakeSayWindow(0, TOKEN_BANK, 109, pFontPartner);
        }
        break;

    case TALKER_BOSS:
        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_AUFSICHT]++;
            if (DialogMedium == MEDIUM_HANDY && (Sim.Players.Players[PlayerNum].DialogWin != nullptr)) {
                pSmackerPartner = &(dynamic_cast<CAufsicht *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Boss;
            }

            BubblePos = XY(470, 158);
            BubbleStyle = 1;
        } else {
            pSmackerPartner = &(dynamic_cast<CAufsicht *>(this))->SP_Boss;

            (dynamic_cast<CAufsicht *>(this))->ExitFromLeft = -1;
            (dynamic_cast<CAufsicht *>(this))->ExitFromMiddle = -1;
            (dynamic_cast<CAufsicht *>(this))->ExitFromRight = -1;

            BubblePos = XY(400, 102); // XY(408,158);
            BubbleStyle = 2;          // 1;
        }
        pFontPartner = &FontDialogPartner;

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_BOSS, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 30) {
            MakeSayWindow(0, TOKEN_BOSS, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 1) {
            if (Sim.Date == 0) {
                if (Sim.Difficulty >= DIFF_ATFS) {
                    MakeSayWindow(0, TOKEN_BOSS, 1800 + (Sim.Difficulty - DIFF_ATFS) * 10, pFontPartner);
                } else if (Sim.Difficulty >= DIFF_ADDON) {
                    if (Sim.Difficulty == DIFF_ADDON03) {
                        MakeSayWindow(0, TOKEN_BOSS, 1600 + (Sim.Difficulty - DIFF_ADDON) * 10, pFontPartner, (LPCTSTR)Cities[Sim.KrisenCity].Name);
                    } else {
                        MakeSayWindow(0, TOKEN_BOSS, 1600 + (Sim.Difficulty - DIFF_ADDON) * 10, pFontPartner);
                    }
                } else if (Sim.Difficulty == DIFF_TUTORIAL || Sim.Difficulty == DIFF_FREEGAME) {
                    MakeSayWindow(0, TOKEN_BOSS, 700, pFontPartner);
                } else {
                    MakeSayWindow(0, TOKEN_BOSS, 1000 + Sim.Difficulty * 100, pFontPartner);
                }
            } else {
                if (Sim.Overtake != 0) {
                    MakeSayWindow(0, TOKEN_BOSS, 5000, pFontPartner);
                } else if (Sim.SabotageActs.AnzEntries() > 0) {
                    MakeSayWindow(0, TOKEN_BOSS, 2004, pFontPartner);
                } else {
                    MakeSayWindow(0, TOKEN_BOSS, 2000 + rand() % 4, pFontPartner);
                }
            }
        } else {
            MakeSayWindow(0, TOKEN_BOSS, 4000 + DialogMedium, pFontPartner);
        }
        break;

    case TALKER_MAKLER:
        pFontPartner = &FontDialogPartner;

        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_MAKLER]++;
            if (DialogMedium == MEDIUM_HANDY && (Sim.Players.Players[PlayerNum].DialogWin != nullptr)) {
                pSmackerPartner = &(dynamic_cast<CMakler *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Makler;
                (Sim.Players.Players[PlayerNum].DialogWin)->HandyOffset = 378;
            }

            BubblePos = XY(540, 200);
            BubbleStyle = 2;
        } else {
            pSmackerPartner = &(dynamic_cast<CMakler *>(this))->SP_Makler;

            BubblePos = XY(540, 200);
            BubbleStyle = 2;
        }

        if (DialogPar1 == -1) {
            if (Medium == MEDIUM_HANDY) {
                MakeSayWindow(0, TOKEN_MAKLER, 82, pFontPartner);
            } else if (TalkedToA != 0) {
                MakeSayWindow(0, TOKEN_MAKLER, 83, pFontPartner);
            } else if ((Sim.DialogOvertureFlags & DIALOG_MAKLER) != 0U) {
                MakeSayWindow(0, TOKEN_MAKLER, 81, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_MAKLER, 80, pFontPartner);
            }

            Sim.DialogOvertureFlags |= DIALOG_MAKLER;
            TalkedToA = TRUE;
        } else if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_MAKLER, DialogPar1, pFontPartner);
        } else {
            if (Sim.Players.Players[PlayerNum].Money - PlaneTypes[DialogPar2].Preis < DEBT_LIMIT) {
                MakeSayWindow(0, TOKEN_MAKLER, 6000, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_MAKLER, 130 + rand() % 4, pFontPartner);
            }
        }
        break;

    case TALKER_MECHANIKER:
        pFontPartner = &FontDialogPartner;

        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_WERKSTATT]++;
            if (Sim.Players.Players[PlayerNum].DialogWin != nullptr) {
                pSmackerPartner = &(dynamic_cast<CWerkstatt *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Mann;
                (Sim.Players.Players[PlayerNum].DialogWin)->HandyOffset = 267 - 136;
            }

            BubblePos = XY(445, 199);
            BubbleStyle = 2; // Von Süden
        } else {
            pSmackerPartner = &(dynamic_cast<CWerkstatt *>(this))->SP_Mann;

            BubblePos = XY(200, 203);
            BubbleStyle = 2; // Von Süden
        }

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_MECH, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 1) {
            if (Sim.Players.Players[PlayerNum].MechAngry == 2) {
                if (DialogMedium == MEDIUM_AIR) {
                    MakeSayWindow(0, TOKEN_MECH, 5002, pFontPartner);
                } else {
                    MakeSayWindow(0, TOKEN_MECH, 5003, pFontPartner);
                }
                // if (DialogMedium==MEDIUM_AIR) Sim.Players.Players[(SLONG)PlayerNum].LeaveRoom();
            } else if (Medium != 0) {
                MakeSayWindow(0, TOKEN_MECH, 1001, pFontPartner);
            } else if (TalkedToA != 0) {
                MakeSayWindow(0, TOKEN_MECH, 1002, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_MECH, 1000, pFontPartner);
            }
        } else if (DialogPar1 == 2) {
            MakeSayWindow(0, TOKEN_MECH, 10000, pFontPartner);
        } else if (DialogPar1 == 20) {
            MakeSayWindow(0, TOKEN_MECH, 1020, pFontPartner);
        } else if (DialogPar1 == 21) {
            MakeSayWindow(0, TOKEN_MECH, 1021, pFontPartner);
        }
        TalkedToA = TRUE;
        break;

    case TALKER_MUSEUM:
        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_MUSEUM]++;
            if (Sim.Players.Players[PlayerNum].DialogWin != nullptr) {
                pSmackerPartner = &(dynamic_cast<CMuseum *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Mann;
                (Sim.Players.Players[PlayerNum].DialogWin)->HandyOffset = 367;
            }

            BubblePos = XY(440, 234);
            BubbleStyle = 2; // Von Süden
        } else {
            pSmackerPartner = &(dynamic_cast<CMuseum *>(this))->SP_Mann;

            BubblePos = XY(440, 234);
            BubbleStyle = 2; // Von Süden
        }

        pFontPartner = &FontDialogPartner;

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_MUSEUM, DialogPar1, pFontPartner);
        } else if (DialogPar1 == -1) {
            if (DialogMedium == MEDIUM_HANDY) {
                MakeSayWindow(0, TOKEN_MUSEUM, 101, pFontPartner);
            } else if ((Sim.DialogOvertureFlags & DIALOG_MUSEUM) != 0U) {
                MakeSayWindow(0, TOKEN_MUSEUM, 102, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_MUSEUM, 100, pFontPartner);
            }

            Sim.DialogOvertureFlags |= DIALOG_MUSEUM;
        } else if (DialogPar1 == 100) {
            MakeSayWindow(0, TOKEN_MUSEUM, 710, pFontPartner);
            MakeNumberWindow(TOKEN_MUSEUM, 9990710, (LPCTSTR)Insert1000erDots(Sim.Players.Players[PlayerNum].Planes[DialogPar2].CalculatePrice() * 9 / 10));
        } else if (DialogPar1 == 101) {
            MakeSayWindow(0, TOKEN_MUSEUM, 711, pFontPartner);
            MakeNumberWindow(TOKEN_MUSEUM, 9990711, (LPCTSTR)Insert1000erDots(Sim.Players.Players[PlayerNum].Planes[DialogPar2].CalculatePrice() * 9 / 10));
        } else if (DialogPar1 == 102) {
            MakeSayWindow(0, TOKEN_MUSEUM, 712, pFontPartner);
            MakeNumberWindow(TOKEN_MUSEUM, 9990712, (LPCTSTR)Insert1000erDots(Sim.Players.Players[PlayerNum].Planes[DialogPar2].CalculatePrice() * 9 / 10));
        } else if (DialogPar1 == 103) {
            MakeSayWindow(0, TOKEN_MUSEUM, 713, pFontPartner);
            MakeNumberWindow(TOKEN_MUSEUM, 9990713, (LPCTSTR)Insert1000erDots(Sim.Players.Players[PlayerNum].Planes[DialogPar2].CalculatePrice() * 9 / 10));
        } else {
            MakeSayWindow(0, TOKEN_MUSEUM, 500, pFontPartner, (LPCTSTR)Insert1000erDots(Sim.UsedPlanes[0x1000000 + DialogPar1].CalculatePrice()));
        }
        break;

    case TALKER_PERSONAL1a:
    case TALKER_PERSONAL2a:
    case TALKER_PERSONAL3a:
    case TALKER_PERSONAL4a:
        if (Medium == MEDIUM_HANDY) {
            if (Sim.Players.Players[PlayerNum].DialogWin != nullptr) {
                pSmackerPartner = &(dynamic_cast<CPersonal *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Frau;
                (Sim.Players.Players[PlayerNum].DialogWin)->HandyOffset = 320;
            }

            BubblePos = XY(535, 207);
            BubbleStyle = 1;
        } else {
            pSmackerPartner = &(dynamic_cast<CPersonal *>(this))->SP_Frau;

            BubblePos = XY(443, 208);
            BubbleStyle = 1;
        }
        pFontPartner = &FontDialogPartner;

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_JOBS, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 20) {
            if (Sim.Players.Players[PlayerNum].SeligTrust != 0) {
                MakeSayWindow(0, TOKEN_JOBS, 301, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_JOBS, 302, pFontPartner);
            }
        } else if (DialogPar1 == 300) {
            MakeSayWindow(0, TOKEN_JOBS, 300, pFontPartner);
        } else if (Medium != 0) {
            MakeSayWindow(0, TOKEN_JOBS, 101, pFontPartner);
            MakeNumberWindow(TOKEN_JOBS, 9990101, Workers.GetNumJoblessBerater(), Workers.GetNumJoblessPiloten(), Workers.GetNumJoblessFlugbegleiter());
        } else if (TalkedToA != 0) {
            MakeSayWindow(0, TOKEN_JOBS, 102, pFontPartner);
        } else {
            MakeSayWindow(1, TOKEN_JOBS, 99, 99, FALSE, &FontDialog, &FontDialogLight);
        }
        TalkedToA = TRUE;
        break;

    case TALKER_PERSONAL1b:
    case TALKER_PERSONAL2b:
    case TALKER_PERSONAL3b:
    case TALKER_PERSONAL4b:
        if (Medium == MEDIUM_HANDY) {
            if (Sim.Players.Players[PlayerNum].DialogWin != nullptr) {
                pSmackerPartner = &(dynamic_cast<CPersonal *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Mann;
                (Sim.Players.Players[PlayerNum].DialogWin)->HandyOffset = 160;
            }

            BubblePos = XY(435, 124);
            BubbleStyle = 1;
        } else {
            pSmackerPartner = &(dynamic_cast<CPersonal *>(this))->SP_Mann;

            BubblePos = XY(280, 144);
            BubbleStyle = 3;
        }
        pFontPartner = &FontDialogPartner;

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_JOBS, DialogPar1, pFontPartner);
        } else if (DialogPar1 >= 1000 && DialogPar1 <= 1010) {
            MakeSayWindow(0, TOKEN_JOBS, DialogPar1, pFontPartner);
        } else if ((Sim.Players.Players[PlayerNum].StrikeHours != 0) && Sim.Players.Players[PlayerNum].StrikeEndCountdown == 0) {
            MakeSayWindow(0, TOKEN_JOBS, 950, pFontPartner);
        } else if (Medium != 0) {
            MakeSayWindow(0, TOKEN_JOBS, 501, pFontPartner);
        } else if (TalkedToB != 0) {
            MakeSayWindow(0, TOKEN_JOBS, 503, pFontPartner);
        } else {
            MakeSayWindow(1, TOKEN_JOBS, 499, 499, FALSE, &FontDialog, &FontDialogLight);
        }
        TalkedToB = TRUE;
        break;

    case TALKER_WERBUNG:
        pFontPartner = &FontDialogPartner;

        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_WERBUNG]++;
            if (DialogMedium == MEDIUM_HANDY && (Sim.Players.Players[PlayerNum].DialogWin != nullptr)) {
                pSmackerPartner = &(dynamic_cast<CWerbung *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Mann;
            }

            BubblePos = XY(526, 300);
            BubbleStyle = 1;
        } else {
            pSmackerPartner = &(dynamic_cast<CWerbung *>(this))->SP_Mann;

            BubblePos = XY(400, 250);
            BubbleStyle = 2;
        }

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_WERBUNG, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 8001) {
            MakeSayWindow(0, TOKEN_WERBUNG, 8001, pFontPartner);
        } else if (DialogPar1 == 800) {
            if (Sim.Players.Players[PlayerNum].WerbungTrust != 0) {
                MakeSayWindow(0, TOKEN_WERBUNG, 8002, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_WERBUNG, 8000, pFontPartner);
            }
        } else if (DialogPar1 == 3501) {
            MakeSayWindow(0, TOKEN_WERBUNG, 3501, pFontPartner);
        } else if (Medium == MEDIUM_HANDY) {
            MakeSayWindow(0, TOKEN_WERBUNG, 1001, pFontPartner);
        } else if (TalkedToA != 0) {
            MakeSayWindow(0, TOKEN_WERBUNG, 1003, pFontPartner);
        } else if ((Sim.DialogOvertureFlags & DIALOG_WERBUNG) != 0U) {
            MakeSayWindow(0, TOKEN_WERBUNG, 1002, pFontPartner);
        } else {
            MakeSayWindow(0, TOKEN_WERBUNG, 1000, pFontPartner);
        }
        Sim.DialogOvertureFlags |= DIALOG_WERBUNG;
        TalkedToA = TRUE;
        break;

    case TALKER_NASA:
        pFontPartner = &FontDialogPartner;

        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_NASA]++;
            if (DialogMedium == MEDIUM_HANDY && (Sim.Players.Players[PlayerNum].DialogWin != nullptr)) {
                pSmackerPartner = &(dynamic_cast<CNasa *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Mann;
            }

            BubblePos = XY(394 - 44, 160 - 86 + 218);
            BubbleStyle = 1;
        } else {
            pSmackerPartner = &(dynamic_cast<CNasa *>(this))->SP_Mann;

            BubblePos = XY(394, 160 - 86 + 40);
            BubbleStyle = 2;
        }

        if (DialogPar1 == 0) {
            MakeSayWindow(0, TOKEN_NASA, 500, pFontPartner);
        } else if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_NASA, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 1) {
            if (Medium == MEDIUM_HANDY) {
                MakeSayWindow(0, TOKEN_NASA, 1001, pFontPartner);
            } else if (TalkedToA != 0) {
                MakeSayWindow(0, TOKEN_NASA, 1003, pFontPartner);
            } else if ((Sim.DialogOvertureFlags & DIALOG_NASA) != 0U) {
                MakeSayWindow(0, TOKEN_NASA, 1002, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_NASA, 1000, pFontPartner);
            }
            Sim.DialogOvertureFlags |= DIALOG_NASA;
            TalkedToA = TRUE;
        } else if (DialogPar1 == 2) {
            MakeSayWindow(0, TOKEN_NASA, 9999, pFontPartner);
        }
        break;

    case TALKER_DUTYFREE:
        pFontPartner = &FontDialogPartner;

        if (Medium == MEDIUM_HANDY) {
            Sim.RoomBusy[ROOM_SHOP1]++;
            if (DialogMedium == MEDIUM_HANDY && (Sim.Players.Players[PlayerNum].DialogWin != nullptr)) {
                pSmackerPartner = &(dynamic_cast<CDutyFree *>(Sim.Players.Players[PlayerNum].DialogWin))->SP_Frau;
            }

            BubblePos = XY(396, 198);
            BubbleStyle = 1;
        } else {
            pSmackerPartner = &(dynamic_cast<CDutyFree *>(this))->SP_Frau;

            BubblePos = XY(396, 128);
            BubbleStyle = 1;
        }

        if (DialogPar1 == 3000) {
            MakeSayWindow(0, TOKEN_DUTYFREE, 3000, pFontPartner);
        } else if (DialogPar1 == 2000) {
            MakeSayWindow(1, TOKEN_DUTYFREE, 2000, 2000, TRUE, &FontDialog, &FontDialogLight);
        } else if (DialogPar1 == 800) {
            if (Sim.Players.Players[PlayerNum].DutyTrust != 0) {
                MakeSayWindow(0, TOKEN_DUTYFREE, 802, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_DUTYFREE, 800, pFontPartner);
            }
        } else if (DialogPar1 == 801) {
            MakeSayWindow(0, TOKEN_DUTYFREE, 801, pFontPartner);
        } else if (DialogPar1 == 2010 || DialogPar1 == 2020) {
            MakeSayWindow(0, TOKEN_DUTYFREE, DialogPar1, pFontPartner);
        } else if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_DUTYFREE, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 3) {
            if (Sim.Date < 7) {
                MakeSayWindow(0, 100, DialogTexte.GetS(TOKEN_DUTYFREE, 97), pFontPartner);
                MakeNumberWindow(TOKEN_DUTYFREE, 9990097, 8 - Sim.Date);
            } else {
                MakeSayWindow(0, TOKEN_DUTYFREE, 98, pFontPartner);
            }
        } else if (DialogPar1 == 1) {
            if (Sim.localPlayer != DialogPar2) {
                MakeSayWindow(0, 100, bprintf(DialogTexte.GetS(TOKEN_DUTYFREE, 100), (LPCTSTR)Sim.Players.Players[DialogPar2].AirlineX), pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_DUTYFREE, 99, pFontPartner);
            }
        } else if (DialogPar1 == 2) {
            MakeSayWindow(0, TOKEN_DUTYFREE, 1000, pFontPartner);
        } else if (DialogPar1 == 4) {
            MakeSayWindow(0, TOKEN_DUTYFREE, 1010, pFontPartner);
        }
        break;

    case TALKER_KIOSK:
        pFontPartner = &FontDialogPartner;
        BubblePos = XY(288, 192);
        BubbleStyle = 2;

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_KIOSK, DialogPar1, pFontPartner);
        } else if (DialogPar1 == 1000) {
            if (Sim.Players.Players[PlayerNum].KioskTrust != 0) {
                MakeSayWindow(0, TOKEN_KIOSK, 1021, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_KIOSK, 1020, pFontPartner);
            }
        } else if (DialogPar1 == 1010) {
            MakeSayWindow(0, TOKEN_KIOSK, 1010, pFontPartner);
        } else {
            MakeSayWindow(0, TOKEN_KIOSK, 1000, pFontPartner);
        }
        break;

    case TALKER_PASSENGER:
        pFontPartner = &FontDialogPartner;
        BubblePos = XY(DialogPar2, DialogPar3);
        BubbleStyle = 2;

        Sim.Persons[DialogPar1].State |= PERSON_TALKING;

        MakeSayWindow(1, TOKEN_PASSENGER, 100, 100, FALSE, &FontDialog, &FontDialogLight);
        break;

    case TALKER_WORLD:
        pFontPartner = &FontDialogPartner;

        delete Sim.Players.Players[PlayerNum].DialogWin;
        Sim.Players.Players[PlayerNum].DialogWin = new CWorld(TRUE, PlayerNum, MouseClickPar2);

        BubblePos = (Sim.Players.Players[PlayerNum].DialogWin)->BubblePos;
        BubbleStyle = (Sim.Players.Players[PlayerNum].DialogWin)->BubbleStyle;

        if (Sim.Players.Players[PlayerNum].CalledCities[MouseClickPar2] == 0) {
            // Haben wir ein Sample dafür?
            if (Cities[DialogPar2].Wave != "-" && !Cities[DialogPar2].Wave.empty()) {
                MakeSayWindow(0, TOKEN_WELT, 1000, pFontPartner, (LPCTSTR)Sim.Players.Players[DialogPar1].AirlineX, (LPCTSTR)Cities[DialogPar2].Name);
            } else {
                MakeSayWindow(1, TOKEN_WELT, 2000, 2000, FALSE, &FontDialog, &FontDialogLight, (LPCTSTR)Sim.Players.Players[DialogPar1].AirlineX);
            }
        } else {
            PayingForCall = TRUE;
            DialogPartner = TALKER_WORLD;
            MenuStart(MENU_AUSLANDSAUFTRAG, DialogPar2);
            MenuSetZoomStuff(XY(480, 220), 0.1, FALSE);
        }
        break;

    case TALKER_RICK:
        pFontPartner = &FontDialogPartner;

        pSmackerPartner = &(dynamic_cast<CRicks *>(this))->SP_Rick;

        BubblePos = XY(540 - 320, 200 - 95);
        BubbleStyle = 2;

        if (DialogPar1 >= 10000) {
            MakeSayWindow(0, TOKEN_RICK, DialogPar1, pFontPartner);
        } else {
            if ((Sim.DialogOvertureFlags & DIALOG_RICK) != 0U) {
                if ((Sim.DialogOvertureFlags & DIALOG_RICK_TODAY) != 0U) {
                    MakeSayWindow(0, TOKEN_RICK, 3000, pFontPartner);
                } else {
                    MakeSayWindow(1, TOKEN_RICK, 2000, 2000, TRUE, &FontDialog, &FontDialogLight);
                }
            } else {
                MakeSayWindow(0, TOKEN_RICK, 1000, pFontPartner);
            }

            Sim.DialogOvertureFlags |= (DIALOG_RICK | DIALOG_RICK_TODAY);
        }
        break;

    case TALKER_TRINKER:
        pFontPartner = &FontDialogPartner;

        pSmackerPartner = &(dynamic_cast<CRicks *>(this))->SP_TrinkerMund;

        BubblePos = XY(540 - 60, 200 - 87);
        BubbleStyle = 2;

        if (DialogPar1 == 800) {
            MakeSayWindow(0, TOKEN_RICK, 800, pFontPartner);
        } else if ((Sim.Players.Players[Sim.localPlayer].TrinkerTrust != 0) && (Sim.Players.Players[Sim.localPlayer].StrikeHours != 0)) {
            if ((Sim.Players.Players[Sim.localPlayer].StrikeEndCountdown != 0) && Sim.Players.Players[Sim.localPlayer].StrikeEndType == 3) {
                MakeSayWindow(0, TOKEN_RICK, 6001, pFontPartner);
            } else {
                MakeSayWindow(0, TOKEN_RICK, 6000, pFontPartner);

                Sim.Players.Players[Sim.localPlayer].StrikeEndCountdown = 10;
                Sim.Players.Players[Sim.localPlayer].StrikeEndType = 3;
            }
        } else {
            MakeSayWindow(0, TOKEN_RICK, 5000 + rand() % 9, pFontPartner);
        }
        break;

    case TALKER_COMPETITOR:
        pFontPartner = &FontDialogPartner;
        Sim.Players.Players[DialogPar1].IsTalking = TRUE;

        BubblePos = XY(640 - BeraterSprechblasenOffset[12 + DialogPar1].x, 380);
        BubbleStyle = 1;

        if (DialogPar2 < 0 || DialogPar2 > 1) {
            DialogPar2 = 0;
        }
        if (DialogPar2 == 0) {
            MakeSayWindow(0, TOKEN_PLAYER, 1000 + static_cast<SLONG>(DialogMedium == MEDIUM_AIR), pFontPartner,
                          (LPCTSTR)Sim.Players.Players[DialogPar1].AirlineX);
        } else {
            MakeSayWindow(1, TOKEN_PLAYER, 1000 + static_cast<SLONG>(DialogMedium == MEDIUM_AIR), 1000 + static_cast<SLONG>(DialogMedium == MEDIUM_AIR), FALSE,
                          &FontDialog, &FontDialogLight, (LPCTSTR)Sim.Players.Players[Sim.localPlayer].AirlineX);
        }
        break;
    default:
        hprintf("StdRaum.cpp: Default case should not be reached.");
        DebugBreak();
    }
}

//--------------------------------------------------------------------------------------------
// Stoppt den Dialog:
//--------------------------------------------------------------------------------------------
void CStdRaum::StopDialog() {
    SLONG LastDialogPartner = DialogPartner;

    if (IsDialogOpen() != 0) {
        if (LastMenu == MENU_AUSLANDSAUFTRAG && DialogPartner == TALKER_WORLD) {
            Sim.Players.Players[Sim.localPlayer].CalledCities[DialogPar2] = 1; // new
        }

        if (DialogPartner == TALKER_COMPETITOR && Sim.Players.Players[DialogPar1].Owner == 2) {
            DialogPartner = TALKER_NONE;

            Sim.Players.Players[Sim.localPlayer].IsTalking = FALSE;
            Sim.Players.Players[DialogPar1].IsTalking = FALSE;
            SIM::SendSimpleMessage(ATNET_DIALOG_END, Sim.Players.Players[DialogPar1].NetworkID);
        }

        bHandy = FALSE;
        UsingHandy = FALSE;
        Ferngespraech = FALSE;

        if (TalkingSpeechFx != 0) {
            SpeechFx.Stop();
            TalkingSpeechFx = FALSE;
        }

        if (DialogMedium == MEDIUM_HANDY) {
            SLONG room = 0;

            switch (DialogPartner) {
            case TALKER_ARAB:
                room = ROOM_ARAB_AIR;
                break;
            case TALKER_BANKER1:
                room = ROOM_BANK;
                break;
            case TALKER_BANKER2:
                room = ROOM_BANK;
                break;
            case TALKER_BOSS:
                room = ROOM_AUFSICHT;
                break;
            case TALKER_MAKLER:
                room = ROOM_MAKLER;
                break;
            case TALKER_MECHANIKER:
                room = ROOM_WERKSTATT;
                break;
            case TALKER_MUSEUM:
                room = ROOM_MUSEUM;
                break;
            case TALKER_NASA:
                room = ROOM_NASA;
                break;
            case TALKER_DUTYFREE:
                room = ROOM_SHOP1;
                break;
            case TALKER_SABOTAGE:
                room = ROOM_SABOTAGE;
                break;
            case TALKER_WERBUNG:
                room = ROOM_WERBUNG;
                break;
            default:
                break;
            }

            if ((room != 0) && Sim.RoomBusy[room] > 0) {
                Sim.RoomBusy[room]--;
            }
        }

        if (DialogPartner == TALKER_COMPETITOR) {
            Sim.Players.Players[DialogPar1].IsTalking = FALSE;
            SIM::SendSimpleMessage(ATNET_DIALOG_UNLOCK, 0, Sim.localPlayer);
            SIM::SendSimpleMessage(ATNET_DIALOG_UNLOCK, 0, DialogPar1);
            // Sim.Players.Players[(SLONG)PlayerNum].BroadcastPosition();
        }

        Sim.Players.Players[PlayerNum].Messages.StopDialog();

        if (pSmackerPartner != nullptr) {
            pSmackerPartner->SetDesiredMood(SPM_IDLE);
        }
        pSmackerPartner = nullptr;
        pSmackerPlayer = nullptr;

        if (DialogPartner == TALKER_NASA && CurrentTextSubIdVon == 9999) {
            Sim.Players.Players[PlayerNum].LeaveRoom();
        }

        if (DialogPartner == TALKER_PASSENGER) {
            Sim.Persons[DialogPar1].State &= (~PERSON_TALKING);
        }

        RightAirportClip = 640;

        // Bei Handy, den Raum wieder rauswerfen:
        if (DialogMedium == MEDIUM_HANDY && (Sim.Players.Players[PlayerNum].DialogWin != nullptr)) {
            (Sim.Players.Players[PlayerNum].DialogWin)->TempScreenScrollV = 1;
        }

        if (IsInBuro != 0) {
            (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_IDLE);
        }

        if (DialogPartner != TALKER_NONE) {
            Talkers.Talkers[DialogPartner].StopDialog();
        }
        DialogPartner = TALKER_NONE;

        CloseTextWindow();
        CurrentTextGroupId = 0;
        DialogPartner = TALKER_NONE;

        if (LastDialogPartner >= TALKER_PERSONAL1a && LastDialogPartner <= TALKER_PERSONAL4b) {
            Sim.Players.Players[Sim.localPlayer].MapWorkerOverflow(TRUE);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Bereitet eine neue Runde mit den Tooltips vor:
//--------------------------------------------------------------------------------------------
void CStdRaum::InitToolTips() {
    SLONG c = 0;
    XY CursorPos = gMousePosition; // Sim.Players.Players[(SLONG)PlayerNum].CursorPos;

    gHighlightFontColor = 0;

    if (CalculatorIsOpen != 0) {
        return;
    }

    if (bHandy == FALSE) {
        SetMouseLook(CURSOR_NORMAL, 0, -100, 0);

        // Onscreen-Menüs:
        if (CurrentMenu != MENU_NONE && (CalculatorIsOpen == 0)) {
            CursorPos -= MenuPos;

            switch (CurrentMenu) {
            case MENU_PERSONAL:
                if (CursorPos.IfIsWithin(0, 5, 35, 330) && MenuPage > 0) {
                    SetMouseLook(CURSOR_LEFT, 0, -100, 1);
                }

                if (CursorPos.IfIsWithin(230, 292, 260, 330) && MenuPage < MenuPageMax) {
                    SetMouseLook(CURSOR_RIGHT, 0, -100, 2);
                }
                break;

            case MENU_LETTERS:
                if (CursorPos.IfIsWithin(6, 332, 56, 389) && MenuPage > 0) {
                    SetMouseLook(CURSOR_LEFT, 4000, -100, 1);
                }
                if (CursorPos.IfIsWithin(244, 335, 298, 389) && MenuPage < Sim.Players.Players[PlayerNum].Letters.AnzLetters() - 1) {
                    SetMouseLook(CURSOR_RIGHT, 4002, -100, 3);
                }

                if (CursorPos.IfIsWithin(131, 358, 150, 376)) {
                    SetMouseLook(CURSOR_HOT, 4004, -100, 10);
                }
                if (CursorPos.IfIsWithin(162, 359, 182, 376)) {
                    SetMouseLook(CURSOR_HOT, 4001, -100, 11);
                }
                break;
            default:
                break;
            }

            CursorPos += MenuPos;
        }

        // Die ToolTips für die Statuszeile: Konto, Logo, Speed, Zeit
        if (TopWin == nullptr) {
            if (CursorPos.IfIsWithin(80, 460, 242, 479)) {
                SetMouseLook(CURSOR_HOT, 1002, -100, 1002);
            } else if (CursorPos.IfIsWithin(0, 440, 79, 479)) {
                SetMouseLook(CURSOR_HOT, 2102, -100, 1000);
            } else if (CursorPos.IfIsWithin(80, 440, 102, 459)) {
                SetMouseLook(CURSOR_HOT, 1003, -100, 1003);
            } else if (CursorPos.IfIsWithin(102, 440, 242, 459)) {
                SetMouseLook(CURSOR_HOT, 1001, -100, 1001);

                // Inventar
            } else if (CursorPos.IfIsWithin(250, 440, 639 - 8 - 24, 479)) {
                SLONG ItemIndex = (CursorPos.x - 250) / 60;

                if (ItemIndex < 6 && Sim.Players.Players[PlayerNum].Items.AnzEntries() > ItemIndex) {
                    SLONG Item = 2000 + Sim.Players.Players[PlayerNum].Items[ItemIndex];

                    // Klick ins Inventar erfolgreich?
                    if (Sim.Players.Players[PlayerNum].Items[ItemIndex] != 0xff) {
                        SetMouseLook(CURSOR_HOT, Item, -100, Item);
                    } else {
                        SetMouseLook(CURSOR_NORMAL, 0, -100, 0);
                    }
                }
            }
        }

        // Ist der Cursor irgenwo in einem Menü?
        if (MenuIsOpen() != 0) {
            CursorPos -= MenuPos;

            switch (CurrentMenu) {
            case MENU_FILOFAX:
                if (CursorPos.IfIsWithin(32, 4, 204, 4 + 18 * 13)) {
                    c = (CursorPos.y - 4) / 13;

                    if (MenuPar1 == 1 && c < 18) // Filofax
                    {
                        if (Sim.Players.Players[PlayerNum].WasInRoom[static_cast<SLONG>(FilofaxRoomRemapper[c])] != 0) {
                            if (Sim.Players.Players[PlayerNum].IsLocationInQueue(FilofaxRoomRemapper[c]) == 0) {
                                CheckCursorHighlight(CursorPos, CRect(32, (CursorPos.y - 4) / 13 * 13 + 4, 204, (CursorPos.y - 4) / 13 * 13 + 4 + 15),
                                                     ColorOfFontBlack);
                                SetMouseLook(CURSOR_HOT, 0, -101, MENU_FILOFAX, CursorPos.x, c);
                            }
                        }
                    } else if (MenuPar1 == 2 && c < 14) // Handy
                    {
                        if (HandyRoomRemapper[c] != 0) {
                            if (Sim.Players.Players[PlayerNum].WasInRoom[static_cast<SLONG>(HandyRoomRemapper[c])] != 0) {
                                if (Sim.Players.Players[PlayerNum].IsLocationInQueue(HandyRoomRemapper[c]) == 0) {
                                    CheckCursorHighlight(CursorPos, CRect(32, (CursorPos.y - 4) / 13 * 13 + 4, 204, (CursorPos.y - 4) / 13 * 13 + 4 + 15),
                                                         ColorOfFontBlack);
                                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_FILOFAX, CursorPos.x, c);
                                }
                            }
                        }
                    }
                }
                break;

            case MENU_KEROSIN:
                if (Sim.Players.Players[PlayerNum].TankInhalt != 0) {
                    CheckCursorHighlight(CursorPos, CRect(18, 30 + 68, 300, 30 + 88 + 15), ColorOfFontBlack, CURSOR_HOT, 0, -102, MENU_KEROSIN, -20);
                }

                CheckCursorHighlight(CursorPos, CRect(18, 30 + 98 + 13, 300, 30 + 130 + 13), ColorOfFontBlack, CURSOR_HOT, 0, -102, MENU_KEROSIN, -99);
                break;

            case MENU_BUYKEROSIN:
                if (CursorPos.IfIsWithin(237, 125, 304, 156)) {
                    SetMouseLook(CURSOR_HOT, 4510, -102, MENU_BUYKEROSIN, 10);
                } else if (CursorPos.IfIsWithin(153, 133, 208, 159)) {
                    SetMouseLook(CURSOR_HOT, 4511, -102, MENU_BUYKEROSIN, 11);
                }

                CheckCursorHighlight(CursorPos, CRect(126, 87, 223, 100), ColorOfFontBlack, CURSOR_HOT, 0, -102, MENU_BUYKEROSIN, -20);
                CheckCursorHighlight(CursorPos, CRect(223, 87, 254, 100), ColorOfFontBlack, CURSOR_HOT, 0, -102, MENU_BUYKEROSIN, -21);
                CheckCursorHighlight(CursorPos, CRect(254, 87, 327, 100), ColorOfFontBlack, CURSOR_HOT, 0, -102, MENU_BUYKEROSIN, -22);
                break;

            case MENU_PLAYERLIST:
                if (CursorPos.IfIsWithin(32, 4, 204, 4 + 15 * 13)) {
                    c = (CursorPos.y - 4) / 13;

                    if (c >= 1 && c <= 3 && (Sim.Players.Players[c - 1 + static_cast<SLONG>((c - 1) >= PlayerNum)].IsOut == 0)) {
                        CheckCursorHighlight(CursorPos, CRect(32, (CursorPos.y - 4) / 13 * 13 + 4, 204, (CursorPos.y - 4) / 13 * 13 + 4 + 15),
                                             ColorOfFontBlack);
                        SetMouseLook(CURSOR_HOT, 0, -101, MENU_PLAYERLIST, CursorPos.x, c - 1 + static_cast<SLONG>((c - 1) >= PlayerNum));
                    }

                    if (c >= 6 && c <= 8 && (Sim.Players.Players[c - 6 + static_cast<SLONG>((c - 6) >= PlayerNum)].IsOut == 0) &&
                        (Sim.Players.Players[c - 6 + static_cast<SLONG>((c - 6) >= PlayerNum)].HasItem(ITEM_HANDY) != 0)) {
                        CheckCursorHighlight(CursorPos, CRect(32, (CursorPos.y - 4) / 13 * 13 + 4, 204, (CursorPos.y - 4) / 13 * 13 + 4 + 15),
                                             ColorOfFontBlack);
                        SetMouseLook(CURSOR_HOT, 0, -101, MENU_PLAYERLIST, CursorPos.x, c - 6 + static_cast<SLONG>((c - 6) >= PlayerNum) + 4);
                    }
                }
                if (CursorPos.IfIsWithin(12, 187, 36, 208)) {
                    SetMouseLook(CURSOR_LEFT, 0, -103, MENU_BRANCHLIST, -1);
                }
                break;

            case MENU_BRANCHLIST:
                if (CursorPos.IfIsWithin(32, 4, 204, 4 + 15 * 13)) {
                    c = MenuPage * 15 + (CursorPos.y - 4) / 13;
                    if (c >= 0 && c < MenuRemapper.AnzEntries()) {
                        CheckCursorHighlight(CursorPos, CRect(32, (CursorPos.y - 4) / 13 * 13 + 4, 204, (CursorPos.y - 4) / 13 * 13 + 4 + 15),
                                             ColorOfFontBlack);
                        SetMouseLook(CURSOR_HOT, 0, -101, MENU_BRANCHLIST, CursorPos.x, MenuRemapper[c]);
                    }
                }
                if (CursorPos.IfIsWithin(12, 187, 36, 208)) {
                    SetMouseLook(CURSOR_LEFT, 0, -103, MENU_BRANCHLIST, -1);
                }
                if (MenuPage < SLONG((MenuRemapper.AnzEntries() - 1) / 15) && CursorPos.IfIsWithin(190, 196, 203, 209)) {
                    SetMouseLook(CURSOR_RIGHT, 0, -103, MENU_BRANCHLIST, -2);
                }
                break;

            case MENU_AUSLANDSAUFTRAG:
                c = (CursorPos.y - 7 - 26) / 13;
                if (CursorPos.y - 7 - 26 < 0) {
                    c = -1;
                }

                if ((CursorPos.y - 7 - 26 >= 0 && CursorPos.x >= 216 && CursorPos.x <= 387 &&
                     (c >= 0 && c < 6 && (AuslandsAuftraege[MenuPar1][c].Praemie != 0))) ||
                    (c >= 7 && c < 7 + 6 && (AuslandsFrachten[MenuPar1][c - 7].Praemie != 0)) || c == 7 + 7) {
                    CheckCursorHighlight(CursorPos, CRect(32 + 180, c * 13 + 7 + 26, 204 + 180, c * 13 + 7 + 15 + 26), ColorOfFontBlack);
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_AUSLANDSAUFTRAG, CursorPos.x, c);
                }
                if (c != MenuPar2 && c >= 0 && c <= 5 && CursorPos.x >= 216 && CursorPos.x <= 387 && (AuslandsAuftraege[MenuPar1][c].Praemie != 0)) {
                    MenuRepaint();
                    Sim.Players.Players[PlayerNum].CheckAuftragsBerater(AuslandsAuftraege[MenuPar1][c]);

                    MenuPar2 = c;
                } else if (c != MenuPar2 && c >= 7 && c < 7 + 6 && CursorPos.x >= 216 && CursorPos.x <= 387 &&
                           (AuslandsFrachten[MenuPar1][c - 7].Praemie != 0)) {
                    MenuRepaint();
                    Sim.Players.Players[PlayerNum].CheckAuftragsBerater(AuslandsFrachten[MenuPar1][c - 7]);

                    MenuPar2 = c;
                } else if (c < 0 || c > 5) {
                    MenuPar2 = -1;
                }
                break;

            case MENU_ADROUTE:
                if (CursorPos.IfIsWithin(216, 6, 387, 212)) {
                    SLONG n = (CursorPos.y - 25) / 13 + MenuPage;

                    if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13) {
                        CheckCursorHighlight(CursorPos, CRect(32 + 180, n * 13 + 23, 204 + 180, n * 13 + 23 + 15), ColorOfFontBlack);
                        SetMouseLook(CURSOR_HOT, 0, -102, CurrentMenu, 0);
                    }
                }

                if (CursorPos.IfIsWithin(196, 192, 196 + 16, 192 + 16) && MenuPage > 0) {
                    SetMouseLook(CURSOR_LEFT, 0, -102, CurrentMenu, -1);
                } else if (CursorPos.IfIsWithin(374, 199, 374 + 16, 199 + 16) && MenuPage / 13 < MenuPageMax) {
                    SetMouseLook(CURSOR_RIGHT, 0, -102, CurrentMenu, -2);
                }
                break;

            case MENU_PLANEJOB:
                if (CursorPos.IfIsWithin(216, 6, 387, 212)) {
                    SLONG n = (CursorPos.y - 25) / 13 + MenuPage;

                    if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13) {
                        if (MenuDataTable.ValueFlags[0 + n * MenuDataTable.AnzColums] != 0U) {
                            CheckCursorHighlight(CursorPos, CRect(32 + 180, (n - MenuPage) * 13 + 23, 204 + 180, (n - MenuPage) * 13 + 23 + 14),
                                                 ColorOfFontRed);
                        } else {
                            CheckCursorHighlight(CursorPos, CRect(32 + 180, (n - MenuPage) * 13 + 23, 204 + 180, (n - MenuPage) * 13 + 23 + 14),
                                                 ColorOfFontBlack);
                        }

                        SetMouseLook(CURSOR_HOT, 0, -102, CurrentMenu, 0);
                    }
                }

                if (CursorPos.IfIsWithin(196, 192, 196 + 16, 192 + 16) && MenuPage > 0) {
                    SetMouseLook(CURSOR_LEFT, 0, -102, CurrentMenu, -1);
                } else if (CursorPos.IfIsWithin(374, 199, 374 + 16, 199 + 16) && MenuPage / 13 < MenuPageMax) {
                    SetMouseLook(CURSOR_RIGHT, 0, -102, CurrentMenu, -2);
                }
                break;

            case MENU_KONTOAUSZUG:
                if (CursorPos.IfIsWithin(374, 186, 400, 214) && MenuPage < 9) {
                    SetMouseLook(CURSOR_RIGHT, 0, -102, CurrentMenu, 1);
                }
                if (CursorPos.IfIsWithin(0, 189, 30, 214) && MenuPage > 0 &&
                    Sim.Players.Players[PlayerNum].History.HistoryLine[MenuPage * 10 - 1].Description != "*") {
                    SetMouseLook(CURSOR_LEFT, 0, -102, CurrentMenu, -1);
                }
                break;

            case MENU_PLANECOSTS:
                if (CursorPos.IfIsWithin(196, 192, 196 + 16, 192 + 16) && MenuPage > 0) {
                    SetMouseLook(CURSOR_LEFT, 0, -102, CurrentMenu, -1);
                } else if (CursorPos.IfIsWithin(374, 199, 374 + 16, 199 + 16) && MenuPage / 13 < MenuPageMax) {
                    SetMouseLook(CURSOR_RIGHT, 0, -102, CurrentMenu, -2);
                }
                break;

            case MENU_PLANEREPAIRS:
                if (CursorPos.IfIsWithin(55, 51, 348, 212)) {
                    SLONG n = (CursorPos.y - 51) / 12 + MenuPage;

                    if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13 && n >= MenuPage) {
                        CheckCursorHighlight(CursorPos, CRect(55, (n - MenuPage) * 12 + 53 - 2, 346, (n - MenuPage) * 12 + 53 + 11), ColorOfFontBlack);
                        SetMouseLook(CURSOR_HOT, 0, -102, CurrentMenu, n);
                    }
                }

                if (CursorPos.IfIsWithin(32, 208, 62, 238) && MenuPage > 0) {
                    SetMouseLook(CURSOR_LEFT, 0, -102, CurrentMenu, -1);
                } else if (CursorPos.IfIsWithin(328, 208, 358, 238) && MenuPage / 13 < MenuPageMax) {
                    SetMouseLook(CURSOR_RIGHT, 0, -102, CurrentMenu, -2);
                }

                CheckCursorHighlight(CursorPos, CRect(55, 211, 120, 226), ColorOfFontBlack, CURSOR_HOT, 0, -102, MENU_PLANEREPAIRS, -20);
                CheckCursorHighlight(CursorPos, CRect(214, 211, 346, 226), ColorOfFontBlack, CURSOR_HOT, 0, -102, MENU_PLANEREPAIRS, -21);
                break;

            case MENU_SECURITY: //==>+<==
                if (CursorPos.IfIsWithin(55, 51, 348, 212)) {
                    SLONG n = (CursorPos.y - 51) / 12 + MenuPage;

                    if (n >= 0 && n <= 8) {
                        CheckCursorHighlight(CursorPos, CRect(63, 53 + n * 12, 346, 55 + n * 12 + 11), ColorOfFontBlack);
                        SetMouseLook(CURSOR_HOT, 0, -102, CurrentMenu, -21, n);
                    }
                }

                CheckCursorHighlight(CursorPos, CRect(55, 211, 120, 226), ColorOfFontBlack, CURSOR_HOT, 0, -102, CurrentMenu, -20);
                break;

            case MENU_PANNEN:
                if (CursorPos.IfIsWithin(55, 51, 348, 212) && MenuInfo3 == -1) {
                    SLONG n = (CursorPos.y - 51) / 12 + MenuPage;

                    if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13 && n >= MenuPage) {
                        CheckCursorHighlight(CursorPos, CRect(55, (n - MenuPage) * 12 + 53 - 2, 346, (n - MenuPage) * 12 + 53 + 11), ColorOfFontBlack);
                        SetMouseLook(CURSOR_HOT, 0, -102, CurrentMenu, n);
                    }
                }

                if (MenuInfo3 == -1) {
                    if (CursorPos.IfIsWithin(32, 208, 62, 238) && MenuPage > 0) {
                        SetMouseLook(CURSOR_LEFT, 0, -102, CurrentMenu, -1);
                    } else if (CursorPos.IfIsWithin(328, 208, 358, 238) && MenuPage / 13 < MenuPageMax) {
                        SetMouseLook(CURSOR_RIGHT, 0, -102, CurrentMenu, -2);
                    }
                } else {
                    if (CursorPos.IfIsWithin(32, 208, 62, 238) && MenuInfo3 > 0) {
                        SetMouseLook(CURSOR_LEFT, 0, -102, CurrentMenu, -1);
                    } else if (CursorPos.IfIsWithin(328, 208, 358, 238) && MenuInfo3 < MenuDataTable.AnzRows - 1) {
                        SetMouseLook(CURSOR_RIGHT, 0, -102, CurrentMenu, -2);
                    }
                }

                CheckCursorHighlight(CursorPos, CRect(55, 211, 120, 226), ColorOfFontBlack, CURSOR_HOT, 0, -102, CurrentMenu, -20);
                break;

            case MENU_SABOTAGEPLANE:
            case MENU_SELLPLANE:
                if (CursorPos.IfIsWithin(216, 6, 387, 212)) {
                    SLONG n = (CursorPos.y - 25) / 13 + MenuPage;

                    if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13 &&
                        (Sim.Players.Players[PlayerNum].Planes.IsInAlbum(MenuDataTable.LineIndex[n]) != 0)) {
                        CheckCursorHighlight(CursorPos, CRect(32 + 180, n * 13 + 23, 204 + 180, n * 13 + 23 + 13), ColorOfFontBlack);
                        SetMouseLook(CURSOR_HOT, 0, -102, CurrentMenu, -55);
                    }
                }

                if (CursorPos.IfIsWithin(196, 192, 196 + 16, 192 + 16) && MenuPage > 0) {
                    SetMouseLook(CURSOR_LEFT, 0, -102, CurrentMenu, -1);
                } else if (CursorPos.IfIsWithin(374, 199, 374 + 16, 199 + 16) && MenuPage / 13 < MenuPageMax) {
                    SetMouseLook(CURSOR_RIGHT, 0, -102, CurrentMenu, -2);
                }
                break;

            case MENU_SABOTAGEROUTE:
                if (CursorPos.IfIsWithin(216, 6, 387, 212)) {
                    SLONG n = (CursorPos.y - 25) / 13 + MenuPage;

                    if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13) {
                        CheckCursorHighlight(CursorPos, CRect(32 + 180, n * 13 + 23, 204 + 180, n * 13 + 23 + 13), ColorOfFontBlack);
                        SetMouseLook(CURSOR_HOT, 0, -102, CurrentMenu, -55);
                    }
                }

                if (CursorPos.IfIsWithin(196, 192, 196 + 16, 192 + 16) && MenuPage > 0) {
                    SetMouseLook(CURSOR_LEFT, 0, -102, CurrentMenu, -1);
                } else if (CursorPos.IfIsWithin(374, 199, 374 + 16, 199 + 16) && MenuPage / 13 < MenuPageMax) {
                    SetMouseLook(CURSOR_RIGHT, 0, -102, CurrentMenu, -2);
                }
                break;

            case MENU_BUYPLANE:
            case MENU_BUYXPLANE:
                if (CursorPos.IfIsWithin(216, 6, 387, 212)) {
                    SLONG n = (CursorPos.y - 25) / 13 + MenuPage;

                    if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13 &&
                        (CurrentMenu == MENU_BUYXPLANE || (Sim.Players.Players[PlayerNum].Planes.IsInAlbum(MenuDataTable.LineIndex[n]) != 0))) {
                        CheckCursorHighlight(CursorPos, CRect(32 + 180, n * 13 + 23, 204 + 180, n * 13 + 23 + 15), ColorOfFontBlack);
                        SetMouseLook(CURSOR_HOT, 0, -102, CurrentMenu, 0);
                    }
                }

                if (CursorPos.IfIsWithin(196, 192, 196 + 16, 192 + 16) && MenuPage > 0) {
                    SetMouseLook(CURSOR_LEFT, 0, -102, CurrentMenu, -1);
                } else if (CursorPos.IfIsWithin(374, 199, 374 + 16, 199 + 16) && MenuPage < MenuPageMax) {
                    SetMouseLook(CURSOR_RIGHT, 0, -102, CurrentMenu, -2);
                }
                break;

            case MENU_PERSONAL:
                if (CursorPos.IfIsWithin(0, 5, 35, 330) && MenuPage > 0) {
                    SetMouseLook(CURSOR_LEFT, 0, -103, MENU_PERSONAL, -1);
                } else if (CursorPos.IfIsWithin(230, 292, 260, 330) && MenuPage < MenuPageMax) {
                    SetMouseLook(CURSOR_RIGHT, 0, -103, MENU_PERSONAL, -2);
                } else if (MenuPage > 0) {
                    if (Workers.Workers[MenuRemapper[MenuPage - 1]].Employer == WORKER_JOBLESS) {
                        CheckCursorHighlight(CursorPos, CRect(40, 254 + 22 - 22, 250, 266 + 24 - 22), ColorOfFontBlack, CURSOR_HOT, 0, -103, MENU_PERSONAL, -3);
                        // if (CursorPos.IfIsWithin (40, 254+22, 250, 266+26)) SetMouseLook (CURSOR_HOT, 0, -103, MENU_PERSONAL, -3);
                    } else if (Workers.Workers[MenuRemapper[MenuPage - 1]].Employer == PlayerNum) {
                        if (Workers.Workers[MenuRemapper[MenuPage - 1]].Typ >= WORKER_PILOT && CursorPos.IfIsWithin(40, 181, 260, 200)) {
                            CheckCursorHighlight(CursorPos, CRect(40, 178, 260, 200), ColorOfFontBlack, CURSOR_HOT, 0, -103, MENU_PERSONAL, -7);
                        }

                        // Feuern:
                        CheckCursorHighlight(CursorPos, CRect(40, 254 + 24 - 22, 200, 266 + 24 - 22), ColorOfFontBlack, CURSOR_HOT, 0, -103, MENU_PERSONAL, -4);

                        /// Gehalt erhöhen/kürzen:
                        if (MenuPar2 != 1) {
                            CheckCursorHighlight(CursorPos, CRect(40, 267 + 24 - 22, 200, 279 + 24 - 22), ColorOfFontBlack, CURSOR_HOT, 0, -103, MENU_PERSONAL,
                                                 -5);
                            CheckCursorHighlight(CursorPos, CRect(40, 280 + 24 - 22, 200, 293 + 24 - 22), ColorOfFontBlack, CURSOR_HOT, 0, -103, MENU_PERSONAL,
                                                 -6);
                        }
                    }
                }
                break;

            case MENU_REQUEST:
                // if (MenuPar1==MENU_REQUEST_NORENTROUTE1 || MenuPar1==MENU_REQUEST_NORENTROUTE2 || MenuPar1==MENU_REQUEST_NORENTROUTE3 ||
                // MenuPar1==MENU_REQUEST_NORENTROUTE4 || MenuPar1==MENU_REQUEST_FORBIDDEN || MenuPar1==MENU_REQUEST_DESTROYED || MenuPar1==MENU_REQUEST_ITEMS
                // || MenuPar1==MENU_REQUEST_NOGLUE || MenuPar1==MENU_REQUEST_NOSTENCH || (MenuPar1>=MENU_REQUEST_NO_LM && MenuPar1<=MENU_REQUEST_NO_WERBUNG))
                if (MenuIsPlain()) {
                    if (CursorPos.IfIsWithin(100, 160, 190, 200)) {
                        SetMouseLook(CURSOR_HOT, 0, -101, MENU_REQUEST, 1);
                    }
                } else {
                    if (CursorPos.IfIsWithin(0, 160, 100, 200)) {
                        SetMouseLook(CURSOR_HOT, 0, -101, MENU_REQUEST, 1);
                    } else if (CursorPos.IfIsWithin(190, 160, 290, 200)) {
                        SetMouseLook(CURSOR_HOT, 0, -101, MENU_REQUEST, 2);
                    }
                }
                break;

            case MENU_CLOSED:
                if (CursorPos.IfIsWithin(100, 160, 190, 200)) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_CLOSED, 1);
                }
                break;

            case MENU_RENAMEPLANE:
                if (CursorPos.IfIsWithin(0, 160, 100, 200) && Optionen[0].GetLength() > 0) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_RENAMEPLANE, 1);
                } else if (CursorPos.IfIsWithin(190, 160, 290, 200)) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_RENAMEPLANE, 2);
                }
                break;

            case MENU_RENAMEEDITPLANE:
                if (CursorPos.IfIsWithin(0, 160, 100, 200) && Optionen[0].GetLength() > 0) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_RENAMEEDITPLANE, 1);
                } else if (CursorPos.IfIsWithin(190, 160, 290, 200)) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_RENAMEEDITPLANE, 2);
                }
                break;

            case MENU_ENTERTCPIP:
                if (CursorPos.IfIsWithin(100, 160, 190, 200)) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_ENTERTCPIP, 1);
                }
                break;

            case MENU_BROADCAST:
                if (CursorPos.IfIsWithin(0, 160, 100, 200) && Optionen[0].GetLength() > 0) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_RENAMEPLANE, 1);
                } else if (CursorPos.IfIsWithin(190, 160, 290, 200)) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_RENAMEPLANE, 2);
                }

                for (c = 0; c < 4; c++) {
                    if (Sim.Players.Players[c].Owner == 2) {
                        if (CursorPos.IfIsWithin(60 + (c - static_cast<SLONG>(c > Sim.localPlayer)) * 60, 80,
                                                 60 + (c - static_cast<SLONG>(c > Sim.localPlayer)) * 60 + 60, 80 + 60)) {
                            SetMouseLook(CURSOR_HOT, 1000 + c,
                                         bprintf("%s (%s)", (LPCTSTR)Sim.Players.Players[c].NameX, (LPCTSTR)Sim.Players.Players[c].AirlineX), -101,
                                         MENU_RENAMEPLANE, 10 + c);
                        }
                    }
                }
                break;

            case MENU_GAMEOVER:
                break;

            case MENU_BANK:
                if (CursorPos.IfIsWithin(111, 159, 198, 190)) {
                    SetMouseLook(CURSOR_HOT, 4101 + static_cast<SLONG>(MenuPar2 == 1) * 10, -102, MENU_BANK, 11);
                } else if (CursorPos.IfIsWithin(204, 156, 271, 187)) {
                    SetMouseLook(CURSOR_HOT, 4100 + static_cast<SLONG>(MenuPar2 == 1) * 10, -102, MENU_BANK, 10);
                }

                CheckCursorHighlight(CursorPos, CRect(56, 81, 318, 96), ColorOfFontBlack, CURSOR_HOT, 4102 + static_cast<SLONG>(MenuPar2 == 1), -102, MENU_BANK,
                                     12);
                break;

            case MENU_AKTIE:
            case MENU_SETRENDITE:
                if (CursorPos.IfIsWithin(237, 125, 304, 156)) {
                    SetMouseLook(CURSOR_HOT, 4200 + static_cast<SLONG>(MenuPar2 == 1) * 10 + static_cast<SLONG>(CurrentMenu == MENU_SETRENDITE) * 100, -102,
                                 MENU_SETRENDITE, 10);
                } else if (CursorPos.IfIsWithin(153, 133, 208, 159)) {
                    SetMouseLook(CURSOR_HOT, 4201 + static_cast<SLONG>(MenuPar2 == 1) * 10 + static_cast<SLONG>(CurrentMenu == MENU_SETRENDITE) * 100, -102,
                                 MENU_SETRENDITE, 11);
                }

                if (CurrentMenu == MENU_AKTIE) {
                    CheckCursorHighlight(CursorPos, CRect(140, 71, 198, 85), ColorOfFontBlack, CURSOR_HOT, 4220, -102, MENU_SETRENDITE, 20);
                    CheckCursorHighlight(CursorPos, CRect(200, 71, 238, 85), ColorOfFontBlack, CURSOR_HOT, 4221, -102, MENU_SETRENDITE, 21);
                    CheckCursorHighlight(CursorPos, CRect(240, 71, 316, 85), ColorOfFontBlack, CURSOR_HOT, 4222, -102, MENU_SETRENDITE, 22);
                } else {
                    CheckCursorHighlight(CursorPos, CRect(180, 35, 325, 49), ColorOfFontBlack, CURSOR_HOT, 4320, -102, MENU_SETRENDITE, 20);
                }
                break;

            case MENU_CHAT:
                if (CursorPos.IfIsWithin(18, 172, 80, 190)) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_CHAT, 1);
                } else if (CursorPos.IfIsWithin(135, 172, 274, 190)) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_CHAT, 2);
                }
                break;

            case MENU_BRIEFING:
                if (CursorPos.IfIsWithin(136, 385, 312, 418)) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_BRIEFING, 1);
                } else if (CursorPos.IfIsWithin(325, 385, 500, 418) && MenuPar1 == 0) {
                    SetMouseLook(CURSOR_HOT, 0, -101, MENU_BRIEFING, 2);
                }
                break;
            default:
                break;
            }
        }
        // Oder in einem Dialog?
        else if (DialogPartner != TALKER_NONE) {
            if (CurrentTextSubIdBis == 0 || (CurrentTextSubIdVon == CurrentTextSubIdBis)) {
                SetMouseLook(CURSOR_HOT, 0, -102, 1, CurrentTextSubIdVon);
            } else if (CurrentHighlight == 0) {
                // kein gültiger Klick
            } else {
                SetMouseLook(CURSOR_HOT, 0, -102, 2, CurrentHighlight);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Bringt CursorHighlighting und ToolTips auf den neusten Stand:
//--------------------------------------------------------------------------------------------
void CStdRaum::PumpToolTips() {
    if (Sim.Players.Players.AnzEntries() == 0) {
        return;
    }

    if (gHighlightFontColor != 0U) {
        if (gHighlightFontColor == ColorOfFontBlack) {
            ColorFX.HighlightText(&PrimaryBm.PrimaryBm, gHighlightArea, gHighlightFontColor, 0x20ff64);
        } else if (gHighlightFontColor == ColorOfFontGrey) {
            ColorFX.HighlightText(&PrimaryBm.PrimaryBm, gHighlightArea, gHighlightFontColor, 0x305030); // 0x108030);
        } else if (gHighlightFontColor == ColorOfFontRed) {
            ColorFX.HighlightText(&PrimaryBm.PrimaryBm, gHighlightArea, gHighlightFontColor, 0xff8080);
        }
    }

    if (GlowEffects.AnzEntries() != 0) {
        XY Offset(0, 0);

        if (Sim.Players.Players[Sim.localPlayer].GetRoom() == ROOM_GLOBE && (Sim.Players.Players[Sim.localPlayer].Blocks.IsInAlbum(ULONG(0)) != 0)) {
            Offset = Sim.Players.Players[Sim.localPlayer].Blocks[ULONG(0)].ScreenPos;

            if ((dynamic_cast<CGlobe *>(this))->FensterVisible == 0) {
                Offset = XY(1000, 1000);
            }
        }

        for (SLONG c = 0; c < GlowEffects.AnzEntries(); c++) {
            ColorFX.BlitGlow(gGlowBms[GlowBitmapIndices[c]].pBitmap, &PrimaryBm.PrimaryBm, GlowEffects[c] + Offset);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Malt ggf. über den gesammten Raum noch den Text drüber:
//--------------------------------------------------------------------------------------------
void CStdRaum::PostPaint() {
    if (bHandy != 0) {
        return;
    }

    if (bLeaveGameLoop != 0) {
        return;
    }
    if (PlayerNum < 0 || PlayerNum > 3) {
        return;
    }

    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    XY p;
    SLONG c = 0;
    SLONG Rand = 0;

    if ((IsDialogOpen() != 0) && (qPlayer.DialogWin != nullptr) && (PayingForCall != 0)) {
        if (SLONG((Sim.Time - LetzteEinheit) / 50) > (2 - UsingHandy) * 10 + (3 - Ferngespraech) * 60) {
            LetzteEinheit = Sim.Time;
            qPlayer.History.AddCallCost(1);
            qPlayer.Money -= 1;

            qPlayer.Statistiken[STAT_A_SONSTIGES].AddAtPastDay(-1);
            qPlayer.Bilanz.SonstigeAusgaben -= 1;
            if (PlayerNum == Sim.localPlayer) {
                SIM::SendSimpleMessage(ATNET_CHANGEMONEY, 0, Sim.localPlayer, -1, STAT_A_SONSTIGES);
            }

            StatusCount = max(StatusCount, 3);
        }
    }

    if (PlayerNum != -1 && (qPlayer.DialogWin != nullptr)) {
        CStdRaum &qRoom = *qPlayer.DialogWin;
        CRect SrcRect(qRoom.HandyOffset, 0, qRoom.HandyOffset + 340, 440);
        XY Dest(qRoom.TempScreenScroll, 0);

        SDL_SetColorKey(qPlayer.DialogWin->RoomBm.pBitmap->GetSurface(), 0, 0);
        SDL_FillRect(qPlayer.DialogWin->RoomBm.pBitmap->GetSurface(), nullptr, SDL_MapRGB(qPlayer.DialogWin->RoomBm.pBitmap->GetPixelFormat(), 1, 1, 1));
        qPlayer.DialogWin->OnPaint();
        // Handy-Screen einblenden; entweder direkt in die primary-Bitmap (wenn am Airport) oder in die Zwischenbitmap (wenn in einem Raum)
        if (RoomBm.Size.x > 0) {
            RoomBm.pBitmap->SetClipRect(CRect(0, 0, 640, 440));
            RoomBm.BlitFrom(qRoom.RoomBm, SrcRect, Dest);

            for (SLONG cy = 0; cy < 9; cy++) {
                RoomBm.BlitFrom(qRoom.RoomBm, CRect(qRoom.HandyOffset - 13 * cy, -24 + cy * 52, qRoom.HandyOffset + 13, min(440, 28 + cy * 52)),
                                XY(qRoom.TempScreenScroll - 13 * cy, -24 + cy * 52));

                RoomBm.BlitFromT(gDialogBarBm, qRoom.TempScreenScroll + 1 - 13 - 13 * cy, -24 + cy * 52);
            }
            RoomBm.pBitmap->SetClipRect(CRect(0, 0, 640, 480));
        } else {
            PrimaryBm.PrimaryBm.SetClipRect(CRect(0, 0, 640, 440));
            PrimaryBm.BlitFrom(qRoom.RoomBm, SrcRect, Dest);

            for (SLONG cy = 0; cy < 9; cy++) {
                PrimaryBm.BlitFrom(qRoom.RoomBm, CRect(qRoom.HandyOffset - 13 * cy, -24 + cy * 52, qRoom.HandyOffset + 13, min(440, 28 + cy * 52)),
                                   XY(qRoom.TempScreenScroll - 13 * cy, -24 + cy * 52));

                PrimaryBm.BlitFromT(gDialogBarBm, qRoom.TempScreenScroll + 1 - 13 - 13 * cy, -24 + cy * 52);
            }
            PrimaryBm.PrimaryBm.SetClipRect(CRect(0, 0, 640, 480));
        }
    }

    if (bHandy == FALSE) {
        // Die Texte organisieren:
        if ((bActive != 0) && (CurrentTextGroupId != 0U) && (bHandy == FALSE || TempScreenScrollV == 0)) {
            dword status = 0;
            if (TalkingSpeechFx != 0) {
                SpeechFx.pFX->GetStatus(&status);
            }

            // Nicht anzeigen, wenn der Kerl sich erst zum Sprechen bereit macht:
            if (pSmackerPartner == nullptr || TextAlign != 0 || pSmackerPartner->GetMood() == SPM_TALKING || AtGetTime() > DWORD(SmackerTimeToTalk)) {
                // hprintvar (SpeechFx.pFX->IsMouthOpen(400));
                if ((pSmackerPartner != nullptr) && ReadyToStartSpeechFx == 0 && pSmackerPartner->GetMood() == SPM_TALKING &&
                    ((AtGetTime() > DWORD(SmackerTimeToTalk) && TalkingSpeechFx == 0) || ((TalkingSpeechFx != 0) && !SpeechFx.pFX->IsMouthOpen(400)) ||
                     TextAlign != 0)) {
                    pSmackerPartner->SetDesiredMood(SPM_LISTENING);
                }

                // Den Cursor-Highlight überprüfen:
                CheckHighlight(qPlayer.CursorPos);
            } else if ((pSmackerPartner != nullptr) && pSmackerPartner->GetMood() == SPM_LISTENING && (TalkingSpeechFx != 0) &&
                       (SpeechFx.pFX->IsMouthOpen(400) || (ReadyToStartSpeechFx != 0)) && TextAlign == 0) {
                pSmackerPartner->SetDesiredMood(SPM_TALKING);
            } else {
                // Zeit neu berechnen, weil er ja noch nicht spricht:
                SmackerTimeToTalk = AtGetTime() + Optionen[0].GetLength() * 2 * 50;
            }

            if (pSmackerPartner == nullptr || TextAlign != 0 ||
                (pSmackerPartner->GetMood() == SPM_TALKING ||
                 (pSmackerPartner->GetMood() == SPM_LISTENING && TextAlign == 0 && ((status & DSBSTATUS_PLAYING) != 0U) && (TalkingSpeechFx != 0))) ||
                AtGetTime() > DWORD(SmackerTimeToTalk)) {
                if ((NumberBitmap.Size.x != 0) && (RoomBm.pBitmap != nullptr)) {
                    RoomBm.BlitFrom(NumberBitmap, NumberBitmapPos);
                }

                // Text kopieren:
                if (OnscreenBitmap.pBitmap != nullptr) {
                    if (RoomBm.Size.x > 0) {
                        if (DisplayThisBubble != 0) {
                            ColorFX.BlitWhiteTrans(TRUE, OnscreenBitmap.pBitmap, RoomBm.pBitmap, XY(WinP1.x + BubbleRect.left, WinP1.y + BubbleRect.top),
                                                   &BubbleRect);
                        }
                    } else {
                        if (DisplayThisBubble != 0) {
                            ColorFX.BlitWhiteTrans(TRUE, OnscreenBitmap.pBitmap, &PrimaryBm.PrimaryBm, XY(WinP1.x + BubbleRect.left, WinP1.y + BubbleRect.top),
                                                   &BubbleRect);
                        }
                    }
                }
            }

            if (TalkingSpeechFx != 0) {
                dword status = 0;
                SpeechFx.pFX->GetStatus(&status);
                if ((status & DSBSTATUS_PLAYING) != 0U) {
                    SmackerTimeToTalk = AtGetTime() + Optionen[0].GetLength() * 2 * 50;
                } else {
                    SmackerTimeToTalk = AtGetTime() - 1;
                }
            }
        }

        PrimaryBm.BlitFrom(RoomBm);
    }

    bDestructorCalledInMeantime = false;
    if (nOptionsOpen == 0 && IgnoreNextPostPaintPump == 0) {
        MessagePump();
    }

    if (bDestructorCalledInMeantime) {
        return;
    }
    if (bLeaveGameLoop != 0) {
        return;
    }
    if (PlayerNum < 0 || PlayerNum > 3) {
        return;
    }

    if (IgnoreNextPostPaintPump > 0) {
        IgnoreNextPostPaintPump--;
    }

    if (CurrentMenu == MENU_WC_F || CurrentMenu == MENU_WC_M) {
        MenuRepaint();
    }
    if (CurrentMenu == MENU_FLUEGE && (Sim.TimeSlice & 1) == 0) {
        MenuRepaint();
    }

    // Ikonen rechts vom Inventar:
    if (gMousePosition.x >= 634 - 24 - 39 && gMousePosition.y >= WinP2.y - StatusLineSizeY && gMousePosition.x < 640 && gMousePosition.y < WinP2.y) {
        StatusCount = max(StatusCount, 3);
    }

    if ((bActive != 0) && Sim.Gamestate == (GAMESTATE_PLAYING | GAMESTATE_WORKING)) {
        CMessages &qMessages = qPlayer.Messages;

        // Berater zeichnen wenn er kommt oder geht:
        if (qMessages.AktuellerBeraterTyp != -1 && qMessages.BeraterWalkState != 0 && PicBitmap.Size.y <= 440) {
            StatusCount = max(StatusCount, 3);

            if (qMessages.AktuellerBeraterTyp < 100) { // Berater rechts, andere links
                PrimaryBm.BlitFromT(BeraterBms[qMessages.AktuelleBeraterBitmap][0], 640 - BeraterBms[qMessages.AktuelleBeraterBitmap][0].Size.x,
                                    qPlayer.Messages.BeraterPosY);
            } else {
                if (DontDisplayPlayer == -1 && (IsInBuro == 0)) {
                    PrimaryBm.BlitFromT(BeraterBms[qMessages.AktuelleBeraterBitmap][0], 0, qPlayer.Messages.BeraterPosY);
                } else if (qPlayer.Messages.IsDialogTalking <= 0 && (IsInBuro != 0) && (IsDialogOpen() != 0)) {
                    (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_LISTENING);
                }
            }
        }

        // Die Statuszeile
        if ((StatusCount > 0 || (bNoSpeedyBar != 0) || (gBlendState >= 0 && gBlendState <= 8)) && PicBitmap.Size.y <= 440) {
            if (StatusCount > 0) {
                StatusCount--;
            }

            PrimaryBm.BlitFrom(StatusLineBms[0], WinP1.x, WinP2.y - StatusLineSizeY);
            PrimaryBm.BlitFrom(StatusLineBms[6], WinP2.x - StatusLineBms[6].Size.x, WinP2.y - StatusLineSizeY);

            if (qPlayer.Messages.LastMessage.BeraterTyp != -1) {
                PrimaryBm.BlitFromT(gRepeatMessageBms[0], WinP2.x - StatusLineBms[6].Size.x + 1, WinP2.y - StatusLineSizeY + 1);
            }

            for (c = 0; c < 6; c++) {
                if (qPlayer.Items[c] != 0xff) {
                    PrimaryBm.BlitFrom(gItemBms[static_cast<SLONG>(qPlayer.Items[c])], WinP1.x + 256 - 8 + c * 60, WinP2.y - StatusLineSizeY);
                } else {
                    PrimaryBm.BlitFrom(StatusLineBms[5], WinP1.x + 256 - 8 + c * 60, WinP2.y - StatusLineSizeY);
                }
            }

            if (Sim.IsTutorial != 0) {
                PrimaryBm.BlitFromT(gTutoriumBms[0], WinP2.x - StatusLineBms[6].Size.x + 1 - gTutoriumBms[0].Size.x, 440);
            }

            PrimaryBm.BlitFromT(gStatisticBms[0], WinP2.x - StatusLineBms[6].Size.x + 1, WinP2.y - StatusLineSizeY + 21);

            // Die Ikonen rechts vom Inventar?
            if (gMousePosition.x >= 634 - 24 && gMousePosition.y >= WinP2.y - StatusLineSizeY && gMousePosition.x < 640 && gMousePosition.y < WinP2.y &&
                (IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
                if (gMousePosition.y >= WinP2.y - StatusLineSizeY / 2) {
                    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && (MouseWait == 0) && (Sim.GetHour() > 9 || Sim.GetMinute() > 0)) {
                        PrimaryBm.BlitFromT(gStatisticBms[1], WinP2.x - StatusLineBms[6].Size.x + 1, WinP2.y - StatusLineSizeY + 21);

                        SetMouseLook(CURSOR_HOT, 1000, -100, 0);
                    }
                } else if (gMousePosition.y < WinP2.y - StatusLineSizeY / 2 && qPlayer.Messages.LastMessage.BeraterTyp != -1) {
                    PrimaryBm.BlitFromT(gRepeatMessageBms[1], WinP2.x - StatusLineBms[6].Size.x + 1, WinP2.y - StatusLineSizeY + 1);

                    SetMouseLook(CURSOR_HOT, 2100, -100, 0);
                }

                if ((gStatButton != 0) && AtGetTime() - gStatButtonTimer < 400) {
                    if (gStatButton == 2) {
                        PrimaryBm.BlitFromT(gRepeatMessageBms[2], WinP2.x - StatusLineBms[6].Size.x + 1, WinP2.y - StatusLineSizeY + 1);
                    } else if (gStatButton == 3) {
                        PrimaryBm.BlitFromT(gStatisticBms[2], WinP2.x - StatusLineBms[6].Size.x + 1, WinP2.y - StatusLineSizeY + 21);
                    }
                } else if ((gStatButton != 0) && AtGetTime() - gStatButtonTimer > 400) {
                    gStatButton = 0;
                }
            } else if (((Sim.IsTutorial != 0) || gStatButton == 1) && gMousePosition.IfIsWithin(640 - 32 - 39, 440, 607, 479) && (IsDialogOpen() == 0) &&
                       (MenuIsOpen() == 0)) {
                PrimaryBm.BlitFromT(gTutoriumBms[1], WinP2.x - StatusLineBms[6].Size.x + 1 - gTutoriumBms[0].Size.x + 8, 440 + 11);
                SetMouseLook(CURSOR_HOT, 2101, -100, 0);

                if ((gStatButton != 0) && AtGetTime() - gStatButtonTimer < 400) {
                    if (gStatButton == 1) {
                        PrimaryBm.BlitFromT(gTutoriumBms[0], WinP2.x - StatusLineBms[6].Size.x + 1 - gTutoriumBms[0].Size.x, 440);
                        PrimaryBm.BlitFromT(gTutoriumBms[2], WinP2.x - StatusLineBms[6].Size.x + 1 - gTutoriumBms[0].Size.x + 8, 440 + 11);
                    }
                } else if ((gStatButton != 0) && AtGetTime() - gStatButtonTimer > 400) {
                    gStatButton = 0;
                }
            }

            if (Sim.bNetwork != 0) {
                SLONG Speed = 10;

                for (SLONG c = 0; c < 4; c++) {
                    if (Sim.Players.Players[c].IsOut == 0 && Sim.Players.Players[c].CallItADay == 0) {
                        if (Sim.Players.Players[c].Owner == 0 || Sim.Players.Players[c].Owner == 2) {
                            if (Sim.Players.Players[c].Owner == 2) {
                                ColorFX.BlitTrans(gClockBms[Sim.Players.Players[c].GameSpeed + 4].pBitmap, &PrimaryBm.PrimaryBm,
                                                  XY(79, WinP2.y - StatusLineSizeY));
                            }

                            Speed = min(Speed, Sim.Players.Players[c].GameSpeed);
                        }
                    }
                }

                PrimaryBm.BlitFromT(gClockBms[Speed], 79, WinP2.y - StatusLineSizeY);
                PrimaryBm.BlitFromT(gClockBms[qPlayer.GameSpeed + 8], 79, WinP2.y - StatusLineSizeY);
            } else {
                PrimaryBm.BlitFromT(gClockBms[qPlayer.GameSpeed], 79, WinP2.y - StatusLineSizeY);
            }

            // Das Logo anzeigen
            PrimaryBm.BlitFrom(LogoBms[static_cast<SLONG>(qPlayer.Logo)], 7, 443);

            // Geld/Zeit anzeigen:
            if (Sim.Time != LastTime || qPlayer.Money != LastMoney) {
                StatusBm.Clear(0);

                LastMoney = qPlayer.Money;
                LastTime = Sim.Time;

                {
                    // Money:
                    if (LastMoney < 1000000000) {
                        StatusBm.PrintAt(Insert1000erDots64(LastMoney), FontCash, TEC_FONT_LEFT, 8, 21, 230, StatusLineSizeY);
                    } else {
                        StatusBm.PrintAt(Insert1000erDots64(LastMoney / 1000000) + ETexte.GetS(1000, 1000 + EINH_MIO), FontCash, TEC_FONT_LEFT, 8, 21, 230,
                                         StatusLineSizeY);
                    }

                    // Time:
                    StatusBm.PrintAt(CString(StandardTexte.GetS(TOKEN_SCHED, 3000 + (Sim.StartWeekday + Sim.Date) % 7)) + "  " + Sim.GetTimeString(), FontCash,
                                     TEC_FONT_LEFT, 8, 2, 230, StatusLineSizeY);
                }
            }

            PrimaryBm.BlitFromT(StatusBm, WinP1.x + 102, WinP2.y - StatusLineSizeY);

            if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
                if (gMousePosition.IfIsWithin(78, 440, 241, 459)) {
                    PrimaryBm.BlitFromT(gStatLineHigh[0], 78, 440);
                    UpdateStatusBar();
                }
                if (gMousePosition.IfIsWithin(78, 460, 241, 479)) {
                    PrimaryBm.BlitFromT(gStatLineHigh[1], 78, 461);
                    UpdateStatusBar();
                }
            }
        }

        // Einen anderen Spieler als Dialogpartner anzeigen:
        if (DialogPartner == TALKER_COMPETITOR) {
            if (AtGetTime() > DWORD(SmackerTimeToTalk) || TextAlign != 0 || ((TalkingSpeechFx > 0 && !SpeechFx.pFX->IsMouthOpen(200)))) {
                PrimaryBm.FlipBlitFromT(BeraterBms[12 + DialogPar1][static_cast<SLONG>(DialogMedium == MEDIUM_HANDY) * 4],
                                        XY(640 - BeraterBms[12 + DialogPar1][0].Size.x,
                                           440 - BeraterSlideY[12 + DialogPar1 + static_cast<SLONG>(DialogMedium == MEDIUM_HANDY) * 4]));
            } else {
                PrimaryBm.FlipBlitFromT(BeraterBms[12 + DialogPar1]["\x0\x2\x1\x3\x2\x3\x0\x1\x2"[(AtGetTime() / 50 / 3) & 7] +
                                                                    static_cast<SLONG>(DialogMedium == MEDIUM_HANDY) * 4],
                                        XY(640 - BeraterBms[12 + DialogPar1][0].Size.x,
                                           440 - BeraterSlideY[12 + DialogPar1 + static_cast<SLONG>(DialogMedium == MEDIUM_HANDY) * 4]));
            }

            // Gedankenblase für nachdenkende Netzwerkspieler:
            if (((IsDialogOpen() != 0) && pSmackerPartner == nullptr) || TextAlign != 0 ||
                (pSmackerPartner->GetMood() == SPM_TALKING || (pSmackerPartner->GetMood() == SPM_LISTENING && TextAlign == 0))) {
                if (OnscreenBitmap.pBitmap == nullptr) {
                    if (DisplayThisBubble != 0) {
                        ColorFX.BlitTrans(MoodBms[MoodPersonEmpty].pBitmap, &PrimaryBm.PrimaryBm,
                                          XY(640 - BeraterBms[12 + DialogPar1][0].Size.x,
                                             440 - BeraterSlideY[12 + DialogPar1 + static_cast<SLONG>(DialogMedium == MEDIUM_HANDY) * 4]) -
                                              XY(-5, MoodBms[MoodPersonEmpty].Size.y),
                                          nullptr, 4);
                    }
                }
            }
        }

        // Onscreen-Menüs:
        if (CurrentMenu != MENU_NONE) {
            // Menu zooming:
            if (CurrentMenu == MENU_GAMEOVER && AtGetTime() - MenuInfo < 270) {
                PlayUniversalFx("cam.raw", Sim.Options.OptionEffekte);
                PrimaryBm.PrimaryBm.Clear(SB_Hardwarecolor(0xffff)); // Flash-Effekt
            } else {
                if (ZoomCounter < 100 && MinimumZoom >= 1.0) {
                    ZoomCounter = 100;
                }

                if (OnscreenBitmap.Size.y == 480) {
                    PrimaryBm.BlitFrom(OnscreenBitmap, 0, 0);
                } else if (ZoomCounter == 100) {
                    if (CurrentMenu == MENU_EXTRABLATT && ZoomSpeed == 0) {
                        PrimaryBm.BlitFrom(OnscreenBitmap, MenuPos);
                    } else {
                        PrimaryBm.BlitFromT(OnscreenBitmap, MenuPos);
                    }
                } else {
                    SDL_Rect SrcRect = {0, 0, OnscreenBitmap.Size.x, OnscreenBitmap.Size.y};
                    SDL_Rect DestRect;

                    XY p = MenuStartPos;
                    if (ZoomFromAirport != 0) {
                        p -= qPlayer.ViewPos;
                    }

                    if (CurrentMenu == MENU_EXTRABLATT) {
                        DestRect.x = 320 - OnscreenBitmap.Size.x / 2 * ZoomCounter / 100;
                        DestRect.y = 220 - OnscreenBitmap.Size.y / 2 * ZoomCounter / 100;
                        DestRect.w = OnscreenBitmap.Size.x * ZoomCounter / 100;
                        DestRect.h = OnscreenBitmap.Size.y * ZoomCounter / 100;
                    } else {
                        DestRect.x = (p.x * (100 - ZoomCounter) + MenuPos.x * ZoomCounter) / 100;
                        DestRect.y = (p.y * (100 - ZoomCounter) + MenuPos.y * ZoomCounter) / 100;
                        DestRect.w = SLONG(OnscreenBitmap.Size.x * (MinimumZoom * 100 + ((ZoomCounter * (1.0 - MinimumZoom)))) / 100);
                        DestRect.h = SLONG(OnscreenBitmap.Size.y * (MinimumZoom * 100 + ((ZoomCounter * (1.0 - MinimumZoom)))) / 100);
                    }

                    SDL_BlitScaled(OnscreenBitmap.pBitmap->GetSurface(), &SrcRect, PrimaryBm.PrimaryBm.GetSurface(), &DestRect);
                }

                if (CurrentMenu == MENU_EXTRABLATT) {
                    if (ZoomCounter < 200) {
                        StatusCount = max(StatusCount, 3);
                        ZoomCounter += ZoomSpeed;
                        if (ZoomCounter > 200) {
                            ZoomCounter = 200;
                        }
                    }

                    MenuRepaint();
                } else if (ZoomCounter < 100) {
                    StatusCount = max(StatusCount, 3);
                    ZoomCounter += ZoomSpeed;
                    if (ZoomCounter > 100) {
                        ZoomCounter = 100;
                    }
                }
            }
        }
        // Menu unzoom:
        else if (ZoomCounter > 0 && OnscreenBitmap.Size.x > 0 && (IsDialogOpen() == 0)) {
            if (MinimumZoom >= 1.0) {
                ZoomCounter = 0;
            } else {
                ZoomCounter -= 25;
                if (ZoomCounter < 0) {
                    ZoomCounter = 0;
                }
            }

            if (ZoomCounter > 0) {
                SDL_Rect SrcRect = {0, 0, OnscreenBitmap.Size.x, OnscreenBitmap.Size.y};
                SDL_Rect DestRect;

                StatusCount = max(StatusCount, 3);

                XY p = MenuStartPos;
                if (ZoomFromAirport != 0) {
                    p -= qPlayer.ViewPos;
                }

                DestRect.x = (p.x * (100 - ZoomCounter) + MenuPos.x * ZoomCounter) / 100;
                DestRect.y = (p.y * (100 - ZoomCounter) + MenuPos.y * ZoomCounter) / 100;
                DestRect.w = SLONG(OnscreenBitmap.Size.x * (MinimumZoom * 100 + ((ZoomCounter * (1.0 - MinimumZoom)))) / 100);
                DestRect.h = SLONG(OnscreenBitmap.Size.y * (MinimumZoom * 100 + ((ZoomCounter * (1.0 - MinimumZoom)))) / 100);

                SDL_BlitScaled(OnscreenBitmap.pBitmap->GetSurface(), &SrcRect, PrimaryBm.PrimaryBm.GetSurface(), &DestRect);
            }
        }

        MenuPostPaint();
        CalcRepaint();

        // Berater zeichnen wenn er da ist:
        if (qMessages.AktuellerBeraterTyp != -1 && qMessages.BeraterWalkState == 0 && PicBitmap.Size.y <= 440) {
            StatusCount = max(StatusCount, 3);

            if (qMessages.AktuellerBeraterTyp < 100) { // Berater rechts, andere links
                PrimaryBm.BlitFromT(
                    BeraterBms[qMessages.AktuelleBeraterBitmap]["\0\0\x1\0\x1\x1"[(qMessages.TalkPhase / 3) % 6] +
                                                                (static_cast<SLONG>(qMessages.BlinkCountdown > 0) * BeraterBms[0].AnzEntries() / 2)],
                    640 - BeraterBms[qMessages.AktuelleBeraterBitmap][0].Size.x, qMessages.BeraterPosY);
            } else if (DontDisplayPlayer == -1 && (IsInBuro == 0)) {
                PrimaryBm.BlitFromT(BeraterBms[qMessages.AktuelleBeraterBitmap]
                                              ["\x0\x2\x1\x3\x2\x3\x0\x1\x2"[static_cast<SLONG>(TalkingSpeechFx == 0 || SpeechFx.pFX->IsMouthOpen(200)) *
                                                                             ((qMessages.TalkPhase / 3) & 7)] +
                                               static_cast<SLONG>(DialogMedium == MEDIUM_HANDY) * 4],
                                    0, qMessages.BeraterPosY);
            } else if ((qPlayer.Messages.IsDialogTalking <= 0 || ((TalkingSpeechFx != 0) && !SpeechFx.pFX->IsMouthOpen(200))) && (IsInBuro != 0) &&
                       (IsDialogOpen() != 0)) {
                (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_LISTENING);
            }

            if (qMessages.Messages[static_cast<SLONG>(0)].Message.GetLength() > 0 && qMessages.AktuellerBeraterTyp < 100) {
                ColorFX.BlitWhiteTrans(TRUE, qMessages.SprechblaseBm.pBitmap, &PrimaryBm.PrimaryBm,
                                       XY(640 - BeraterBms[qMessages.AktuelleBeraterBitmap][0].Size.x +
                                              BeraterSprechblasenOffset[qMessages.AktuelleBeraterBitmap].x - qMessages.SprechblaseBm.Size.x,
                                          qMessages.BeraterPosY + BeraterSprechblasenOffset[qMessages.AktuelleBeraterBitmap].y));

                if (qMessages.Messages[static_cast<SLONG>(0)].Mood != -1) {
                    if (qMessages.Messages[static_cast<SLONG>(0)].BubbleType == 1) {
                        ColorFX.BlitTrans(SmileyBms[qMessages.Messages[static_cast<SLONG>(0)].Mood].pBitmap, &PrimaryBm.PrimaryBm,
                                          XY(640 - BeraterBms[qMessages.AktuelleBeraterBitmap][0].Size.x +
                                                 BeraterSprechblasenOffset[qMessages.AktuelleBeraterBitmap].x - qMessages.SprechblaseBm.Size.x,
                                             qMessages.BeraterPosY + BeraterSprechblasenOffset[qMessages.AktuelleBeraterBitmap].y) +
                                              XY(5, 7),
                                          nullptr, 2);
                    } else if (qMessages.Messages[static_cast<SLONG>(0)].BubbleType == 2) {
                        ColorFX.BlitTrans(SmileyBms[qMessages.Messages[static_cast<SLONG>(0)].Mood].pBitmap, &PrimaryBm.PrimaryBm,
                                          XY(640 - BeraterBms[qMessages.AktuelleBeraterBitmap][0].Size.x +
                                                 BeraterSprechblasenOffset[qMessages.AktuelleBeraterBitmap].x - qMessages.SprechblaseBm.Size.x,
                                             qMessages.BeraterPosY + BeraterSprechblasenOffset[qMessages.AktuelleBeraterBitmap].y) +
                                              XY(7, 18),
                                          nullptr, 2);
                    }
                }
            }
        } else {
            Rand++;
        }
    }

    if (gBroadcastBm.Size.y > 10) {
        ColorFX.BlitWhiteTrans(TRUE, gBroadcastBm.pBitmap, &PrimaryBm.PrimaryBm, XY(10, 10));
    } else if (gBroadcastBm.Size.y > 0) {
        ColorFX.BlitWhiteTrans(TRUE, gBroadcastBm.pBitmap, &PrimaryBm.PrimaryBm, XY(10 - (10 - gBroadcastBm.Size.y) * 20, 10 + (10 - gBroadcastBm.Size.y) * 5));
    }

    Rand = 0;

    if (bHandy == FALSE) {
        if ((ForceRedrawTip != 0) || CurrentTipType != LastTipType || LastTipId != CurrentTipId || CurrentTipIdPar1 != LastTipIdPar1 ||
            CurrentTipIdPar2 != LastTipIdPar2) {
            RepaintTip();
        }

        if (CurrentTipType != TIP_NONE) {
            XY p = TipPos;

            if (p.x + TipBm.Size.x >= WinP2.x - WinP1.x) {
                p.x = WinP2.x - WinP1.x - TipBm.Size.x - 1;
            }
            if (p.y + TipBm.Size.y >= WinP2.y - WinP1.y - StatusLineSizeY) {
                p.y = WinP2.y - WinP1.y - StatusLineSizeY - TipBm.Size.y - 1;
            }
            if (p.x < 0) {
                p.x = 0;
            }
            if (p.y < 0) {
                p.y = 0;
            }

            p.x = p.x * (WinP2.x - WinP1.x + 1) / 640;
            p.y = p.y * (WinP2.y - WinP1.y - StatusLineSizeY + 1) / 440;

            PrimaryBm.BlitFromT(TipBm, p);
        }
    }

    if (CheatTestGame != 0) {
        static SLONG LastTime = 0;

        if (CheatTestGame == 2 && Sim.GetHour() == 9 && Sim.GetMinute() == 0 && rand() % 30 == 0) {
            FrameWnd->OnRButtonDown(WM_RBUTTONDOWN, gMousePosition.x + gMousePosition.y * 65536);
        }

        if (Sim.Time - LastTime > 255 && Sim.Persons[Sim.Persons.GetPlayerIndex(Sim.localPlayer)].Dir >= 8 && Sim.GetHour() < 17 && Sim.GetHour() > 9) {
            static XY LastCursor;
            PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];

            LastTime = Sim.Time;

            if (rand() % 10 == 1) {
                TXY<SLONG> rcWindow;

                gMousePosition = XY(rand() % 640, rand() % 480);

                SDL_GetWindowPosition(FrameWnd->m_hWnd, &rcWindow.x, &rcWindow.y);

                SDL_WarpMouseGlobal(rcWindow.x + gMousePosition.x, rcWindow.y + gMousePosition.y);
            } else if ((LastCursor == gMousePosition && rand() % 2 == 0) || rand() % 10 != 0) {
                if (rand() % 5 == 0) {
                    gMousePosition = XY(rand() % 640, rand() % 480);
                }
                FrameWnd->OnLButtonDown(WM_LBUTTONDOWN, gMousePosition.x + gMousePosition.y * 65536);
            } else if ((LastCursor == gMousePosition && rand() % 2 == 0) || rand() % 10 < 5) {
                if (rand() % 5 == 0) {
                    gMousePosition = XY(rand() % 640, rand() % 480);
                }
                FrameWnd->OnRButtonDown(WM_RBUTTONDOWN, gMousePosition.x + gMousePosition.y * 65536);
            } else {
                SLONG x = 0;

                do {
                    x = FilofaxRoomRemapper[rand() % 14];
                } while (Sim.Difficulty != DIFF_FINAL && x == ROOM_NASA);

                if (x != 255) {
                    qPlayer.LeaveAllRooms();
                    if (x == ROOM_BURO_A || x == ROOM_PERSONAL_A) {
                        qPlayer.WalkToRoom(x + PlayerNum * 10);
                    } else {
                        qPlayer.WalkToRoom(UBYTE(x));
                    }
                }
            }

            LastCursor = gMousePosition;
        }
    }

    // Der Computergegner hat es ggf eilig und bricht den Dialog ab:
    if (DialogPartner == TALKER_COMPETITOR && Sim.Players.Players[DialogPar1].Owner == 1 && AtGetTime() - TimeBubbleDisplayed > 15 * 1000) {
        if (CurrentTextSubIdBis == 0 || CurrentTextSubIdVon == CurrentTextSubIdBis) {
            PreLButtonDown(XY(160, 100));
        } else {
            if (CurrentTextSubIdVon == 2499) {
                StopDialog();
            } else {
                MakeSayWindow(0, TOKEN_PLAYER, 2499, pFontPartner);
                Sim.Players.Players[DialogPar1].BoredOfPlayer = TRUE;
            }
        }
    }

    if ((IsDialogOpen() != 0) && (CurrentTextSubIdVon == CurrentTextSubIdBis || CurrentTextSubIdBis == 0) && (TalkingSpeechFx != 0)) {
        dword status = 0;
        SpeechFx.pFX->GetStatus(&status);
        if ((status & DSBSTATUS_PLAYING) == 0 && ReadyToStartSpeechFx == 0) {
            PreLButtonDown(CPoint(1, 1));
        }
    }

    if ((pSmackerPartner != nullptr) && pSmackerPartner->GetMood() == SPM_TALKING && (ReadyToStartSpeechFx != 0) && (IsDialogOpen() != 0)) {
        pSmackerPartner->SetDesiredMood(SPM_TALKING);
        if (ReadyToStartSpeechFx == 1) {
            SpeechFx.Play(0, Sim.Options.OptionTalking * 100 / 7);
        }
        ReadyToStartSpeechFx--;
    }
}

//--------------------------------------------------------------------------------------------
// Überprüft, ob der Cursor mitlerweile auf einem anderem Highlight steht:
//--------------------------------------------------------------------------------------------
void CStdRaum::CheckHighlight(const CPoint &point) {
    // Außerhalb geklickt? Dann finden wir eh' nichts!
    if (point.x < WinP1.x || point.y < WinP1.y || point.x > WinP2.x || point.y > WinP2.y) {
        if (CurrentHighlight != 0U) {
            CurrentHighlight = 0;
            RepaintText(FALSE);
        }
        return;
    }

    // Falls wir hier ein Auswahlmenü haben:
    if ((CurrentTextSubIdBis != 0U) && CurrentTextSubIdVon != CurrentTextSubIdBis) {
        ULONG c = 0;
        ULONG Summe = 1;

        if (TextAlign != 0) {
            for (c = CurrentTextSubIdVon; c <= CurrentTextSubIdBis; c++) {
                Summe += TextAreaSizeY[c - CurrentTextSubIdVon] + LINE_DIST;
            }

            if (Sim.Players.Players[PlayerNum].GetRoom() == ROOM_BURO_A + PlayerNum * 10) { // Im Büro?
                Summe = BigPlayerOffset[Sim.localPlayer].y - Summe / 2 - 10 + 10;
            } else // normal
                if (SLONG(430 - Summe / 2) > BeraterSprechblasenOffset[12 + Sim.localPlayer].y) {
                    Summe = BeraterSprechblasenOffset[12 + Sim.localPlayer].y - Summe / 2;
                } else {
                    Summe = 430 - Summe;
                }
        }

        // Welcher Text ist unter dem Cursor?
        for (c = CurrentTextSubIdVon; c <= CurrentTextSubIdBis; c++) {
            if (SLONG(Summe + TextAreaSizeY[c - CurrentTextSubIdVon] + LINE_DIST) >= point.y && point.y >= 0 && point.y >= SLONG(Summe)) {
                if (CurrentHighlight != c) {
                    PlayUniversalFx("change.raw", Sim.Options.OptionEffekte);

                    CurrentHighlight = c;
                    RepaintText(FALSE);
                }
                return;
            }

            Summe += TextAreaSizeY[c - CurrentTextSubIdVon] + LINE_DIST;
        }

        // Kein Text unter dem Cursor! War das schon vorher so?
        if (CurrentHighlight != 0U) {
            CurrentHighlight = 0;
            RepaintText(FALSE);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// CStdRaum message handlers

//--------------------------------------------------------------------------------------------
// void CStdRaum::OnLButtonDblClk(UINT, CPoint point)
//--------------------------------------------------------------------------------------------
void CStdRaum::OnLButtonDblClk(UINT /*unused*/, CPoint point) {
    if ((nOptionsOpen != 0) && (Sim.bNetwork != 0) && Sim.localPlayer < Sim.Players.Players.AnzEntries() &&
        Sim.Players.Players[Sim.localPlayer].GetRoom() != ROOM_OPTIONS && Sim.Players.Players[Sim.localPlayer].GetRoom() != 0) {
        return;
    }

    // Ist das Fenster hier zuständig? Ist der Klick in diesem Fenster?
    if (point.x >= WinP1.x && point.x <= WinP2.x && point.y >= WinP1.y && point.y <= WinP2.y && (Editor == 0)) {
        if (MenuIsOpen() != 0) {
            if (CalculatorIsOpen != 0) {
                CalcClick();
                return;
            }
        } else if (IsDialogOpen() != 0) {
            PreLButtonDown(point);
        }
    }
}

//--------------------------------------------------------------------------------------------
// OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CStdRaum::OnLButtonDown(UINT /*unused*/, CPoint point) {
    if ((nOptionsOpen != 0) && (Sim.bNetwork != 0) && Sim.localPlayer < Sim.Players.Players.AnzEntries() &&
        Sim.Players.Players[Sim.localPlayer].GetRoom() != ROOM_OPTIONS && Sim.Players.Players[Sim.localPlayer].GetRoom() != 0) {
        return;
    }

    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    gMouseClickPosition = gMousePosition;

    // Klick auf Uhr?
    if (point.x >= 78 && point.y >= 440 && point.x < 102 && point.y < 460 && (CheatTestGame == 0)) {
        if (Sim.Date > 2) {
            Sim.GiveHint(HINT_GAMESPEED);
        }
        qPlayer.GameSpeed = (qPlayer.GameSpeed + 1) % 4;

        SIM::SendSimpleMessage(ATNET_SETSPEED, 0, Sim.localPlayer, Sim.Players.Players[Sim.localPlayer].GameSpeed);

        StatusCount = max(StatusCount, 3);
    }
    // Klick auf das Geld?
    else if ((Editor == 0) && gMousePosition.IfIsWithin(103 - 23, 460, 242, 479)) {
        // MoneyTipWait=AtGetTime()-1001;
        MenuStart(MENU_KONTOAUSZUG);
        MenuSetZoomStuff(XY(100, 470), 0.17, FALSE);
    }
    // Cancel Tutorial:
    else if ((Sim.IsTutorial != 0) && gMousePosition.IfIsWithin(640 - 32 - 39, 440, 607, 479) && (IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
        PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];

        gStatButton = 1;
        gStatButtonTimer = AtGetTime();

        qPlayer.Messages.IsMonolog = FALSE;
        qPlayer.Messages.NextMessage();
        qPlayer.Messages.Pump();
        qPlayer.Messages.NextMessage();
        qPlayer.Messages.Pump();
        qPlayer.Messages.NextMessage();
        Sim.IsTutorial = FALSE;

        GlowEffects.ReSize(0);

        if (Sim.GetHour() == 9 && Sim.GetMinute() == 0 && (Sim.Tutorial == 1300 || Sim.Tutorial == 1200 + 40)) {
            Sim.bNoTime = FALSE;
            Sim.DayState = 2;
            if (qPlayer.GetRoom() != ROOM_AIRPORT) {
                qPlayer.LeaveRoom();
            }
        }
    }
    // Klick rechts vom Inventar?
    else if (gMousePosition.x >= 634 - 24 && gMousePosition.y >= WinP2.y - StatusLineSizeY && gMousePosition.x < 640 && gMousePosition.y < WinP2.y) {
        PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];

        // Options:
        if (gMousePosition.y >= WinP2.y - StatusLineSizeY / 2) {
            if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && (MouseWait == 0) && Sim.Time > 9 * 60000) {
                if (qPlayer.IsLocationInQueue(ROOM_OPTIONS) == 0) {
                    qPlayer.EnterRoom(ROOM_OPTIONS);
                    PrimaryBm.BlitFromT(gStatisticBms[2], WinP2.x - StatusLineBms[6].Size.x + 1, WinP2.y - StatusLineSizeY + 21);
                }
            }
        } else // ReGet Message:
        {
            if (qPlayer.Messages.LastMessage.BeraterTyp != -1) {
                gStatButton = 2;
                gStatButtonTimer = AtGetTime();

                if (qPlayer.Messages.LastMessage.Urgent != MESSAGE_COMMENT) {
                    qPlayer.Messages.LastMessage.Urgent = MESSAGE_URGENT;
                }
                qPlayer.Messages.AddMessage(qPlayer.Messages.LastMessage.BeraterTyp, qPlayer.Messages.LastMessage.Message, qPlayer.Messages.LastMessage.Urgent,
                                            qPlayer.Messages.LastMessage.Mood);
                qPlayer.Messages.LastMessage.BeraterTyp = -1;
            }
        }
    }
    // Klick ins Inventar?
    else if (point.x >= WinP1.x + (WinP2.x - WinP1.x) * 4 / 10 && point.y >= WinP2.y - StatusLineSizeY && point.x < 634 && point.y < WinP2.y) {
        SLONG ItemIndex = 0;

        ItemIndex = (point.x - 250) / 60;

        // Klick ins Inventar erfolgreich?
        if (ItemIndex < 6 && Sim.bNoTime == FALSE) {
            switch (qPlayer.Items[ItemIndex]) {
            // Leer:
            case 0xff:
                break;

                // Laptop:
            case ITEM_LAPTOP:
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
                    if (qPlayer.IsLocationInQueue(ROOM_LAPTOP) == 0) {
                        qPlayer.EnterRoom(ROOM_LAPTOP);
                    }
                }
                break;

            case ITEM_HANDY:
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
                    if (qPlayer.GetRoom() == ROOM_BURO_A + PlayerNum * 10) {
                        (dynamic_cast<CBuero *>(this))->KommVarTelefon = 1;
                        (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_LISTENING);
                    } else {
                        MenuDialogReEntryB = -1;
                        UsingHandy = TRUE;
                        MenuStart(MENU_FILOFAX, 2);
                        MenuSetZoomStuff(XY(248 + ItemIndex * 60, 446), 0.17, FALSE);
                    }
                }
                break;

            case ITEM_FILOFAX:
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
                    MenuDialogReEntryB = -1;
                    MenuStart(MENU_FILOFAX, 1);
                    MenuSetZoomStuff(XY(248 + ItemIndex * 60, 446), 0.17, FALSE);
                }
                break;

                // Kalaschnikow:
            case ITEM_MG:
                if ((Sim.Headlines.IsInteresting == 0) && qPlayer.GetRoom() == ROOM_KIOSK) {
                    break;
                }
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && qPlayer.GetRoom() == ROOM_ARAB_AIR) {
                    qPlayer.ArabTrust = max(1, qPlayer.ArabTrust);
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                    StartDialog(TALKER_ARAB, MEDIUM_AIR, 1);
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_MG);
                }
                break;

                // Bier:
            case ITEM_BIER:
                if ((Sim.Headlines.IsInteresting == 0) && qPlayer.GetRoom() == ROOM_KIOSK) {
                    break;
                }
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && qPlayer.GetRoom() == ROOM_WERKSTATT) {
                    qPlayer.MechTrust = max(1, qPlayer.MechTrust);
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                    qPlayer.MechAngry = 0;
                    qPlayer.ReformIcons();
                    StartDialog(TALKER_MECHANIKER, MEDIUM_AIR, 2);
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_BIER);
                } else {
                    PlayUniversalFx("gulps.raw", Sim.Options.OptionEffekte);
                    qPlayer.IsDrunk += 400;
                    qPlayer.Items[ItemIndex] = 0xff;
                }
                break;

            case ITEM_OEL:
                if ((Sim.Headlines.IsInteresting == 0) && qPlayer.GetRoom() == ROOM_KIOSK) {
                    break;
                }
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && qPlayer.GetRoom() == ROOM_GLOBE) {
                    qPlayer.GlobeOiled = TRUE;
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_OEL);
                }
                break;

            case ITEM_TABLETTEN:
                if ((Sim.Headlines.IsInteresting == 0) && qPlayer.GetRoom() == ROOM_KIOSK) {
                    break;
                }
                if (qPlayer.SickTokay != 0) {
                    qPlayer.SickTokay = 0;
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_TABLETTEN);
                }
                break;

            case ITEM_POSTKARTE:
                if ((Sim.Headlines.IsInteresting == 0) && qPlayer.GetRoom() == ROOM_KIOSK) {
                    break;
                }
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && qPlayer.GetRoom() == ROOM_PERSONAL_A + PlayerNum * 10) {
                    qPlayer.SeligTrust = TRUE;
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                    StartDialog(TALKER_PERSONAL1a + PlayerNum * 2, MEDIUM_AIR, 300);
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_POSTKARTE);
                }
                break;

            case ITEM_SPINNE:
                if ((Sim.Headlines.IsInteresting == 0) && qPlayer.GetRoom() == ROOM_KIOSK) {
                    break;
                }
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && qPlayer.GetRoom() == ROOM_SABOTAGE) {
                    if (qPlayer.ArabTrust != 0) {
                        qPlayer.SpiderTrust = TRUE;
                        qPlayer.Items[ItemIndex] = 0xff;
                        qPlayer.ReformIcons();
                    }
                    StartDialog(TALKER_SABOTAGE, MEDIUM_AIR, 3000);
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_SPINNE);
                }
                break;

            case ITEM_DART:
                if ((Sim.Headlines.IsInteresting == 0) && qPlayer.GetRoom() == ROOM_KIOSK) {
                    break;
                }
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && qPlayer.GetRoom() == ROOM_WERBUNG &&
                    (Sim.Difficulty >= DIFF_NORMAL || Sim.Difficulty == DIFF_FREEGAME)) {
                    qPlayer.WerbungTrust = TRUE;
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                    StartDialog(TALKER_WERBUNG, MEDIUM_AIR, 8001);
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_DART);
                }
                break;

            case ITEM_DISKETTE:
                if ((Sim.Headlines.IsInteresting == 0) && qPlayer.GetRoom() == ROOM_KIOSK) {
                    break;
                }
                if ((qPlayer.LaptopVirus != 0) && (qPlayer.HasItem(ITEM_LAPTOP) != 0)) {
                    if (qPlayer.GetRoom() == ROOM_LAPTOP) {
                        qPlayer.LeaveRoom();
                    }

                    qPlayer.LaptopVirus = 0;
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_DISKETTE);
                }
                break;

            case ITEM_BH:
                if ((Sim.Headlines.IsInteresting == 0) && qPlayer.GetRoom() == ROOM_KIOSK) {
                    break;
                }
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && qPlayer.GetRoom() == ROOM_SHOP1) {
                    qPlayer.DutyTrust = TRUE;
                    qPlayer.Items[ItemIndex] = 0xff;
                    StartDialog(TALKER_DUTYFREE, MEDIUM_AIR, 801);
                    qPlayer.ReformIcons();
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_BH);
                }
                break;

            case ITEM_HUFEISEN:
                if ((Sim.Headlines.IsInteresting == 0) && qPlayer.GetRoom() == ROOM_KIOSK) {
                    break;
                }
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && qPlayer.GetRoom() == ROOM_RICKS) {
                    qPlayer.TrinkerTrust = TRUE;
                    qPlayer.Items[ItemIndex] = 0xff;
                    StartDialog(TALKER_TRINKER, MEDIUM_AIR, 800);
                    qPlayer.ReformIcons();
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_HUFEISEN);
                }
                break;

            case ITEM_PRALINEN:
                if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_PRALINEN);
                } else {
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                    PlayUniversalFx("eating.raw", Sim.Options.OptionEffekte);
                }
                break;

            case ITEM_PRALINEN_A:
                if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_PRALINEN_A);
                } else {
                    PlayUniversalFx("eating.raw", Sim.Options.OptionEffekte);
                    qPlayer.IsDrunk += 400;
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                }
                break;

            case ITEM_PAPERCLIP:
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && qPlayer.GetRoom() == ROOM_FRACHT && Sim.ItemGlue == 0 &&
                    (qPlayer.HasItem(ITEM_GLUE) == 0)) {
                    StartDialog(TALKER_FRACHT, MEDIUM_AIR, 1020);
                    qPlayer.DropItem(ITEM_PAPERCLIP);
                    Sim.ItemGlue = 1;
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_PAPERCLIP);
                }
                break;

            case ITEM_GLUE:
                if (qPlayer.GetRoom() == ROOM_AIRPORT) {
                    PERSON &qPerson = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)];

                    // Obere oder untere Ebene?
                    if (qPerson.Position.y >= 4000) {
                        // oben!
                        if (Sim.AddGlueSabotage(qPerson.Position, qPerson.Dir, qPlayer.NewDir, qPerson.Phase)) {
                            TEAKFILE Message;

                            Message.Announce(30);
                            Message << ATNET_SABOTAGE_DIRECT << SLONG(ITEM_GLUE) << qPerson.Position << qPerson.Dir << qPlayer.NewDir << qPerson.Phase;
                            SIM::SendMemFile(Message, 0);

                            qPlayer.DropItem(ITEM_GLUE);
                        }
                    } else {
                        MenuStart(MENU_REQUEST, MENU_REQUEST_NOGLUE);
                        MenuSetZoomStuff(XY(320, 220), 0.17, FALSE);
                    }
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_GLUE);
                }
                break;

            case ITEM_GLOVE:
                if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_GLOVE);
                }
                break;

            case ITEM_REDBULL:
                if (qPlayer.GetRoom() == ROOM_KIOSK) {
                    StartDialog(TALKER_KIOSK, MEDIUM_AIR, 1010);
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                    qPlayer.KioskTrust = 1;
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_REDBULL);
                } else {
                    PlayUniversalFx("gulps.raw", Sim.Options.OptionEffekte);
                    qPlayer.Items[ItemIndex] = 0xff;
                    qPlayer.ReformIcons();
                    qPlayer.Koffein += 20 * 120;
                    PLAYER::NetSynchronizeFlags();
                    SIM::SendSimpleMessage(ATNET_CAFFEINE, 0, PlayerNum, qPlayer.Koffein);
                }
                break;

            case ITEM_STINKBOMBE:
                if (qPlayer.GetRoom() == ROOM_AIRPORT) {
                    PERSON &qPerson = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)];

                    // Untere Ebene?
                    if (qPerson.Position.y < 4000) {
                        SIM::SendSimpleMessage(ATNET_SABOTAGE_DIRECT, 0, ITEM_STINKBOMBE, qPerson.Position.x, qPerson.Position.y);
                        Sim.AddStenchSabotage(qPerson.Position);

                        qPlayer.DropItem(ITEM_STINKBOMBE);
                    } else {
                        MenuStart(MENU_REQUEST, MENU_REQUEST_NOSTENCH);
                        MenuSetZoomStuff(XY(320, 220), 0.17, FALSE);
                    }
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_STINKBOMBE);
                }
                break;

            case ITEM_ZIGARRE:
                break;

            case ITEM_GLKOHLE:
                if (qPlayer.GetRoom() == ROOM_LAST_MINUTE) {
                    SLONG cnt = 0;
                    for (SLONG c = LastMinuteAuftraege.AnzEntries() - 1; c >= 0; c--) {
                        if (LastMinuteAuftraege[c].Praemie > 0) {
                            LastMinuteAuftraege[c].Praemie = 0;
                            qPlayer.NetUpdateTook(2, c);
                            cnt++;
                        }
                    }

                    if (cnt > 0) {
                        gUniversalFx.Stop();
                        gUniversalFx.ReInit("fireauft.raw");
                        gUniversalFx.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                    }
                } else if (qPlayer.GetRoom() == ROOM_REISEBUERO) {
                    SLONG cnt = 0;
                    for (SLONG c = ReisebueroAuftraege.AnzEntries() - 1; c >= 0; c--) {
                        if (ReisebueroAuftraege[c].Praemie > 0) {
                            ReisebueroAuftraege[c].Praemie = 0;
                            qPlayer.NetUpdateTook(1, c);
                            cnt++;
                        }
                    }

                    if (cnt > 0) {
                        gUniversalFx.Stop();
                        gUniversalFx.ReInit("fireauft.raw");
                        gUniversalFx.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                    }
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_GLKOHLE);
                }
                break;

            case ITEM_KOHLE:
                if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_KOHLE);
                }
                break;

            case ITEM_ZANGE:
                if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_ZANGE);
                } else if (qPlayer.GetRoom() == ROOM_AIRPORT) {
                    XY Position = Sim.Persons[Sim.Persons.GetPlayerIndex(qPlayer.PlayerNum)].Position;

                    if (ROOM_SECURITY == Airport.GetRuneParNear(XY(Position.x, Position.y), XY(22, 22), RUNE_2SHOP)) {
                        qPlayer.DropItem(ITEM_ZANGE);
                        PLAYER::NetSynchronizeItems();
                        qPlayer.WalkStopEx();
                        Sim.nSecOutDays = 3;

                        SIM::SendSimpleMessage(ATNET_SYNC_OFFICEFLAG, 0, 55, 3);

                        for (SLONG c = 0; c < 4; c++) {
                            if (Sim.Players.Players[c].IsOut == 0) {
                                Sim.Players.Players[c].SecurityFlags = 0;
                                Sim.Players.Players[c].NetSynchronizeFlags();
                            }
                        }
                    }
                }
                break;

            case ITEM_PARFUEM:
                if (qPlayer.GetRoom() == ROOM_WERKSTATT && Sim.Slimed != -1) {
                    for (SLONG d = 0; d < 6; d++) {
                        if (qPlayer.Items[d] == ITEM_PARFUEM) {
                            qPlayer.Items[d] = ITEM_XPARFUEM;
                        }
                    }
                } else if (DefaultDialogPartner != TALKER_NONE) {
                    StartDialog(DefaultDialogPartner, MEDIUM_AIR, 10000 + ITEM_PARFUEM);
                }
                break;

            case ITEM_XPARFUEM:
                if (qPlayer.GetRoom() == ROOM_AIRPORT) {
                    qPlayer.PlayerStinking = 2000;
                    qPlayer.DropItem(ITEM_XPARFUEM);
                    PLAYER::NetSynchronizeFlags();
                }
                break;

            default:
                TeakLibW_Exception(FNL, ExcCreateWindow);
            }
        }
    }
    // Klick auf Logo?
    else if (point.x < 78 && point.y >= WinP2.y - StatusLineSizeY && point.y < WinP2.y && (CheatTestGame == 0)) {
        if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && (MouseWait == 0) && Sim.Time > 9 * 60000) {
            gStatButton = 3;
            gStatButtonTimer = AtGetTime();

            if (qPlayer.IsLocationInQueue(ROOM_STATISTICS) == 0) {
                qPlayer.EnterRoom(ROOM_STATISTICS);
            } else {
                qPlayer.LeaveRoom();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// OnLButtonUp(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CStdRaum::OnLButtonUp(UINT /*unused*/, CPoint /*point*/) {
    if ((nOptionsOpen != 0) && (Sim.bNetwork != 0) && Sim.localPlayer < Sim.Players.Players.AnzEntries() &&
        Sim.Players.Players[Sim.localPlayer].GetRoom() != ROOM_OPTIONS && Sim.Players.Players[Sim.localPlayer].GetRoom() != 0) {
        return;
    }
    DefaultOnLButtonUp();
}

//--------------------------------------------------------------------------------------------
// OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CStdRaum::OnRButtonDown(UINT /*nFlags*/, CPoint point) {
    if ((nOptionsOpen != 0) && (Sim.bNetwork != 0) && Sim.localPlayer < Sim.Players.Players.AnzEntries() &&
        Sim.Players.Players[Sim.localPlayer].GetRoom() != ROOM_OPTIONS && Sim.Players.Players[Sim.localPlayer].GetRoom() != 0) {
        return;
    }

    // Dialog beenden?
    if (DialogPartner != TALKER_NONE) {
        if (DialogPartner == TALKER_WORLD) {
            StopDialog();
            MenuStart(MENU_FILOFAX, 2);
            CurrentMenu = MENU_BRANCHLIST;
            MenuPage = 0;
            MenuRepaint();
            return;
        }
        StopDialog();
    }
    // Klick auf Uhr?
    else if (point.x >= 78 && point.y >= 440 && point.x < 102 && point.y < 460 && (CheatTestGame == 0)) {
        if (Sim.Date > 2) {
            Sim.GiveHint(HINT_GAMESPEED);
        }
        Sim.Players.Players[PlayerNum].GameSpeed = (Sim.Players.Players[PlayerNum].GameSpeed + 3) % 4;

        SIM::SendSimpleMessage(ATNET_SETSPEED, 0, Sim.localPlayer, Sim.Players.Players[PlayerNum].GameSpeed);

        StatusCount = max(StatusCount, 3);
    }
    // Klick ins Inventar?
    else if (point.x >= WinP1.x + (WinP2.x - WinP1.x) * 4 / 10 && point.y >= WinP2.y - StatusLineSizeY && point.x < 634 && point.y < WinP2.y &&
             CurrentMenu == MENU_NONE && DialogPartner == TALKER_NONE) {
        SLONG ItemIndex = 0;

        ItemIndex = (point.x - 250) / 60;

        if (ItemIndex < 6 && Sim.Players.Players[PlayerNum].Items[ItemIndex] != 0xff) {
            MenuStart(MENU_REQUEST, MENU_REQUEST_KILLITEM, ItemIndex);
            MenuSetZoomStuff(XY(250 + ItemIndex * 60, 446), 0.17, FALSE);
        }
    } else if (CurrentMenu != MENU_NONE && point.y < 440) {
        if (CurrentMenu == MENU_PLAYERLIST || CurrentMenu == MENU_BRANCHLIST) {
            CurrentMenu = MENU_FILOFAX;
            MenuPage = 0;
            MenuPar1 = 2; // Handy
            MenuRepaint();
            if (Sim.Options.OptionEffekte != 0) {
                gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            }
            return;
        }

        if (CurrentMenu == MENU_WC_M || CurrentMenu == MENU_WC_F) {
            return;
        }

        if (CurrentMenu == MENU_LETTERS && (Sim.Options.OptionEffekte != 0)) {
            PaperFX.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
        }

        if (CurrentMenu == MENU_REQUEST && (MenuPar1 == MENU_REQUEST_THROWNOUT || MenuPar1 == MENU_REQUEST_THROWNOUT2)) {
            Sim.Players.Players[PlayerNum].WaitForRoom = 0;
            Sim.Players.Players[PlayerNum].ThrownOutOfRoom = 0;
        }

        MenuStop();
        RepaintText(TRUE);
    }
}

//--------------------------------------------------------------------------------------------
// void CGlobe::OnRButtonUp(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CStdRaum::OnRButtonUp(UINT /*unused*/, CPoint /*unused*/) {
    if ((nOptionsOpen != 0) && (Sim.bNetwork != 0) && Sim.localPlayer < Sim.Players.Players.AnzEntries() &&
        Sim.Players.Players[Sim.localPlayer].GetRoom() != ROOM_OPTIONS && Sim.Players.Players[Sim.localPlayer].GetRoom() != 0) {
        return;
    }
    DefaultOnRButtonUp();
}

//--------------------------------------------------------------------------------------------
// CStdRaum::OnPaint(void)
//--------------------------------------------------------------------------------------------
void CStdRaum::OnPaint() { OnPaint(FALSE); }
void CStdRaum::OnPaint(BOOL /*bHandyDialog*/) {
    ReferenceCursorPos = gMousePosition;

    if ((PleaseCancelTextBubble != 0) && (bHandy == 0) || (CheatAutoSkip != 0)) {
        PleaseCancelTextBubble = FALSE;
        if (IsDialogOpen() != 0) {
            PreLButtonDown(WasLButtonDownPoint);
        }
    }

    if ((bHandy == 0) && (WasLButtonDown != 0)) {
        if (IsDialogOpen() != 0) {
            MouseClickArea = WasLButtonDownMouseClickArea;
            MouseClickId = WasLButtonDownMouseClickId;
            MouseClickPar1 = WasLButtonDownMouseClickPar1;
            MouseClickPar2 = WasLButtonDownMouseClickPar2;

            PreLButtonDown(WasLButtonDownPoint);
        }

        WasLButtonDown = FALSE;
    }

    {
        SLONG c = 0;

        // Wird hier abgeschaltet und jedesmal wieder neu eingeschaltet. Oder auch nicht...
        CurrentTipType = TIP_NONE;

        if (PicBitmap.Size.x != 0) {
            RoomBm.BlitFrom(PicBitmap, WinP1.x, WinP1.y);
        }

        if (bHandy != 0) // Handy einblendung?
        {
            DWORD Time = AtGetTime();

            if (LastScrollTime == -1) {
                LastScrollTime = Time;
            }

            // Dialogbild rein oder rausscrollen:
            if (TempScreenScroll > 378 && TempScreenScrollV < 0) {
                TempScreenScroll += TempScreenScrollV * (Time - LastScrollTime) / 3;
            } else if (TempScreenScroll < 800 && TempScreenScrollV > 0) {
                TempScreenScroll += TempScreenScrollV * (Time - LastScrollTime) / 3;
            }

            if (TempScreenScroll <= 378 && TempScreenScrollV < 0) {
                TempScreenScroll = 378;
                TempScreenScrollV = 0;
                RightAirportClip = 378;
            }
            if (TempScreenScroll >= 800 && TempScreenScrollV > 0) {
                TempScreenScrollV = 0;
            }

            LastScrollTime = Time;
        } else {
            // Alten Handy-Schirm ggf. zerstören?
            if (PlayerNum >= 0 && (Sim.Players.Players[PlayerNum].DialogWin != nullptr) && (Sim.Players.Players[PlayerNum].DialogWin)->TempScreenScrollV == 0 &&
                (Sim.Players.Players[PlayerNum].DialogWin)->TempScreenScroll >= 800 && (IsDialogOpen() == 0)) {
                IgnoreNextPostPaintPump = 3;
                delete Sim.Players.Players[PlayerNum].DialogWin;
                Sim.Players.Players[PlayerNum].DialogWin = nullptr;
            }
        }

        DWORD Time = AtGetTime();

        for (c = 0; c < 5; c++) {
            if (Time > BackgroundCount[c]) {
                BackgroundFX[c].Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7 * BackgroundLautstaerke[c] / 100);
                BackgroundCount[c] = Time + BackgroundWait[c] * (rand() % 100 + 50) / 100;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Stellt einen, welchen Tool-Tip man haben will:
//--------------------------------------------------------------------------------------------
void CStdRaum::SetTip(SBBM *pBitmapSource, void *pBitmapSource2, BOOL ForceRedraw, XY Pos, SLONG TipType, SLONG TipId, SLONG TipPar1, SLONG TipPar2) {
    pTipOriginBm = pBitmapSource;
    pTipOriginBm2 = pBitmapSource2;
    ForceRedrawTip |= ForceRedraw;
    CurrentTipType = TipType;
    CurrentTipId = TipId;
    CurrentTipIdPar1 = TipPar1;
    CurrentTipIdPar2 = TipPar2;
    TipPos = Pos;
}

//--------------------------------------------------------------------------------------------
// Mitteilung, daß Daten aktualisiert wurden. Ggf. muß der Tip neugezeichnet werden:
//--------------------------------------------------------------------------------------------
void CStdRaum::AnnouceTipDataUpdate(SLONG TipType) {
    if (TipType == LastTipType) {
        ForceRedrawTip = TRUE;
    }
}

//--------------------------------------------------------------------------------------------
// Malt den ToolTip neu:
//--------------------------------------------------------------------------------------------
void CStdRaum::RepaintTip() {
    switch (CurrentTipType) {
    case TIP_NONE:
        break;

    case TIP_CITY:
        DrawCityTip(TipBm, CurrentTipId);
        break;

    case TIP_PLANE:
        if (CurrentTipIdPar1 == 0) {
            DrawPlaneTip(TipBm, pTipOriginBm, nullptr, &Sim.Players.Players[PlayerNum].Planes[CurrentTipId]);
        }
        // DrawPlaneTip (TipBm, pTipOriginBm, &PlaneTypes[Sim.Players.Players[PlayerNum].Planes[CurrentTipId].TypeId],
        // &Sim.Players.Players[PlayerNum].Planes[CurrentTipId]);
        break;

    case TIP_BUYPLANE:
        break;

    case TIP_AUFTRAG:
        DrawAuftragTip(PlayerNum, TipBm, static_cast<SBBMS *>(pTipOriginBm2), pTipOriginBm, &Sim.Players.Players[PlayerNum].Auftraege[CurrentTipId],
                       CurrentTipIdPar1, CurrentTipIdPar2, XY(0, 0));
        break;

    case TIP_BUYAUFTRAG:
        DrawAuftragTip(PlayerNum, TipBm, static_cast<SBBMS *>(pTipOriginBm2), pTipOriginBm,
                       (CurrentTipId == -1) ? nullptr : (&LastMinuteAuftraege[CurrentTipId]), CurrentTipIdPar1, CurrentTipIdPar2, XY(0, 0));
        break;

    case TIP_BUYAUFTRAGR:
        DrawAuftragTip(PlayerNum, TipBm, static_cast<SBBMS *>(pTipOriginBm2), pTipOriginBm,
                       (CurrentTipId == -1) ? nullptr : (&ReisebueroAuftraege[CurrentTipId]), CurrentTipIdPar1, CurrentTipIdPar2, XY(0, 0));
        break;

    case TIP_BUYFRACHT:
        DrawFrachtTip(PlayerNum, TipBm, static_cast<SBBMS *>(pTipOriginBm2), pTipOriginBm, (CurrentTipId == -1) ? nullptr : (&gFrachten[CurrentTipId]), 0,
                      CurrentTipIdPar1, 777, CurrentTipIdPar2, XY(0, 0));
        break;

    case TIP_FRACHT:
        DrawFrachtTip(PlayerNum, TipBm, static_cast<SBBMS *>(pTipOriginBm2), pTipOriginBm, &Sim.Players.Players[PlayerNum].Frachten[CurrentTipId], 0,
                      CurrentTipIdPar1, 777, CurrentTipIdPar2, XY(0, 0));
        break;

    case TIP_ROUTE:
        break;
    case TIP_BUYROUTE:
        break;
    case TIP_AKTIE:
        break;
    case TIP_ITEM:
        break;
    default:
        hprintf("StdRaum.cpp: Default case should not be reached.");
        DebugBreak();
    }

    CStdRaum::LastTipType = CStdRaum::CurrentTipType;
    CStdRaum::LastTipId = CStdRaum::CurrentTipId;
    CStdRaum::LastTipIdPar1 = CStdRaum::CurrentTipIdPar1;
    CStdRaum::LastTipIdPar2 = CStdRaum::CurrentTipIdPar2;

    ForceRedrawTip = FALSE;
}

//--------------------------------------------------------------------------------------------
// Konvertiert die Koordinaten auf einheitlich 640x440, FALSE wenn außerhalb
//--------------------------------------------------------------------------------------------
BOOL CStdRaum::ConvertMousePosition(const XY &WindowsBased, XY *RoomBased) const {
    // Koordinaten für kleine Fenster konvertieren:
    *RoomBased = WindowsBased + XY(-WinP1.x, -WinP1.y);

    // Klick außerhalb vom Fenster?
    if (WindowsBased.x < WinP1.x || WindowsBased.y < WinP1.y || WindowsBased.x > WinP2.x || WindowsBased.y > WinP2.y) {
        return (FALSE); // außerhalb?
    }
    return (TRUE); // innerhalb
}

//--------------------------------------------------------------------------------------------
// Startet ein Onscreen-Menü:
//--------------------------------------------------------------------------------------------
void CStdRaum::MenuStart(SLONG MenuType, SLONG MenuPar1, SLONG MenuPar2, SLONG MenuPar3) {
    SLONG c = 0;

    CWaitCursorNow wc; // CD-Cursor anzeigen
    TimeAtStart = AtGetTime();

    gMouseScroll = 0;

    if (TalkingSpeechFx != 0) {
        SpeechFx.Stop();
        TalkingSpeechFx = FALSE;
    }

    // Ein Menü unterbricht den Dialog; wenn man danach in den Dialog zurückkehrt, dann bleibt der Dialogpartner interessiert:
    if (IsDialogOpen() != 0) {
        if (MenuDialogReEntryB != -1 || MenuType == MENU_BUYPLANE || MenuType == MENU_BUYXPLANE) {
            if (pSmackerPartner != nullptr) {
                pSmackerPartner->SetDesiredMood(SPM_LISTENING);
            }
        } else {
            if (pSmackerPartner != nullptr) {
                pSmackerPartner->SetDesiredMood(SPM_IDLE);
            }
            Sim.Players.Players[PlayerNum].Messages.StopDialog();
        }
    }

    CStdRaum::CurrentMenu = MenuType;
    CStdRaum::MenuPar1 = MenuPar1;
    CStdRaum::MenuPar2 = MenuPar2;
    CStdRaum::MenuPar3 = MenuPar3;

    CStdRaum::ZoomCounter = 0;
    CStdRaum::MinimumZoom = 1.0;

    switch (CurrentMenu) {
    case MENU_BANK:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("kredit.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "KREDIT OK CANCEL");
        PaperTearFX.ReInit("pap2.raw");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_BUYKEROSIN:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("buykero.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "FORM OK CANCEL");
        PaperTearFX.ReInit("pap2.raw");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_FILOFAX:
        if (MenuPar1 == 2 && Sim.Players.Players[PlayerNum].GetRoom() == ROOM_BURO_A + PlayerNum * 10) {
            (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_LISTENING, SPM_IDLE);
        }
        if (MenuPar1 == 1) {
            pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("adressen.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        }
        if (MenuPar1 == 2) {
            pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("telefons.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        }
        MenuBms.ReSize(pMenuLib1, "BLOCK LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_KEROSIN:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("oilmenu.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "OILMENU");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_PERSONAL:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("personal.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "ORDNER1 ORDNER2 ORDNER3 KREUZLE");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = MenuPageMax = MenuInfo = MenuInfo2 = MenuInfo3 = 0;
        MenuRemapper.ReSize(Workers.Workers.AnzEntries());
        for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
            if (((Workers.Workers[c].Employer == PlayerNum && MenuPar2 == 2) || (Workers.Workers[c].Employer == WORKER_JOBLESS && MenuPar2 == 1)) &&
                (Workers.Workers[c].Typ == MenuPar1 || (Workers.Workers[c].Typ < WORKER_PILOT && MenuPar1 < WORKER_PILOT))) {
                MenuInfo += Workers.Workers[c].Gehalt;
                MenuRemapper[MenuPageMax] = c;
                MenuPageMax++;
            }
        }

        if (MenuPageMax != 0) {
            MenuInfo /= MenuPageMax;
        }
        MenuRemapper.ReSize(MenuPageMax);
        break;

    case MENU_SABOTAGEPLANE: {
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("list.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "BLOCK LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuDataTable.FillWithPlanes(&Sim.Players.Players[Sim.Players.Players[PlayerNum].ArabOpfer].Planes);
        MenuPageMax = (MenuDataTable.AnzRows - 1) / 13;

        for (SLONG c = 0; c < MenuDataTable.AnzRows; c++) {
            MenuDataTable.ValueFlags[c * 4 + 0] = FALSE;
        }
    } break;

    case MENU_SABOTAGEROUTE: {
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("list.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "BLOCK LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuDataTable.FillWithRouten(&::Routen, &Sim.Players.Players[Sim.Players.Players[PlayerNum].ArabOpfer].RentRouten);
        MenuPageMax = (MenuDataTable.AnzRows - 1) / 13;

        for (SLONG c = 0; c < MenuDataTable.AnzRows; c++) {
            MenuDataTable.ValueFlags[c * 4 + 0] = FALSE;
        }
    } break;

    case MENU_SELLPLANE:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("list.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "BLOCK LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuDataTable.FillWithPlanes(&Sim.Players.Players[PlayerNum].Planes);
        MenuPageMax = (MenuDataTable.AnzRows - 1) / 13;
        break;

    case MENU_BUYPLANE:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("list.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "BLOCK LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuDataTable.FillWithPlaneTypes();
        MenuPageMax = ((MenuDataTable.AnzRows - 1) / 13) * 13;
        break;

    case MENU_BUYXPLANE:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("list.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "BLOCK LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuPar4 = "";
        MenuDataTable.FillWithXPlaneTypes();
        MenuPageMax = ((MenuDataTable.AnzRows - 1) / 13) * 13;
        break;

    case MENU_CLOSED:
        bgWarp = FALSE;
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("request.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "REQUEST");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_EXTRABLATT:
        bgWarp = FALSE;
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("extrablt.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "ROT1 ROT2 ROT3 ROT4 ROT5 ROT6 ROT7 ROT8 BACKGND SUN_SUN SUN_FAL SUN_PHO SUN_HON FAL_SUN FAL_FAL FAL_PHO FAL_HON PHO_SUN "
                                  "PHO_FAL PHO_PHO PHO_HON HON_SUN HON_FAL HON_PHO HON_HON");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_REQUEST:
        bgWarp = FALSE;
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("request.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "REQUEST");
        OnscreenBitmap.ReSize(MenuBms[0].Size);

        if (MenuPar1 == MENU_REQUEST_THROWNOUT || MenuPar1 == MENU_REQUEST_THROWNOUT2) {
            Sim.Players.Players[PlayerNum].WaitForRoom = Sim.Players.Players[PlayerNum].ThrownOutOfRoom;
            Sim.Players.Players[PlayerNum].ThrownOutOfRoom = 0;
        }
        break;

    case MENU_RENAMEPLANE:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("rename.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "MENU");
        Optionen[0] = Sim.Players.Players[PlayerNum].Planes[MenuPar1].Name;
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_RENAMEEDITPLANE: {
        PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("rename.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "MENU");
        if (qPlayer.GetRoom() == ROOM_EDITOR) {
            Optionen[0] = (dynamic_cast<CEditor *>(qPlayer.LocationWin))->Plane.Name;
        }
        OnscreenBitmap.ReSize(MenuBms[0].Size);
    } break;

    case MENU_ENTERTCPIP:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("rename.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "MENU");
        Optionen[0] = "";
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_BROADCAST:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("broad.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "MENU PL0 PL1 PL2 PL3 PL4 PL5 PL6 PL7");
        Optionen[0] = "";
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        CStdRaum::MenuPar1 = 15;
        break;

    case MENU_GAMEOVER:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("gameover.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "GAMEOVER OKAY");
        OnscreenBitmap.ReSize(640, 480);
        {
            SB_CBitmapKey SrcKey(PrimaryBm.PrimaryBm);
            SB_CBitmapKey TgtKey(*OnscreenBitmap.pBitmap);

            for (SLONG y = 0; y < 480; y++) {
                memcpy(static_cast<char *>(TgtKey.Bitmap) + y * TgtKey.lPitch, static_cast<char *>(SrcKey.Bitmap) + y * SrcKey.lPitch, 640 * 2);
            }
        }
        {
            SB_CColorFX GreyFX(SB_COLORFX_GREY, 1, OnscreenBitmap.pBitmap);

            GreyFX.Apply(0, OnscreenBitmap.pBitmap);
        }
        MenuInfo = AtGetTime();
        break;

    case MENU_LETTERS:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("letter.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "LETTER RETURN NEXT OK LOECHEN RETURN_F NEXT_F FAX SALZ SKELETT REIFEN FEUER SUPERMAN KAPUTT KAPUTT STREIK");
        MenuPage = 0;
        OnscreenBitmap.ReSize(MenuBms[0].Size, CREATE_SYSMEM);
        PaperFX.ReInit("pap3.raw");
        PaperTearFX.ReInit("pap2.raw");
        if (Sim.Options.OptionEffekte != 0) {
            PaperFX.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
        }
        break;

    case MENU_SABOTAGEFAX:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("fax.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "FAX SALZ SKELETT REIFEN FEUER SUPERMAN KAPUTT KAPUTT STREIK");
        MenuPage = 0;
        OnscreenBitmap.ReSize(MenuBms[0].Size, CREATE_SYSMEM);
        PaperFX.ReInit("pap3.raw");
        PaperTearFX.ReInit("pap2.raw");
        if (Sim.Options.OptionEffekte != 0) {
            PaperFX.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
        }
        break;

    case MENU_ADROUTE:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("listrout.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "BLOCK LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuDataTable.FillWithRouten(&Routen, &Sim.Players.Players[PlayerNum].RentRouten, TRUE);
        MenuPageMax = (MenuDataTable.AnzRows - 1) / 13;
        break;

    case MENU_AKTIE:
        if (MenuPar2 == 0) {
            MenuInfo = Sim.Players.Players[MenuPar1].AnzAktien;
            for (SLONG c = 0; c < 4; c++) {
                if (Sim.Players.Players[c].IsOut == 0) {
                    MenuInfo -= Sim.Players.Players[c].OwnsAktien[MenuPar1];
                }
            }
            MenuInfo /= 2;
        } else if (MenuPar2 == 1) {
            MenuInfo = Sim.Players.Players[PlayerNum].OwnsAktien[MenuPar1] / 2;
        }
        [[fallthrough]];
    case MENU_SETRENDITE:
        if (CurrentMenu == MENU_SETRENDITE) {
            MenuInfo = Sim.Players.Players[PlayerNum].Dividende;
        }
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("form.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "FORM OK CANCEL");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        PaperTearFX.ReInit("pap2.raw");
        break;

    case MENU_FLUEGE:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("fluege.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "MONITOR BLINK1 BLINK2 DOT1 DOT2 DOT3 DOT4");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_AUSLANDSAUFTRAG:
        Sim.NetRefill(4, MenuPar1);
        Sim.NetRefill(5, MenuPar1);
        AuslandsAuftraege[MenuPar1].RefillForAusland(MenuPar1);
        AuslandsFrachten[MenuPar1].RefillForAusland(MenuPar1);
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("listauft.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "BLOCK LEFT RIGHT");
        MenuBms2.ReSize(5);
        MenuBms2[0].ReSize(pMenuLib1, "PL_B00", 1 + 8);
        MenuBms2[1].ReSize(pMenuLib1, "PL_V00", 1 + 8);
        MenuBms2[2].ReSize(pMenuLib1, "PL_R00", 1 + 8);
        MenuBms2[3].ReSize(pMenuLib1, "PL_J00", 1 + 8);
        MenuBms2[4].ReSize(pMenuLib1, "PL_SW00", 1 + 8);
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuPar2 = -1;
        break;

    case MENU_WC_F:
    case MENU_WC_M:
        OnscreenBitmap.ReSize(gCityMarkerBm.Size, CREATE_SYSMEM);
        break;

    case MENU_PLANEREPAIRS:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("wartung.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "WARTUNG LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuInfo3 = 100;
        MenuDataTable.FillWithPlanes(&Sim.Players.Players[PlayerNum].Planes, FALSE);
        MenuPageMax = (MenuDataTable.AnzRows - 1) / 13;
        break;

    case MENU_SECURITY:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("wartung.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "WARTUNG LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuInfo3 = 100;
        MenuPageMax = 1;
        break;

    case MENU_PANNEN:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("wartung.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "WARTUNG LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuInfo3 = -1;
        MenuDataTable.FillWithPlanes(&Sim.Players.Players[PlayerNum].Planes, 2);
        MenuPageMax = (MenuDataTable.AnzRows - 1) / 13;
        break;

    case MENU_PLANEJOB:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("list.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "BLOCK LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuDataTable.FillWithPlanes(&Sim.Players.Players[PlayerNum].Planes, FALSE);
        MenuPageMax = (MenuDataTable.AnzRows - 1) / 13;
        break;

    case MENU_PLANECOSTS:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("list.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "BLOCK LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 0;
        MenuDataTable.FillWithPlanes(&Sim.Players.Players[PlayerNum].Planes, 99);
        MenuPageMax = (MenuDataTable.AnzRows - 1) / 13;
        break;

    case MENU_KONTOAUSZUG:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("konto.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "TIPMONEY LEFT RIGHT");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        MenuPage = 9; // Seite 9: Einträge 90-99; Seite 8: Einträge 80-89; ...
        break;

    case MENU_QUITMESSAGE:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("exitmenu.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "EXITMENU");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_CHAT:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("glchat.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "CHAT TEXTAREA");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        Optionen[0] = "";
        MenuBms[1].PrintAt(
            bprintf(StandardTexte.GetS(TOKEN_MISC, 3004), Sim.Players.Players[MenuPar1].NameX.c_str(), Sim.Players.Players[MenuPar1].AirlineX.c_str()),
            FontNormalGrey, TEC_FONT_LEFT, 6, 119, 279, 147);
        MenuBms[1].ShiftUp(10);
        break;

    case MENU_BRIEFING:
        Sim.Players.UpdateStatistics();
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("briefing.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "MENU PLAYER0 PLAYER1 PLAYER2 PLAYER3 PLAYER4 PLAYER5 PLAYER6 PLAYER7 AUSRUF SABOT BRIFO01 BRIFO02 BRIAB01 BRIAB02");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    case MENU_CALLITADAY:
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("network3.gli", GliPath)), &pMenuLib1, L_LOCMEM);
        MenuBms.ReSize(pMenuLib1, "MENU PL0 PL1 PL2 PL3 PL4 PL5 PL6 PL7");
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        break;

    default:
        return;
    }

    MenuRepaint();
    MenuPos = (XY(640, 440) - OnscreenBitmap.Size) / SLONG(2);
}

//--------------------------------------------------------------------------------------------
// Zeichnet die Offscreen-Bitmap neu:
//--------------------------------------------------------------------------------------------
void CStdRaum::MenuRepaint() {
    SLONG c = 0;

    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    switch (CurrentMenu) {
    case MENU_BANK:
        if (qPlayer.GetRoom() == ROOM_BANK) {
            SB_CFont &qFontBankBlack = (dynamic_cast<Bank *>(this))->FontBankBlack;
            SB_CFont &qFontBankRed = (dynamic_cast<Bank *>(this))->FontBankRed;

            OnscreenBitmap.BlitFrom(MenuBms[0]);

            // Name:
            OnscreenBitmap.PrintAt(qPlayer.Name, qFontBankRed, TEC_FONT_LEFT, 58, 17, 318, 144);

            // Kredit (alt):
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_KONTO, 1000), qFontBankBlack, TEC_FONT_LEFT, 58, 35, 318, 144);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_KONTO, 1001), qFontBankBlack, TEC_FONT_LEFT, 58, 45, 318, 144);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_KONTO, 1002), qFontBankBlack, TEC_FONT_LEFT, 58, 55, 318, 144);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_KONTO, 1003), qFontBankBlack, TEC_FONT_LEFT, 58, 65, 318, 144);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(qPlayer.Credit), qFontBankBlack, TEC_FONT_RIGHT, 58, 35, 270, 144);
            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(qPlayer.SollZins), qFontBankBlack, TEC_FONT_RIGHT, 58, 45, 270, 144);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(qPlayer.SollZins * qPlayer.Credit / 1200), qFontBankBlack, TEC_FONT_RIGHT, 58, 55, 270, 144);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(qPlayer.SollZins * qPlayer.Credit / 100), qFontBankBlack, TEC_FONT_RIGHT, 58, 65, 270, 144);

            // Neuaufname:
            if (MenuPar2 == 1) {
                OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_KONTO, 1004), FontSmallBlack, TEC_FONT_LEFT, 58, 83, 318, 144);
            } else {
                OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_KONTO, 1005), FontSmallBlack, TEC_FONT_LEFT, 58, 83, 318, 144);
            }

            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString(MenuPar1), FontSmallBlack, TEC_FONT_RIGHT, 58, 83, 270, 144);

            if (MenuPar2 == 2) {
                MenuPar1 = -MenuPar1;
            }

            // Kredit (neu):
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_KONTO, 1006), qFontBankBlack, TEC_FONT_LEFT, 58, 103, 318, 144);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_KONTO, 1001), qFontBankBlack, TEC_FONT_LEFT, 58, 113, 318, 144);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_KONTO, 1002), qFontBankBlack, TEC_FONT_LEFT, 58, 123, 318, 144);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_KONTO, 1003), qFontBankBlack, TEC_FONT_LEFT, 58, 133, 318, 144);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(qPlayer.Credit + MenuPar1), qFontBankBlack, TEC_FONT_RIGHT, 58, 103, 270, 144);
            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(qPlayer.SollZins), qFontBankBlack, TEC_FONT_RIGHT, 58, 113, 270, 144);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(qPlayer.SollZins * (qPlayer.Credit + MenuPar1) / 1200), qFontBankBlack, TEC_FONT_RIGHT, 58, 123,
                                   270, 144);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(qPlayer.SollZins * (qPlayer.Credit + MenuPar1) / 100), qFontBankBlack, TEC_FONT_RIGHT, 58, 133,
                                   270, 144);

            if (MenuPar2 == 2) {
                MenuPar1 = -MenuPar1;
            }
        }
        break;

    case MENU_KEROSIN:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 1030), FontSmallBlack, TEC_FONT_LEFT, 18, 28, 250, 320);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 1031), FontSmallBlack, TEC_FONT_LEFT, 18, 41, 250, 320);

        OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString(Sim.Kerosin), FontSmallBlack, TEC_FONT_LEFT, 160, 28, 250, 320);
        OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString(SLONG(qPlayer.TankPreis)), FontSmallBlack, TEC_FONT_LEFT, 160, 41, 250, 320);

        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 1000), FontSmallBlack, TEC_FONT_LEFT, 18, 8, 250, 320);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 1001), FontSmallBlack, TEC_FONT_LEFT, 18, 58, 250, 320);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 1002), FontSmallBlack, TEC_FONT_LEFT, 18, 71, 250, 320);

        if (gLanguage == LANGUAGE_O) {
            OnscreenBitmap.PrintAt(bprintf("%s Barris", (LPCTSTR)Insert1000erDots(qPlayer.Tank)), FontSmallBlack, TEC_FONT_LEFT, 160, 58, 250, 320);
        } else {
            OnscreenBitmap.PrintAt(bprintf("%s Barrel", (LPCTSTR)Insert1000erDots(qPlayer.Tank)), FontSmallBlack, TEC_FONT_LEFT, 160, 58, 250, 320);
        }
        if (gLanguage == LANGUAGE_O) {
            OnscreenBitmap.PrintAt(bprintf("%s Barris (%li%%)", (LPCTSTR)Insert1000erDots(qPlayer.TankInhalt), qPlayer.TankInhalt * 100 / qPlayer.Tank),
                                   FontSmallBlack, TEC_FONT_LEFT, 160, 71, 250, 320);
        } else {
            OnscreenBitmap.PrintAt(bprintf("%s Barrel (%li%%)", (LPCTSTR)Insert1000erDots(qPlayer.TankInhalt), qPlayer.TankInhalt * 100 / qPlayer.Tank),
                                   FontSmallBlack, TEC_FONT_LEFT, 160, 71, 250, 320);
        }

        if (qPlayer.TankInhalt != 0) {
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 1010 + qPlayer.TankOpen), FontSmallBlack, TEC_FONT_LEFT, 18, 100, 250, 320);
        } else {
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 1012), FontSmallBlack, TEC_FONT_LEFT, 18, 100, 250, 320);
        }

        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 1020), FontSmallBlack, TEC_FONT_LEFT, 18, 130 + 13, 250, 320);
        break;

    case MENU_FILOFAX: // The telefone filofax:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        for (c = 0; c < 18; c++) {
            if (MenuPar1 == 1) // Filofax
            {
                if (FilofaxRoomRemapper[c] == 255) {
                    break;
                }
                if (qPlayer.WasInRoom[static_cast<SLONG>(FilofaxRoomRemapper[c])] != 0) {
                    if (qPlayer.IsLocationInQueue(FilofaxRoomRemapper[c]) == 0) {
                        // OnscreenBitmap.PrintAt (StandardTexte.GetS (TOKEN_FILOFAX, 1000+c), FontSmallBlack, TEC_FONT_LEFT, 34, 8+c*13, 204, 253);
                        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_FILOFAX, 1000 + c), FontSmallBlack, TEC_FONT_LEFT, 34, 7 + c * 13, 204, 256);
                    } else {
                        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_FILOFAX, 1000 + c), FontSmallGrey, TEC_FONT_LEFT, 34, 7 + c * 13, 204, 256);
                    }
                }
            } else // Handy
            {
                if (HandyRoomRemapper[c] == 255) {
                    break;
                }
                if (qPlayer.WasInRoom[static_cast<SLONG>(HandyRoomRemapper[c])] != 0) {
                    if (qPlayer.IsLocationInQueue(HandyRoomRemapper[c]) == 0) {
                        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_FILOFAX, 2000 + c), FontSmallBlack, TEC_FONT_LEFT, 34, 7 + c * 13, 204, 210);
                    } else {
                        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_FILOFAX, 2000 + c), FontSmallGrey, TEC_FONT_LEFT, 34, 7 + c * 13, 204, 210);
                    }
                }
            }
        }
        break;

    case MENU_BRANCHLIST: {
        SLONG n = 0;

        for (c = n = 0; c < Cities.AnzEntries(); c++) {
            if (Cities.IsInAlbum(c) != 0) {
                SLONG d = 0;
                SLONG coop = 0;

                for (d = 0; d < 4; d++) {
                    if ((d == Sim.localPlayer || (Sim.Players.Players[d].Kooperation[Sim.localPlayer] != 0)) &&
                        (Sim.Players.Players[d].RentCities.RentCities[c].Rang != 0U) && Sim.Players.Players[d].IsOut == 0) {
                        coop = 1;
                    }
                }

                if ((coop != 0) && c != SLONG(Cities(Sim.HomeAirportId))) {
                    n++;
                }
            }
        }

        MenuRemapper.ReSize(n);

        for (c = n = 0; c < Cities.AnzEntries(); c++) {
            if (Cities.IsInAlbum(c) != 0) {
                SLONG d = 0;
                SLONG coop = 0;

                for (d = 0; d < 4; d++) {
                    if ((d == Sim.localPlayer || (Sim.Players.Players[d].Kooperation[Sim.localPlayer] != 0)) &&
                        (Sim.Players.Players[d].RentCities.RentCities[c].Rang != 0U) && Sim.Players.Players[d].IsOut == 0) {
                        coop = 1;
                    }
                }

                if ((coop != 0) && c != SLONG(Cities(Sim.HomeAirportId))) {
                    MenuRemapper[n++] = c;
                }
            }
        }
    }

        OnscreenBitmap.BlitFrom(MenuBms[0]);
        OnscreenBitmap.BlitFrom(MenuBms[1], 12, 187);
        if (MenuPage < SLONG(MenuRemapper.AnzEntries() - 1) / 15) {
            OnscreenBitmap.BlitFrom(MenuBms[2], 190, 196);
        }

        for (c = MenuPage * 15; c < MenuRemapper.AnzEntries(); c++) {
            SLONG line = c - MenuPage * 15;

            if (Sim.Players.Players[Sim.localPlayer].RentCities.RentCities[MenuRemapper[c]].Rang != 0U) {
                OnscreenBitmap.PrintAt(Cities[MenuRemapper[c]].Name, FontSmallBlack, TEC_FONT_LEFT, 34, 7 + line * 13, 204, 210);
            } else {
                for (SLONG d = 0; d < 4; d++) {
                    if ((Sim.Players.Players[d].Kooperation[Sim.localPlayer] != 0) &&
                        (Sim.Players.Players[d].RentCities.RentCities[MenuRemapper[c]].Rang != 0U) && Sim.Players.Players[d].IsOut == 0) {
                        OnscreenBitmap.PrintAt(Cities[MenuRemapper[c]].Name + " (" + Sim.Players.Players[d].Abk + ")", FontSmallBlack, TEC_FONT_LEFT, 34,
                                               7 + line * 13, 204, 210);
                        break;
                    }
                }
            }
        }
        break;

    case MENU_PLAYERLIST:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        OnscreenBitmap.BlitFrom(MenuBms[1], 12, 187);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 1130), FontSmallBlack, TEC_FONT_LEFT, 34, 7 + 0 * 13, 204, 210);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 1131), FontSmallBlack, TEC_FONT_LEFT, 34, 7 + 5 * 13, 204, 210);
        for (c = 0; c < 3; c++) {
            PLAYER &qPlayer = Sim.Players.Players[c + static_cast<SLONG>(c >= PlayerNum)];

            if (qPlayer.IsOut != 0) {
                OnscreenBitmap.PrintAt(qPlayer.NameX, FontSmallGrey, TEC_FONT_LEFT, 40, 7 + (c + 1) * 13, 204, 210);
            } else {
                OnscreenBitmap.PrintAt(qPlayer.NameX, FontSmallBlack, TEC_FONT_LEFT, 40, 7 + (c + 1) * 13, 204, 210);
            }

            if ((qPlayer.IsOut != 0) || (qPlayer.HasItem(ITEM_HANDY) == 0)) {
                OnscreenBitmap.PrintAt(qPlayer.NameX, FontSmallGrey, TEC_FONT_LEFT, 40, 7 + (c + 6) * 13, 204, 210);
            } else {
                OnscreenBitmap.PrintAt(qPlayer.NameX, FontSmallBlack, TEC_FONT_LEFT, 40, 7 + (c + 6) * 13, 204, 210);
            }
        }
        break;

    case MENU_PERSONAL:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        if (MenuPage == 0) {
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_JOBS, 1000 + MenuPar1), FontDialogInk, TEC_FONT_CENTERED, 40, 38, 250, 319);
            OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_JOBS, 5002 - MenuPar2), MenuPageMax), FontSmallBlack, TEC_FONT_CENTERED, 40, 70, 250, 319);
            if (MenuInfo != 0) {
                OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_JOBS, 5010), MenuInfo), FontSmallBlack, TEC_FONT_LEFT, 40, 88, 250, 319);
            }
        } else {
            OnscreenBitmap.BlitFrom(MenuBms[1]);
            OnscreenBitmap.PrintAt(Workers.Workers[MenuRemapper[MenuPage - 1]].Name, FontDialogInk, TEC_FONT_LEFT, 40, 10, 250, 319);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_JOBS, 2000 + Workers.Workers[MenuRemapper[MenuPage - 1]].Typ +
                                                                      Workers.Workers[MenuRemapper[MenuPage - 1]].Geschlecht * 1000),
                                   FontSmallBlack, TEC_FONT_LEFT, 40, 80, 250, 319);

            auto gehalt = Workers.Workers[MenuRemapper[MenuPage - 1]].Gehalt;
            auto origGehalt = Workers.Workers[MenuRemapper[MenuPage - 1]].OriginalGehalt;
            if (gehalt > origGehalt) {
                OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_JOBS, 5050), gehalt), FontNormalGreen, TEC_FONT_LEFT, 40, 93, 250, 319);
            } else if (gehalt < origGehalt) {
                OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_JOBS, 5050), gehalt), FontSmallRed, TEC_FONT_LEFT, 40, 93, 250, 319);
            } else {
                OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_JOBS, 5050), gehalt), FontSmallBlack, TEC_FONT_LEFT, 40, 93, 250, 319);
            }

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_JOBS, 5020 + Workers.Workers[MenuRemapper[MenuPage - 1]].Talent / 10), FontSmallBlack,
                                   TEC_FONT_LEFT, 40, 106, 250, 319);

            // Auf welchem Flugzeug arbeitet er?
            if (Workers.Workers[MenuRemapper[MenuPage - 1]].Employer == PlayerNum && Workers.Workers[MenuRemapper[MenuPage - 1]].Typ >= WORKER_PILOT) {
                if (Workers.Workers[MenuRemapper[MenuPage - 1]].PlaneId == -1) {
                    OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_JOBS, 1100), FontSmallBlack, TEC_FONT_LEFT, 40, 155, 250, 319);
                } else {
                    CString planeName = qPlayer.Planes[Workers.Workers[MenuRemapper[MenuPage - 1]].PlaneId].Name;
                    OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_JOBS, 1101), (LPCTSTR)planeName), FontSmallBlack, TEC_FONT_LEFT, 40, 155, 250, 319);
                }
                OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_JOBS, 1102), FontSmallBlack, TEC_FONT_LEFT, 40, 181, 250, 319);
            }

            OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_JOBS, 1105), Workers.Workers[MenuRemapper[MenuPage - 1]].Alter), FontSmallBlack,
                                   TEC_FONT_LEFT, 40, 119, 250, 319);

            if (Workers.Workers[MenuRemapper[MenuPage - 1]].Employer == WORKER_JOBLESS) {
                // Kommentar
                OnscreenBitmap.PrintAt(Workers.Workers[MenuRemapper[MenuPage - 1]].Kommentar, FontSmallBlack, TEC_FONT_LEFT, 40, 158 + 26, 250, 319);

                // Einstellen:
                OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_JOBS, 5060), FontSmallBlack, TEC_FONT_LEFT, 40, 254 + 26 - 22, 250, 319);
            } else if (Workers.Workers[MenuRemapper[MenuPage - 1]].Employer == PlayerNum) {
                if (MenuPar2 == 1) {
                    // Feuern/Gehalt erhöhen/kürzen:
                    OnscreenBitmap.PrintAt(
                        bprintf(StandardTexte.GetS(TOKEN_JOBS, 5055), min(100, (Workers.Workers[MenuRemapper[MenuPage - 1]].Happyness + 100) / 2)),
                        FontSmallBlack, TEC_FONT_LEFT, 40, 132, 250, 319);
                    OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_JOBS, 5064), FontSmallBlack, TEC_FONT_LEFT, 40, 254 + 26 - 22, 250, 319);
                    OnscreenBitmap.BlitFrom(MenuBms[3], 216, 235);
                } else {
                    // Feuern/Gehalt erhöhen/kürzen:
                    OnscreenBitmap.PrintAt(
                        bprintf(StandardTexte.GetS(TOKEN_JOBS, 5055), min(100, (Workers.Workers[MenuRemapper[MenuPage - 1]].Happyness + 100) / 2)),
                        FontSmallBlack, TEC_FONT_LEFT, 40, 132, 250, 319);
                    OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_JOBS, 5061), FontSmallBlack, TEC_FONT_LEFT, 40, 254 + 26 - 22, 250, 319);
                    OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_JOBS, 5062), FontSmallBlack, TEC_FONT_LEFT, 40, 267 + 26 - 22, 250, 319);
                    OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_JOBS, 5063), FontSmallBlack, TEC_FONT_LEFT, 40, 280 + 26 - 22, 250, 319);
                }
            }

            OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_JOBS, 1020), MenuPage, MenuPageMax), FontSmallBlack, TEC_FONT_LEFT, 40, 280 + 26, 250, 319);
        }

        if (MenuPage < MenuPageMax) {
            OnscreenBitmap.BlitFrom(MenuBms[2], OnscreenBitmap.Size - MenuBms[2].Size);
        }
        break;

    case MENU_SABOTAGEROUTE:
    case MENU_SABOTAGEPLANE:
    case MENU_SELLPLANE:
        DrawPlanesNotepad(OnscreenBitmap, &MenuDataTable, MenuPage, &MenuBms);
        break;

    case MENU_BUYPLANE:
    case MENU_BUYXPLANE:
        DrawPlanesNotepad(OnscreenBitmap, &MenuDataTable, MenuPage, &MenuBms);
        break;

    case MENU_PLANECOSTS:
        DrawPlanesNotepad(OnscreenBitmap, &MenuDataTable, MenuPage, &MenuBms, TRUE);
        break;

    case MENU_PLANEREPAIRS: {
        SLONG c = 0;

        PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];

        // Hintergrund machen:
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        OnscreenBitmap.BlitFrom(MenuBms[0]);

        // Überschriften:
        OnscreenBitmap.PrintAt(MenuDataTable.ColTitle[0], FontSmallBlack, TEC_FONT_LEFT, XY(63, 40), MenuBms[0].Size);

        // Zustand, Ziel, Kosten
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1002), FontSmallBlack, TEC_FONT_LEFT, XY(175, 40), XY(346, 226));
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1005), FontSmallBlack, TEC_FONT_LEFT, XY(220, 40), XY(346, 226));
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1006), FontSmallBlack, TEC_FONT_LEFT, XY(265, 40), XY(346, 226));

        for (c = MenuPage; c < MenuPage + 13 && c < MenuDataTable.AnzRows; c++) {
            OnscreenBitmap.PrintAt(MenuDataTable.Values[0 + c * MenuDataTable.AnzColums], FontSmallBlack, TEC_FONT_LEFT, XY(63, 53 + (c - MenuPage) * 12),
                                   XY(346, 226));

            OnscreenBitmap.PrintAt(MenuDataTable.Values[1 + c * MenuDataTable.AnzColums], FontSmallBlack, TEC_FONT_LEFT, XY(185, 53 + (c - MenuPage) * 12),
                                   XY(346, 226));
            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(qPlayer.Planes[MenuDataTable.LineIndex[c]].TargetZustand), FontSmallBlack, TEC_FONT_LEFT,
                                   XY(220, 53 + (c - MenuPage) * 12), XY(346, 226));

            SLONG Improvement = 0;
            if (qPlayer.Planes[MenuDataTable.LineIndex[c]].TargetZustand > qPlayer.Planes[MenuDataTable.LineIndex[c]].WorstZustand + 20) {
                Improvement = qPlayer.Planes[MenuDataTable.LineIndex[c]].TargetZustand - (qPlayer.Planes[MenuDataTable.LineIndex[c]].WorstZustand + 20);
            }

            Improvement = SLONG(__int64(Improvement) * qPlayer.Planes[MenuDataTable.LineIndex[c]].ptPreis / 110);
            // Improvement = SLONG(__int64(Improvement) * PlaneTypes[qPlayer.Planes[MenuDataTable.LineIndex[c]].TypeId].Preis/110);

            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString(Improvement), FontSmallBlack, TEC_FONT_RIGHT, XY(285, 53 + (c - MenuPage) * 12), XY(346, 226));
        }

        // Unterschriften:
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 500), FontSmallBlack, TEC_FONT_LEFT, XY(63, 213), XY(346, 226));
        OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_SCHED, 1008), MenuInfo3), FontSmallBlack, TEC_FONT_LEFT, XY(216, 213), XY(346, 226));

        if (MenuPage > 0) {
            OnscreenBitmap.BlitFrom(MenuBms[1], 42, 218);
        }
        if (MenuPage + 13 < MenuDataTable.AnzRows) {
            OnscreenBitmap.BlitFrom(MenuBms[2], 338, 218);
        }
    } break;

    case MENU_SECURITY: //==>+<==
    {
        SLONG c = 0;

        PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];

        // Hintergrund machen:
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        OnscreenBitmap.BlitFrom(MenuBms[0]);

        // Überschriften:
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 8020), FontSmallBlack, TEC_FONT_LEFT, XY(63, 40), MenuBms[0].Size);

        // Die verschiedenen Posten:
        for (c = 0; c < 9; c++) {
            CString prefix = (qPlayer.SecurityFlags & (1 << c)) != 0U ? bprintf("%li x ", c < 6 ? 1 : qPlayer.Planes.GetNumUsed()) : "";

            if (c != 1 || (qPlayer.HasItem(ITEM_LAPTOP) != 0)) {
                if (c == 8) {
                    if ((qPlayer.SecurityFlags & (1 << 11)) != 0U) {
                        OnscreenBitmap.PrintAt(bprintf("%s%s", (LPCTSTR)prefix, StandardTexte.GetS(TOKEN_MISC, 8011)), FontSmallBlack, TEC_FONT_LEFT,
                                               XY(63, 55 + c * 12), XY(346, 226));
                    } else if ((qPlayer.SecurityFlags & (1 << 10)) != 0U) {
                        OnscreenBitmap.PrintAt(bprintf("%s%s", (LPCTSTR)prefix, StandardTexte.GetS(TOKEN_MISC, 8010)), FontSmallBlack, TEC_FONT_LEFT,
                                               XY(63, 55 + c * 12), XY(346, 226));
                    } else {
                        OnscreenBitmap.PrintAt(bprintf("%s%s", (LPCTSTR)prefix, StandardTexte.GetS(TOKEN_MISC, 8000 + c)), FontSmallBlack, TEC_FONT_LEFT,
                                               XY(63, 55 + c * 12), XY(346, 226));
                    }
                } else {
                    OnscreenBitmap.PrintAt(bprintf("%s%s", (LPCTSTR)prefix, StandardTexte.GetS(TOKEN_MISC, 8000 + c)), FontSmallBlack, TEC_FONT_LEFT,
                                           XY(63, 55 + c * 12), XY(346, 226));
                }
            } else {
                OnscreenBitmap.PrintAt(bprintf("%s%s", (LPCTSTR)prefix, StandardTexte.GetS(TOKEN_MISC, 8000 + c)), FontSmallGrey, TEC_FONT_LEFT,
                                       XY(63, 55 + c * 12), XY(346, 226));
            }
        }

        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 8021), FontSmallBlack, TEC_FONT_LEFT, XY(155, 50 + 113 + 22), XY(346, 226));
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 8022), FontSmallBlack, TEC_FONT_LEFT, XY(155, 50 + 113 + 34), XY(346, 226));
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 8023), FontSmallBlack, TEC_FONT_LEFT, XY(155, 213), XY(346, 226));
        OnscreenBitmap.PrintAt(Insert1000erDots(qPlayer.CalcSecurityCosts(true, false)), FontSmallBlack, TEC_FONT_RIGHT, XY(200, 50 + 113 + 22), XY(346, 226));
        OnscreenBitmap.PrintAt(Insert1000erDots(qPlayer.CalcSecurityCosts(false, true)), FontSmallBlack, TEC_FONT_RIGHT, XY(200, 50 + 113 + 34), XY(346, 226));
        OnscreenBitmap.PrintAt(Insert1000erDots(qPlayer.CalcSecurityCosts()), FontSmallBlack, TEC_FONT_RIGHT, XY(200, 213), XY(346, 226));

        // Unterschriften:
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 500), FontSmallBlack, TEC_FONT_LEFT, XY(63, 213), XY(346, 226));
    } break;

    case MENU_PANNEN: {
        SLONG c = 0;

        PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];

        // Hintergrund machen:
        OnscreenBitmap.ReSize(MenuBms[0].Size);
        OnscreenBitmap.BlitFrom(MenuBms[0]);

        if (MenuInfo3 == -1) {
            // Überschriften:
            OnscreenBitmap.PrintAt(MenuDataTable.ColTitle[0], FontSmallBlack, TEC_FONT_LEFT, XY(63, 40), MenuBms[0].Size);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1009), FontSmallBlack, TEC_FONT_LEFT, XY(265, 40), XY(346, 226));

            for (c = MenuPage; c < MenuPage + 13 && c < MenuDataTable.AnzRows; c++) {
                OnscreenBitmap.PrintAt(MenuDataTable.Values[0 + c * MenuDataTable.AnzColums], FontSmallBlack, TEC_FONT_LEFT, XY(63, 53 + (c - MenuPage) * 12),
                                       XY(346, 226));

                OnscreenBitmap.PrintAt(MenuDataTable.Values[1 + c * MenuDataTable.AnzColums], FontSmallBlack, TEC_FONT_LEFT, XY(265, 53 + (c - MenuPage) * 12),
                                       XY(346, 226));
            }

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 500), FontSmallBlack, TEC_FONT_LEFT, XY(63, 213), XY(346, 226));

            if (MenuPage > 0) {
                OnscreenBitmap.BlitFrom(MenuBms[1], 42, 218);
            }
            if (MenuPage + 13 < MenuDataTable.AnzRows) {
                OnscreenBitmap.BlitFrom(MenuBms[2], 338, 218);
            }
        } else {
            OnscreenBitmap.PrintAt(MenuDataTable.Values[0 + MenuInfo3 * MenuDataTable.AnzColums], FontSmallBlack, TEC_FONT_LEFT, XY(63, 40), MenuBms[0].Size);
            OnscreenBitmap.PrintAt(MenuDataTable.Values[1 + MenuInfo3 * MenuDataTable.AnzColums] + " " + StandardTexte.GetS(TOKEN_SCHED, 1009), FontSmallBlack,
                                   TEC_FONT_LEFT, XY(63, 55), MenuBms[0].Size);

            CPlane &qPlane = qPlayer.Planes[MenuDataTable.LineIndex[MenuInfo3]];

            for (c = 0; c < qPlane.Pannen.AnzEntries(); c++) {
                time_t Time = Sim.StartTime + qPlane.Pannen[c].Date * 60 * 60 * 24;
                struct tm *pTimeStruct = localtime(&Time);

                OnscreenBitmap.PrintAt(bprintf("%li:%02li", qPlane.Pannen[c].Time / 60000, (qPlane.Pannen[c].Time / 1000) % 60), FontSmallBlack, TEC_FONT_LEFT,
                                       XY(63, 73 + (c) * 12), XY(346, 226));
                OnscreenBitmap.PrintAt(bprintf("%li.%li.", pTimeStruct->tm_mday, pTimeStruct->tm_mon + 1), FontSmallBlack, TEC_FONT_LEFT,
                                       XY(108, 73 + (c) * 12), XY(346, 226));
                OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 2360 + qPlane.Pannen[c].Code), FontSmallBlack, TEC_FONT_LEFT, XY(153, 73 + (c) * 12),
                                       XY(346, 226));
            }

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1010), FontSmallBlack, TEC_FONT_LEFT, XY(63, 213), XY(346, 226));

            if (MenuInfo3 > 0) {
                OnscreenBitmap.BlitFrom(MenuBms[1], 42, 218);
            }
            if (MenuInfo3 < MenuDataTable.AnzRows - 1) {
                OnscreenBitmap.BlitFrom(MenuBms[2], 338, 218);
            }
        }
    } break;

    case MENU_PLANEJOB:
        DrawPlanesNotepad(OnscreenBitmap, &MenuDataTable, MenuPage, &MenuBms);
        break;

    case MENU_EXTRABLATT:
        if (ZoomCounter < 200 && (ZoomSpeed != 0)) {
            OnscreenBitmap.ReSize(MenuBms[SLONG((AtGetTime() / 50) % 8)].Size);
            OnscreenBitmap.BlitFrom(MenuBms[SLONG((AtGetTime() / 50) % 8)]);
        } else {
            if (OnscreenBitmap.Size != MenuBms[8].Size) {
                OnscreenBitmap.ReSize(MenuBms[8].Size);
                MenuPos = (XY(640, 440) - OnscreenBitmap.Size) / SLONG(2);
            }

            OnscreenBitmap.BlitFrom(MenuBms[8]);
            OnscreenBitmap.BlitFrom(MenuBms[9 + MenuPar1], 48, 59);

            OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_MISC, 2100), Sim.Players.Players[MenuPar1 / 4].AirlineX.c_str(),
                                           Sim.Players.Players[MenuPar1 % 4].AirlineX.c_str()),
                                   FontSmallBlack, TEC_FONT_LEFT, XY(6, 37), XY(192, 57));

            ZoomCounter = 100;
            ZoomSpeed = 0;
        }
        break;

    case MENU_CLOSED:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        {
            CString str = StandardTexte.GetS(TOKEN_MISC, 3200 + MenuPar1);

            char *p = strtok(const_cast<char *>((LPCTSTR)str), "\xb5");
            SLONG y = 20;

            while (p != nullptr) {
                if ((strchr(p, ':') != nullptr) && p[strlen(p) - 1] != ':') {
                    char *pp = strchr(p, ':');

                    *pp = 0;

                    OnscreenBitmap.PrintAt(p, FontBigGrey, TEC_FONT_LEFT, 20, y, 270, 190);
                    OnscreenBitmap.PrintAt(pp + 1, FontBigGrey, TEC_FONT_RIGHT, 20, y, 270, 190);
                } else {
                    OnscreenBitmap.PrintAt(p, FontBigGrey, TEC_FONT_LEFT, 20, y, 270, 190);
                }

                y += 21;

                p = strtok(nullptr, "\xb5");
            }
        }
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3002), FontBigGrey, TEC_FONT_CENTERED, 16, 160, 274, 190);
        break;

    case MENU_REQUEST:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        // if (MenuPar1==MENU_REQUEST_NORENTROUTE1 || MenuPar1==MENU_REQUEST_NORENTROUTE2 || MenuPar1==MENU_REQUEST_NORENTROUTE3 ||
        // MenuPar1==MENU_REQUEST_NORENTROUTE4 || MenuPar1==MENU_REQUEST_FORBIDDEN || MenuPar1==MENU_REQUEST_DESTROYED || MenuPar1==MENU_REQUEST_ITEMS ||
        // MenuPar1==MENU_REQUEST_NOGLUE || MenuPar1==MENU_REQUEST_NOSTENCH || (MenuPar1>=MENU_REQUEST_NO_LM && MenuPar1<=MENU_REQUEST_NO_WERBUNG))
        if (MenuIsPlain()) {
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3010 + MenuPar1), FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3002), FontBigGrey, TEC_FONT_CENTERED, 16, 160, 274, 190);
        } else {
            if (MenuPar1 == MENU_REQUEST_KILLITEM) {
                OnscreenBitmap.PrintAt(bprintf((LPCTSTR)CString(StandardTexte.GetS(TOKEN_MISC, 3010 + MenuPar1)),
                                               (LPCTSTR)CString(StandardTexte.GetS(TOKEN_TOOLTIP, 2000 + qPlayer.Items[MenuPar2]))),
                                       FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);
            } else if (MenuPar1 == MENU_REQUEST_KILLAUFTRAG || MenuPar1 == MENU_REQUEST_KILLAUFTRAG0) {
                OnscreenBitmap.PrintAt(bprintf((LPCTSTR)CString(StandardTexte.GetS(TOKEN_MISC, 3010 + MenuPar1)),
                                               (LPCTSTR)Cities[qPlayer.Auftraege[MenuPar2].VonCity].Name,
                                               (LPCTSTR)Cities[qPlayer.Auftraege[MenuPar2].NachCity].Name, qPlayer.Auftraege[MenuPar2].Strafe),
                                       FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);
            } else if (MenuPar1 == MENU_REQUEST_KILLFAUFTRAG || MenuPar1 == MENU_REQUEST_KILLFAUFTRAG0) {
                OnscreenBitmap.PrintAt(bprintf((LPCTSTR)CString(StandardTexte.GetS(TOKEN_MISC, 3010 + MenuPar1)),
                                               (LPCTSTR)Cities[qPlayer.Frachten[MenuPar2].VonCity].Name,
                                               (LPCTSTR)Cities[qPlayer.Frachten[MenuPar2].NachCity].Name, qPlayer.Frachten[MenuPar2].Strafe),
                                       FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);
            } else if (MenuPar1 == MENU_REQUEST_KILLPLAN) {
                OnscreenBitmap.PrintAt(bprintf((LPCTSTR)CString(StandardTexte.GetS(TOKEN_MISC, 3010 + MenuPar1)),
                                               (LPCTSTR)Sim.Players.Players[Sim.localPlayer].Planes[MenuPar2].Name),
                                       FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);
            } else if (MenuPar1 == MENU_REQUEST_KILLROUTE) {
                OnscreenBitmap.PrintAt(bprintf((LPCTSTR)CString(StandardTexte.GetS(TOKEN_MISC, 3010 + MenuPar1)),
                                               (LPCTSTR)Cities[Routen[MenuPar2].VonCity].Name, (LPCTSTR)Cities[Routen[MenuPar2].NachCity].Name,
                                               Routen[MenuPar2].Miete / 30, Routen[MenuPar2].Miete),
                                       FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);
            } else if (MenuPar1 == MENU_REQUEST_RENTROUTE) {
                OnscreenBitmap.PrintAt(bprintf((LPCTSTR)CString(StandardTexte.GetS(TOKEN_MISC, 3010 + MenuPar1)),
                                               (LPCTSTR)Cities[Routen[MenuPar2].VonCity].Name, (LPCTSTR)Cities[Routen[MenuPar2].NachCity].Name,
                                               Routen[MenuPar2].Miete / 30, Routen[MenuPar2].Miete),
                                       FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);
            } else {
                OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3010 + MenuPar1), FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);
            }

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3000), FontBigGrey, TEC_FONT_LEFT, 16, 160, 274, 190);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3001), FontBigGrey, TEC_FONT_RIGHT, 16, 160, 274, 190);
        }
        break;

    case MENU_RENAMEPLANE:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_MISC, 3099), (LPCTSTR)CString(qPlayer.Planes[MenuPar1].Name)), FontBigGrey, TEC_FONT_LEFT, 10,
                               10, 280, 190);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3002), FontBigGrey, TEC_FONT_LEFT, 16, 160, 274, 190);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3003), FontBigGrey, TEC_FONT_RIGHT, 16, 160, 274, 190);
        OnscreenBitmap.PrintAt(Optionen[0] + "_", FontSmallBlack, TEC_FONT_LEFT, 26, 141, 268, 155);
        break;

    case MENU_RENAMEEDITPLANE: {
        PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];

        OnscreenBitmap.BlitFrom(MenuBms[0]);
        OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_MISC, 3099), (LPCTSTR)CString((dynamic_cast<CEditor *>(qPlayer.LocationWin))->Plane.Name)),
                               FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3002), FontBigGrey, TEC_FONT_LEFT, 16, 160, 274, 190);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3003), FontBigGrey, TEC_FONT_RIGHT, 16, 160, 274, 190);
        OnscreenBitmap.PrintAt(Optionen[0] + "_", FontSmallBlack, TEC_FONT_LEFT, 26, 141, 268, 155);
    } break;

    case MENU_ENTERTCPIP:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3097), FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3002), FontBigGrey, TEC_FONT_CENTERED, 16, 160, 274, 190);
        OnscreenBitmap.PrintAt(Optionen[0] + "_", FontSmallBlack, TEC_FONT_LEFT, 26, 141, 268, 155);
        break;

    case MENU_BROADCAST: {
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3098), FontBigGrey, TEC_FONT_LEFT, 10, 10, 280, 190);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3002), FontBigGrey, TEC_FONT_LEFT, 16, 160, 274, 190);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3003), FontBigGrey, TEC_FONT_RIGHT, 16, 160, 274, 190);

        while (TipBm.TryPrintAt(Optionen[0] + " _", FontSmallBlack, TEC_FONT_LEFT, 26, 141, 268, 155) >= 14) {
            Optionen[0] = Optionen[0].Left(Optionen[0].GetLength() - 1);
        }

        OnscreenBitmap.PrintAt(Optionen[0] + "_", FontSmallBlack, TEC_FONT_LEFT, 26, 141, 268, 155);

        for (c = 0; c < 4; c++) {
            if (c != Sim.localPlayer) {
                if (((MenuPar1 & (1 << c)) != 0) && Sim.Players.Players[c].Owner == 2) {
                    OnscreenBitmap.BlitFromT(MenuBms[c + 1], 60 + (c - static_cast<SLONG>(c > Sim.localPlayer)) * 60, 80);
                } else {
                    OnscreenBitmap.BlitFromT(MenuBms[c + 5], 60 + (c - static_cast<SLONG>(c > Sim.localPlayer)) * 60, 80);
                }
            }
        }

    } break;

    case MENU_GAMEOVER:
        OnscreenBitmap.BlitFromT(MenuBms[static_cast<SLONG>(MenuPar1 == 0)], 320 - MenuBms[static_cast<SLONG>(MenuPar1 == 0)].Size.x / 2, 330);
        break;

    case MENU_SABOTAGEFAX:
        OnscreenBitmap.BlitFrom(MenuBms[0]);

        if (MenuPar1 == 6) {
            OnscreenBitmap.PrintAt(
                bprintf(StandardTexte.GetS(TOKEN_LETTER, 500 + MenuPar1), (LPCTSTR)Sim.Players.Players[Sim.localPlayer].Planes[MenuPar2].Name, MenuPar3),
                FontSmallBlack, TEC_FONT_LEFT, 30, 70, 250, 190);
            OnscreenBitmap.BlitFrom(MenuBms[MenuPar1], 30, 180);
        } else if (MenuPar1 == 7) {
            PLAYER &qPlayer = Sim.Players.Players[Sim.localPlayer];
            SLONG CityId = qPlayer.Planes[MenuPar2].Ort;

            if (CityId <= 0 && (qPlayer.Planes[MenuPar2].GetFlugplanEintrag() != nullptr)) {
                CityId = qPlayer.Planes[MenuPar2].GetFlugplanEintrag()->NachCity;
            }

            if (CityId <= 0) {
                CityId = Sim.HomeAirportId;
            }

            OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_LETTER, 500 + MenuPar1), (LPCTSTR)qPlayer.Planes[MenuPar2].Name, MenuPar3,
                                           (LPCTSTR)Cities[Sim.HomeAirportId].Name, (LPCTSTR)Cities[CityId].Name),
                                   FontSmallBlack, TEC_FONT_LEFT, 30, 70, 250, 190);
            OnscreenBitmap.BlitFrom(MenuBms[6], 30, 180);
        } else if (MenuPar1 == 8) {
            OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_LETTER, 500 + MenuPar1), MenuPar2), FontSmallBlack, TEC_FONT_LEFT, 30, 70, 250, 190);
            OnscreenBitmap.BlitFrom(MenuBms[8], 30, 180);
        } else {
            OnscreenBitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_LETTER, 500 + MenuPar1), (LPCTSTR)Sim.Players.Players[MenuPar2].Planes[MenuPar3].Name),
                                   FontSmallBlack, TEC_FONT_LEFT, 30, 70, 250, 190);
            OnscreenBitmap.BlitFrom(MenuBms[MenuPar1], 30, 180);
        }
        break;

    case MENU_LETTERS:
        if (qPlayer.Letters.Letters[MenuPage].IsLetter != 0) {
            PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

            OnscreenBitmap.BlitFrom(MenuBms[0]);
            if (MenuPage > 0) {
                OnscreenBitmap.BlitFrom(MenuBms[1], 0, 332);
            }
            if (MenuPage < qPlayer.Letters.AnzLetters() - 1) {
                OnscreenBitmap.BlitFrom(MenuBms[2], 245, 335);
            }

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3120), FontSmallBlack, TEC_FONT_LEFT, 19, 55, 295, 195);
            OnscreenBitmap.PrintAt(qPlayer.Name, FontSmallBlack, TEC_FONT_LEFT, 57, 55, 295, 195);
            OnscreenBitmap.PrintAt(qPlayer.AirlineX, FontSmallBlack, TEC_FONT_LEFT, 57, 67, 295, 195);
            OnscreenBitmap.PrintAt(qPlayer.Letters.Letters[MenuPage].Subject, FontSmallBlack, TEC_FONT_LEFT, 19, 110, 284, 151);
            OnscreenBitmap.PrintAt(qPlayer.Letters.Letters[MenuPage].Letter, FontSmallBlack, TEC_FONT_LEFT, 19, 151, 284, 272 + 50);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3121), FontSmallBlack, TEC_FONT_LEFT, 19, 242 + 50, 295, 300 + 50);
            OnscreenBitmap.PrintAt(qPlayer.Letters.Letters[MenuPage].Absender, FontSmallBlack, TEC_FONT_LEFT, 19, 272 + 50, 295, 300 + 50);
        } else {
            PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

            OnscreenBitmap.BlitFrom(MenuBms[7]);
            OnscreenBitmap.PrintAt(qPlayer.Letters.Letters[MenuPage].Subject, FontSmallBlack, TEC_FONT_LEFT, 30, 70, 250, 290);

            if (qPlayer.Letters.Letters[MenuPage].PictureId != 0) {
                OnscreenBitmap.BlitFrom(MenuBms[8 + qPlayer.Letters.Letters[MenuPage].PictureId - 1], 30, 180);
            }

            if (MenuPage > 0) {
                OnscreenBitmap.BlitFromT(MenuBms[5], 16, 331);
            }
            if (MenuPage < qPlayer.Letters.AnzLetters() - 1) {
                OnscreenBitmap.BlitFrom(MenuBms[6], 236, 328);
            }
        }
        break;

    case MENU_ADROUTE:
        DrawPlanesNotepad(OnscreenBitmap, &MenuDataTable, MenuPage, &MenuBms);
        break;

    case MENU_BUYKEROSIN:
        if (qPlayer.GetRoom() == ROOM_ARAB_AIR) {
            SLONG Kerosinpreis = Sim.HoleKerosinPreis(MenuPar1);

            __int64 Rabatt = 0;
            __int64 Kosten = 0;

            Kosten = __int64(Kerosinpreis) * __int64(MenuPar2);

            if (MenuPar2 >= 50000) {
                Rabatt = Kosten / 10;
            } else if (MenuPar2 >= 10000) {
                Rabatt = Kosten / 20;
            }

            // Geldmenge begrenzen, damit's keinen Overflow gibt:
            if (Kosten - Rabatt > qPlayer.Money - (DEBT_LIMIT)) {
                MenuPar2 = SLONG((qPlayer.Money - (DEBT_LIMIT)) / __int64(Kerosinpreis));

                if (MenuPar2 < 0) {
                    MenuPar2 = 0;
                }
                Kosten = __int64(Kerosinpreis) * __int64(MenuPar2);

                if (MenuPar2 >= 50000) {
                    Rabatt = Kosten / 10;
                } else if (MenuPar2 >= 10000) {
                    Rabatt = Kosten / 20;
                } else {
                    Rabatt = 0;
                }
            }

            MenuInfo = SLONG(Kosten - Rabatt);

            OnscreenBitmap.BlitFrom(MenuBms[0]);

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 2000), FontSmallBlack, TEC_FONT_LEFT, 56, 10, 325, 113);

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 2010), FontSmallBlack, TEC_FONT_LEFT, 36, 37, 325, 113);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 2011), FontSmallBlack, TEC_FONT_LEFT, 36, 50, 325, 113);

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 2020), FontSmallBlack, TEC_FONT_LEFT, 36, 76, 325, 113);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 2021), FontSmallBlack, TEC_FONT_LEFT, 36, 89, 325, 113);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 2022), FontSmallBlack, TEC_FONT_LEFT, 36, 102, 325, 167);

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_TANK, 2030), FontSmallBlack, TEC_FONT_LEFT, 36, 120, 325, 167);

            SLONG _tank = 0;
            if (qPlayer.Tank == 0) {
                _tank = 9999999;
            } else {
                _tank = qPlayer.Tank;
            }

            // Volume total
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString(Kerosinpreis), FontSmallBlack, TEC_FONT_LEFT, 200, 37, 325, 113);
            OnscreenBitmap.PrintAt(
                bprintf(ETexte.GetS(1000, 3000 + (Sim.Players.Players[PlayerNum].Tank > 1)), LPCTSTR(Shorten1000erDots(Sim.Players.Players[PlayerNum].Tank))),
                FontSmallBlack, TEC_FONT_LEFT, 200, 50, 325, 113);

            // Volume available
            OnscreenBitmap.PrintAt(bprintf(ETexte.GetS(1000, 3000 + (Sim.Players.Players[PlayerNum].TankInhalt > 1)),
                                           LPCTSTR(Shorten1000erDots(Sim.Players.Players[PlayerNum].TankInhalt))),
                                   FontSmallBlack, TEC_FONT_RIGHT, 56, 76, 225, 113);
            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(Sim.Players.Players[PlayerNum].TankInhalt * 100 / _tank), FontSmallBlack, TEC_FONT_RIGHT, 56, 76,
                                   256, 113);

            // Buy
            OnscreenBitmap.PrintAt(bprintf(ETexte.GetS(1000, 3000 + (MenuPar2 > 1)), LPCTSTR(Shorten1000erDots(MenuPar2))), FontSmallBlack, TEC_FONT_RIGHT, 0,
                                   89, 225, 113);

            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(MenuPar2 * 100 / _tank), FontSmallBlack, TEC_FONT_RIGHT, 56, 89, 256, 113);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bShortString(MenuInfo), FontSmallBlack, TEC_FONT_RIGHT, 56, 89, 327, 113);

            // New volume
            OnscreenBitmap.PrintAt(bprintf(ETexte.GetS(1000, 3000 + (MenuPar2 + Sim.Players.Players[PlayerNum].TankInhalt > 1)),
                                           LPCTSTR(Shorten1000erDots((MenuPar2 + Sim.Players.Players[PlayerNum].TankInhalt)))),
                                   FontSmallBlack, TEC_FONT_RIGHT, 0, 102, 225, 160);

            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString((MenuPar2 + Sim.Players.Players[PlayerNum].TankInhalt) * 100 / _tank), FontSmallBlack,
                                   TEC_FONT_RIGHT, 56, 102, 256, 160);

            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString(SLONG(Rabatt)), FontSmallBlack, TEC_FONT_LEFT, 120, 120, 327, 160);
        }
        break;

    case MENU_SETRENDITE:
        if (qPlayer.GetRoom() == ROOM_BANK) {
            SB_CFont &qFontBankBlack = (dynamic_cast<Bank *>(this))->FontBankBlack;
            SB_CFont &qFontBankRed = (dynamic_cast<Bank *>(this))->FontBankRed;

            OnscreenBitmap.BlitFrom(MenuBms[0]);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 2000), qFontBankRed, TEC_FONT_LEFT, 56, 10, 325, 113);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 2001), qFontBankBlack, TEC_FONT_LEFT, 56, 37, 325, 113);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 2002), qFontBankBlack, TEC_FONT_LEFT, 56, 50, 325, 113);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 2003), qFontBankBlack, TEC_FONT_LEFT, 56, 63, 325, 113);
            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(MenuInfo), FontSmallBlack, TEC_FONT_RIGHT, 200, 37, 320, 113);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString(MenuInfo * qPlayer.AnzAktien), qFontBankBlack, TEC_FONT_RIGHT, 200, 50, 320, 113);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString(MenuInfo * qPlayer.AnzAktien / 365), qFontBankBlack, TEC_FONT_RIGHT, 200, 63, 320, 113);
        }
        break;

    case MENU_AKTIE:
        if (qPlayer.GetRoom() == ROOM_BANK) {
            SLONG Max = Sim.Players.Players[MenuPar1].AnzAktien;
            for (SLONG c = 0; c < 4; c++) {
                if (Sim.Players.Players[c].IsOut == 0) {
                    Max -= Sim.Players.Players[c].OwnsAktien[MenuPar1];
                }
            }

            SB_CFont &qFontBankBlack = (dynamic_cast<Bank *>(this))->FontBankBlack;
            SB_CFont &qFontBankRed = (dynamic_cast<Bank *>(this))->FontBankRed;

            SLONG neu = 0;
            if (MenuPar2 == 0) {
                neu = qPlayer.OwnsAktien[MenuPar1] + MenuInfo;
            } else {
                neu = qPlayer.OwnsAktien[MenuPar1] - MenuInfo;
            }

            OnscreenBitmap.BlitFrom(MenuBms[0]);

            // Überschrift:
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 3000 + MenuPar2), qFontBankRed, TEC_FONT_LEFT, 60, 10, 325, 139);

            // Tabelle mit Werten (Überschrift):
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 1001), qFontBankBlack, TEC_FONT_LEFT, 90, 27, 325, 139);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 3010), qFontBankBlack, TEC_FONT_LEFT, 140, 27, 325, 139);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 3011), qFontBankBlack, TEC_FONT_LEFT, 200, 27, 325, 139);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 3012), qFontBankBlack, TEC_FONT_LEFT, 240, 27, 325, 139);

            // Markt:
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 1000), qFontBankBlack, TEC_FONT_LEFT, 30, 40, 325, 139);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString(SLONG(Sim.Players.Players[MenuPar1].Kurse[0])), qFontBankBlack, TEC_FONT_LEFT, 90, 40, 260, 139);
            OnscreenBitmap.PrintAt(bitoa(Max), qFontBankBlack, TEC_FONT_LEFT, 140, 40, 260, 139);
            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(SLONG(__int64(Max) * 100 / Sim.Players.Players[MenuPar1].AnzAktien)), qFontBankBlack,
                                   TEC_FONT_LEFT, 200, 40, 325, 139);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(__int64(Max) * SLONG(Sim.Players.Players[MenuPar1].Kurse[0])), qFontBankBlack, TEC_FONT_LEFT,
                                   240, 40, 325, 139);

            // Tabelle mit Werten (Bestand):
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 3020), qFontBankBlack, TEC_FONT_LEFT, 30, 60, 325, 139);
            OnscreenBitmap.PrintAt(bitoa(qPlayer.OwnsAktien[MenuPar1]), qFontBankBlack, TEC_FONT_LEFT, 140, 60, 325, 139);
            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(SLONG(__int64(qPlayer.OwnsAktien[MenuPar1]) * 100 / Sim.Players.Players[MenuPar1].AnzAktien)),
                                   qFontBankBlack, TEC_FONT_LEFT, 200, 60, 325, 139);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(__int64(qPlayer.OwnsAktien[MenuPar1]) * SLONG(Sim.Players.Players[MenuPar1].Kurse[0])),
                                   qFontBankBlack, TEC_FONT_LEFT, 240, 60, 325, 139);

            // Tabelle mit Werten (ver/kaufen):
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 3021 + MenuPar2), qFontBankBlack, TEC_FONT_LEFT, 30, 73, 325, 139);
            OnscreenBitmap.PrintAt(bitoa(MenuInfo), FontSmallBlack, TEC_FONT_LEFT, 140, 73, 325, 139);
            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(SLONG(__int64(MenuInfo) * 100 / Sim.Players.Players[MenuPar1].AnzAktien)), FontSmallBlack,
                                   TEC_FONT_LEFT, 200, 73, 325, 139);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(__int64(MenuInfo) * SLONG(Sim.Players.Players[MenuPar1].Kurse[0])), FontSmallBlack,
                                   TEC_FONT_LEFT, 240, 73, 325, 139);

            // Tabelle mit Werten (neuer Bestand):
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 3023), qFontBankBlack, TEC_FONT_LEFT, 30, 86, 325, 139);
            OnscreenBitmap.PrintAt(bitoa(neu), qFontBankBlack, TEC_FONT_LEFT, 140, 86, 325, 139);
            OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(SLONG(__int64(neu) * 100 / Sim.Players.Players[MenuPar1].AnzAktien)), qFontBankBlack,
                                   TEC_FONT_LEFT, 200, 86, 325, 139);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(__int64(neu) * SLONG(Sim.Players.Players[MenuPar1].Kurse[0])), qFontBankBlack, TEC_FONT_LEFT,
                                   240, 86, 325, 139);

            // Neuer Kontostand:
            auto aktienWert = __int64(Sim.Players.Players[MenuPar1].Kurse[0] * MenuInfo);
            __int64 gesamtPreis = 0;
            if (MenuPar2 == 0) {
                gesamtPreis = aktienWert + aktienWert / 10 + 100;
            } else {
                gesamtPreis = aktienWert - aktienWert / 10 - 100;
                gesamtPreis = -gesamtPreis;
            }

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_AKTIE, 3030), qFontBankBlack, TEC_FONT_LEFT, 30, 111, 325, 139);
            OnscreenBitmap.PrintAt(Einheiten[EINH_DM].bString64(__int64(qPlayer.Money - gesamtPreis)), qFontBankBlack, TEC_FONT_LEFT, 220, 111, 325, 139);
        }
        break;

    case MENU_KONTOAUSZUG:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        if (MenuPage < 9) {
            OnscreenBitmap.BlitFrom(MenuBms[2], OnscreenBitmap.Size - MenuBms[2].Size);
        }
        if (MenuPage > 0 && qPlayer.History.HistoryLine[MenuPage * 10 - 1].Description != "*") {
            OnscreenBitmap.BlitFrom(MenuBms[1], 0, OnscreenBitmap.Size.y - MenuBms[2].Size.y);
        }
        DrawMoneyTip(OnscreenBitmap, PlayerNum, MenuPage);
        break;

    case MENU_QUITMESSAGE:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        break;

    case MENU_FLUEGE:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        // Texte reinschreiben:
        {
            SLONG Time = Sim.GetHour() - 1;
            SLONG pass = 0;
            SLONG py = 0;

            OnscreenBitmap.PrintAt(Cities[Sim.HomeAirportId].Name, FontSmallWhiteX, TEC_FONT_CENTERED, 8 + 55, 46 - 14, 310, 243);

            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 1500), FontSmallWhiteX, TEC_FONT_RIGHT, 8 + 55, 46 + 3, 82 + 7, 243 + 3);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 1502), FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 46 + 3, 310, 243 + 3);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 1503), FontSmallWhiteX, TEC_FONT_LEFT, 280, 46 + 3, 320, 243 + 3);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 1500), FontSmallWhiteX, TEC_FONT_RIGHT, 8 + 55, 151 + 3, 82 + 7, 243 + 3);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 1501), FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 151 + 3, 310, 243 + 3);
            OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 1503), FontSmallWhiteX, TEC_FONT_LEFT, 280, 151 + 3, 320, 243 + 3);

            SLONG c = 0;
            SLONG d = 0;
            SLONG e = 0;
            CFlugplan *Plan = nullptr;

            // Arrivals:
            for (pass = 1; pass < 48; pass++) {
                // Für alle Spieler:
                for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                    if (Sim.Players.Players[c].IsOut == 0) {
                        // Für alle Flugzeuge des Spielers:
                        for (d = 0; d < Sim.Players.Players[c].Planes.AnzEntries(); d++) {
                            if (Sim.Players.Players[c].Planes.IsInAlbum(d) != 0) {
                                CPlane &qPlane = Sim.Players.Players[c].Planes[d];
                                Plan = &qPlane.Flugplan;

                                // Für alle Flüge des Flugzeuges:
                                for (e = 0; e < Plan->Flug.AnzEntries(); e++) {
                                    if ((Plan->Flug[e].ObjectType == 1 || Plan->Flug[e].ObjectType == 2) &&
                                        (Plan->Flug[e].Landedate - Sim.Date == ((Time + 1) / 24) && Plan->Flug[e].Landezeit == Time + 1 &&
                                         ((Plan->Flug[e].Gate != -2 && Plan->Flug[e].NachCity == static_cast<ULONG>(Sim.HomeAirportId)) ||
                                          (CheatShowAll != 0)))) {
                                        // Blinkendes Etwas:
                                        if (Plan->Flug[e].Landedate == Sim.Date && abs(Plan->Flug[e].Landezeit - Sim.GetHour()) <= 1) {
                                            if (qPlane.Ort == -3 || qPlane.Ort == Sim.HomeAirportId || (qPlane.Ort == -1 && qPlane.AirportPos.y >= 22)) {
                                                OnscreenBitmap.BlitFromT(MenuBms[1 + (Sim.TimeSlice / 4) % 2], XY(40, 62 + 3 + py));
                                            }
                                        }

                                        OnscreenBitmap.PrintAt(bprintf("%2li:00", Plan->Flug[e].Landezeit), FontSmallWhiteX, TEC_FONT_LEFT, 2 + 55, 59 + py + 3,
                                                               310, 255 + 3);
                                        OnscreenBitmap.BlitFromT(MenuBms[3 + c], 90, 60 + py + 3);
                                        OnscreenBitmap.PrintAt(bprintf("%s", (LPCTSTR)Sim.Players.Players[c].Abk), FontSmallWhiteX, TEC_FONT_LEFT, 11 + 90,
                                                               59 + py + 3, 310, 255 + 3);

                                        // Rio-Rom
                                        if (OnscreenBitmap.TryPrintAt(
                                                bprintf("%s-%s", (LPCTSTR)Cities[Plan->Flug[e].VonCity].Name, (LPCTSTR)Cities[Plan->Flug[e].NachCity].Name),
                                                FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 59 + py + 3, 290, 255 + 3) < 13) {
                                            OnscreenBitmap.PrintAt(
                                                bprintf("%s-%s", (LPCTSTR)Cities[Plan->Flug[e].VonCity].Name, (LPCTSTR)Cities[Plan->Flug[e].NachCity].Name),
                                                FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 59 + py + 3, 290, 255 + 3);
                                        } else if (OnscreenBitmap.TryPrintAt(bprintf("%s-%s", (LPCTSTR)Cities[Plan->Flug[e].VonCity].Name,
                                                                                     (LPCTSTR)Cities[Plan->Flug[e].NachCity].Kuerzel),
                                                                             FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 59 + py + 3, 290, 255 + 3) < 13) {
                                            OnscreenBitmap.PrintAt(
                                                bprintf("%s-%s", (LPCTSTR)Cities[Plan->Flug[e].VonCity].Name, (LPCTSTR)Cities[Plan->Flug[e].NachCity].Kuerzel),
                                                FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 59 + py + 3, 290, 255 + 3);
                                        } else {
                                            OnscreenBitmap.PrintAt(bprintf("%s-%s", (LPCTSTR)Cities[Plan->Flug[e].VonCity].Kuerzel,
                                                                           (LPCTSTR)Cities[Plan->Flug[e].NachCity].Kuerzel),
                                                                   FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 59 + py + 3, 290, 255 + 3);
                                        }

                                        // Gate:
                                        if (Plan->Flug[e].Gate == -1) {
                                            OnscreenBitmap.PrintAt("-", FontSmallWhiteX, TEC_FONT_LEFT, 280, 59 + py + 3, 310, 255 + 3);
                                        } else if (Plan->Flug[e].Gate == -2) {
                                            OnscreenBitmap.PrintAt("ext", FontSmallWhiteX, TEC_FONT_LEFT, 280, 59 + py + 3, 310, 255 + 3);
                                        } else {
                                            OnscreenBitmap.PrintAt(bprintf("%li", Plan->Flug[e].Gate + 1), FontSmallWhiteX, TEC_FONT_LEFT, 280, 59 + py + 3,
                                                                   310, 255 + 3);
                                        }

                                        py += 13;
                                        if (py >= 13 * 4) {
                                            break;
                                        }
                                    }
                                }
                            }
                            if (py >= 13 * 4) {
                                break;
                            }
                        }
                        if (py >= 13 * 4) {
                            break;
                        }
                    }
                }

                Time++;
                if (py >= 13 * 4) {
                    break;
                }
            }

            // Departures:
            py = 0;
            Time = Sim.GetHour() - 1;
            for (pass = 1; pass < 48; pass++) {
                // Für alle Spieler:
                for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                    // Für alle Flugzeuge des Spielers:
                    for (d = 0; d < Sim.Players.Players[c].Planes.AnzEntries(); d++) {
                        if (Sim.Players.Players[c].Planes.IsInAlbum(d) != 0) {
                            CPlane &qPlane = Sim.Players.Players[c].Planes[d];
                            Plan = &qPlane.Flugplan;

                            // Für alle Flüge des Flugzeuges:
                            for (e = 0; e < Plan->Flug.AnzEntries(); e++) {
                                if ((Plan->Flug[e].ObjectType == 1 || Plan->Flug[e].ObjectType == 2) &&
                                    (Plan->Flug[e].Startdate - Sim.Date == (Time / 24) && Plan->Flug[e].Startzeit == Time &&
                                     ((Plan->Flug[e].Gate != -2 && Plan->Flug[e].VonCity == static_cast<ULONG>(Sim.HomeAirportId)) || (CheatShowAll != 0)))) {
                                    // Blinkendes Etwas:
                                    if (Plan->Flug[e].Startdate == Sim.Date && abs(Plan->Flug[e].Startzeit + 1 - Sim.GetHour()) <= 1) {
                                        if (qPlane.Ort == -3 && abs(qPlane.TargetX - qPlane.AirportPos.x) < 4) {
                                            OnscreenBitmap.BlitFromT(MenuBms[1 + (Sim.TimeSlice / 4) % 2], XY(40, 167 + py + 3));
                                        }
                                    }

                                    OnscreenBitmap.PrintAt(bprintf("%2li:00", Plan->Flug[e].Startzeit + 1), FontSmallWhiteX, TEC_FONT_LEFT, 2 + 55,
                                                           164 + py + 3, 310, 255 + 3);
                                    OnscreenBitmap.BlitFromT(MenuBms[3 + c], 90, 165 + py + 3);
                                    OnscreenBitmap.PrintAt(bprintf("%s", (LPCTSTR)Sim.Players.Players[c].Abk), FontSmallWhiteX, TEC_FONT_LEFT, 11 + 90,
                                                           164 + py + 3, 310, 255 + 3);

                                    // Rio-Rom
                                    if (OnscreenBitmap.TryPrintAt(
                                            bprintf("%s-%s", (LPCTSTR)Cities[Plan->Flug[e].VonCity].Name, (LPCTSTR)Cities[Plan->Flug[e].NachCity].Name),
                                            FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 164 + py + 3, 290, 255 + 3) < 13) {
                                        OnscreenBitmap.PrintAt(
                                            bprintf("%s-%s", (LPCTSTR)Cities[Plan->Flug[e].VonCity].Name, (LPCTSTR)Cities[Plan->Flug[e].NachCity].Name),
                                            FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 164 + py + 3, 290, 255 + 3);
                                    } else if (OnscreenBitmap.TryPrintAt(bprintf("%s-%s", (LPCTSTR)Cities[Plan->Flug[e].VonCity].Name,
                                                                                 (LPCTSTR)Cities[Plan->Flug[e].NachCity].Kuerzel),
                                                                         FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 164 + py + 3, 290, 255 + 3) < 13) {
                                        OnscreenBitmap.PrintAt(
                                            bprintf("%s-%s", (LPCTSTR)Cities[Plan->Flug[e].VonCity].Name, (LPCTSTR)Cities[Plan->Flug[e].NachCity].Kuerzel),
                                            FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 164 + py + 3, 290, 255 + 3);
                                    } else {
                                        OnscreenBitmap.PrintAt(
                                            bprintf("%s-%s", (LPCTSTR)Cities[Plan->Flug[e].VonCity].Kuerzel, (LPCTSTR)Cities[Plan->Flug[e].NachCity].Kuerzel),
                                            FontSmallWhiteX, TEC_FONT_LEFT, 8 + 115, 164 + py + 3, 290, 255 + 3);
                                    }

                                    // Gate:
                                    if (Plan->Flug[e].Gate == -1) {
                                        OnscreenBitmap.PrintAt("-", FontSmallWhiteX, TEC_FONT_LEFT, 280, 164 + py + 3, 310, 255 + 3);
                                    } else if (Plan->Flug[e].Gate == -2) {
                                        OnscreenBitmap.PrintAt("ext", FontSmallWhiteX, TEC_FONT_LEFT, 280, 164 + py + 3, 310, 255 + 3);
                                    } else {
                                        OnscreenBitmap.PrintAt(bprintf("%li", Plan->Flug[e].Gate + 1), FontSmallWhiteX, TEC_FONT_LEFT, 280, 164 + py + 3, 310,
                                                               255 + 3);
                                    }

                                    py += 13;
                                    if (py >= 13 * 4) {
                                        break;
                                    }
                                }
                            }
                        }
                        if (py >= 13 * 4) {
                            break;
                        }
                    }
                    if (py >= 13 * 4) {
                        break;
                    }
                }
                Time++;
                if (py >= 13 * 4) {
                    break;
                }
            }
        }
        break;

    case MENU_AUSLANDSAUFTRAG:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        OnscreenBitmap.PrintAt(Cities[MenuPar1].Name, FontSmallBlack, TEC_FONT_LEFT, 34 + 180, 12, 204 + 180, 13 + 14);
        for (c = 0; c < 6; c++) {
            CAuftrag &qAuftrag = AuslandsAuftraege[MenuPar1][c];

            if (qAuftrag.Praemie != 0) {
                CITY &qVon = Cities[qAuftrag.VonCity];
                CITY &qNach = Cities[qAuftrag.NachCity];

                if (OnscreenBitmap.TryPrintAt(bprintf("%s - %s", (LPCTSTR)qVon.Name, (LPCTSTR)qNach.Name), FontSmallBlack, TEC_FONT_LEFT, 34 + 180,
                                              12 + c * 13 + 26, 204 + 180, 13 + 14 + c * 13 + 26) < 14) {
                    OnscreenBitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)qVon.Name, (LPCTSTR)qNach.Name), FontSmallBlack, TEC_FONT_LEFT, 34 + 180,
                                           12 + c * 13 + 26, 204 + 180, 13 + 14 + c * 13 + 26);
                } else if (OnscreenBitmap.TryPrintAt(bprintf("%s - %s", (LPCTSTR)qVon.Name, (LPCTSTR)qNach.Kuerzel), FontSmallBlack, TEC_FONT_LEFT, 34 + 180,
                                                     12 + c * 13 + 26, 204 + 180, 13 + 14 + c * 13 + 26) < 14) {
                    OnscreenBitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)qVon.Name, (LPCTSTR)qNach.Kuerzel), FontSmallBlack, TEC_FONT_LEFT, 34 + 180,
                                           12 + c * 13 + 26, 204 + 180, 13 + 14 + c * 13 + 26);
                } else {
                    OnscreenBitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)qVon.Kuerzel, (LPCTSTR)qNach.Kuerzel), FontSmallBlack, TEC_FONT_LEFT, 34 + 180,
                                           12 + c * 13 + 26, 204 + 180, 13 + 14 + c * 13 + 26);
                }

                if (MouseClickArea == -101 && MouseClickPar2 >= 0 && MouseClickPar2 <= 5 && MouseClickPar2 == c) {
                    DrawAuftragTipContents(PlayerNum, OnscreenBitmap, MenuBms2.getData(), &qAuftrag, -2, 0, 0, XY(6, 6), XY(6, 28), XY(-37 - 2, -8 + 5),
                                           &FontSmallBlack, &FontSmallBlack);
                }
            }
        }
        for (c = 0; c < 6; c++) {
            CFracht &qFracht = AuslandsFrachten[MenuPar1][c];

            if (qFracht.Praemie > 0) {
                CITY &qVon = Cities[qFracht.VonCity];
                CITY &qNach = Cities[qFracht.NachCity];

                if (OnscreenBitmap.TryPrintAt(bprintf("%s - %s", (LPCTSTR)qVon.Name, (LPCTSTR)qNach.Name), FontSmallBlack, TEC_FONT_LEFT, 34 + 180,
                                              12 + (c + 7) * 13 + 26, 204 + 180, 13 + 14 + (c + 7) * 13 + 26) < 14) {
                    OnscreenBitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)qVon.Name, (LPCTSTR)qNach.Name), FontSmallBlack, TEC_FONT_LEFT, 34 + 180,
                                           12 + (c + 7) * 13 + 26, 204 + 180, 13 + 14 + (c + 7) * 13 + 26);
                } else if (OnscreenBitmap.TryPrintAt(bprintf("%s - %s", (LPCTSTR)qVon.Name, (LPCTSTR)qNach.Kuerzel), FontSmallBlack, TEC_FONT_LEFT, 34 + 180,
                                                     12 + (c + 7) * 13 + 26, 204 + 180, 13 + 14 + (c + 7) * 13 + 26) < 14) {
                    OnscreenBitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)qVon.Name, (LPCTSTR)qNach.Kuerzel), FontSmallBlack, TEC_FONT_LEFT, 34 + 180,
                                           12 + (c + 7) * 13 + 26, 204 + 180, 13 + 14 + (c + 7) * 13 + 26);
                } else {
                    OnscreenBitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)qVon.Kuerzel, (LPCTSTR)qNach.Kuerzel), FontSmallBlack, TEC_FONT_LEFT, 34 + 180,
                                           12 + (c + 7) * 13 + 26, 204 + 180, 13 + 14 + (c + 7) * 13 + 26);
                }

                if (MouseClickArea == -101 && MouseClickPar2 >= 0 + 7 && MouseClickPar2 <= 5 + 7 && MouseClickPar2 == c + 7) {
                    DrawFrachtTipContents(PlayerNum, OnscreenBitmap, MenuBms2.getData(), &qFracht, 0, 0, 0, 777 * 0, XY(6, 6), XY(6, 28), XY(-37 - 2, -8 + 5),
                                          &FontSmallBlack, &FontSmallBlack);
                    // DrawFrachtTipContents (PlayerNum, OnscreenBitmap, MenuBms2.getData(), &qFracht, SLONG TonsThis, SLONG Costs, SLONG Income, SLONG Okay, XY
                    // Headline, XY Contents, XY MapOffset, SB_CFont *pHeadFont, SB_CFont *pFont, BOOL Unlocked) DrawAuftragTipContents(         , , , , SLONG
                    // Gate,     SLONG Costs,               SLONG Okay, XY Headline, XY Contents, XY MapOffset, SB_CFont *pHeadFont, SB_CFont *pFont, BOOL
                    // Unlocked)
                }
            }
        }
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 1100), FontSmallBlack, TEC_FONT_LEFT, 34 + 180, 12 + (7 + 7) * 13 + 26, 204 + 180,
                               13 + 14 + (7 + 7) * 13 + 26);
        break;

    case MENU_WC_M:
        if (Sim.TimeSlice > MenuPar1 + 20 && MenuPar2 == 0) {
            MenuPar2 = 1;
            if (PlayerNum == 0) {
                gToiletFx.ReInit("toiletfm.raw");
            } else {
                gToiletFx.ReInit("toilet.raw");
            }
            gToiletFx.Play(0, Sim.Options.OptionEffekte * 100 / 7);

            qPlayer.RunningToToilet = FALSE;
            Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Running = FALSE;
        }

        if (Sim.TimeSlice > MenuPar1 + 80 || (PlayerNum == 0 && Sim.TimeSlice > MenuPar1 + 40)) {
            Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Target = Airport.GetRandomTypedRune(RUNE_2SHOP, static_cast<UBYTE>(ROOM_WC_M)) + XY(0, 44);
            MenuStop();
        }
        break;

    case MENU_WC_F:
        if (Sim.TimeSlice > MenuPar1 + 20 && MenuPar2 == 0) {
            MenuPar2 = 1;
            qPlayer.RunningToToilet = FALSE;
            if (PlayerNum != 0) {
                SLONG r = rand() % 100;

                if (r > 93) {
                    gToiletFx.ReInit("toile3mf.raw");
                } else if (r > 60) {
                    gToiletFx.ReInit("toile2mf.raw");
                } else {
                    gToiletFx.ReInit("toiletmf.raw");
                }

                if (Sim.ItemParfuem != 0) {
                    Sim.Players.Players[PlayerNum].BuyItem(ITEM_PARFUEM);
                    if (Sim.Players.Players[PlayerNum].HasItem(ITEM_PARFUEM) != 0) {
                        Sim.ItemParfuem = 0;
                        SIM::SendSimpleMessage(ATNET_TAKETHING, 0, ITEM_PARFUEM);
                    }
                }
            } else {
                gToiletFx.ReInit("toilet.raw");
            }
            gToiletFx.Play(0, Sim.Options.OptionEffekte * 100 / 7);
        }

        if (Sim.TimeSlice > MenuPar1 + 80) {
            Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Target = Airport.GetRandomTypedRune(RUNE_2SHOP, static_cast<UBYTE>(ROOM_WC_F)) + XY(0, 44);
            MenuStop();
        }
        break;

    case MENU_CHAT: {
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        OnscreenBitmap.BlitFromT(MenuBms[1], 11, 11);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3005), FontBigGrey, TEC_FONT_LEFT, 18, 170, 274, 195);
        OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 3006), FontBigGrey, TEC_FONT_RIGHT, 18, 170, 274, 195);

        while (TipBm.TryPrintAt(Optionen[0] + " _", FontSmallBlack, TEC_FONT_LEFT, 14, 152, 279, 170) >= 14) {
            Optionen[0] = Optionen[0].Left(Optionen[0].GetLength() - 1);
        }

        OnscreenBitmap.PrintAt(Optionen[0] + "_", FontSmallBlack, TEC_FONT_LEFT, 14, 152, 279, 170);
    } break;

    case MENU_BRIEFING: {
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        MenuPar1 = 0;

        for (c = 0; c < 4; c++) {
            PLAYER &qPlayer = Sim.Players.Players[c];
            XY p = XY(15 + 46 + (c & 1) * (282 - 8), 15 + 68 + (c / 2) * (153));

            if (qPlayer.IsOut != 0) {
                OnscreenBitmap.BlitFromT(MenuBms[1 + c + 4], p.x, p.y + 3);
            } else {
                OnscreenBitmap.BlitFromT(MenuBms[1 + c], p.x, p.y + 3);

                OnscreenBitmap.PrintAt(qPlayer.AirlineX, FontBigGrey, TEC_FONT_LEFT, p.x + 51, p.y + 5, 600, 340);

                SLONG diff = Sim.Difficulty;
                if (diff == DIFF_FREEGAME) {
                    diff = DIFF_FIRST;
                }

                OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 1300 + diff), FontBigGrey, TEC_FONT_LEFT, p.x + 51, p.y + 33, 600, 340);

                OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 6010), FontBigGrey, TEC_FONT_LEFT, p.x + 51, p.y + 15 + 51, 600, 340);
                OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 6011), FontBigGrey, TEC_FONT_LEFT, p.x + 51, p.y + 15 + 69, 600, 340);
                OnscreenBitmap.PrintAt(StandardTexte.GetS(TOKEN_MISC, 6012), FontBigGrey, TEC_FONT_LEFT, p.x + 51, p.y + 15 + 87, 600, 360);

                // Missionsfortschritt:
                if (diff == DIFF_EASY) {
                    OnscreenBitmap.PrintAt(Einheiten[EINH_XDM].bString(qPlayer.GetMissionRating()), FontBigGrey, TEC_FONT_RIGHT, p.x, p.y + 33, p.x + 240, 340);
                } else if (diff == DIFF_HARD) {
                    OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(qPlayer.GetMissionRating()), FontBigGrey, TEC_FONT_RIGHT, p.x, p.y + 33, p.x + 240, 340);
                } else if (diff == DIFF_ATFS07) {
                    OnscreenBitmap.PrintAt(Insert1000erDots(qPlayer.GetMissionRating()) + "/" + Insert1000erDots(qPlayer.GetMissionRating(true)), FontBigGrey,
                                           TEC_FONT_RIGHT, p.x, p.y + 33, p.x + 240, 340);
                } else {
                    SLONG r = qPlayer.GetMissionRating();
                    if (r < 10000000) {
                        OnscreenBitmap.PrintAt(Insert1000erDots(r), FontBigGrey, TEC_FONT_RIGHT, p.x, p.y + 33, p.x + 240, 340);
                    } else {
                        OnscreenBitmap.PrintAt(Insert1000erDots64(r / 1000000) + ETexte.GetS(1000, 1000 + EINH_MIO), FontBigGrey, TEC_FONT_RIGHT, p.x, p.y + 33,
                                               p.x + 240, 340);
                    }
                }

                // Äußerung zu den Flugzeugen:
                SLONG d = 0;
                SLONG tmp = 0;
                SLONG tmp2 = 0;
                for (d = tmp = tmp2 = 0; d < qPlayer.Planes.AnzEntries(); d++) {
                    if (qPlayer.Planes.IsInAlbum(d) != 0) {
                        tmp += qPlayer.Planes[d].Zustand;
                        tmp2++;
                    }
                }

                OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(qPlayer.Image / 10), FontBigGrey, TEC_FONT_RIGHT, p.x, p.y + 15 + 51, p.x + 240, 340);

                if (qPlayer.Statistiken[STAT_MITARBEITER].GetAtPastDay(0) != 0) {
                    OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString64(qPlayer.Statistiken[STAT_ZUFR_PERSONAL].GetAtPastDay(0) * 100 /
                                                                       qPlayer.Statistiken[STAT_MITARBEITER].GetAtPastDay(0)),
                                           FontBigGrey, TEC_FONT_RIGHT, p.x, p.y + 15 + 69, p.x + 240, 340);
                }

                if (tmp2 != 0) {
                    OnscreenBitmap.PrintAt(Einheiten[EINH_P].bString(tmp / tmp2), FontBigGrey, TEC_FONT_RIGHT, p.x, p.y + 15 + 87, p.x + 240, 360);
                }

                bool Important = false;

                if (qPlayer.HasWon() != 0) {
                    Important = true;
                }

                // Add-On Mission #3 Dauert immer 21 und die Mission #4 30 Tage:
                if (Sim.Difficulty == DIFF_ADDON03 && Sim.Date < TARGET_DAYS) {
                    Important = false;
                }
                if (Sim.Difficulty == DIFF_ADDON04 && Sim.Date < TARGET_MILESDAYS) {
                    Important = false;
                }
                if (Sim.Difficulty == DIFF_ADDON06 && Sim.Date < TARGET_VALUEDAYS) {
                    Important = false;
                }
                if (Sim.Difficulty == DIFF_ATFS09 && Sim.Date < BTARGET_NDAYS9) {
                    Important = false;
                }
                if (Sim.Difficulty == DIFF_ATFS10 && Sim.Date < BTARGET_NDAYS10) {
                    Important = false;
                }

                if (qPlayer.Image < -990 || qPlayer.Money < DEBT_GAMEOVER) {
                    Important = true;
                }

                if (Important) {
                    MenuPar1 = 1;
                    OnscreenBitmap.BlitFromT(MenuBms[9], p.x + 37 + 36 - 65, p.y + 13 + 70 + 18 + 4);
                }

#ifdef DEMO
                if (Sim.Date >= 30)
                    MenuPar1 = 1;
#endif

                if ((Sim.Overtake != 0) && Sim.OvertakenAirline == c) {
                    MenuPar1 = 1;
                    OnscreenBitmap.BlitFromT(MenuBms[9], p.x + 37 + 36 - 65, p.y + 13 + 70 + 18 + 4);
                }
            }

            for (SLONG d = 0; d < Sim.SabotageActs.AnzEntries(); d++) {
                if (Sim.SabotageActs[d].Player != -1) {
                    XY p = XY(15 + 46 + (Sim.SabotageActs[d].Opfer & 1) * (282 - 8), 15 + 68 + (Sim.SabotageActs[d].Opfer / 2) * 153);
                    OnscreenBitmap.BlitFromT(MenuBms[10], p.x + 32 - 46 + 8, p.y + 55 + 70 - 68);

                    if (Sim.Players.Players[Sim.SabotageActs[d].Player].ArabHints >= 100) {
                        MenuPar1 = 1;
                        XY p = XY(15 + 46 + (Sim.SabotageActs[d].Player & 1) * (282 - 8), 15 + 68 + (Sim.SabotageActs[d].Player / 2) * 153);

                        OnscreenBitmap.BlitFromT(MenuBms[9], p.x + 37 + 36 - 65, p.y + 13 + 70 + 18 + 4);
                    }
                }
            }
        }

        OnscreenBitmap.BlitFrom(MenuBms[11], 136, 385);
        if (MenuPar1 == 0) {
            OnscreenBitmap.BlitFrom(MenuBms[13], 325, 385);
        }
    } break;

    case MENU_CALLITADAY:
        OnscreenBitmap.BlitFrom(MenuBms[0]);
        for (c = 0; c < 4; c++) {
            if (Sim.Players.Players[c].IsOut == 0 && (Sim.Players.Players[c].Owner == 0 || Sim.Players.Players[c].Owner == 2)) {
                SLONG add = Sim.Players.Players[c].CallItADay;

                OnscreenBitmap.BlitFrom(MenuBms[1 + c + 4 - 4 * add], 30 - 14 + c * 48, 76);
            }
        }
        break;
    default:
        break;
    }
}

//--------------------------------------------------------------------------------------------
// Returns true if the request menu is "plain" i.e. it ows only one okay-button (instead of yes/no)
//--------------------------------------------------------------------------------------------
bool CStdRaum::MenuIsPlain() const {
    switch (MenuPar1) {
    case MENU_REQUEST_NORENTROUTE1:
    case MENU_REQUEST_NORENTROUTE2:
    case MENU_REQUEST_NORENTROUTE3:
    case MENU_REQUEST_NORENTROUTE4:
    case MENU_REQUEST_FORBIDDEN:
    case MENU_REQUEST_DESTROYED:
    case MENU_REQUEST_ITEMS:
    case MENU_REQUEST_NOGLUE:
    case MENU_REQUEST_NOSTENCH:
    case MENU_REQUEST_NET_NUM:
    case MENU_REQUEST_NET_LOADONE:
    case MENU_REQUEST_NET_LOADTHIS:
    case MENU_REQUEST_NET_VERSION:
    case MENU_REQUEST_BETATEST:
    case MENU_REQUEST_BETATEST2:
        return true;
        break;
    default:
        break;
    }

    if (MenuPar1 >= MENU_REQUEST_NO_LM && MenuPar1 <= MENU_REQUEST_NO_WERBUNG) {
        return (true);
    }

    return (false);
}

//--------------------------------------------------------------------------------------------
// Nachträgliche Menu-Veränderungen, wie z.B. Icon Highlights
//--------------------------------------------------------------------------------------------
void CStdRaum::MenuPostPaint() {
    XY Pos = gMousePosition - MenuPos;

    // Special functions:
    switch (CurrentMenu) {
    case MENU_ADROUTE:
        if (Pos.IfIsWithin(216, 6, 387, 212)) {
            SLONG n = (Pos.y - 25) / 13 + MenuPage;

            if (n >= 0 && n < SLONG(MenuDataTable.AnzRows) && (Routen.IsInAlbum(MenuDataTable.LineIndex[n]) != 0)) {
                SBBM Bitmap(OnscreenBitmap.Size);
                CRentRoute &qRRoute = Sim.Players.Players[PlayerNum].RentRouten.RentRouten[Routen(MenuDataTable.LineIndex[n])];

                DrawRouteTipContents(Bitmap, PlayerNum, MenuDataTable.LineIndex[n], -2, 0, 0, 0, qRRoute.Ticketpreis, qRRoute.TicketpreisFC, XY(6, 6),
                                     XY(6, 28), &FontSmallBlack, &FontSmallBlack);

                PrimaryBm.BlitFromT(Bitmap, 135, 119);

                ReferenceCursorPos = gMousePosition;
                CheckCursorHighlight(Pos, CRect(32 + 180, n * 13 + 23, 204 + 180, n * 13 + 23 + 15), ColorOfFontBlack);
            }
        }
        break;
    default:
        break;
    }

    // Basic Repainting:
    switch (CurrentMenu) {
    case MENU_LETTERS:
        if (MouseClickArea == -100 && MouseClickId == 10) {
            PrimaryBm.BlitFrom(MenuBms[3], MenuPos + XY(131, 358));
        }
        if (MouseClickArea == -100 && MouseClickId == 11) {
            PrimaryBm.BlitFrom(MenuBms[4], MenuPos + XY(162, 358));
        }
        break;

    case MENU_AKTIE:
    case MENU_SETRENDITE:
    case MENU_BUYKEROSIN:
        if (MouseClickArea == -102) {
            if (MouseClickPar1 == 10) {
                PrimaryBm.BlitFrom(MenuBms[1], MenuPos + XY(248, 125));
            }
            if (MouseClickPar1 != 11) {
                PrimaryBm.BlitFrom(MenuBms[2], MenuPos + XY(168, 137));
            }
        } else {
            PrimaryBm.BlitFrom(MenuBms[2], MenuPos + XY(168, 137));
        }
        break;

    case MENU_BANK:
        if (MouseClickArea == -102) {
            if (MouseClickPar1 == 10) {
                PrimaryBm.BlitFrom(MenuBms[1], MenuPos + XY(215, 156));
            }
            if (MouseClickPar1 == 11) {
                PrimaryBm.BlitFrom(MenuBms[2], MenuPos + XY(136, 169));
            }
        }
        break;

    case MENU_BRIEFING:
        OnscreenBitmap.BlitFrom(MenuBms[11 + static_cast<SLONG>(MouseClickPar1 == 1)], 136, 385);
        if (MenuPar1 == 0) {
            OnscreenBitmap.BlitFrom(MenuBms[13 + static_cast<SLONG>(MouseClickPar1 == 2)], 325, 385);
        }
        break;
    default:
        break;
    }
}

//--------------------------------------------------------------------------------------------
// Berichtet, ob zur Zeit ein Menü offen ist:
//--------------------------------------------------------------------------------------------
BOOL CStdRaum::MenuIsOpen() const { return static_cast<BOOL>(CurrentMenu != MENU_NONE); }

//--------------------------------------------------------------------------------------------
// Reaktion auf linken Mausklick im Menü:
//--------------------------------------------------------------------------------------------
void CStdRaum::MenuLeftClick(XY Pos) {
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    Pos -= MenuPos;

    if (CalculatorIsOpen != 0) {
        CalcClick();
        return;
    }

    switch (CurrentMenu) {
    case MENU_BANK:
        // Betrag einstellen:
        if (MouseClickArea == -102 && MouseClickPar1 == 11) {
            if (Sim.Options.OptionEffekte != 0) {
                PaperTearFX.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            }
            MenuStop();
        }

        if (MouseClickArea == -102 && MouseClickPar1 == 12) {
            CalcOpen(MenuPos + XY(276, 60), MenuPar1);
        }

        // Kredit aufnehmen:
        if (MenuPar2 == 1) {
            Limit(static_cast<SLONG>(0), MenuPar1, static_cast<SLONG>(qPlayer.CalcCreditLimit()));
            MenuRepaint();

            if (MouseClickArea == -102 && MouseClickPar1 == 10) {
                if (MenuPar1 >= 1000) {
                    qPlayer.Credit += MenuPar1;
                    qPlayer.ChangeMoney(MenuPar1, 2003, "");
                    PLAYER::NetSynchronizeMoney();
                    MenuStop();
                    StartDialog(TALKER_BANKER2, MEDIUM_AIR, 101);
                } else {
                    MenuStop();
                    StartDialog(TALKER_BANKER2, MEDIUM_AIR, 100);
                }
            }
        } else // Kredit zurückzahlen
        {
            Limit(static_cast<SLONG>(0), MenuPar1, static_cast<SLONG>(qPlayer.Credit));
            MenuRepaint();

            if (MouseClickArea == -102 && MouseClickPar1 == 10) {
                qPlayer.Credit -= MenuPar1;
                qPlayer.ChangeMoney(-MenuPar1, 2004, "");
                PLAYER::NetSynchronizeMoney();
                MenuStop();
                StartDialog(TALKER_BANKER2, MEDIUM_AIR, 102);
            }
        }
        break;

    case MENU_KEROSIN:
        if (MouseClickArea == -102 && MouseClickId == MENU_KEROSIN && MouseClickPar1 == -20) {
            qPlayer.TankOpen ^= 1;
            qPlayer.NetUpdateKerosin();
            MenuRepaint();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_KEROSIN && MouseClickPar1 == -99) {
            MenuStop();
        }
        break;

    case MENU_BUYKEROSIN:
        if (MouseClickArea == -102 && MouseClickId == MENU_BUYKEROSIN && MouseClickPar1 == -20) {
            MenuInfo2 = 0;
            ColorFX.HighlightText(OnscreenBitmap.pBitmap, CRect(126, 87, 223, 100), ColorOfFontBlack, 0x20ff64);
            CalcOpen(MenuPos + XY(318, 51 + 16), MenuInfo);
        } else if (MouseClickArea == -102 && MouseClickId == MENU_BUYKEROSIN && MouseClickPar1 == -21) {
            MenuInfo2 = 1;
            ColorFX.HighlightText(OnscreenBitmap.pBitmap, CRect(223, 87, 254, 100), ColorOfFontBlack, 0x20ff64);
            CalcOpen(MenuPos + XY(318, 51 + 16), MenuInfo);
        } else if (MouseClickArea == -102 && MouseClickId == MENU_BUYKEROSIN && MouseClickPar1 == -22) {
            MenuInfo2 = 2;
            ColorFX.HighlightText(OnscreenBitmap.pBitmap, CRect(254, 87, 322, 100), ColorOfFontBlack, 0x20ff64);
            CalcOpen(MenuPos + XY(318, 51 + 16), MenuInfo);
        }
        if (MouseClickArea == -102 && MouseClickId == MENU_BUYKEROSIN && MouseClickPar1 == 11) {
            MenuStop();
        }
        if (MouseClickArea == -102 && MouseClickId == MENU_BUYKEROSIN && MouseClickPar1 == 10) {
            auto preis = MenuInfo;
            auto menge = MenuPar2;
            if ((preis != 0) && qPlayer.Money - preis < DEBT_LIMIT) {
                MenuStop();
                MakeSayWindow(0, TOKEN_ARAB, 6000, pFontPartner);
            } else if (preis != 0) {
                SLONG OldInhalt = qPlayer.TankInhalt;
                qPlayer.TankInhalt += menge;

                qPlayer.TankPreis = (OldInhalt * qPlayer.TankPreis + preis) / qPlayer.TankInhalt;
                qPlayer.KerosinQuali = (OldInhalt * qPlayer.KerosinQuali + menge * qPlayer.KerosinKind) / qPlayer.TankInhalt;
                qPlayer.NetUpdateKerosin();

                qPlayer.ChangeMoney(-preis, 2020, "");
                SIM::SendSimpleMessage(ATNET_CHANGEMONEY, 0, Sim.localPlayer, -preis, -1);

                qPlayer.DoBodyguardRabatt(preis);

                MakeSayWindow(0, TOKEN_ARAB, 700, pFontPartner);
            }
            MenuStop();
        }
        break;

    case MENU_KONTOAUSZUG:
        if (MouseClickArea == -102 && MouseClickId == MENU_KONTOAUSZUG && (MouseClickPar1 == 1 || MouseClickPar1 == -1)) {
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);

            MenuPage += MouseClickPar1;
            if (MenuPage > 9) {
                MenuPage = 9;
            }
            if (MenuPage < 0) {
                MenuPage = 0;
            }

            MenuRepaint();
        } else {
            MenuStop();
        }
        break;

    case MENU_FLUEGE:
        MenuStop();
        break;

    case MENU_QUITMESSAGE:
        SDL_Quit();
        Sim.Gamestate = GAMESTATE_QUIT;
        bLeaveGameLoop = TRUE;
        break;

    case MENU_FILOFAX:
        if (MouseClickArea == -101 && MouseClickId == MENU_FILOFAX) {
            if (MenuPar1 != 2 || (HandyRoomRemapper[MouseClickPar2] != ROOM_WORLD && HandyRoomRemapper[MouseClickPar2] != 254)) {
                MenuStop();
            }

            if (MenuPar1 == 2) // Handy
            {
                Ferngespraech = FALSE;
                LetzteEinheit = Sim.Time;
                LastScrollTime = -1;
                if ((qPlayer.IsLocationInQueue(HandyRoomRemapper[MouseClickPar2]) == 0) && (qPlayer.TelephoneDown == 0)) {
                    if (HandyRoomRemapper[MouseClickPar2] != ROOM_PERSONAL_A) {
                        if (Sim.RoomBusy[static_cast<SLONG>(HandyRoomRemapper[MouseClickPar2])] != 0U) {
                            goto phone_busy;
                        }
                    }

                    if (qPlayer.DialogWin != nullptr) {
                        delete qPlayer.DialogWin;
                        qPlayer.DialogWin = nullptr;
                    }

                    switch (HandyRoomRemapper[MouseClickPar2]) {
                    case ROOM_ARAB_AIR:
                        if ((Talkers.Talkers[TALKER_ARAB].IsBusy() != 0) || SLONG(Sim.Time) < timeArabOpen || Sim.Weekday == 6) {
                            goto phone_busy;
                        }
                        qPlayer.DialogWin = new CArabAir(TRUE, PlayerNum);
                        StartDialog(TALKER_ARAB, MEDIUM_HANDY, 0);
                        break;

                    case ROOM_AUFSICHT:
                        if (Talkers.Talkers[TALKER_BOSS].IsBusy() != 0) {
                            goto phone_busy;
                        }
                        qPlayer.DialogWin = new CAufsicht(TRUE, PlayerNum);
                        StartDialog(TALKER_BOSS, MEDIUM_HANDY);
                        break;

                    case ROOM_MUSEUM:
                        if ((Talkers.Talkers[TALKER_MUSEUM].IsBusy() != 0) || (SLONG(Sim.Time) < timeMuseOpen || Sim.Weekday == 5 || Sim.Weekday == 6)) {
                            goto phone_busy;
                        }
                        qPlayer.DialogWin = new CMuseum(TRUE, PlayerNum);
                        StartDialog(TALKER_MUSEUM, MEDIUM_HANDY);
                        break;

                    case ROOM_MAKLER:
                        if ((Talkers.Talkers[TALKER_MAKLER].IsBusy() != 0) || SLONG(Sim.Time) > timeMaklClose) {
                            goto phone_busy;
                        }
                        qPlayer.DialogWin = new CMakler(TRUE, PlayerNum);
                        StartDialog(TALKER_MAKLER, MEDIUM_HANDY);
                        break;

                    case ROOM_WERKSTATT:
                        if (Talkers.Talkers[TALKER_MECHANIKER].IsBusy() != 0) {
                            goto phone_busy;
                        }
                        qPlayer.DialogWin = new CWerkstatt(TRUE, PlayerNum);
                        StartDialog(TALKER_MECHANIKER, MEDIUM_HANDY, 1);
                        break;

                    case ROOM_WERBUNG:
                        if ((Talkers.Talkers[TALKER_WERBUNG].IsBusy() != 0) || SLONG(Sim.Time) < timeWerbOpen || Sim.Weekday == 5 || Sim.Weekday == 6) {
                            goto phone_busy;
                        }
                        qPlayer.DialogWin = new CWerbung(TRUE, PlayerNum);
                        StartDialog(TALKER_WERBUNG, MEDIUM_HANDY, 1);
                        break;

                    case ROOM_NASA:
                        if (Talkers.Talkers[TALKER_NASA].IsBusy() != 0) {
                            goto phone_busy;
                        }
                        qPlayer.DialogWin = new CNasa(TRUE, PlayerNum);
                        StartDialog(TALKER_NASA, MEDIUM_HANDY, 1);
                        break;

                    case ROOM_BANK:
                        if (MouseClickPar2 > 0 && HandyRoomRemapper[MouseClickPar2 - 1] != HandyRoomRemapper[MouseClickPar2]) {
                            if (Talkers.Talkers[TALKER_BANKER2].IsBusy() != 0) {
                                goto phone_busy;
                            }
                            qPlayer.DialogWin = new Bank(TRUE, PlayerNum);
                            StartDialog(TALKER_BANKER2, MEDIUM_HANDY);
                        } else {
                            if (Talkers.Talkers[TALKER_BANKER2].IsBusy() != 0) {
                                goto phone_busy;
                            }
                            qPlayer.DialogWin = new Bank(TRUE, PlayerNum);
                            StartDialog(TALKER_BANKER1, MEDIUM_HANDY);
                        }
                        break;

                    case ROOM_PERSONAL_A:
                        if (MouseClickPar2 > 0 && HandyRoomRemapper[MouseClickPar2 - 1] != HandyRoomRemapper[MouseClickPar2]) {
                            qPlayer.DialogWin = new CPersonal(TRUE, PlayerNum);
                            StartDialog(TALKER_PERSONAL1a + PlayerNum * 2, MEDIUM_HANDY);
                            (qPlayer.DialogWin)->HandyOffset = 280;
                        } else {
                            qPlayer.DialogWin = new CPersonal(TRUE, PlayerNum);
                            StartDialog(TALKER_PERSONAL1b + PlayerNum * 2, MEDIUM_HANDY);
                            (qPlayer.DialogWin)->HandyOffset = 120;
                        }
                        break;

                    case ROOM_WORLD:
#ifdef DEMO
                        MenuStart(MENU_REQUEST, MENU_REQUEST_NO_WORLD);
                        MenuSetZoomStuff(XY(320, 220), 0.17, FALSE);

                        // Hörer wieder auflegen:
                        if (qPlayer.GetRoom() == ROOM_BURO_A + PlayerNum * 10) {
                            ((CBuero *)this)->SP_Player.SetDesiredMood(SPM_IDLE);
                        }
#else
                        CurrentMenu = MENU_BRANCHLIST;
                        MenuPage = 0;
                        MenuRepaint();
#endif
                        break;

                    case 254:
                        CurrentMenu = MENU_PLAYERLIST;
                        MenuPage = 0;
                        MenuRepaint();
                        break;
                    default:
                        hprintf("StdRaum.cpp: Default case should not be reached.");
                        DebugBreak();
                    }

                    if (HandyRoomRemapper[MouseClickPar2] != ROOM_WORLD && HandyRoomRemapper[MouseClickPar2] != 254) {
                        if ((IsDialogOpen() != 0) && (Sim.Options.OptionEffekte != 0)) {
                            if (rand() % 2 == 0) {
                                Dial1FX.ReInit("dial1.raw");
                                Dial1FX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                            } else {
                                Dial2FX.ReInit("dial2.raw");
                                Dial2FX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                            }
                        }

                        qPlayer.History.AddNewCall(UsingHandy + Ferngespraech * 2);
                        qPlayer.Money -= 1;

                        qPlayer.Statistiken[STAT_A_SONSTIGES].AddAtPastDay(-1);
                        qPlayer.Bilanz.SonstigeAusgaben -= 1;
                        if (PlayerNum == Sim.localPlayer) {
                            SIM::SendSimpleMessage(ATNET_CHANGEMONEY, 0, Sim.localPlayer, -1, STAT_A_SONSTIGES);
                        }
                    }
                } else if (Sim.Options.OptionEffekte != 0) {
                phone_busy:
                    DialBusyFX.ReInit("busy.raw");
                    DialBusyFX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                }
            } else if (qPlayer.IsLocationInQueue(FilofaxRoomRemapper[MouseClickPar2]) == 0) {
                gMouseScroll = 0;
                qPlayer.LeaveAllRooms();
                if (FilofaxRoomRemapper[MouseClickPar2] == ROOM_BURO_A || FilofaxRoomRemapper[MouseClickPar2] == ROOM_PERSONAL_A) {
                    qPlayer.WalkToRoom(FilofaxRoomRemapper[MouseClickPar2] + PlayerNum * 10);
                } else {
                    qPlayer.WalkToRoom(FilofaxRoomRemapper[MouseClickPar2]);
                }

                Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Running = TRUE;
            }
        }
        // Klick ins Inventar?
        else if (gMousePosition.x >= WinP1.x + (WinP2.x - WinP1.x) * 4 / 10 && gMousePosition.y >= WinP2.y - StatusLineSizeY && gMousePosition.x < 634 - 24 &&
                 gMousePosition.y < WinP2.y) {
            SLONG ItemIndex = (gMousePosition.x - 250) / 60;

            // Klick ins Inventar erfolgreich?
            if (ItemIndex < 6 && MenuPar1 == 2 && Sim.bNoTime == FALSE && qPlayer.Items[ItemIndex] == ITEM_FILOFAX) {
                MenuDialogReEntryB = -1;
                MenuStop();
                MenuStart(MENU_FILOFAX, 1);
                return;
            }
            if (ItemIndex < 6 && MenuPar1 == 1 && Sim.bNoTime == FALSE && qPlayer.Items[ItemIndex] == ITEM_HANDY) {
                MenuDialogReEntryB = -1;
                MenuStop();
                UsingHandy = TRUE;
                MenuStart(MENU_FILOFAX, 2);
                return;
            }
        }
        break;

    case MENU_BRANCHLIST:
        if (MouseClickPar1 == -1) {
            MenuPage--;

            if (MenuPage >= 0) {
                MenuRepaint();
                gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            } else {
                CurrentMenu = MENU_FILOFAX;
                MenuPage = 0;
                MenuPar1 = 2; // Handy
                MenuRepaint();
                if (Sim.Options.OptionEffekte != 0) {
                    gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                }
                return;
            }
        } else if (MouseClickPar1 == -2) {
            MenuPage++;
            MenuRepaint();
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
        } else if (MouseClickArea == -101) {
            SLONG c = 0;

            MenuStop();

            if (DialogPar1 == Sim.HomeAirportId) {
                Ferngespraech = 1;
            } else {
                Ferngespraech = 2;
            }

            LetzteEinheit = Sim.Time;

            for (c = 0; c < 4; c++) {
                if ((Sim.Players.Players[c].Kooperation[Sim.localPlayer] != 0) && (Sim.Players.Players[c].RentCities.RentCities[MouseClickPar2].Rang != 0U) &&
                    (Sim.Players.Players[c].IsOut == 0)) {
                    break;
                }
            }

            if (Sim.Players.Players[Sim.localPlayer].RentCities.RentCities[MouseClickPar2].Rang != 0U) {
                c = Sim.localPlayer;
            }

            if (Sim.Options.OptionEffekte != 0) {
                if (rand() % 2 == 0) {
                    Dial1FX.ReInit("dial1.raw");
                    Dial1FX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                } else {
                    Dial2FX.ReInit("dial2.raw");
                    Dial2FX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                }
            }

            qPlayer.History.AddNewCall(UsingHandy + Ferngespraech * 2);
            qPlayer.Money -= 1;

            qPlayer.Statistiken[STAT_A_SONSTIGES].AddAtPastDay(-1);
            qPlayer.Bilanz.SonstigeAusgaben -= 1;
            if (PlayerNum == Sim.localPlayer) {
                SIM::SendSimpleMessage(ATNET_CHANGEMONEY, 0, Sim.localPlayer, -1, STAT_A_SONSTIGES);
            }

            LastMenu = -1;
            StartDialog(TALKER_WORLD, MEDIUM_HANDY, c, MouseClickPar2);
        }
        // Klick ins Inventar?
        else if (gMousePosition.x >= WinP1.x + (WinP2.x - WinP1.x) * 4 / 10 && gMousePosition.y >= WinP2.y - StatusLineSizeY && gMousePosition.x < 634 - 24 &&
                 gMousePosition.y < WinP2.y) {
            SLONG ItemIndex = (gMousePosition.x - 250) / 60;

            // Klick ins Inventar erfolgreich?
            if (ItemIndex < 6 && MenuPar1 == 2 && Sim.bNoTime == FALSE && qPlayer.Items[ItemIndex] == ITEM_FILOFAX) {
                MenuDialogReEntryB = -1;
                MenuStop();
                MenuStart(MENU_FILOFAX, 1);
                return;
            }
        }
        break;

    case MENU_PLAYERLIST:
        if (MouseClickPar1 == -1) {
            CurrentMenu = MENU_FILOFAX;
            MenuPage = 0;
            MenuPar1 = 2; // Handy
            MenuRepaint();
            if (Sim.Options.OptionEffekte != 0) {
                gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            }
            return;
        }

        if (MouseClickArea == -101 && MouseClickPar2 != Sim.localPlayer) {
            if ((MouseClickPar2 >= 0 && MouseClickPar2 <= 7 && (Sim.Players.Players[MouseClickPar2 % 4].IsOut == 0)) ||
                (MouseClickPar2 >= 4 && MouseClickPar2 <= 7 && (Sim.Players.Players[MouseClickPar2 - 4].IsOut == 0))) {
                MenuStop();
                Ferngespraech = FALSE;

                LetzteEinheit = Sim.Time;

                BOOL bHandy = static_cast<BOOL>(MouseClickPar2 >= 4);
                MouseClickPar2 %= 4;

                PLAYER &qOther = Sim.Players.Players[MouseClickPar2];

                if ((bHandy != 0) && (qOther.HasItem(ITEM_HANDY) == 0)) {
                    break;
                }

                if (qOther.Owner == 2) {
                    // Telefonat mit Netzwerkspieler:
                    Dial1FX.ReInit("dial1.raw");
                    Dial1FX.Play(0, Sim.Options.OptionEffekte * 100 / 7);

                    SIM::SendSimpleMessage(ATNET_PHONE_DIAL, qOther.NetworkID, PlayerNum, bHandy);
                } else {
                    // Telefonat mit Computerspieler:
                    if (qPlayer.TelephoneDown != 0) {
                        if (Sim.Options.OptionEffekte != 0) {
                            DialBusyFX.ReInit("busy.raw");
                            DialBusyFX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                        }
                        if (qPlayer.GetRoom() == ROOM_BURO_A + PlayerNum * 10) {
                            (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_IDLE);
                        }
                    } else if ((bHandy == 0 && qOther.GetRoom() != ROOM_BURO_A + MouseClickPar2 * 10) || (qOther.IsStuck != 0)) {
                        if (Sim.Options.OptionEffekte != 0) {
                            DialBusyFX.ReInit("noanswer.raw");
                            DialBusyFX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                        }
                        if (qPlayer.GetRoom() == ROOM_BURO_A + PlayerNum * 10) {
                            (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_IDLE);
                        }
                    } else if (qOther.IsTalking != 0) {
                        if (Sim.Options.OptionEffekte != 0) {
                            DialBusyFX.ReInit("busy.raw");
                            DialBusyFX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                        }
                        if (qPlayer.GetRoom() == ROOM_BURO_A + PlayerNum * 10) {
                            (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_IDLE);
                        }
                    } else if (qOther.BoredOfPlayer != 0) {
                        if (Sim.Options.OptionEffekte != 0) {
                            DialBusyFX.ReInit("noanswer.raw");
                            DialBusyFX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                        }
                        if (qPlayer.GetRoom() == ROOM_BURO_A + PlayerNum * 10) {
                            (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_IDLE);
                        }
                    } else {
                        if (Sim.Options.OptionEffekte != 0) {
                            if (rand() % 2 == 0) {
                                Dial1FX.ReInit("dial1.raw");
                                Dial1FX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                            } else {
                                Dial2FX.ReInit("dial2.raw");
                                Dial2FX.Play(0, Sim.Options.OptionEffekte * 100 / 7);
                            }
                        }

                        qPlayer.History.AddNewCall(UsingHandy + Ferngespraech * 2);
                        qPlayer.Money -= 1;

                        qPlayer.Statistiken[STAT_A_SONSTIGES].AddAtPastDay(-1);
                        qPlayer.Bilanz.SonstigeAusgaben -= 1;
                        if (PlayerNum == Sim.localPlayer) {
                            SIM::SendSimpleMessage(ATNET_CHANGEMONEY, 0, Sim.localPlayer, -1, STAT_A_SONSTIGES);
                        }

                        StartDialog(TALKER_COMPETITOR, MEDIUM_HANDY, MouseClickPar2, 0);

                        // Die Figur aus der Animation rausreissen:
                        Sim.Players.Players[MouseClickPar2].DisplayAsTelefoning();
                    }
                }
            }
            // Klick ins Inventar?
            else if (gMousePosition.x >= WinP1.x + (WinP2.x - WinP1.x) * 4 / 10 && gMousePosition.y >= WinP2.y - StatusLineSizeY &&
                     gMousePosition.x < 634 - 24 && gMousePosition.y < WinP2.y) {
                SLONG ItemIndex = (gMousePosition.x - 250) / 60;

                // Klick ins Inventar erfolgreich?
                if (ItemIndex < 6 && MenuPar1 == 2 && Sim.bNoTime == FALSE && qPlayer.Items[ItemIndex] == ITEM_FILOFAX) {
                    MenuDialogReEntryB = -1;
                    MenuStop();
                    MenuStart(MENU_FILOFAX, 1);
                    return;
                }
            }
        }
        break;

    case MENU_AUSLANDSAUFTRAG:
        if (MouseClickArea == -101 && MouseClickPar2 == 7 + 7) {
            if (qPlayer.CalledCities[MenuPar1] == 0) {
                MenuDialogReEntryB = 5000;
                MenuStop();
            } else {
                if (IsDialogOpen() != 0) {
                    StopDialog();
                }
                MenuStart(MENU_FILOFAX, 2);
                CurrentMenu = MENU_BRANCHLIST;
                MenuPage = 0;
                MenuRepaint();
                return;
            }
        } else if (MouseClickArea == -101 && MouseClickPar2 >= 0 && MouseClickPar2 <= 5) {
            CAuftrag &qAuftrag = AuslandsAuftraege[MenuPar1][MouseClickPar2];

            if (qAuftrag.Praemie != 0) {
                if (qPlayer.Auftraege.GetNumFree() < 3) {
                    qPlayer.Auftraege.ReSize(qPlayer.Auftraege.AnzEntries() + 10);
                }

                qPlayer.Auftraege += qAuftrag;
                qPlayer.NetUpdateOrder(qAuftrag);
                qPlayer.Statistiken[STAT_AUFTRAEGE].AddAtPastDay(1);

                // Blöcke refeshen:
                if (qPlayer.GetRoom() == ROOM_LAPTOP) {

                    for (SLONG c = qPlayer.Blocks.AnzEntries() - 1; c >= 1; c--) {
                        if (qPlayer.Blocks.IsInAlbum(c) != 0) {
                            BLOCK &qBlock = qPlayer.Blocks[ULONG(c)];

                            qBlock.RefreshData(PlayerNum);
                            if (qPlayer.LaptopBattery > 0) {
                                qBlock.Refresh(PlayerNum, TRUE);
                            }
                        }
                    }
                } else if (qPlayer.GetRoom() == ROOM_GLOBE) {
                    BLOCK &qBlock = qPlayer.Blocks[ULONG(0)];

                    qBlock.RefreshData(PlayerNum);
                    qBlock.Refresh(PlayerNum, FALSE);
                }

                qAuftrag.Praemie = 0;
                qPlayer.NetUpdateTook(4, MouseClickPar2, MenuPar1);
                MenuRepaint();
            }
        } else if (MouseClickArea == -101 && MouseClickPar2 >= 7 && MouseClickPar2 <= 7 + 5) {
            CFracht &qFracht = AuslandsFrachten[MenuPar1][MouseClickPar2 - 7];

            if (qFracht.Praemie > 0) {
                if (qPlayer.Frachten.GetNumFree() < 3) {
                    qPlayer.Frachten.ReSize(qPlayer.Frachten.AnzEntries() + 10);
                }

                qPlayer.Frachten += qFracht;
                qPlayer.NetUpdateFreightOrder(qFracht);
                qPlayer.Statistiken[STAT_AUFTRAEGE].AddAtPastDay(1);

                // Blöcke refeshen:
                if (qPlayer.GetRoom() == ROOM_LAPTOP) {

                    for (SLONG c = qPlayer.Blocks.AnzEntries() - 1; c >= 1; c--) {
                        if (qPlayer.Blocks.IsInAlbum(c) != 0) {
                            BLOCK &qBlock = qPlayer.Blocks[ULONG(c)];

                            qBlock.RefreshData(PlayerNum);
                            if (qPlayer.LaptopBattery > 0) {
                                qBlock.Refresh(PlayerNum, TRUE);
                            }
                        }
                    }
                } else if (qPlayer.GetRoom() == ROOM_GLOBE) {
                    BLOCK &qBlock = qPlayer.Blocks[ULONG(0)];

                    qBlock.RefreshData(PlayerNum);
                    qBlock.Refresh(PlayerNum, FALSE);
                }

                qFracht.Praemie = 0;
                qPlayer.NetUpdateTook(5, MouseClickPar2 - 7, MenuPar1);
                MenuRepaint();
            }
        }
        break;

    case MENU_PERSONAL:
        if (MouseClickId == MENU_PERSONAL) {
            if (MouseClickPar1 == -1 && MenuPage > 0) {
                if (AtGetAsyncKeyState(ATKEY_CONTROL) / 256 != 0) {
                    MenuPage = std::max(0, MenuPage - 100);
                } else if (AtGetAsyncKeyState(ATKEY_SHIFT) / 256 != 0) {
                    MenuPage = std::max(0, MenuPage - 10);
                } else {
                    MenuPage--;
                }
                gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            } else if (MouseClickPar1 == -2 && MenuPage < MenuPageMax) {
                if (AtGetAsyncKeyState(ATKEY_CONTROL) / 256 != 0) {
                    MenuPage = std::min(MenuPageMax, MenuPage + 100);
                } else if (AtGetAsyncKeyState(ATKEY_SHIFT) / 256 != 0) {
                    MenuPage = std::min(MenuPageMax, MenuPage + 10);
                } else {
                    MenuPage++;
                }
                gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            } else if (MouseClickPar1 == -3) {
                Workers.Workers[MenuRemapper[MenuPage - 1]].Employer = PlayerNum;
                Workers.Workers[MenuRemapper[MenuPage - 1]].PlaneId = -1;
                qPlayer.MapWorkers(TRUE);
                qPlayer.UpdateWalkSpeed();
            } else if (MouseClickPar1 == -4) {
                Workers.Workers[MenuRemapper[MenuPage - 1]].Employer = WORKER_JOBLESS;
                if (Workers.Workers[MenuRemapper[MenuPage - 1]].TimeInPool > 0) {
                    Workers.Workers[MenuRemapper[MenuPage - 1]].TimeInPool = 0;
                }
                qPlayer.MapWorkers(TRUE);
                qPlayer.UpdateWalkSpeed();
            }
            // Gehalt erhöhen/kürzen:
            else if (MouseClickPar1 == -5) {
                Workers.Workers[MenuRemapper[MenuPage - 1]].Gehaltsaenderung(1);
            } else if (MouseClickPar1 == -6) {
                Workers.Workers[MenuRemapper[MenuPage - 1]].Gehaltsaenderung(0);
            } else if (MouseClickPar1 == -7) {
                // Anderes Flugzeug:
                SLONG m1 = MenuPage;
                SLONG m2 = MenuRemapper[MenuPage - 1];

                MenuStart(MENU_PLANEJOB, MenuPar1, MenuPar2);

                MenuInfo2 = m1;
                MenuInfo3 = m2;
            }
            PersonalPageFlipped();
        }
        break;

    case MENU_PLANECOSTS:
        if (MouseClickArea == -102 && MouseClickId == MENU_PLANECOSTS && MouseClickPar1 == -3) {
            MenuStop();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_PLANECOSTS && MouseClickPar1 == -1) {
            MenuPrevPage();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_PLANECOSTS && MouseClickPar1 == -2) {
            MenuNextPage();
        }
        break;

    case MENU_PLANEREPAIRS:
        if (MouseClickArea == -102 && MouseClickId == MENU_PLANEREPAIRS && MouseClickPar1 == -20) {
            MenuStop();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_PLANEREPAIRS && MouseClickPar1 == -21) {
            for (SLONG c = 0; c < MenuDataTable.AnzRows; c++) {
                qPlayer.Planes[MenuDataTable.LineIndex[c]].TargetZustand = UBYTE(MenuInfo3);
                qPlayer.NetUpdatePlaneProps(MenuDataTable.LineIndex[c]);
            }

            MenuRepaint();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_PLANEREPAIRS && MouseClickPar1 >= 0) {
            SLONG n = MouseClickPar1;
            if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13 && n >= MenuPage) {
                MenuInfo = MenuDataTable.LineIndex[n];

                ColorFX.HighlightText(OnscreenBitmap.pBitmap, CRect(220, n * 12 + 53 - 2, 260, n * 12 + 51 + 13), ColorOfFontBlack, 0x20ff64);

                CalcOpen(MenuPos + XY(510 - 130 - 30, (n - MenuPage) * 12 + 30), 0);
            }
        } else if (MouseClickArea == -102 && MouseClickId == MENU_PLANEREPAIRS && MouseClickPar1 == -1) {
            MenuPrevPage();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_PLANEREPAIRS && MouseClickPar1 == -2) {
            MenuNextPage();
        }
        break;

    case MENU_SECURITY: //==>+<==
        if (MouseClickArea == -102 && MouseClickId == MENU_SECURITY && MouseClickPar1 == -20) {
            MenuStop();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_SECURITY && MouseClickPar1 == -21) {
            if (MouseClickPar2 >= 0 && MouseClickPar2 < 9) {
                if (MouseClickPar2 != 1 || (qPlayer.HasItem(ITEM_LAPTOP) != 0)) {
                    if (MouseClickPar2 == 8) {
                        if ((qPlayer.SecurityFlags & (1 << 11)) != 0U) {
                            qPlayer.SecurityFlags = qPlayer.SecurityFlags & ~(1 << 8) & ~(1 << 10) & ~(1 << 11);
                        } else if ((qPlayer.SecurityFlags & (1 << 10)) != 0U) {
                            qPlayer.SecurityFlags = qPlayer.SecurityFlags | (1 << 11);
                        } else if ((qPlayer.SecurityFlags & (1 << 8)) != 0U) {
                            qPlayer.SecurityFlags = qPlayer.SecurityFlags | (1 << 10);
                        } else {
                            qPlayer.SecurityFlags = qPlayer.SecurityFlags | (1 << 8);
                        }
                    } else {
                        qPlayer.SecurityFlags ^= (1 << MouseClickPar2);
                    }

                    PLAYER::NetSynchronizeFlags();
                }
            }

            MenuRepaint();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_SECURITY && MouseClickPar1 >= 0) {
            SLONG n = MouseClickPar1;
            if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13 && n >= MenuPage) {
                MenuInfo = MenuDataTable.LineIndex[n];

                ColorFX.HighlightText(OnscreenBitmap.pBitmap, CRect(220, n * 12 + 53 - 2, 260, n * 12 + 51 + 13), ColorOfFontBlack, 0x20ff64);

                CalcOpen(MenuPos + XY(510 - 130 - 30, (n - MenuPage) * 12 + 30), 0);
            }
        }
        break;

    case MENU_PANNEN:
        if (MouseClickArea == -102 && MouseClickId == MENU_PANNEN && MouseClickPar1 == -20) {
            if (MenuInfo3 == -1) {
                MenuStop();
            } else {
                MenuInfo3 = -1;
                MenuPage = 0;
                MenuRepaint();
            }
        } else if (MouseClickArea == -102 && MouseClickId == MENU_PANNEN && MouseClickPar1 >= 0) {
            SLONG n = MouseClickPar1;
            if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13 && n >= MenuPage) {
                MenuInfo3 = n;

                MenuRepaint();
            }
        } else if (MouseClickArea == -102 && MouseClickId == MENU_PANNEN && MouseClickPar1 == -1) {
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);

            if (MenuInfo3 == -1) {
                MenuPage = MenuPage - 13;
                if (MenuPage < 0) {
                    MenuPage = max(0, MenuDataTable.AnzRows - 1) / 13;
                }
            } else {
                MenuInfo3--;
                if (MenuInfo3 < 0) {
                    MenuInfo3 = 0;
                }
            }

            MenuRepaint();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_PANNEN && MouseClickPar1 == -2) {
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);

            if (MenuInfo3 == -1) {
                MenuPage = MenuPage + 13;
                if (MenuPage >= MenuDataTable.AnzRows) {
                    MenuPage = 0;
                }
            } else {
                MenuInfo3++;
                if (MenuInfo3 >= MenuDataTable.AnzRows) {
                    MenuInfo3 = MenuDataTable.AnzRows - 1;
                }
            }

            MenuRepaint();
        }
        break;

    case MENU_SABOTAGEPLANE:
        if (MouseClickArea == -102 && MouseClickId == MENU_SABOTAGEPLANE && MouseClickPar1 == -3) {
            MenuStop();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_SABOTAGEPLANE && MouseClickPar1 == -1) {
            MenuPrevPage();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_SABOTAGEPLANE && MouseClickPar1 == -2) {
            MenuNextPage();
        } else if (Pos.IfIsWithin(216, 6, 387, 212)) {
            SLONG n = ((Pos.y - 25)) / 13 + MenuPage;

            if (n >= 0 && n - MenuPage < 13 && n < MenuDataTable.AnzRows &&
                (Sim.Players.Players[qPlayer.ArabOpfer].Planes.IsInAlbum(MenuDataTable.LineIndex[n]) != 0)) {
                MenuPar1 = MenuDataTable.LineIndex[n];

                auto tmp = MenuDialogReEntryB;
                MenuStop();
                MenuDialogReEntryB = tmp;

                qPlayer.ArabPlane = MenuPar1;

                StartDialog(TALKER_SABOTAGE, MEDIUM_AIR, MenuDialogReEntryB);
            }
        }
        break;

    case MENU_SABOTAGEROUTE:
        if (MouseClickArea == -102 && MouseClickId == MENU_SABOTAGEROUTE && MouseClickPar1 == -3) {
            MenuStop();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_SABOTAGEROUTE && MouseClickPar1 == -1) {
            MenuPrevPage();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_SABOTAGEROUTE && MouseClickPar1 == -2) {
            MenuNextPage();
        } else if (Pos.IfIsWithin(216, 6, 387, 212)) {
            SLONG n = ((Pos.y - 25)) / 13 + MenuPage;

            if (n >= 0 && n - MenuPage < 13 && n < MenuDataTable.AnzRows &&
                Sim.Players.Players[qPlayer.ArabOpfer].RentRouten.RentRouten[Routen(MenuDataTable.LineIndex[n])].Rang != 0) {
                MenuPar1 = MenuDataTable.LineIndex[n];

                auto tmp = MenuDialogReEntryB;
                MenuStop();
                MenuDialogReEntryB = tmp;

                qPlayer.ArabPlane = MenuPar1;

                StartDialog(TALKER_SABOTAGE, MEDIUM_AIR, MenuDialogReEntryB);
            }
        }
        break;

    case MENU_SELLPLANE:
        if (MouseClickArea == -102 && MouseClickId == MENU_SELLPLANE && MouseClickPar1 == -3) {
            MenuStop();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_SELLPLANE && MouseClickPar1 == -1) {
            MenuPrevPage();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_SELLPLANE && MouseClickPar1 == -2) {
            MenuNextPage();
        } else if (Pos.IfIsWithin(216, 6, 387, 212)) {
            SLONG n = ((Pos.y - 25)) / 13 + MenuPage;

            if (n >= 0 && n < MenuDataTable.AnzRows && (qPlayer.Planes.IsInAlbum(MenuDataTable.LineIndex[n]) != 0)) {
                MenuPar1 = MenuDataTable.LineIndex[n];

                MenuStop();

                // Das Flugzeug kann nur verkauft werden, wenn keine Flüge darauf gebucht sind:
                if (qPlayer.Planes[MenuPar1].CanBeSold() != 0) {
                    StartDialog(TALKER_MUSEUM, MEDIUM_AIR, 100 + qPlayer.Planes[MenuPar1].Sponsored * 2, MenuPar1);
                } else {
                    StartDialog(TALKER_MUSEUM, MEDIUM_AIR, 101 + qPlayer.Planes[MenuPar1].Sponsored * 2, MenuPar1);
                }
            }
        }
        break;

    case MENU_BUYPLANE:
        if (MouseClickArea == -102 && MouseClickId == MENU_BUYPLANE && MouseClickPar1 == -3) {
            MenuStop();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_BUYPLANE && MouseClickPar1 == -1) {
            MenuPrevPage();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_BUYPLANE && MouseClickPar1 == -2) {
            MenuNextPage();
        } else if (Pos.IfIsWithin(216, 6, 387, 212)) {
            SLONG n = (Pos.y - 21) / 13 + MenuPage;

            if (n >= 0 && n <= MenuDataTable.AnzRows && (PlaneTypes.IsInAlbum(MenuDataTable.LineIndex[n]) != 0)) {
                MenuPar1 = MenuDataTable.LineIndex[n];
                MenuStop();
                StartDialog(TALKER_MAKLER, MEDIUM_AIR, 130 + rand() % 4, MenuPar1);
            }
        }
        break;

    case MENU_BUYXPLANE:
        if (MouseClickArea == -102 && MouseClickId == MENU_BUYXPLANE && MouseClickPar1 == -3) {
            MenuStop();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_BUYXPLANE && MouseClickPar1 == -1) {
            MenuPrevPage();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_BUYXPLANE && MouseClickPar1 == -2) {
            MenuNextPage();
        } else if (Pos.IfIsWithin(216, 6, 387, 212)) {
            SLONG n = (Pos.y - 21) / 13 + MenuPage;

            if (n >= 0 && n <= MenuDataTable.AnzRows) {
                MenuPar1 = n;
                MenuPar4 = MenuDataTable.Values[n * MenuDataTable.AnzColums + 2];
                MenuStop();
                StartDialog(TALKER_DESIGNER, MEDIUM_AIR, 6000, MenuPar1);
            }
        }
        break;

    case MENU_EXTRABLATT:
        MenuStop();
        Sim.Players.Players[Sim.localPlayer].WalkToRoom(UBYTE(ROOM_BURO_A + Sim.localPlayer * 10));
        break;

    case MENU_CLOSED:
        if (MouseClickArea == -101 && MouseClickId == MENU_CLOSED && MouseClickPar1 > 0) {
            MenuStop();
        }
        break;

    case MENU_REQUEST:
        switch (MenuPar1) {
        case MENU_REQUEST_CALLITADAY:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                if (qPlayer.GetRoom() == ROOM_AIRPORT && MenuPar2 == 1) {
                    qPlayer.NewDir = 0;

                    XY p = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Position;
                    XY ArrayPos;

                    ArrayPos.x = (p.x + 4400) / 44 - 100;
                    ArrayPos.y = (p.y + 2200) / 22 - 100 + 5 - 2;

                    qPlayer.WalkToPlate(ArrayPos);
                }

                if ((Sim.Options.OptionAutosave != 0) && (Sim.bNetwork == 0)) {
                    Sim.SaveGame(11, StandardTexte.GetS(TOKEN_MISC, 5000));
                }

                qPlayer.CallItADay = TRUE;
                Sim.DayState = 2;

                if (Sim.bNetwork != 0) {
                    MenuStart(MENU_CALLITADAY);
                    Sim.Players.Players[Sim.localPlayer].WalkStopEx();
                    SIM::SendSimpleMessage(ATNET_DAYFINISH, 0, PlayerNum);
                    // Sim.SendSimpleMessage (ATNET_DAYFINISH, qPlayer.NetworkID, PlayerNum);
                    SIM::SendChatBroadcast(bprintf(StandardTexte.GetS(TOKEN_MISC, 7020), Sim.Players.Players[Sim.localPlayer].NameX.c_str()));
                }
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 2) {
                if (qPlayer.GetRoom() == ROOM_AIRPORT && MenuPar2 == 1) {
                    qPlayer.NewDir = 0;

                    XY p = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Position;
                    XY ArrayPos;

                    ArrayPos.x = (p.x + 4400) / 44 - 100;
                    ArrayPos.y = (p.y + 2200) / 22 - 100 + 5 - 2;

                    qPlayer.WalkToPlate(ArrayPos);
                }

                MenuStop();
            }
            break;

        case MENU_REQUEST_THROWNOUT:
        case MENU_REQUEST_THROWNOUT2:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                // qPlayer.WaitForRoom=qPlayer.ThrownOutOfRoom;
                // qPlayer.ThrownOutOfRoom=0;
                MenuStop();
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 2) {
                qPlayer.WaitForRoom = 0;
                qPlayer.ThrownOutOfRoom = 0;
                MenuStop();
            }
            break;

        case MENU_REQUEST_WRONGROOM:
        case MENU_REQUEST_WRONGROOM2:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                qPlayer.WalkToRoom(static_cast<UBYTE>(qPlayer.ThrownOutOfRoom));
                qPlayer.ThrownOutOfRoom = 0;
                MenuStop();
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 2) {
                MenuStop();
            }
            break;

        case MENU_REQUEST_KILLCITY:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                BLOCKS &qBlocks = qPlayer.Blocks;
                for (SLONG c = 0; c < Routen.AnzEntries(); c++) {
                    if ((qBlocks.IsInAlbum(c) != 0) && qBlocks[c].Index == 0 && qBlocks[c].BlockType == 1 && Cities(qBlocks[c].SelectedId) == ULONG(MenuPar2)) {
                        qBlocks[c].Index = TRUE;
                        qBlocks[c].Page = 0;
                        qBlocks[c].RefreshData(PlayerNum);
                        qBlocks[c].AnzPages = max(0, (qBlocks[c].Table.AnzRows - 1) / 13) + 1;
                    }
                }

                qPlayer.RentCities.RentCities[MenuPar2].Rang = 0;
                qPlayer.Blocks.RepaintAll = TRUE;
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;

        case MENU_REQUEST_KILLROUTE:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                BLOCKS &qBlocks = qPlayer.Blocks;
                for (SLONG c = 0; c < Routen.AnzEntries(); c++) {
                    if ((qBlocks.IsInAlbum(c) != 0) && qBlocks[c].IndexB == 0 && qBlocks[c].BlockTypeB == 4 &&
                        (Routen(qBlocks[c].SelectedIdB) == ULONG(MenuPar2) || Routen(qBlocks[c].SelectedIdB) == ULONG(MenuPar3))) {
                        qBlocks[c].IndexB = TRUE;
                        qBlocks[c].PageB = 0;
                        qBlocks[c].RefreshData(PlayerNum);
                        qBlocks[c].AnzPagesB = max(0, (qBlocks[c].TableB.AnzRows - 1) / 6) + 1;
                    }
                }

                for (SLONG e = 0; e < 4; e++) {
                    if ((Sim.Players.Players[e].IsOut == 0) &&
                        Sim.Players.Players[e].RentRouten.RentRouten[MenuPar2].Rang > qPlayer.RentRouten.RentRouten[MenuPar2].Rang) {
                        Sim.Players.Players[e].RentRouten.RentRouten[MenuPar2].Rang--;
                    }

                    if ((Sim.Players.Players[e].IsOut == 0) &&
                        Sim.Players.Players[e].RentRouten.RentRouten[MenuPar3].Rang > qPlayer.RentRouten.RentRouten[MenuPar3].Rang) {
                        Sim.Players.Players[e].RentRouten.RentRouten[MenuPar3].Rang--;
                    }
                }

                if ((qPlayer.LocationWin != nullptr) && (qPlayer.GetRoom() == ROOM_LAPTOP || qPlayer.GetRoom() == ROOM_GLOBE)) {
                    CPlaner &qPlaner = *dynamic_cast<CPlaner *>(qPlayer.LocationWin);

                    if (qPlaner.CurrentPostItType == 1 && (qPlaner.CurrentPostItId == static_cast<SLONG>(Routen.GetIdFromIndex(MenuPar2)) ||
                                                           qPlaner.CurrentPostItId == static_cast<SLONG>(Routen.GetIdFromIndex(MenuPar3)))) {
                        qPlaner.CurrentPostItType = 0;
                    }
                }

                qPlayer.RentRouten.RentRouten[MenuPar2].Rang = 0;
                qPlayer.RentRouten.RentRouten[MenuPar3].Rang = 0;
                qPlayer.RentRouten.RentRouten[MenuPar2].TageMitGering = 99;
                qPlayer.RentRouten.RentRouten[MenuPar3].TageMitGering = 99;
                qPlayer.Blocks.RepaintAll = TRUE;

                if (qPlayer.GetRoom() == ROOM_ROUTEBOX) {
                    if (qPlayer.LocationWin != nullptr) {
                        (dynamic_cast<CRouteBox *>(qPlayer.LocationWin))->UpdateDataTable();
                    }
                }

                qPlayer.NetUpdateRentRoute(MenuPar2, MenuPar3);
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;

        case MENU_REQUEST_RENTROUTE:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                SLONG d = 0;
                SLONG Rang = 1;

                for (d = 0; d < 4; d++) {
                    if ((Sim.Players.Players[d].IsOut == 0) && Sim.Players.Players[d].RentRouten.RentRouten[MenuPar3].Rang >= Rang) {
                        Rang = Sim.Players.Players[d].RentRouten.RentRouten[MenuPar3].Rang + 1;
                    }
                }

                qPlayer.RentRoute(Routen[MenuPar2].VonCity, Routen[MenuPar2].NachCity, Routen[MenuPar2].Miete);

                qPlayer.RentRouten.RentRouten[MenuPar2].TageMitGering = 0;
                qPlayer.RentRouten.RentRouten[MenuPar3].TageMitGering = 0;
                qPlayer.RentRouten.RentRouten[MenuPar2].Rang = UBYTE(Rang);
                qPlayer.RentRouten.RentRouten[MenuPar3].Rang = UBYTE(Rang);

                qPlayer.Blocks.RepaintAll = TRUE;

                if (qPlayer.GetRoom() == ROOM_ROUTEBOX) {
                    if (qPlayer.LocationWin != nullptr) {
                        (dynamic_cast<CRouteBox *>(qPlayer.LocationWin))->UpdateDataTable();
                    }
                }

                qPlayer.NetUpdateRentRoute(MenuPar2, MenuPar3);
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;

        case MENU_REQUEST_KILLAUFTRAG0:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                BLOCKS &qBlocks = qPlayer.Blocks;
                for (SLONG c = 0; c < qBlocks.AnzEntries(); c++) {
                    if ((qBlocks.IsInAlbum(c) != 0) && qBlocks[c].IndexB == 0 && qBlocks[c].BlockTypeB == 3 &&
                        qPlayer.Auftraege(qBlocks[c].SelectedIdB) == ULONG(MenuPar2)) {
                        qBlocks[c].IndexB = TRUE;
                        qBlocks[c].PageB = 0;
                        qBlocks[c].RefreshData(PlayerNum);
                        qBlocks[c].AnzPagesB = max(0, (qBlocks[c].TableB.AnzRows - 1) / 6) + 1;
                    }
                }

                if ((qPlayer.LocationWin != nullptr) && (qPlayer.GetRoom() == ROOM_LAPTOP || qPlayer.GetRoom() == ROOM_GLOBE)) {
                    CPlaner &qPlaner = *dynamic_cast<CPlaner *>(qPlayer.LocationWin);

                    if (qPlaner.CurrentPostItType == 2 && qPlaner.CurrentPostItId == static_cast<SLONG>(qPlayer.Auftraege.GetIdFromIndex(MenuPar2))) {
                        qPlaner.CurrentPostItType = 0;
                    }
                }

                qPlayer.Auftraege -= MenuPar2;
                qPlayer.Blocks.RepaintAll = TRUE;
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;

        case MENU_REQUEST_KILLAUFTRAG:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                BLOCKS &qBlocks = qPlayer.Blocks;
                for (SLONG c = 0; c < qBlocks.AnzEntries(); c++) {
                    if ((qBlocks.IsInAlbum(c) != 0) && qBlocks[c].IndexB == 0 && qBlocks[c].BlockTypeB == 3 &&
                        qPlayer.Auftraege(qBlocks[c].SelectedIdB) == ULONG(MenuPar2)) {
                        qBlocks[c].IndexB = TRUE;
                        qBlocks[c].PageB = 0;
                        qBlocks[c].RefreshData(PlayerNum);
                        qBlocks[c].AnzPagesB = max(0, (qBlocks[c].TableB.AnzRows - 1) / 6) + 1;
                    }
                }

                qPlayer.ChangeMoney(-qPlayer.Auftraege[MenuPar2].Strafe, 2060,
                                    (LPCTSTR)CString(bprintf("%s-%s", (LPCTSTR)Cities[qPlayer.Auftraege[MenuPar2].VonCity].Kuerzel,
                                                             (LPCTSTR)Cities[qPlayer.Auftraege[MenuPar2].NachCity].Kuerzel)));

                if ((qPlayer.LocationWin != nullptr) && (qPlayer.GetRoom() == ROOM_LAPTOP || qPlayer.GetRoom() == ROOM_GLOBE)) {
                    CPlaner &qPlaner = *dynamic_cast<CPlaner *>(qPlayer.LocationWin);

                    if (qPlaner.CurrentPostItType == 2 && qPlaner.CurrentPostItId == static_cast<SLONG>(qPlayer.Auftraege.GetIdFromIndex(MenuPar2))) {
                        qPlaner.CurrentPostItType = 0;
                    }
                }

                qPlayer.Auftraege -= MenuPar2;
                qPlayer.Blocks.RepaintAll = TRUE;
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;

        case MENU_REQUEST_KILLFAUFTRAG0:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                BLOCKS &qBlocks = qPlayer.Blocks;
                for (SLONG c = 0; c < qBlocks.AnzEntries(); c++) {
                    if ((qBlocks.IsInAlbum(c) != 0) && qBlocks[c].IndexB == 0 && qBlocks[c].BlockTypeB == 6 &&
                        qPlayer.Frachten(qBlocks[c].SelectedIdB) == ULONG(MenuPar2)) {
                        qBlocks[c].IndexB = TRUE;
                        qBlocks[c].PageB = 0;
                        qBlocks[c].RefreshData(PlayerNum);
                        qBlocks[c].AnzPagesB = max(0, (qBlocks[c].TableB.AnzRows - 1) / 6) + 1;
                    }
                }

                if ((qPlayer.LocationWin != nullptr) && (qPlayer.GetRoom() == ROOM_LAPTOP || qPlayer.GetRoom() == ROOM_GLOBE)) {
                    CPlaner &qPlaner = *dynamic_cast<CPlaner *>(qPlayer.LocationWin);

                    if (qPlaner.CurrentPostItType == 2 && qPlaner.CurrentPostItId == static_cast<SLONG>(qPlayer.Frachten.GetIdFromIndex(MenuPar2))) {
                        qPlaner.CurrentPostItType = 0;
                    }
                }

                // qPlayer.Frachten-= MenuPar2;
                qPlayer.Frachten[MenuPar2].Okay = -1;
                qPlayer.Frachten[MenuPar2].InPlan = -1;

                qPlayer.Blocks.RepaintAll = TRUE;
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;

        case MENU_REQUEST_KILLFAUFTRAG:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                BLOCKS &qBlocks = qPlayer.Blocks;
                for (SLONG c = 0; c < qBlocks.AnzEntries(); c++) {
                    if ((qBlocks.IsInAlbum(c) != 0) && qBlocks[c].IndexB == 0 && qBlocks[c].BlockTypeB == 6 &&
                        qPlayer.Frachten(qBlocks[c].SelectedIdB) == ULONG(MenuPar2)) {
                        qBlocks[c].IndexB = TRUE;
                        qBlocks[c].PageB = 0;
                        qBlocks[c].RefreshData(PlayerNum);
                        qBlocks[c].AnzPagesB = max(0, (qBlocks[c].TableB.AnzRows - 1) / 6) + 1;
                    }
                }

                qPlayer.ChangeMoney(-qPlayer.Frachten[MenuPar2].Strafe, 2065,
                                    (LPCTSTR)CString(bprintf("%s-%s", (LPCTSTR)Cities[qPlayer.Frachten[MenuPar2].VonCity].Kuerzel,
                                                             (LPCTSTR)Cities[qPlayer.Frachten[MenuPar2].NachCity].Kuerzel)));

                if ((qPlayer.LocationWin != nullptr) && (qPlayer.GetRoom() == ROOM_LAPTOP || qPlayer.GetRoom() == ROOM_GLOBE)) {
                    CPlaner &qPlaner = *dynamic_cast<CPlaner *>(qPlayer.LocationWin);

                    if (qPlaner.CurrentPostItType == 2 && qPlaner.CurrentPostItId == static_cast<SLONG>(qPlayer.Frachten.GetIdFromIndex(MenuPar2))) {
                        qPlaner.CurrentPostItType = 0;
                    }
                }

                qPlayer.Frachten[MenuPar2].Okay = -1;
                qPlayer.Frachten[MenuPar2].InPlan = -1;

                // qPlayer.Frachten-= MenuPar2;
                qPlayer.Blocks.RepaintAll = TRUE;
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;

        case MENU_REQUEST_KILLITEM:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                if (qPlayer.Items[MenuPar2] == ITEM_LAPTOP && (qPlayer.IsLocationInQueue(ROOM_LAPTOP) != 0) && (qPlayer.IsLocationInQueue(ROOM_GLOBE) == 0)) {
                    qPlayer.LeaveRoom();
                }

                qPlayer.Items[MenuPar2] = 0xff;
                qPlayer.ReformIcons();
                if (qPlayer.HasItem(ITEM_LAPTOP) == 0) {
                    qPlayer.SecurityFlags &= ~(1 << 1);
                }
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;

        case MENU_REQUEST_KILLPLAN:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                CFlugplan &qPlan = Sim.Players.Players[Sim.localPlayer].Planes[MenuPar2].Flugplan;

                for (SLONG c = qPlan.Flug.AnzEntries() - 1; c >= 0; c--) {
                    if (qPlan.Flug[c].ObjectType != 0) {
                        if (qPlan.Flug[c].Startdate > Sim.Date || (qPlan.Flug[c].Startdate == Sim.Date && qPlan.Flug[c].Startzeit > Sim.GetHour() + 1)) {
                            qPlan.Flug[c].ObjectType = 0;
                        }
                    }
                }

                qPlan.UpdateNextFlight();
                qPlan.UpdateNextStart();

                if (Sim.bNetwork != 0) {
                    SLONG key = MenuPar2;

                    if (key < 0x1000000) {
                        key = Sim.Players.Players[Sim.localPlayer].Planes.GetIdFromIndex(key);
                    }

                    Sim.Players.Players[Sim.localPlayer].NetUpdateFlightplan(key);
                }

                Sim.Players.Players[Sim.localPlayer].UpdateAuftragsUsage();
                Sim.Players.Players[Sim.localPlayer].UpdateFrachtauftragsUsage();
                Sim.Players.Players[Sim.localPlayer].Planes[MenuPar2].CheckFlugplaene(PlayerNum, FALSE);
                Sim.Players.Players[Sim.localPlayer].Blocks.RepaintAll = TRUE;
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;

        case MENU_REQUEST_KILLPLANE:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 == 1) {
                if ((qPlayer.LocationWin != nullptr) && qPlayer.GetRoom() == ROOM_EDITOR) {
                    CEditor &qEditor = *dynamic_cast<CEditor *>(qPlayer.LocationWin);

                    if (MenuPar2 == 77) {
                        qEditor.DeleteCurrent();
                    } else {
                        qEditor.Plane.Clear();
                    }
                }
            }
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;

        default:
            if (MouseClickArea == -101 && MouseClickId == MENU_REQUEST && MouseClickPar1 > 0) {
                MenuStop();
            }
            break;
        }
        break;

    case MENU_SABOTAGEFAX:
        MenuStop();
        break;

    case MENU_LETTERS:
        if (MouseClickArea == -100 && MouseClickId == 1) {
            if (Sim.Options.OptionEffekte != 0) {
                PaperFX.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            }
            MenuPage--;
            MenuRepaint();
        } else if (MouseClickArea == -100 && MouseClickId == 11) {
            if (Sim.Options.OptionEffekte != 0) {
                PaperTearFX.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            }
            qPlayer.Letters.Letters[MenuPage].Date = -1;
            qPlayer.Letters.SortLetters();
            if (qPlayer.Letters.AnzLetters() == 0) {
                (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_IDLE);
            } else {
                if (MenuPage > qPlayer.Letters.AnzLetters() - 1) {
                    MenuPage = qPlayer.Letters.AnzLetters() - 1;
                }
                MenuRepaint();
            }
        } else if (MouseClickArea == -100 && MouseClickId == 10) {
            (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_IDLE);
            // MenuStop();
        } else if (MouseClickArea == -100 && MouseClickId == 3) {
            if (Sim.Options.OptionEffekte != 0) {
                PaperFX.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            }
            MenuPage++;
            MenuRepaint();
        }
        break;

    case MENU_ADROUTE:
        if (MouseClickArea == -102 && MouseClickId == MENU_ADROUTE && MouseClickPar1 == -3) {
            MenuStop();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_ADROUTE && MouseClickPar1 == -1) {
            MenuPrevPage();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_ADROUTE && MouseClickPar1 == -2) {
            MenuNextPage();
        } else if (Pos.IfIsWithin(216, 6, 387, 212)) {
            SLONG n = (Pos.y - 25) / 13 + MenuPage;

            if (n >= 0 && n < SLONG(MenuDataTable.AnzRows) && (Routen.IsInAlbum(MenuDataTable.LineIndex[n]) != 0) &&
                qPlayer.RentRouten.RentRouten.AnzEntries() > SLONG(Routen(MenuDataTable.LineIndex[n])) &&
                (qPlayer.RentRouten.RentRouten[Routen(MenuDataTable.LineIndex[n])].Rang != 0U)) {
                MenuPar1 = Routen(MenuDataTable.LineIndex[n]);
                MenuStop();
                StartDialog(TALKER_WERBUNG, DialogMedium, 3501, MenuPar1);
            }
        }
        break;

    case MENU_PLANEJOB:
        if (MouseClickArea == -102 && MouseClickId == MENU_PLANEJOB && MouseClickPar1 == -1) {
            MenuPrevPage();
        } else if (MouseClickArea == -102 && MouseClickId == MENU_PLANEJOB && MouseClickPar1 == -2) {
            MenuNextPage();
        } else if (Pos.IfIsWithin(216, 6, 387, 212)) {
            SLONG n = (Pos.y - 25) / 13 + MenuPage;

            SLONG page = MenuInfo2;

            if (n >= 0 && (Pos.y - 25) / 13 < 13 && n < MenuDataTable.AnzRows) {
                Workers.Workers[MenuInfo3].PlaneId = MenuDataTable.LineIndex[n];
                qPlayer.UpdatePilotCount();
                MenuStart(MENU_PERSONAL, MenuPar1, MenuPar2);
                MenuPage = min(page, MenuPageMax);
                MenuRepaint();
            }
        }
        break;

    case MENU_RENAMEPLANE:
        if (MouseClickArea == -101 && MouseClickId == MENU_RENAMEPLANE && MouseClickPar1 == 1) {
            MenuStop();
            qPlayer.Planes[MenuPar1].Name = Optionen[0];
        }
        if (MouseClickArea == -101 && MouseClickId == MENU_RENAMEPLANE && MouseClickPar1 == 2) {
            MenuStop();
        }
        break;

    case MENU_RENAMEEDITPLANE:
        if (MouseClickArea == -101 && MouseClickId == MENU_RENAMEEDITPLANE && MouseClickPar1 == 1 && qPlayer.GetRoom() == ROOM_EDITOR) {
            MenuStop();
            (dynamic_cast<CEditor *>(qPlayer.LocationWin))->Plane.Name = Optionen[0];
        }
        if (MouseClickArea == -101 && MouseClickId == MENU_RENAMEEDITPLANE && MouseClickPar1 == 2) {
            MenuStop();
        }
        break;

    case MENU_ENTERTCPIP:
        if (MouseClickArea == -101 && MouseClickId == MENU_ENTERTCPIP && MouseClickPar1 == 1) {
            MenuStop();
            gHostIP = Optionen[0];
        }
        break;

    case MENU_BROADCAST:
        if (MouseClickArea == -101 && MouseClickId == MENU_RENAMEPLANE && MouseClickPar1 == 1) {
            MenuStop();

            for (SLONG c = 0; c < 4; c++) {
                if (((MenuPar1 & (1 << c)) != 0) && Sim.Players.Players[c].Owner == 2 && (strlen(Optionen[0]) != 0U)) {
                    TEAKFILE Message;

                    SIM::SendChatBroadcast(Optionen[0], true, Sim.Players.Players[c].NetworkID);
                }
            }

            DisplayBroadcastMessage(Optionen[0], Sim.localPlayer);
        }
        if (MouseClickArea == -101 && MouseClickId == MENU_RENAMEPLANE && MouseClickPar1 == 2) {
            MenuStop();
        }

        if (MouseClickArea == -101 && MouseClickId == MENU_RENAMEPLANE && MouseClickPar1 >= 10 && MouseClickPar1 <= 13) {
            MenuPar1 ^= (1 << (MouseClickPar1 - 10));
            MenuRepaint();
        }
        break;

    case MENU_GAMEOVER:
        Sim.bNoTime = FALSE;
        Sim.DayState = 2;

        if (Sim.bNetwork != 0) {
            gNetwork.DisConnect();
            Sim.bNetwork = 0;
        }

        if (Sim.Difficulty == DIFF_FINAL && MenuPar1 == 0) {
            Sim.Gamestate = UBYTE(GAMESTATE_OUTRO);
        } else if (Sim.Difficulty == DIFF_ADDON10 && MenuPar1 == 0) {
            Sim.Gamestate = UBYTE(GAMESTATE_OUTRO2);
        } else {
            Sim.Gamestate = GAMESTATE_BOOT; // Ins Hauptmenü!
        }
        break;

    case MENU_SETRENDITE:
        if (MouseClickArea == -102 && MouseClickId == MENU_SETRENDITE && MouseClickPar1 == 11) {
            MenuStop();
        }
        if (MouseClickArea == -102 && MouseClickId == MENU_SETRENDITE && MouseClickPar1 == 10) {
            qPlayer.Dividende = MenuInfo;
            PLAYER::NetSynchronizeMoney();
            if (Sim.Options.OptionEffekte != 0) {
                PaperTearFX.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            }
            MenuStop();
        }
        if (MouseClickArea == -102 && MouseClickId == MENU_SETRENDITE && MouseClickPar1 == 20) {
            CalcOpen(MenuPos + XY(322, 61 - 46), MenuInfo);
        }
        break;

    case MENU_AKTIE:
        if (MouseClickArea == -102 && MouseClickId == MENU_SETRENDITE && MouseClickPar1 == 11) {
            if (Sim.Options.OptionEffekte != 0) {
                PaperTearFX.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            }

            if (CurrentMenu == MENU_AKTIE && (MenuDialogReEntryB == 2030 || MenuDialogReEntryB == 2130) && (CalculatorIsOpen == 0)) {
                MenuDialogReEntryB++;
            }

            MenuStop();
        }

        if (MouseClickArea == -102 && MouseClickId == MENU_SETRENDITE && MouseClickPar1 == 10) {
            if (MenuPar2 == 0) // kaufen
            {
                auto Preis = __int64(Sim.Players.Players[MenuPar1].Kurse[0] * MenuInfo);
                Preis += Preis / 10 + 100;

                if (Sim.Options.OptionAmbiente != 0) {
                    BackgroundFX[1].Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                }

                MenuStop();
                if ((Preis != 0) && qPlayer.Money - Preis < DEBT_LIMIT) {
                    MakeSayWindow(0, TOKEN_BANK, 6000, pFontPartner);
                } else {
                    if ((Sim.bNetwork != 0) && Sim.Players.Players[MenuPar1].Owner == 2) {
                        SIM::SendSimpleMessage(ATNET_ADVISOR, Sim.Players.Players[MenuPar1].NetworkID, 4, PlayerNum, MenuInfo);
                    }

                    auto preis = qPlayer.TradeStock(MenuPar1, MenuInfo);
                    if (preis != 0) {
                        if (PlayerNum == Sim.localPlayer) {
                            SIM::SendSimpleMessage(ATNET_CHANGEMONEY, 0, Sim.localPlayer, -SLONG(preis), STAT_A_SONSTIGES);
                        }
                    }

                    PLAYER::NetSynchronizeMoney();
                }
            } else if (MenuPar2 == 1) // verkaufen
            {
                MenuStop();
                auto preis = qPlayer.TradeStock(MenuPar1, -MenuInfo);
                if (preis != 0) {
                    if (PlayerNum == Sim.localPlayer) {
                        SIM::SendSimpleMessage(ATNET_CHANGEMONEY, 0, Sim.localPlayer, SLONG(preis), STAT_E_SONSTIGES);
                    }
                }

                PLAYER::NetSynchronizeMoney();
            }
        }

        if (MouseClickArea == -102 && MouseClickId == MENU_SETRENDITE && (MouseClickPar1 >= 20 && MouseClickPar1 <= 22)) {
            MenuInfo2 = MouseClickPar1 - 20;
        }

        if (MouseClickArea == -102 && MouseClickId == MENU_SETRENDITE && MouseClickPar1 == 20) {
            ColorFX.HighlightText(OnscreenBitmap.pBitmap, CRect(140, 71, 198, 85), ColorOfFontBlack, 0x20ff64);
            CalcOpen(MenuPos + XY(318, 51), MenuInfo);
        } else if (MouseClickArea == -102 && MouseClickId == MENU_SETRENDITE && MouseClickPar1 == 21) {
            ColorFX.HighlightText(OnscreenBitmap.pBitmap, CRect(200, 71, 238, 85), ColorOfFontBlack, 0x20ff64);
            CalcOpen(MenuPos + XY(318, 51), MenuInfo * 100 / Sim.Players.Players[MenuPar1].AnzAktien);
        } else if (MouseClickArea == -102 && MouseClickId == MENU_SETRENDITE && MouseClickPar1 == 22) {
            ColorFX.HighlightText(OnscreenBitmap.pBitmap, CRect(240, 71, 316, 85), ColorOfFontBlack, 0x20ff64);
            CalcOpen(MenuPos + XY(318, 51), MenuInfo * SLONG(Sim.Players.Players[MenuPar1].Kurse[0]));
        }
        /*if (MouseClickArea==-102 && MouseClickId==MENU_SETRENDITE && MouseClickPar1==10) MenuInfo = AddToNthDigit (MenuInfo, 3, 1);
          if (MouseClickArea==-102 && MouseClickId==MENU_SETRENDITE && MouseClickPar1==11) MenuInfo = AddToNthDigit (MenuInfo, 3, -1);
          if (MenuPar2==0) //kaufen
          {
          SLONG Max=Sim.Players.Players[MenuPar1].AnzAktien;
          for (SLONG c=0; c<4; c++) Max-=Sim.Players.Players[c].OwnsAktien[MenuPar1];

          Limit (SLONG(0), MenuInfo, Max);
          MenuRepaint ();
          }
          else if (MenuPar2==1) //verkaufen
          {
          Limit (SLONG(0), MenuInfo, qPlayer.OwnsAktien[MenuPar1]);
          MenuRepaint ();
          } */
        break;

    case MENU_CHAT:
        if (MouseClickArea == -101 && MouseClickId == MENU_CHAT && MouseClickPar1 == 1) {
            MenuStop();
        }

        if (MouseClickArea == -101 && MouseClickId == MENU_CHAT && MouseClickPar1 == 2) {
            SLONG c = 0;

            // Sind nur Zahlen im Text?
            for (c = strlen(Optionen[0]) - 1; c >= 0; c--) {
                if (Optionen[0].GetAt(c) < '0' || Optionen[0].GetAt(c) > '9') {
                    break;
                }
            }

            if (c == -1 && Optionen[0].GetLength() > 0 && atoll(Optionen[0]) != 0) {
                if (Sim.Players.Players[Sim.localPlayer].Money >= atoll(Optionen[0]) && atoll(Optionen[0]) > 0 && Optionen[0].GetLength() < 9) {
                    Sim.Players.Players[MenuPar1].ChangeMoney(atoi(Optionen[0]), 3700, Sim.Players.Players[Sim.localPlayer].NameX);
                    Sim.Players.Players[Sim.localPlayer].ChangeMoney(-atoi(Optionen[0]), 3701, Sim.Players.Players[MenuPar1].NameX);
                    SIM::SendSimpleMessage(ATNET_CHATMONEY, Sim.Players.Players[MenuPar1].NetworkID, atoi(Optionen[0]), PlayerNum);
                    MenuBms[1].ShiftUp(10);
                    MenuBms[1].PrintAt(bprintf(StandardTexte.GetS(TOKEN_MISC, 3009), atoi(Optionen[0])), FontNormalGrey, TEC_FONT_LEFT, 6, 119, 279, 147);
                    Optionen[0].Empty();
                    MenuRepaint();
                } else {
                    MenuBms[1].ShiftUp(10);
                    MenuBms[1].PrintAt(StandardTexte.GetS(TOKEN_MISC, 3007), FontNormalGrey, TEC_FONT_LEFT, 6, 119, 279, 147);
                    MenuRepaint();
                }
            } else {
                MenuBms[1].ShiftUp(10);
                MenuBms[1].PrintAt(StandardTexte.GetS(TOKEN_MISC, 3008), FontNormalGrey, TEC_FONT_LEFT, 6, 119, 279, 147);
                MenuRepaint();
            }
        }
        break;

    case MENU_BRIEFING:
        if (MouseClickArea == -101 && MouseClickId == MENU_BRIEFING && MouseClickPar1 == 1) {
            MenuStop();
            StartDialog(TALKER_BOSS, MEDIUM_AIR, 1);
            DontDisplayPlayer = Sim.localPlayer;
        }
        if (MouseClickArea == -101 && MouseClickId == MENU_BRIEFING && MouseClickPar1 == 2 && MenuPar1 == 0) {
            Sim.bNoTime = FALSE;
            Sim.DayState = 2;

            (dynamic_cast<CAufsicht *>(this))->TryLeaveAufsicht();
        }
        break;
    default:
        break;
    }
}

//--------------------------------------------------------------------------------------------
// Reaktion auf rechten Mausklick im Menü:
//--------------------------------------------------------------------------------------------
void CStdRaum::MenuRightClick(XY /*unused*/) {
    if (CurrentMenu == MENU_WC_M || CurrentMenu == MENU_WC_F) {
        return;
    }

    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    if (CurrentMenu == MENU_AKTIE && (MenuDialogReEntryB == 2030 || MenuDialogReEntryB == 2130) && (CalculatorIsOpen == 0)) {
        MenuDialogReEntryB++;
    }

    // Fraglich wozu das gut war; Führte aber zu problemen mit dem Sabotage-Dialog
    if ((CurrentMenu == MENU_SABOTAGEPLANE || CurrentMenu == MENU_SABOTAGEROUTE) && MenuDialogReEntryB != 1090 && MenuDialogReEntryB != 1091) {
        MenuDialogReEntryB++;
    }

    if (CurrentMenu == MENU_BUYXPLANE) {
        MenuDialogReEntryB = -1;
    } else if (CurrentMenu == MENU_AUSLANDSAUFTRAG && qPlayer.CalledCities[MenuPar1] == 0) {
        MenuDialogReEntryB = 5000;
    }
    if (CurrentMenu == MENU_AUSLANDSAUFTRAG && (qPlayer.CalledCities[MenuPar1] != 0U)) {
        if (IsDialogOpen() != 0) {
            StopDialog();
        }
        MenuStart(MENU_FILOFAX, 2);
        CurrentMenu = MENU_BRANCHLIST;
        MenuPage = 0;
        MenuRepaint();
        return;
    }

    if (CurrentMenu == MENU_LETTERS) {
        (dynamic_cast<CBuero *>(this))->SP_Player.SetDesiredMood(SPM_IDLE);
        return;
    }
    if (CurrentMenu == MENU_REQUEST && MenuPar1 == MENU_REQUEST_CALLITADAY) {
        if (qPlayer.GetRoom() == ROOM_AIRPORT && MenuPar2 == 1) {
            qPlayer.NewDir = 0;

            XY p = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Position;
            XY ArrayPos;

            ArrayPos.x = (p.x + 4400) / 44 - 100;
            ArrayPos.y = (p.y + 2200) / 22 - 100 + 5 - 2;

            qPlayer.WalkToPlate(ArrayPos);
        }
    }

    if (CurrentMenu == MENU_PLAYERLIST || CurrentMenu == MENU_BRANCHLIST) {
        CurrentMenu = MENU_FILOFAX;
        MenuPage = 0;
        MenuPar1 = 2; // Handy
        MenuRepaint();
        if (Sim.Options.OptionEffekte != 0) {
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
        }
        return;
    }
    if (CurrentMenu == MENU_PLANEJOB) {
        SLONG page = MenuInfo2;

        MenuStart(MENU_PERSONAL, MenuPar1, MenuPar2);

        MenuPage = min(page, MenuPageMax);
        MenuRepaint();
        return;
    }

    if (CurrentMenu == MENU_REQUEST && (MenuPar1 == MENU_REQUEST_THROWNOUT || MenuPar1 == MENU_REQUEST_THROWNOUT2)) {
        qPlayer.WaitForRoom = 0;
        qPlayer.ThrownOutOfRoom = 0;
    }

    if (CurrentMenu == MENU_GAMEOVER) {
        if (Sim.bNetwork != 0) {
            gNetwork.DisConnect();
            Sim.bNetwork = 0;
        }

        Sim.bNoTime = FALSE;
        Sim.DayState = 2;
        if (Sim.Difficulty == DIFF_FINAL && MenuPar1 == 0) {
            Sim.Gamestate = UBYTE(GAMESTATE_OUTRO);
        } else {
            Sim.Gamestate = GAMESTATE_BOOT; // Ins Hauptmenü!
        }

        return;
    }

    if (CalculatorIsOpen != 0) {
        CalcStop(TRUE);
    } else {
        MenuStop();
    }
}

//--------------------------------------------------------------------------------------------
// Beendet das Menü:
//--------------------------------------------------------------------------------------------
void CStdRaum::MenuStop() {
    if (PlayerNum >= Sim.Players.Players.AnzEntries()) {
        return;
    }

    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    LastMenu = CurrentMenu;

    if (CurrentMenu == MENU_CLOSED || CurrentMenu == MENU_REQUEST) {
        IgnoreNextLButtonUp = TRUE;
    }

    if (CurrentMenu == MENU_QUITMESSAGE) {
        SDL_Quit();
        Sim.Gamestate = GAMESTATE_QUIT;
        bLeaveGameLoop = TRUE;
        return;
    }

    if (CurrentMenu == MENU_CALLITADAY && Sim.CallItADay == FALSE) {
        if (Sim.CallItADayAt == 0) {
            SIM::SendSimpleMessage(ATNET_DAYBACK, 0, PlayerNum);
            SIM::SendChatBroadcast(bprintf(StandardTexte.GetS(TOKEN_MISC, 7021), (LPCTSTR)Sim.Players.Players[Sim.localPlayer].NameX));
            qPlayer.CallItADay = FALSE;
        }
    }

    if (CurrentMenu == MENU_CLOSED && MenuPar1 == MENU_CLOSED_PROTECTION) {
        exit(-1);
    }

    if (CurrentMenu == MENU_LETTERS || (CurrentMenu == MENU_FILOFAX && MenuPar1 != 1 && qPlayer.GetRoom() == ROOM_BURO_A + PlayerNum * 10)) {
        auto *buero = dynamic_cast<CBuero *>(this);
        if (buero) {
            buero->SP_Player.SetDesiredMood(SPM_IDLE);
        }
    }

    CurrentMenu = MENU_NONE;
    CurrentHighlight = 0;

    if (LastMenu == MENU_CHAT) {
        SIM::SendSimpleMessage(ATNET_CHATSTOP, Sim.Players.Players[DialogPar1].NetworkID);
    }

    // Keine Kommentare mehr:
    qPlayer.Messages.AddMessage(BERATERTYP_AUFTRAG, "", MESSAGE_COMMENT);
    qPlayer.Messages.AddMessage(BERATERTYP_PERSONAL, "", MESSAGE_COMMENT);
    qPlayer.Messages.NoComments();

    MenuBms.Destroy();
    MenuBms2.ReSize(0);

    if ((pMenuLib1 != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pMenuLib1);
        pMenuLib1 = nullptr;
    }
    if ((pMenuLib2 != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pMenuLib2);
        pMenuLib2 = nullptr;
    }

    if (MenuDialogReEntryB != -1 && (qPlayer.CallItADay == 0) && Sim.GetHour() < 18) {
        for (SLONG c = 0; c < qPlayer.Messages.Messages.AnzEntries(); c++) {
            if (qPlayer.Messages.Messages[c].Urgent == MESSAGE_COMMENT) {
                qPlayer.Messages.Messages[c].BeraterTyp = -1;
            }
        }

        // Dialog geht weiter, der Partner sagt etwas:
        MakeSayWindow(0, reinterpret_cast<char *>(&MenuDialogReEntryA), MenuDialogReEntryB, pFontPartner);

        /*if (LastMenu==MENU_AUSLANDSAUFTRAG)
          qPlayer.Messages.StartDialog (PlayerNum, MEDIUM_HANDY);
          else*/
        qPlayer.Messages.StartDialog(PlayerNum);
        qPlayer.Messages.BeraterWalkState = 0; //! new

        if (LastMenu == MENU_AUSLANDSAUFTRAG) {
            DialogMedium = MEDIUM_HANDY;
            // qPlayer.Messages.BeraterWalkState=0;
        }

        if (qPlayer.Messages.BeraterPosY > 420) {
            qPlayer.Messages.BeraterWalkState = -1; //! new
        }
    } else if ((qPlayer.CallItADay == 0) && Sim.GetHour() < 18) {
        if (DialogPartner != TALKER_NONE && (pSmackerPartner != nullptr)) {
            DialogPartner = TALKER_NONE;
            pSmackerPartner->SetDesiredMood(SPM_IDLE);
            qPlayer.Messages.StopDialog();
        }
    }

    MenuDialogReEntryB = -1;
}

//--------------------------------------------------------------------------------------------
// Öffnet den Taschenrechner:
//--------------------------------------------------------------------------------------------
void CStdRaum::CalcOpen(XY Position, SLONG Value) {
    CalculatorIsOpen = TRUE;
    CalculatorPos = Position;
    CalculatorValue = Value;
    CalculatorValue = 0;

    CalculatorFX.ReInit("calc.raw");

    CalculatorKeyTimer = AtGetTime() - 1000;

    pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("rechner.gli", GliPath)), &pCalculatorLib, L_LOCMEM);

    CalculatorBms.ReSize(pCalculatorLib, "RECHNER D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 T0 T1 T2 T3 T4 T5 T6 T7 T8 T9 TASTE_OK TASTE_C TASTE_X DOT");
}

//--------------------------------------------------------------------------------------------
// Blittet den Rechner
//--------------------------------------------------------------------------------------------
void CStdRaum::CalcRepaint() {
    if (CalculatorIsOpen != 0) {
        SLONG c = 0;
        SLONG n = 0;

        PrimaryBm.BlitFromT(CalculatorBms[0], CalculatorPos);

        n = CalculatorValue;

        for (c = 0; c < 8 && n > 0; c++) {
            PrimaryBm.BlitFrom(CalculatorBms[1 + n % 10], CalculatorPos + CalcOffsets[1 + n % 10] - XY(c * 10, 0));
            n /= 10;

            if (c == 3 || c == 6) {
                PrimaryBm.BlitFrom(CalculatorBms[24], CalculatorPos + CalcOffsets[1 + n % 10] - XY(c * 10 - 9, -14));
            }
        }

        if (AtGetTime() - CalculatorKeyTimer < 300) {
            PrimaryBm.BlitFrom(CalculatorBms[CalculatorKey], CalculatorPos + CalcOffsets[CalculatorKey]);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Calculator wird beendet
//--------------------------------------------------------------------------------------------
void CStdRaum::CalcStop(BOOL Cancel) {
    CalculatorIsOpen = FALSE;
    CalculatorBms.Destroy();
    if ((pCalculatorLib != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pCalculatorLib);
        pCalculatorLib = nullptr;
    }

    CalculatorFX.Destroy();

    if (Cancel == 0) {
        if (CurrentMenu == MENU_AKTIE) {
            if (MouseClickArea == -102 && MouseClickId == MENU_SETRENDITE && (MouseClickPar1 >= 20 && MouseClickPar1 <= 22)) {
                MenuInfo2 = MouseClickPar1 - 20;
            }

            if (MenuInfo2 == 0) {
                MenuInfo = CalculatorValue;
            }
            if (MenuInfo2 == 1) {
                if (CalculatorValue > 100) {
                    CalculatorValue = 100;
                }
                MenuInfo = SLONG(__int64(CalculatorValue) * Sim.Players.Players[MenuPar1].AnzAktien / 100);
            }
            if (MenuInfo2 == 2) {
                MenuInfo = CalculatorValue / SLONG(Sim.Players.Players[MenuPar1].Kurse[0]);
            }

            if (MenuPar2 == 0) // kaufen
            {
                SLONG Max = Sim.Players.Players[MenuPar1].AnzAktien;
                for (SLONG c = 0; c < 4; c++) {
                    if (Sim.Players.Players[c].IsOut == 0) {
                        Max -= Sim.Players.Players[c].OwnsAktien[MenuPar1];
                    }
                }

                Limit(SLONG(0), MenuInfo, Max);
                MenuRepaint();
            } else if (MenuPar2 == 1) // verkaufen
            {
                Limit(SLONG(0), MenuInfo, Sim.Players.Players[PlayerNum].OwnsAktien[MenuPar1]);
                MenuRepaint();
            }
        } else if (CurrentMenu == MENU_BUYKEROSIN) {
            if (MenuInfo2 == 0) {
                MenuPar2 = CalculatorValue;
            }

            if (MenuInfo2 == 1) {
                if (CalculatorValue > 100) {
                    CalculatorValue = 100;
                }
                MenuPar2 = CalculatorValue * Sim.Players.Players[PlayerNum].Tank / 100;
            }
            if (MenuInfo2 == 2) {
                SLONG Kerosinpreis = Sim.HoleKerosinPreis(MenuPar1);
                MenuPar2 = CalculatorValue / Kerosinpreis;
            }

            if (MenuPar2 + Sim.Players.Players[PlayerNum].TankInhalt > Sim.Players.Players[PlayerNum].Tank) {
                MenuPar2 = Sim.Players.Players[PlayerNum].Tank - Sim.Players.Players[PlayerNum].TankInhalt;
            }

            if (MenuPar2 < 0) {
                MenuPar2 = 0;
            }

            MenuRepaint();
        } else if (CurrentMenu == MENU_SETRENDITE) {
            MenuInfo = CalculatorValue;

            Limit(SLONG(0), MenuInfo, SLONG(25));
            MenuRepaint();
        } else if (CurrentMenu == MENU_BANK) {
            MenuPar1 = CalculatorValue;

            if (MenuPar2 == 1) {
                Limit(static_cast<SLONG>(0), MenuPar1, static_cast<SLONG>(Sim.Players.Players[PlayerNum].CalcCreditLimit()));
            } else { // Kredit zurückzahlen
                Limit(static_cast<SLONG>(0), MenuPar1, static_cast<SLONG>(Sim.Players.Players[PlayerNum].Credit));
            }

            MenuRepaint();
        } else if (CurrentMenu == MENU_PLANEREPAIRS) {
            Limit(SLONG(0), CalculatorValue, SLONG(100));
            Sim.Players.Players[PlayerNum].Planes[MenuInfo].TargetZustand = UBYTE(CalculatorValue);
            Sim.Players.Players[PlayerNum].NetUpdatePlaneProps(MenuInfo);

            MenuInfo3 = CalculatorValue;

            MenuRepaint();
        }
    } else {
        MenuRepaint();
    }
}

//--------------------------------------------------------------------------------------------
// Ein Mausklick auf den Calculator
//--------------------------------------------------------------------------------------------
void CStdRaum::CalcClick() {
    SLONG c = 0;

    XY Pos = gMousePosition - CalculatorPos;

    for (c = 11; c < CalculatorBms.AnzEntries(); c++) {
        if (Pos.IfIsWithin(CalcOffsets[c].x + 4, CalcOffsets[c].y + 4, CalcOffsets[c].x + CalculatorBms[c].Size.x - 4,
                           CalcOffsets[c].y + CalculatorBms[c].Size.y - 4)) {
            CalculatorFX.Play(0, Sim.Options.OptionEffekte * 100 / 7);

            CalculatorKeyTimer = AtGetTime();
            CalculatorKey = c;

            if (c >= 11 && c < 21 && CalculatorValue < 10000000) {
                CalculatorValue = CalculatorValue * 10 + c - 11;
            } else if (c == 21) // OK
            {
                CalcStop(FALSE);
            } else if (c == 22) // C
            {
                CalculatorValue = 0;
            } else if (c == 23) // X
            {
                CalcStop(TRUE);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Ein Mausklick auf den Calculator
//--------------------------------------------------------------------------------------------
void CStdRaum::CalcKey(SLONG Key) {
    if (Key >= '0' && Key <= '9') {
        CalculatorKeyTimer = AtGetTime();
        CalculatorKey = 11 + Key - '0';

        CalculatorFX.Play(0, Sim.Options.OptionEffekte * 100 / 7);

        if (CalculatorValue < 10000000) {
            CalculatorValue = CalculatorValue * 10 + Key - '0';
        }
    } else if (Key == ATKEY_RETURN || Key == ATKEY_RETURN2) {
        CalculatorFX.Play(0, Sim.Options.OptionEffekte * 100 / 7);

        CalculatorKeyTimer = AtGetTime();
        CalculatorKey = 21;

        CalcStop(FALSE);
    } else if (Key == ATKEY_DELETE || Key == ATKEY_BACK) {
        CalculatorFX.Play(0, Sim.Options.OptionEffekte * 100 / 7);

        CalculatorKeyTimer = AtGetTime();
        CalculatorKey = 22;

        CalculatorValue = 0;
    } else if (Key == ATKEY_ESCAPE) {
        CalculatorFX.Play(0, Sim.Options.OptionEffekte * 100 / 7);

        CalculatorKeyTimer = AtGetTime();
        CalculatorKey = 23;

        CalcStop(TRUE);
    }
}

//--------------------------------------------------------------------------------------------
// Übergibt die Daten für das Zoommenü:
//--------------------------------------------------------------------------------------------
void CStdRaum::MenuSetZoomStuff(const XY &MenuStartPos, DOUBLE MinimumZoom, BOOL ZoomFromAirport, SLONG ZoomSpeed) {
    CStdRaum::ZoomCounter = 0;
    CStdRaum::MenuStartPos = MenuStartPos;
    CStdRaum::MinimumZoom = MinimumZoom;
    CStdRaum::ZoomFromAirport = ZoomFromAirport;
    CStdRaum::ZoomSpeed = ZoomSpeed;
}

//--------------------------------------------------------------------------------------------
// NewGamePopup::OnChar
//--------------------------------------------------------------------------------------------
void CStdRaum::OnChar(UINT nChar, UINT /*unused*/, UINT /*unused*/) {
    if (CalculatorIsOpen != 0) {
        CalcKey(nChar);
    } else if ((MenuIsOpen() != 0) && (CurrentMenu == MENU_RENAMEPLANE || CurrentMenu == MENU_RENAMEEDITPLANE || CurrentMenu == MENU_ENTERTCPIP ||
                                       CurrentMenu == MENU_BROADCAST || CurrentMenu == MENU_CHAT)) {
        if (nChar == ' ' || nChar == ':' || nChar == ',' || nChar == ';' || nChar == '!' || nChar == '?' || nChar == '\'' || nChar == '-' || nChar == '+' ||
            nChar == '(' || nChar == ')' || (nChar >= '0' && nChar <= '9') || (nChar >= 'A' && nChar <= 'Z') || (nChar >= 'a' && nChar <= 'z') ||
            nChar == UBYTE('\xC4') || nChar == UBYTE('\xD6') || nChar == UBYTE('\xDC') || nChar == UBYTE('\xE4') || nChar == UBYTE('\xF6') ||
            nChar == UBYTE('\xFC') || nChar == '.' || nChar == UBYTE('\xDF')) {
            if (Optionen[0].GetLength() < 20 || CurrentMenu == MENU_BROADCAST || CurrentMenu == MENU_CHAT) {
                Optionen[0] += static_cast<unsigned char>(nChar);
                MenuRepaint();
            }
        }
    } else if ((MenuIsOpen() != 0) && CurrentMenu == MENU_PLANEREPAIRS && (nChar == '+' || nChar == '-')) {
        if (MouseClickArea == -102 && MouseClickId == MENU_PLANEREPAIRS && MouseClickPar1 >= 0) {
            SLONG n = MouseClickPar1;
            if (n >= 0 && n < MenuDataTable.LineIndex.AnzEntries() && n - MenuPage < 13 && n >= MenuPage) {
                UBYTE &qZustand = Sim.Players.Players[PlayerNum].Planes[MenuDataTable.LineIndex[n]].TargetZustand;

                if (nChar == '+') {
                    if (qZustand <= 95) {
                        qZustand += 5;
                    } else {
                        qZustand = 100;
                    }
                }
                if (nChar == '-') {
                    if (qZustand >= 5) {
                        qZustand -= 5;
                    } else {
                        qZustand = 0;
                    }
                }

                Sim.Players.Players[PlayerNum].NetUpdatePlaneProps(MenuDataTable.LineIndex[n]);

                MenuRepaint();
            }
        }
    } else if (CurrentMenu == MENU_PERSONAL) {
        bool change = false;
        if (nChar == '+') {
            Workers.Workers[MenuRemapper[MenuPage - 1]].Gehaltsaenderung(1);
            change = true;
        } else if (nChar == '-') {
            Workers.Workers[MenuRemapper[MenuPage - 1]].Gehaltsaenderung(0);
            change = true;
        }
        if (change) {
            PersonalPageFlipped();
        }
    }
}

//--------------------------------------------------------------------------------------------
// Hotkeys:
//--------------------------------------------------------------------------------------------
void CStdRaum::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/) {
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    if ((CalculatorIsOpen != 0) && (nChar == ATKEY_DELETE || nChar == ATKEY_RETURN || nChar == ATKEY_RETURN2 || nChar == ATKEY_BACK || nChar == ATKEY_ESCAPE)) {
        CalcKey(nChar);
        return;
    }
    if (nChar == ATKEY_F2 && Editor == 0) {
        if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) && (MouseWait == 0) && (Sim.Time > 9 * 60000)) {
            if (Sim.Players.Players[PlayerNum].IsLocationInQueue(ROOM_OPTIONS) == 0) {
                Sim.Players.Players[PlayerNum].EnterRoom(ROOM_OPTIONS);
            }
        }
        return;
    }

    bool change = false;
    switch (CurrentMenu) {
    case MENU_RENAMEPLANE:
    case MENU_RENAMEEDITPLANE:
    case MENU_ENTERTCPIP:
    case MENU_BROADCAST:
    case MENU_CHAT:
        switch (nChar) {
        case ATKEY_BACK:
            if (Optionen[0].GetLength() > 0) {
                Optionen[0] = Optionen[0].Left(Optionen[0].GetLength() - 1);
                MenuRepaint();
            }
            break;

        case ATKEY_RETURN:
        case ATKEY_RETURN2:
            if (CurrentMenu == MENU_ENTERTCPIP) {
                gHostIP = Optionen[0];
                MenuStop();
            } else if (Optionen[0].GetLength() > 0) {

                if (CurrentMenu == MENU_CHAT) {
                    TEAKFILE Message;

                    MenuBms[1].ShiftUp(10);
                    MenuBms[1].PrintAt(Optionen[0], FontSmallBlack, TEC_FONT_LEFT, 6, 119, 279, 147);

                    Message.Announce(30);
                    Message << ATNET_CHATMESSAGE << Optionen[0];
                    SIM::SendMemFile(Message, Sim.Players.Players[MenuPar1].NetworkID);

                    Optionen[0].Empty();
                    MenuRepaint();
                } else if (CurrentMenu == MENU_BROADCAST) {
                    for (SLONG c = 0; c < 4; c++) {
                        if (((MenuPar1 & (1 << c)) != 0) && Sim.Players.Players[c].Owner == 2 && (strlen(Optionen[0]) != 0U)) {
                            SIM::SendChatBroadcast(Optionen[0], true, Sim.Players.Players[c].NetworkID);
                        }
                    }

                    DisplayBroadcastMessage(Optionen[0], Sim.localPlayer);
                    MenuStop();
                } else if (CurrentMenu == MENU_RENAMEPLANE) {
                    qPlayer.Planes[MenuPar1].Name = Optionen[0];
                    MenuStop();
                } else if (CurrentMenu == MENU_RENAMEEDITPLANE && qPlayer.GetRoom() == ROOM_EDITOR) {
                    MenuStop();
                    (dynamic_cast<CEditor *>(qPlayer.LocationWin))->Plane.Name = Optionen[0];
                }
            }
        default:
            break;
        }
        break;

    case MENU_PLANECOSTS:
    case MENU_PLANEREPAIRS:
    case MENU_SABOTAGEPLANE:
    case MENU_SABOTAGEROUTE:
    case MENU_SELLPLANE:
    case MENU_BUYPLANE:
    case MENU_BUYXPLANE:
    case MENU_ADROUTE:
    case MENU_PLANEJOB:
        switch (nChar) {
        case ATKEY_LEFT:
            MenuPrevPage();
            break;
        case ATKEY_RIGHT:
            MenuNextPage();
            break;
        default:
            break;
        }
        break;

    case MENU_PERSONAL: {
        switch (nChar) {
        case ATKEY_LEFT:
            if (AtGetAsyncKeyState(ATKEY_CONTROL) / 256 != 0) {
                MenuPage = std::max(0, MenuPage - 100);
            } else if (AtGetAsyncKeyState(ATKEY_SHIFT) / 256 != 0) {
                MenuPage = std::max(0, MenuPage - 10);
            } else {
                MenuPage = std::max(0, MenuPage - 1);
            }
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            change = true;
            break;
        case ATKEY_RIGHT:
            if (AtGetAsyncKeyState(ATKEY_CONTROL) / 256 != 0) {
                MenuPage = std::min(MenuPageMax, MenuPage + 100);
            } else if (AtGetAsyncKeyState(ATKEY_SHIFT) / 256 != 0) {
                MenuPage = std::min(MenuPageMax, MenuPage + 10);
            } else {
                MenuPage = std::min(MenuPageMax, MenuPage + 1);
            }
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
            change = true;
            break;
        case ATKEY_RETURN:
            Workers.Workers[MenuRemapper[MenuPage - 1]].Employer = PlayerNum;
            Workers.Workers[MenuRemapper[MenuPage - 1]].PlaneId = -1;
            qPlayer.MapWorkers(TRUE);
            qPlayer.UpdateWalkSpeed();
            change = true;
            break;
        case ATKEY_BACK:
            Workers.Workers[MenuRemapper[MenuPage - 1]].Employer = WORKER_JOBLESS;
            if (Workers.Workers[MenuRemapper[MenuPage - 1]].TimeInPool > 0) {
                Workers.Workers[MenuRemapper[MenuPage - 1]].TimeInPool = 0;
            }
            qPlayer.MapWorkers(TRUE);
            qPlayer.UpdateWalkSpeed();
            change = true;
            break;
        default:
            break;
        }
        if (change) {
            PersonalPageFlipped();
        }
        break;
    }
    default:
        break;
    }
}

//--------------------------------------------------------------------------------------------
// BOOL CStdRaum::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) : AG:
//--------------------------------------------------------------------------------------------
BOOL CStdRaum::OnSetCursor(void *pWnd, UINT nHitTest, UINT message) { return (FrameWnd->OnSetCursor(pWnd, nHitTest, message)); }

//--------------------------------------------------------------------------------------------
// void CStdRaum::OnMouseMove(UINT nFlags, CPoint point): AG:
//--------------------------------------------------------------------------------------------
void CStdRaum::OnMouseMove(UINT nFlags, CPoint point) { FrameWnd->OnMouseMove(nFlags, point); }

//--------------------------------------------------------------------------------------------
// Der Raum der als nächstes geladen wird, übernimmt die Roomlib; sie wird nicht vernichtet!
//--------------------------------------------------------------------------------------------
void CStdRaum::KeepRoomLib() { pRoomLibStatic = pRoomLib; }

void CStdRaum::OnTimer(UINT /*nIDEvent*/) {}

Uint32 CStdRaum::TimerFunc(Uint32 interval, void *param) {
    (static_cast<CStdRaum *>(param))->OnTimer(1);
    return interval;
}

void CStdRaum::MenuPrevPage() {
    if (MenuPage <= 0) {
        return;
    }

    gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);

    if (AtGetAsyncKeyState(ATKEY_CONTROL) / 256 != 0) {
        MenuPage = std::max(0, MenuPage - 13 * 10);
    } else if (AtGetAsyncKeyState(ATKEY_SHIFT) / 256 != 0) {
        MenuPage = std::max(0, MenuPage - 13 * 3);
    } else {
        MenuPage = std::max(0, MenuPage - 13);
    }

    MenuRepaint();
}

void CStdRaum::MenuNextPage() {
    if (MenuPage >= MenuDataTable.AnzRows - 13) {
        return;
    }

    gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);

    auto MenuPageMax = MenuDataTable.AnzRows - 13;
    if (AtGetAsyncKeyState(ATKEY_CONTROL) / 256 != 0) {
        MenuPage = std::min(MenuPageMax, MenuPage + 13 * 10);
    } else if (AtGetAsyncKeyState(ATKEY_SHIFT) / 256 != 0) {
        MenuPage = std::min(MenuPageMax, MenuPage + 13 * 3);
    } else {
        MenuPage = std::min(MenuPageMax, MenuPage + 13);
    }

    MenuRepaint();
}

void CStdRaum::PersonalPageFlipped() {
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];
    if (MenuPar2 == 1 && (qPlayer.HasBerater(BERATERTYP_PERSONAL) != 0) && MenuPage > 0) {
        auto &qWorker = Workers.Workers[MenuRemapper[MenuPage - 1]];
        if (qWorker.Employer == WORKER_RESERVE) {
            // rausgeekelt:
            qPlayer.Messages.AddMessage(BERATERTYP_PERSONAL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 1004 + qWorker.Geschlecht), qWorker.Name.c_str()),
                                        MESSAGE_COMMENT);
        } else if (qWorker.Employer == WORKER_JOBLESS) {
            if (qWorker.Typ <= BERATERTYP_SICHERHEIT && (qPlayer.HasBerater(qWorker.Typ) != 0)) {
                if (qWorker.Typ == BERATERTYP_PERSONAL) {
                    qPlayer.Messages.AddMessage(BERATERTYP_PERSONAL, StandardTexte.GetS(TOKEN_ADVICE, 1272), MESSAGE_COMMENT);
                } else if (qWorker.Talent <= qPlayer.HasBerater(qWorker.Typ)) {
                    qPlayer.Messages.AddMessage(BERATERTYP_PERSONAL, StandardTexte.GetS(TOKEN_ADVICE, 1270), MESSAGE_COMMENT, SMILEY_BAD);
                } else {
                    qPlayer.Messages.AddMessage(BERATERTYP_PERSONAL, StandardTexte.GetS(TOKEN_ADVICE, 1271), MESSAGE_COMMENT, SMILEY_GOOD);
                }
            } else if ((MenuRemapper[MenuPage - 1] * 7777 % 100) > qPlayer.HasBerater(BERATERTYP_PERSONAL)) {
                qPlayer.Messages.AddMessage(BERATERTYP_PERSONAL, StandardTexte.GetS(TOKEN_ADVICE, 1000), MESSAGE_COMMENT, SMILEY_NEUTRAL);
            } else {
                SLONG n = Workers.GetQualityRatio(MenuRemapper[MenuPage - 1]);
                SLONG Smiley = 0;

                if (qWorker.Talent <= 25) {
                    n = 1006;
                    Smiley = SMILEY_BAD;
                } else if (qWorker.Talent <= 45) {
                    n = 1007;
                    Smiley = SMILEY_BAD;
                } else if (n < -5) {
                    n = 1001;
                    Smiley = SMILEY_BAD;
                } else if (n > 5) {
                    if (qWorker.Talent >= 90) {
                        n = 1008;
                        Smiley = SMILEY_GREAT;
                    } else {
                        n = 1003;
                        Smiley = SMILEY_GOOD;
                    }

                } else {
                    n = 1002;
                    Smiley = SMILEY_NEUTRAL;
                }

                qPlayer.Messages.AddMessage(BERATERTYP_PERSONAL, StandardTexte.GetS(TOKEN_ADVICE, n), MESSAGE_COMMENT, Smiley);
            }
        } else {
            // kein text:
            qPlayer.Messages.AddMessage(BERATERTYP_PERSONAL, "", MESSAGE_COMMENT);
        }
    }

    if (Sim.bNetwork != 0) {
        qPlayer.NetUpdateWorkers();
    }
    MenuRepaint();
}
