//============================================================================================
// TakeOff.cpp : Defines the class behaviors for the application.
//============================================================================================
// Link: "Takeoff.h"
//============================================================================================
#include "StdAfx.h"
#include "Abend.h"
#include "ArabAir.h"
#include "Aufsicht.h"
#include "Bank.h"
#include "Buero.h"
#include "Checkup.h"
#include "Credits.h"
#include "Designer.h"
#include "DutyFree.h"
#include "Editor.h"
#include "Fracht.h"
#include "HLine.h"
#include "Insel.h"
#include "Kiosk.h"
#include "Makler.h"
#include "Museum.h"
#include "Nasa.h"
#include "NewGamePopup.h" //Fenster zum Wahl der Gegner und der Spielstärke
#include "CVideo.h"
#include "Intro.h"
#include "Outro.h"
#include "PlanProp.h"
#include "Reise.h"
#include "Ricks.h"
#include "RouteBox.h"
#include "Rushmore.h"
#include "Sabotage.h"
#include "Security.h"
#include "Statistk.h"
#include "Tafel.h"
#include "TitlePopup.h"
#include "WeltAll.h"
#include "Werbung.h"
#include "World.h"
#include <cstdio>
#include <ctime>

#include "gltitel.h"

#include "AtNet.h"
#include "SbLib.h"
class TeakLibException;
extern SBNetwork gNetwork;

#include <filesystem>
#include <fstream>

#ifdef SENTRY
#include "sentry.h"
#endif

CHLPool HLPool;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void Unvideo(const CString &Filename, const CString &TargetFilename);

extern SLONG bCAbendOpen;
extern SLONG SkipPlaneCalculation;
extern SLONG TankSize[];
extern SLONG TankPrice[];

static CString PlaneSounds[] = {"prop.raw", "flyby.raw", "flyby2.raw", "flyby3.raw", "flyby4.raw", "flyby5.raw"};

BOOL gSpawnOnly = TRUE;

extern char chRegKey[];

SLONG gLoadGameNumber = -1;

CTakeOffApp *pTakeOffApp;

/*
#define SND_TYPE unsigned char
void CompressWave(BUFFER_V<SND_TYPE> &Input, BUFFER_V<SND_TYPE> &Output);
void DecompressWave(BUFFER_V<SND_TYPE> &Input, BUFFER_V<SND_TYPE> &Output);
*/

CString MakeVideoPath, MakeVideoPath2;
BOOL MakeUnvideoOn555 = FALSE;

SLONG gTimerCorrection = 0; // Is it necessary to adapt the local clock to the server clock?

// StackSaver MySaver;

char *UCharToReadableAnsi(const unsigned char *pData, unsigned uLen);
unsigned char *ReadableAnsiToUChar(const char *pData, unsigned uLen);

#ifdef __cplusplus
extern "C"
#endif

int main(int argc, char *argv[]) {
#ifdef SENTRY
    const bool disableSentry = DoesFileExist("no-sentry");

    if (!disableSentry) {
        sentry_options_t* options = sentry_options_new();
        sentry_options_set_dsn(options, "https://6c9b29cfe559442b98417942e221250d@o4503905572225024.ingest.sentry.io/4503905573797888");
        // This is also the default-path. For further information and recommendations:
        // https://docs.sentry.io/platforms/native/configuration/options/#database-path
        sentry_options_set_database_path(options, ".sentry-native");
        sentry_options_set_release(options, VersionString);
        sentry_options_set_debug(options, 0);
        sentry_options_add_attachment(options, "debug.txt");

        srand(time(nullptr));
        int crashId = rand() % 1000 + rand() % 1000 * 1000;

        sentry_options_set_on_crash(
            options,
            [](const sentry_ucontext_t *uctx, sentry_value_t event, void *closure) -> sentry_value_t {
            TeakLibException *e = GetLastException();
            if (e != nullptr) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "AT - Exception", e->what(), nullptr);
            }

            const std::string id = std::to_string(*static_cast<int *>(closure));
            const std::string msg = std::string("Airline Tycoon experienced an unexpected exception\nPress OK to send crash information to sentry\nPress Abort to not send the crash to sentry\n\nCustom Crash ID is: ") + id;
            AT_Log_I("CRASH", msg);
            std::filesystem::copy_file("debug.txt", "crash-" + id + ".txt");
            if (AbortMessageBox(MESSAGEBOX_ERROR, "Airline Tycoon Deluxe Crash Handler", msg.c_str(), nullptr)) {
                return sentry_value_new_null(); // Skip
            }
            
            return event;
        }, &crashId);
        sentry_init(options);

        sentry_set_tag("Crash ID", std::to_string(crashId).c_str());
    }
#endif

#ifdef TEST
    if (!run_regression()) {
        hprintf("Regression test failed!");
        return 1;
    }
#endif
#ifndef SENTRY
    try {
        theApp.InitInstance(argc, argv);
    } catch (TeakLibException &e) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "AT - Exception", e.what(), nullptr);
        
        throw;
    }
#else
    theApp.InitInstance(argc, argv);
#endif


#ifdef SENTRY
    if (!disableSentry) {
        sentry_close();
    }
#endif

        return 0;
}

//--------------------------------------------------------------------------------------------
// CTakeOffApp construction:
//--------------------------------------------------------------------------------------------
CTakeOffApp::CTakeOffApp() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    }

    if (TTF_Init() < 0) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
    }

    if (Mix_Init(MIX_INIT_OGG) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", TTF_GetError());
    }
}

//--------------------------------------------------------------------------------------------
// CTakeOffApp deconstruction:
//--------------------------------------------------------------------------------------------
CTakeOffApp::~CTakeOffApp() {
    if (gpClickFx != nullptr) {
        gpClickFx->Release();
    }
    if (gpPlaneFx != nullptr) {
        gpPlaneFx->Release();
    }

    if (gpSSE != nullptr) {
        gpSSE->EnableSound(false);
        delete gpSSE;
        gpSSE = nullptr;
    }

    bLeaveGameLoop = TRUE;

    gNetwork.DisConnect();
}

#define LOADING_TEXT(text)                                                                                                                                     \
    {                                                                                                                                                          \
        PrimaryBm.BlitFrom(TitleBitmap);                                                                                                                       \
        FontBigWhite.DrawTextBlock(&PrimaryBm.PrimaryBm, 2, 450 + 8, 640, 480, text);                                                                          \
        FrameWnd->Invalidate();                                                                                                                                \
        MessagePump();                                                                                                                                         \
        PrimaryBm.BlitFrom(TitleBitmap);                                                                                                                       \
        FontBigWhite.DrawTextBlock(&PrimaryBm.PrimaryBm, 2, 450 + 8, 640, 480, text);                                                                          \
        FrameWnd->Invalidate();                                                                                                                                \
        MessagePump();                                                                                                                                         \
    }

//--------------------------------------------------------------------------------------------
// CTakeOffApp initialization:
//--------------------------------------------------------------------------------------------
BOOL CTakeOffApp::InitInstance(int argc, char *argv[]) {

    char localVersionString[80];
    strcpy(localVersionString, VersionString);

    // Hdu.Disable();
    time_t t = time(nullptr);
    Hdu.HercPrintf(0, "Airline Tycoon Deluxe logfile");
    Hdu.HercPrintf(0, VersionString);
    Hdu.HercPrintf(0, "===============================================================================");
    Hdu.HercPrintf(0, "Copyright (C) 2002 Spellbound Software");
    Hdu.HercPrintf(0, "TakeOff.Cpp was compiled at %s at %s", __DATE__, __TIME__);
    Hdu.HercPrintf(0, "===============================================================================");
    Hdu.HercPrintf(0, "logging starts %s", asctime(localtime(&t)));

    pTakeOffApp = this;

    // Initialisierung:
    TopWin = nullptr;
    bFullscreen = TRUE;
    bCursorCaptured = FALSE;
    gMouseStartup = TRUE;

    // Die Standardsprachen:
    //#define LANGUAGE_D       0             //D-Deutsch, inklusive
    //#define LANGUAGE_E       1             //E-Englisch, bezahlt
    //#define LANGUAGE_F       2             //F-Französisch, bezahlt
    //#define LANGUAGE_T       3             //T-Taiwanesisch, gilt als englische
    //#define LANGUAGE_P       4             //P-Polnisch, inklusive
    //#define LANGUAGE_N       5             //N-Niederländisch, bezahlt
    //#define LANGUAGE_I       6             //I-Italienisch, bezahlt
    //#define LANGUAGE_S       7             //S-Spanisch, bezahlt
    //#define LANGUAGE_O       8             //O-Portugisisch, bezahlt
    //#define LANGUAGE_B       9             //B-Brasiliasnisch, nicht von mir
    //#define LANGUAGE_1      10             //J-Tschechisch
    //#define LANGUAGE_2      11             //K-noch frei
    //#define LANGUAGE_3      12             //L-noch frei
    //#define LANGUAGE_4      13             //M-noch frei
    //#define LANGUAGE_5      14             //N-noch frei
    //#define LANGUAGE_6      15             //Q-noch frei
    //#define LANGUAGE_7      16             //R-noch frei
    //#define LANGUAGE_8      17             //T-noch frei
    //#define LANGUAGE_9      18             //U-noch frei
    //#define LANGUAGE_10     19             //V-noch frei

    DoAppPath();
    gLanguage = LANGUAGE_E;
    std::ifstream ifil = std::ifstream(AppPath + "misc/sabbel.dat");
    if (ifil.is_open()) {
        ifil.read(reinterpret_cast<char *>(&gLanguage), sizeof(gLanguage));
        ifil.close();
    }

    // gUpdatingPools = TRUE; //Zum testen; für Release auskommentieren

    // Flag-Ersatzstücke aus der Registry lesen:
    {
        CRegistryAccess reg(chRegKey);

        SLONG bConfigNoVgaRam = 0;
        SLONG bConfigNoSpeedyMouse = 0;
        SLONG bConfigWinMouse = 0;
        SLONG bConfigNoDigiSound = 0;

        reg.ReadRegistryKey_l(bConfigNoVgaRam);
        reg.ReadRegistryKey_l(bConfigNoSpeedyMouse);
        reg.ReadRegistryKey_l(bConfigWinMouse);
        reg.ReadRegistryKey_l(bConfigNoDigiSound);

        if (bConfigNoVgaRam != 0) {
            bNoVgaRam = TRUE;
        }
        if (bConfigNoSpeedyMouse != 0) {
            bNoQuickMouse = TRUE;
        }
        if (bConfigWinMouse != 0) {
            gUseWindowsMouse = TRUE;
        }
        if (bConfigNoDigiSound != 0) {
            Sim.Options.OptionDigiSound = FALSE;
        }
    }

    // Schneller Mode zum Debuggen?
    for (int i = 0; i < argc; i++) {
        char *Argument = argv[i];

        if (stricmp(Argument, "/fc") == 0) {
            bFirstClass = TRUE;
        }
        if (stricmp(Argument, "/p") == 0 || stricmp(Argument, "-p") == 0 || stricmp(Argument, "p") == 0) {
            return (FALSE);
        }

        // if (stricmp (Argument, "/e")==0) gLanguage = LANGUAGE_E;
        // if (stricmp (Argument, "/quick")==0) bQuick = TRUE;
        // if (stricmp (Argument, "/fast")==0) bQuick = TRUE;
        // if (stricmp (Argument, "/d")==0) gLanguage = LANGUAGE_D;
        // if (stricmp (Argument, "/f")==0) gLanguage = LANGUAGE_F;
        // if (stricmp (Argument, "/test")==0) bTest = TRUE;
        if (stricmp(Argument, "/window") == 0) {
            bFullscreen = FALSE;
        }
        if (stricmp(Argument, "/savedata") == 0) {
            CRLEReader::TogglePlainTextSaving(true);
        }
        // if (stricmp (Argument, "/windowed")==0) bFullscreen = FALSE;

        if (stricmp(Argument, "/novgaram") == 0) {
            bNoVgaRam = TRUE;
        }
        if (stricmp(Argument, "/noquickmouse") == 0) {
            bNoQuickMouse = TRUE;
        }
        if (stricmp(Argument, "/nodigisound") == 0) {
            Sim.Options.OptionDigiSound = FALSE;
        }
        if (stricmp(Argument, "/nospeedybar") == 0) {
            bNoSpeedyBar = TRUE;
        }
        if (stricmp(Argument, "/winmouse") == 0) {
            gUseWindowsMouse = TRUE;
        }
        if (stricmp(Argument, "/showallpools") == 0) {
            gShowAllPools = TRUE;
        }
        if (stricmp(Argument, "/load") == 0) {
            gLoadGameNumber = atoi(strtok(nullptr, " "));
        }
        if (stricmp(Argument, "/savegamelocal") == 0) {
            SavegamePath = "d:\\Savegame\\%s";
        }

        if (stricmp(Argument, "/useclangli") == 0) {
            gUpdatingPools = TRUE;
        }
        if (stricmp(Argument, "/555") == 0) {
            MakeUnvideoOn555 = TRUE;
        }

        if (stricmp(Argument, "/video") == 0) {
            MakeVideoPath = strtok(nullptr, " ");
            bNoVgaRam = TRUE;
        }
        if (stricmp(Argument, "/unvideo") == 0) {
            MakeVideoPath = CString(":") + strtok(nullptr, " ");
            MakeVideoPath2 = strtok(nullptr, " ");
        }
        if (stricmp(Argument, "/updatepools") == 0) {
            InitPathVars();

            FrameWnd = new GameFrame;

            gUpdatingPools = TRUE;

            pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("glclan.gli", GliPath)), &pGLibClan, L_LOCMEM);
            pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("glstd.gli", GliPath)), &pGLibStd, L_LOCMEM);
            Clans.ReInit("clan.csv");
            Clans.LoadBitmaps();

            UpdateHLinePool();
            exit(0);
        }
    }

    Sim.Options.ReadOptions();

    InitPathVars();
    // UpdateSavegames ();

    bFirstClass |=
        static_cast<SLONG>((DoesFileExist(FullFilename("builds.csv", ExcelPath)) == 0) && (DoesFileExist(FullFilename("relation.csv", ExcelPath))) == 0);

    FrameWnd = new GameFrame;

    if ((MakeVideoPath.GetLength() != 0) && MakeVideoPath[0] == ':') {
        Unvideo(MakeVideoPath.Mid(1), MakeVideoPath2);
        exit(-1);
    }

    PrimaryBm.Clear();
    FrameWnd->Invalidate();
    MessagePump();
    PrimaryBm.Clear();
    FrameWnd->Invalidate();
    MessagePump();

    MessagePump();
    InitFonts();

    // Waiting Area
    {
        CWait Waiting;
        GfxLib *pRoomLib = nullptr;
        GfxLib *pRoomLib2 = nullptr;
        SBBM TitleBitmap;

        try {
            pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("titel.gli", RoomPath)), &pRoomLib, L_LOCMEM);
            pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("titel2.gli", RoomPath)), &pRoomLib2, L_LOCMEM);
        } catch (TeakLibException &e) {
            if (pRoomLib == nullptr && pRoomLib2 == nullptr) {
                throw;
            }

            e.caught();
            //title2.gli is not present in every installation of ATD
        }

        if (Sim.Options.OptionDigiSound == TRUE) {
            InitSoundSystem(FrameWnd->m_hWnd);
        }

        if (Sim.Options.OptionViewedIntro == 0 && IntroPath.GetLength() != 0) {
            Sim.Gamestate = GAMESTATE_INTRO | GAMESTATE_WORKING;
            TopWin = new CIntro();
            TitleBitmap.ReSize(pRoomLib, GFX_TITEL);

            while (Sim.Gamestate != GAMESTATE_BOOT) {
                FrameWnd->Invalidate();
                MessagePump();
                SDL_Delay(10);
            }

            delete TopWin;
            TopWin = nullptr;
        } else {
            TitleBitmap.ReSize(pRoomLib, GFX_SPELLOGO);
        }

        PrimaryBm.BlitFrom(TitleBitmap);
        gMousePosition = XY(600, 440);
        FrameWnd->Invalidate();
        MessagePump();
        PrimaryBm.BlitFrom(TitleBitmap);
        gMousePosition = XY(600, 440);
        FrameWnd->Invalidate();
        MessagePump();
        PrimaryBm.BlitFrom(TitleBitmap);
        gMousePosition = XY(600, 440);
        FrameWnd->Invalidate();
        MessagePump();

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Karakters worden opgestart...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Loading People...");

        if (gUpdatingPools != 0) {
            pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("glclan.gli", GliPath)), &pGLibClan, L_LOCMEM);
        }

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Adviseurs worden opgestart...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Loading Advisors...");
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("glberatr.gli", GliPath)), &pGLibBerater, L_LOCMEM);

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Verscheidene afbeeldingen worden opgestart...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Loading miscellanous grafix...");

        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("glstd.gli", GliPath)), &pGLibStd, L_LOCMEM);

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Vliegtuigen worden opgestart...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Loading planes...");
        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("glplanes.gli", GliPath)), &pGLibPlanes, L_LOCMEM);

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Dialoogteksten worden opgestart...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Loading dialogue texts...");
        DialogTexte.Open(FullFilename("dlg_ger.res", MiscPath), TEXTRES_CACHED);

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Verscheidene teksten worden opgestart...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Loading miscellanous texts...");
        StandardTexte.Open(FullFilename("std_ger.res", MiscPath), TEXTRES_CACHED);
        LadeWeitereStandardTexte();

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Eenheidteksten worden opgestart...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Loading unit texts...");
        InitEinheiten(FullFilename("ein_ger.res", MiscPath));

        // Großes und kleines Icon setzen:
        // FrameWnd->SetIcon (m_hBigIcon = LoadIcon(IDR_MAINFRAME1), 1);
        // FrameWnd->SetIcon (m_hSmallIcon = LoadIcon(IDR_MAINFRAME), 0);

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseren...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing...");
        InitItems();

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseert de globe...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing globe...");
        InitGlobeMapper();

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseert statusbalk...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing status bar...");
        InitStatusLines();

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseert scherm...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing screen...");

        TitleBitmap.ReSize(pRoomLib, GFX_TITEL);
        PrimaryBm.BlitFrom(TitleBitmap);
        gMousePosition = XY(600, 440);
        FrameWnd->Invalidate();
        MessagePump();
        PrimaryBm.BlitFrom(TitleBitmap);
        gMousePosition = XY(600, 440);
        FrameWnd->Invalidate();
        MessagePump();
        PrimaryBm.BlitFrom(TitleBitmap);
        gMousePosition = XY(600, 440);
        FrameWnd->Invalidate();
        MessagePump();

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Zoekt midi-apparaat...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Looking for midi device...");

        // Hamma Midi?
        FrameWnd->Invalidate();
        MessagePump(); // lpDD->FlipToGDISurface();
        bMidiAvailable = IsMidiAvailable();

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseert geluidssysteem...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing music sound system...");

        if ((bMidiAvailable != 0) && (gpSSE != nullptr)) {
            FrameWnd->Invalidate();
            MessagePump(); // lpDD->FlipToGDISurface();
            gpSSE->CreateMidi(&gpMidi);

            if (gpMidi != nullptr) {
                if (gLanguage == LANGUAGE_N)
                    LOADING_TEXT("Stelt het midi-volume in...")
                else if (gLanguage == LANGUAGE_F)
                    LOADING_TEXT("Francais...")
                else
                    LOADING_TEXT("Setting midi volume...");

                FrameWnd->Invalidate();
                MessagePump(); // lpDD->FlipToGDISurface();
                SetMidiVolume(Sim.Options.OptionMusik);
                gpMidi->SetMode(Sim.Options.OptionMusicType);

                if (Sim.Options.OptionViewedIntro != 0) {
                    if (gLanguage == LANGUAGE_N)
                        LOADING_TEXT("Start de eerste midi...")
                    else if (gLanguage == LANGUAGE_F)
                        LOADING_TEXT("Francais...")
                    else
                        LOADING_TEXT("Starting first midi...");

                    FrameWnd->Invalidate();
                    MessagePump(); // lpDD->FlipToGDISurface();
                    NextMidi();

                    if (gLanguage == LANGUAGE_N)
                        LOADING_TEXT("Herstelt het midi-volume...")
                    else if (gLanguage == LANGUAGE_F)
                        LOADING_TEXT("Francais...")
                    else
                        LOADING_TEXT("Resetting midi volume...");

                    FrameWnd->Invalidate();
                    MessagePump(); // lpDD->FlipToGDISurface();
                    SetMidiVolume(Sim.Options.OptionMusik);
                }
            } else {
                bMidiAvailable = FALSE;
            }
        }

        // Registration.ReSize ("Misc\\Register.res", 0x54a8fe83);

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseert de karakterdata...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing people data...");
        Clans.ReInit("clan.csv");
        Clans.UpdateClansInGame(TRUE);

        if ((pRoomLib2 != nullptr) && gLanguage != LANGUAGE_N) {
            TitleBitmap.ReSize(pRoomLib2, GFX_TITEL);
            PrimaryBm.BlitFrom(TitleBitmap);
            gMousePosition = XY(600, 440);
            FrameWnd->Invalidate();
            MessagePump();
            PrimaryBm.BlitFrom(TitleBitmap);
            gMousePosition = XY(600, 440);
            FrameWnd->Invalidate();
            MessagePump();
            PrimaryBm.BlitFrom(TitleBitmap);
            gMousePosition = XY(600, 440);
            FrameWnd->Invalidate();
            MessagePump();
        }

        if (gUpdatingPools == 0) {
            if (gLanguage == LANGUAGE_N)
                LOADING_TEXT("Initialiseert de r\xF6ntgenfoto's...")
            else if (gLanguage == LANGUAGE_F)
                LOADING_TEXT("Francais...")
            else
                LOADING_TEXT("Initializing roentgens...");
            SkelettPool.ReSize("skelett.pol", nullptr, nullptr);
            SkelettPool.Load();

            SLONG n = 0;

            for (SLONG c = Clans.AnzEntries() - 1; c >= 0; c--, n++) {
                if (Clans.IsInAlbum(c) != 0) {
                    if (c < SLONG(Clans.AnzEntries() - 1) && (Clans.IsInAlbum(c + 1) != 0) && Clans[c].PalFilename == Clans[SLONG(c + 1)].PalFilename &&
                        Clans[c].Type != 30) {
                        Clans[c].ClanPool.ReSize(bprintf("clan%li.pol", c), &Clans[SLONG(c + 1)].ClanPool, nullptr);
                        Clans[c].ClanGimmick.ReSize(bprintf("clang%li.pol", c), &Clans[SLONG(c + 1)].ClanPool, &Clans[c].ClanPool);
                        Clans[c].ClanWarteGimmick.ReSize(bprintf("clanw%li.pol", c), &Clans[SLONG(c + 1)].ClanPool, &Clans[c].ClanPool);
                    } else {
                        Clans[c].ClanPool.ReSize(bprintf("clan%li.pol", c), nullptr, nullptr);
                        Clans[c].ClanGimmick.ReSize(bprintf("clang%li.pol", c), &Clans[c].ClanPool, nullptr);
                        Clans[c].ClanWarteGimmick.ReSize(bprintf("clanw%li.pol", c), &Clans[c].ClanPool, &Clans[c].ClanGimmick);
                    }

                    Clans[c].ClanWarteGimmick.PreLoad();
                    Clans[c].ClanGimmick.PreLoad();

                    if (Clans[c].TodayInGame != 0) {
                        Clans[c].ClanPool.Load();
                    } else {
                        Clans[c].ClanPool.PreLoad();
                    }

                    if (gLanguage == LANGUAGE_N)
                        LOADING_TEXT((LPCTSTR)(CString("Karakterdata wordt opgestart...") +
                                               CString("................................................................").Left(n / 4)))
                    else if (gLanguage == LANGUAGE_F)
                        LOADING_TEXT((LPCTSTR)(CString("Initializing people data...") +
                                               CString("................................................................").Left(n / 4)))
                    else
                        LOADING_TEXT((LPCTSTR)(CString("Initializing people data...") +
                                               CString("................................................................").Left(n / 4)));
                    n++;
                }
            }
        }

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Karakterdata wordt opgestart...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Loading people data...");
        Clans.LoadBitmaps();

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseert de steden...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing cities...");
        Cities.ReInit("city.csv");

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseert de vliegtuigen...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing planes...");
        PlaneTypes.ReInit("planetyp.csv");

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseert de namen...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing names...");
        PlaneNames.ReInit("pnames.csv");

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseert de kranten...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing newspapers...");
        Sim.Headlines.ReInit("stdpaper.csv");

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Initialiseert de tips...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Francais...")
        else
            LOADING_TEXT("Initializing tooltips...");
        InitTipBms();

        if (gLanguage == LANGUAGE_N)
            LOADING_TEXT("Spel wordt gestart...")
        else if (gLanguage == LANGUAGE_F)
            LOADING_TEXT("Lancer le jeu...")
        else
            LOADING_TEXT("Starting game...");

        TitleBitmap.Destroy();
        if ((pRoomLib != nullptr) && (pGfxMain != nullptr)) {
            pGfxMain->ReleaseLib(pRoomLib);
        }
        if ((pRoomLib2 != nullptr) && (pGfxMain != nullptr)) {
            pGfxMain->ReleaseLib(pRoomLib2);
        }
    }

    gMouseStartup = FALSE;
    pCursor->SetImage(gCursorBm.pBitmap);
    pCursor->Show(TRUE);

    PrimaryBm.SetVSync(TRUE);

    GameLoop(nullptr);

    if (FrameWnd != nullptr) {
        delete FrameWnd;
        FrameWnd = nullptr;
    }
    return FALSE;
}

//--------------------------------------------------------------------------------------------
// Sorgt für Screen Refresh und für Ablauf der Simulation:
//--------------------------------------------------------------------------------------------
void CTakeOffApp::GameLoop(void * /*unused*/) {
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;
    DWORD LastTime = 0xffffffff;
    DWORD Time = 0;
    DWORD NumSimSteps = 0;
    SLONG Faktor = 1;
    BOOL RefreshNeccessary = FALSE;

    DWORD SimStepsCounter = 0; // Zählt wieviele SimSteps an einem Stück gemacht wurden um ab&zu einen ScreenRefresh zu erzwingen

    Sim.TimeSlice = 0;

    while (bLeaveGameLoop == 0) {
        Time = SDL_GetTicks();

        if (LastTime == 0xffffffff || (bgJustDidLotsOfWork != 0) || bActive == FALSE) {
            LastTime = Time;
        }

        bgJustDidLotsOfWork = FALSE;

        if (bActive != 0) {
            Faktor = 1;
            RefreshNeccessary = FALSE;
            if (((Time - LastTime) / 50) > 0) {
                DWORD tmp = 0;

                tmp = (Time - LastTime) / 50; // 20 Schritte pro Sekunde
                NumSimSteps += tmp;
                LastTime += 50 * tmp;
            }

            if (Sim.Gamestate == GAMESTATE_BOOT) {
                delete TopWin;
                TopWin = nullptr;

                for (SLONG c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
                    if (Sim.Players.Players[c].LocationWin != nullptr) {
                        delete Sim.Players.Players[c].LocationWin;
                        Sim.Players.Players[c].LocationWin = nullptr;
                    }
                    if (Sim.Players.Players[c].DialogWin != nullptr) {
                        delete Sim.Players.Players[c].DialogWin;
                        Sim.Players.Players[c].DialogWin = nullptr;
                    }
                }

                // Titelmenü anzeigen:
                if ((Sim.Options.OptionViewedIntro != 0) || IntroPath.GetLength() == 0) {
                    Sim.Gamestate = GAMESTATE_TITLE | GAMESTATE_WORKING;
                    TopWin = new TitlePopup(FALSE, 0);
                } else {
                    Sim.Gamestate = GAMESTATE_INTRO | GAMESTATE_WORKING;
                    TopWin = new CIntro();
                }
            }

            if (Sim.Gamestate == (GAMESTATE_TITLE | GAMESTATE_DONE)) {
                // New Game -> Abfragen für Spielmodus:
                CStdRaum *TmpWin = TopWin;
                TopWin = nullptr;
                delete TmpWin;

                Sim.Gamestate = GAMESTATE_INIT | GAMESTATE_WORKING;
                TopWin = new NewGamePopup(FALSE, 0);
            } else if (Sim.Gamestate == (GAMESTATE_CREDITS)) {
                CStdRaum *TmpWin = TopWin;
                TopWin = nullptr;
                delete TmpWin;
                Sim.Gamestate = GAMESTATE_INIT | GAMESTATE_WORKING;
                TopWin = new CCredits(FALSE, 0);
            } else if (Sim.Gamestate == (GAMESTATE_OPTIONS)) {
                CStdRaum *TmpWin = TopWin;
                TopWin = nullptr;
                delete TmpWin;
                Sim.Gamestate = GAMESTATE_INIT | GAMESTATE_WORKING;
                TopWin = new Options(FALSE, 0);
            } else if (Sim.Gamestate == GAMESTATE_INTRO) {
                CStdRaum *TmpWin = TopWin;
                TopWin = nullptr;
                delete TmpWin;
                Sim.Gamestate = GAMESTATE_INTRO | GAMESTATE_WORKING;
                TopWin = new CIntro();
            } else if (Sim.Gamestate == GAMESTATE_OUTRO || Sim.Gamestate == GAMESTATE_OUTRO2) {
                for (SLONG c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
                    if (Sim.Players.Players[c].LocationWin != nullptr) {
                        delete Sim.Players.Players[c].LocationWin;
                        Sim.Players.Players[c].LocationWin = nullptr;
                    }
                    if (Sim.Players.Players[c].DialogWin != nullptr) {
                        delete Sim.Players.Players[c].DialogWin;
                        Sim.Players.Players[c].DialogWin = nullptr;
                    }
                }

                CStdRaum *TmpWin = TopWin;
                TopWin = nullptr;
                delete TmpWin;
                Sim.Gamestate |= GAMESTATE_WORKING;

                if ((Sim.Gamestate & (~GAMESTATE_WORKING)) == GAMESTATE_OUTRO) {
                    TopWin = new COutro("outro.smk");
                } else {
                    TopWin = new COutro("outro2.smk");
                }
            } else if (Sim.Gamestate == (GAMESTATE_INIT | GAMESTATE_DONE)) {
                // Das Spielfenster mit Flughafensicht
                CStdRaum *TmpWin = TopWin;
                TopWin = nullptr;
                delete TmpWin;

                if (gLoadGameNumber > -1) {
                    Sim.LoadGame(gLoadGameNumber - 1);
                }

                if (gLoadGameNumber == -1) {
                    for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                        Sim.Players.Players[c].EnterRoom(ROOM_AIRPORT, true);
                    }

                    UpdateStatusBar();
                    Sim.Gamestate = GAMESTATE_PLAYING | GAMESTATE_WORKING;
                    Sim.DayState = 1;

                    if (1 == 0) {
                        // Speedup zum testen; für Release beides auskommentieren:
                        Sim.IsTutorial = FALSE;
                        Sim.bNoTime = FALSE;
                        Sim.DayState = 2;
                    } else {
                        if (Sim.Difficulty == DIFF_TUTORIAL) {
                            for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                                Sim.Players.Players[c].Blocks.ClearAlbum();
                            }
                            Sim.IsTutorial = TRUE;
                        }
                        MouseWait++;
                    }

                    NumSimSteps = 0; // Noch nicht mit SimSteps beginnen

                    for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                        Sim.Players.Players[c].WalkToRoom(ROOM_AUFSICHT);
                    }
                }

                gLoadGameNumber = -1;
            }

            if (Sim.Gamestate == (GAMESTATE_PLAYING | GAMESTATE_WORKING)) {
                // Feierabend berechnen:
                if (Sim.bNetwork != 0) {
                    if (Sim.CallItADay == FALSE && (Sim.bIsHost != 0) && Sim.CallItADayAt == 0) {
                        Sim.CallItADay = TRUE;
                        for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                            Sim.CallItADay &= (Sim.Players.Players[c].CallItADay | static_cast<SLONG>(Sim.Players.Players[c].Owner == 1));

                            if (Sim.Players.Players[c].IsOut == 0 && Sim.Players.Players[c].Owner == 1 && Sim.Players.Players[c].WaitWorkTill != -1) {
                                Sim.CallItADay = FALSE;
                            }
                        }

                        if (Sim.CallItADay != 0) {
                            if ((Sim.Options.OptionAutosave != 0) && (Sim.bNetwork != 0)) {
                                Sim.SaveGame(11, StandardTexte.GetS(TOKEN_MISC, 5000));
                            }

                            Sim.CallItADayAt = Sim.TimeSlice + 30;
                            SIM::SendSimpleMessage(ATNET_DAYFINISHALL, 0, Sim.CallItADayAt);
                            Sim.CallItADay = 0;
                        }
                    }
                } else {
                    Sim.CallItADay = Sim.Players.Players[Sim.localPlayer].CallItADay;
                }

                // Dinge, die nicht beschleunigt werden:
                if (nOptionsOpen == 0 && nWaitingForPlayer == 0 && (Sim.bPause == 0) && (Sim.CallItADay == 0) && Sim.Time < 18 * 60000 &&
                    Sim.Time >= 9 * 60000) {
                    for (c = min(NumSimSteps, 400); c > 0; c--) {
                        Sim.Players.MessagePump();
                    }
                }

                for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                    // Klickt gerade jemand ins Zeit-Fenster?
                    if ((Sim.CallItADay == 0) && (Sim.bNetwork == 0) && XY(Sim.Players.Players[c].CursorPos).IfIsWithin(102, 440, 255, 459)) {
                        if ((Sim.Players.Players[c].Buttons & 1) != 0) {
                            NumSimSteps *= 5;
                            Faktor *= 5;
                        }
                        if ((Sim.Players.Players[c].Buttons & 2) != 0) {
                            NumSimSteps *= 20;
                            Faktor *= 20;
                        }
                    }

                    if ((Sim.Players.Players[c].WaitForRoom != 0U) && (IsRoomBusy(Sim.Players.Players[c].WaitForRoom, c) == 0)) {
                        Sim.Players.Players[c].WalkToRoom(UBYTE(Sim.Players.Players[c].WaitForRoom));
                    }
                }

                // Tutorium:
                if (Sim.IsTutorial != 0) {
                    PLAYER &qLocalPlayer = Sim.Players.Players[Sim.localPlayer];

                    if (Sim.Tutorial == 999) {
                        Sim.Tutorial = 1000;
                        qLocalPlayer.Messages.IsMonolog = TRUE;
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1000));
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1001));
                    } else if (Sim.Tutorial == 1000 && (qLocalPlayer.Messages.IsSilent() != 0)) {
                        Sim.Tutorial = 1100;
                        qLocalPlayer.Messages.IsMonolog = FALSE;
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1100));
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1101));
                    } else if (Sim.Tutorial == 1100 && (qLocalPlayer.Messages.IsSilent() != 0) && qLocalPlayer.GetRoom() == ROOM_AUFSICHT) {
                        Sim.Tutorial = 1200 + 30;
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1200));
                        qLocalPlayer.Messages.Pump();
                    } else if (Sim.Tutorial == 1200 + 30 && (qLocalPlayer.Messages.IsSilent() != 0)) {
                        SLONG c = 0;
                        for (c = 9; c >= 0; c--) {
                            if (qLocalPlayer.Locations[c] == ROOM_AUFSICHT) {
                                break;
                            }
                        }

                        if (c >= 0) {
                            Sim.Tutorial = 1200 + 40;
                            (*qLocalPlayer.LocationWin).StartDialog(TALKER_BOSS, MEDIUM_AIR, 1);
                            (*qLocalPlayer.LocationWin).DontDisplayPlayer = Sim.localPlayer;
                        }
                    } else if (Sim.Tutorial == 1200 + 40 && qLocalPlayer.GetRoom() == ROOM_AIRPORT) {
                        Sim.bNoTime = FALSE;
                        Sim.DayState = 2;
                        if (qLocalPlayer.GetRoom() != ROOM_AIRPORT) {
                            qLocalPlayer.LeaveRoom();
                        }

                        Sim.Tutorial = 1310;
                        qLocalPlayer.Messages.NextMessage();
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1300));
                        qLocalPlayer.WalkToRoom(UBYTE(ROOM_BURO_A + Sim.localPlayer * 10));
                    } else if (Sim.Tutorial == 1310 && qLocalPlayer.GetRoom() != ROOM_AIRPORT) {
                        Sim.Tutorial = 1400;
                        qLocalPlayer.Messages.NextMessage();
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, bprintf(StandardTexte.GetS(TOKEN_TUTORIUM, 1400), (LPCTSTR)qLocalPlayer.AirlineX));
                    } else if (Sim.Tutorial == 1400 && qLocalPlayer.Messages.Messages[0].Message.GetLength() == 0 &&
                               ((qLocalPlayer.Messages.IsSilent() != 0) || qLocalPlayer.Messages.TalkCountdown <= 1)) {
                        if (qLocalPlayer.LocationWin != nullptr) {
                            CStdRaum &qRaum = *(qLocalPlayer.LocationWin);

                            qRaum.GlowEffects.ReSize(1);
                            qRaum.GlowBitmapIndices.ReSize(1);
                            qRaum.GlowEffects[0] = XY(311, 165);
                            qRaum.GlowBitmapIndices[0] = 0;
                        }

                        Sim.Tutorial = 1401;
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1401));
                    } else if (Sim.Tutorial == 1401 && qLocalPlayer.GetRoom() == ROOM_GLOBE) {
                        if (qLocalPlayer.LocationWin != nullptr) {
                            (qLocalPlayer.LocationWin)->GlowEffects.ReSize(0);
                        }

                        Sim.Tutorial = 1500;
                        qLocalPlayer.Messages.NextMessage();
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1500));
                    } else if (Sim.Tutorial == 1500) {
                        Sim.Tutorial = 1501;
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1501));
                    } else if (Sim.Tutorial == 1501 && (qLocalPlayer.Messages.IsSilent() != 0)) {
                        if (qLocalPlayer.LocationWin != nullptr) {
                            CStdRaum &qRaum = *(qLocalPlayer.LocationWin);

                            qRaum.GlowEffects.ReSize(1);
                            qRaum.GlowBitmapIndices.ReSize(1);
                            qRaum.GlowEffects[0] = XY(20 + 20, 248 + 11);
                            qRaum.GlowBitmapIndices[0] = 1;
                        }

                        Sim.Tutorial = 1502;
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1502));
                    } else if (Sim.Tutorial == 1507 && qLocalPlayer.GetRoom() == ROOM_AIRPORT) {
                        Sim.Tutorial = 1600;
                        qLocalPlayer.Messages.NextMessage();
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1600));
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1601));
                    } else if (Sim.Tutorial == 2000 && (qLocalPlayer.Messages.IsSilent() != 0)) {
                        Sim.Tutorial = 2002;
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 2002));
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 2003));
                    } else if (Sim.Tutorial == 2002 && (qLocalPlayer.Messages.IsSilent() != 0)) {
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 2004));
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 2005));
                        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 2006));
                        qLocalPlayer.Messages.IsMonolog = FALSE;
                        Sim.IsTutorial = FALSE;
                    }
                }

                // CurrentGameSpeed:
                d = 3;
                for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                    if (Sim.Players.Players[c].Owner != 1 && Sim.Players.Players[c].CallItADay == FALSE) {
                        d = min(d, Sim.Players.Players[c].GameSpeed);
                    }
                }

                if ((Sim.CallItADay != 0) || Sim.Time < 9 * 60000) {
                    d = 5;
                }
                if ((Sim.Time < 8 * 60000 || Sim.Time >= 18 * 60000) && Sim.DayState == 4) {
                    d = 5;
                }

                if (bgWarp != 0) {
                    PERSON &qPerson = Sim.Persons[Sim.Persons.GetPlayerIndex(Sim.localPlayer)];

                    if (Sim.Players.Players[Sim.localPlayer].GetRoom() != ROOM_AIRPORT ||
                        (qPerson.Dir == 8 && Sim.Players.Players[Sim.localPlayer].PrimaryTarget == Sim.Players.Players[Sim.localPlayer].TertiaryTarget)) {
                        bgWarp = FALSE;

                        if (Sim.Players.Players[Sim.localPlayer].LocationWin != nullptr) {
                            (dynamic_cast<AirportView *>(Sim.Players.Players[Sim.localPlayer].LocationWin))->CenterCameraOnPlayer();
                        }
                    }

                    if (bgWarp != 0) {
                        d = 4;
                    }
                }

                {
                    SLONG Multiplier = 1;

                    switch (d) {
                    case 0:
                        break;
                    case 1:
                        Multiplier = 2;
                        break;
                    case 2:
                        if (Sim.bNetwork != 0) {
                            Multiplier = 3;
                        } else {
                            Multiplier = 4;
                        }
                        break;
                    case 3:
                        if (Sim.bNetwork != 0) {
                            Multiplier = 4;
                        } else {
                            Multiplier = 8;
                        }
                        break;
                    case 4:
                        Multiplier = 150;
                        break;
                    case 5:
                        Multiplier = 600;
                        break;
                    default:
                        printf("Takeoff.cpp: Default case should not be reached.");
                        DebugBreak();
                    }
                    NumSimSteps *= Multiplier;
                    Faktor *= Multiplier;
                }

                if (Sim.bNetwork != 0) {
                    // Synchronisierung morgends:
                    if (Sim.GetHour() >= 9 && (Sim.bWatchForReady != 0) && Sim.CallItADay == 0) {
                        if (!static_cast<bool>(Sim.Players.Players[Sim.localPlayer].bReadyForMorning) &&
                            (Sim.Players.Players[Sim.localPlayer].GetRoom() != ROOM_AUFSICHT || Sim.GetHour() > 9 || Sim.GetMinute() > 0)) {
                            SIM::SendSimpleMessage(ATNET_READYFORMORNING, 0, Sim.localPlayer);
                            Sim.Players.Players[Sim.localPlayer].bReadyForMorning = 1;
                        }

                        if ((Sim.bWatchForReady != 0) &&
                            (Sim.Players.Players[Sim.localPlayer].GetRoom() != ROOM_AUFSICHT ||
                             (Sim.Players.Players[Sim.localPlayer].GetRoom() == ROOM_AUFSICHT &&
                              (dynamic_cast<CAufsicht *>(Sim.Players.Players[Sim.localPlayer].LocationWin))->bExitASAP)) &&
                            (Sim.Players.Players[Sim.localPlayer].LocationWin != nullptr)) {
                            for (c = 0; c < 4; c++) {
                                if (!static_cast<bool>(Sim.Players.Players[c].bReadyForMorning) && Sim.Players.Players[c].Owner != 1) {
                                    NumSimSteps = 0;

                                    if (Sim.Time == 9 * 60000) {
                                        SetNetworkBitmap(2);
                                    }
                                }
                            }

                            if (NumSimSteps != 0U) {
                                Sim.bWatchForReady = FALSE;
                                SetNetworkBitmap(0);

                                // wirkt wie bgJustDidLotsOfWork:
                                LastTime = Time;
                                NumSimSteps = min(1, NumSimSteps);
                            }
                        } else if (Sim.Players.Players[Sim.localPlayer].LocationWin == nullptr) {
                            NumSimSteps = min(1, NumSimSteps);
                        }
                    }

                    // Synchronisierung beim Feierabend:
                    if (Sim.GetHour() >= 9 && Sim.GetHour() < 18 && Sim.CallItADay == 1) {
                        if (Sim.Players.GetAnzRobotPlayers() > 0) {
                            if (Sim.bIsHost != 0) {
                                // Host: Nicht weitermachen, wenn ein Client noch nicht so weit ist:
                                for (c = 0; c < 4; c++) {
                                    if (!static_cast<bool>(Sim.Players.Players[c].bReadyForMorning) && Sim.Players.Players[c].Owner == 2 &&
                                        (Sim.Players.Players[c].IsOut == 0)) {
                                        NumSimSteps = 0;
                                    }
                                }
                            } else if (Sim.b18Uhr == 0) {
                                // Client: Maximal bis zum Punkt rechnen, den der Host vorgegeben hat:
                                SLONG LeastWaitTill = -1;

                                for (c = 0; c < 4; c++) {
                                    if (Sim.Players.Players[c].Owner == 1 && (Sim.Players.Players[c].IsOut == 0)) {
                                        if (LeastWaitTill == -1 ||
                                            (Sim.Players.Players[c].WaitWorkTill < LeastWaitTill && Sim.Players.Players[c].WaitWorkTill != -1)) {
                                            LeastWaitTill = Sim.Players.Players[c].WaitWorkTill;
                                        }
                                    }
                                }

                                if (LeastWaitTill == -1) {
                                    NumSimSteps = 0;
                                } else if (Sim.TimeSlice > LeastWaitTill) {
                                    NumSimSteps = 1;
                                } else {
                                    NumSimSteps = min(NumSimSteps, ULONG(LeastWaitTill - Sim.TimeSlice + 1));
                                }
                            }
                        }
                        if (NumSimSteps == 0 && bCAbendOpen == 0) {
                            NumSimSteps = 1;
                        }
                    } else if (Sim.GetHour() >= 18 && Sim.b18Uhr == FALSE && (Sim.bIsHost != 0)) {
                        Sim.b18Uhr = TRUE;
                        SIM::SendSimpleMessage(ATNET_PLAYER_18UHR);

                        for (c = 0; c < 4; c++) {
                            Sim.Players.Players[c].bReadyForMorning = 0;
                        }
                    }

                    if (Sim.GetHour() >= 19) {
                        Sim.b18Uhr = TRUE;
                    }
                } else {
                    NumSimSteps = min(static_cast<SLONG>(NumSimSteps), Faktor);
                }

                if (Sim.Time > 8 * 60000 && CheatTestGame == 2 && (Sim.Players.Players[Sim.localPlayer].CallItADay == 0)) {
                    if (Sim.Date % 10 == 0) {
                        Sim.SaveGame(11, StandardTexte.GetS(TOKEN_MISC, 5000));
                    }

                    Sim.Players.Players[Sim.localPlayer].CallItADay = TRUE;
                }

                // Im Netzwerk die Zeit aufgrund der ATNET_TIMEPING Nachricht fliessend anpassen
                if ((Sim.bNetwork != 0) && (NumSimSteps != 0U) && (gTimerCorrection != 0)) {
                    static UBYTE MyPrivateRandom = 0;

                    MyPrivateRandom++;

                    if (((MyPrivateRandom & 1) == 0 && abs(gTimerCorrection) > 10) || (MyPrivateRandom & 7) == 0) {
                        if (gTimerCorrection > 0) {
                            NumSimSteps++;
                            gTimerCorrection--;
                        } else if (gTimerCorrection < 0) {
                            NumSimSteps--;
                            gTimerCorrection++;
                        }
                    }
                }

                // Sind noch Simulationsschritte offen ?
                if (NumSimSteps > 0 && (Sim.Gamestate == (GAMESTATE_PLAYING | GAMESTATE_WORKING) || Sim.Gamestate == (GAMESTATE_QUIT | GAMESTATE_WORKING))) {
                    RefreshNeccessary = TRUE;
                    SimStepsCounter = 0;

                    SLONG NumSimStepsBegin = NumSimSteps;

                    while (NumSimSteps > 0 &&
                           (SimStepsCounter < 400 || (SimStepsCounter < 1600 && ((Sim.CallItADay != 0) || Sim.Time > 18 * 60000 || Sim.Time < 9 * 60000)))) {
                        // Synchronisierung beim Feierabend:
                        if (Sim.GetHour() >= 9 && Sim.GetHour() < 18 && Sim.CallItADay == 1 && (Sim.bIsHost != 0) && NumSimStepsBegin != 1) {
                            for (c = 0; c < 4; c++) {
                                if (!static_cast<bool>(Sim.Players.Players[c].bReadyForMorning) && Sim.Players.Players[c].Owner == 2 &&
                                    (Sim.Players.Players[c].IsOut == 0)) {
                                    NumSimSteps = 0;
                                }
                            }

                            if (NumSimSteps == 0) {
                                break;
                            }
                        }

                        if ((Sim.CallItADayAt != 0) && Sim.CallItADay == FALSE) {
                            if (Sim.TimeSlice >= Sim.CallItADayAt) {
                                Sim.CallItADay = TRUE;
                            } else {
                                NumSimSteps = min(NumSimSteps, ULONG(Sim.CallItADayAt - Sim.TimeSlice));
                                if (NumSimSteps < 0) {
                                    NumSimSteps = 0;
                                }
                            }
                        }

                        if (nOptionsOpen == 0 && Sim.Players.Players[Sim.localPlayer].IsDrunk > 0 &&
                            ((SDL_GetWindowFlags(FrameWnd->m_hWnd) & SDL_WINDOW_MOUSE_FOCUS) == SDL_WINDOW_MOUSE_FOCUS)) {
                            SLONG mouseX = 0;
                            SLONG mouseY = 0;
                            SDL_GetGlobalMouseState(&mouseX, &mouseY);

                            SDL_WarpMouseGlobal(SLONG(mouseX + sin(Sim.TimeSlice * 70 / 200.0) * cos(Sim.TimeSlice * 70 / 160.0) *
                                                                   Sim.Players.Players[Sim.localPlayer].IsDrunk / 30),
                                                SLONG(mouseY + cos(Sim.TimeSlice * 70 / 230.0) * sin(Sim.TimeSlice * 70 / 177.0) *
                                                                   Sim.Players.Players[Sim.localPlayer].IsDrunk / 30));
                            Sim.Players.Players[Sim.localPlayer].IsDrunk--;
                        }

                        if ((Sim.Time >= 9 * 60000 && Sim.Time <= 18 * 60000 && (Sim.CallItADay == 0)) ||
                            (Sim.Players.Players.AnzEntries() > 0 && Sim.Players.Players[Sim.localPlayer].GetRoom() != ROOM_ABEND && bCAbendOpen == 0)) {
                            for (c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
                                PLAYER &qPlayer = Sim.Players.Players[c];

                                if (qPlayer.Koffein > 0) {
                                    qPlayer.Koffein--;
                                }

                                if ((Sim.Time & 31) == 0) {
                                    qPlayer.CalcRoom();
                                }

                                // Veränderung am Second-Level-Raum?
                                if (Sim.Difficulty != DIFF_TUTORIAL || Sim.Tutorial >= 1200 || Sim.IsTutorial == 0 || Sim.Tutorial == 0) {
                                    if (qPlayer.Owner == 0) // Nur Spieler bekommen Fenster
                                    {
                                        BOOL JustLeftRoom = FALSE;

                                        // Raum verlassen...?
                                        for (d = 0; d < 10; d++) {
                                            if ((qPlayer.Locations[d] & ROOM_LEAVING) != 0) {
                                                if (Sim.RoomBusy[qPlayer.Locations[d] & 255] > 0) {
                                                    Sim.RoomBusy[qPlayer.Locations[d] & 255]--;
                                                }

                                                if (qPlayer.LocationWin != nullptr) {
                                                    JustLeftRoom = TRUE;
                                                    if ((Sim.Options.OptionBlenden != 0) && qPlayer.Locations[d] != ROOM_LAPTOP &&
                                                        (Sim.GetHour() > 9 || Sim.Date == 0)) {
                                                        gBlendState = -1;
                                                        if (FrameWnd != nullptr) {
                                                            GameFrame::PrepareFade();
                                                        }
                                                        FrameWnd->Invalidate();
                                                        MessagePump();
                                                        if (Sim.Players.Players.AnzEntries() == 0) {
                                                            break;
                                                        }
                                                    }

                                                    bIngnoreNextDoor = TRUE;

                                                    delete qPlayer.LocationWin;
                                                    qPlayer.LocationWin = nullptr;
                                                }

                                                qPlayer.Locations[d] = 0;

                                                if (d > 0 && qPlayer.Locations[d - 1] != 0) {
                                                    qPlayer.Locations[d - 1] = UWORD((qPlayer.Locations[d - 1] & (~ROOM_LEAVING)) | ROOM_ENTERING);
                                                }

                                                qPlayer.CalcRoom();
                                                qPlayer.BroadcastPosition();
                                                qPlayer.BroadcastRooms(ATNET_LEAVEROOM);
                                                Sim.UpdateRoomUsage();
                                            }
                                        }
                                        if (Sim.Players.Players.AnzEntries() == 0) {
                                            break;
                                        }

                                        // Raum betreten...?
                                        for (d = 0; d < 10; d++) {
                                            if (((qPlayer.Locations[d] & ROOM_ENTERING) != 0) && ((qPlayer.Locations[d] & ROOM_LEAVING) != 0)) {
                                                qPlayer.Locations[d] = 0;
                                            }

                                            if ((qPlayer.Locations[d] & ROOM_ENTERING) != 0) {
                                                if (qPlayer.LocationWin != nullptr) {
                                                    if ((JustLeftRoom == 0) && (Sim.Options.OptionBlenden != 0) && qPlayer.Locations[d] != ROOM_LAPTOP &&
                                                        qPlayer.Locations[d] != ROOM_GLOBE && (Sim.GetHour() > 9 || Sim.Date == 0)) {
                                                        gBlendState = -1;
                                                        if (FrameWnd != nullptr) {
                                                            GameFrame::PrepareFade();
                                                        }
                                                        FrameWnd->Invalidate();
                                                        MessagePump();
                                                        if (Sim.Players.Players.AnzEntries() == 0) {
                                                            break; // Spiel beendet
                                                        }
                                                    }

                                                    delete qPlayer.LocationWin;
                                                    qPlayer.LocationWin = nullptr;
                                                }

                                                qPlayer.Locations[d] &= ~ROOM_ENTERING;
                                                gMouseScroll = 0;

                                                if (qPlayer.Locations[d] != ROOM_LAPTOP && (gBlendBm.Size.y != 0) && (Sim.GetHour() > 9 || Sim.Date == 0)) {
                                                    gBlendState = -2;
                                                }

                                                SLONG Room = (qPlayer.Locations[d] & ~ROOM_LEAVING);
                                                if (Room == ROOM_ARAB_AIR || Room == ROOM_ROUTEBOX || Room == ROOM_KIOSK || Room == ROOM_RICKS ||
                                                    Room == ROOM_LAST_MINUTE || Room == ROOM_REISEBUERO) {
                                                    if (Room == ROOM_ARAB_AIR || Room == ROOM_ROUTEBOX || Room == ROOM_KIOSK || Room == ROOM_RICKS) {
                                                        Sim.Persons[Sim.Persons.GetPlayerIndex(c)].LookAt(0);
                                                    }

                                                    if (Room == ROOM_LAST_MINUTE) {
                                                        Sim.Persons[Sim.Persons.GetPlayerIndex(c)].LookAt(1);
                                                    }

                                                    if (Room == ROOM_REISEBUERO) {
                                                        Sim.Persons[Sim.Persons.GetPlayerIndex(c)].LookAt(3);
                                                    }

                                                    qPlayer.CalcRoom();
                                                    qPlayer.BroadcastRooms(ATNET_ENTERROOM);
                                                    qPlayer.BroadcastPosition(true);
                                                }

                                                // Fenster für Raum eröffnen:
                                                switch (Room) {
                                                case ROOM_BURO_A:
                                                case ROOM_BURO_B:
                                                case ROOM_BURO_C:
                                                case ROOM_BURO_D:
                                                    qPlayer.LocationWin = new CBuero(FALSE, c);
                                                    break;

                                                case ROOM_PERSONAL_A:
                                                case ROOM_PERSONAL_B:
                                                case ROOM_PERSONAL_C:
                                                case ROOM_PERSONAL_D:
                                                    qPlayer.LocationWin = new CPersonal(FALSE, c);
                                                    break;

                                                case ROOM_AIRPORT:
                                                    qPlayer.LocationWin = new AirportView(FALSE, c);
                                                    break;

                                                case ROOM_PLANEPROPS:
                                                    qPlayer.LocationWin = new CPlaneProps(FALSE, c);
                                                    break;
                                                case ROOM_EDITOR:
                                                    qPlayer.LocationWin = new CEditor(FALSE, c);
                                                    break;
                                                case ROOM_SHOP1:
                                                    qPlayer.LocationWin = new CDutyFree(FALSE, c);
                                                    break;
                                                case ROOM_GLOBE:
                                                    qPlayer.LocationWin = new CGlobe(FALSE, c);
                                                    break;
                                                case ROOM_LAPTOP:
                                                    qPlayer.LocationWin = new CLaptop(FALSE, c);
                                                    break;
                                                case ROOM_ARAB_AIR:
                                                    qPlayer.LocationWin = new CArabAir(FALSE, c);
                                                    break;
                                                case ROOM_SABOTAGE:
                                                    qPlayer.LocationWin = new CSabotage(FALSE, c);
                                                    break;
                                                case ROOM_AUFSICHT:
                                                    qPlayer.LocationWin = new CAufsicht(FALSE, c);
                                                    break;
                                                case ROOM_ABEND:
                                                    qPlayer.LocationWin = new CAbend(FALSE, c);
                                                    break;
                                                case ROOM_BANK:
                                                    qPlayer.LocationWin = new Bank(FALSE, c);
                                                    break;
                                                case ROOM_KIOSK:
                                                    qPlayer.LocationWin = new CKiosk(FALSE, c);
                                                    break;
                                                case ROOM_DESIGNER:
                                                    qPlayer.LocationWin = new CDesigner(FALSE, c);
                                                    break;
                                                case ROOM_LAST_MINUTE:
                                                    qPlayer.LocationWin = new CLastMinute(FALSE, c);
                                                    break;
                                                case ROOM_FRACHT:
                                                    qPlayer.LocationWin = new CFrachtRaum(FALSE, c);
                                                    break;
                                                case ROOM_MAKLER:
                                                    qPlayer.LocationWin = new CMakler(FALSE, c);
                                                    break;
                                                case ROOM_TAFEL:
                                                    qPlayer.LocationWin = new CTafel(FALSE, c);
                                                    break;
                                                case ROOM_OPTIONS:
                                                    qPlayer.LocationWin = new Options(FALSE, c);
                                                    break;
                                                case ROOM_WERKSTATT:
                                                    qPlayer.LocationWin = new CWerkstatt(FALSE, c);
                                                    break;
                                                case ROOM_WERBUNG:
                                                    qPlayer.LocationWin = new CWerbung(FALSE, c);
                                                    break;
                                                case ROOM_REISEBUERO:
                                                    qPlayer.LocationWin = new CReisebuero(FALSE, c);
                                                    break;
                                                case ROOM_NASA:
                                                    qPlayer.LocationWin = new CNasa(FALSE, c);
                                                    break;
                                                case ROOM_ROUTEBOX:
                                                    qPlayer.LocationWin = new CRouteBox(FALSE, c);
                                                    break;
                                                case ROOM_STATISTICS:
                                                    qPlayer.LocationWin = new CStatistik(FALSE, c);
                                                    break;
                                                case ROOM_WORLD:
                                                    qPlayer.LocationWin = new CWorld(FALSE, c, Sim.HomeAirportId);
                                                    break;
                                                case ROOM_MUSEUM:
                                                    qPlayer.LocationWin = new CMuseum(FALSE, c);
                                                    break;
                                                case ROOM_RICKS:
                                                    qPlayer.LocationWin = new CRicks(FALSE, c);
                                                    break;
                                                case ROOM_SECURITY:
                                                    qPlayer.LocationWin = new CSecurity(FALSE, c);
                                                    break;

                                                case ROOM_INSEL:
                                                    PlayUniversalFx("coin.raw", Sim.Options.OptionEffekte);
                                                    qPlayer.LocationWin = new CInsel(FALSE, c);
                                                    break;

                                                case ROOM_WELTALL:
                                                    PlayUniversalFx("coin.raw", Sim.Options.OptionEffekte);
                                                    qPlayer.LocationWin = new CWeltAll(FALSE, c);
                                                    break;

                                                case ROOM_RUSHMORE:
                                                    PlayUniversalFx("coin.raw", Sim.Options.OptionEffekte);
                                                    qPlayer.LocationWin = new CRushmore(FALSE, c);
                                                    break;

                                                default:
                                                    if (qPlayer.Locations[d] != 2049) {
                                                        hprintf(0, "Room %li not implemented!", qPlayer.Locations[d]);
                                                    }
                                                    qPlayer.Locations[d] = 0;
                                                    break;
                                                }

                                                // Globe wegen den MessagePumps erst jetzt faden...
                                                if ((JustLeftRoom == 0) && qPlayer.Locations[d] == ROOM_GLOBE && (Sim.GetHour() > 9 || Sim.Date == 0)) {
                                                    gBlendState = -1;
                                                    if ((FrameWnd != nullptr) && gBlendBm.Size.y == 0) {
                                                        GameFrame::PrepareFade();
                                                    }
                                                    FrameWnd->Invalidate();
                                                    MessagePump();
                                                }

                                                qPlayer.CalcRoom();
                                                qPlayer.BroadcastPosition(true);
                                                qPlayer.BroadcastRooms(ATNET_ENTERROOM);
                                                Sim.UpdateRoomUsage();
                                            }
                                        }
                                    } else if (Sim.Players.Players[c].Owner == 1) // Und das gleiche für Roboter:
                                    {
                                        for (d = 0; d < 10; d++) {
                                            if ((qPlayer.Locations[d] & ROOM_ENTERING) != 0) {
                                                if (IsRoomBusy(UWORD(qPlayer.Locations[d] & (~ROOM_ENTERING)), c) != 0) {
                                                    // Raum schon besetzt? Erst zweite Priorität ausführen:
                                                    for (e = qPlayer.RobotActions.AnzEntries() - 1; e >= 1; e--) {
                                                        qPlayer.RobotActions[e] = qPlayer.RobotActions[e - 1];
                                                    }
                                                    qPlayer.RobotActions[1] = qPlayer.RobotActions[2];
                                                    qPlayer.RobotActions[2] = qPlayer.RobotActions[0];

                                                    if (qPlayer.RobotActions[1].ActionId == ACTION_NONE) {
                                                        if (qPlayer.OfficeState == 2) {
                                                            qPlayer.RobotActions[1].ActionId = ACTION_PERSONAL;
                                                        } else {
                                                            qPlayer.RobotActions[1].ActionId = ACTION_BUERO;
                                                        }
                                                    }

                                                    qPlayer.RobotActions[0].ActionId = ACTION_NONE;
                                                    // qPlayer.RobotPump (); //wg. Verdacht auf Netzprobleme rausgenommen
                                                } else {
                                                    qPlayer.Locations[d] &= ~ROOM_ENTERING;
                                                    qPlayer.CalcRoom();
                                                    if (qPlayer.Locations[d] != ROOM_AIRPORT) {
                                                        qPlayer.RobotExecuteAction();
                                                    }

                                                    SLONG Room = (qPlayer.Locations[d] & ~ROOM_LEAVING);
                                                    if (Room == ROOM_ARAB_AIR || Room == ROOM_ROUTEBOX || Room == ROOM_KIOSK || Room == ROOM_RICKS ||
                                                        Room == ROOM_LAST_MINUTE || Room == ROOM_REISEBUERO) {
                                                        if (Room == ROOM_ARAB_AIR || Room == ROOM_ROUTEBOX || Room == ROOM_KIOSK || Room == ROOM_RICKS) {
                                                            Sim.Persons[Sim.Persons.GetPlayerIndex(c)].LookAt(0);
                                                        }

                                                        if (Room == ROOM_LAST_MINUTE) {
                                                            Sim.Persons[Sim.Persons.GetPlayerIndex(c)].LookAt(1);
                                                        }

                                                        if (Room == ROOM_REISEBUERO) {
                                                            Sim.Persons[Sim.Persons.GetPlayerIndex(c)].LookAt(3);
                                                        }
                                                    }

                                                    switch (qPlayer.Locations[d]) {
                                                    case ROOM_AUFSICHT:
                                                        Talkers.Talkers[TALKER_BOSS].IncreaseLocking();
                                                        break;
                                                    case ROOM_ARAB_AIR:
                                                        Talkers.Talkers[TALKER_ARAB].IncreaseLocking();
                                                        break;
                                                    case ROOM_SABOTAGE:
                                                        Talkers.Talkers[TALKER_SABOTAGE].IncreaseLocking();
                                                        break;
                                                    case ROOM_BANK:
                                                        Talkers.Talkers[TALKER_BANKER1].IncreaseLocking();
                                                        Talkers.Talkers[TALKER_BANKER2].IncreaseLocking();
                                                        break;
                                                    case ROOM_MUSEUM:
                                                        Talkers.Talkers[TALKER_MUSEUM].IncreaseLocking();
                                                        break;
                                                    case ROOM_MAKLER:
                                                        Talkers.Talkers[TALKER_MAKLER].IncreaseLocking();
                                                        break;
                                                    case ROOM_WERKSTATT:
                                                        Talkers.Talkers[TALKER_MECHANIKER].IncreaseLocking();
                                                        break;
                                                    case ROOM_WERBUNG:
                                                        Talkers.Talkers[TALKER_WERBUNG].IncreaseLocking();
                                                        break;
                                                    case ROOM_SECURITY:
                                                        Talkers.Talkers[TALKER_SECURITY].IncreaseLocking();
                                                        break;
                                                    default:
                                                        break;
                                                    }

                                                    if ((Sim.bNetwork != 0) && (Sim.bIsHost != 0)) {
                                                        qPlayer.BroadcastPosition();
                                                        qPlayer.BroadcastRooms(ATNET_ENTERROOM, qPlayer.Locations[d]);
                                                        Sim.UpdateRoomUsage();
                                                    }
                                                }
                                            } else if ((qPlayer.Locations[d] & ROOM_LEAVING) != 0) {
                                                switch (qPlayer.Locations[d] & (~ROOM_LEAVING)) {
                                                case ROOM_AUFSICHT:
                                                    Talkers.Talkers[TALKER_BOSS].DecreaseLocking();
                                                    break;
                                                case ROOM_ARAB_AIR:
                                                    Talkers.Talkers[TALKER_ARAB].DecreaseLocking();
                                                    break;
                                                case ROOM_SABOTAGE:
                                                    Talkers.Talkers[TALKER_SABOTAGE].DecreaseLocking();
                                                    break;
                                                case ROOM_BANK:
                                                    Talkers.Talkers[TALKER_BANKER1].DecreaseLocking();
                                                    Talkers.Talkers[TALKER_BANKER2].DecreaseLocking();
                                                    break;
                                                case ROOM_MUSEUM:
                                                    Talkers.Talkers[TALKER_MUSEUM].DecreaseLocking();
                                                    break;
                                                case ROOM_MAKLER:
                                                    Talkers.Talkers[TALKER_MAKLER].DecreaseLocking();
                                                    break;
                                                case ROOM_WERKSTATT:
                                                    Talkers.Talkers[TALKER_MECHANIKER].DecreaseLocking();
                                                    break;
                                                case ROOM_WERBUNG:
                                                    Talkers.Talkers[TALKER_WERBUNG].DecreaseLocking();
                                                    break;
                                                case ROOM_SECURITY:
                                                    Talkers.Talkers[TALKER_SECURITY].DecreaseLocking();
                                                    break;
                                                default:
                                                    break;
                                                }

                                                SLONG RoomLeft = qPlayer.Locations[d] & 255;

                                                if (Sim.RoomBusy[RoomLeft] != 0U) {
                                                    Sim.RoomBusy[RoomLeft]--;
                                                }

                                                qPlayer.Locations[d] = 0;
                                                qPlayer.Locations[d - 1] = UWORD((qPlayer.Locations[d - 1] & (~ROOM_LEAVING)) | ROOM_ENTERING);
                                                qPlayer.CalcRoom();

                                                if ((Sim.bNetwork != 0) && (Sim.bIsHost != 0)) {
                                                    qPlayer.BroadcastPosition();
                                                    qPlayer.BroadcastRooms(ATNET_LEAVEROOM, RoomLeft);
                                                    Sim.UpdateRoomUsage();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if (nOptionsOpen == 0 && nWaitingForPlayer == 0 && (Sim.bPause == 0)) {
                            // Personen bewegen:
                            if (Sim.Time >= 8 * 60000 && Sim.Time <= 18 * 60000 && Editor == EDITOR_NONE && (Sim.CallItADay == 0)) {
                                if (Sim.Difficulty != DIFF_TUTORIAL || Sim.Tutorial >= 1100 || (Sim.IsTutorial == 0)) {
                                    for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                                        if ((Sim.Time & 255) == 0 && (Sim.Players.Players[c].iWalkActive != 0)) {
                                            Sim.Players.Players[c].UpdateWaypoints();
                                        }
                                        Sim.Players.Players[c].UpdateWaypointWalkingDirection();
                                    }
                                }
                            }

                            if (!(Sim.Time < 8 * 60000 || (Sim.Time >= 18 * 60000 && Sim.DayState == 4))) {
                                if (Editor == EDITOR_NONE) {
                                    if (Sim.CallItADay == 0) {
                                        Sim.PersonQueue.Pump();
                                    }
                                    Sim.Persons.DoOneStep();

                                    if (Sim.CallItADay == 0) {
                                        Talkers.Pump();
                                        Airport.PumpDoors();
                                    }
                                }
                            }

                            if (Sim.Time > 9 * 60000 && Sim.Time <= 18 * 60000 && Sim.bNoTime == FALSE && (Editor == 0)) {
                                if (Sim.bNoTime == FALSE) { // Neu, wegen Bugs im Netzwerk
                                    if (Sim.Difficulty != DIFF_TUTORIAL || Sim.Tutorial >= 1100 || (Sim.IsTutorial == 0)) {
                                        Sim.Players.RobotPump();
                                    }
                                }
                            }

                            // Flugzeuge bewegen:
                            {
                                AnzPlanesOnScreen = 0;
                                if (Editor == EDITOR_NONE) {
                                    if (SkipPlaneCalculation <= 0) {
                                        SkipPlaneCalculation = 10;
                                        for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                                            if (Sim.Players.Players[c].IsOut == 0) {
                                                Sim.Players.Players[c].Planes.DoOneStep(c);
                                            }
                                        }
                                    } else {
                                        SkipPlaneCalculation--;
                                    }
                                }

                                if (LastAnzPlanesOnScreen < AnzPlanesOnScreen) {
                                    LastAnzPlanesOnScreen = min(LastAnzPlanesOnScreen + 3, AnzPlanesOnScreen);
                                }
                                if (LastAnzPlanesOnScreen > AnzPlanesOnScreen) {
                                    LastAnzPlanesOnScreen = max(LastAnzPlanesOnScreen - 3, AnzPlanesOnScreen);
                                }

                                AmbientManager.SetVolume(AMBIENT_JET_OUTSIDE, LastAnzPlanesOnScreen);
                            }

                            // Flugzeuge, die vorbeifliegen:
                            if (Sim.Options.OptionEffekte != 0) {
                                if (gUniversalPlaneFxCountdown == 0 || AtGetTime() > DWORD(gUniversalPlaneFxCountdown)) {
                                    gUniversalPlaneFxCountdown = AtGetTime() + 15000 + rand() % 40 * 1000;

                                    if (gUniversalPlaneFxCountdown != 0) {
                                        gUniversalPlaneFx.ReInit(PlaneSounds[rand() % 6]);
                                        gUniversalPlaneFx.Play(0, Sim.Options.OptionPlaneVolume * 100 / 7 * AmbientManager.GlobalVolume / 100);
                                    }
                                }
                            }

                            // Zeit weitermachen:
                            if (Sim.bNoTime == 0) {
                                // Zeitabhängige Events auslösen & Uhr weitersetzten:
                                if (Sim.Time < 8 * 60000) {
                                    Sim.DoTimeStep();
                                    if (Sim.Time >= 8 * 60000) {
                                        NumSimSteps = 1;
                                    }
                                } else if (Sim.Time < 9 * 60000) {
                                    Sim.DoTimeStep();
                                    if (Sim.Time >= 9 * 60000) {
                                        NumSimSteps = 1;
                                    }
                                } else {
                                    Sim.DoTimeStep();
                                }

                                if (!((Sim.CallItADay != 0) || Sim.Time < 9 * 60000 || (Sim.Time >= 18 * 60000 && Sim.DayState == 4)) &&
                                    ((Sim.Time + NumSimSteps) & 1023) == 0) {
                                    Sim.Players.RandomBeraterMessage();
                                }
                            }

                            if (Sim.Time >= 18 * 60000 || ((Sim.CallItADay != 0) && Sim.DayState == 2) || Sim.DayState == 4) {
                                if (Sim.Time >= 18 * 60000 && Sim.DayState != 4) {
                                    Sim.Players.Players[Sim.localPlayer].CallItADay = FALSE;
                                    Sim.bNoTime = TRUE;
                                }

                                if (Sim.DayState == 2) // Begin Evening Sequenz
                                {
                                    BOOL now = TRUE;

                                    if ((Sim.Players.Players[Sim.localPlayer].LocationWin != nullptr) &&
                                        ((Sim.Players.Players[Sim.localPlayer].LocationWin)->IsDialogOpen() != 0)) {
                                        (Sim.Players.Players[Sim.localPlayer].LocationWin)->StopDialog();
                                    }

                                    for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                                        // Raum verlassen:
                                        if (Sim.Players.Players[c].Owner != 1 && (Sim.Players.Players[c].Locations[0] != 0U) &&
                                            Sim.Players.Players[c].GetRoom() != ROOM_ABEND) {
                                            Sim.Players.Players[c].LeaveRoom();
                                            now = FALSE;
                                        }
                                    }

                                    if (now != 0) {
                                        TopWin = new CAbend(FALSE, Sim.localPlayer);
                                        Sim.DayState = 3;
                                    }
                                } else if (Sim.DayState == 4) // End Evening Sequenz
                                {
                                    if (Sim.Time >= 9 * 60000 && Sim.Time <= 10 * 60000) {
                                        if (TopWin != nullptr) {
                                            delete TopWin;
                                            TopWin = nullptr;
                                        }

                                        Sim.bNoTime = TRUE;
                                        Sim.DayState = 1;
                                        MouseWait++;

                                        for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                                            if (Sim.Players.Players[c].IsOut == 0) {
                                                Sim.Players.Players[c].EnterRoom(ROOM_AIRPORT);
                                                Sim.Players.Players[c].EnterRoom(ROOM_AUFSICHT);
                                            }
                                        }
                                    }
                                }
                            }

                            // Shopper ankommen lassen:
                            if ((Sim.CallItADay == 0) && Sim.Time >= 8 * 60000 && Sim.Time <= 18 * 60000) {
                                if (nOptionsOpen == 0 && nWaitingForPlayer == 0 && (Sim.bPause == 0) &&
                                    !static_cast<bool>(Sim.bNoTime)) // Neu, wegen Bugs im Netzwerk
                                {
                                    if ((Sim.TimeSlice & 63) == 0) {
                                        Sim.AddNewShoppers();
                                    }
                                    if (((Sim.TimeSlice + 16) & 31) == 0) {
                                        Sim.AddNewPassengers();
                                    }

                                    if ((Sim.TimeSlice & 7) == 0 && Sim.Players.Players[Sim.localPlayer].GetRoom() == ROOM_AIRPORT) {
                                        Sim.Persons.TryMoods();
                                    }

                                    if (Sim.Time >= 9 * 60000 && Sim.GetMinute() == 30 && (Sim.GetHour() & 1) == 0 && VoiceScheduler.AnzEntries() == 0 &&
                                        (bgWarp == 0) && Sim.Players.Players[Sim.localPlayer].GameSpeed < 2) {
                                        TEAKRAND LocalRand(Sim.Date + Sim.StartTime + Sim.GetHour());
                                        if (LocalRand.Rand(10) == 0) {
                                            VoiceScheduler.AddVoice("fahrzeug");
                                        } else if (((Sim.GetHour() / 2) & 1) == 0) {
                                            VoiceScheduler.AddVoice("zoneok");
                                        } else {
                                            VoiceScheduler.AddVoice("hinweis");
                                        }
                                    }
                                }
                            }

                            // Gimmicks ggf. rauswerfen:
                            if ((Sim.Time & 127) == 0) {
                                for (SLONG c = Clans.AnzEntries() - 1; c >= 0; c--) {
                                    if (Clans.IsInAlbum(c) != 0) {
                                        // Gimmick seit 2 Sekunden nicht genutzt?
                                        if (Sim.TickerTime - Clans[c].GimmickTime > 60) {
                                            Clans[c].ClanGimmick.Unload();
                                            Clans[c].ClanWarteGimmick.Unload();
                                        }
                                    }
                                }
                            }

                            if (Sim.Time == 9 * 60000 && Sim.TimeSlice != 0) {
                                Sim.TimeSlice = 0;
                                Sim.PersonQueue.NewDay();
                            } else if (nOptionsOpen == 0 && nWaitingForPlayer == 0 && (Sim.bPause == 0) &&
                                       !static_cast<bool>(Sim.bNoTime)) { // Neu, wegen Bugs im Netzwerk
                                Sim.TimeSlice++;
                            }
                        }

                        if (Sim.Time > 9 * 60000 && Sim.Time < 18 * 60000) {
                            VoiceScheduler.Pump();
                        }

                        // if (nOptionsOpen==0 && nWaitingForPlayer==0 && !Sim.bPause) //Neu, wegen Bugs im Netzwerk
                        {
                            NumSimSteps--;
                            SimStepsCounter++;
                            Sim.TicksPlayed++;
                            Sim.TicksSinceLoad++;
                        }

                        PumpNetwork();

                        // Nur zum testen der Flugpläne
                        // Sim.Players.CheckFlighplans();

                        // Exe Check             10 Minuten                  2 Stunden
                        /*if (Sim.TicksSinceLoad==20*60*10 && Sim.TicksPlayed>20*60*60*2 && Sim.NumRestored>0)
                          {
                          BUFFER_V<char> Filename(1002);

                          GetModuleFileName (NULL, Filename, 1000);

                          if (!CheckMainFile (CString((char*)Filename)))
                          {
                          Sim.bExeChanged=20*60*5+rand()%100;
                          }
                          else //Re-Legalisieren:
                          Sim.bExeChanged=0;

                          }
                          else if (Sim.bExeChanged>1)
                          {
                          Sim.bExeChanged--;

                          if (Sim.bExeChanged==1)
                          {
                          CRegistryAccess reg (chRegKey);
                          int temp=1;
                          reg.WriteRegistryKeyEx ((int*)&temp, "&OptionViewedSeq");
                          }
                          }*/

                        // Ggf. das Spiel verlassen
                        if (Sim.Gamestate == (GAMESTATE_QUIT | GAMESTATE_WORKING)) {
                            Sim.QuitCountDown--;
                            while (Sim.QuitCountDown == 0U) {
                                Sim.QuitCountDown--;
                            }
                        }

                        PumpBroadcastBitmap();
                    }
                } else {
                    PumpBroadcastBitmap(true);
                }

                if (Sim.Date == 0 && (Sim.IsTutorial != 0) && Sim.Tutorial == 0 && (Sim.Players.Players[Sim.localPlayer].LocationWin != nullptr)) {
                    Sim.Tutorial = 999;
                }

                // Zeit für die AnimBricks aktualisieren:
                Sim.TickerTime = Time >> 5;
            } else {
                RefreshNeccessary = TRUE;
                Sim.TickerTime = AtGetTime() >> 5;
                PumpBroadcastBitmap(true);
            } // Not In Game ==> Always refresh

            // Bildschirm neu zeichnen:
            if (RefreshNeccessary != 0) {
                if (nOptionsOpen == 0 && nWaitingForPlayer == 0 && (Sim.bPause == 0) && (Sim.CallItADay == 0) && Editor == EDITOR_NONE) {
                    for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                        if (Sim.Players.Players[c].GetRoom() == ROOM_AIRPORT && (Sim.Players.Players[c].LocationWin != nullptr)) {
                            (dynamic_cast<AirportView *>(Sim.Players.Players[c].LocationWin))->MoveCamera();
                        }
                    }
                }

                FrameWnd->Invalidate();

                RefreshNeccessary = FALSE;
            } else {
                MessagePump();
                PrimaryBm.PrimaryBm.Present();
            }

            PumpNetwork();
        }

        /*for (c=0; c<Sim.Players.AnzPlayers; c++)
          {
          static SLONG Emergency=0;

          if (Emergency==0)
          {
          for (SLONG d=0; d<Sim.Players.Players[c].Letters.Letters.AnzEntries(); d++)
          {
          if ((*(ULONG*)&Sim.Players.Players[c].Letters.Letters[d].Subject)==NULL)
          Emergency=TRUE;
          if ((*(ULONG*)&Sim.Players.Players[c].Letters.Letters[d].Letter)==NULL)
          Emergency=TRUE;
          if ((*(ULONG*)&Sim.Players.Players[c].Letters.Letters[d].Absender)==NULL)
          Emergency=TRUE;
          }

          for (d=0; d<Sim.Players.Players[c].Statistiken.AnzEntries(); d++)
          {
          if (Sim.Players.Players[c].Statistiken[d].Days.AnzEntries()==0)
          Emergency=TRUE;
          if (Sim.Players.Players[c].Statistiken[d].Months.AnzEntries()==0)
          Emergency=TRUE;
          }

          if (Emergency)
          {
          MessageBeep (0xffffffff); Sleep (1000);
          MessageBeep (0xffffffff); Sleep (1000);
          MessageBeep (0xffffffff); Sleep (1000);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (1000);
          MessageBeep (0xffffffff); Sleep (1000);
          MessageBeep (0xffffffff); Sleep (1000);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (300);
          MessageBeep (0xffffffff); Sleep (1000);
          MessageBeep (0xffffffff); Sleep (1000);
          MessageBeep (0xffffffff); Sleep (1000);
          }
          }
          }*/
        
        MessagePump();
    }

    if (bFullscreen != 0) {
        // lpDD->RestoreDisplayMode ();
        // ClipCursor (NULL);
    }

    // Shutdown:
    if (TopWin != nullptr) {
        CStdRaum *TmpWin = TopWin;
        TopWin = nullptr;
        delete TmpWin;
    }

    if (Sim.Players.Players.AnzEntries() > 0) {
        for (c = 0; c < Sim.Players.AnzPlayers; c++) {
            if (Sim.Players.Players[c].LocationWin != nullptr) {
                auto *TmpWin = Sim.Players.Players[c].LocationWin;
                Sim.Players.Players[c].LocationWin = nullptr;
                delete TmpWin;
            }
        }
    }

    VoiceScheduler.Clear();
}

#ifdef DEBUG
//void CTakeOffApp::CheckSystem(void) {
//    for (SLONG c = 0; c < Sim.Players.AnzPlayers; c++) {
//        static SLONG Emergency = 0;
//
//        if (Emergency == 0) {
//            for (SLONG d = 0; d < Sim.Players.Players[c].Letters.Letters.AnzEntries(); d++) {
//                if ((*(ULONG *)&Sim.Players.Players[c].Letters.Letters[d].Subject) == NULL)
//                    Emergency = TRUE;
//                if ((*(ULONG *)&Sim.Players.Players[c].Letters.Letters[d].Letter) == NULL)
//                    Emergency = TRUE;
//                if ((*(ULONG *)&Sim.Players.Players[c].Letters.Letters[d].Absender) == NULL)
//                    Emergency = TRUE;
//            }
//
//            for (SLONG d = 0; d < Sim.Players.Players[c].Statistiken.AnzEntries(); d++) {
//                if (Sim.Players.Players[c].Statistiken[d].Days.AnzEntries() == 0)
//                    Emergency = TRUE;
//                if (Sim.Players.Players[c].Statistiken[d].Months.AnzEntries() == 0)
//                    Emergency = TRUE;
//            }
//
//            if (Emergency)
//                DebugBreak();
//        }
//    }
//}
#endif

//--------------------------------------------------------------------------------------------
// Ruft das Help-File auf:
//--------------------------------------------------------------------------------------------
void CTakeOffApp::WinHelp(DWORD /*unused*/, UINT /*unused*/) {
    ToolTipState = FALSE;

    ToolTipTimer = AtGetTime() - 1001;
}

/*!
 * \brief	This method takes a character string and converts it to a string which only contains easy readable characters like A - Z and 1 - 6.
 *
 * \author	Martin Mayer
 *
 * \param   pData	Character string which has to converted to readable characters. No trailing 0 is needed.
 * \param   uLen	Length of the pData buffer.
 *
 * \return	Returns an readable character string which is zero terminated. The returned pointer has to be deleted manually!
 */
char *UCharToReadableAnsi(const unsigned char *pData, const unsigned uLen) {
    unsigned uBitSize = uLen * 8;
    unsigned i = 0;
    unsigned short int wData = 0;
    char *pReturn = nullptr;

    pReturn = new char[(uBitSize + 4) / 5 + 1]; // ( char * ) malloc( ( ( uBitSize + 4 ) / 5 + 1 ) * sizeof( char ) );
    pReturn[(uBitSize + 4) / 5] = '\0';
    for (i = 0; i < uBitSize; i += 5) {
        // Example binary stream which gets split into 5 bit sequences; this string is unique for every input
        // -> so this uses A - Z and 1 - 6 to get 2^5 combinations
        // 11001110 00010101 01001100 00111000
        // 11001 11000 01010 10100 11000 01110 00000

        wData = pData[i / 8] << 8;
        if (i / 8 + 1 < uLen) {
            wData |= (pData[i / 8 + 1]);
        }
        wData = wData >> ((16 - 5) - (i % 8));
        wData &= 0x1f;

        if ('A' + wData > 'Z') {
            wData -= ('Z' - 'A');
            wData += '0';
        } else {
            wData += 'A';
        }

        pReturn[i / 5] = static_cast<char>(wData);
    }
    return pReturn;
}

/*!
 * \brief	This method converts a encoded to readability string back to its origin data.
 *
 * \author	Martin Mayer
 *
 * \param   pData	Character string which has to be converted to its origin data.
 * \param   uLen	Length of the pData buffer.
 *
 * \return	Returns the origin data string with a trailing zero. The returned pointer has to be deleted manually!
 */
unsigned char *ReadableAnsiToUChar(const char *pData, const unsigned uLen) {
    unsigned uNewSize = uLen * 5 / 8;
    auto *pReturn = new unsigned char[uNewSize + 1]; //( unsigned char * ) malloc( uNewSize + 1 );
    memset(pReturn, 0, uNewSize + 1);

    for (unsigned i = 0; i < uLen; i++) {
        char cChar = (pData[i] < 'A' ? (pData[i] - '0') + ('Z' - 'A') : pData[i] - 'A');

        unsigned uBitPos = i * 5;
        unsigned uBitPosMod8 = uBitPos % 8;
        if (uBitPosMod8 > 3) {
            unsigned uRightShift = (5 - (8 - uBitPosMod8));
            char cTmp = cChar >> uRightShift;
            pReturn[uBitPos / 8] |= cTmp;
            if (uBitPos / 8 + 1 < uNewSize) {
                pReturn[uBitPos / 8 + 1] |= cChar << (8 - uRightShift);
            }
        } else {
            cChar <<= (3 - uBitPosMod8);
            pReturn[uBitPos / 8] |= cChar;
        }
    }

    return pReturn;
}

void CTakeOffApp::LadeWeitereStandardTexte() {
    const char TOKEN_STAT[] = "STAT";
    const char TOKEN_MONEY[] = "Mony";
    const char TOKEN_AKTIE[] = "Akti";
    const char TOKEN_JOBS[] = "Jobs";
    const char TOKEN_ARAB[] = "Arab";
    const char TOKEN_MISC[] = "Misc";

    StandardTexte.AddText(TOKEN_STAT, 10000, "E::Bodyguard discountD::Bodyguard-Rabatt");
    StandardTexte.AddText(TOKEN_STAT, 10001, "E::InterestD::Zinsen");
    StandardTexte.AddText(TOKEN_STAT, 10002, "E::LoansD::Kredite");
    StandardTexte.AddText(TOKEN_STAT, 10003, "E::Planes soldD::Flugzeugverk\xE4ufe");
    StandardTexte.AddText(TOKEN_STAT, 10004, "E::Stock tradingD::Aktiengesch\xE4\x66te");
    StandardTexte.AddText(TOKEN_STAT, 10010, "E::Planes boughtD::Flugzeugk\xE4ufe");
    StandardTexte.AddText(TOKEN_STAT, 10011, "E::InterestD::Zinsen");
    StandardTexte.AddText(TOKEN_STAT, 10012, "E::LoansD::Kredite");
    StandardTexte.AddText(TOKEN_STAT, 10013, "E::Meals f. passengersD::Essen f. Passagiere");
    StandardTexte.AddText(TOKEN_STAT, 10014, "E::ExpansionD::Expansion");
    StandardTexte.AddText(TOKEN_STAT, 10015, "E::Sabotage jobsD::Sabotageauftr\xE4ge");
    StandardTexte.AddText(TOKEN_STAT, 10016, "E::Stock tradingD::Aktiengesch\xE4\x66te");
    StandardTexte.AddText(TOKEN_STAT, 10017, "E::Agency feesD::Agenturhonorare");

    StandardTexte.UpdateText(TOKEN_MONEY, 2021, "E::Kerosine for flight %sD::Kauf von Kerosin f\xFCr Flug %s");
    StandardTexte.AddText(TOKEN_MONEY, 2022, "E::Meals for passengers for flight %sD::Essen f\xFCr Passagiere f\xFCr Flug %s");
    StandardTexte.UpdateText(TOKEN_MONEY, 2040, "E::Auctioning of branch officeD::Ersteigerung einer Niederlassung in %s");
    StandardTexte.UpdateText(TOKEN_MONEY, 2041, "E::Auctioning of routeD::Ersteigerung der Route %s");
    StandardTexte.UpdateText(TOKEN_MONEY, 2042, "E::Auctioning of gateD::Ersteigerung eines Gates");

    StandardTexte.UpdateText(TOKEN_MONEY, 2100, "E::(Balance of route %s)D::(Saldo f\xFCr Routenflug %s)");
    StandardTexte.UpdateText(TOKEN_MONEY, 2101, "E::(Balance of order %s)D::(Saldo f\xFCr Auftragsflug %s)");
    StandardTexte.UpdateText(TOKEN_MONEY, 2102, "E::(Balance of empty flight %s)D::(Saldo f\xFCr Leerflug %s)");
    StandardTexte.UpdateText(TOKEN_MONEY, 2103, "E::(Balance of cargo flight %s)D::(Saldo f\xFCr Frachtflug %s)");
    StandardTexte.UpdateText(TOKEN_MONEY, 3161, "E::Emission indemnifications (paid)D::Emissions-Entsch\xE4""digung (gezahlt)");
    StandardTexte.AddText(TOKEN_MONEY, 3163, "E::Emission indemnifications(received)D::Emissions-Entsch\xE4""digung (erhalten)");

    StandardTexte.UpdateText(TOKEN_AKTIE, 3030, "E::New account balance (incl. fee)D::Neuer Kontostand (inkl. Geb\xFChr)");

    StandardTexte.UpdateText(TOKEN_EXPERT, 2001, "E::SummaryD::Zusammenfassung");
    StandardTexte.UpdateText(TOKEN_EXPERT, 2002, "E::Daily balanceD::Tagesbilanz");
    StandardTexte.UpdateText(TOKEN_EXPERT, 2003, "E::Weekly balanceD::Wochenbilanz");
    StandardTexte.AddText(TOKEN_EXPERT, 2004, "E::Overall balanceD::Gesamtbilanz");
    StandardTexte.AddText(TOKEN_EXPERT, 2005, "E::Income through planesD::Flugzeugeinnahmen");
    StandardTexte.AddText(TOKEN_EXPERT, 2006, "E::Competitors (overview)D::Konkurrenz (\xDC""bersicht)");
    StandardTexte.AddText(TOKEN_EXPERT, 2007, "E:: SummariesD:: Zusammenfassungen");
    StandardTexte.AddText(TOKEN_EXPERT, 2008, "E:: Weekly balance of %sD:: Wochenbilanz von %s");
    StandardTexte.AddText(TOKEN_EXPERT, 2009, "E:: Weekly balance of %sD:: Wochenbilanz von %s");
    StandardTexte.AddText(TOKEN_EXPERT, 2010, "E:: Weekly balance of %sD:: Wochenbilanz von %s");
    StandardTexte.AddText(TOKEN_EXPERT, 2011, "E::Kerosene advisorD::Kerosinberater");
    StandardTexte.AddText(TOKEN_EXPERT, 3003, "E::You need a better qualified informer for this report.D::Sie brauchen einen h\xF6her qualifizierteren Informanten um diesen Bericht zu erhalten.");
    StandardTexte.AddText(TOKEN_EXPERT, 3004, "E::You need a better qualified kerosene advisor for this report.D::Sie brauchen einen h\xF6her qualifizierteren Kerosinberater um diesen Bericht zu erhalten.");

    StandardTexte.UpdateText(TOKEN_EXPERT, 3404, "E::OrdersD::Auftr\xE4ge");
    StandardTexte.AddText(TOKEN_EXPERT, 10000, "E::Kerosine for planeD::Kerosinkauf Flugzeug");
    StandardTexte.AddText(TOKEN_EXPERT, 10001, "E::Kerosine for stockD::Kerosinkauf Vorrat");
    StandardTexte.AddText(TOKEN_EXPERT, 10002, "E::Meals for passengersD::Essen f\xFCr Passagiere");
    StandardTexte.AddText(TOKEN_EXPERT, 10003, "E::Plane rework\r\ncargo/passengersD::Flugzeugumr\xFCstung\r\nFracht/Passagiere");
    StandardTexte.AddText(TOKEN_EXPERT, 10004, "E::Current loansD::Aktuelle Kredite");
    StandardTexte.AddText(TOKEN_EXPERT, 10005, "E::Loan repaymentD::Kredittilgung");
    StandardTexte.UpdateText(TOKEN_EXPERT, 3501, "E::Loan interestD::Kreditzinsen");
    StandardTexte.AddText(TOKEN_EXPERT, 10006, "E::Wealth taxD::Verm\xF6genssteuer");
    StandardTexte.AddText(TOKEN_EXPERT, 10007, "E::Theft (hacking)D::Diebstahl (Hacking)");
    StandardTexte.AddText(TOKEN_EXPERT, 10008, "E::CompensationD::Kompensation");
    StandardTexte.AddText(TOKEN_EXPERT, 10009, "E::paid to saboteurD::gezahlt an Saboteur");
    StandardTexte.AddText(TOKEN_EXPERT, 10010, "E::Fine for sabotageD::Strafe wegen Sabotage");
    StandardTexte.AddText(TOKEN_EXPERT, 10011, "E::Sabotage damageD::Schaden durch Sabotage");
    StandardTexte.AddText(TOKEN_EXPERT, 10012, "E::Bodyguard discountD::Bodyguard-Rabatt");
    StandardTexte.AddText(TOKEN_EXPERT, 10013, "E::received from\r\nplayersD::von Mitspielern\r\nerhalten");
    StandardTexte.AddText(TOKEN_EXPERT, 10014, "E::Other incomeD::Sonstige Einnahmen");
    StandardTexte.AddText(TOKEN_EXPERT, 10015, "E::Breakdown repair costD::Pannenreparatur");
    StandardTexte.AddText(TOKEN_EXPERT, 10016, "E::Security costD::Sicherheitskosten");
    StandardTexte.AddText(TOKEN_EXPERT, 10017, "E::Advertisement costD::Werbungskosten");
    StandardTexte.AddText(TOKEN_EXPERT, 10018, "E::sent to other\r\nplayersD::an Mitspieler\r\ngeschickt");
    StandardTexte.AddText(TOKEN_EXPERT, 10019, "E::Other expensesD::Sonstige Ausgaben");
    StandardTexte.AddText(TOKEN_EXPERT, 10020, "E::Stock saleD::Aktienverkauf");
    StandardTexte.AddText(TOKEN_EXPERT, 10021, "E::Stock emissionsD::Aktienemissionen");
    StandardTexte.AddText(TOKEN_EXPERT, 10022, "E::Emission indemnifications\r\n(received)D::Emissions-Entsch\xE4""digung\r\n(erhalten)");
    StandardTexte.AddText(TOKEN_EXPERT, 10023, "E::Stock purchaseD::Aktienkauf");
    StandardTexte.AddText(TOKEN_EXPERT, 10024, "E::Emission feesD::Emissions-Geb\xFChr");
    StandardTexte.AddText(TOKEN_EXPERT, 10025, "E::Emission indemnifications\r\n(received)D::Emissions-Entsch\xE4""digung\r\n(gezahlt)");
    StandardTexte.AddText(TOKEN_EXPERT, 10026, "E::Planes soldD::Flugzeugverkauf");
    StandardTexte.AddText(TOKEN_EXPERT, 10027, "E::TakeoversD::\xDC""bernahmen");
    StandardTexte.AddText(TOKEN_EXPERT, 10028, "E::Planes boughtD::Flugzeugkauf");
    StandardTexte.AddText(TOKEN_EXPERT, 10029, "E::Plane equipmentD::Flugzeugausr\xFCstung");
    StandardTexte.AddText(TOKEN_EXPERT, 10030, "E::Buying of\r\nbranch officesD::Kauf von\r\nNiederlassungen");
    StandardTexte.AddText(TOKEN_EXPERT, 10031, "E::Buying of routesD::Kauf von Routen");
    StandardTexte.AddText(TOKEN_EXPERT, 10032, "E::Buying of gatesD::Kauf von Gates");
    StandardTexte.AddText(TOKEN_EXPERT, 10033, "E::Kerosene tanksD::Kerosintanks");
    StandardTexte.AddText(TOKEN_EXPERT, 10034, "E::Branch office\r\nrental feesD::Mieten f\xFCr\r\nNiederlassungen");
    StandardTexte.AddText(TOKEN_EXPERT, 10035, "E::Route rental feesD::Mieten f\xFCr Routen");
    StandardTexte.AddText(TOKEN_EXPERT, 10036, "E::Credit interestD::HabenZinsen");
    StandardTexte.AddText(TOKEN_EXPERT, 10037, "E::Debit interestD::SollZinsen");
    StandardTexte.AddText(TOKEN_EXPERT, 10038, "E::Dividend receivedD::Dividende erhalten");
    StandardTexte.AddText(TOKEN_EXPERT, 10039, "E::Dividend paidD::Dividende gezahlt");

    StandardTexte.AddText(TOKEN_EXPERT, 10050, "E::Overall balance (1000 \x24)D::Bilanzsumme (1000 \x80)");
    StandardTexte.AddText(TOKEN_EXPERT, 10051, "E::Business operations (1000 \x24)D::Operatives Gesch\xE4""ft (1000 \x80)");
    StandardTexte.AddText(TOKEN_EXPERT, 10052, "E::Personal and rent (1000 \x24)D::Personal und Miete (1000 \x80)");
    StandardTexte.AddText(TOKEN_EXPERT, 10053, "E::Interest and loans (1000 \x24)D::Zinsen und Kredite (1000 \x80)");
    StandardTexte.AddText(TOKEN_EXPERT, 10054, "E::Stock trading (1000 \x24)D::Aktiengesch\xE4""fte (1000 \x80)");
    StandardTexte.AddText(TOKEN_EXPERT, 10055, "E::Expansion (1000 \x24)D::Expansion (1000 \x80)");
    StandardTexte.AddText(TOKEN_EXPERT, 10056, "E::Sabotage (1000 \x24)D::Sabotage (1000 \x80)");
    StandardTexte.AddText(TOKEN_EXPERT, 10057, "E::Other (1000 \x24)D::Sonstiges (1000 \x80)");

    StandardTexte.AddText(TOKEN_EXPERT, 10100, "E::Operating profitD::Operativer Gewinn");
    StandardTexte.AddText(TOKEN_EXPERT, 10101, "E::Operational lossD::Operativer Verlust");
    StandardTexte.AddText(TOKEN_EXPERT, 10102, "E::Operating balanceD::Operatives Saldo");
    StandardTexte.AddText(TOKEN_EXPERT, 10103, "E::proportion of op. profitD::Anteil an op. Gewinn");
    StandardTexte.AddText(TOKEN_EXPERT, 10150, "E::thereof business operationsD::davon operatives Gesch\xE4""ft");
    StandardTexte.AddText(TOKEN_EXPERT, 10151, "E::Weekly balance (1000 \x24)D::Wochenbilanz (1000 \x80)");
    StandardTexte.AddText(TOKEN_EXPERT, 10152, "E::Obligations (1000 \x24)D::Verpflichtungen");
    StandardTexte.AddText(TOKEN_EXPERT, 10153, "E::each week (1000 \x24)D::je Woche (1000 \x80)");

    StandardTexte.AddText(TOKEN_EXPERT, 10200, "E::Competitor does not exist anymoreD::Konkurrent existiert nicht mehr");
    StandardTexte.AddText(TOKEN_EXPERT, 10201, "E::%s considers us the strongest competitor!D::%s sieht uns als gr\xF6\xDFten Konkurrenten!");
    StandardTexte.AddText(TOKEN_EXPERT, 10202, "E::%s does not have enough gates!D::%s hat nicht gen\xFCgend Gates!");
    StandardTexte.AddText(TOKEN_EXPERT, 10203, "E::%s saves for a plane.D::%s spart aktuell auf ein Flugzeug.");
    StandardTexte.AddText(TOKEN_EXPERT, 10204, "E::%s wants to start flying routes.D::%s will anfangen, Routen zu fliegen.");

    StandardTexte.AddText(TOKEN_EXPERT, 10300, "E::TankD::Tank");
    StandardTexte.AddText(TOKEN_EXPERT, 10301, "E::Price/barrel (market)D::Preis/Barrel (Markt)");
    StandardTexte.AddText(TOKEN_EXPERT, 10302, "E::Price/barrel (tank)D::Preis/Barrel (Tank)");
    StandardTexte.AddText(TOKEN_EXPERT, 10303, "E::Value of tank (millions)D::Tankwert (Mio.)");
    StandardTexte.AddText(TOKEN_EXPERT, 10304, "E::Kerosine qualityD::Kerosinqualit\xE4t");
    StandardTexte.AddText(TOKEN_EXPERT, 10305, "E::top of the rangeD::Mega-Ultra");
    StandardTexte.AddText(TOKEN_EXPERT, 10306, "E::very goodD::sehr hoch");
    StandardTexte.AddText(TOKEN_EXPERT, 10307, "E::goodD::hoch");
    StandardTexte.AddText(TOKEN_EXPERT, 10308, "E::regularD::normal");
    StandardTexte.AddText(TOKEN_EXPERT, 10309, "E::moderateD::m\xE4\xDFig");
    StandardTexte.AddText(TOKEN_EXPERT, 10310, "E::badD::schlecht");
    StandardTexte.AddText(TOKEN_EXPERT, 10311, "E::very badD::sehr schlecht");
    StandardTexte.AddText(TOKEN_EXPERT, 10312, "E::Purchase qualityD::Einkaufsqualit\xE4t");
    StandardTexte.AddText(TOKEN_EXPERT, 10313, "E::cheapD::billig");
    StandardTexte.AddText(TOKEN_EXPERT, 10314, "E::Money saved (yesterday)D::Geld gespart (gestern)");
    StandardTexte.AddText(TOKEN_EXPERT, 10315, "E::Money saved (week)D::Geld gespart (Woche)");
    StandardTexte.AddText(TOKEN_EXPERT, 10316, "E::Money saved (overall)D::Geld gespart (gesamt)");
    StandardTexte.AddText(TOKEN_EXPERT, 10317, "E::Kerosene quality is disastrous! We should buy top of the range kerosene.D::Die Kerosinqualit\xE4t ist katastrohpal! Wir sollten hochwertiges Kerosin kaufen.");
    StandardTexte.AddText(TOKEN_EXPERT, 10318, "E::Kerosene quality is disastrous! We should buy %li barrel top of the range kerosene.D::Die Kerosinqualit\xE4t ist katastrohpal! Wir sollten %li Barrel hochwertiges Kerosin kaufen.");
    StandardTexte.AddText(TOKEN_EXPERT, 10319, "E::Kerosene quality is bad. We should buy top of the range kerosene.D::Die Kerosinqualit\xE4t ist schlecht. Wir sollten hochwertiges Kerosin kaufen.");
    StandardTexte.AddText(TOKEN_EXPERT, 10320, "E::Kerosene quality is bad. We should buy %li barrel top of the range kerosene.D::Die Kerosinqualit\xE4t ist schlecht. Wir sollten %li Barrel hochwertiges Kerosin kaufen.");
    StandardTexte.AddText(TOKEN_EXPERT, 10321, "E::Kerosene quality is way too good. We should save money and buy cheap kerosene.D::Die Kerosinqualit\xE4t ist viel zu gut. Wir sollten Geld sparen und billiges Kerosin kaufen.");
    StandardTexte.AddText(TOKEN_EXPERT, 10322, "E::Kerosene quality is way too good. We should save money and buy %li barrel of cheap kerosene.D::Die Kerosinqualit\xE4t ist viel zu gut. Wir sollten Geld sparen und %li Barrel billiges Kerosin kaufen.");
    StandardTexte.AddText(TOKEN_EXPERT, 10323, "E::Kerosene quality is fine. We could save money and buy cheap kerosene if our planes can support this.D::Die Kerosinqualit\xE4t ist okay. Wir k\xF6nnten Geld sparen und billiges Kerosin kaufen, wenn unsere Flugzeuge das noch vertragen.");
    StandardTexte.AddText(TOKEN_EXPERT, 10324, "E::Kerosene quality is fine. We could save money and buy %li barrel of cheap kerosene if our planes can support this.D::Die Kerosinqualit\xE4t ist okay. Wir k\xF6nnten Geld sparen und %li Barrel billiges Kerosin kaufen, wenn unsere Flugzeuge das noch vertragen.");
    StandardTexte.AddText(TOKEN_EXPERT, 10350, "E::Kerosene reportD::Kerosinbericht");

    /*SLONG tmpList[5] = {1, 2, 3, 5, 10};
    for (SLONG i = 0; i < 5; ++i) {
        DialogTexte.UpdateText(TOKEN_ARAB, 691+i, bprintf("[[P1\\MA\\141]]%li St\xFC""ck (=%%s \x80)", tmpList[i]));
    }*/
    for (SLONG i = 0; i < 4; ++i) {
        DialogTexte.UpdateText(TOKEN_ARAB, 900 + i,
                               bprintf("E::I will take the one with %lik gallons (\x24 %li).D::Ich nehme den mit %lik l (%li \x80).",
                    TankSize[i] / 1000, TankPrice[i], TankSize[i] / 1000, TankPrice[i]));
    }

    StandardTexte.AddText(TOKEN_JOBS, 2003, "E::Route advisorD::Routenberaterin");

    StandardTexte.UpdateText(TOKEN_MISC, 3201, "E::Opening hours.\xB5 \xB5Mon-Sat:10-19\xB5Sun:closedD::\xD6""ffnungszeiten.\xB5 \xB5Mo-Sa:10-19 Uhr\xB5So:geschlossen");

    StandardTexte.AddText(TOKEN_MISC, 1152, "E::Mission %li: %s (%s)D::Mission %li: %s (%s)");
    StandardTexte.AddText(TOKEN_MISC, 1153, "E::Free game: %s (%s)D::Freies Spiel: %s (%s)");
}
