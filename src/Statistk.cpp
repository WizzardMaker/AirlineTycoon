//============================================================================================
// Statistk.cpp : Der Statistik-Screen
//============================================================================================
// Link: "Statistk.h"
//============================================================================================
#include "StdAfx.h"
#include "Statistk.h"
#include "glstat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD const ClickHighlightTime = 300; // 300 ms bleibt ein Button nach einem Click ganz hell

extern SB_CColorFX ColorFX;

// Die Farben der Fluglinien; definiert in Bank.cpp; Aber bitte erst mittels GetHardwarecolor()
// in die grafikkartenspezifische Farbe umrechnen...
extern ULONG AktienKursLineColor[4];
ULONG DarkColors[4] = {0x00407f, 0x007f20, 0x7f0000, 0x7f7f00};

// Das Ausgabefenster für den Graphen
#define G_LEFT (6)
#define G_TOP (30)
#define G_RIGHT (439)
#define G_BOTTOM (329)
#define G_HEIGHT (G_BOTTOM - G_TOP)
#define G_WIDTH (G_RIGHT - G_LEFT)

SLONG days[] = {7, 14, 21, 30, 90, 180, 270, 360};

const char TOKEN_STAT[] = "STAT";

//--------------------------------------------------------------------------------------------
// Die Screen wird eröffnet:
//--------------------------------------------------------------------------------------------
CStatistik::CStatistik(BOOL bHandy, ULONG PlayerNum) : CStdRaum(bHandy, PlayerNum, "statistk.gli", GFX_STATISTK) {
    SLONG c = 0;

    DropDownPos = XY(0, 0);
    DropDownSpeed = 0;

    _playerMask = Sim.StatplayerMask;
    _group = Sim.Statgroup;
    _fGraphVisible = Sim.StatfGraphVisible;
    DropDownPos.y = Sim.DropDownPosY;

    // Umständlich, aber wahr...
    {
        for (short p = 0; p < STAT_MAX_GROUPS; p++) {
            for (short i = 0; i < STAT_MAX_ITEMS; i++) {
                _iArray[p][i].visible = Sim.StatiArray[p][i];
                _iArray[p][i].typOfItem = TYP_LINEFEED;
            }
        }

        // Erste Gruppe
        short c = 0;
        short g = 0;
        _iArray[g][c].typOfItem = TYP_GROUP;
        _iArray[g][c].textId = 8002;
        c++;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8003;
        _iArray[g][c].beraterSkillInfo = 1;
        _iArray[g][c++].define = STAT_E_ROUTEN;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8004;
        _iArray[g][c].beraterSkillInfo = 1;
        _iArray[g][c++].define = STAT_E_AUFTRAEGE;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10020;
        _iArray[g][c].beraterSkillInfo = 1;
        _iArray[g][c++].define = STAT_E_FRACHT;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10001;
        _iArray[g][c].beraterSkill = 40;
        _iArray[g][c++].define = STAT_E_ZINS;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10002;
        _iArray[g][c].beraterSkill = 40;
        _iArray[g][c++].define = STAT_E_KREDIT;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10004;
        _iArray[g][c].beraterSkill = 50;
        _iArray[g][c++].define = STAT_E_AKTIEN;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10003;
        _iArray[g][c].beraterSkill = 60;
        _iArray[g][c++].define = STAT_E_FLUGZEUGE;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10000;
        _iArray[g][c].beraterSkill = 70;
        _iArray[g][c++].define = STAT_E_RABATT;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8006;
        _iArray[g][c].beraterSkill = 70;
        _iArray[g][c++].define = STAT_E_SONSTIGES;

        _iArray[g][c].typOfItem = TYP_SUM_CURR;
        _iArray[g][c].textId = 8014;
        _iArray[g][c].beraterSkill = 0;
        _iArray[g][c++].define = 0;
        c++;
        c++;

        _iArray[g][c].typOfItem = TYP_GROUP;
        _iArray[g][c].textId = 8007;
        c++;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8010;
        _iArray[g][c].beraterSkill = 1;
        _iArray[g][c++].define = STAT_A_KEROSIN;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8011;
        _iArray[g][c].beraterSkill = 1;
        _iArray[g][c++].define = STAT_A_WARTUNG;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8012;
        _iArray[g][c].beraterSkill = 1;
        _iArray[g][c++].define = STAT_A_STRAFEN;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10013;
        _iArray[g][c].beraterSkill = 1;
        _iArray[g][c++].define = STAT_A_ESSEN;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8008;
        _iArray[g][c].beraterSkill = 30;
        _iArray[g][c++].define = STAT_A_MIETEN;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8009;
        _iArray[g][c].beraterSkill = 30;
        _iArray[g][c++].define = STAT_A_GEHAELTER;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10011;
        _iArray[g][c].beraterSkill = 40;
        _iArray[g][c++].define = STAT_A_ZINS;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10012;
        _iArray[g][c].beraterSkill = 40;
        _iArray[g][c++].define = STAT_A_KREDIT;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10016;
        _iArray[g][c].beraterSkill = 50;
        _iArray[g][c++].define = STAT_A_AKTIEN;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10010;
        _iArray[g][c].beraterSkill = 60;
        _iArray[g][c++].define = STAT_A_FLUGZEUGE;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10014;
        _iArray[g][c].beraterSkill = 60;
        _iArray[g][c++].define = STAT_A_EXPANSION;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10015;
        _iArray[g][c].beraterSkill = 70;
        _iArray[g][c++].define = STAT_A_SABOTAGE;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 10017;
        _iArray[g][c].beraterSkill = 70;
        _iArray[g][c++].define = STAT_A_AGENTUREN;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8006;
        _iArray[g][c].beraterSkill = 70;
        _iArray[g][c++].define = STAT_A_SONSTIGES;

        _iArray[g][c].typOfItem = TYP_SUM_CURR;
        _iArray[g][c].textId = 8014;
        _iArray[g][c].beraterSkill = 0;
        _iArray[g][c++].define = 0;
        c++;

        _iArray[g][c].typOfItem = TYP_SUM_DIFF;
        _iArray[g][c].textId = 8015;
        _iArray[g][c].beraterSkill = 0;
        _iArray[g][c].define = 0;

        // Zweite Seite
        assert(c < STAT_MAX_ITEMS);
        c = 0;
        g = 1;
        _iArray[g][c].typOfItem = TYP_GROUP;
        _iArray[g][c].textId = 8100;
        c++;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8101;
        _iArray[g][c].beraterSkill = 50;
        _iArray[g][c++].define = STAT_AKTIEN_ANZAHL;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8102;
        _iArray[g][c].beraterSkill = 0;
        _iArray[g][c++].define = STAT_AKTIENKURS;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8103;
        _iArray[g][c].beraterSkill = 50;
        _iArray[g][c++].define = STAT_AKTIEN_SA;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8104;
        _iArray[g][c].beraterSkill = 50;
        _iArray[g][c++].define = STAT_AKTIEN_FL;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8105;
        _iArray[g][c].beraterSkill = 50;
        _iArray[g][c++].define = STAT_AKTIEN_PT;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8106;
        _iArray[g][c].beraterSkill = 50;
        _iArray[g][c++].define = STAT_AKTIEN_HA;

        _iArray[g][c].typOfItem = TYP_SHAREVALUE;
        _iArray[g][c].textId = 8107;
        _iArray[g][c].beraterSkill = 60;
        _iArray[g][c++].define = 0;
        c++;
        c++;

        _iArray[g][c].typOfItem = TYP_GROUP;
        _iArray[g][c].textId = 8110;
        c++;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8114;
        _iArray[g][c].beraterSkillInfo = 1;
        _iArray[g][c++].define = STAT_KONTO;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8116;
        _iArray[g][c].beraterSkillInfo = 1;
        _iArray[g][c++].define = STAT_KREDIT;

        _iArray[g][c].typOfItem = TYP_CURRENCY;
        _iArray[g][c].textId = 8115;
        _iArray[g][c].beraterSkill = 80;
        _iArray[g][c++].define = STAT_FIRMENWERT;

        _iArray[g][c].typOfItem = TYP_SINGLE_PERCENT;
        _iArray[g][c].textId = 8113;
        _iArray[g][c].beraterSkill = 50;
        _iArray[g][c++].define = STAT_BEKANNTHEIT;

        _iArray[g][c].typOfItem = TYP_SINGLE_PERCENT;
        if (Sim.Difficulty == DIFF_FREEGAME || Sim.Difficulty == DIFF_ADDON01 || Sim.Difficulty == DIFF_ADDON03 || Sim.Difficulty == DIFF_ADDON04 ||
            Sim.Difficulty == DIFF_ADDON06 || Sim.Difficulty == DIFF_ATFS07 || Sim.Difficulty == DIFF_ATFS09 || Sim.Difficulty == DIFF_ATFS10) {
            _iArray[g][c].typOfItem = TYP_VALUE;
        }
        _iArray[g][c].textId = 10018;
        _iArray[g][c].beraterSkill = 0;
        _iArray[g][c].define = STAT_MISSIONSZIEL;

        // Dritte Seite
        assert(c < STAT_MAX_ITEMS);
        c = 0;
        g = 2;
        _iArray[g][c].typOfItem = TYP_GROUP;
        _iArray[g][c].textId = 8200;
        c++;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8203;
        _iArray[g][c].beraterSkillInfo = 1;
        _iArray[g][c++].define = STAT_FLUEGE;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8204;
        _iArray[g][c].beraterSkillInfo = 1;
        _iArray[g][c++].define = STAT_AUFTRAEGE;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8205;
        _iArray[g][c].beraterSkillInfo = 1;
        _iArray[g][c++].define = STAT_LMAUFTRAEGE;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 10020;
        _iArray[g][c].beraterSkillInfo = 1;
        _iArray[g][c++].define = STAT_FRACHTEN;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8201;
        _iArray[g][c].beraterSkillInfo = 30;
        _iArray[g][c++].define = STAT_FLUGZEUGE;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8202;
        _iArray[g][c].beraterSkillInfo = 40;
        _iArray[g][c++].define = STAT_ROUTEN;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8221;
        _iArray[g][c].beraterSkillInfo = 50;
        _iArray[g][c++].define = STAT_NIEDERLASSUNGEN;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8206;
        _iArray[g][c].beraterSkillInfo = 60;
        _iArray[g][c++].define = STAT_VERSPAETUNG;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8207;
        _iArray[g][c].beraterSkillInfo = 60;
        _iArray[g][c++].define = STAT_UNFAELLE;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8208;
        _iArray[g][c].beraterSkillInfo = 70;
        _iArray[g][c++].define = STAT_SABOTIERT;
        c++;
        c++;

        _iArray[g][c].typOfItem = TYP_GROUP;
        _iArray[g][c].textId = 8210;
        c++;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8211;
        _iArray[g][c].beraterSkillInfo = 70;
        _iArray[g][c++].define = STAT_PASSAGIERE;

        _iArray[g][c].typOfItem = TYP_PERCENT;
        _iArray[g][c].textId = 8212;
        _iArray[g][c].beraterSkillInfo = 80;
        _iArray[g][c++].define = STAT_ZUFR_PASSAGIERE;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 10019;
        _iArray[g][c].beraterSkillInfo = 70;
        _iArray[g][c++].define = STAT_TONS;

        _iArray[g][c].typOfItem = TYP_VALUE;
        _iArray[g][c].textId = 8111;
        _iArray[g][c].beraterSkillInfo = 20;
        _iArray[g][c++].define = STAT_MITARBEITER;

        _iArray[g][c].typOfItem = TYP_PERCENT;
        _iArray[g][c].textId = 8112;
        _iArray[g][c].beraterSkillInfo = 80;
        _iArray[g][c].define = STAT_ZUFR_PERSONAL;

        assert(c < STAT_MAX_ITEMS);
    }

    // Temporäre Dinge
    _fRepaint = true;
    _selectedItem = -1; // Kein (Font-)Item selektiert
    _oldSelectedItem = -1;
    //_days = _newDays = 7;			// Soviele Tage werden angezeigt
    _days = Sim.Statdays;
    _newDays = Sim.StatnewDays;

    // Thomas' Part
    Sim.Players.UpdateStatistics();

    for (c = 1; c < 5; c++) {
        hprintf("stat_%li.mcf", c);
        StatFonts[c - 1].Load(lpDD, const_cast<char *>((LPCTSTR)FullFilename("stat_%li.mcf", MiscPath, c)));
    }

    HighlightBar.ReSize(438, 10);
    HighlightBar.FillWith(0xffffff);

    TopSaver.ReSize(pRoomLib, "TOPSAVER");

    DropDownBackgroundBm.ReSize(pRoomLib, GFX_DECKEL);
    DropDownBm.ReSize(DropDownBackgroundBm.Size.x, DropDownBackgroundBm.Size.y);
    UpDownArrows.ReSize(pRoomLib, "DOWN DOWNHIGH UPHIGH");

    TextTableBm.ReSize(440, 330);

    LupeLogoBms.ReSize(pRoomLib, bprintf("BUTA%liOFF BUTA%liHI BUTA%liON", PlayerNum + 1, PlayerNum + 1, PlayerNum + 1));
    LupeZoomBms.ReSize(pRoomLib, "ZOOMIN ZOOMOUT");
    ButtonGeldBms.ReSize(pRoomLib, "BUTBHI BUTBON");
    ButtonRouteBms.ReSize(pRoomLib, "BUTCHI BUTCON");

    ExitBms.ReSize(pRoomLib, "EXIT EXITHIGH");
    LogoBms.ReSize(pRoomLib, "SWTCH1ON SWTCH1HI SWTCH1SU SWTCH2ON SWTCH2HI SWTCH2SU SWTCH3ON SWTCH3HI SWTCH3SU SWTCH4ON SWTCH4HI SWTCH4SU");
    PobelBms.ReSize(pRoomLib, "POBEL_1 POBEL_2 POBEL_3 POBEL_4");

    Haeckchen.ReSize(pRoomLib, "HAKCHEN");

    CalcGraph();
    RepaintGraphWindow();
    RepaintTextWindow();

    StatButtons.ReSize(11);

    // Die Buttons:         .ID HelpId  ........MouseRect......  .BlitOffset  & #2  ..NormalBm.  ...HighBm..  ..ClickBm..  ClckHi Toggle
    StatButtons[0].ReSize(999, 0, CRect(0, 440, 79, 479), XY(0, 440), nullptr, &ExitBms[0], &ExitBms[1], nullptr, nullptr, FALSE);
    // Hinweis: Der Exit-Button muß immer Button 0 sein!

    // Der Buttons für die DropDown Liste:
    StatButtons[1].ReSize(100, 0, CRect(190, 11, 639, 38), XY(390, 10), &DropDownPos, nullptr, &UpDownArrows[1], nullptr, nullptr, FALSE);

    // Die vier Spielerportraits:
    StatButtons[2].ReSize(110, -1000, CRect(190, 370, 299, 429), XY(190, 370), nullptr, nullptr, &LogoBms[1], &LogoBms[0], &LogoBms[2], TRUE);
    StatButtons[3].ReSize(111, -1001, CRect(300, 370, 409, 429), XY(300, 370), nullptr, nullptr, &LogoBms[4], &LogoBms[3], &LogoBms[5], TRUE);
    StatButtons[4].ReSize(112, -1002, CRect(410, 370, 519, 429), XY(410, 370), nullptr, nullptr, &LogoBms[7], &LogoBms[6], &LogoBms[8], TRUE);
    StatButtons[5].ReSize(113, -1003, CRect(520, 370, 629, 429), XY(520, 370), nullptr, nullptr, &LogoBms[10], &LogoBms[9], &LogoBms[11], TRUE);

    // Die 3 Radio-Buttons links unten
    StatButtons[6].ReSize(120, 3300, CRect(10, 370, 65, 429), XY(10, 370), nullptr, &LupeLogoBms[0], &LupeLogoBms[1], &LupeLogoBms[2], &LupeLogoBms[2], TRUE);
    StatButtons[7].ReSize(121, 3301, CRect(70, 370, 125, 429), XY(70, 370), nullptr, nullptr, &ButtonGeldBms[0], &ButtonGeldBms[1], &ButtonGeldBms[1], TRUE);
    StatButtons[8].ReSize(122, 3302, CRect(130, 370, 185, 429), XY(130, 370), nullptr, nullptr, &ButtonRouteBms[0], &ButtonRouteBms[1], &ButtonRouteBms[1],
                          TRUE);

    // Die Zoom-Buttons für die Drop-Down Liste:
    StatButtons[9].ReSize(130, 0, CRect(374, -318, 408, -293), XY(374, -318), &DropDownPos, nullptr, &LupeZoomBms[1], &LupeZoomBms[1], nullptr, FALSE);
    StatButtons[10].ReSize(131, 0, CRect(412, -318, 446, -293), XY(412, -318), &DropDownPos, nullptr, &LupeZoomBms[0], &LupeZoomBms[0], nullptr, FALSE);

    // Den entsprechenden Group-Button hilighten
    for (c = StatButtons.AnzEntries() - 1; c >= 0; c--) {
        if (StatButtons[c].Id == 120 + _group) {
            StatButtons[c].LastClicked = 1;
        }

        if (StatButtons[c].Id == 110 && _playerMask[0]) {
            StatButtons[c].LastClicked = 1;
        }
        if (StatButtons[c].Id == 111 && _playerMask[1]) {
            StatButtons[c].LastClicked = 1;
        }
        if (StatButtons[c].Id == 112 && _playerMask[2]) {
            StatButtons[c].LastClicked = 1;
        }
        if (StatButtons[c].Id == 113 && _playerMask[3]) {
            StatButtons[c].LastClicked = 1;
        }
    }

    if (DropDownPos.y > 0) {
        RepaintGraphWindow();
    }
}

//--------------------------------------------------------------------------------------------
// Die Welt geht unter:
//--------------------------------------------------------------------------------------------
CStatistik::~CStatistik() {
    Sim.StatplayerMask = _playerMask;
    Sim.Statgroup = _group;
    Sim.StatfGraphVisible = _fGraphVisible;
    Sim.Statdays = _days;
    Sim.StatnewDays = _newDays;
    Sim.DropDownPosY = DropDownPos.y;

    if (Sim.DropDownPosY < 329 / 2) {
        Sim.DropDownPosY = 0;
    } else {
        Sim.DropDownPosY = 329;
    }

    for (short p = 0; p < STAT_MAX_GROUPS; p++) {
        for (short i = 0; i < STAT_MAX_ITEMS; i++) {
            Sim.StatiArray[p][i] = _iArray[p][i].visible;
        }
    }

    HighlightBar.Destroy();
    TopSaver.Destroy();

    DropDownBackgroundBm.Destroy();
    DropDownBm.Destroy();
    UpDownArrows.Destroy();

    TextTableBm.Destroy();

    LupeLogoBms.Destroy();
    LupeZoomBms.Destroy();
    ButtonGeldBms.Destroy();
    ButtonRouteBms.Destroy();

    ExitBms.Destroy();
    LogoBms.Destroy();

    Haeckchen.Destroy();
    PobelBms.Destroy();

    StatusCount = 20;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CStatistik message handlers
//////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------
// void CStatistik::OnPaint()
//--------------------------------------------------------------------------------------------
void CStatistik::OnPaint() {
    SLONG c = 0;
    SBBM *ExitBm = nullptr;
    XY ExitBmPos;
    DWORD CurrentTime = AtGetTime();
    static SLONG RefreshStatistics = 0;

    if (((++RefreshStatistics) & 7) == 0) {
        Sim.Players.UpdateStatistics();
    }

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    // Die Tafel bewegen:
    if (DropDownSpeed > 0) {
        DropDownSpeed += 10;
        while (Calc1nSum(DropDownSpeed) / 3 > 329 - DropDownPos.y) {
            DropDownSpeed--;
        }

        DropDownPos.y += DropDownSpeed;
        if (DropDownPos.y >= 329) {
            DropDownSpeed = 0;
            DropDownPos.y = 329;
            StatButtons[1].BitmapNormal = nullptr;
            StatButtons[1].BitmapHi = &UpDownArrows[2];
            StatButtons[1].BitmapClicked = nullptr;
        }
    } else if (DropDownSpeed < 0) {
        DropDownSpeed -= 10;
        while (Calc1nSum(-DropDownSpeed) / 3 > DropDownPos.y) {
            DropDownSpeed++;
        }

        DropDownPos.y += DropDownSpeed;
        if (DropDownPos.y <= 0) {
            DropDownSpeed = 0;
            DropDownPos.y = 0;
        }
    }

    // Den Graphen in X animieren
    if (_newDays != _days) {
        SLONG diff = (_newDays - _days) / 3 * 2;
        if (diff == 0) {
            _days = _newDays;
        } else {
            _days = _days + diff;
        }

        _fRepaint = true;

        if (DropDownPos.y != 0) {
            CalcGraph();
        }
    }

    // Den Screen bemalen:
    if (DropDownPos.y != 329) {
        RoomBm.BlitFrom(TextTableBm, 190, 40);
    }
    if (DropDownPos.y != 0) {
        RoomBm.pBitmap->SetClipRect(CRect(190, 10, 630, 370));
        RoomBm.BlitFrom(DropDownBm, 190, 40 - 360 + DropDownPos.y);
        RoomBm.pBitmap->SetClipRect(CRect(0, 0, 640, 440));
    }

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips();

    // Highlights und ToolTips der Buttons verwalten:
    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
        for (c = StatButtons.AnzEntries() - 1; c >= 0; c--) {
            XY BlitPos = StatButtons[c].BitmapOffset;
            XY off = XY(0, 0);

            if (StatButtons[c].HelpId <= -1000) {
                if (Sim.Players.Players[-StatButtons[c].HelpId - 1000].IsOut != 0) {
                    continue;
                }
            }

            ExitBmPos = BlitPos;

            if (StatButtons[c].BitmapOffset2 != nullptr) {
                off = *StatButtons[c].BitmapOffset2;
                BlitPos += off;
            }

            if (DropDownPos.y != 329 && StatButtons[c].HelpId < 0 && (IsDialogOpen() == 0) && (MenuIsOpen() == 0) &&
                gMousePosition.IfIsWithin(StatButtons[c].HotArea.left, StatButtons[c].HotArea.top, StatButtons[c].HotArea.right,
                                          StatButtons[c].HotArea.bottom)) {
                SetMouseLook(CURSOR_HOT, -StatButtons[c].HelpId,
                             Sim.Players.Players[-StatButtons[c].HelpId - 1000].AirlineX + " (" + Sim.Players.Players[-StatButtons[c].HelpId - 1000].NameX +
                                 ")",
                             ROOM_STATISTICS, -1);
                if (StatButtons[c].BitmapClicked != nullptr) {
                    RoomBm.BlitFromT(*StatButtons[c].BitmapClicked, BlitPos);
                }
            } else if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) &&
                       gMousePosition.IfIsWithin(StatButtons[c].HotArea.left + off.x, StatButtons[c].HotArea.top + off.y, StatButtons[c].HotArea.right + off.x,
                                                 StatButtons[c].HotArea.bottom + off.y)) {
                // Highlight:
                if ((StatButtons[c].IsToggle != 0) && StatButtons[c].LastClicked != 0) {
                    if (StatButtons[c].BitmapSuperHi != nullptr) {
                        if (c == 0) {
                            ExitBm = StatButtons[c].BitmapSuperHi;
                        }
                        RoomBm.BlitFromT(*StatButtons[c].BitmapSuperHi, BlitPos);
                    }
                } else if (StatButtons[c].BitmapHi != nullptr) {
                    if (c == 0) {
                        ExitBm = StatButtons[c].BitmapHi;
                    }
                    RoomBm.BlitFromT(*StatButtons[c].BitmapHi, BlitPos);
                }

                if (StatButtons[c].HelpId >= 0) {
                    SetMouseLook((c == 0) ? CURSOR_EXIT : CURSOR_HOT, StatButtons[c].HelpId, ROOM_STATISTICS, StatButtons[c].Id);
                } else {
                    SetMouseLook((c == 0) ? CURSOR_EXIT : CURSOR_HOT, -StatButtons[c].HelpId,
                                 Sim.Players.Players[-StatButtons[c].HelpId - 1000].AirlineX + " (" + Sim.Players.Players[-StatButtons[c].HelpId - 1000].NameX +
                                     ")",
                                 ROOM_STATISTICS, StatButtons[c].Id);
                }
            } else {
                // Kein Highlight, aber vielleicht geklickt?
                if (((StatButtons[c].IsToggle != 0) && StatButtons[c].LastClicked != 0) ||
                    ((StatButtons[c].IsToggle == 0) && CurrentTime - StatButtons[c].LastClicked < ClickHighlightTime) ||
                    (DropDownPos.y != 329 && StatButtons[c].HelpId < 0)) {
                    if ((StatButtons[c].BitmapClicked != nullptr) || (DropDownPos.y != 329 && StatButtons[c].HelpId < 0)) {
                        if (c == 0) {
                            ExitBm = StatButtons[c].BitmapClicked;
                        }
                        RoomBm.BlitFromT(*StatButtons[c].BitmapClicked, BlitPos);
                    }
                } else // ganz normal:
                {
                    if (StatButtons[c].BitmapNormal != nullptr) {
                        if (c == 0) {
                            ExitBm = StatButtons[c].BitmapNormal;
                        }
                        RoomBm.BlitFromT(*StatButtons[c].BitmapNormal, BlitPos);
                    }
                }
            }
        }
    }

    RoomBm.BlitFrom(TopSaver, 374, 0);

    // Den wievielte Tag spielen wir gerade?
    CString output = getCurrentDayString();

    SLONG length = FontDialogPartner.GetWidth((LPCTSTR)output, output.GetLength());
    SLONG xPos = 35 + ((160 - 35) >> 1) - (length >> 1);
    RoomBm.PrintAt(output, FontDialogPartner, TEC_FONT_LEFT, xPos, 15, xPos + length + 5, 45);

    // Zeichnet die Einträge einer Gruppe. Ist der Mauscursor
    // über einem Eintrag wird dieser focusiert.
    SLONG xOffset[] = {0, -18, -11, -11, -11, -11};
    SLONG xOffset2[] = {-11, -18, -11, -11, -11, -11};
    RECT rc = {45, 56, 155, 66};

    _selectedItem = -1;
    rc.top += LINE_DISTANCE / 2;
    rc.bottom += LINE_DISTANCE / 2;
    for (short i = 0; i < STAT_MAX_ITEMS; i++) {
        if (_iArray[_group][i].typOfItem != TYP_LINEFEED) {
            word group = _iArray[_group][i].typOfItem;
            if (group >= 20) {
                group = 0;
            }

            CString text = StandardTexte.GetS("STAT", _iArray[_group][i].textId);
            RoomBm.PrintAt(text, (group == 0U) ? FontSmallBlack : FontSmallPlastic, TEC_FONT_LEFT, rc.left + xOffset[group], rc.top, rc.right, rc.bottom);

            if (_iArray[_group][i].visible && group == 0) {
                RoomBm.BlitFromT(Haeckchen, rc.left - 11, rc.top);
            }

            if (_selectedItem == -1) {
                if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0) &&
                    (CheckCursorHighlight(gMousePosition, CRect(rc.left + xOffset2[group], rc.top, rc.right, rc.bottom), ColorOfFontBlack) != 0)) {
                    _selectedItem = i;
                    if (_oldSelectedItem != _selectedItem) {
                        _oldSelectedItem = _selectedItem;
                        _fRepaint = true;
                    }

                    if (DropDownPos.y == 0) {
                        ColorFX.BlitTrans(HighlightBar.pBitmap, RoomBm.pBitmap, XY(190, rc.top), nullptr, 6);
                    }
                } else {
                    if (_oldSelectedItem != _selectedItem) {
                        _oldSelectedItem = _selectedItem;
                        _fRepaint = true;
                    }
                }
            }
        }
        rc.top += LINE_DISTANCE;
        rc.bottom = rc.top + 10;
    }

    // Die Werte sollten gelegentlich neu gezeichnet werden
    static SLONG tickers = 0;
    if (_fRepaint || (AtGetTickCount() - tickers > 1500 && DropDownSpeed == 0)) {
        tickers = AtGetTickCount();
        _fRepaint = false;

        if (DropDownPos.y == 0) { // Ganz oben...
            RepaintTextWindow();
        } else { // Unten
            RepaintGraphWindow();
        }
    }

    // Statuszeile zeichnen:
    CStdRaum::PostPaint();

    // Ein Patch: Die Exit-Bitmap nach der Statuszeile erneut zeichnen:
    if (ExitBm != nullptr) {
        PrimaryBm.PrimaryBm.SetClipRect(CRect(0, 0, 640, 480));
        PrimaryBm.BlitFrom(*ExitBm, ExitBmPos);
    }

    CStdRaum::PumpToolTips();
}

//--------------------------------------------------------------------------------------------
// Berechnet einige Sachen für den Graphen
//--------------------------------------------------------------------------------------------
void CStatistik::CalcGraph() {
    __int64 min = 0;
    __int64 max = 0;

    // Max-Min Werte
    for (short i = 0; i < STAT_MAX_ITEMS; i++) {
        auto item = _iArray[_group][i];
        if (item.typOfItem < TYP_VALUE || !item.visible) {
            continue;
        }
        for (SLONG p = 0; p < 4; p++) {
            if (Sim.Players.Players[p].IsOut != 0) {
                continue;
            }
            if (!_playerMask[p]) {
                continue;
            }
            auto berater = (p == PlayerNum) ? BERATERTYP_GELD : BERATERTYP_INFO;
            if (Sim.Players.Players[PlayerNum].HasBerater(berater) < item.beraterSkill) {
                continue;
            }
            if (p != PlayerNum && Sim.Players.Players[PlayerNum].HasBerater(berater) < item.beraterSkillInfo) {
                continue;
            }

            for (SLONG d = 0; d <= _days; d++) {
                __int64 val = Sim.Players.Players[p].Statistiken[static_cast<SLONG>(item.define)].GetAtPastDay(d);
                if (item.typOfItem == TYP_PERCENT) {
                    auto prevItem = _iArray[_group][i - 1];
                    auto prevVal = Sim.Players.Players[p].Statistiken[static_cast<SLONG>(prevItem.define)].GetAtPastDay(d);
                    if (prevVal != 0) {
                        min = Min(val * __int64(100) / prevVal, min);
                        max = Max(val * __int64(100) / prevVal, max);
                    }
                } else {
                    min = Min(val, min);
                    max = Max(val, max);
                }
            }
        }
    }

    DOUBLE summe = DOUBLE(max) + DOUBLE(-min);
    _yGraph = static_cast<DOUBLE> G_HEIGHT / summe;
    _yAxis = static_cast<DOUBLE>(-min);
}

//--------------------------------------------------------------------------------------------
// Zeichnet das Menü mit dem Graphen neu:
//--------------------------------------------------------------------------------------------
void CStatistik::RepaintGraphWindow() {
    SLONG x1 = 0;
    SLONG y1 = 0;
    SLONG x2 = 0;
    SLONG y2 = -1;
    SLONG value = 0;
    bool fDrawAxis = false;

    DropDownBm.BlitFrom(DropDownBackgroundBm);

    // Ausgabe des Zoom-Faktors in Tage oder Monaten
    CString output;
    if (_days <= 30) {
        output = bitoa(_days);
        output += " ";
        output += StandardTexte.GetS(TOKEN_STAT, 9001);
    } else {
        output = bitoa(_days / 30);
        output += " ";
        output += StandardTexte.GetS(TOKEN_STAT, 9002);
    }

    DropDownBm.PrintAt(output, StatFonts[1], TEC_FONT_LEFT, 263, 12, G_RIGHT, G_BOTTOM);

    if (Sim.Date == 0) { // Am ersten Tag keine Statistik möglich
        return;
    }

    // Y-Axis
    SLONG yAxis = static_cast<SLONG>(_yAxis * _yGraph);

    _xGraph = static_cast<DOUBLE> G_WIDTH / static_cast<DOUBLE>(_days);

    SLONG days = _days;
    if (days > (Sim.Date + 1)) {
        days = Sim.Date + 1;
    }

    DropDownBm.pBitmap->SetClipRect(CRect(G_LEFT, G_TOP, G_RIGHT, G_BOTTOM + 1));

    for (short i = 0; i < STAT_MAX_ITEMS; i++) {
        auto item = _iArray[_group][i];
        if (item.typOfItem < TYP_VALUE || !item.visible) {
            continue;
        }
        for (SLONG p = 0; p < 4; p++) {
            if (Sim.Players.Players[p].IsOut != 0) {
                continue;
            }
            if (!_playerMask[p]) {
                continue;
            }
            auto berater = (p == PlayerNum) ? BERATERTYP_GELD : BERATERTYP_INFO;
            if (Sim.Players.Players[PlayerNum].HasBerater(berater) < item.beraterSkill) {
                continue;
            }
            if (p != PlayerNum && Sim.Players.Players[PlayerNum].HasBerater(berater) < item.beraterSkillInfo) {
                continue;
            }

            for (SLONG d = 0; d <= days; d++) {
                DOUBLE val = Sim.Players.Players[p].Statistiken[static_cast<SLONG>(item.define)].GetAtPastDay(d);
                if (item.typOfItem == TYP_PERCENT) {
                    auto prevItem = _iArray[_group][i - 1];
                    auto prevVal = Sim.Players.Players[p].Statistiken[static_cast<SLONG>(prevItem.define)].GetAtPastDay(d);
                    if (prevVal != 0) {
                        value = static_cast<SLONG>(val * __int64(100) / prevVal * _yGraph);
                    } else {
                        value = 0;
                    }
                } else {
                    value = static_cast<SLONG>(val * _yGraph);
                }

                if (d == 0) {
                    x2 = G_RIGHT;
                    y2 = G_BOTTOM - yAxis - value;
                    continue;
                }

                x1 = G_RIGHT - static_cast<SLONG>(static_cast<DOUBLE>(d) * _xGraph);
                y1 = G_BOTTOM - yAxis - value;

                if (_selectedItem != -1 && _selectedItem != i && _iArray[_group][_selectedItem].typOfItem != TYP_GROUP) {
                    DropDownBm.pBitmap->Line(x1, y1, x2, y2, DropDownBm.pBitmap->GetHardwarecolor(DarkColors[p]));
                } else {
                    DropDownBm.pBitmap->Line(x1, y1, x2, y2, DropDownBm.pBitmap->GetHardwarecolor(AktienKursLineColor[p]));
                }

                DropDownBm.BlitFrom(PobelBms[p], x2 - 2, y2 - 2);
                DropDownBm.BlitFrom(PobelBms[p], x1 - 2, y1 - 2);

                x2 = x1;
                y2 = y1;
                fDrawAxis = true;
            }
        }
    }

    DropDownBm.pBitmap->InitClipRect();

    // Die Y-Achse zeichnen
    if (fDrawAxis) {
        SLONG yPos = G_BOTTOM - yAxis;
        DropDownBm.pBitmap->Line(G_LEFT, yPos, G_RIGHT, yPos, DropDownBm.pBitmap->GetHardwarecolor(0xffffff));
    }
}

//--------------------------------------------------------------------------------------------
// Zeichnet das Menü mit dem Text neu:
//--------------------------------------------------------------------------------------------
void CStatistik::RepaintTextWindow() {
    SLONG p = 0;

    TextTableBm.BlitFrom(PicBitmap, -191, -40);

    for (p = 0; p < 4; p++) {
        if (Sim.Players.Players[p].IsOut == 0) {
            CString output;
            __int64 summe = 0;
            __int64 differenz = 0;

            RECT rc;
            rc.left = p * 109;
            rc.top = 16;
            rc.right = rc.left + 109;
            rc.bottom = rc.top + 10;

            rc.top += LINE_DISTANCE / 2;
            rc.bottom += LINE_DISTANCE / 2;
            for (short i = 0; i < STAT_MAX_ITEMS; i++) {
                auto item = _iArray[_group][i];
                if (item.typOfItem > TYP_GROUP) {
                    __int64 val = Sim.Players.Players[p].Statistiken[static_cast<SLONG>(item.define)].GetAtPastDay(0);

                    auto type = item.typOfItem;
                    auto berater = (p == PlayerNum) ? BERATERTYP_GELD : BERATERTYP_INFO;
                    if (Sim.Players.Players[PlayerNum].HasBerater(berater) < item.beraterSkill) {
                        type = TYP_UNAVAILABLE;
                    }
                    if (p != PlayerNum && Sim.Players.Players[PlayerNum].HasBerater(berater) < item.beraterSkillInfo) {
                        type = TYP_UNAVAILABLE;
                    }
                    switch (type) {
                    case TYP_UNAVAILABLE:
                        output = CString("???");
                        break;

                    case TYP_VALUE:
                        output = bprintf("%I64i", val);

                        if (item.visible) {
                            summe += val;
                        }
                        break;

                    case TYP_CURRENCY:
                        output = Einheiten[EINH_DM].bString64(val);

                        if (item.visible) {
                            summe += val;
                        }
                        break;

                    case TYP_SINGLE_PERCENT:
                        output = Einheiten[EINH_P].bString64(val);

                        if (item.visible) {
                            summe += val;
                        }
                        break;

                    case TYP_PERCENT: {
                        auto prevItem = _iArray[_group][i - 1];
                        auto prevVal = Sim.Players.Players[p].Statistiken[static_cast<SLONG>(prevItem.define)].GetAtPastDay(0);
                        if (prevVal != 0) {
                            output = Einheiten[EINH_P].bString64(val * 100 / prevVal);
                        } else {
                            output = "0%";
                        }

                        if (item.visible) {
                            summe += val;
                        }
                    } break;

                    case TYP_SUM_CURR:
                        output = Einheiten[EINH_DM].bString64(summe);

                        differenz += summe;
                        summe = 0;
                        break;

                    case TYP_SUM_DIFF:
                        output = Einheiten[EINH_DM].bString64(differenz);
                        differenz = 0;
                        break;

                    case TYP_SHAREVALUE: {
                        __int64 shares = 0;

                        if (_iArray[_group][2].visible) {
                            if (_iArray[_group][3].visible) {
                                shares += Sim.Players.Players[p].Statistiken[STAT_AKTIEN_SA].GetAtPastDay(0) *
                                          Sim.Players.Players[0].Statistiken[STAT_AKTIENKURS].GetAtPastDay(0);
                            }
                            if (_iArray[_group][4].visible) {
                                shares += Sim.Players.Players[p].Statistiken[STAT_AKTIEN_FL].GetAtPastDay(0) *
                                          Sim.Players.Players[1].Statistiken[STAT_AKTIENKURS].GetAtPastDay(0);
                            }
                            if (_iArray[_group][5].visible) {
                                shares += Sim.Players.Players[p].Statistiken[STAT_AKTIEN_PT].GetAtPastDay(0) *
                                          Sim.Players.Players[2].Statistiken[STAT_AKTIENKURS].GetAtPastDay(0);
                            }
                            if (_iArray[_group][6].visible) {
                                shares += Sim.Players.Players[p].Statistiken[STAT_AKTIEN_HA].GetAtPastDay(0) *
                                          Sim.Players.Players[3].Statistiken[STAT_AKTIENKURS].GetAtPastDay(0);
                            }
                        }

                        // if (shares>2147483647) shares=2147483647;    //Overflow verhindern
                        output = Einheiten[EINH_DM].bString64(shares);
                        break;
                    }
                    default:
                        hprintf("Statistik.cpp: Default case should not be reached.");
                        DebugBreak();
                    }

                    TextTableBm.PrintAt(output, StatFonts[p], TEC_FONT_RIGHT, rc.left, rc.top, rc.right - 5, rc.bottom);
                }

                rc.top += LINE_DISTANCE;
                rc.bottom = rc.top + 10;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// void CStatistik::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CStatistik::OnLButtonDown(UINT nFlags, CPoint point) {
    XY RoomPos;
    SLONG c = 0;
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    DefaultOnLButtonDown();

    if (ConvertMousePosition(point, &RoomPos) == 0) {
        CStdRaum::OnLButtonDown(nFlags, point);
        return;
    }

    if (PreLButtonDown(point) == 0) {
        if (MouseClickArea == ROOM_STATISTICS) {
            // Die Internen Daten des Buttons anpassen:
            for (c = StatButtons.AnzEntries() - 1; c >= 0; c--) {
                if (StatButtons[c].Id == MouseClickId) {
                    if (StatButtons[c].IsToggle != 0) {
                        StatButtons[c].LastClicked ^= 1;
                    } else {
                        StatButtons[c].LastClicked = AtGetTime();
                    }
                }
            }

            // Hier kann man auf einen Button-Click reagieren:
            switch (MouseClickId) {
            case 999:
                qPlayer.LeaveRoom();
                break;
            case 100:
                if (DropDownPos.y > 0) {
                    DropDownSpeed = -25;
                    StatButtons[1].BitmapHi = &UpDownArrows[1];
                    RepaintTextWindow();
                } else {
                    DropDownSpeed = 25;
                    StatButtons[1].BitmapHi = nullptr;
                    StatButtons[1].BitmapClicked = &UpDownArrows[0];
                    StatButtons[1].BitmapNormal = &UpDownArrows[0];
                    RepaintGraphWindow();
                }
                break;

            case 110:
                _playerMask[0] = !_playerMask[0];
                CalcGraph();
                _fRepaint = true;
                break;
            case 111:
                _playerMask[1] = !_playerMask[1];
                CalcGraph();
                _fRepaint = true;
                break;
            case 112:
                _playerMask[2] = !_playerMask[2];
                CalcGraph();
                _fRepaint = true;
                break;
            case 113:
                _playerMask[3] = !_playerMask[3];
                CalcGraph();
                _fRepaint = true;
                break;

            case 120:
                for (c = StatButtons.AnzEntries() - 1; c >= 0; c--) {
                    if (StatButtons[c].Id == 120) {
                        StatButtons[c].LastClicked = 1;
                    }
                    if (StatButtons[c].Id == 122 || StatButtons[c].Id == 121) {
                        StatButtons[c].LastClicked = 0;
                    }
                }

                _group = 0;
                RepaintTextWindow();
                CalcGraph();
                RepaintGraphWindow();
                break;

            case 121:
                for (c = StatButtons.AnzEntries() - 1; c >= 0; c--) {
                    if (StatButtons[c].Id == 121) {
                        StatButtons[c].LastClicked = 1;
                    }
                    if (StatButtons[c].Id == 120 || StatButtons[c].Id == 122) {
                        StatButtons[c].LastClicked = 0;
                    }
                }

                _group = 1;
                RepaintTextWindow();
                CalcGraph();
                RepaintGraphWindow();
                break;

            case 122:
                for (c = StatButtons.AnzEntries() - 1; c >= 0; c--) {
                    if (StatButtons[c].Id == 122) {
                        StatButtons[c].LastClicked = 1;
                    }
                    if (StatButtons[c].Id == 120 || StatButtons[c].Id == 121) {
                        StatButtons[c].LastClicked = 0;
                    }
                }

                _group = 2;
                RepaintTextWindow();
                CalcGraph();
                RepaintGraphWindow();
                break;

                // Zoom out
            case 130: {
                if (_days > days[0]) {
                    for (word i = 0; i < (sizeof(days) / sizeof(days[0])); i++) {
                        if (days[i] == _days) {
                            _newDays = days[i - 1];
                            break;
                        }
                    }
                }
            } break;

                // Zoom in
            case 131:
                if (_days < days[(sizeof(days) / sizeof(days[0])) - 1]) {
                    for (word i = 0; i < (sizeof(days) / sizeof(days[0])); i++) {
                        if (days[i] == _days) {
                            _newDays = days[i + 1];
                            break;
                        }
                    }
                }
                break;
            default:
                break;
            }
        }

        // Prüfen ob ein (Font-)Item angeklickt wurde
        if (_selectedItem != -1) {
            // Falls das gewählte Icon eine Gruppe war diese komplett
            // ein- oder ausschalten.
            if (_iArray[_group][_selectedItem].typOfItem == TYP_GROUP) {
                while (++_selectedItem < STAT_MAX_ITEMS && _iArray[_group][_selectedItem].typOfItem != TYP_GROUP) {
                    _iArray[_group][_selectedItem].visible = !_iArray[_group][_selectedItem].visible;
                }
            } else {
                if ((AtGetAsyncKeyState(ATKEY_SHIFT) & 0xff00) != 0) {
                    SLONG c = _selectedItem;

                    while (c >= 0 && _iArray[_group][c].typOfItem != TYP_GROUP) {
                        c--;
                    }

                    while (++c < STAT_MAX_ITEMS && _iArray[_group][c].typOfItem != TYP_GROUP) {
                        _iArray[_group][c].visible = FALSE;
                    }

                    _iArray[_group][_selectedItem].visible = TRUE;
                } else {
                    _iArray[_group][_selectedItem].visible = !_iArray[_group][_selectedItem].visible;
                }
            }

            _selectedItem = -1;
            RepaintTextWindow();

            CalcGraph();
            RepaintGraphWindow();
        } else {
            CStdRaum::OnLButtonDown(nFlags, point);
        }
    }
}

//--------------------------------------------------------------------------------------------
// void CStatistik::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CStatistik::OnRButtonDown(UINT nFlags, CPoint point) {
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

//--------------------------------------------------------------------------------------------
// Initialisieren:
//--------------------------------------------------------------------------------------------
void CStatButton::ReSize(SLONG Id, SLONG HelpId, CRect HotArea, XY BitmapOffset, XY *BitmapOffset2, SBBM *BitmapNormal, SBBM *BitmapHi, SBBM *BitmapClicked,
                         SBBM *BitmapSuperHi, BOOL IsToggle) {
    CStatButton::Id = Id;
    CStatButton::HelpId = HelpId;
    CStatButton::HotArea = HotArea;
    CStatButton::BitmapOffset = BitmapOffset;
    CStatButton::BitmapOffset2 = BitmapOffset2;
    CStatButton::BitmapNormal = BitmapNormal;
    CStatButton::BitmapHi = BitmapHi;
    CStatButton::BitmapClicked = BitmapClicked;
    CStatButton::BitmapSuperHi = BitmapSuperHi;
    CStatButton::IsToggle = IsToggle;
    CStatButton::LastClicked = 0;
}
