//============================================================================================
// Personal.Cpp - Das Personalbüro des Spielers
//============================================================================================
// Link: "Personal.h"
//============================================================================================
#include "StdAfx.h"
#include "glpers.h"

#define AT_Log(...) // AT_Log_I("Personal", __VA_ARGS__)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern SLONG SaveVersion;
extern SLONG SaveVersionSub;

SLONG ReadLine(BUFFER_V<UBYTE> &Buffer, SLONG BufferStart, char *Line, SLONG LineLength);

//--------------------------------------------------------------------------------------------
// Konstruktor
//--------------------------------------------------------------------------------------------
CPersonal::CPersonal(BOOL bHandy, ULONG PlayerNum) : CStdRaum(bHandy, PlayerNum, "personal.gli", GFX_PERSONAL) {
    SetRoomVisited(PlayerNum, ROOM_PERSONAL_A);

    Sim.FocusPerson = -1;

    if (bHandy == 0) {
        AmbientManager.SetGlobalVolume(40);
    }

    KugelBm.ReSize(pRoomLib, "KUGEL");
    WildBm.ReSize(pRoomLib, "ZEITSCHR");
    LetterBm.ReSize(pRoomLib, "MITTELB");
    StapelBriefBm.ReSize(pRoomLib, "BRIEF");
    NoSaboBm.ReSize(pRoomLib, "NOSABO");

    FlugbahnType = -1;

    PaperOnTable = -1;
    PaperDropping = -1;
    StapelBrief = TRUE;
    Zeitung = TRUE;
    KommVar = -1;
    KommVar2 = -1;

    SP_Frau.ReSize(11);
    //--------------------------------------------------------------------------------------------
    SP_Frau.Clips[0].ReSize(0, "pfbrief.smk", "pfbrief.raw", XY(340, 113), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &StapelBrief,
                            SMACKER_CLIP_XOR | (SMACKER_CLIP_FRAME + 1 * SMACKER_CLIP_MULT), 1, nullptr, // Warten
                            "A9", 1);
    SP_Frau.Clips[1].ReSize(1, "pfleseb.smk", "", XY(340, 113), SPM_IDLE, CRepeat(1, 1), CPostWait(30, 90), SMACKER_CLIP_CANCANCEL, &PaperOnTable,
                            SMACKER_CLIP_SET | SMACKER_CLIP_PRE, 2, nullptr, // Warten
                            "A9", 2);
    SP_Frau.Clips[2].ReSize(2, "pfwerf.smk", "pfwerf.raw", XY(340, 113), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &FlugbahnType,
                            SMACKER_CLIP_SET | (SMACKER_CLIP_FRAME + 5 * SMACKER_CLIP_MULT), 0, &KommVar, // Warten
                            "A9A2E1", 0, 3, 6);

    SP_Frau.Clips[3].ReSize(3, "pfwild.smk", "pfwild.raw", XY(340, 113), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &Zeitung,
                            SMACKER_CLIP_SET | SMACKER_CLIP_POST, 0, nullptr, // Warten
                            "A9", 4);
    SP_Frau.Clips[4].ReSize(4, "pflesew.smk", "", XY(340, 113), SPM_IDLE, CRepeat(1, 1), CPostWait(70, 130), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, &KommVar2, // Warten
                            "A9", 5);
    SP_Frau.Clips[5].ReSize(5, "pfwildz.smk", "pfwildz.raw", XY(340, 113), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &Zeitung,
                            SMACKER_CLIP_SET | SMACKER_CLIP_PRE, 1, nullptr, // Warten
                            "A9E1", 0, 6);

    SP_Frau.Clips[6].ReSize(6, "pfturn.smk", "", XY(340, 113), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "E1E1E1", 8, 7, 9);
    SP_Frau.Clips[7].ReSize(7, "pfrede.smk", "", XY(340, 113), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, nullptr, // Labern
                            "A9E1E1", 7, 8, 9);
    SP_Frau.Clips[8].ReSize(8, "pfturnw.smk", "", XY(340, 113), SPM_LISTENING, CRepeat(1, 1), CPostWait(20, 20), SMACKER_CLIP_CANCANCEL, nullptr,
                            SMACKER_CLIP_SET, 0, nullptr, // Zuhören
                            "A9A1E1E1", 8, 10, 7, 9);
    SP_Frau.Clips[9].ReSize(9, "pfturnz.smk", "", XY(340, 113), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 0);
    SP_Frau.Clips[10].ReSize(10, "pfturnk.smk", "", XY(340, 113), SPM_LISTENING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                             SMACKER_CLIP_SET, 0, nullptr, // Warten
                             "A9", 8);

    SP_Mann.ReSize(9);
    //--------------------------------------------------------------------------------------------
    SP_Mann.Clips[0].ReSize(0, "pmlese.smk", "", XY(130, 80), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            &PaperOnTable, // Warten
                            "A1A1E1", 0, 1, 3);
    SP_Mann.Clips[1].ReSize(1, "pmblat.smk", "", XY(130, 80), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            &PaperOnTable, // Warten
                            "A9", 0);

    SP_Mann.Clips[2].ReSize(2, "pmkorb.smk", "", XY(130, 80), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &PaperOnTable,
                            SMACKER_CLIP_SET | (SMACKER_CLIP_FRAME + 4 * SMACKER_CLIP_MULT), -1, nullptr, "A9", 7);
    SP_Mann.Clips[7].ReSize(7, "pmkorbb.smk", "", XY(130, 80), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &PaperDropping,
                            SMACKER_CLIP_SET | SMACKER_CLIP_PRE, 1, nullptr, "A9E1", 1, 3);

    SP_Mann.Clips[3].ReSize(3, "pmturn.smk", "", XY(130, 80), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "E1E1A1E1", 4, 5, 5, 6);
    SP_Mann.Clips[4].ReSize(4, "pmreden.smk", "", XY(130, 80), SPM_TALKING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                            0, nullptr, // Warten
                            "A9E1E1", 4, 5, 6);
    SP_Mann.Clips[5].ReSize(5, "pmwait.smk", "", XY(130, 80), SPM_LISTENING, CRepeat(1, 1), CPostWait(99, 99), SMACKER_CLIP_CANCANCEL, nullptr,
                            SMACKER_CLIP_SET, 0, nullptr, // Warten
                            "A9A1E1E1", 5, 8, 4, 6);
    SP_Mann.Clips[6].ReSize(6, "pmturnz.smk", "", XY(130, 80), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr, SMACKER_CLIP_SET, 0,
                            nullptr, // Warten
                            "A9", 0);
    SP_Mann.Clips[8].ReSize(8, "pmwaitk.smk", "", XY(130, 80), SPM_LISTENING, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, nullptr,
                            SMACKER_CLIP_SET, 0, nullptr, // Warten
                            "A9", 5);

    SP_DroppingPaper.ReSize(2);
    //--------------------------------------------------------------------------------------------
    SP_DroppingPaper.Clips[0].ReSize(0, "", "", XY(133, 260), SPM_LISTENING, CRepeat(1, 1), CPostWait(2, 2), SMACKER_CLIP_CANCANCEL, nullptr, SMACKER_CLIP_SET,
                                     0, &PaperDropping, // Warten
                                     "A1", 0);
    SP_DroppingPaper.Clips[1].ReSize(1, "brief.smk", "", XY(133, 260), SPM_IDLE, CRepeat(1, 1), CPostWait(0, 0), SMACKER_CLIP_DONTCANCEL, &PaperDropping,
                                     SMACKER_CLIP_SET | SMACKER_CLIP_POST, -1, nullptr, // Warten
                                     "A9", 0);

    Talkers.Talkers[SLONG(TALKER_PERSONAL1a + PlayerNum * 2)].IncreaseReference();
    Talkers.Talkers[SLONG(TALKER_PERSONAL1b + PlayerNum * 2)].IncreaseReference();

    /*if (gLanguage==LANGUAGE_E)
      DefaultDialogPartner=TALKER_PERSONAL1b+PlayerNum*2; //Fehler bei englischer Sprachaufnahme ausgleichen
      else*/

    DefaultDialogPartner = TALKER_PERSONAL1a + PlayerNum * 2;

    Sim.Players.Players[static_cast<SLONG>(PlayerNum)].UpdatePersonalberater(3);
}

//--------------------------------------------------------------------------------------------
// Destruktor
//--------------------------------------------------------------------------------------------
CPersonal::~CPersonal() {
    NoSaboBm.Destroy();

    Talkers.Talkers[SLONG(TALKER_PERSONAL1a + PlayerNum * 2)].DecreaseReference();
    Talkers.Talkers[SLONG(TALKER_PERSONAL1b + PlayerNum * 2)].DecreaseReference();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Personalbüro message handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// void CPersonal::OnPaint()
//--------------------------------------------------------------------------------------------
void CPersonal::OnPaint() {
    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    if (PaperOnTable != -1) {
        if (SP_Frau.GetDesiredMood() == SPM_TALKING || SP_Frau.GetDesiredMood() == SPM_LISTENING) {
            KommVar = 6; // Drehen
        } else {
            KommVar = 3; // Zeitung lesen
        }

        KommVar2 = 4;
    } else {
        KommVar = -1;
        KommVar2 = -1;
    }

    if (FlugbahnType == 0) {
        if (rand() % 100 < 95) {
            FlugbahnType = 1;
        } else {
            FlugbahnType = 2;
        }

        FlugbahnCounter = AtGetTime();
    }

    if ((Sim.Players.Players[PlayerNum].SecurityFlags & 4) != 0U) {
        RoomBm.BlitFrom(NoSaboBm, 276, 0);
    }

    SP_Frau.Pump();
    SP_Mann.Pump();
    SP_DroppingPaper.Pump();
    if (Zeitung != 0) {
        RoomBm.BlitFromT(WildBm, 483, 225);
    }
    if (PaperOnTable != -1) {
        RoomBm.BlitFromT(LetterBm, 342, 224);
    }
    if (StapelBrief != 0) {
        RoomBm.BlitFromT(StapelBriefBm, 361, 292);
    }

    SP_Frau.BlitAtT(RoomBm);
    SP_Mann.BlitAtT(RoomBm);
    SP_DroppingPaper.BlitAtT(RoomBm);

    // Die Papierkugel blitten:
    if (FlugbahnType > 0) {
        XY Pos;
        SLONG Size = 0;

        if (FlugbahnType == 1) {
            Size = 5;
            Pos = XY(491 + (AtGetTime() - FlugbahnCounter) / 2, 143 - (AtGetTime() - FlugbahnCounter) / 10);
            if (Pos.x > 640) {
                FlugbahnType = -1;
            }
        } else if (FlugbahnType == 2) {
            if (AtGetTime() - FlugbahnCounter <= 400) {
                Size = 5 + (AtGetTime() - FlugbahnCounter) * 95 / 400;
                Pos = XY(491 - 166 * (AtGetTime() - FlugbahnCounter) / 400, 143 + 20 * (AtGetTime() - FlugbahnCounter) / 400);
            } else {
                Size = 100 - (AtGetTime() - FlugbahnCounter - 400) * 95 / 400;
                Pos = XY(325 - 166 * (AtGetTime() - FlugbahnCounter - 400) / 400, 163 + 225 * (AtGetTime() - FlugbahnCounter - 400) / 400);
            }
            if (Pos.x < 159) {
                FlugbahnType = -1;
            }
        }

        if (FlugbahnType > 0) {
            SDL_Rect SrcRect = {0, 0, KugelBm.Size.x, KugelBm.Size.y};
            SDL_Rect DestRect;

            DestRect.x = Pos.x - (KugelBm.Size.x / 2 * Size) / 100;
            DestRect.y = Pos.y - (KugelBm.Size.y / 2 * Size) / 100;
            DestRect.w = SLONG(KugelBm.Size.x * Size / 100);
            DestRect.h = SLONG(KugelBm.Size.y * Size / 100);

            SDL_BlitScaled(KugelBm.pBitmap->GetSurface(), &SrcRect, RoomBm.pBitmap->GetSurface(), &DestRect);
        }
    }

    // Ggf. Onscreen-Texte einbauen:
    CStdRaum::InitToolTips();

    if ((IsDialogOpen() == 0) && (MenuIsOpen() == 0)) {
        if (gMousePosition.IfIsWithin(640 - 620, 0, 640 - 457, 293)) {
            SetMouseLook(CURSOR_EXIT, 0, ROOM_PERSONAL_A, 999);
        } else if (gMousePosition.IfIsWithin(640 - 255, 145, 640 - 74, 352)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_PERSONAL_A, 10);
        } else if (gMousePosition.IfIsWithin(640 - 463, 94, 640 - 311, 248)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_PERSONAL_A, 11);
        } else if (gMousePosition.IfIsWithin(394, 378, 446, 425) && (Sim.Players.Players[PlayerNum].HasItem(ITEM_TABLETTEN) == 0)) {
            SetMouseLook(CURSOR_HOT, 0, ROOM_PERSONAL_A, 20);
        }
    }

    CStdRaum::PostPaint();
    CStdRaum::PumpToolTips();
}

//--------------------------------------------------------------------------------------------
// void CPersonal::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CPersonal::OnLButtonDown(UINT nFlags, CPoint point) {
    XY RoomPos;

    DefaultOnLButtonDown();

    if (ConvertMousePosition(point, &RoomPos) == 0) {
        return;
    }

    if (PreLButtonDown(point) == 0) {
        if (MouseClickArea == ROOM_PERSONAL_A && MouseClickId == 999) {
            Sim.Players.Players[PlayerNum].LeaveRoom();
        } else if (MouseClickArea == ROOM_PERSONAL_A && MouseClickId == 10) {
            StartDialog(TALKER_PERSONAL1a + PlayerNum * 2, MEDIUM_AIR);
        } else if (MouseClickArea == ROOM_PERSONAL_A && MouseClickId == 11) {
            StartDialog(TALKER_PERSONAL1b + PlayerNum * 2, MEDIUM_AIR);
        } else if (MouseClickArea == ROOM_PERSONAL_A && MouseClickId == 20) {
            StartDialog(TALKER_PERSONAL1a + PlayerNum * 2, MEDIUM_AIR, 20);
        } else {
            CStdRaum::OnLButtonDown(nFlags, point);
        }
    }
}

//--------------------------------------------------------------------------------------------
// void CPersonal::OnRButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CPersonal::OnRButtonDown(UINT nFlags, CPoint point) {
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

//============================================================================================
// CWorker::
//============================================================================================
// Konstruktor:
//============================================================================================
CWorkers::CWorkers(const CString &TabFilename, const CString &TabFilename2) { ReInit(TabFilename, TabFilename2); }

//--------------------------------------------------------------------------------------------
// Lädt die Tabelle von der Platte
//--------------------------------------------------------------------------------------------
void CWorkers::ReInit(const CString &TabFilename, const CString &TabFilename2) {
    // CStdioFile    Tab;
    BUFFER_V<char> Line(300);
    SLONG Num = 0;
    CString TmpStr;
    ULONG line = 0;

    // Load Table header:
    auto FileData = LoadCompleteFile(FullFilename(TabFilename, ExcelPath));
    SLONG FileP = 0;

    // Die erste Zeile einlesen
    FileP = ReadLine(FileData, FileP, Line.getData(), 300);
    line++;

    Workers.ReSize(0);
    Workers.ReSize(MAX_WORKERS);
    Num = 0;
     
    while (true) {
        if (FileP >= FileData.AnzEntries()) {
            break;
        }
        FileP = ReadLine(FileData, FileP, Line.getData(), 300);
        line++;

        TeakStrRemoveEndingCodes(Line.getData(), "\xd\xa\x1a\r");

        if (Num >= Workers.AnzEntries()) {
            TeakLibW_Exception(FNL, ExcNever);
            return;
        }
        try {
            CString name = strtok(Line.getData(), TabSeparator);
            if (name.length() == 0) {
                AT_Log("Loading", "Empty worker found in file \"%s\" at line: %d - skipping...", FullFilename(TabFilename, ExcelPath).c_str(), line);
                continue;
            }

            Workers[Num].Name = name;
            Workers[Num].Geschlecht = atoi(strtok(nullptr, TabSeparator));
            Workers[Num].Typ = atoi(strtok(nullptr, TabSeparator));
            Workers[Num].Gehalt = atoi(strtok(nullptr, TabSeparator));
            Workers[Num].Talent = atoi(strtok(nullptr, TabSeparator));
            Workers[Num].Alter = atoi(strtok(nullptr, TabSeparator));
            CString Kommentar(strtok(nullptr, ""));
            Workers[Num].Kommentar = KorrigiereUmlaute(Kommentar);
            Workers[Num].Employer = WORKER_RESERVE;
            Workers[Num].Happyness = 100;

            Workers[Num].WarnedToday = 0;
            Workers[Num].TimeInPool = -1;
            Workers[Num].OriginalGehalt = Workers[Num].Gehalt;

            if (Workers[Num].Kommentar.GetLength() > 0) {
                if (Workers[Num].Kommentar[0] == '"') {
                    Workers[Num].Kommentar = Workers[Num].Kommentar.Right(Workers[Num].Kommentar.GetLength() - 1);
                }
                if (Workers[Num].Kommentar[Workers[Num].Kommentar.GetLength() - 1] == '"') {
                    Workers[Num].Kommentar = Workers[Num].Kommentar.Left(Workers[Num].Kommentar.GetLength() - 1);
                }
            }
        } catch (...) {
            if (Num >= Workers.AnzEntries()) {
                AT_Log("Loading", "Failed to load new worker in file \"%s\" at line: %d", FullFilename(TabFilename, ExcelPath).c_str(), Num + 1);
            }
        }
        Num++;
    }

    Workers.ReSize(Num);

    // Load Table header:
    auto FileData2 = LoadCompleteFile(FullFilename(TabFilename2, ExcelPath));
    line = 0;

    FNames.ReSize(0);
    MNames.ReSize(0);
    LNames.ReSize(0);
    FNames.ReSize(40);
    MNames.ReSize(40);
    LNames.ReSize(80);

    // Die erste Zeile einlesen
    FileP = 0;
    FileP = ReadLine(FileData2, FileP, Line.getData(), 300);
    line++;

    SLONG i1 = 0;
    SLONG i2 = 0;
    SLONG i3 = 0;

    while (true) {
        if (FileP >= FileData2.AnzEntries()) {
            break;
        }
        FileP = ReadLine(FileData2, FileP, Line.getData(), 300);
        line++;

        TeakStrRemoveEndingCodes(Line.getData(), "\xd\xa\x1a\r");

        Num = atoi(strtok(Line.getData(), TabSeparator));

        if (Num == 0) {
            FNames[i1++] = strtok(nullptr, TabSeparator);
        }
        if (Num == 1) {
            MNames[i2++] = strtok(nullptr, TabSeparator);
        }
        if (Num == 2) {
            LNames[i3++] = strtok(nullptr, TabSeparator);
        }

        if (i1 >= FNames.AnzEntries() || i2 >= MNames.AnzEntries() || i3 >= LNames.AnzEntries()) {
            DebugBreak();
        }
    }

    FileData2.FillWith(0);

    FNames.ReSize(i1);
    MNames.ReSize(i2);
    LNames.ReSize(i3);
}

//--------------------------------------------------------------------------------------------
// Neuer Tag, die Leute sind etwas unglücklicher und der Arbeitsmarkt ändert sich:
//--------------------------------------------------------------------------------------------
void CWorkers::NewDay() {
    SLONG c = 0;
    SLONG m = 0;

    // Entferne Bewerber aus dem Pool, die zu lange nicht eingestellt wurden
    for (c = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == WORKER_RESERVE || Workers[c].Employer == WORKER_JOBLESS) {
            if (Workers[c].TimeInPool >= 7) {
                Workers[c].Employer = WORKER_EXPIRED;
                AT_Log("Removing %s from pool", (const char *)Workers[c].Name);
            } else if (Workers[c].TimeInPool >= 0) {
                Workers[c].TimeInPool++;
            }
        }
    }

    CheckShortageAndSort();

    TEAKRAND LocalRand(Sim.Date + Sim.StartTime);

    // Eingestellte Leute bei Mensch-Spielern unglücklich machen:
    for (c = 0; c < Workers.AnzEntries(); c++) {
        Workers[c].WarnedToday = FALSE;
        if (Workers[c].Employer >= 0 && Workers[c].Employer <= 3) {
            auto &qPlayer = Sim.Players.Players[Workers[c].Employer];

            // Worker u.U. mehrfach um 1%-Punkt unglücklicher machen
            if (qPlayer.Owner != 2) {
                if (qPlayer.Image < 500) {
                    Workers[c].Happyness--;
                }
                if (qPlayer.Image < 0) {
                    Workers[c].Happyness--;
                }
                if (qPlayer.Image < -500) {
                    Workers[c].Happyness--;
                }

                if (qPlayer.Image >= 750) {
                    Workers[c].Happyness++;
                }
            }

            // Happyness verändert sich nach Gehalt
            if (Workers[c].Gehalt > Workers[c].OriginalGehalt + LocalRand.Rand(2000)) {
                Workers[c].Happyness++;
            } else if (Workers[c].Gehalt < Workers[c].OriginalGehalt - LocalRand.Rand(2000)) {
                Workers[c].Happyness--;
            }

            if (Workers[c].Happyness > 100) {
                Workers[c].Happyness = 100;
            }

            if (Workers[c].Happyness < -100) {
                // Ihm reicht's! Er kündigt:
                if (qPlayer.Owner == 0) {
                    qPlayer.Messages.AddMessage(BERATERTYP_GIRL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2000 + Workers[c].Typ + Workers[c].Geschlecht * 100),
                                                                         Workers[c].Name.c_str()));
                }

                SLONG ExEmployer = Workers[c].Employer;
                Workers[c].Employer = WORKER_RESERVE;
                if (Workers[c].TimeInPool > 0) {
                    Workers[c].TimeInPool = 0;
                }
                Workers[c].Gehalt = Workers[c].OriginalGehalt;

                Sim.Players.Players[ExEmployer].MapWorkers(FALSE);
                break;
            }
        }
    }

    // Arbeitsmarkt reorganisieren:
    for (c = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == WORKER_JOBLESS) {
            Workers[c].Employer = WORKER_RESERVE;
        }
    }

    SLONG AnzahlBerater = 5 + LocalRand.Rand(2);
    SLONG AnzahlPiloten = 10 + LocalRand.Rand(3);
    SLONG AnzahlStewardessen = 25 + LocalRand.Rand(5);

    while (AnzahlBerater > 0 || AnzahlPiloten > 0 || AnzahlStewardessen > 0) {
        m = LocalRand.Rand(Workers.AnzEntries());

        SLONG d = 0;
        for (d = 0; d < Workers.AnzEntries(); d++) {
            SLONG idx = (m + d) % Workers.AnzEntries();
            if (Workers[idx].Employer != WORKER_RESERVE) {
                continue;
            }

            if (Workers[idx].Typ == WORKER_PILOT && AnzahlPiloten > 0) {
                AnzahlPiloten--;
                Workers[idx].Employer = WORKER_JOBLESS;
                Workers[idx].Gehalt = Workers[idx].OriginalGehalt;
                break;
            }
            if (Workers[idx].Typ == WORKER_STEWARDESS && AnzahlStewardessen > 0) {
                AnzahlStewardessen--;
                Workers[idx].Employer = WORKER_JOBLESS;
                Workers[idx].Gehalt = Workers[idx].OriginalGehalt;
                break;
            }
            if (AnzahlBerater > 0) {
                AnzahlBerater--;
                Workers[idx].Employer = WORKER_JOBLESS;
                Workers[idx].Gehalt = Workers[idx].OriginalGehalt;
                break;
            }
        }

        if (d >= Workers.AnzEntries()) {
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erhöht oder erniedrigt einer Personen das Gehalt
//--------------------------------------------------------------------------------------------
void CWorker::Gehaltsaenderung(BOOL Art) {
    if (Employer == WORKER_RESERVE || Employer == WORKER_JOBLESS || Employer == WORKER_EXPIRED) {
        return;
    }

    if (Art != 0) {
        // Gehaltserhöhung:
        SLONG OldGehalt = Gehalt;

        Gehalt += (OriginalGehalt / 100 * 10);
        if (Gehalt > 1000000) {
            Gehalt = 1000000;
        }

        if (Gehalt != OldGehalt) {
            Happyness += 20;
        }
    } else {
        // Gehaltskürzung:
        Gehalt -= (OriginalGehalt / 100 * 10);
        Happyness -= 25;

        if (Happyness < -100) {
            if (Sim.Players.Players[Employer].Owner == 0) {
                Sim.Players.Players[Employer].Messages.AddMessage(BERATERTYP_GIRL,
                                                                  bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2000 + Typ + Geschlecht * 100), Name.c_str()));
            }

            SLONG ExEmployer = Employer;

            Employer = WORKER_RESERVE;
            Gehalt = OriginalGehalt;
            Happyness = 100;
            if (TimeInPool > 0) {
                TimeInPool = 0;
            }

            Sim.Players.Players[ExEmployer].UpdateWalkSpeed();
            Sim.Players.Players[ExEmployer].MapWorkers(FALSE);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erzeugt einen zufälligen Namen
//--------------------------------------------------------------------------------------------
CString CWorkers::GetRandomName(BOOL Geschlecht) const {
    if (Geschlecht != 0) {
        return (MNames[rand() % MNames.AnzEntries()] + " " + LNames[rand() % LNames.AnzEntries()]);
    }
    return (FNames[rand() % FNames.AnzEntries()] + " " + LNames[rand() % LNames.AnzEntries()]);
}

//--------------------------------------------------------------------------------------------
// Verhindert, dass es zu wenig Piloten oder Stewardessen gibt:
//--------------------------------------------------------------------------------------------
CWorker CWorkers::createBerater(TEAKRAND &LocalRand, SLONG typ) const {
    CWorker worker;
    worker.Geschlecht = static_cast<BOOL>((LocalRand.Rand(100)) > 20);
    worker.Name = GetRandomName(worker.Geschlecht);
    worker.Typ = typ;
    worker.Gehalt = (30 + LocalRand.Rand(80)) * 100;
    worker.Talent = std::min(SLONG(100), worker.Gehalt / 200 + LocalRand.Rand(30) + 20);
    worker.Alter = (19 + LocalRand.Rand(50));
    worker.Kommentar = "";
    worker.Employer = WORKER_RESERVE;
    worker.Happyness = 100;
    worker.WarnedToday = 0;
    worker.TimeInPool = 0;
    worker.OriginalGehalt = worker.Gehalt;
    return worker;
}
CWorker CWorkers::createPilot(TEAKRAND &LocalRand) const {
    CWorker worker;
    worker.Geschlecht = static_cast<BOOL>((LocalRand.Rand(100)) > 20);
    worker.Name = GetRandomName(worker.Geschlecht);
    worker.Typ = WORKER_PILOT;
    worker.Gehalt = (30 + LocalRand.Rand(83)) * 100;
    worker.Talent = std::min(SLONG(100), worker.Gehalt / 200 + LocalRand.Rand(30) + 20);
    worker.Alter = (19 + LocalRand.Rand(50));
    worker.Kommentar = "";
    worker.Employer = WORKER_RESERVE;
    worker.Happyness = 100;
    worker.WarnedToday = 0;
    worker.TimeInPool = 0;
    worker.OriginalGehalt = worker.Gehalt;
    return worker;
}
CWorker CWorkers::createStewardess(TEAKRAND &LocalRand) const {
    CWorker worker;
    worker.Geschlecht = static_cast<BOOL>((rand() % 100) > 80);
    worker.Name = GetRandomName(worker.Geschlecht);
    worker.Typ = WORKER_STEWARDESS;
    worker.Gehalt = (30 + LocalRand.Rand(60)) * 100;
    worker.Talent = std::min(SLONG(100), worker.Gehalt * 100 / 80 / 200 + LocalRand.Rand(30) + 20);
    worker.Alter = (19 + LocalRand.Rand(40));
    worker.Kommentar = "";
    worker.Employer = WORKER_RESERVE;
    worker.Happyness = 100;
    worker.WarnedToday = 0;
    worker.TimeInPool = 0;
    worker.OriginalGehalt = worker.Gehalt;
    return worker;
}
SLONG CWorkers::AddToPool(SLONG typ, TEAKRAND &LocalRand, SLONG zielAnzahlKompetent) {
    SLONG nExpired = 0;
    SLONG anz = 0;
    SLONG anzKompetent = 0;
    for (SLONG c = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Typ != typ) {
            continue;
        }
        if (Workers[c].Employer == WORKER_EXPIRED) {
            nExpired++;
        } else if (Workers[c].Employer == WORKER_RESERVE || Workers[c].Employer == WORKER_JOBLESS) {
            if (Workers[c].Talent > 60) {
                anzKompetent++;
            }
            anz++;
        }
    }

    AT_Log("Number of expired workers: %li (Typ: %s)", nExpired, Translate_WORKER_TYPE(typ));
    AT_Log("Number of competent workers: %li / %li (Typ: %s)", anzKompetent, anz, Translate_WORKER_TYPE(typ));

    if (anzKompetent < zielAnzahlKompetent) {
        // Zielwert ist 80 kompetente Leute, aber generiere nie mehr als 10 pro Tag
        SLONG toCreate = std::min(SLONG(10), (zielAnzahlKompetent - anzKompetent));
        // Berechne, um wie viel wir die Liste vergrößern müssen. Berücksichtige, dass wir Karteileichen ersetzen können
        SLONG delta = std::max(SLONG(0), toCreate - nExpired);
        if (delta > 0) {
            Workers.ReSize(Workers.AnzEntries() + delta);
        }

        for (SLONG c = 0; c < Workers.AnzEntries() && toCreate > 0; c++) {
            BOOL isNew = static_cast<BOOL>(c >= Workers.AnzEntries() - delta);
            BOOL canReplace = static_cast<BOOL>(Workers[c].Employer == WORKER_EXPIRED && Workers[c].Typ == typ);
            if ((isNew == 0) && (canReplace == 0)) {
                continue;
            }

            if (typ >= BERATERTYP_PERSONAL && typ <= BERATERTYP_SICHERHEIT) {
                Workers[c] = createBerater(LocalRand, typ);
            } else if (typ == WORKER_STEWARDESS) {
                Workers[c] = createStewardess(LocalRand);
            } else if (typ == WORKER_PILOT) {
                Workers[c] = createPilot(LocalRand);
            } else {
                TeakLibW_Exception(FNL, ExcNever);
                return nExpired;
            }

            if (isNew != 0) {
                AT_Log("Adding new worker: %s (Typ: %s)", (const char *)Workers[c].Name, Translate_WORKER_TYPE(Workers[c].Typ));
            } else {
                AT_Log("Replacing expired worker: %s (Typ: %s)", (const char *)Workers[c].Name, Translate_WORKER_TYPE(Workers[c].Typ));
                --nExpired;
            }

            --toCreate;
        }
    }
    return nExpired;
}
void CWorkers::CheckShortageAndSort() {
    TEAKRAND LocalRand(Sim.Date + Sim.StartTime);

    AT_Log("Worker pool size: %li", Workers.AnzEntries());

    SLONG nExpired = 0;
    for (SLONG i = BERATERTYP_PERSONAL; i <= BERATERTYP_SICHERHEIT; i++) {
        nExpired += AddToPool(i, LocalRand, 5);
    }
    nExpired += AddToPool(WORKER_PILOT, LocalRand, 80);
    nExpired += AddToPool(WORKER_STEWARDESS, LocalRand, 80);

    std::sort(Workers.begin(), Workers.end());

    AT_Log("Still %li expired workers in pool", nExpired);
    if (nExpired > 100) {
        SLONG i = Workers.AnzEntries() - 1;
        while (Workers[i].Employer == WORKER_EXPIRED) {
            --i;
        }
        AT_Log("Shrinking worker pool from %li to %li", Workers.AnzEntries(), i + 1);
        Workers.ReSize(i + 1);
    }
}

//--------------------------------------------------------------------------------------------
// Erhöht oder erniedrigt allen Personen das Gehalt
//--------------------------------------------------------------------------------------------
void CWorkers::Gehaltsaenderung(BOOL Art, SLONG PlayerNum) {
    for (SLONG c = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == PlayerNum) {
            Workers[c].Gehaltsaenderung(Art);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Vergleicht Qualifikation mit Gehaltsforderung. Ist es angemessen?
//--------------------------------------------------------------------------------------------
SLONG CWorkers::GetQualityRatio(SLONG prs) {
    SLONG c = 0;
    SLONG p1 = 0;

    for (c = p1 = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Typ == Workers[prs].Typ) {
            if (abs(Workers[c].Talent - Workers[prs].Talent) < 15) {
                if (Workers[c].Gehalt > Workers[prs].Gehalt) {
                    p1++;
                } else {
                    p1--;
                }
            }
        }
    }

    return (p1);
}

//--------------------------------------------------------------------------------------------
// Stellt sicher, dass der gewünschte Berater heute im Angebot ist:
//--------------------------------------------------------------------------------------------
void CWorkers::EnsureBerater(SLONG Typ) {
    SLONG c = 0;
    SLONG m = 0;

    // Arbeitsmarkt reorganisieren:
    for (c = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == WORKER_JOBLESS && Workers[c].Typ == Typ) {
            return;
        }
    }

    TEAKRAND LocalRand(Sim.Date + Sim.StartTime);

    for (c = 0; c < 1000; c++) {
        m = LocalRand.Rand(Workers.AnzEntries());

        if (Workers[m].Employer == WORKER_RESERVE && Workers[m].Typ == Typ) {
            Workers[m].Employer = WORKER_JOBLESS;
            Workers[m].Gehalt = Workers[m].OriginalGehalt;
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
SLONG CWorkers::GetAverageHappyness(SLONG PlayerNum) {
    SLONG c = 0;
    SLONG Anz = 0;
    SLONG Value = 0;

    for (c = Anz = Value = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == PlayerNum) {
            Anz++;
            Value += Workers[c].Happyness;
        }
    }

    if (Anz != 0) {
        return (Value / Anz);
    }
    return (100);
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
SLONG CWorkers::GetMaxHappyness(SLONG PlayerNum) {
    SLONG c = 0;
    SLONG Max = -1;

    for (c = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == PlayerNum && Workers[c].Happyness > Max) {
            Max = Workers[c].Happyness;
        }
    }

    if (Max != -1) {
        return (Max);
    }
    return (100);
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
SLONG CWorkers::GetMinHappyness(SLONG PlayerNum) {
    SLONG c = 0;
    SLONG Min = 999;

    for (c = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == PlayerNum && Workers[c].Happyness < Min) {
            Min = Workers[c].Happyness;
        }
    }

    if (Min != 999) {
        return (Min);
    }
    return (100);
}

//--------------------------------------------------------------------------------------------
// Verändert die Happiness aller Worker um den angegebenen Betrag:
//--------------------------------------------------------------------------------------------
void CWorkers::AddHappiness(SLONG PlayerNum, SLONG Value) {
    SLONG c = 0;

    for (c = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == PlayerNum) {
            Workers[c].Happyness += Value;
            if (Workers[c].Happyness < -100) {
                Workers[c].Happyness = -100;
            }
            if (Workers[c].Happyness > 100) {
                Workers[c].Happyness = 100;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Berechnet die Zahl der Berater-Bewerbungen:
//--------------------------------------------------------------------------------------------
SLONG CWorkers::GetNumJoblessBerater() {
    SLONG c = 0;
    SLONG d = 0;

    for (c = d = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == WORKER_JOBLESS) {
            if (Workers[c].Typ < WORKER_PILOT) {
                d++;
            }
        }
    }

    return (d);
}

//--------------------------------------------------------------------------------------------
// Berechnet die Zahl der Piloten-Bewerbungen:
//--------------------------------------------------------------------------------------------
SLONG CWorkers::GetNumJoblessPiloten() {
    SLONG c = 0;
    SLONG d = 0;

    for (c = d = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == WORKER_JOBLESS) {
            if (Workers[c].Typ == WORKER_PILOT) {
                d++;
            }
        }
    }

    return (d);
}

//--------------------------------------------------------------------------------------------
// Berechnet die Zahl der Flugbegleiter-Bewerbungen
//--------------------------------------------------------------------------------------------
SLONG CWorkers::GetNumJoblessFlugbegleiter() {
    SLONG c = 0;
    SLONG d = 0;

    for (c = d = 0; c < Workers.AnzEntries(); c++) {
        if (Workers[c].Employer == WORKER_JOBLESS) {
            if (Workers[c].Typ == WORKER_STEWARDESS) {
                d++;
            }
        }
    }

    return (d);
}

//--------------------------------------------------------------------------------------------
// Speichert ein Worker-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CWorker &Worker) {
    File << Worker.Name;
    File << Worker.Geschlecht;
    File << Worker.Typ;
    File << Worker.Gehalt;
    File << Worker.OriginalGehalt;
    File << Worker.Talent;
    File << Worker.Alter;
    File << Worker.Kommentar;
    File << Worker.Employer;
    File << Worker.PlaneId;
    File << Worker.Happyness;
    File << Worker.WarnedToday;
    File << Worker.TimeInPool;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein Worker-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CWorker &Worker) {
    File >> Worker.Name;
    File >> Worker.Geschlecht;
    File >> Worker.Typ;
    File >> Worker.Gehalt;
    File >> Worker.OriginalGehalt;
    File >> Worker.Talent;
    File >> Worker.Alter;
    File >> Worker.Kommentar;
    File >> Worker.Employer;
    File >> Worker.PlaneId;
    File >> Worker.Happyness;
    File >> Worker.WarnedToday;

    if (SaveVersionSub >= 200) {
        File >> Worker.TimeInPool;
    }

    return (File);
}

//--------------------------------------------------------------------------------------------
// Speichert ein Workers-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CWorkers &Workers) {
    File << Workers.Workers;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein Workers-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CWorkers &Workers) {
    File >> Workers.Workers;

    /*SLONG c,NewAnz;
      ULONG Tmp;

      File >> NewAnz;

      Workers.Workers.ReSize (NewAnz);

      File >> Tmp;
      for (c=0; c<NewAnz; c++)
      File >> Workers.Workers[c]; */

    return (File);
}
