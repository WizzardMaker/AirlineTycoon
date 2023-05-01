//============================================================================================
// Sim.cpp : Routinen zur allgemeinen Simulationsverwaltung:
//============================================================================================
#include "StdAfx.h"
#include "AtNet.h"
#include "Checkup.h"
#include "Sabotage.h"
#include <filesystem>
#include <numeric>

#define AT_Log(a,...) AT_Log_I("Sim", a, __VA_ARGS__)
                        // Für Menschen       Für Computer
                        // Money    Credit    Money    Credit
static long InitMoney[] = {1500000, 0,        2000000, 0,                                                        // DIFF_FREEGAME
                           5000000, 0,        200000,  0,                                                        // DIFF_TUTORIAL
                           5000000, 0,        500000,  0,                                                        // FIRST
                           3000000, 0,        1000000, 0,                                                        // DIFF_EASY
                           2000000, 0,        3000000, 0,                                                        // DIFF_NORMAL
                           1800000, 0,        4000000, 0,                                                        // DIFF_HARD
                           1500000, 0,        6000000, 0,                                                        // DIFF_FINAL
                           0,       0,        0,       0,        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Nada
                           3000000, 10000000, 1000000, 10000000,                                                 // DIFF_ADDON01
                           3000000, 0,        1000000, 0,                                                        // DIFF_ADDON02
                           3000000, 0,        1000000, 0,                                                        // DIFF_ADDON03
                           3000000, 0,        1000000, 0,                                                        // DIFF_ADDON04
                           3000000, 0,        1000000, 0,                                                        // DIFF_ADDON05
                           3000000, 0,        1000000, 0,                                                        // DIFF_ADDON06
                           3000000, 0,        1000000, 0,                                                        // DIFF_ADDON07
                           3000000, 0,        1000000, 0,                                                        // DIFF_ADDON08
                           3000000, 0,        1000000, 0,                                                        // DIFF_ADDON09
                           3000000, 0,        1000000, 0,                                                        // DIFF_ADDON10
                           3000000, 0,        3000000, 0,                                                        // DIFF_ATFS01
                           3000000, 0,        3000000, 0,                                                        // DIFF_ATFS02
                           3000000, 0,        3000000, 0,                                                        // DIFF_ATFS03
                           3000000, 0,        3000000, 0,                                                        // DIFF_ATFS04
                           3000000, 0,        3000000, 0,                                                        // DIFF_ATFS05
                           3000000, 0,        3000000, 0,                                                        // DIFF_ATFS06
                           3000000, 0,        3000000, 0,                                                        // DIFF_ATFS07
                           3000000, 0,        3000000, 0,                                                        // DIFF_ATFS08
                           3000000, 0,        3000000, 0,                                                        // DIFF_ATFS09
                           3000000, 0,        3000000, 0};                                                       // DIFF_ATFS10

static SLONG MonthLength[] = {31, 28, 31, 30, 31, 30, 30, 31, 30, 31, 30, 31};

char chRegKey[] = R"(Software\Spellbound Software\Airline Tycoon Deluxe\1.0)";
char chRegKeyOld[] = R"(Software\Spellbound Software\Airline Tycoon Evolution\1.0)";
// char chRegKeyOld[] = "Software\\Spellbound Software\\Airline Tycoon FirstClass\\1.0";

extern SLONG NewgameWantsToLoad;
extern SLONG gTimerCorrection;

const char TOKEN_NEWGAME[] = "NewG";

void CalcPlayerMaximums(bool bForce);

// Daten des aktuellen Savegames beim laden:
SLONG SaveVersion = 1;
SLONG SaveVersionSub = 202;

//Öffnungszeiten:
extern SLONG timeDutyOpen;
extern SLONG timeDutyClose;
extern SLONG timeArabOpen;
extern SLONG timeLastClose;
extern SLONG timeMuseOpen;
extern SLONG timeReisClose;
extern SLONG timeMaklClose;
extern SLONG timeWerbOpen;

extern SLONG PlayerMaxPassagiere;
extern SLONG PlayerMaxLength;
extern SLONG PlayerMinPassagiere;
extern SLONG PlayerMinLength;

extern TEAKRAND *pSurvisedRandom1;
extern TEAKRAND *pSurvisedRandom2;

bool bgIsLoadingSavegame = false;

// Zum Debuggen:
extern SLONG rChkTime;
extern ULONG rChkPersonRandCreate, rChkPersonRandMisc, rChkHeadlineRand;
extern ULONG rChkLMA, rChkRBA, rChkAA[MAX_CITIES], rChkFrachen;
extern SLONG rChkGeneric, CheckGeneric;
extern SLONG rChkActionId[5 * 4];

extern SLONG GenericSyncIds[4];
extern SLONG GenericSyncIdPars[4];
extern SLONG GenericAsyncIds[4 * 100];
extern SLONG GenericAsyncIdPars[4 * 100];

void DumpAASeedSum(SLONG /*CallerId*/);
#ifdef _DEBUG
void DumpAASeedSum(SLONG CallerId) {
    __int64 sum = 0;
    for (SLONG c = 0; c < MAX_CITIES; c++)
        sum += AuslandsAuftraege[c].Random.GetSeed();

    AT_Log("AA Seed sum for %li is %lli\n", CallerId, sum);
}
#else
void DumpAASeedSum(SLONG /*CallerId*/) {}
#endif

//--------------------------------------------------------------------------------------------
// Sorgt für Screen Refresh und für Ablauf der Simulation:
//--------------------------------------------------------------------------------------------
SIM::SIM() {
    PersonRandMisc.SRand(0);
    PersonRandCreate.SRand(0);
    HeadlineRand.SRand(0);

    Jahreszeit = -1;

    // Variableninhalt ist zufällig:
    bReloadAirport = 0;
    Gamestate = GAMESTATE_BOOT;
    QuitCountDown = 0;
    Time = 9 * 60000;
    GameSpeed = 30; // ex:51;
    bPause = FALSE;
    bNoTime = FALSE;
    CallItADay = FALSE;
    bNetwork = 0;
    localPlayer = 0;

    bAllowCheating = TRUE;

    RoomBusy.ReSize(256);
    RoomBusy.FillWith(0);

    gDisablePauseKey = FALSE;

    Options.OptionDigiSound = TRUE;
    LoadOptions();

    TafelData.Clear();
}

//--------------------------------------------------------------------------------------------
// Destruktor:
//--------------------------------------------------------------------------------------------
SIM::~SIM() { SaveOptions(); }

//--------------------------------------------------------------------------------------------
// Fügt einen Smacker im Flughafen hinzu:
//--------------------------------------------------------------------------------------------
void SIM::AddSmacker(const CString &Filename, SLONG BrickId, XY Offset) {
    AirportSmacks.ReSize(AirportSmacks.AnzEntries() + 1);
    AirportSmacks[AirportSmacks.AnzEntries() - 1].Open(Filename);
    AirportSmacks[AirportSmacks.AnzEntries() - 1].BrickId = BrickId;
    AirportSmacks[AirportSmacks.AnzEntries() - 1].Offset = Offset;
}

//--------------------------------------------------------------------------------------------
// Einige Leute gehen am Flughafen einkaufen
//--------------------------------------------------------------------------------------------
void SIM::AddNewShoppers() {
    if (((Time > 9 * 60000 && Time <= 12 * 60000) || (Time >= 14 * 60000 && Time <= 16 * 60000)) && CheckIn != -1 && Shops != -1 && Editor == EDITOR_NONE) {

        CheckGeneric++;

        if (Persons.GetNumShoppers() < 70 && bNoTime == FALSE && bPause == FALSE) {
            UBYTE CustomerId = Clans.GetCustomerId(0, 99);
            XY Birthplace = Airport.GetRandomBirthplace(0, 0, &PersonRandCreate);

            if (Persons.GetNumFree() > 100 && Persons.GetNumUsed() < 100) {
                PersonQueue.AddPerson(CustomerId, Birthplace, REASON_SHOPPING, 0, 0, 0);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Fügt Flugpassagiere hinzu:
//--------------------------------------------------------------------------------------------
void SIM::AddNewPassengers() {
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;
    SLONG n = 0;
    CFlugplan *Plan = nullptr;

    // Für alle Spieler:
    for (c = 0; c < Players.AnzPlayers; c++) {
        // Für alle Flugzeuge des Spielers:
        if (Players.Players[c].IsOut == 0) {
            for (d = 0; d < Players.Players[c].Planes.AnzEntries(); d++) {
                if (Players.Players[c].Planes.IsInAlbum(d) != 0) {
                    Plan = &Players.Players[c].Planes[d].Flugplan;

                    // Für alle Flüge des Flugzeuges:
                    e = Players.Players[c].Planes[d].Flugplan.NextStart;

                    if (e != -1) {
                        CFlugplanEintrag &qFPE = Plan->Flug[e];

                        // Dadurch wird's unten kompakter:
                        // Eintrag vorhanden?
                        if (qFPE.ObjectType == 1 || qFPE.ObjectType == 2) {
                            if (qFPE.Startzeit - 1 == GetHour() && qFPE.Startdate == Date && qFPE.PArrived < qFPE.Passagiere + qFPE.PassagiereFC &&
                                qFPE.Gate != -1 && qFPE.VonCity == static_cast<ULONG>(HomeAirportId)) {
                                for (n = 0; n < 2; n++) {
                                    if (PersonRandCreate.Rand(6) == 0 || GetMinute() >= 20) {
                                        // Endlich werden Leute hinzugefügt:
                                        if (Persons.GetNumFree() > 2) {
                                            if (c == localPlayer && Tutorial == 1600 && (IsTutorial != 0)) {
                                                XY p;
                                                XY pp;

                                                p = Airport.GetRandomBirthplace(0, 0, &PersonRandCreate);

                                                for (SLONG f = 0; f < 100; f++) {
                                                    pp = Airport.GetRandomBirthplace(0, 0, &PersonRandCreate);
                                                    if (pp.x < p.x) {
                                                        p = pp;
                                                    }
                                                }

                                                PersonQueue.AddPerson(Clans.GetCustomerId(0, PersonRandCreate.Rand(2)), p, REASON_FLYING,
                                                                      UBYTE(c),                                    // Airline
                                                                      Players.Players[c].Planes.GetIdFromIndex(d), // PlaneId
                                                                      UBYTE(e),                                    // FlightIndex
                                                                      UBYTE(MoodPersonTicket));                    // Mood(neu)

                                                Tutorial = 1602;
                                                Players.Players[localPlayer].Messages.NextMessage();
                                                Players.Players[localPlayer].Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1602));
                                            } else {
                                                PersonQueue.AddPerson(
                                                    Clans.GetCustomerId(0, PersonRandCreate.Rand(2)), Airport.GetRandomBirthplace(0, 0, &PersonRandCreate),
                                                    REASON_FLYING,
                                                    UBYTE(c),                                                                           // Airline
                                                    Players.Players[c].Planes.GetIdFromIndex(d),                                        // PlaneId
                                                    UBYTE(e),                                                                           // FlightIndex
                                                    UBYTE((qFPE.PArrived < qFPE.PassagiereFC) ? MoodPersonTicketFC : MoodPersonTicket), // Mood(neu)
                                                    UBYTE(qFPE.PArrived < qFPE.PassagiereFC));                                          // FirstClass?
                                            }
                                        }

                                        if (qFPE.PArrived < 100) {
                                            qFPE.PArrived += CUSTOMERS_PER_PERSON;
                                        } else {
                                            qFPE.PArrived += CUSTOMERS_PER_PERSONB;
                                        }

                                        if (qFPE.PArrived > qFPE.Passagiere + qFPE.PassagiereFC) {
                                            qFPE.PArrived = qFPE.Passagiere + qFPE.PassagiereFC;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
// Fügt dem ganzen eine Stinkbombe hinzu:
//------------------------------------------------------------------------------
void SIM::AddStenchSabotage(XY Position) {
    SLONG Id = Persons *= PERSON(UBYTE(Clans.GetAnimationId(20)), Position, REASON_SHOPPING, -1, -1, -1, UBYTE(MoodPersonNone));

    PERSON &qNewPerson = Persons[Id];

    for (SLONG c = 0; c < 5; c++) {
        if (Smokers[c].Smoke.AnzEntries() == 0) {
            Smokers[c].Smoke.ReSize(20);
            Smokers[c].Smoking = 33 * 60 * 2;
            qNewPerson.State = UBYTE(c);
            break;
        }
    }
}

//------------------------------------------------------------------------------
// Fügt dem ganzen etwas Klebstoff hinzu:
//------------------------------------------------------------------------------
bool SIM::AddGlueSabotage(XY Position, SLONG Dir, SLONG NewDir, SLONG Phase) {
    XY ArrayPos;

    ArrayPos.x = Position.x / 44;
    ArrayPos.y = (Position.y - 5000) / 22;

    bool okay = false;
    if (NewDir == 8 && Dir == 8) {
        switch (Phase) {
        case 0:
            if ((Airport.iPlate[ArrayPos.y + (ArrayPos.x << 4)] & 128) != 0) {
                ArrayPos.y--;
                okay = true;
            }
            break;

        case 1:
            if ((Airport.iPlate[ArrayPos.y + (ArrayPos.x << 4)] & 64) != 0) {
                ArrayPos.x++;
                okay = true;
            }
            break;

        case 2:
            if ((Airport.iPlate[ArrayPos.y + (ArrayPos.x << 4)] & 32) != 0) {
                ArrayPos.y++;
                okay = true;
            }
            break;

        case 3:
            if ((Airport.iPlate[ArrayPos.y + (ArrayPos.x << 4)] & 16) != 0) {
                ArrayPos.x--;
                okay = true;
            }
            break;

        default:
            break;
        }
    }

    if (okay) {
        Persons *=
            PERSON(UBYTE(Clans.GetAnimationId(10)), XY(ArrayPos.x * 44, ArrayPos.y * 22 + 5000), UBYTE(REASON_SHOPPING), -1, -1, -1, UBYTE(MoodPersonNone));
    }

    return (okay);
}

//------------------------------------------------------------------------------
// Läßt den Benutzer Schwierigkeit, Anfangsjahr und Spieler wählen:
//------------------------------------------------------------------------------
void SIM::ChooseStartup(BOOL /*GameModeQuick*/) {
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;

    DumpAASeedSum(1000);

    bCheatedSession = 0;

    PersonRandMisc.SRand(0);
    PersonRandCreate.SRand(0);
    HeadlineRand.SRand(0);

    LastMinuteAuftraege.Random.SRand(0);
    ReisebueroAuftraege.Random.SRand(0);
    gFrachten.Random.SRand(0);
    for (c = 0; c < MAX_CITIES; c++) {
        AuslandsAuftraege[c].Random.SRand(0);
    }
    for (c = 0; c < MAX_CITIES; c++) {
        AuslandsFrachten[c].Random.SRand(0);
    }

    rChkTime = rChkPersonRandCreate = rChkPersonRandMisc = rChkHeadlineRand = 0;
    rChkLMA = rChkRBA = rChkFrachen = rChkGeneric = CheckGeneric = 0;

    for (c = 0; c < MAX_CITIES; c++) {
        rChkAA[c] = 0;
    }
    for (c = 0; c < 5 * 4; c++) {
        rChkActionId[c] = 0;
    }
    for (c = 0; c < 4; c++) {
        GenericSyncIds[c] = GenericSyncIdPars[c] = 0;
    }
    for (c = 0; c < 4 * 100; c++) {
        GenericAsyncIds[c] = GenericAsyncIdPars[c] = 0;
    }

    PlayerMaxPassagiere = 90;
    PlayerMaxLength = 2000000;
    PlayerMinPassagiere = 90;
    PlayerMinLength = 2000000;

    gTimerCorrection = 0;

    bCompatibleRoutes = 0;
    UniqueGameId2 = ((AtGetTime() ^ DWORD(rand() % 30000) ^ gMousePosition.x ^ gMousePosition.y) & 0x7fffffff);

    // Das Referenzflugzeug
    PlanePropId = -1;
    RFActivePage = 100;
    RFBegleiterFaktor = 1.0;
    RFSitzeRF = 0;
    RFEssen = 0;
    RFTabletts = 0;
    RFDeco = 0;
    RFTriebwerk = 0;
    RFReifen = 0;
    RFElektronik = 0;
    RFSicherheit = 0;
    CallItADayAt = 0;

    Headlines.IsInteresting = FALSE;
    TicksPlayed = 0;
    NumRestored = 0;
    TicksSinceLoad = 0;
    bExeChanged = FALSE;
    ExpandAirport = FALSE;

    ItemGlove = 1;
    ItemClips = 1;
    ItemGlue = 0;
    ItemKohle = 1;
    ItemParfuem = 1;
    ItemZange = 1;

    ItemPostcard = 1;

    for (c = 0; c < 5; c++) {
        Smokers[c].Smoke.ReSize(0);
    }

    Players.AnzPlayers = 4;
    Tutorial = 0;

    // Bei Netzwerkspielen wird die Starttime schon in NewGamePopup erstellt:
    if (bNetwork == 0) {
        if (Options.OptionRandomStartday != 0) {
            StartTime = (rand() % 365) * 60 * 60 * 24;
        } else {
            StartTime = time(nullptr);
        }
    }

    Date = 0;
    Month = 1;
    MonthDay = 1;
    Slimed = -1;
    nSecOutDays = 0;

    UpdateSeason();

    TEAKRAND LocalRand(StartTime);

    MoneyInBankTrash = static_cast<SLONG>((LocalRand.Rand(100)) > 75);
    FocusPerson = -1;

    struct tm *pTimeStruct = localtime(&StartTime);
    StartWeekday = ((pTimeStruct->tm_wday + 6) % 7);

    LastExpansionDate = 0;
    SabotageActs.ReSize(0);

    IsTutorial = static_cast<BOOL>(Difficulty == DIFF_TUTORIAL);
    DialogOvertureFlags = 0;

    // Wochentag für die Öffnungszeiten:
    Weekday = StartWeekday;

    Players.Players.ReSize(Players.AnzPlayers);
    for (c = 0; c < Players.AnzPlayers; c++) {
        Players.Players[c].PlayerNum = c;
    }

    for (c = 0; c < 4; c++) {
        Players.Players[c].PlayerWalkRandom.SRand(0);
    }
    for (c = 0; c < 4; c++) {
        Players.Players[c].PlayerExtraRandom.SRand(0);
    }

    for (c = 0; c < Players.AnzPlayers; c++) {
        PLAYER &qPlayer = Players.Players[c];

        if (bNetwork != 0) {
            qPlayer.bReadyForMorning = 0;
            qPlayer.bReadyForBriefing = 0;
        }

        qPlayer.LaptopBattery = 0;
        qPlayer.LaptopQuality = 0;
        qPlayer.ReferencePlane = -1;
        qPlayer.ImageGotWorse = 0;
        qPlayer.StandStillSince = 0;
        qPlayer.SickTokay = FALSE;
        qPlayer.RunningToToilet = FALSE;
        qPlayer.Stunned = 0;
        qPlayer.OfficeState = 0;
        qPlayer.PlayerSmoking = 0;
        qPlayer.LaptopVirus = 0;
        qPlayer.GlobeFileOpen = 0;
        qPlayer.WalkToGlobe = 0;
        qPlayer.IsStuck = 0;

        qPlayer.StrikeHours = 0;
        qPlayer.StrikeNotified = 0;
        qPlayer.StrikeEndCountdown = 0;
        qPlayer.StrikeEndType = 0;
        qPlayer.DaysWithoutStrike = 20;
        qPlayer.DaysWithoutSabotage = 0;
        qPlayer.StrikePlanned = 0;
        qPlayer.TrinkerTrust = 0;
        qPlayer.SeligTrust = 0;
        qPlayer.SpiderTrust = 0;
        qPlayer.WerbungTrust = 0;
        qPlayer.DutyTrust = 0;
        qPlayer.HasAlkohol = TRUE;
        qPlayer.Koffein = 0;
        qPlayer.IsDrunk = 0;
        qPlayer.SecurityFlags = 0;
        qPlayer.SecurityNeeded = 0;
        qPlayer.WaitWorkTill = -1;
        qPlayer.WaitWorkTill2 = Time;

        qPlayer.LocationTime = -1;
        qPlayer.LocationForbidden = -1;
        qPlayer.LocationForbiddenTime = -1;

        switch (Difficulty) {
        case DIFF_TUTORIAL:
        case DIFF_FIRST:
        case DIFF_EASY:
            qPlayer.SollZins = 5;
            qPlayer.HabenZins = 5;
            break;

        case DIFF_NORMAL:
            qPlayer.SollZins = 6;
            qPlayer.HabenZins = 4;
            break;

        case DIFF_FREEGAME:
        case DIFF_HARD:
            qPlayer.SollZins = 10;
            qPlayer.HabenZins = 3;
            break;

        case DIFF_FINAL:
            qPlayer.SollZins = 15;
            qPlayer.HabenZins = 1;
            break;

        case DIFF_ADDON01:
        case DIFF_ADDON02:
        case DIFF_ADDON03:
        case DIFF_ADDON04:
        case DIFF_ADDON05:
        case DIFF_ADDON06:
        case DIFF_ADDON07:
        case DIFF_ADDON08:
        case DIFF_ADDON09:
        case DIFF_ADDON10:
            qPlayer.SollZins = 15;
            qPlayer.HabenZins = 1;
            break;

        case DIFF_ATFS01:
        case DIFF_ATFS02:
        case DIFF_ATFS03:
        case DIFF_ATFS04:
        case DIFF_ATFS05:
        case DIFF_ATFS06:
        case DIFF_ATFS07:
        case DIFF_ATFS08:
        case DIFF_ATFS09:
        case DIFF_ATFS10:
            qPlayer.SollZins = 15;
            qPlayer.HabenZins = 0;
            break;

        default:
            TeakLibW_Exception(FNL, ExcNever);
            break;
        }
    }

    gDisablePauseKey = FALSE;

    LeftEnd = CheckIn = Office = Entry = Shops = Cafe = Security = Suitcase = WaitZone = RightEnd = 1;

    bNoTime = TRUE;
    Time = 9 * 60000;
    TimeSlice = 0;

    PersonRandMisc.SRand(Date);
    PersonRandCreate.SRand(Date);

    Kerosin = 500;
    KerosinPast.ReSize(10);
    KerosinPast[9] = 508;

    TEAKRAND KerosinRand(Date);
    for (c = KerosinPast.AnzEntries() - 2; c >= 0; c--) {
        KerosinPast[c] =
            KerosinPast[c + 1] + (KerosinRand.Rand(20)) - 10 - SLONG(sin((Date - c + KerosinRand.Rand(6)) / 3.0) * 20) - SLONG(sin((Date - c) * 1.7) * 20);
    }

    PersonQueue.NewDay();

    Workers.ReInit("piloten.csv", "names.csv");
    Workers.NewDay();
    Workers.EnsureBerater(BERATERTYP_PERSONAL);
    Workers.EnsureBerater(BERATERTYP_AUFTRAG);
    Workers.EnsureBerater(BERATERTYP_FLUGZEUG);

    if (Options.OptionAirport == -1) {
        Options.OptionAirport = Cities.GetIdFromName("Berlin");
        if (gLanguage == LANGUAGE_E) {
            Options.OptionAirport = Cities.GetIdFromName("London");
        }
        if (gLanguage == LANGUAGE_F) {
            Options.OptionAirport = Cities.GetIdFromName("Paris");
        }
    }
    HomeAirportId = Options.OptionAirport;

    if (Difficulty == DIFF_NORMAL) {
        CreateMissionCities();
    }
    if (Difficulty == DIFF_ADDON03) {
        CreateEarthquakeCity();
    }

    Overtake = 0;
    RoomBusy.FillWith(0);

    UsedTelescope = FALSE;
    UsedPlaneProp2 = FALSE;
    DontDisplayPlayer = -1;
    ShowExtrablatt = -1;

    TickReisebueroRefill = TickLastMinuteRefill = TickMuseumRefill = TickFrachtRefill = 6;

    // Bedarf der Leute initialisieren:
    Routen.NewDay();

    // Statistik-Screen:
    {
        for (c = 0; c < Players.AnzPlayers; c++) {
            StatplayerMask[c] = true;
        }

        Statgroup = 0;
        StatfGraphVisible = false;
        Statdays = 7;
        StatnewDays = 7;
        DropDownPosY = 0;

        for (auto &p : StatiArray) {
            for (auto &i : p) {
                i = true;
            }
        }
    }

    // Daten der Spieler initialisieren:
    for (c = 0; c < Players.AnzPlayers; c++) {
        PLAYER &qPlayer = Players.Players[c];

        qPlayer.Messages.PlayerNum = c;

        qPlayer.Statistiken.ReSize(STAT_ANZ);
        qPlayer.PlayerWalkRandom.SRand(0);
        qPlayer.PlayerExtraRandom.SRand(0);

        for (d = 0; d < STAT_ANZ; d++) {
            qPlayer.Statistiken[d].Init();
        }

        qPlayer.IsOut = FALSE;
        qPlayer.NumFlights = 0;
        qPlayer.RoutePage = 0;
        qPlayer.StandCount = 0;
        qPlayer.ImageGotWorse = FALSE;
        qPlayer.RentCities.RentCities.ReSize(0);
        qPlayer.RentCities.RentCities.ReSize(Cities.AnzEntries());
        qPlayer.RentCities.RentCities[Cities(HomeAirportId)].Rang = 1;
        qPlayer.RentCities.RentCities[Cities(HomeAirportId)].Miete = Cities[HomeAirportId].BuroRent;

        qPlayer.CalledCities.ReSize(Cities.AnzEntries());
        qPlayer.CalledCities.FillWith(0);

        qPlayer.Locations.fill(0);
        qPlayer.TopLocation = ROOM_AIRPORT;

        qPlayer.RentRouten.RentRouten.ReSize(0);
        qPlayer.RentRouten.RentRouten.ReSize(Routen.AnzEntries());

        for (d = 0; d < Players.AnzPlayers; d++) {
            qPlayer.DisplayRoutes[d] = FALSE;
            qPlayer.DisplayPlanes[d] = FALSE;
        }
        qPlayer.DisplayPlanes[c] = TRUE;

        qPlayer.Gates.Gates.ReSize(30);
        qPlayer.Gates.NumRented = 0;
        for (d = 0; d < qPlayer.Gates.Gates.AnzEntries(); d++) {
            qPlayer.Gates.Gates[d].Miete = -1;
        }

        qPlayer.RentGate(c, 1000);

        // Statussymbole:
        qPlayer.KerosinQuali = 1.0;
        qPlayer.KerosinKind = 1; // Normales Kerosin
        qPlayer.Tank = 100;
        qPlayer.TankOpen = FALSE;
        qPlayer.TankInhalt = 0;
        qPlayer.TankPreis = 0;
        qPlayer.ArabTrust = 0;
        qPlayer.ArabOpfer = -1;
        qPlayer.ArabOpfer2 = -1;
        qPlayer.ArabPlane = -1;
        qPlayer.ArabMode = 0;
        qPlayer.ArabMode2 = 0;
        qPlayer.ArabMode3 = 0;
        qPlayer.ArabHints = 0;
        qPlayer.MechTrust = 0;
        qPlayer.GlobeOiled = FALSE;
        qPlayer.WerbeBroschuere = -1;
        qPlayer.TelephoneDown = FALSE;
        qPlayer.Presseerklaerung = FALSE;

        qPlayer.MechAngry = 0;
        qPlayer.MechMode = 2; // Karl, Mechaniker
        qPlayer.EarthAlpha = UWORD((Cities[HomeAirportId].GlobusPosition.x + 170) * (3200 / 18) - 16000 + 1300);

        if (Difficulty != DIFF_ADDON06) {
            qPlayer.Image = 0;
        } else {
            qPlayer.Image = 300;
        }

        if ((Difficulty == DIFF_ATFS09 || Difficulty == DIFF_ATFS10) && qPlayer.Owner == 1) {
            qPlayer.Tank = 5000;
            qPlayer.TankOpen = 1;
        }

        qPlayer.HasFlownRoutes = 0;
        qPlayer.NumPassengers = 0;
        qPlayer.NumAuftraege = 0;
        qPlayer.Gewinn = 0;
        qPlayer.ConnectFlags = 0;
        qPlayer.RocketFlags = 0;
        qPlayer.LastRocketFlags = 0;
        qPlayer.SpeedCount = 0;
        qPlayer.bWasInMuseumToday = FALSE;
        qPlayer.bHasPlanesUpgradedToday = FALSE;
        qPlayer.NumFracht = 0;
        qPlayer.NumFrachtFree = 0;
        qPlayer.NumMiles = 0;
        qPlayer.NumServicePoints = 0;
        qPlayer.NumOrderFlights = 0;
        qPlayer.TalkedToNasa = FALSE;
        qPlayer.NumOrderFlightsToday = 0;
        qPlayer.NumOrderFlightsToday2 = 0;

        qPlayer.Planes.ClearAlbum();

        // Die Flugzeuge kaufen:
        {
            TEAKRAND localRand;
            localRand.SRand(1);
            switch (Difficulty) {
            case DIFF_EASY: // Mission: 2500 Passagiere befördern
                qPlayer.BuyPlane(100, &localRand);
                qPlayer.BuyPlane(114, &localRand);
                break;

            case DIFF_NORMAL: // Mission: Berlin mit New York, Rio, etc. verbinden
                qPlayer.BuyPlane(100, &localRand);
                qPlayer.BuyPlane(104, &localRand);
                break;

            default:
                qPlayer.BuyPlane(100, &localRand);
                qPlayer.BuyPlane(105, &localRand);
                break;
            }
        }

        for (d = 0; d < SLONG(qPlayer.Planes.AnzEntries()); d++) {
            if (qPlayer.Planes.IsInAlbum(d) != 0) {
                qPlayer.Planes[d].Sponsored = TRUE;

                // Add-On Mission #1 : Schlechteflugzeuge
                if (Difficulty == DIFF_ADDON01) {
                    qPlayer.Planes[d].Zustand = qPlayer.Planes[d].WorstZustand = qPlayer.Planes[d].TargetZustand = 60;
                }

                // Add-On Mission #7 : Schrottflugzeuge
                if (Difficulty == DIFF_ADDON07) {
                    qPlayer.Planes[d].Zustand = qPlayer.Planes[d].WorstZustand = qPlayer.Planes[d].TargetZustand = 35;
                }
            }
        }

        qPlayer.Auftraege.ReSize(40);

        CAuftrag a;
        CalcPlayerMaximums(true);
        qPlayer.Auftraege.ClearAlbum();
        qPlayer.Frachten.ClearAlbum();

        if (GlobalUse(USE_TRAVELHOLDING) && Difficulty != DIFF_ADDON09) {
            if (bNetwork == 0) {
                qPlayer.Auftraege.Random.SRand(AtGetTime());
            }

            a.RefillForBegin(0, &qPlayer.Auftraege.Random);
            qPlayer.Auftraege += a;
            a.RefillForBegin(0, &qPlayer.Auftraege.Random);
            qPlayer.Auftraege += a;
            qPlayer.Statistiken[STAT_AUFTRAEGE].AddAtPastDay(2);
        }

        if (Difficulty >= DIFF_ATFS01 && Difficulty <= DIFF_ATFS10) {
            qPlayer.Money = InitMoney[(Difficulty - DIFF_ATFS01 + 22) * 4 + 0];
            qPlayer.Bonus =
                InitMoney[(Difficulty - DIFF_ATFS01 + 22) * 4 + 0 + static_cast<SLONG>(qPlayer.Owner == 1) * 2] - InitMoney[(Difficulty - DIFF_ATFS01 + 22) * 4];
            qPlayer.Credit = InitMoney[(Difficulty - DIFF_ATFS01 + 22) * 4 + 1];
        } else {
            qPlayer.Money = InitMoney[(Difficulty + 1) * 4 + 0];
            qPlayer.Bonus = InitMoney[(Difficulty + 1) * 4 + 0 + static_cast<SLONG>(qPlayer.Owner == 1) * 2] - InitMoney[(Difficulty + 1) * 4];
            qPlayer.Credit = InitMoney[(Difficulty + 1) * 4 + 1];
            if (qPlayer.Bonus < 0) {
                qPlayer.Bonus = -qPlayer.Bonus;
            }
        }

        qPlayer.MoneyPast.ReSize(20);
        qPlayer.MoneyPast.FillWith(0);

        qPlayer.History.ReInit();
        qPlayer.History.HistoricMoney = qPlayer.Money;

        qPlayer.Bilanz.Clear();
        qPlayer.BilanzGestern.Clear();
        qPlayer.BilanzGesamt.Clear();
        qPlayer.BilanzWoche.Clear();

        for (d = 0; d < STAT_ANZ; d++) {
            qPlayer.Statistiken[d].Init();
        }

        // Aktien:
        for (d = 0; d < 4; d++) {
            qPlayer.AktienWert[d] = 0;
        }
        for (d = 0; d < 10; d++) {
            qPlayer.Kurse[d] = 100;
        }
        for (d = 0; d < 4; d++) {
            qPlayer.OwnsAktien[d] = 0;
        }

        qPlayer.AnzAktien = 10000;
        qPlayer.MaxAktien = 10000 * 2;
        qPlayer.Dividende = 10;
        qPlayer.TrustedDividende = 10;
        qPlayer.OwnsAktien[c] = qPlayer.AnzAktien * 8 / 10;
        qPlayer.AktienWert[c] = SLONG(qPlayer.Kurse[0] * qPlayer.OwnsAktien[c]);
        qPlayer.IsTalking = 0;
        qPlayer.IsWalking2Player = -1;
        qPlayer.IslandViewPos = XY(320, 0);
        qPlayer.CalledPlayer = 0;
        qPlayer.BoredOfPlayer = FALSE;

        qPlayer.WasInRoom.ReSize(256);
        qPlayer.WasInRoom.FillWith(0);
        SetRoomVisited(c, ROOM_WORLD);
        SetRoomVisited(c, 254); // Spieler-Konkurrenten in Telefonbuch

        // Inventar:
        qPlayer.Items.ReSize(6);
        for (d = 0; d < 6; d++) {
            qPlayer.Items[d] = 0xff;
        }

        qPlayer.CallItADay = FALSE;
        LaptopSoldTo = -1;

        qPlayer.Messages.NewDay();
        qPlayer.UpdateWalkSpeed();
        qPlayer.Letters.Clear();

        if (qPlayer.Owner == 0 && (qPlayer.IsOut == 0)) {
            qPlayer.Letters.AddLetter(TRUE, StandardTexte.GetS(TOKEN_LETTER, 1500), StandardTexte.GetS(TOKEN_LETTER, 1501),
                                      StandardTexte.GetS(TOKEN_LETTER, 1502), -1);
        }

        if (Difficulty == DIFF_FINAL) { // Raumfahrt:
            if (qPlayer.Owner == 0 && (qPlayer.IsOut == 0)) {
                qPlayer.Letters.AddLetter(TRUE, StandardTexte.GetS(TOKEN_LETTER, 1200), StandardTexte.GetS(TOKEN_LETTER, 1201),
                                          StandardTexte.GetS(TOKEN_LETTER, 1202), -1);
            }
        }

        if (qPlayer.Owner == 0) {
            localPlayer = c;
            qPlayer.GameSpeed = 0;
        } else if (qPlayer.Owner == 2) {
            qPlayer.GameSpeed = 0;
        } else {
            qPlayer.GameSpeed = 3;
        }

        // Piloten und Flugbegleiter einstellen (nur bei Menschen):
        if (qPlayer.Owner != 1 || !qPlayer.RobotUse(ROBOT_USE_FAKE_PERSONAL)) {
            SLONG Anzahl = 0;
            TEAKRAND LocalRand(StartTime + c);

            // Piloten:
            for (Anzahl = d = 0; d < qPlayer.Planes.AnzEntries(); d++) {
                if (qPlayer.Planes.IsInAlbum(d) != 0) {
                    Anzahl += qPlayer.Planes[d].ptAnzPiloten;
                }
            }
            // Anzahl+=PlaneTypes[qPlayer.Planes[d].TypeId].AnzPiloten;

            for (d = 0; d < Anzahl; d++) {
                do {
                    e = LocalRand.Rand(Workers.Workers.AnzEntries());
                } while (Workers.Workers[e].Employer != WORKER_RESERVE || Workers.Workers[e].Typ != WORKER_PILOT);

                Workers.Workers[e].Employer = c;
                Workers.Workers[e].PlaneId = -1;
            }

            // Flugbegleiter:
            for (Anzahl = d = 0; d < qPlayer.Planes.AnzEntries(); d++) {
                if (qPlayer.Planes.IsInAlbum(d) != 0) {
                    Anzahl += qPlayer.Planes[d].ptAnzBegleiter;
                }
            }
            // Anzahl+=PlaneTypes[qPlayer.Planes[d].TypeId].AnzBegleiter;

            for (d = 0; d < Anzahl; d++) {
                do {
                    e = LocalRand.Rand(Workers.Workers.AnzEntries());
                } while (Workers.Workers[e].Employer != WORKER_RESERVE || Workers.Workers[e].Typ != WORKER_STEWARDESS);

                Workers.Workers[e].Employer = c;
                Workers.Workers[e].PlaneId = -1;
            }
        }

        // Sachen vom Computerspieler:
        qPlayer.ImageGotWorse = 0;
        qPlayer.LastActionId = -1;
        qPlayer.WorkCountdown = 0;
        qPlayer.TimeBuro = -1;
        qPlayer.TimePersonal = -1;
        qPlayer.TimeAufsicht = -1;
        qPlayer.TimeReiseburo = -1;
        qPlayer.GefallenRatio = 0;
        qPlayer.DoRoutes = FALSE;
        qPlayer.WantToDoRoutes = FALSE;
        qPlayer.SavesForPlane = FALSE;
        qPlayer.BuyBigPlane = 0;
        qPlayer.SavesForRocket = FALSE;
        qPlayer.TargetedPlayer = -1;
        qPlayer.PlayerDialog = -1;
        qPlayer.bDialogStartSent = FALSE;
        qPlayer.PlayerDialogState = -1;

        if (qPlayer.Owner == 1) {
            qPlayer.Kooperation.FillWith(1);
            for (SLONG c = 0; c < 4; c++) {
                if (Players.Players[c].Owner != 1) {
                    qPlayer.Kooperation[c] = 0;
                }
            }
        } else {
            qPlayer.Kooperation.FillWith(0);
        }

        qPlayer.Sympathie.FillWith(50);
        qPlayer.Sympathie[(c + 1) % 4] += 10;
        ;

        for (d = 0; d < qPlayer.RobotActions.AnzEntries(); d++) {
            qPlayer.RobotActions[d].ActionId = ACTION_NONE;
        }
    }

    // Daten der Spieler initialisieren: (Teil II)
    if (Difficulty > DIFF_NORMAL && Difficulty != DIFF_ADDON02) {
        SLONG NearCities[4];
        SLONG c = 0;
        SLONG d = 0;
        SLONG e = 0;
        SLONG f = 0;
        SLONG Dist = 0;
        SLONG DistD = 0;
        SLONG LoopCount = 0;

        // Eine Route anmieten:
        for (c = 0; c < 4; c++) {
            e = c + 1;

            while (true) {
                LoopCount++;
                for (d = Routen.AnzEntries() - 1; d >= 0; d--) {
                    if (Routen.IsInAlbum(d) != 0) {
                        if ((Routen[d].Ebene == 1 || LoopCount > 0) &&
                            (Routen[d].VonCity == static_cast<ULONG>(HomeAirportId) || Routen[d].NachCity == static_cast<ULONG>(HomeAirportId))) {
                            e--;
                            if (e <= 0) {
                                Players.Players[c].RentRoute(Routen[d].VonCity, Routen[d].NachCity, Routen[d].Miete);

                                // hprintf ("Player %li rents %s-%s", c, (LPCTSTR)Cities[Routen[d].VonCity].Name, (LPCTSTR)Cities[Routen[d].NachCity].Name);

                                if (GlobalUse(USE_TRAVELHOLDING)) {
                                    CRentCity *pRentCity = nullptr;

                                    if (Routen[d].VonCity != ULONG(HomeAirportId)) {
                                        pRentCity = &Players.Players[c].RentCities.RentCities[Cities(Routen[d].VonCity)];

                                        SLONG Anz = 1;
                                        for (f = 0; f < 4; f++) {
                                            if (f != c && (Players.Players[f].RentCities.RentCities[Cities(Routen[d].VonCity)].Rang != 0U)) {
                                                Anz++;
                                            }
                                        }

                                        pRentCity->Rang = UBYTE(Anz);
                                        pRentCity->Miete = Cities[Routen[d].VonCity].BuroRent;
                                        // hprintf ("Player %li rents %s", c, (LPCTSTR)Cities[Routen[d].VonCity].Name);
                                    }
                                    if (Routen[d].NachCity != ULONG(HomeAirportId)) {
                                        pRentCity = &Players.Players[c].RentCities.RentCities[Cities(Routen[d].NachCity)];

                                        SLONG Anz = 1;
                                        for (f = 0; f < 4; f++) {
                                            if (f != c && (Players.Players[f].RentCities.RentCities[Cities(Routen[d].NachCity)].Rang != 0U)) {
                                                Anz++;
                                            }
                                        }

                                        pRentCity->Rang = UBYTE(Anz);
                                        pRentCity->Miete = Cities[Routen[d].NachCity].BuroRent;
                                        // hprintf ("Player %li rents %s", c, (LPCTSTR)Cities[Routen[d].NachCity].Name);
                                    }
                                }

                                goto leave_loop;
                            }
                        }
                    }
                }
            }

        leave_loop:;
        }

        // Eine zweite Stadt anmieten:
        for (c = 0; c < 4; c++) {
            Dist = 2147483647;

            for (d = Cities.AnzEntries() - 1; d >= 0; d--) {
                if ((Cities.IsInAlbum(d) != 0) && Cities.GetIdFromIndex(d) != ULONG(HomeAirportId)) {
                    SLONG delta = Cities.CalcDistance(d, HomeAirportId);

                    for (e = 0; e < c; e++) {
                        if (NearCities[e] == d) {
                            delta = 2147483647;
                        }
                    }

                    if (delta > 1000000 && delta < Dist) {
                        Dist = delta;
                        DistD = d;
                    }
                }
            }

            NearCities[c] = DistD;
        }

        if (GlobalUse(USE_TRAVELHOLDING)) {
            for (c = 0; c < 4; c++) {
                for (d = 0; d < 4; d++) {
                    if (Players.Players[c].RentCities.RentCities[NearCities[(c + d) % 4]].Rang == 0) {
                        CRentCity &qRentCity = Players.Players[c].RentCities.RentCities[NearCities[(c + d) % 4]];

                        // hprintf ("Player %li rents %s", c, (LPCTSTR)Cities[NearCities[(c+d)%4]].Name);

                        SLONG f = 0;
                        SLONG Anz = 1;
                        for (f = 0; f < 4; f++) {
                            if (f != c && (Players.Players[f].RentCities.RentCities[NearCities[(c + d) % 4]].Rang != 0U)) {
                                Anz++;
                            }
                        }

                        qRentCity.Rang = UBYTE(Anz);
                        qRentCity.Miete = Cities[NearCities[(c + d) % 4]].BuroRent;
                        break;
                    }
                }
            }
        }
    }
    for (c = 0; c < Players.AnzPlayers; c++) {
        Players.Players[c].MapWorkers(FALSE);
        Players.Players[c].PlanGates();

        for (d = 0; d < Players.Players[c].Planes.AnzEntries(); d++) {
            if (Players.Players[c].Planes.IsInAlbum(d) != 0) {
                Players.Players[c].Planes[d].UpdatePersonalQuality(c);
            }
        }
    }

    Players.RobotInit();

    // Aushänge für Versteigerungen, Aufträge, ..:
    TafelData.Randomize(2);
    gFrachten.Random.SRand(Date);
    LastMinuteAuftraege.Random.SRand(Date + 1);
    ReisebueroAuftraege.Random.SRand(Date + 2);
    if (bNetwork == 0) {
        gFrachten.Random.SRand(AtGetTime());
        LastMinuteAuftraege.Random.SRand(AtGetTime());
        ReisebueroAuftraege.Random.SRand(AtGetTime());
    }

    gFrachten.Fill();
    LastMinuteAuftraege.FillForLastMinute();
    ReisebueroAuftraege.FillForReisebuero();

    DumpAASeedSum(1001);

    for (c = 0; c < SLONG(Cities.AnzEntries()); c++) {
        AuslandsRefill[c] = 6;
        AuslandsAuftraege[c].Random.SRand(Date + c + 3);
        AuslandsAuftraege[c].FillForAusland(c);
        AuslandsRefill[c] = 6;

        AuslandsFRefill[c] = 6;
        AuslandsFrachten[c].Random.SRand(Date + c + 3);
        AuslandsFrachten[c].FillForAusland(c);
        AuslandsFRefill[c] = 6;
    }

    // Die Schlagzeilen der Zeitungen laden:
    Headlines.Init();
    Headlines.ReloadHeadline();
    Headlines.InterpolateHeadline(); // Variablen ersetzen
    Headlines.ComparisonHeadlines(); // Variablen ersetzen
    CreateRandomUsedPlanes();

    DumpAASeedSum(1002);
}

//----------------------------------------------------------------------------------------
// Sucht die Städte aus, die der Spieler in der Routenmissionen verbinden muß
//----------------------------------------------------------------------------------------
void SIM::CreateMissionCities() {
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;
    BUFFER_V<SLONG> Map(Cities.AnzEntries());

    // Die Map-Tabelle ist ein Filter, der alle Städte nach ihrer Einwohnerzahl sortiert:
    for (c = Map.AnzEntries() - 1; c >= 0; c--) {
        Map[c] = Cities.GetIdFromIndex(c);
    }

    for (c = 0; c < Map.AnzEntries() - 1; c++) {
        if (Cities[Map[c]].Einwohner > Cities[Map[c + 1]].Einwohner) {
            SLONG h = Map[c];
            Map[c] = Map[c + 1];
            Map[c + 1] = h;
            c -= 2;
            if (c < -1) {
                c = -1;
            }
        }
    }

    MissionCities.ReSize(6);

    // Default-Städte:
    MissionCities[0] = Cities.GetIdFromNames("R\xEDo de Janeiro", "Rio de Janeiro", NULL);
    MissionCities[1] = Cities.GetIdFromNames("Nova Iorque", "Nueva York", "New York", NULL);
    MissionCities[2] = Cities.GetIdFromNames("T\xF3quio", "Tokio", "Tokyo", NULL);
    MissionCities[3] = Cities.GetIdFromNames("Deli", "Delhi", "Dill\xED", NULL);
    MissionCities[4] = Cities.GetIdFromNames("Joanesburgo", "Johanesburgo", "Johannesburg", NULL);
    MissionCities[5] = Cities.GetIdFromNames("Moskau", "Moskou", "Moscovo", "Moscou", "Mosca", "Mosc\xFA", "Moscow", "Moskva", NULL);

    /*if (gLanguage==LANGUAGE_S) MissionCities[0]=Cities.GetIdFromName ("Río de Janeiro");
      else                       MissionCities[0]=Cities.GetIdFromName ("Rio de Janeiro");

      if (gLanguage==LANGUAGE_O)      MissionCities[1]=Cities.GetIdFromName ("Nova Iorque"); //Portugisisch
      else if (gLanguage==LANGUAGE_S) MissionCities[1]=Cities.GetIdFromName ("Nueva York");  //Spanisch
      else                            MissionCities[1]=Cities.GetIdFromName ("New York");

      if (gLanguage==LANGUAGE_O)      MissionCities[2]=Cities.GetIdFromName ("Tóquio"); //Portugisisch
      else if (gLanguage==LANGUAGE_S) MissionCities[2]=Cities.GetIdFromName ("Tokio");  //Spanisch
      else                            MissionCities[2]=Cities.GetIdFromName ("Tokyo");

      if (gLanguage==LANGUAGE_O) MissionCities[3]=Cities.GetIdFromName ("Deli");
      else                       MissionCities[3]=Cities.GetIdFromName ("Delhi");

      if (gLanguage==LANGUAGE_O)      MissionCities[4]=Cities.GetIdFromName ("Joanesburgo"); //Portugisisch
      else if (gLanguage==LANGUAGE_S) MissionCities[4]=Cities.GetIdFromName ("Johanesburgo"); //Spanisch
      else                            MissionCities[4]=Cities.GetIdFromName ("Johannesburg");

      if (gLanguage==LANGUAGE_D)      MissionCities[5]=Cities.GetIdFromName ("Moskau");
      else if (gLanguage==LANGUAGE_N) MissionCities[5]=Cities.GetIdFromName ("Moskou");
      else if (gLanguage==LANGUAGE_O) MissionCities[5]=Cities.GetIdFromName ("Moscovo");
      else if (gLanguage==LANGUAGE_F) MissionCities[5]=Cities.GetIdFromName ("Moscou");
      else if (gLanguage==LANGUAGE_I) MissionCities[5]=Cities.GetIdFromName ("Mosca");
      else if (gLanguage==LANGUAGE_S) MissionCities[5]=Cities.GetIdFromName ("Moscú");
      else                            MissionCities[5]=Cities.GetIdFromName ("Moscow");*/

    // Ist eine Stadt doppelt oder zu weit weg? Dann eine Bessere raussuchen:
    for (c = 0; c < 6; c++) {
        if (MissionCities[c] == HomeAirportId || Cities.CalcDistance(MissionCities[c], HomeAirportId) > 12000000) {
            for (d = Map.AnzEntries() - 1; d >= 0; d--) {
                if (Map[d] == HomeAirportId) {
                    continue;
                }
                if (Cities.CalcDistance(Map[d], HomeAirportId) > 12000000) {
                    continue;
                }

                for (e = 0; e < 6; e++) {
                    if (Map[d] == MissionCities[e]) {
                        break;
                    }
                }
                if (e < 6) {
                    continue;
                }

                MissionCities[c] = Map[d];
                break;
            }
        }
    }

    // Wenn die notwendigen Routen normalerweise nicht existieren, müssen wir sie halt erzeugen:
    for (c = 0; c < 6; c++) {
        for (d = Routen.AnzEntries() - 1; d >= 0; d--) {
            if (Routen.IsInAlbum(d) != 0) {
                if (Routen[d].VonCity == ULONG(HomeAirportId) && Routen[d].NachCity == ULONG(MissionCities[c])) {
                    break;
                }
            }
        }

        if (d < 0) {
            CRoute routeHin;
            routeHin.Ebene = 1;
            routeHin.VonCity = HomeAirportId;
            routeHin.NachCity = MissionCities[c];
            routeHin.Miete = 6000;
            routeHin.Faktor = 1;
            routeHin.Bedarf = 0;

            CRoute routeHer = routeHin;
            std::swap(routeHer.VonCity, routeHer.NachCity);

            Routen += routeHin;
            Routen += routeHer;
        }
    }
}

//----------------------------------------------------------------------------------------
// Sucht die Stadt mit dem Erdbeben aus:
//----------------------------------------------------------------------------------------
void SIM::CreateEarthquakeCity() {
    SLONG c = 0;
    SLONG best = -1;
    SLONG bestDiff = 2000000000;

    for (c = Cities.AnzEntries() - 1; c >= 0; c--) {
        if (abs(Cities.CalcDistance(c, HomeAirportId) - 5000000) < bestDiff && abs(Cities[c].GlobusPosition.y) < 20) {
            best = c;
            bestDiff = abs(Cities.CalcDistance(c, HomeAirportId) - 5000000);
        }
    }

    if (bestDiff > 2000000) {
        for (c = Cities.AnzEntries() - 1; c >= 0; c--) {
            if (abs(Cities.CalcDistance(c, HomeAirportId) - 5000000) < bestDiff) {
                best = c;
                bestDiff = abs(Cities.CalcDistance(c, HomeAirportId) - 5000000);
            }
        }
    }

    KrisenCity = Cities.GetIdFromIndex(best);
}

//----------------------------------------------------------------------------------------
// Zählt die Uhr einen Schritt weiter und löst ggf. Ereignisse aus.
//----------------------------------------------------------------------------------------
void SIM::DoTimeStep() {
    BOOL DoCalc = 0;
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;
    SLONG count = 0;
    SLONG Minute = 0;
    SLONG OldMinute = 0;
    SLONG OldHour = GetHour();
    SLONG City = 0;

    PLAYER &qLocalPlayer = Players.Players[localPlayer];

    OldMinute = GetMinute();
    Time += GameSpeed;
    PlayerDidntMove++; // Wird ggf. bei WalkPersons resettet

    if (Time >= 24 * 60000) {
        NewDay();
        Airport.NewDay();
    }

    Minute = GetMinute();

    if (Minute != OldMinute) // Redraw der Statuszeile erzwingen:
    {
        PersonRandCreate.SRand(Minute * 333 + GetHour() * 7);
        PersonRandMisc.SRand(Minute * 333 + GetHour() * 7);

        if ((Minute % 5) == 0) {
            // NetGenericSync (1300, TickReisebueroRefill);
            // NetGenericSync (1301, TickLastMinuteRefill);
            // NetGenericSync (1302, TickFrachtRefill);
            // NetGenericSync (1303, TickMuseumRefill);

            TickReisebueroRefill++;
            TickLastMinuteRefill++;
            TickFrachtRefill++;
            TickMuseumRefill++;

            for (SLONG c = 0; c < SLONG(Cities.AnzEntries()); c++) {
                // NetGenericSync (1310+c, AuslandsRefill[c]);
                AuslandsRefill[c]++;
                AuslandsFRefill[c]++;
            }
        }

        if (GetHour() >= 9 && GetHour() < 18 && (CallItADay == 0)) {
            if ((GetMinute() % 5) == 0) {
                rChkTime = GetMinute();
                rChkPersonRandCreate = PersonRandCreate.GetSeed();
                rChkPersonRandMisc = PersonRandMisc.GetSeed();
                rChkHeadlineRand = HeadlineRand.GetSeed();
                rChkLMA = LastMinuteAuftraege.Random.GetSeed();
                rChkRBA = ReisebueroAuftraege.Random.GetSeed();
                rChkFrachen = gFrachten.Random.GetSeed();
                rChkGeneric = CheckGeneric;

                for (SLONG c = 0; c < MAX_CITIES; c++) {
                    rChkAA[c] = AuslandsAuftraege[c].Random.GetSeed();
                }

                for (c = 0; c < 4; c++) {
                    for (d = 0; d < 5; d++) {
                        rChkActionId[c * 5 + d] = Players.Players[c].RobotActions[d].ActionId;
                    }
                }
            }
            if ((GetMinute() % 5) == 3) {
                TEAKFILE Message;

                Message.Announce(512);

                Message << ATNET_CHECKRANDS << rChkTime;
                Message << rChkPersonRandCreate << rChkPersonRandMisc << rChkHeadlineRand;
                Message << rChkLMA << rChkRBA << rChkFrachen << rChkGeneric;

                for (auto &c : rChkAA) {
                    Message << c;
                }
                for (c = 0; c < 4; c++) {
                    for (d = 0; d < 5; d++) {
                        Message << rChkActionId[c * 5 + d];
                    }
                }

                SIM::SendMemFile(Message);
            }
        }

        if (GetHour() == 18 && OldHour != 18) {
            // Synchronisierung durch Robots; wird um 18 Uhr auf false gesetzt (würde sonst in CAbend.cpp geschehen)
            if (Players.GetAnzRobotPlayers() == 0) {
                for (SLONG c = 0; c < 4; c++) {
                    Players.Players[c].bReadyForMorning = 1;
                }
            }
        }

        // Verschiedene Sync's für's Netzwerk:
        if ((CallItADay == 0) && (bNetwork != 0)) {
            if (Minute >= 10 && OldMinute < 10 && Time >= 9 * 60000 && Time <= 18 * 60000) {
                PLAYER::NetSynchronizeMoney();
            }
            if (Minute >= 20 && OldMinute < 20) {
                PLAYER::NetSynchronizeImage();
            }
            if (Minute >= 30 && OldMinute < 30) {
                PLAYER::NetSynchronizeRoutes();
            }
        }

        // if (bNetwork && (Time<9*60000 || Time>18*60000 || CallItADay))
        //{
        //  if (Minute>=50 && OldMinute<50) NetGenericSync (0x100000+GetHour()*60);
        //}

        if (qLocalPlayer.LocationWin != nullptr) {
            (qLocalPlayer.LocationWin)->StatusCount = 3;
        } else if (TopWin != nullptr) {
            (TopWin)->StatusCount = 3;
        }

        for (c = 0; c < 4; c++) {
            if (Players.Players[c].IsOut == 0) {
                PLAYER &qPlayer = Players.Players[c];

                if ((qPlayer.StrikeHours != 0) && (qPlayer.StrikeEndCountdown != 0)) {
                    qPlayer.StrikeEndCountdown--;

                    if (qPlayer.StrikeEndCountdown == 0) {
                        qPlayer.StrikeHours = 0;

                        if (qPlayer.Owner == 0) {
                            qPlayer.StrikeNotified = FALSE; // Dem Spieler bei nächster Gelegenheit bescheid sagen
                            // Die Variable qPlayer.StrikeEndType muß bereits gesetzt sein
                        }
                    }
                }

                // Notify wegen Streikanfang:
                if ((qPlayer.StrikeHours != 0) && (qPlayer.StrikeNotified == 0) && GetHour() >= 9 && GetHour() < 18 && (GetMinute() > 10 || GetHour() > 9)) {
                    if ((qPlayer.LocationWin != nullptr) && ((qPlayer.LocationWin)->IsDialogOpen() == 0) && ((qPlayer.LocationWin)->MenuIsOpen() == 0) &&
                        (CallItADay == 0)) {
                        qPlayer.StrikeNotified = TRUE;
                        qPlayer.WalkStop();

                        (qPlayer.LocationWin)->MenuStart(MENU_SABOTAGEFAX, 8, qPlayer.StrikeHours);
                        (qPlayer.LocationWin)->MenuSetZoomStuff(XY(320, 220), 0.17, FALSE);

                        if (qPlayer.Owner == 0 && (qPlayer.IsOut == 0)) {
                            qPlayer.Letters.AddLetter(FALSE, bprintf(StandardTexte.GetS(TOKEN_LETTER, 508), qPlayer.StrikeHours), "", "", 8);
                        }

                        bgWarp = FALSE;
                        if (CheatTestGame == 0) {
                            qPlayer.GameSpeed = 0;
                        }
                    }
                }

                // Notify wegen Streikende:
                if (qPlayer.StrikeHours == 0 && (qPlayer.StrikeNotified == 0) && qPlayer.StrikeEndType != 0 && GetHour() >= 9 && GetHour() < 18) {
                    if (qPlayer.IsOkayToCallThisPlayer() != 0) {
                        if ((qPlayer.HasItem(ITEM_HANDY) != 0) || qPlayer.GetRoom() == ROOM_BURO_A + c * 10 || qPlayer.GetRoom() == ROOM_PERSONAL_A + c * 10) {
                            if (qPlayer.GetRoom() != ROOM_PERSONAL_A + c * 10) {
                                gUniversalFx.Stop();
                                gUniversalFx.ReInit("phone.raw");
                                gUniversalFx.Play(DSBPLAY_NOSTOP, Options.OptionEffekte * 100 / 7);

                                delete qPlayer.DialogWin;
                                qPlayer.DialogWin = new CPersonal(TRUE, localPlayer);
                                (qPlayer.LocationWin)->StartDialog(TALKER_PERSONAL1b + c * 2, MEDIUM_HANDY, 1000 + qPlayer.StrikeEndType);
                                (qPlayer.DialogWin)->HandyOffset = 120;
                            } else {
                                (qPlayer.LocationWin)->StartDialog(TALKER_PERSONAL1b + c * 2, MEDIUM_AIR, 1000 + qPlayer.StrikeEndType);
                            }

                            bgWarp = FALSE;
                            qPlayer.StrikeNotified = TRUE;
                            qPlayer.StrikeEndType = 0;
                            if (CheatTestGame == 0) {
                                qPlayer.GameSpeed = 0;
                            }
                        }
                    }
                }
            }
        }

        if (Minute < OldMinute) {
            // Streik vorbereiten?
            if (GetHour() == 10) {
                PLAYER &qPlayer = qLocalPlayer;

                if (qPlayer.StrikeHours == 0 && qPlayer.StrikePlanned == 0) {
                    if ((Workers.GetAverageHappyness(localPlayer) - static_cast<SLONG>(Workers.GetMinHappyness(localPlayer) < 0) * 10 < 20 &&
                         qPlayer.DaysWithoutStrike > 7) ||
                        (Workers.GetAverageHappyness(localPlayer) - static_cast<SLONG>(Workers.GetMinHappyness(localPlayer) < 0) * 10 < 0 &&
                         qPlayer.DaysWithoutStrike > 3)) {
                        qPlayer.StrikePlanned = TRUE;
                    }
                }
            }

            // Streik beginnen:
            for (c = 0; c < 4; c++) {
                if (Players.Players[c].IsOut == 0) {
                    PLAYER &qPlayer = Players.Players[c];

                    if ((qPlayer.StrikePlanned != 0) && GetHour() > 6 && GetHour() < 18 && CallItADay == FALSE) {
                        SLONG c = 0;
                        SLONG AnyPlanes = FALSE;

                        for (c = qPlayer.Planes.AnzEntries() - 1; c >= 0; c--) {
                            if (qPlayer.Planes.IsInAlbum(c) != 0) {
                                CPlane &qPlane = qPlayer.Planes[c];

                                if (qPlane.Ort > 0) {
                                    AnyPlanes = TRUE;
                                }

                                if (qPlane.Flugplan.NextFlight != -1) {
                                    if (qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Landezeit == GetHour() &&
                                        qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Landedate == Date) {
                                        AnyPlanes = TRUE;
                                    }
                                    if (qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Landezeit == GetHour() + 1 &&
                                        qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Landedate == Date) {
                                        AnyPlanes = TRUE;
                                    }
                                    if (qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Landezeit == GetHour() + 2 &&
                                        qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Landedate == Date) {
                                        AnyPlanes = TRUE;
                                    }
                                }
                            }
                        }

                        if (AnyPlanes != 0) {
                            qPlayer.StrikePlanned = FALSE;
                            qPlayer.StrikeEndCountdown = 0;
                            qPlayer.StrikeNotified = FALSE; // Dem Spieler bei nächster Gelegenheit bescheid sagen

                            TEAKRAND LocalRand(Date + GetHour());

                            if (qPlayer.DaysWithoutStrike > 10) {
                                qPlayer.StrikeHours = 2;
                            } else if (Workers.GetAverageHappyness(localPlayer) < -10) {
                                qPlayer.StrikeHours = 72;
                            } else {
                                qPlayer.StrikeHours = 4 + LocalRand.Rand(20);
                            }

                            qPlayer.DaysWithoutStrike = 0;

                            Headlines.AddOverride(1, bprintf(StandardTexte.GetS(TOKEN_MISC, 2090), qPlayer.AirlineX.c_str()), GetIdFromString("STREIK"),
                                                  25 + static_cast<SLONG>(c == localPlayer) * 10);
                        }
                    } else if (qPlayer.StrikeHours != 0) {
                        qPlayer.StrikeHours--;

                        if (qPlayer.StrikeHours == 0 && (qPlayer.Owner == 0 || (qPlayer.Owner == 1 && !qPlayer.RobotUse(ROBOT_USE_FAKE_PERSONAL)))) {
                            qPlayer.StrikeNotified = FALSE; // Dem Spieler bei nächster Gelegenheit bescheid sagen
                            qPlayer.StrikeEndType = 3;      // Streik beendet durch abwarten

                            Workers.AddHappiness(c, -10);
                        }
                    }
                }
            }

            // Probleme bei einem Flugzeug auslösen:
            if ((CallItADay == 0) && (GetHour() > 5 && GetHour() < 17)) {
                PLAYER &qPlayer = qLocalPlayer;

                if (qPlayer.Planes.HasProblemPlane() == 0) {
                    if (qPlayer.Planes.GetNumUsed() >= 2) {
                        TEAKRAND LocalRand(Date + GetHour() + GetMinute());

                        for (c = qPlayer.Planes.AnzEntries() - 1; c >= 0; c--) {
                            if (qPlayer.Planes.IsInAlbum(c) != 0) {
                                if (qPlayer.Planes[c].Ort == -5 && (qPlayer.Planes[c].GetFlugplanEintrag() != nullptr) &&
                                    qPlayer.Planes[c].GetFlugplanEintrag()->Landedate == Date &&
                                    qPlayer.Planes[c].GetFlugplanEintrag()->Landezeit == GetHour()) {
                                    if (LocalRand.Rand(10) == 0 && (LocalRand.Rand(100)) > qPlayer.Planes[c].Zustand && qPlayer.Planes[c].Zustand < 90) {
                                        SLONG Extra = 0;

                                        qPlayer.Planes[c].Problem =
                                            4 + (LocalRand.Rand(101 - qPlayer.Planes[c].Zustand)) / 3 + static_cast<SLONG>((LocalRand.Rand(4)) == 0) * 15;

                                        if (qPlayer.Planes[c].GetFlugplanEintrag()->NachCity == static_cast<ULONG>(HomeAirportId)) {
                                            qPlayer.Planes[c].Problem = max(4, qPlayer.Planes[c].Problem - 15);
                                        } else if (qPlayer.Planes[c].Problem > 20) {
                                            Extra = 1;
                                        }

                                        if (Extra != 0) {
                                            if (qPlayer.Owner == 0 && (qPlayer.IsOut == 0)) {
                                                qPlayer.Letters.AddLetter(FALSE,
                                                                          bprintf(StandardTexte.GetS(TOKEN_LETTER, 507), (LPCTSTR)qPlayer.Planes[c].Name,
                                                                                  qPlayer.Planes[c].Problem, (LPCTSTR)Cities[HomeAirportId].Name,
                                                                                  (LPCTSTR)Cities[qPlayer.Planes[c].GetFlugplanEintrag()->NachCity].Name),
                                                                          "", "", 6);
                                            }

                                            CAuftrag Auftrag;

                                            Auftrag.VonCity = HomeAirportId;
                                            Auftrag.NachCity = qPlayer.Planes[c].GetFlugplanEintrag()->NachCity;
                                            Auftrag.Personen = 0;
                                            Auftrag.Date = static_cast<UWORD>(Date);
                                            Auftrag.BisDate = Date + ((qPlayer.Planes[c].Problem - (24 - GetHour())) / 24);
                                            Auftrag.InPlan = 0;
                                            Auftrag.Okay = 0;
                                            Auftrag.Praemie = 0;
                                            Auftrag.Strafe = 0;
                                            qPlayer.Auftraege += Auftrag;
                                        } else if (qPlayer.Owner == 0 && (qPlayer.IsOut == 0)) {
                                            qPlayer.Letters.AddLetter(
                                                FALSE,
                                                bprintf(StandardTexte.GetS(TOKEN_LETTER, 506), (LPCTSTR)qPlayer.Planes[c].Name, qPlayer.Planes[c].Problem), "",
                                                "", 7);
                                        }

                                        if (qPlayer.LocationWin != nullptr) {
                                            if (((qPlayer.LocationWin)->IsDialogOpen() == 0) && ((qPlayer.LocationWin)->MenuIsOpen() == 0) &&
                                                (Options.OptionFax != 0) && (CallItADay == 0)) {
                                                (qPlayer.LocationWin)->MenuStart(MENU_SABOTAGEFAX, 6 + Extra, c, qPlayer.Planes[c].Problem);
                                                (qPlayer.LocationWin)->MenuSetZoomStuff(XY(320, 220), 0.17, FALSE);

                                                qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2308));

                                                bgWarp = FALSE;
                                                if (CheatTestGame == 0) {
                                                    qLocalPlayer.GameSpeed = 0;
                                                }
                                            } else if (CallItADay == 0) {
                                                qPlayer.Messages.AddMessage(BERATERTYP_GIRL,
                                                                            bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2309), (LPCTSTR)qPlayer.Planes[c].Name));
                                            }
                                        }

                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (Minute < OldMinute || (Date == 0 && GetHour() == 9 && GetMinute() == 0)) {
            if (CallItADay == 0 && Time < 18 * 60000 && Time > 8 * 60000) {
                // Conditional Block: DutyFree, ArabAir, LastMinute, Museum, Reisebüro:
                Airport.SetConditionBlock(0,
                                          static_cast<BOOL>(SLONG(Time) < timeDutyOpen || (((Weekday == 5 || Weekday == 6) && SLONG(Time) > timeDutyClose))));
                Airport.SetConditionBlock(1, static_cast<BOOL>(SLONG(Time) < timeArabOpen || Weekday == 6));
                Airport.SetConditionBlock(2, static_cast<BOOL>(SLONG(Time) >= timeLastClose || Weekday == 5));
                Airport.SetConditionBlock(3, static_cast<BOOL>(SLONG(Time) < timeMuseOpen || Weekday == 5 || Weekday == 6));
                Airport.SetConditionBlock(4, static_cast<BOOL>(SLONG(Time) >= timeReisClose));
                Airport.SetConditionBlock(5, static_cast<BOOL>(SLONG(Time) >= timeMaklClose));
                Airport.SetConditionBlock(6, static_cast<BOOL>((SLONG(Time) < timeWerbOpen || Weekday == 5 || Weekday == 6) &&
                                                               (Difficulty >= DIFF_NORMAL || Difficulty == DIFF_FREEGAME)));
                Airport.SetConditionBlock(20, static_cast<BOOL>(Players.Players[0].OfficeState == 2));
                Airport.SetConditionBlock(21, static_cast<BOOL>(Players.Players[1].OfficeState == 2));
                Airport.SetConditionBlock(22, static_cast<BOOL>(Players.Players[2].OfficeState == 2));
                Airport.SetConditionBlock(23, static_cast<BOOL>(Players.Players[3].OfficeState == 2));
            }
        }

        if (Minute < OldMinute) {
            SLONG c = 0;

            // Sabotage:
            for (c = 0; c < Players.Players.AnzEntries(); c++) {
                if (Players.Players[c].ArabMode != 0) {
                    PLAYER &qOpfer = Players.Players[Players.Players[c].ArabOpfer];

                    if (qOpfer.Planes.IsInAlbum(Players.Players[c].ArabPlane) == 0) {
                        Players.Players[c].ArabMode = 0; // Flugzeug gibt es nicht mehr
                    } else {
                        if (Players.Players[c].ArabActive == FALSE && qOpfer.Planes[Players.Players[c].ArabPlane].Ort >= 0) {
                            Players.Players[c].ArabActive = TRUE;
                        }

                        if (Players.Players[c].ArabActive == TRUE) {
                            BOOL ActNow = FALSE;

                            if (Players.Players[c].ArabMode != 3 && qOpfer.Planes[Players.Players[c].ArabPlane].Ort < 0) {
                                ActNow = TRUE;
                            }
                            if (Players.Players[c].ArabMode == 3 && qOpfer.Planes[Players.Players[c].ArabPlane].Ort >= 0) {
                                CPlane &qPlane = qOpfer.Planes[Players.Players[c].ArabPlane];
                                SLONG e = qPlane.Flugplan.NextStart;

                                if (e != -1 && qPlane.Flugplan.Flug[e].Startdate * 24 + qPlane.Flugplan.Flug[e].Startzeit == Date * 24 + GetHour()) {
                                    ActNow = TRUE;
                                }
                            }

                            if (ActNow != 0) {
                                // Die Anschläge ausführen:
                                __int64 PictureId = 0;
                                CPlane &qPlane = qOpfer.Planes[Players.Players[c].ArabPlane];

                                qOpfer.Statistiken[STAT_UNFAELLE].AddAtPastDay(1);
                                Players.Players[c].Statistiken[STAT_SABOTIERT].AddAtPastDay(1);

                                if (qPlane.Flugplan.NextFlight != -1) {
                                    CFlugplanEintrag &qFPE = qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight];

                                    bool bFremdsabotage = false;
                                    if (Players.Players[c].ArabMode < 0) {
                                        Players.Players[c].ArabMode = -Players.Players[c].ArabMode;
                                        bFremdsabotage = true;
                                    }

                                    switch (Players.Players[c].ArabMode) {
                                    case 1:
                                        if (!bFremdsabotage) {
                                            Players.Players[c].ArabHints += 2;
                                        }
                                        qOpfer.Kurse[0] *= 0.95;
                                        qOpfer.TrustedDividende -= 1;
                                        qOpfer.Sympathie[c] -= 5;
                                        if (qFPE.ObjectType == 1) {
                                            qOpfer.RentRouten.RentRouten[Routen(qFPE.ObjectId)].Image =
                                                qOpfer.RentRouten.RentRouten[Routen(qFPE.ObjectId)].Image * 99 / 100;
                                        }
                                        if (qOpfer.TrustedDividende < 0) {
                                            qOpfer.TrustedDividende = 0;
                                        }
                                        PictureId = GetIdFromString("SALZ");
                                        break;

                                    case 2:
                                        if (!bFremdsabotage) {
                                            Players.Players[c].ArabHints += 4;
                                        }
                                        qOpfer.ChangeMoney(-2000, 3500, "");
                                        qOpfer.Kurse[0] *= 0.9;
                                        qOpfer.TrustedDividende -= 2;
                                        qOpfer.Sympathie[c] -= 15;
                                        if (qOpfer.TrustedDividende < 0) {
                                            qOpfer.TrustedDividende = 0;
                                        }
                                        qOpfer.Image -= 2;
                                        if (qFPE.ObjectType == 1) {
                                            qOpfer.RentRouten.RentRouten[Routen(qFPE.ObjectId)].Image =
                                                qOpfer.RentRouten.RentRouten[Routen(qFPE.ObjectId)].Image * 95 / 100;
                                        }
                                        PictureId = GetIdFromString("SKELETT");
                                        break;

                                    case 3: // Platter Reifen:
                                    {
                                        if (!bFremdsabotage) {
                                            Players.Players[c].ArabHints += 10;
                                        }
                                        qOpfer.ChangeMoney(-8000, 3500, "");
                                        qOpfer.Kurse[0] *= 0.85;
                                        qOpfer.TrustedDividende -= 3;
                                        qOpfer.Sympathie[c] -= 35;
                                        if (qOpfer.TrustedDividende < 0) {
                                            qOpfer.TrustedDividende = 0;
                                        }
                                        qOpfer.Image -= 3;
                                        if (qFPE.ObjectType == 1) {
                                            qOpfer.RentRouten.RentRouten[Routen(qFPE.ObjectId)].Image =
                                                qOpfer.RentRouten.RentRouten[Routen(qFPE.ObjectId)].Image * 90 / 100;
                                        }

                                        SLONG e = qPlane.Flugplan.NextStart;

                                        qPlane.Flugplan.Flug[e].Startzeit++;
                                        qPlane.Flugplan.UpdateNextFlight();
                                        qPlane.Flugplan.UpdateNextStart();
                                        if (!bFremdsabotage) {
                                            Players.Players[c].Statistiken[STAT_VERSPAETUNG].AddAtPastDay(1);
                                        }

                                        if (qPlane.Flugplan.Flug[e].Startzeit == 24) {
                                            qPlane.Flugplan.Flug[e].Startzeit = 0;
                                            qPlane.Flugplan.Flug[e].Startdate++;
                                        }
                                        qPlane.CheckFlugplaene(Players.Players[c].ArabOpfer);
                                        qOpfer.UpdateAuftragsUsage();
                                        qOpfer.Messages.AddMessage(BERATERTYP_GIRL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2304), (LPCTSTR)qPlane.Name));
                                        PictureId = GetIdFromString("REIFEN");
                                    } break;

                                    case 4:
                                        if (!bFremdsabotage) {
                                            Players.Players[c].ArabHints += 20;
                                        }
                                        qOpfer.ChangeMoney(-40000, 3500, "");
                                        qOpfer.Kurse[0] *= 0.75;
                                        qOpfer.TrustedDividende -= 4;
                                        qOpfer.Sympathie[c] -= 80;
                                        if (qOpfer.TrustedDividende < 0) {
                                            qOpfer.TrustedDividende = 0;
                                        }
                                        qOpfer.Image -= 8;
                                        if (qFPE.ObjectType == 1) {
                                            qOpfer.RentRouten.RentRouten[Routen(qFPE.ObjectId)].Image =
                                                qOpfer.RentRouten.RentRouten[Routen(qFPE.ObjectId)].Image * 50 / 100;
                                        }
                                        PictureId = GetIdFromString("FEUER");
                                        break;

                                    case 5:
                                        if (!bFremdsabotage) {
                                            Players.Players[c].ArabHints += 100;
                                        }
                                        qOpfer.ChangeMoney(-70000, 3500, "");
                                        qOpfer.Kurse[0] *= 0.70;
                                        qOpfer.TrustedDividende -= 5;
                                        qOpfer.Sympathie[c] -= 200;
                                        if (qOpfer.TrustedDividende < 0) {
                                            qOpfer.TrustedDividende = 0;
                                        }
                                        PictureId = GetIdFromString("SUPERMAN");
                                        break;
                                    default:
                                        hprintf("Sim.cpp: Default case should not be reached.");
                                        DebugBreak();
                                    }
                                    if (qOpfer.Kurse[0] < 0) {
                                        qOpfer.Kurse[0] = 0;
                                    }
                                    // log: hprintf ("Player[%li].Image now (sabo) = %li", (LPCTSTR)Players.Players[c].ArabOpfer, (LPCTSTR)qOpfer.Image);

                                    // Für's Briefing vermerken:
                                    SabotageActs.ReSize(SabotageActs.AnzEntries() + 1);
                                    SabotageActs[SabotageActs.AnzEntries() - 1].Player = bFremdsabotage ? -2 : c;
                                    SabotageActs[SabotageActs.AnzEntries() - 1].ArabMode = Players.Players[c].ArabMode;
                                    SabotageActs[SabotageActs.AnzEntries() - 1].Opfer = Players.Players[c].ArabOpfer;

                                    Headlines.AddOverride(
                                        0, bprintf(StandardTexte.GetS(TOKEN_MISC, 2000 + Players.Players[c].ArabMode), (LPCTSTR)qOpfer.AirlineX), PictureId,
                                        static_cast<SLONG>(Players.Players[c].ArabOpfer == localPlayer) * 50 + Players.Players[c].ArabMode);
                                    Limit(SLONG(-1000), qOpfer.Image, SLONG(1000));

                                    // Araber meldet sich, oder Fax oder Brief sind da.
                                    if (c == localPlayer && (qLocalPlayer.IsOkayToCallThisPlayer() != 0)) {
                                        if (qLocalPlayer.GetRoom() == ROOM_SABOTAGE) {
                                            (qLocalPlayer.LocationWin)->StartDialog(TALKER_SABOTAGE, MEDIUM_AIR, 2000);
                                            bgWarp = FALSE;
                                            if (CheatTestGame == 0) {
                                                qLocalPlayer.GameSpeed = 0;
                                            }
                                        } else {
                                            gUniversalFx.Stop();
                                            gUniversalFx.ReInit("phone.raw");
                                            gUniversalFx.Play(DSBPLAY_NOSTOP, Options.OptionEffekte * 100 / 7);

                                            bgWarp = FALSE;
                                            if (CheatTestGame == 0) {
                                                qLocalPlayer.GameSpeed = 0;
                                            }

                                            delete qLocalPlayer.DialogWin;
                                            qLocalPlayer.DialogWin = new CSabotage(TRUE, localPlayer);
                                            (qLocalPlayer.LocationWin)->StartDialog(TALKER_SABOTAGE, MEDIUM_HANDY, 2000);
                                            (qLocalPlayer.LocationWin)->PayingForCall = FALSE;
                                        }
                                    } else if (Players.Players[c].ArabOpfer == localPlayer) {
                                        if (qOpfer.Owner == 0 && (qOpfer.IsOut == 0)) {
                                            qOpfer.Letters.AddLetter(FALSE,
                                                                     bprintf(StandardTexte.GetS(TOKEN_LETTER, 500 + Players.Players[c].ArabMode),
                                                                             (LPCTSTR)qOpfer.Planes[Players.Players[c].ArabPlane].Name),
                                                                     "", "", Players.Players[c].ArabMode);
                                        }

                                        if (qLocalPlayer.LocationWin != nullptr) {
                                            if (((qLocalPlayer.LocationWin)->IsDialogOpen() == 0) && ((qLocalPlayer.LocationWin)->MenuIsOpen() == 0) &&
                                                (Options.OptionFax != 0) && (CallItADay == 0)) {
                                                (qOpfer.LocationWin)
                                                    ->MenuStart(MENU_SABOTAGEFAX, Players.Players[c].ArabMode, Players.Players[c].ArabOpfer,
                                                                Players.Players[c].ArabPlane);
                                                (qOpfer.LocationWin)->MenuSetZoomStuff(XY(320, 220), 0.17, FALSE);

                                                qOpfer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2308));

                                                bgWarp = FALSE;
                                                if (CheatTestGame == 0) {
                                                    qLocalPlayer.GameSpeed = 0;
                                                }
                                            } else if (CallItADay == 0) {
                                                qOpfer.Messages.AddMessage(BERATERTYP_GIRL,
                                                                           bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2301 + Players.Players[c].ArabMode),
                                                                                   (LPCTSTR)qOpfer.Planes[Players.Players[c].ArabPlane].Name));
                                            }
                                        }
                                    }

                                    Players.Players[c].ArabMode = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Warnung wegen Tagesende:
    if (GetHour() == 17 && Minute >= 30 && OldMinute < 30 && (CallItADay == 0)) {
        PlayUniversalFx("daywarn.raw", Options.OptionEffekte);

        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2390));
    } else if (GetHour() == 17 && Minute >= 59 && OldMinute < 58 && (CallItADay == 0)) {
        PlayUniversalFx("daywarn.raw", Options.OptionEffekte);
    }

    // Die Positionen der fliegenden Maschinen auf der Weltkarte aktualisieren
    if (Minute != OldMinute && (Minute & 3) == 0) {
        c = qLocalPlayer.GetRoom();
        if (c == ROOM_LAPTOP || c == ROOM_GLOBE) {
            DoCalc = TRUE;
        } else {
            DoCalc = FALSE;
        }

        // Flugzeuge von allen relevanten Spielern:
        for (c = 0; c < 4; c++) {
            if (Players.Players[c].IsOut == 0) {
                PLAYER &qPlayer = Players.Players[c];

                // Für alle Flugzeuge die er besitzt
                for (d = 0; d < qPlayer.Planes.AnzEntries(); d++) {
                    if (qPlayer.Planes.IsInAlbum(d) != 0) {
                        CPlane &qPlane = qPlayer.Planes[d];

                        if (qPlane.Ort == -5 && qPlane.Flugplan.NextFlight != -1) {
                            CFlugplanEintrag &qFPE = qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight];

                            if (Date > qFPE.Startdate || (Date == qFPE.Startdate && GetHour() >= qFPE.Startzeit + 1)) {
                                // Unfälle wegen schlechter Qualität?
                                TEAKRAND LocalRand(Date + GetHour() + GetMinute());
                                if (qPlane.Zustand < 80 && LocalRand.Rand(5000) > qPlane.Zustand + 4920) {
                                    // Nur wenn wirklich Personen mitfliegen:
                                    if ((qFPE.ObjectType == 1 || qFPE.ObjectType == 2) && qFPE.Passagiere > 0) {
                                        qPlayer.ImageGotWorse = TRUE;
                                        qPlayer.Image -= 2;
                                        // log: hprintf ("Player[%li].Image! now = %li", c, qPlayer.Image);

                                        if (qFPE.ObjectType == 1) {
                                            qPlayer.RentRouten.RentRouten[Routen(qFPE.ObjectId)].Image =
                                                qPlayer.RentRouten.RentRouten[Routen(qFPE.ObjectId)].Image * 8 / 10;
                                        }

                                        SLONG r = LocalRand.Rand(9);
                                        qPlane.AddPanne(r);
                                        qPlayer.Messages.AddMessage(BERATERTYP_GIRL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2360 + r), (LPCTSTR)qPlane.Name));

                                        Limit(SLONG(-1000), qPlayer.Image, SLONG(1000));
                                        qPlayer.Kurse[0] *= 0.7;
                                        if (qPlayer.TrustedDividende > 4) {
                                            qPlayer.TrustedDividende -= 1;
                                        }
                                    }
                                }

                                // Unfälle wegen Jahreszeit:
                                if (LocalRand.Rand(5000) > 4980) {
                                    bool bCanHappenToThisPlane = false;

                                    switch (GetSeason()) {
                                    // 0-3 : Frühling, Sommer, Herbst, Winter:
                                    case 0:
                                        if (qPlane.Elektronik != 2) {
                                            bCanHappenToThisPlane = true;
                                        }
                                        break;
                                    case 1:
                                        if (qPlane.Sicherheit != 2) {
                                            bCanHappenToThisPlane = true;
                                        }
                                        break;
                                    case 2:
                                        if (qPlane.Elektronik != 2) {
                                            bCanHappenToThisPlane = true;
                                        }
                                        break;
                                    case 3:
                                        if (qPlane.Elektronik != 2) {
                                            bCanHappenToThisPlane = true;
                                        }
                                        break;
                                    default:
                                        hprintf("Sim.cpp: Default case should not be reached.");
                                        DebugBreak();
                                    }

                                    if (bCanHappenToThisPlane) {
                                        qPlayer.ChangeMoney(-75000, 3501, "");

                                        qPlane.AddPanne(10 + GetSeason());
                                        qPlayer.Messages.AddMessage(BERATERTYP_GIRL,
                                                                    bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2370 + GetSeason()), (LPCTSTR)qPlane.Name));
                                    }
                                }

                                // Wir müssen die aktuelle Länge/Breite berechnen:
                                if (GetHour() == 8 || (GetHour() >= 9 && GetHour() < 18 && CallItADay == FALSE) || Minute == 0) {
                                    SLONG Advance = 0; // Prozentrechnung von 0 bis 255

                                    Advance = qFPE.Landezeit - qFPE.Startzeit - 1;
                                    Advance += 24 * (qFPE.Landedate - qFPE.Startdate);
                                    Advance *= 60000; // In Ticks umrechnen

                                    if (Advance == 0) {
                                        Advance = 60000;
                                    }

                                    if (qFPE.Startzeit * 60000 <= static_cast<SLONG>(Time)) {
                                        Advance = (Time - (qFPE.Startzeit + 1) * 60000) * 256 / Advance;
                                    } else {
                                        Advance = (24 * 60000 - ((qFPE.Startzeit + 1) * 60000 - Time)) * 256 / Advance;
                                    }

                                    if (abs(Cities[qFPE.NachCity].GlobusPosition.x - Cities[qFPE.VonCity].GlobusPosition.x) < 180) {
                                        qPlane.Position.x =
                                            (Cities[qFPE.VonCity].GlobusPosition.x * (256 - Advance) + Cities[qFPE.NachCity].GlobusPosition.x * Advance) / 256;
                                    } else if (Cities[qFPE.NachCity].GlobusPosition.x > Cities[qFPE.VonCity].GlobusPosition.x) {
                                        qPlane.Position.x =
                                            Cities[qFPE.VonCity].GlobusPosition.x +
                                            (-(360 - (Cities[qFPE.NachCity].GlobusPosition.x - Cities[qFPE.VonCity].GlobusPosition.x)) * Advance) / 256;
                                    } else {
                                        qPlane.Position.x =
                                            Cities[qFPE.VonCity].GlobusPosition.x +
                                            ((360 - (Cities[qFPE.VonCity].GlobusPosition.x - Cities[qFPE.NachCity].GlobusPosition.x)) * Advance) / 256;
                                    }

                                    qPlane.Position.y =
                                        (Cities[qFPE.VonCity].GlobusPosition.y * (256 - Advance) + Cities[qFPE.NachCity].GlobusPosition.y * Advance) / 256;

                                    if (DoCalc != 0) {
                                        if ((qLocalPlayer.GetRoom() == ROOM_LAPTOP && (qLocalPlayer.DisplayPlanes[c] != 0U)) ||
                                            (qLocalPlayer.GetRoom() == ROOM_GLOBE && c == localPlayer)) {
                                            qPlane.UpdateGlobePos(qLocalPlayer.EarthAlpha);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (Minute >= 0 && OldMinute > Minute) {
        // Aktienkurse anpassen:
        for (c = 0; c < Players.Players.AnzEntries(); c++) {
            if (Players.Players[c].TrustedDividende > Players.Players[c].Dividende) {
                Players.Players[c].TrustedDividende = (Players.Players[c].TrustedDividende + Players.Players[c].Dividende) / 2;
            }

            TEAKRAND LocalRand(Date + GetMinute() + GetHour());
            Players.Players[c].Kurse[0] = (Players.Players[c].Kurse[0] * 29 + 10 * Players.Players[c].TrustedDividende) / 30.0;
            Players.Players[c].Kurse[0] += sin((Date * 24 + GetHour()) / 50.0) * Players.Players[c].Kurse[0] / 180.0;
            Players.Players[c].Kurse[0] += sin((Date * 24 + 100 + GetHour()) / (c + 30.0)) * Players.Players[c].Kurse[0] / 150.0;
            Players.Players[c].Kurse[0] += sin(LocalRand.Rand(1000)) * Players.Players[c].Kurse[0] / 50.0;
            if (Players.Players[c].Kurse[0] < 0) {
                Players.Players[c].Kurse[0] = 0;
            }
        }

        // Flugzeuge von allen Spielern:
        for (c = 0; c < Players.AnzPlayers; c++) {
            // Für alle Flugzeuge die er besitzt
            for (d = 0; d < Players.Players[c].Planes.AnzEntries(); d++) {
                if (Players.Players[c].Planes.IsInAlbum(d) != 0) {
                    CPlane &qPlane = Players.Players[c].Planes[d];

                    // Probleme bei einem Flugzeug behandeln:
                    if (qPlane.Problem > 0) {
                        qPlane.Problem--;
                    }
                    if (qPlane.PseudoProblem > 0) {
                        qPlane.PseudoProblem--;
                    }

                    for (e = 0; e < qPlane.Flugplan.Flug.AnzEntries(); e++) {
                        // Passagierzahlen aktualisieren:
                        if (qPlane.Flugplan.Flug[e].ObjectType == 1) {
                            qPlane.Flugplan.Flug[e].CalcPassengers(c, qPlane);
                        }
                        // qPlane.Flugplan.Flug[e].CalcPassengers (qPlane.TypeId, c, (LPCTSTR)qPlane);
                    }
                    qPlane.Flugplan.UpdateNextFlight();

                    for (e = qPlane.Flugplan.Flug.AnzEntries() - 1; e >= 0; e--) {
                        // Müssen wir den Flug evtl. verschieben?
                        if ((Players.Players[c].Owner != 1 || !Players.Players[c].RobotUse(ROBOT_USE_FAKE_PERSONAL)) &&
                            qPlane.Flugplan.Flug[e].ObjectType != 0 &&
                            (qPlane.Flugplan.Flug[e].Startdate * 24 + qPlane.Flugplan.Flug[e].Startzeit - 1 == Date * 24 + GetHour())) {
                            BOOL Delay = FALSE;

                            if (qPlane.PseudoProblem != 0) {
                                Delay = TRUE;
                                if (Players.Players[c].Owner == 0) {
                                    Players.Players[c].Messages.AddMessage(BERATERTYP_GIRL,
                                                                           bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2358), (LPCTSTR)qPlane.Name,
                                                                                   (LPCTSTR)Cities[qPlane.Flugplan.Flug[e].VonCity].Name));
                                }
                            }
                            if (Players.Players[c].StrikeHours != 0) {
                                Delay = TRUE;
                                if (Players.Players[c].Owner == 0) {
                                    Players.Players[c].Messages.AddMessage(BERATERTYP_GIRL,
                                                                           bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2356), (LPCTSTR)qPlane.Name,
                                                                                   (LPCTSTR)Cities[qPlane.Flugplan.Flug[e].VonCity].Name));
                                }
                            }
                            // else if (qPlane.AnzPiloten<PlaneTypes[qPlane.TypeId].AnzPiloten || qPlane.AnzBegleiter<PlaneTypes[qPlane.TypeId].AnzBegleiter)
                            else if (qPlane.AnzPiloten < qPlane.ptAnzPiloten || qPlane.AnzBegleiter < qPlane.ptAnzBegleiter) {
                                Delay = TRUE;
                                if (Players.Players[c].Owner == 0) {
                                    Players.Players[c].Messages.AddMessage(BERATERTYP_GIRL,
                                                                           bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2350), (LPCTSTR)qPlane.Name,
                                                                                   (LPCTSTR)Cities[qPlane.Flugplan.Flug[e].VonCity].Name));
                                }
                            } else if (qPlane.Zustand < 15) {
                                Delay = TRUE;
                                if (Players.Players[c].Owner == 0) {
                                    Players.Players[c].Messages.AddMessage(BERATERTYP_GIRL,
                                                                           bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2351), (LPCTSTR)qPlane.Name,
                                                                                   (LPCTSTR)Cities[qPlane.Flugplan.Flug[e].VonCity].Name));
                                }
                            } else if (qPlane.Flugplan.Flug[e].ObjectType == 2 && qPlane.Flugplan.Flug[e].Okay == 1) {
                                Delay = TRUE;
                                if (Players.Players[c].Owner == 0) {
                                    Players.Players[c].Messages.AddMessage(BERATERTYP_GIRL,
                                                                           bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2352), (LPCTSTR)qPlane.Name,
                                                                                   (LPCTSTR)Cities[qPlane.Flugplan.Flug[e].VonCity].Name));
                                }
                            } else if (qPlane.Problem != 0) {
                                Delay = TRUE;

                                if (Players.Players[c].Owner == 0) {
                                    if ((qLocalPlayer.LocationWin != nullptr) && (qLocalPlayer.LocationWin)->CurrentMenu != MENU_SABOTAGEFAX) {
                                        Players.Players[c].Messages.AddMessage(BERATERTYP_GIRL,
                                                                               bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2355), (LPCTSTR)qPlane.Name,
                                                                                       (LPCTSTR)Cities[qPlane.Flugplan.Flug[e].VonCity].Name));
                                    }
                                }
                            }

                            if (Delay != 0) {
                                qPlane.Flugplan.Flug[e].Startzeit++;
                                Players.Players[c].Statistiken[STAT_VERSPAETUNG].AddAtPastDay(1);

                                if (qPlane.Flugplan.Flug[e].Startzeit >= 24) {
                                    qPlane.Flugplan.Flug[e].Startzeit -= 24;
                                    qPlane.Flugplan.Flug[e].Startdate++;
                                }
                                qPlane.Flugplan.UpdateNextFlight();
                                qPlane.Flugplan.UpdateNextStart();
                                qPlane.CheckFlugplaene(c);
                                Players.Players[c].UpdateAuftragsUsage();
                            }
                        }
                    }

                    qPlane.Flugplan.UpdateNextStart();
                }
            }
        }

        Airport.RepaintTextBricks();
    }
    // Alle Flugzeuge zum landen in Anflugposition? (Anschließend zum Gate zum aussteigen)
    if (Minute >= 15 && (OldMinute <= 14 || OldMinute > Minute)) {
        count = -100;

        // Flugzeuge von allen Spielern:
        for (c = 0; c < Players.AnzPlayers; c++) {
            PLAYER &qPlayer = Players.Players[c];

            // Für alle Flugzeuge die er besitzt
            for (d = 0; d < qPlayer.Planes.AnzEntries(); d++) {
                if (qPlayer.Planes.IsInAlbum(d) != 0) {
                    CPlane &qPlane = qPlayer.Planes[d];

                    // Testen, ob sie jetzt landen müssen:
                    if (qPlane.Flugplan.NextFlight != -1 && qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Landezeit == GetHour() &&
                        qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Landedate == Date) {
                        CFlugplanEintrag &qFPE = qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight];

                        City = qFPE.NachCity;

                        // Ersatzteil für reparaturbedürftige Flugzeuge:
                        if (qFPE.ObjectType == 2 && qFPE.Passagiere == 0) {
                            SLONG d = 0;

                            for (d = 0; d < qPlayer.Planes.AnzEntries(); d++) {
                                if (qPlayer.Planes.IsInAlbum(d) != 0) {
                                    if (qPlayer.Planes[d].Ort == static_cast<SLONG>(qFPE.NachCity) && (qPlayer.Planes[d].Problem != 0)) {
                                        CPlane &qPlane = qPlayer.Planes[d];

                                        if (qPlane.Problem > 1) {
                                            qPlane.Problem = max(1, qPlane.Problem - 15);

                                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2320),
                                                                                                 (LPCTSTR)Cities[qFPE.NachCity].Name, qPlane.Problem));
                                        }
                                    }
                                }
                            }
                        }

                        if (City == HomeAirportId) {
                            qPlane.Ort = -1; // Landend
                            qPlane.AirportPos = XY(((Airport.LeftEnd * 3 + Airport.RightEnd * 5) / 8 / 2) + count, 22 + count / 2);

                            // TEAKRAND LocalRand (Date+GetHour()+qPlane.TypeId);
                            TEAKRAND LocalRand(Date + GetHour());

                            count -= 105 + LocalRand.Rand(30);
                        } else {
                            if (qPlane.GetFlugplanEintrag()->ObjectType != 3 && qPlane.GetFlugplanEintrag()->ObjectType != 4) {
                                qPlane.CalculateHappyPassengers(c, 0);
                            }

                            qPlane.Ort = City;
                        }
                    }
                }
            }
        }
    }
    // Flugzeuge zum Gate fahren lassen? (Und danach auf die Startbahn)
    if (Minute >= 30 && (OldMinute <= 29 || OldMinute > Minute)) {
        count = 100;

        // Flugzeuge von allen Spielern:
        for (c = 0; c < Players.AnzPlayers; c++) {
            // Für alle Flugzeuge die er besitzt
            for (d = 0; d < Players.Players[c].Planes.AnzEntries(); d++) {
                if (Players.Players[c].Planes.IsInAlbum(d) != 0) {
                    CPlane &qPlane = Players.Players[c].Planes[d];

                    // Testen, ob sie jetzt hinters Fenster müssen; dazu Flugplan durchgehen:
                    if (qPlane.Flugplan.NextFlight != -1) {
                        if ((qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].ObjectType != 0) &&
                            qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Startdate == Date &&
                            qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Startzeit == GetHour()) {
                            Players.Players[c].NumFlights++;
                            PLAYER::NetSynchronizeFlags();

                            City = qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].VonCity;

                            if (City == HomeAirportId && qPlane.GetFlugplanEintrag()->Gate != -1 && qPlane.GetFlugplanEintrag()->Gate != -2 &&
                                qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].ObjectType != 3 &&
                                qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].ObjectType != 4) {
                                // Füllflüge starten direkt; andere holen erst Leute ab
                                if (qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].ObjectType == 3 ||
                                    qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].ObjectType == 4 ||
                                    qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].Gate == -1) {
                                    qPlane.Ort = -2; // Startend
                                    qPlane.AirportPos = XY(-40 - count, 26);
                                } else {
                                    qPlane.Ort = -3; // Hinterm Fenstern
                                    qPlane.AirportPos = XY(Airport.RightEnd + count + 200, 191);

                                    qPlane.TargetX = 200 + Airport.GetRandomTypedRune(RUNE_2WAIT, UBYTE(qPlane.GetFlugplanEintrag()->Gate)).x;
                                    qPlane.Startzeit = UBYTE(qPlane.GetFlugplanEintrag()->Startzeit);

                                    // TEAKRAND LocalRand (Date+GetHour()+qPlane.TypeId);
                                    TEAKRAND LocalRand(Date + GetHour());

                                    count += 50 + LocalRand.Rand(30);
                                }
                            } else {
                                qPlane.Ort = -5; // In der Luft
                                qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].BookFlight(&qPlane, c);

                                if (City == HomeAirportId && qPlane.GetFlugplanEintrag()->Gate == -1 &&
                                    qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].ObjectType != 3 &&
                                    qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].ObjectType != 4) {
                                    Players.Players[c].Messages.AddMessage(
                                        BERATERTYP_GIRL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2353), (LPCTSTR)qPlane.Name,
                                                                 (LPCTSTR)Cities[qPlane.Flugplan.Flug[qPlane.Flugplan.NextFlight].VonCity].Name));
                                    Players.Players[c].Image -= 2;
                                    Limit(SLONG(-1000), Players.Players[c].Image, SLONG(1000));
                                    // log: hprintf ("Player[%li].Image! now = %li", c, (LPCTSTR)Players.Players[c].Image);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ((Options.OptionAutosave != 0) && (Time - GameSpeed) / 60000 < 17 && Time / 60000 >= 17 && (CallItADay == 0)) {
        CString str(StandardTexte.GetS(TOKEN_MISC, 5000));

        SaveGame(11, str);
        qLocalPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 500), MESSAGE_URGENT);
    }
}

//------------------------------------------------------------------------------
// Gibt die aktuelle Woche (1,2,3,4) zurück:
//------------------------------------------------------------------------------
SLONG SIM::GetWeek() const { return (Date % 4); }

//--------------------------------------------------------------------------------------------
// Gibt die aktuelle Stunde zurück:
//--------------------------------------------------------------------------------------------
SLONG SIM::GetHour() const { return (Time / 60000); }

//--------------------------------------------------------------------------------------------
// Gibt die Jahrezeit zurück. Das ist nicht immer SIM::Jahreszeit. Die Variable enthält, die
// zur Zeit *geladene* Jahreszeit.
//--------------------------------------------------------------------------------------------
SLONG SIM::GetSeason() const {
    time_t Time = StartTime + Date * 60 * 60 * 24;
    struct tm *pTimeStruct = localtime(&Time);

    SLONG MonthDay = pTimeStruct->tm_mday;
    SLONG Month = pTimeStruct->tm_mon + 1;

    SLONG DayOfYear = Month * 30 + MonthDay; // Zur Vereinfachung hat jeder Monat 31 Tage:

    // Kann mit dem Debugger manipuliert werden, um die Jahreszeiten zu testen:
    static SLONG SeasonCheater = 0;

#ifdef DEMO
    return (0); // In der Demo nur Winter
#endif

    // Frühlung: 5.3. - 20.5
    if (DayOfYear >= 3 * 30 + 5 && DayOfYear <= 5 * 30 + 20) {
        return ((0 + SeasonCheater) % 4);
    }

    // Sommer: 21.5 - 11.8.
    if (DayOfYear >= 5 * 30 + 21 && DayOfYear <= 8 * 30 + 11) {
        return ((1 + SeasonCheater) % 4);
    }

    // Herbst: 12.8. - 19.12.
    if (DayOfYear >= 8 * 30 + 12 && DayOfYear <= 12 * 30 + 19) {
        return ((2 + SeasonCheater) % 4);
    }

    // Winter: 20.12. - 4.3.
    if (DayOfYear <= 3 * 30 + 4 || DayOfYear >= 12 * 30 + 20) {
        return ((3 + SeasonCheater) % 4);
    }

    return (3);
}

//--------------------------------------------------------------------------------------------
//Überprüft, ob wir die richtigen Bricks geladen haben:
//--------------------------------------------------------------------------------------------
void SIM::UpdateSeason() {
    if (GetSeason() != Jahreszeit) {
        if (Jahreszeit != -1) {
            Bricks.Destroy();
            RuneBms.Destroy();
            pGfxMain->ReleaseLib(pGLibBrick);
        }

        pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("glbrick%li.gli", GliPath, GetSeason())), &pGLibBrick, L_LOCMEM);

        Bricks.ReInit("brick.csv");
        Bricks.UpdateBricks();

        RuneBms.ReSize(pGLibBrick, "RUNE50", 6);

        Jahreszeit = GetSeason();
    }
}

//--------------------------------------------------------------------------------------------
// Gibt die aktuelle Minute zurück:
//--------------------------------------------------------------------------------------------
SLONG SIM::GetMinute() const { return ((Time / 1000) % 60); }

//------------------------------------------------------------------------------
// Gibt die aktuelle Zeit als String zurück:
//------------------------------------------------------------------------------
CString SIM::GetTimeString() const { return (bprintf("%2li:%02li", GetHour(), GetMinute())); }

//------------------------------------------------------------------------------
// Läßt neuen Tag & ggf. Monat beginnen
//------------------------------------------------------------------------------
void SIM::NewDay() {
    SLONG c = 0;
    SLONG d = 0;
    CFlugplan TmpFlugplan;

    AnyPersonsInAirport = FALSE;

    TimeSlice = 0;
    PersonRandMisc.SRand(Date);
    PersonRandCreate.SRand(Date);

    ItemGlove = 1;
    ItemClips = 1;
    ItemGlue = 0;
    ItemKohle = 1;
    ItemParfuem = 1;
    ItemZange = 1;
    ItemPostcard = 1;
    UniqueGameId++;

    if (nSecOutDays > 0) {
        nSecOutDays--;
    }

    for (c = 0; c < 5; c++) {
        Smokers[c].Smoke.ReSize(0);
    }

    // Wochentag für die Öffnungszeiten:
    {
        time_t Time = StartTime + Date * 60 * 60 * 24;
        struct tm *pTimeStruct = localtime(&Time);
        Weekday = pTimeStruct->tm_wday;
    }

    if (Date % 6 == 0) {
        Slimed = -1;
    }

    KeyHints[1] = 0;

    if ((CheatTestGame != 0) && Players.Players[localPlayer].Money < 0) {
        Players.Players[localPlayer].Money = 1000000;
        // log: hprintf ("Event: localPlayer gets Money-Boost for testing reasons");
    }

    IsTutorial = FALSE;
    DialogOvertureFlags &= (~DIALOG_RICK_TODAY);

    RoomBusy.FillWith(0);
    Talkers.Init();

    // Andere Personen anzeigen:
    Clans.UpdateClansInGame(FALSE);
    PersonRandMisc.SRand(Date);
    PersonRandCreate.SRand(Date);

    PersonQueue.NewDay();

    LastAnzPeopleOnScreen = 0;
    LaptopSoldTo = -1;

    /*hprintf ("__Tag_%li_(%li.%li.)____________________________________________________%s", Date+1, MonthDay, Month, CTime::GetCurrentTime().Format(
      "%H:%M:%S")); for (c=0; c<Players.AnzPlayers; c++) if (!Players.Players[c].IsOut)
      {
      hprintf ("%s: %s - Sympathien: %li, %li, %li, %li", (LPCTSTR)Players.Players[c].NameX, (LPCTSTR)Players.Players[c].AirlineX,
      (LPCTSTR)Players.Players[c].Sympathie[0], (LPCTSTR)Players.Players[c].Sympathie[1], (LPCTSTR)Players.Players[c].Sympathie[2],
      (LPCTSTR)Players.Players[c].Sympathie[3]); hprintf ("- Geld: %li-%li=%li, %li Aktien, Kurs %li", (LPCTSTR)Players.Players[c].Money,
      (LPCTSTR)Players.Players[c].Credit, (LPCTSTR)Players.Players[c].Money-Players.Players[c].Credit, (LPCTSTR)Players.Players[c].AnzAktien,
      SLONG(Players.Players[c].Kurse[0])); hprintf ("- Aktien: %li, %li, %li, %li", (LPCTSTR)Players.Players[c].OwnsAktien[0],
      (LPCTSTR)Players.Players[c].OwnsAktien[1], (LPCTSTR)Players.Players[c].OwnsAktien[2], (LPCTSTR)Players.Players[c].OwnsAktien[3]); hprintf ("-
      Bilanz: %li (Gestern: %li), Image: %li", (LPCTSTR)Players.Players[c].Bilanz.GetSumme(), (LPCTSTR)Players.Players[c].BilanzGestern.GetSumme(),
      (LPCTSTR)Players.Players[c].Image); hprintf ("- Routen: %li, Aufträge: %li, Flüge (heute): %li, Flugzeuge: %li",
      (LPCTSTR)Players.Players[c].RentRouten.GetNumUsed(), (LPCTSTR)Players.Players[c].Auftraege.GetNumUsed(),
      (LPCTSTR)Players.Players[c].NumFlights, (LPCTSTR)Players.Players[c].Planes.GetNumUsed());
      } */

    // Remove Person, Kleber und Stinkbomben-Flags:
    for (c = Airport.iPlate.AnzEntries() - 1 - 20 * 16; c >= 0; c--) {
        Airport.iPlate[c] &= (~3);
    }

    Airport.CalcSeats();

    for (c = 0; c < Players.AnzPlayers; c++) {
        if ((Players.Players[c].IsOut == 0) && Players.Players[c].OfficeState == 3) {
            Jahreszeit = 55;
        }
    }

    // Einen neuen Tag beginnen lassen:
    Date++;
    Time = 0;
    UpdateSeason();

    // In den Reisebüros die Zettel nachfüllen:
    gFrachten.Random.SRand(Date);
    LastMinuteAuftraege.Random.SRand(Date + 1);
    ReisebueroAuftraege.Random.SRand(Date + 2);

    LastMinuteAuftraege.FillForLastMinute();
    ReisebueroAuftraege.FillForReisebuero();
    gFrachten.Fill();
    TickReisebueroRefill = TickLastMinuteRefill = TickMuseumRefill = TickFrachtRefill = 6;

    for (c = 0; c < SLONG(Cities.AnzEntries()); c++) {
        AuslandsAuftraege[c].Random.SRand(Date + c + 3);
        AuslandsAuftraege[c].FillForAusland(c);
        AuslandsRefill[c] = 6;

        AuslandsFrachten[c].Random.SRand(Date + c + 3);
        AuslandsFrachten[c].FillForAusland(c);
        AuslandsFRefill[c] = 6;
    }

    MonthDay++;
    if (MonthLength[Month - 1] <= MonthDay) {
        Month++;
        if (Month == 13) {
            Month = 1;
        }
        MonthDay = 1;
    }

    if (ExpandAirport != 0) {
        ExpandAirport = 0;
        CheckIn++;
        WaitZone++;

        Suitcase = WaitZone;

        LastExpansionDate = Date;

        Airport.LoadAirport(LeftEnd, CheckIn, Office, Entry, Shops, Cafe, Security, Suitcase, WaitZone, RightEnd);
    } else if (bReloadAirport != 0) {
        Airport.LoadAirport(LeftEnd, CheckIn, Office, Entry, Shops, Cafe, Security, Suitcase, WaitZone, RightEnd);
    }

    bReloadAirport = 0;

    // Kerosinpreis
    for (c = 0; c < KerosinPast.AnzEntries() - 1; c++) {
        KerosinPast[c] = KerosinPast[c + 1];
    }

    TEAKRAND KerosinRand(Date);
    Kerosin += (KerosinRand.Rand(21)) - 10 + static_cast<SLONG>(KerosinRand.Rand(20) < 3) * ((KerosinRand.Rand(41)) - 20) +
               SLONG(sin((Date + KerosinRand.Rand(6)) / 3.0) * 20) + SLONG(sin(Date * 1.7) * 20);
    if (Difficulty == DIFF_ATFS09 || (Difficulty == DIFF_ATFS10 && ((Date >= 3 && Date <= 10) || (Date >= 35 && Date <= 55)))) {
        if (KerosinRand.Rand(20) > 2) {
            Kerosin += Kerosin / 4;
        } else if (KerosinPast[8] > 550 && KerosinPast[7] > 550 && KerosinPast[6] > 550) {
            Kerosin -= Kerosin / 2;
        }
        Limit(SLONG(300), Kerosin, SLONG(700));
        for (c = 0; c < 20; c++) {
            Kerosin += (KerosinRand.Rand(21)) - 10 + static_cast<SLONG>(KerosinRand.Rand(20) < 3) * ((KerosinRand.Rand(41)) - 20);
        }
    }

    Limit(SLONG(300), Kerosin, SLONG(700));
    KerosinPast[9] = Kerosin;

    for (c = 0; c < Players.AnzPlayers; c++) {
        if (Players.Players[c].IsOut == 0) {
            Players.Players[c].NewDay();
        }
    }

    AIRPORT::UpdateStaticDoorImage();

    // Neue Flugzeuge für's Museum:
    CreateRandomUsedPlanes();

    // Variablen ersetzen
    Headlines.ComparisonHeadlines(); // Vergleiche

    // Bedarf der Leute ermitteln:
    Routen.NewDay();

    // Kopierschutz:
    /*CheckSomeCdFileLength ();*/

    // Die Schlagzeilen der Zeitungen laden:
    Headlines.ReloadHeadline();
    Headlines.InterpolateHeadline();
    Headlines.SortByPriority();

    // Gates, Cities und Routen versteigern:
    {
        CString GotIt;
        CString GotItNot;

        for (c = 0; c < 7; c++) {
            // City:
            if (TafelData.City[c].ZettelId > -1 && TafelData.City[c].Player != -1) {
                CRentCity &NewCity = Players.Players[TafelData.City[c].Player].RentCities.RentCities[TafelData.City[c].ZettelId];

                if (TafelData.City[c].WasInterested != 0) {
                    if (TafelData.City[c].Player == localPlayer) {
                        GotIt += bprintf("%s: %s\xb5", LPCTSTR(StandardTexte.GetS(TOKEN_MISC, 2610)), LPCTSTR(Cities[TafelData.City[c].ZettelId].Name));
                    } else {
                        GotItNot += bprintf("%s: %s\xb5", LPCTSTR(StandardTexte.GetS(TOKEN_MISC, 2610)), LPCTSTR(Cities[TafelData.City[c].ZettelId].Name));
                    }
                }

                NewCity.Rang = UBYTE(TafelData.City[c].Rang);
                NewCity.Image = 0;
                NewCity.Miete = TafelData.City[c].Preis;

                // hprintf ("Event: %s (Player %li) buys %s.", Players.Players[TafelData.City[c].Player].NameX, TafelData.City[c].Player+1,
                // (LPCTSTR)Cities[TafelData.City[c].ZettelId].Name);

                Players.Players[TafelData.City[c].Player].ChangeMoney(-TafelData.City[c].Preis * 3,
                                                                      2040, // Ersteigerung eines Schalters
                                                                      Cities[TafelData.City[c].ZettelId].Name);

                TafelData.City[c].ZettelId = -1;
                TafelData.City[c].Player = -1;
            }

            // Route:
            if ((TafelData.Route[c].ZettelId > 0) && TafelData.Route[c].Player != -1) {
                PLAYER &qPlayer = Players.Players[TafelData.Route[c].Player];

                if (TafelData.Route[c].WasInterested != 0) {
                    if (TafelData.Route[c].Player == localPlayer) {
                        GotIt += bprintf("%s: %s-%s\xb5", LPCTSTR(StandardTexte.GetS(TOKEN_MISC, 2611)),
                                         LPCTSTR(Cities[Routen[TafelData.Route[c].ZettelId].VonCity].Name),
                                         LPCTSTR(Cities[Routen[TafelData.Route[c].ZettelId].NachCity].Name));
                    } else {
                        GotItNot += bprintf("%s: %s-%s\xb5", LPCTSTR(StandardTexte.GetS(TOKEN_MISC, 2611)),
                                            LPCTSTR(Cities[Routen[TafelData.Route[c].ZettelId].VonCity].Name),
                                            LPCTSTR(Cities[Routen[TafelData.Route[c].ZettelId].NachCity].Name));
                    }
                }

                qPlayer.RentRoute(Routen[TafelData.Route[c].ZettelId].VonCity, Routen[TafelData.Route[c].ZettelId].NachCity, TafelData.Route[c].Preis);

                qPlayer.ChangeMoney(-TafelData.Route[c].Preis * 3,
                                    2041, // Ersteigerung einer Route
                                    CString(bprintf("%s-%s", (LPCTSTR)Cities[Routen[TafelData.Route[c].ZettelId].VonCity].Kuerzel,
                                                    (LPCTSTR)Cities[Routen[TafelData.Route[c].ZettelId].NachCity].Kuerzel)));

                TafelData.Route[c].ZettelId = 0;
                TafelData.Route[c].Player = -1;
            }

            // Gate:
            if (TafelData.Gate[c].ZettelId > -1 && TafelData.Gate[c].Player != -1) {
                PLAYER &qPlayer = Players.Players[TafelData.Gate[c].Player];

                if (TafelData.Gate[c].WasInterested != 0) {
                    if (TafelData.Gate[c].Player == localPlayer) {
                        GotIt += bprintf("%s: %li\xb5", LPCTSTR(StandardTexte.GetS(TOKEN_MISC, 2612)), TafelData.Gate[c].ZettelId + 1);
                    } else {
                        GotItNot += bprintf("%s: %li\xb5", LPCTSTR(StandardTexte.GetS(TOKEN_MISC, 2612)), TafelData.Gate[c].ZettelId + 1);
                    }
                }

                for (d = 0; d < qPlayer.Gates.Gates.AnzEntries(); d++) {
                    if (qPlayer.Gates.Gates[d].Miete == -1) {
                        qPlayer.Gates.NumRented++;
                        qPlayer.Gates.Gates[d].Miete = TafelData.Gate[c].Preis;
                        qPlayer.Gates.Gates[d].Nummer = TafelData.Gate[c].ZettelId;
                        break;
                    }
                }

                // hprintf ("Event: %s (Player %li) buys Gate.", Players.Players[TafelData.Gate[c].Player].NameX, TafelData.Route[c].Player+1);

                // Ersteigerung eines Gates:
                qPlayer.ChangeMoney(-TafelData.Gate[c].Preis * 3, 2042, "");

                TafelData.Gate[c].ZettelId = -1;
                TafelData.Gate[c].Player = -1;
            }
        }

        // Verschicken:
        if (GotIt.GetLength() > 0 || GotItNot.GetLength() > 0) {
            CString Letter;

            if (GotIt.GetLength() > 0) {
                Letter += StandardTexte.GetS(TOKEN_MISC, 2602) + GotIt;
            }
            if (GotItNot.GetLength() > 0) {
                if (GotIt.GetLength() > 0) {
                    Letter += "\xb5\xb5";
                }
                Letter += StandardTexte.GetS(TOKEN_MISC, 2603) + GotItNot;
            }

            Players.Players[localPlayer].Letters.AddLetter(TRUE, StandardTexte.GetS(TOKEN_MISC, 2600), Letter, StandardTexte.GetS(TOKEN_MISC, 2601), -1);
        }
    }

    ReformGates();

    Airport.CreateGateMapper();

    // Aufträge, Routen, Etc.
    TafelData.Randomize(min(Date + 2, 7));

    Workers.NewDay();

    // Gute Spieler sind den anderen unsympatisch:
    for (c = 0; c < Players.AnzPlayers; c++) {
        for (d = 0; d < Players.AnzPlayers; d++) {
            if (c != d && Players.Players[c].Sympathie[d] < 25 && Players.Players[d].Bilanz.GetSumme() > 0) {
                if (Players.Players[c].Sympathie[d] < -50 && Players.Players[d].Bilanz.GetSumme() > Players.Players[c].Bilanz.GetSumme()) {
                    if (Players.Players[d].Bilanz.GetSumme() > Players.Players[c].Bilanz.GetSumme() + 1000000) {
                        Players.Players[c].Sympathie[d] -= 10 * static_cast<SLONG>(Players.Players[d].Owner != 1);
                    }
                    if (Players.Players[d].Bilanz.GetSumme() > Players.Players[c].Bilanz.GetSumme() + 100000) {
                        Players.Players[c].Sympathie[d] -= 2 * static_cast<SLONG>(Players.Players[d].Owner != 1);
                    }
                } else if (Players.Players[d].Bilanz.GetSumme() > Players.Players[c].Bilanz.GetSumme() * 2) {
                    if (Players.Players[d].Bilanz.GetSumme() > Players.Players[c].Bilanz.GetSumme() + 1000000) {
                        Players.Players[c].Sympathie[d] -= 5 * static_cast<SLONG>(Players.Players[d].Owner != 1);
                    }
                    if (Players.Players[d].Bilanz.GetSumme() > Players.Players[c].Bilanz.GetSumme() + 100000) {
                        Players.Players[c].Sympathie[d] -= 1 * static_cast<SLONG>(Players.Players[d].Owner != 1);
                    }
                }
            }

            if (abs(Players.Players[c].Sympathie[d]) > 70) {
                Players.Players[c].Sympathie[d] = Players.Players[c].Sympathie[d] * 69 / 70;
            }
        }
    }

    // Bei ATFS-Megasabotage-Mission ggf. künstlich Sabotage einfügen:
    if (Difficulty == DIFF_ATFS06) {
        TEAKRAND SaboRand(Date + SLONG(Players.Players[localPlayer].Money));

        for (c = 0; c < Players.AnzPlayers; c++) {
            if (c != localPlayer) {
                PLAYER &qPlayer = Players.Players[c];

                if ((qPlayer.IsOut == 0) && qPlayer.ArabHints < 90 && qPlayer.ArabMode == 0 && qPlayer.ArabMode2 == 0 && qPlayer.ArabMode3 == 0) {
                    PLAYER &qOpfer = Players.Players[static_cast<SLONG>(SaboRand.Rand(4))];

                    if ((qOpfer.IsOut == 0) && qPlayer.PlayerNum != qOpfer.PlayerNum && qOpfer.Planes.GetNumUsed() > 0) {
                        switch (SaboRand.Rand(3)) {
                        case 0:
                            qPlayer.ArabMode = -(SaboRand.Rand(4) + 1);
                            break;
                        case 1:
                            qPlayer.ArabMode2 = -(SaboRand.Rand(4) + 1);
                            break;
                        case 2:
                            qPlayer.ArabMode3 = -(SaboRand.Rand(5) + 1);
                            break;
                        default:
                            hprintf("Sim.cpp: Default case should not be reached.");
                            DebugBreak();
                        }

                        qPlayer.ArabActive = FALSE;
                        qPlayer.ArabPlane = qOpfer.Planes.GetRandomUsedIndex(&SaboRand);

                        qPlayer.ArabOpfer = qPlayer.ArabOpfer2 = qPlayer.ArabOpfer3 = qOpfer.PlayerNum;

                        if (qPlayer.ArabMode2 == -2 && (qOpfer.HasItem(ITEM_LAPTOP) == 0)) {
                            qPlayer.ArabMode2 = 0;
                        }

                        // Wegen Security-Office:
                        if (qPlayer.ArabMode == -1 && ((qOpfer.SecurityFlags & (1 << 6)) != 0U)) {
                            qPlayer.ArabMode = 0;
                        }
                        if (qPlayer.ArabMode == -2 && ((qOpfer.SecurityFlags & (1 << 6)) != 0U)) {
                            qPlayer.ArabMode = 0;
                        }
                        if (qPlayer.ArabMode == -3 && ((qOpfer.SecurityFlags & (1 << 7)) != 0U)) {
                            qPlayer.ArabMode = 0;
                        }
                        if (qPlayer.ArabMode == -4 && ((qOpfer.SecurityFlags & (1 << 7)) != 0U)) {
                            qPlayer.ArabMode = 0;
                        }

                        if (qPlayer.ArabMode2 == -1 && ((qOpfer.SecurityFlags & (1 << 0)) != 0U)) {
                            qPlayer.ArabMode2 = 0;
                        }
                        if (qPlayer.ArabMode2 == -2 && ((qOpfer.SecurityFlags & (1 << 1)) != 0U)) {
                            qPlayer.ArabMode2 = 0;
                        }
                        if (qPlayer.ArabMode2 == -3 && ((qOpfer.SecurityFlags & (1 << 0)) != 0U)) {
                            qPlayer.ArabMode2 = 0;
                        }
                        if (qPlayer.ArabMode2 == -4 && ((qOpfer.SecurityFlags & (1 << 2)) != 0U)) {
                            qPlayer.ArabMode2 = 0;
                        }

                        if (qPlayer.ArabMode3 == -1 && ((qOpfer.SecurityFlags & (1 << 8)) != 0U)) {
                            qPlayer.ArabMode3 = 0;
                        }
                        if (qPlayer.ArabMode3 == -2 && ((qOpfer.SecurityFlags & (1 << 5)) != 0U)) {
                            qPlayer.ArabMode3 = 0;
                        }
                        if (qPlayer.ArabMode3 == -3 && ((qOpfer.SecurityFlags & (1 << 5)) != 0U)) {
                            qPlayer.ArabMode3 = 0;
                        }
                        if (qPlayer.ArabMode3 == -4 && ((qOpfer.SecurityFlags & (1 << 3)) != 0U)) {
                            qPlayer.ArabMode3 = 0;
                        }
                        if (qPlayer.ArabMode3 == -5 && ((qOpfer.SecurityFlags & (1 << 8)) != 0U)) {
                            qPlayer.ArabMode3 = 0;
                        }
                    }
                }
            }
        }
    }

    // Leerzeilen in Moneytips & "Call it a day" Flag
    for (c = 0; c < Players.AnzPlayers; c++) {
        Players.Players[c].History.AddEntry(0, "-------------------------------------");
        Players.Players[c].CallItADay = FALSE;

        Players.Players[c].BilanzWoche.NeuerTag(Players.Players[c].Bilanz);
        Players.Players[c].BilanzGesamt += Players.Players[c].Bilanz;
        Players.Players[c].BilanzGestern = Players.Players[c].Bilanz;
        Players.Players[c].Bilanz.Clear();
    }

    CallItADay = FALSE;
}

//--------------------------------------------------------------------------------------------
// Sucht ein zufälliges Flugzeug für heute aus:
//--------------------------------------------------------------------------------------------
CPlane SIM::CreateRandomUsedPlane(SLONG seed) const {
    TEAKRAND rnd;

    rnd.SRand((Date + seed) * 0x504020 + static_cast<ULONG>(StartTime));

    // Get Random Time: Now > Desired Time > Release Year

    const SLONG thisYear = GetCurrentYear();

    CPlane usedPlane = CPlane(PlaneNames.GetUnused(&rnd), PlaneTypes.GetRandomExistingType(&rnd), 100, 0);

    if (thisYear < usedPlane.ptErstbaujahr) {
        TeakLibW_Exception(FNL, "Tried to add used plane that was built before this year (%d < %d)", thisYear, usedPlane.ptErstbaujahr);
    }

    if (thisYear == usedPlane.ptErstbaujahr) {
        usedPlane.Baujahr = thisYear; //fallback for when the plane is brand new
    } else {
        usedPlane.Baujahr = thisYear - rnd.Rand(thisYear - usedPlane.ptErstbaujahr);
    }
    usedPlane.Zustand = static_cast<UBYTE>(usedPlane.Baujahr - usedPlane.ptErstbaujahr + 25 + rnd.Rand(40) - 20);
    usedPlane.Zustand = static_cast<UBYTE>(max(20, min(usedPlane.Zustand, 100)));
    usedPlane.TargetZustand = usedPlane.Zustand;

    return usedPlane;
}

//--------------------------------------------------------------------------------------------
// Sucht drei zufällige Flugzeuge für heute aus:
//--------------------------------------------------------------------------------------------
void SIM::CreateRandomUsedPlanes() {
    SLONG c = 0;

    UsedPlanes.ReSize(3);
    UsedPlanes.ClearAlbum();

    for (c = 0; c < 3; c++) {
        UsedPlanes.push_front(0x1000000 + c, CreateRandomUsedPlane(c));
    }

    if (Difficulty == DIFF_ATFS10 && Date >= 40 && Date <= 50) {
        for (c = 0; c < SLONG(UsedPlanes.AnzEntries()); c++) {
            UsedPlanes[0x1000000 + c].Name = "";
        }
    }
}

//--------------------------------------------------------------------------------------------
// Füllt nach einiger Zeit die Flieger wieder auf:
//--------------------------------------------------------------------------------------------
void SIM::UpdateUsedPlanes() {
    SLONG c = 0;
    SLONG Anz =
        min(SLONG(UsedPlanes.AnzEntries()), TickMuseumRefill / 20); // Normalerweise war das früher Time-Last / 5000, hier aber /100000, also effektiv /20

    for (c = 0; c < SLONG(UsedPlanes.AnzEntries()) && Anz > 0; c++) {
        if (UsedPlanes[0x1000000 + c].Name.GetLength() == 0) {
            UsedPlanes[0x1000000 + c] = CreateRandomUsedPlane(c);
            Anz--;
            TickMuseumRefill = 0;
        }
    }

    if (Difficulty == DIFF_ATFS10 && Date >= 40 && Date <= 50) {
        for (c = 0; c < SLONG(UsedPlanes.AnzEntries()); c++) {
            UsedPlanes[0x1000000 + c].Name = "";
        }
    }
}

//--------------------------------------------------------------------------------------------
// Die Gates werden ggf. vertauscht um in der perfekten Reihenfolge zu sein:
//--------------------------------------------------------------------------------------------
void SIM::ReformGates() {
    SLONG c = 0;
    SLONG d = 0;
    SLONG n = 0;

    for (c = n = 0; c < 4; c++) {
        if (Players.Players[c].IsOut == 0) {
            for (d = 0; d < Players.Players[c].Gates.NumRented; d++) {
                if (Players.Players[c].Gates.Gates[d].Miete != -1) {
                    Players.Players[c].Gates.Gates[d].Nummer = n;
                    n++;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Ggf. Soundeffekt abspielen, wenn Computer auf Toilette geht:
//--------------------------------------------------------------------------------------------
void SIM::ComputerOnToilet(SLONG ComputerPlayer) {
    if (Players.Players[localPlayer].GetRoom() == ROOM_AIRPORT) {
        if (abs(Players.Players[localPlayer].ViewPos.x - Persons[Persons.GetPlayerIndex(ComputerPlayer)].ScreenPos.x) < 400) {
            gToiletFx.ReInit("toilet.raw");
            gToiletFx.Play(0, Options.OptionEffekte * 100 / 7);
        }
    }

    Players.Players[ComputerPlayer].Stunned = 70;
}

//--------------------------------------------------------------------------------------------
// Gives the user a hint (if he didn't already get enough)
//--------------------------------------------------------------------------------------------
void SIM::GiveHint(SLONG Number) {
    PLAYER &qPlayer = Players.Players[localPlayer];

    if (qPlayer.LocationWin != nullptr) {
        if ((qPlayer.LocationWin)->MenuIsOpen() == 0 && (qPlayer.LocationWin)->IsDialogOpen() == 0) {
            if ((KeyHints[0] & (1 << Number)) == 0 || ((KeyHints[1] & (1 << Number)) == 0 && (KeyHints[2] & (1 << Number)) == 0)) {
                if ((KeyHints[0] & (1 << Number)) == 0) {
                    KeyHints[0] |= (1 << Number);
                } else {
                    KeyHints[2] |= (1 << Number);
                }

                KeyHints[1] |= (1 << Number);

                qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TOOLTIP, 6000 + Number));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Offensichtlich kennt der Spieler diese Funktion schon. Also nicht mehr nerver:
//--------------------------------------------------------------------------------------------
void SIM::InvalidateHint(SLONG Number) {
    KeyHints[0] |= (1 << Number);
    KeyHints[2] |= (1 << Number);
}

//--------------------------------------------------------------------------------------------
// Speichert ein Sim-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const SIM &Sim) {
    SLONG Dummy = 0; // Ehem. Sim.IsDrunk;

    // Daten:
    File.WriteTrap(100);
    File << Sim.Persons << Sim.Players << Sim.UsedPlanes;
    File << Sim.PersonQueue;
    File << Sim.Headlines << Sim.Kerosin << Sim.KerosinPast << Sim.localPlayer;
    File << Sim.FocusPerson << Dummy << Sim.SabotageActs;
    File << Sim.TicksPlayed << Sim.NumRestored << Sim.bExeChanged;

    // Flags & Co.:
    File.WriteTrap(100);
    File << Sim.bCheatedSession;
    File << Sim.ExpandAirport << Sim.Gamestate << Sim.Difficulty << Sim.IsTutorial;
    File << Sim.GameSpeed << Sim.bPause << Sim.bNoTime << Sim.DialogOvertureFlags;
    File << Sim.DayState << Sim.CallItADay << Sim.Tutorial;
    File << Sim.LaptopSoldTo << Sim.MoneyInBankTrash << Sim.Slimed;
    File << Sim.UsedTelescope << Sim.DontDisplayPlayer;
    File << Sim.MissionCities;

    if (SaveVersionSub >= 100) {
        File << Sim.ItemGlove << Sim.ItemClips << Sim.ItemGlue << Sim.ItemPostcard;
        File << Sim.ItemKohle << Sim.ItemParfuem << Sim.ItemZange;

        File << Sim.TickFrachtRefill;
        File << Sim.KrisenCity;
        File << Sim.UsedPlaneProp2;

        File << Sim.RFActivePage;
        File << Sim.RFTriebwerk << Sim.RFReifen << Sim.RFElektronik << Sim.RFSicherheit;

        File << Sim.bIsHost;
    }

    if (SaveVersionSub >= 106) {
        File << Sim.nSecOutDays;
    }

    // Das Referenzflugzeug:
    File << Sim.PlanePropId << Sim.RFBegleiterFaktor << Sim.RFSitzeRF;
    File << Sim.RFEssen << Sim.RFTabletts << Sim.RFDeco;

    // Die Statistik:
    File << Sim.StatfGraphVisible;
    File << Sim.Statgroup << Sim.Statdays << Sim.StatnewDays << Sim.DropDownPosY;
    File << Sim.StatplayerMask;
    File << Sim.StatiArray;

    // Der maximale Schwierigkeitsgrad;
    File.WriteTrap(100);
    File << Sim.MaxDifficulty;

    // Datum und Zeit:
    File.WriteTrap(100);
    SLONG SimDate = Sim.Date;
    File << SimDate << Sim.Time << Sim.Month;
    File << Sim.MonthDay << Sim.QuitCountDown;
    File << Sim.TickerTime << Sim.TimeSlice << Sim.StartWeekday << Sim.Weekday;
    File.Write(reinterpret_cast<const UBYTE *>(&Sim.StartTime), sizeof(4)); // 64Bit Size change!

    // Sonstiges:
    File.WriteTrap(100);
    File << Sim.TickReisebueroRefill << Sim.TickLastMinuteRefill << Sim.TickMuseumRefill;

    // Die Ausbauzustände des Flughafens:
    File.WriteTrap(100);
    File << Sim.LeftEnd << Sim.CheckIn << Sim.Office << Sim.Entry << Sim.Shops;
    File << Sim.Cafe << Sim.Security << Sim.Suitcase << Sim.WaitZone << Sim.RightEnd;

    File.WriteTrap(100);
    File << Sim.OvertakenAirline;
    File << Sim.OvertakerAirline;
    File << Sim.Overtake;

    File.WriteTrap(100);
    File << Sim.HomeAirportId << Sim.RoomBusy;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein Sim-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, SIM &Sim) {
    SLONG c = 0;
    SLONG tmp = 0;
    SLONG Dummy = 0;
    ULONG NetworkIDs[4];
    BOOL bReadyForMornings[4];

    Sim.AirportSmacks.ReSize(0);

    for (c = 0; c < 4; c++) {
        NetworkIDs[c] = Sim.Players.Players[c].NetworkID;
        bReadyForMornings[c] = Sim.Players.Players[c].bReadyForMorning;
    }

    // Daten:
    File.ReadTrap(100);
    File >> Sim.Persons >> Sim.Players >> Sim.UsedPlanes;
    File >> Sim.PersonQueue;
    File >> Sim.Headlines >> Sim.Kerosin >> Sim.KerosinPast >> Sim.localPlayer;
    File >> Sim.FocusPerson >> Dummy >> Sim.SabotageActs;
    File >> Sim.TicksPlayed >> Sim.NumRestored >> Sim.bExeChanged;

    for (c = 0; c < 4; c++) {
        Sim.Players.Players[c].NetworkID = NetworkIDs[c];
        Sim.Players.Players[c].bReadyForMorning = bReadyForMornings[c];
    }

    // Flags & Co.:
    File.ReadTrap(100);
    File >> Sim.bCheatedSession;
    File >> Sim.ExpandAirport >> Sim.Gamestate >> Sim.Difficulty >> Sim.IsTutorial;
    File >> Sim.GameSpeed >> Sim.bPause >> Sim.bNoTime >> Sim.DialogOvertureFlags;
    File >> Sim.DayState >> Sim.CallItADay >> Sim.Tutorial;
    File >> Sim.LaptopSoldTo >> Sim.MoneyInBankTrash >> Sim.Slimed;
    File >> Sim.UsedTelescope >> Sim.DontDisplayPlayer;
    File >> Sim.MissionCities;

    if (SaveVersionSub >= 100) {
        File >> Sim.ItemGlove >> Sim.ItemClips >> Sim.ItemGlue >> Sim.ItemPostcard;
        File >> Sim.ItemKohle >> Sim.ItemParfuem >> Sim.ItemZange;

        File >> Sim.TickFrachtRefill;
        File >> Sim.KrisenCity;
        File >> Sim.UsedPlaneProp2;

        File >> Sim.RFActivePage;
        File >> Sim.RFTriebwerk >> Sim.RFReifen >> Sim.RFElektronik >> Sim.RFSicherheit;

        BOOL bIsHostDummy = 0;
        File >> bIsHostDummy; // Sim.bIsHost; (bringt probleme, wenn wir den Host so ändern; wir müssen beim DX Wert bleiben)
    } else {
        Sim.ItemGlove = Sim.ItemClips = 1;
        Sim.ItemGlue = 0;
        Sim.ItemPostcard = 1;
        Sim.TickFrachtRefill = 0;
        Sim.KrisenCity = 0;
        Sim.UsedPlaneProp2 = FALSE;

        Sim.RFActivePage = 100;
        Sim.RFTriebwerk = Sim.RFReifen = Sim.RFElektronik = Sim.RFSicherheit = 0;
        Sim.bIsHost = 0;
    }

    if (SaveVersionSub >= 106) {
        File >> Sim.nSecOutDays;
    }

    // Das Referenzflugzeug:
    File >> Sim.PlanePropId >> Sim.RFBegleiterFaktor >> Sim.RFSitzeRF;
    File >> Sim.RFEssen >> Sim.RFTabletts >> Sim.RFDeco;

    // Die Statistik:
    File.Read(reinterpret_cast<UBYTE *>(&Sim.StatfGraphVisible), sizeof(Sim.StatfGraphVisible));
    File >> Sim.Statgroup >> Sim.Statdays >> Sim.StatnewDays >> Sim.DropDownPosY;
    File >> Sim.StatplayerMask;
    if (SaveVersionSub >= 200) {
        File >> Sim.StatiArray;
    } else {
        File.Skip(3 * 16 * sizeof(bool));  
    }

    // Der maximale Schwierigkeitsgrad;
    File.ReadTrap(100);
    File >> tmp;
    Sim.MaxDifficulty = max(tmp, Sim.MaxDifficulty);

    // Datum und Zeit:
    File.ReadTrap(100);
    SLONG SimDate = 0;
    File >> SimDate >> Sim.Time >> Sim.Month;
    File >> Sim.MonthDay >> Sim.QuitCountDown;
    File >> Sim.TickerTime >> Sim.TimeSlice >> Sim.StartWeekday >> Sim.Weekday;
    File.Read(reinterpret_cast<UBYTE *>(&Sim.StartTime), 4); // sizeof (Sim.StartTime)
    Sim.Date = SimDate;

    // Sonstiges:
    File.ReadTrap(100);
    File >> Sim.TickReisebueroRefill >> Sim.TickLastMinuteRefill >> Sim.TickMuseumRefill;

    // Die Ausbauzustände des Flughafens:
    File.ReadTrap(100);
    File >> Sim.LeftEnd >> Sim.CheckIn >> Sim.Office >> Sim.Entry >> Sim.Shops;
    File >> Sim.Cafe >> Sim.Security >> Sim.Suitcase >> Sim.WaitZone >> Sim.RightEnd;

    File.ReadTrap(100);
    File >> Sim.OvertakenAirline;
    File >> Sim.OvertakerAirline;
    File >> Sim.Overtake;

    File.ReadTrap(100);
    File >> Sim.HomeAirportId >> Sim.RoomBusy;

    if (Sim.Players.Players[Sim.localPlayer].Messages.AktuellerBeraterTyp != -1) {
        Sim.Players.Players[Sim.localPlayer].Messages.PaintMessage();
    }

    if (Sim.GetHour() < 18) {
        for (SLONG c = 0; c < 4; c++) {
            Sim.Players.Players[c].CallItADay = 0;
        }

        Sim.CallItADay = 0;
    }

    Sim.NumRestored++;
    Sim.TicksSinceLoad = 0;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt einen Spielstand:
//--------------------------------------------------------------------------------------------
BOOL SIM::LoadGame(SLONG Number) {
    const char *pNamebaseStr = nullptr;

    if (bNetwork != 0) {
        pNamebaseStr = "net%li.dat";
    } else {
        pNamebaseStr = "game%li.dat";
    }

    CString Filename = FullFilename((LPCTSTR)bprintf(pNamebaseStr, Number), SavegamePath);
    CString Dummy;
    SLONG c = 0;
    SLONG d = 0;
    SLONG NumSaveGameCities = 55;

    VoiceScheduler.Clear();
    gTimerCorrection = 0;

    if (DoesFileExist(Filename) == 0) {
        return (FALSE);
    }

    bgIsLoadingSavegame = true;

    // Ein paar Dinge, wenn man vom Hauptmenü kommt:
    if (NewgameWantsToLoad != 0) {
        Gamestate = GAMESTATE_PLAYING | GAMESTATE_WORKING;
        DayState = 1;

        delete TopWin;
        TopWin = nullptr;

        BOOL bReadyForMornings[4];

        if (Routen.AnzEntries() == 0) {
            Routen.ReInit("routen.csv", true);
        }

        for (c = 0; c < 4; c++) {
            bReadyForMornings[c] = Players.Players[c].bReadyForMorning;
        }
        ChooseStartup(1);
        for (c = 0; c < 4; c++) {
            Players.Players[c].bReadyForMorning = bReadyForMornings[c];
        }

        for (c = 0; c < Players.AnzPlayers; c++) {
            if (Players.Players[c].LocationWin != nullptr) {
                delete Players.Players[c].LocationWin;
                Players.Players[c].LocationWin = nullptr;
            }
        }
    }

    for (c = 0; c < Players.Players.AnzEntries(); c++) {
        if (Players.Players[c].Owner == 0) {
            if (Players.Players[c].LocationWin != nullptr) {
                delete Players.Players[c].LocationWin;
                Players.Players[c].LocationWin = nullptr;
            }
        }
    }

    TEAKFILE InputFile(Filename, TEAKFILE_READ);

    // Names des Spielstandes:
    InputFile >> Dummy;

    // Versionsnummer:
    InputFile >> SaveVersion >> SaveVersionSub;

    if (SaveVersionSub > 0) {
        // Info-String ignorieren
        InputFile >> Dummy;
    }

    if (SaveVersionSub >= 100) {
        BOOL BoolDummy = 0;
        UBYTE UByteDummy = 0;
        SLONG SlongDummy = 0;

        InputFile >> UniqueGameId >> BoolDummy >> SlongDummy >> SlongDummy;
        InputFile >> UByteDummy >> UByteDummy >> UByteDummy >> UByteDummy;

        InputFile >> bCompatibleRoutes;
    } else {
        bCompatibleRoutes = 1;
    }

    if (SaveVersionSub >= 101) {
        InputFile >> SessionName;
        InputFile >> bAllowCheating;
    } else {
        SessionName = bprintf("%-25s", StandardTexte.GetS(TOKEN_NEWGAME, 703));
        bAllowCheating = 0;
    }

    Routen.ReInit("routen.csv", bCompatibleRoutes == 0);

    InputFile.ReadTrap(100);

    if (SaveVersionSub >= 104) {
        InputFile >> NumSaveGameCities;

        if (SaveVersionSub >= 200 && NumSaveGameCities != Cities.AnzEntries()) {
            TeakLibW_Exception(FNL, "Incorrect City count in savegame: %d (savegame) vs. %d (/data)", NumSaveGameCities, Cities.AnzEntries());
        }
    }

    // Das Spielgerüst:
    InputFile >> Sim;
    InputFile.ReadTrap(100);

    InputFile >> Airport;
    InputFile.ReadTrap(100);

    InputFile >> LastAnzPeopleOnScreen;
    InputFile.ReadTrap(100);

    // Worker & Talker:
    InputFile >> Workers;
    InputFile.ReadTrap(100);

    InputFile >> Talkers;
    InputFile.ReadTrap(100);

    // Sonstige Daten
    InputFile >> TafelData >> Routen;
    InputFile.ReadTrap(100);

    InputFile >> LastMinuteAuftraege;
    InputFile >> ReisebueroAuftraege;
    InputFile >> UniqueGameId2;

    if (SaveVersionSub >= 100) {
        InputFile >> gFrachten;
    } else {
        gFrachten.Fill();
    }

    for (c = 0; c < SLONG(Cities.AnzEntries()); c++) {
        if (Cities[c].bNewInAddOn != 2 || NumSaveGameCities >= 75) {
            if (SaveVersionSub >= 100 || (Cities[c].bNewInAddOn == 0)) {
                InputFile >> AuslandsAuftraege[c];
                InputFile >> AuslandsRefill[c];
            }
        } else {
            AuslandsAuftraege[c].Random.SRand(Date + c + 3);
            AuslandsAuftraege[c].FillForAusland(c);
            AuslandsRefill[c] = 6;
        }

        if (SaveVersionSub >= 105) {
            InputFile >> AuslandsFrachten[c];
            InputFile >> AuslandsFRefill[c];
        } else {
            AuslandsFrachten[c].Random.SRand(Date + c + 3);
            AuslandsFrachten[c].FillForAusland(c);
            AuslandsFRefill[c] = 6;
        }
    }

    InputFile.ReadTrap(100);

    InputFile >> MouseWait;
    MouseWait = 0;

    for (c = 0; c < Players.Players.AnzEntries(); c++) {
        // Update Quality of all local planes:
        if ((Players.Players[c].IsOut == 0) && Players.Players[c].Owner != 2) {
            for (d = 0; d < Players.Players[c].Planes.AnzEntries(); d++) {
                if (Players.Players[c].Planes.IsInAlbum(d) != 0) {
                    Players.Players[c].Planes[d].UpdatePersonalQuality(c);
                }
            }
        }

        if (Players.Players[c].Owner == 0) {
            for (d = 9; d >= 0; d--) {
                if ((Players.Players[c].Locations[d] & (~ROOM_ENTERING) & (~ROOM_LEAVING)) == ROOM_OPTIONS) {
                    Players.Players[c].Locations[d] = 0;
                } else if (Players.Players[c].Locations[d] != 0) {
                    Players.Players[c].Locations[d] |= ROOM_ENTERING;
                    break;
                }
            }
        }

        Players.Players[c].CalcRoom();
    }

    UpdateSeason();

    for (c = d = 0; c < Players.Players.AnzEntries(); c++) {
        if (Players.Players[c].IsOut == 0) {
            d += Players.Players[c].Gates.NumRented;
        }
    }

    // Manchmal wird der Checkin nicht richtig gespeichert. Warum, weiß ich nicht, aber das hier korrigiert es:
    if (d - 4 + 1 > CheckIn) {
        CheckIn = d - 4 + 1;
        WaitZone = d - 4 + 1;
        goto reload_airport;
    }

    if (SaveVersionSub < 107) {
    reload_airport:
        Suitcase = WaitZone;
        Airport.LoadAirport(LeftEnd, CheckIn, Office, Entry, Shops, Cafe, Security, Suitcase, WaitZone, RightEnd);
    }

    AIRPORT::UpdateStaticDoorImage();
    Airport.CreateGateMapper();

    if (bNetwork != 0) {
        DisplayBroadcastMessage(StandardTexte.GetS(TOKEN_MISC, 7004));
    }

    bgJustDidLotsOfWork = TRUE;
    bgIsLoadingSavegame = false;

    // Postfix zum Laden von alten Spielständen in die Deluxe-Version:
    if (NumSaveGameCities != Cities.AnzEntries()) {
        //------------------------
        // Routen + RentRouten:
        //------------------------

        // Die alten Routen-Ids in einem Array merken:
        SLONG RoutenIds[MAX_ROUTES];
        for (c = 0; c < MAX_ROUTES; c++) {
            if (Routen.IsInAlbum(c) != 0) {
                RoutenIds[c] = Routen.GetIdFromIndex(c);
            } else {
                RoutenIds[c] = -1;
            }
        }

        // Die RoutenRouten der Players kopieren:
        CRentRouten tempRentRouten[4];
        for (c = 0; c < Players.Players.AnzEntries(); c++) {
            tempRentRouten[c] = Players.Players[c].RentRouten;
        }

        // Die Routen-Tabelle erweitern:
        Routen.ReInitExtend("routen.csv");

        // Die RentRouten der Players auf die Größe bringen und die neue Reihenfolge mappen
        for (c = 0; c < Players.Players.AnzEntries(); c++) {
            Players.Players[c].RentRouten.RentRouten.ReSize(0);
            Players.Players[c].RentRouten.RentRouten.ReSize(Routen.AnzEntries());

            for (d = 0; d < tempRentRouten[c].RentRouten.AnzEntries(); d++) {
                if (RoutenIds[d] != -1) {
                    Players.Players[c].RentRouten.RentRouten[Routen(RoutenIds[d])] = tempRentRouten[c].RentRouten[d];
                }
            }
        }

        //------------------------
        // RentCities:
        //------------------------

        // Die RentCities der Players auf die Größe bringen und die neue Reihenfolge mappen
        for (d = 0; d < Cities.AnzEntries(); d++) {
            if (Cities[d].bNewInAddOn == 2) {
                for (c = 0; c < Players.Players.AnzEntries(); c++) {
                    Players.Players[c].RentCities.RentCities.ReSize(Players.Players[c].RentCities.RentCities.AnzEntries() + 1);

                    for (SLONG e = Players.Players[c].RentCities.RentCities.AnzEntries() - 1; e > d; e--) {
                        Players.Players[c].RentCities.RentCities[e] = Players.Players[c].RentCities.RentCities[e - 1];
                    }

                    Players.Players[c].RentCities.RentCities[d] = CRentCity();
                }

                for (c = 0; c < 7; c++) {
                    if (TafelData.City[c].ZettelId >= d) {
                        TafelData.City[c].ZettelId++;
                    }
                }
            }
        }
    }

    return (TRUE);
}

//--------------------------------------------------------------------------------------------
// Speichert einen Spielstand
//--------------------------------------------------------------------------------------------
void SIM::SaveGame(SLONG Number, const CString &Name) const {
    const char *pNamebaseStr = nullptr;

    if (bNetwork != 0) {
        pNamebaseStr = "net%li.dat";
    } else {
        pNamebaseStr = "game%li.dat";
    }

    CString Filename = FullFilename((LPCTSTR)bprintf(pNamebaseStr, Number), SavegamePath);
    SLONG NumSaveGameCities = Cities.AnzEntries();

    SaveVersion = 1;
    SaveVersionSub = 202; // Version 1.6.1

    std::filesystem::create_directory(LPCTSTR(AppPath + SavegamePath.Left(SavegamePath.GetLength() - 3)));

    TEAKFILE OutputFile(Filename, TEAKFILE_WRITE);

    if (OutputFile.IsOpen() == 0) {
        return;
    }

    SIM::SendSimpleMessage(ATNET_WAITFORPLAYER, 0, 1, localPlayer);

    // Names des Spielstandes:
    OutputFile << Name;

    // Versionsnummer:
    OutputFile << SaveVersion << SaveVersionSub;

    {
        CString DateString = getCurrentDayString();
        if (Difficulty >= DIFF_ATFS) {
            DateString =
                bprintf((LPCTSTR)CString(StandardTexte.GetS(TOKEN_MISC, 1152)), Difficulty + 1 - DIFF_ATFS + 20,
                        (LPCTSTR)DateString, (LPCTSTR)Players.Players[localPlayer].AirlineX);
        } else if (Difficulty != DIFF_FREEGAME) {
            DateString =
                bprintf((LPCTSTR)CString(StandardTexte.GetS(TOKEN_MISC, 1152)), Difficulty + 1,
                        (LPCTSTR)DateString, (LPCTSTR)Players.Players[localPlayer].AirlineX);
        } else {
            DateString =
                bprintf((LPCTSTR)CString(StandardTexte.GetS(TOKEN_MISC, 1153)),
                        (LPCTSTR)DateString, (LPCTSTR)Players.Players[localPlayer].AirlineX);
        }

        OutputFile << DateString;
        OutputFile << UniqueGameId << bIsHost << Players.GetAnzHumanPlayers() << localPlayer;

        for (SLONG c = 0; c < 4; c++) {
            OutputFile << Players.Players[c].Owner;
        }
    }

    OutputFile << bCompatibleRoutes;

    if (SaveVersionSub >= 101) {
        OutputFile << SessionName;
        OutputFile << bAllowCheating;
    }

    OutputFile.WriteTrap(100);

    if (SaveVersionSub >= 104) {
        OutputFile << NumSaveGameCities;
    }

    // Das Spielgerüst:
    OutputFile << Sim;
    OutputFile.WriteTrap(100);

    OutputFile << Airport;
    OutputFile.WriteTrap(100);

    OutputFile << LastAnzPeopleOnScreen;
    OutputFile.WriteTrap(100);

    // Worker & Talker:
    OutputFile << Workers;
    OutputFile.WriteTrap(100);

    OutputFile << Talkers;
    OutputFile.WriteTrap(100);

    // Sonstige Daten
    OutputFile << TafelData << Routen;
    OutputFile.WriteTrap(100);

    OutputFile << LastMinuteAuftraege;
    OutputFile << ReisebueroAuftraege;
    OutputFile << UniqueGameId2;

    if (SaveVersionSub >= 100) {
        OutputFile << gFrachten;
    }

    for (SLONG c = 0; c < SLONG(Cities.AnzEntries()); c++) {
        OutputFile << AuslandsAuftraege[c];
        OutputFile << AuslandsRefill[c];

        OutputFile << AuslandsFrachten[c];
        OutputFile << AuslandsFRefill[c];
    }

    OutputFile.WriteTrap(100);

    OutputFile << MouseWait;

    if (bNetwork != 0) {
        SIM::SendSimpleMessage(ATNET_WAITFORPLAYER, 0, -1, localPlayer);

        DisplayBroadcastMessage(StandardTexte.GetS(TOKEN_MISC, 7003));
    }

    bgJustDidLotsOfWork = TRUE;
}

//--------------------------------------------------------------------------------------------
// Gibt den Localplayer eines Spielstandes zurück
//--------------------------------------------------------------------------------------------
SLONG SIM::GetSavegameLocalPlayer(SLONG Index) const {
    CString Filename;

    const char *pNamebaseStr = nullptr;

    if (bNetwork != 0) {
        pNamebaseStr = "net%li.dat";
    } else {
        pNamebaseStr = "game%li.dat";
    }

    Filename = FullFilename((LPCTSTR)bprintf(pNamebaseStr, Index), SavegamePath);

    if (DoesFileExist(Filename) != 0) {
        SLONG SaveVersion = 0;
        SLONG SaveVersionSub = 0;
        CString DummyStr;
        TEAKFILE InputFile(Filename, TEAKFILE_READ);

        InputFile >> DummyStr;
        InputFile >> SaveVersion >> SaveVersionSub;

        if (SaveVersionSub >= 100) {
            DWORD LocalPlayer = 0;
            BOOL BoolDummy = 0;
            BOOL SlongDummy = 0;

            // Info-String ignorieren
            InputFile >> DummyStr;
            InputFile >> SlongDummy >> BoolDummy >> SlongDummy;
            InputFile >> LocalPlayer;

            return (LocalPlayer);
        }
        return (-1);
    }
    return (-1);
}

//--------------------------------------------------------------------------------------------
// Gibt den UniqueGameId eines Savegames zurück:
//--------------------------------------------------------------------------------------------
DWORD SIM::GetSavegameUniqueGameId(SLONG Index, bool bForceNetwork) const {
    CString Filename;

    const char *pNamebaseStr = nullptr;

    if ((bNetwork != 0) || bForceNetwork) {
        pNamebaseStr = "net%li.dat";
    } else {
        pNamebaseStr = "game%li.dat";
    }

    Filename = FullFilename((LPCTSTR)bprintf(pNamebaseStr, Index), SavegamePath);

    if (DoesFileExist(Filename) != 0) {
        SLONG SaveVersion = 0;
        SLONG SaveVersionSub = 0;
        CString DummyStr;
        TEAKFILE InputFile(Filename, TEAKFILE_READ);

        InputFile >> DummyStr;
        InputFile >> SaveVersion >> SaveVersionSub;

        if (SaveVersionSub >= 100) {
            DWORD UniqueGameId = 0;

            // Info-String ignorieren
            InputFile >> DummyStr;
            InputFile >> UniqueGameId;

            return (UniqueGameId);
        }
        return static_cast<DWORD>(false);
    }
    return static_cast<DWORD>(false);
}

//--------------------------------------------------------------------------------------------
// Returns the number of human players in a network game:
//--------------------------------------------------------------------------------------------
SLONG SIM::GetSavegameNumHumans(SLONG Index) const {
    CString Filename;

    const char *pNamebaseStr = nullptr;

    if (bNetwork != 0) {
        pNamebaseStr = "net%li.dat";
    } else {
        pNamebaseStr = "game%li.dat";
    }

    Filename = FullFilename((LPCTSTR)bprintf(pNamebaseStr, Index), SavegamePath);

    if (DoesFileExist(Filename) != 0) {
        SLONG SaveVersion = 0;
        SLONG SaveVersionSub = 0;
        CString DummyStr;
        TEAKFILE InputFile(Filename, TEAKFILE_READ);

        InputFile >> DummyStr;
        InputFile >> SaveVersion >> SaveVersionSub;

        if (SaveVersionSub >= 100) {
            SLONG nHumans = 0;
            SLONG DummySlong = 0;
            SLONG DummyBool = 0;

            // Info-String ignorieren
            InputFile >> DummyStr;
            InputFile >> DummySlong >> DummyBool;
            InputFile >> nHumans;

            return (nHumans);
        }
        return (0);
    }
    return (0);
}

//--------------------------------------------------------------------------------------------
// Reads the Sessionname from a savegame:
//--------------------------------------------------------------------------------------------
CString SIM::GetSavegameSessionName(SLONG Index) {
    CString Filename;

    const char *pNamebaseStr = nullptr;

    if (bNetwork != 0) {
        pNamebaseStr = "net%li.dat";
    } else {
        pNamebaseStr = "game%li.dat";
    }

    Filename = FullFilename((LPCTSTR)bprintf(pNamebaseStr, Index), SavegamePath);

    if (DoesFileExist(Filename) != 0) {
        SLONG SaveVersion = 0;
        SLONG SaveVersionSub = 0;
        CString DummyStr;
        TEAKFILE InputFile(Filename, TEAKFILE_READ);

        InputFile >> DummyStr;
        InputFile >> SaveVersion >> SaveVersionSub;

        if (SaveVersionSub >= 100) {
            BOOL BoolDummy = 0;
            SLONG SlongDummy = 0;
            UBYTE ByteDummy = 0;

            // Info-String ignorieren
            InputFile >> DummyStr;
            InputFile >> SlongDummy >> BoolDummy >> SlongDummy >> SlongDummy;

            InputFile >> ByteDummy >> ByteDummy >> ByteDummy >> ByteDummy;
            InputFile >> BoolDummy;
        }
        if (SaveVersionSub >= 101) {
            InputFile >> SessionName;
        } else {
            SessionName = bprintf("%-25s", StandardTexte.GetS(TOKEN_NEWGAME, 703));
            ;
        }
    }

    return (SessionName);
}

//--------------------------------------------------------------------------------------------
// Reads the owners for the players from the savegames:
//--------------------------------------------------------------------------------------------
void SIM::ReadSavegameOwners(SLONG Index) {
    CString Filename;

    const char *pNamebaseStr = nullptr;

    if (bNetwork != 0) {
        pNamebaseStr = "net%li.dat";
    } else {
        pNamebaseStr = "game%li.dat";
    }

    Filename = FullFilename((LPCTSTR)bprintf(pNamebaseStr, Index), SavegamePath);

    if (DoesFileExist(Filename) != 0) {
        SLONG SaveVersion = 0;
        SLONG SaveVersionSub = 0;
        CString DummyStr;
        TEAKFILE InputFile(Filename, TEAKFILE_READ);

        InputFile >> DummyStr;
        InputFile >> SaveVersion >> SaveVersionSub;

        if (SaveVersionSub >= 100) {
            BOOL BoolDummy = 0;
            BOOL SlongDummy = 0;

            // Info-String ignorieren
            InputFile >> DummyStr;
            InputFile >> SlongDummy >> BoolDummy >> SlongDummy >> SlongDummy;

            for (SLONG c = 0; c < 4; c++) {
                InputFile >> Players.Players[c].Owner;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Lädt die Optionen aus der Registry:
//--------------------------------------------------------------------------------------------
void SIM::LoadOptions() {
    Options.ReadOptions();
    Airport.RepaintTextBricks();
}

//--------------------------------------------------------------------------------------------
// Speichert die Optionen in der Registry:
//--------------------------------------------------------------------------------------------
void SIM::SaveOptions() { Options.WriteOptions(); }

//--------------------------------------------------------------------------------------------
// Bringt den Benutzungplan der Räume auf den neusten Stand:
//--------------------------------------------------------------------------------------------
void SIM::UpdateRoomUsage() {
    SLONG c = 0;
    SLONG d = 0;

    RoomBusy[ROOM_SHOP1] = 0;
    RoomBusy[ROOM_SHOP2] = 0;
    RoomBusy[ROOM_SHOP3] = 0;
    RoomBusy[ROOM_SHOP4] = 0;
    RoomBusy[ROOM_RICKS] = 0;
    RoomBusy[ROOM_ROUTEBOX] = 0;
    RoomBusy[ROOM_BANK] = 0;
    RoomBusy[ROOM_KIOSK] = 0;
    RoomBusy[ROOM_MUSEUM] = 0;
    RoomBusy[ROOM_LAST_MINUTE] = 0;
    RoomBusy[ROOM_ARAB_AIR] = 0;
    RoomBusy[ROOM_MAKLER] = 0;
    RoomBusy[ROOM_AUFSICHT] = 0;
    RoomBusy[ROOM_TAFEL] = 0;
    RoomBusy[ROOM_WERBUNG] = 0;
    RoomBusy[ROOM_WERKSTATT] = 0;
    RoomBusy[ROOM_REISEBUERO] = 0;
    RoomBusy[ROOM_SABOTAGE] = 0;
    RoomBusy[ROOM_NASA] = 0;
    RoomBusy[ROOM_INSEL] = 0;
    RoomBusy[ROOM_WORLD] = 0;
    RoomBusy[ROOM_RUSHMORE] = 0;
    RoomBusy[ROOM_PLANEPROPS] = 0;
    RoomBusy[ROOM_FRACHT] = 0;
    RoomBusy[ROOM_WELTALL] = 0;
    RoomBusy[ROOM_SECURITY] = 0;
    RoomBusy[ROOM_EDITOR] = 0;
    RoomBusy[ROOM_DESIGNER] = 0;

    for (c = 0; c < 3; c++) {
        Talkers.Talkers[TALKER_BOSS].DecreaseLocking();
        Talkers.Talkers[TALKER_ARAB].DecreaseLocking();
        Talkers.Talkers[TALKER_SABOTAGE].DecreaseLocking();
        Talkers.Talkers[TALKER_BANKER1].DecreaseLocking();
        Talkers.Talkers[TALKER_BANKER2].DecreaseLocking();
        Talkers.Talkers[TALKER_MUSEUM].DecreaseLocking();
        Talkers.Talkers[TALKER_MAKLER].DecreaseLocking();
        Talkers.Talkers[TALKER_MECHANIKER].DecreaseLocking();
        Talkers.Talkers[TALKER_WERBUNG].DecreaseLocking();
        Talkers.Talkers[TALKER_SECURITY].DecreaseLocking();
        Talkers.Talkers[TALKER_DESIGNER].DecreaseLocking();
    }

    for (c = 0; c < 4; c++) {
        PLAYER &qPlayer = Players.Players[c];

        if (qPlayer.IsOut == 0) {
            for (d = 0; d < 10; d++) {
                SLONG loc = (qPlayer.Locations[d] & 255);

                if (loc > ROOM_PERSONAL_D && loc < ROOM_FLIGHTDISPLAY && loc != ROOM_STATISTICS && loc != ROOM_WC_F) {
                    RoomBusy[loc]++;

                    switch (loc) {
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
                    default:
                        break;
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Send a message to all players: please refill:
// Type: 1 - LastMinute
// Type: 2 - Reisebüro
// Type: 3 - Fracht
// Type: 4 - Ausland, City = CityIndex
//--------------------------------------------------------------------------------------------
void SIM::NetRefill(SLONG Type, SLONG City) const {
    TEAKFILE Message;

    Message.Announce(128);

    Message << ATNET_PLAYER_REFILL << Type << City;

    SLONG Delta = 0;
    SLONG Time = 0;

    switch (Type) {
    case 1:
        Delta = TickLastMinuteRefill;
        break;
    case 2:
        Delta = TickReisebueroRefill;
        break;
    case 3:
        Delta = TickFrachtRefill;
        break;
    case 4:
        Delta = AuslandsRefill[City];
        break;
    case 5:
        break;
    default:
        hprintf("Sim.cpp: Default case should not be reached.");
        DebugBreak();
    }

    Time = Time - Delta;

    Message << Delta << Time;

    SIM::SendMemFile(Message);
}

//--------------------------------------------------------------------------------------------
// Ein Spieler will einen anderen übernehmen:
//--------------------------------------------------------------------------------------------
void SIM::NetSynchronizeOvertake() const { SIM::SendSimpleMessage(ATNET_OVERTAKE, 0, OvertakenAirline, OvertakerAirline, Overtake); }

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
void SIM::AddHighscore(const CString &Name, DWORD UniqueGameId2, __int64 Score) {
    SLONG c = 0;
    SLONG d = 0;

    // Pass 1: Einen existierenden Eintrag der gleichen Session überschreiben?
    for (c = 0; c < 6; c++) {
        if (UniqueGameId2 == Highscores[c].UniqueGameId2) {
            if (Score > Highscores[c].Score) {
                Highscores[c].Score = Score;
                Highscores[c].Name = Name;

                // Erneut sortieren:
                for (c = 0; c < 6 - 1; c++) {
                    if (Highscores[c + 1].Score > Highscores[c].Score) {
                        CHighscore tmp = Highscores[c + 1];
                        Highscores[c + 1] = Highscores[c];
                        Highscores[c] = tmp;
                        c -= 2;
                        if (c < -1) {
                            c = -1;
                        }
                    }
                }

                SaveHighscores();
            }

            return;
        }
    }

    // Pass 2: Eine neue Session hinzufügen:
    for (c = 0; c < 6; c++) {
        if (Score > Highscores[c].Score) {
            for (d = 4; d >= c; d--) {
                Highscores[d + 1] = Highscores[d];
            }

            Highscores[c].Score = Score;
            Highscores[c].UniqueGameId2 = UniqueGameId2;
            Highscores[c].Name = Name;
            SaveHighscores();

            return;
        }
    }
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
void SIM::SaveHighscores() {
#ifdef _DEBUG
    return; // Debug exit for better single installation multiplayer testing
#endif
    CString str;
    auto path = FullFilename("xmlmap.fla", MiscPath);
    try {
        TEAKFILE OutputFile(path, TEAKFILE_WRITE);

        for (auto &Highscore : Highscores) {
            OutputFile.Write(reinterpret_cast<const UBYTE *>((LPCTSTR)Highscore.Name), Highscore.Name.GetLength());
            OutputFile.Write(reinterpret_cast<const UBYTE *>(";"), 1);

            str = bprintf("%lu;", Highscore.UniqueGameId2);
            OutputFile.Write(reinterpret_cast<const UBYTE *>((LPCTSTR)str), str.GetLength());

            __int64 k1 = rand() % 256 + rand() % 256 * 256 + rand() % 256 * 65536 + rand() % 256 * 65536 * 256;
            __int64 k2 = rand() % 256 + rand() % 256 * 256 + rand() % 256 * 65536 + rand() % 256 * 65536 * 256;
            __int64 k3 = rand() % 256 + rand() % 256 * 256 + rand() % 256 * 65536 + rand() % 256 * 65536 * 256;
            __int64 k4 = 0;
            __int64 k5 = 0;

            k4 = k5 = Highscore.Score;
            k4 ^= (k1 ^ k3);
            k5 ^= k2;

            if (Highscore.Score == 0) {
                k1 = k2 = k3 = k4 = k5 = 0;
            }

            str = bprintf("%I64i;", k1);
            OutputFile.Write(reinterpret_cast<const UBYTE *>((LPCTSTR)str), str.GetLength());

            str = bprintf("%I64i;", k2);
            OutputFile.Write(reinterpret_cast<const UBYTE *>((LPCTSTR)str), str.GetLength());

            str = bprintf("%I64i;", k3);
            OutputFile.Write(reinterpret_cast<const UBYTE *>((LPCTSTR)str), str.GetLength());

            str = bprintf("%I64i;", k4);
            OutputFile.Write(reinterpret_cast<const UBYTE *>((LPCTSTR)str), str.GetLength());

            str = bprintf("%I64i\xd\xa", k5);
            OutputFile.Write(reinterpret_cast<const UBYTE *>((LPCTSTR)str), str.GetLength());
        }
    } catch (TeakLibException &e) {
        AT_Log("Failed to save highscores to file - reason: %s", e.what());
        TeakLibW_Exception(FNL, "Failed to save highscores to file '%s'", path.c_str());
    }
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
void SIM::LoadHighscores() {
    try {
        auto path = FullFilename("xmlmap.fla", MiscPath);
        if (DoesFileExist(path) != 0) {
            char Buffer[8192];
            TEAKFILE OutputFile(path, TEAKFILE_READ);

            for (auto &Highscore : Highscores) {
                OutputFile.ReadLine(Buffer, 8192);

                if (*Buffer == ';') {
                    snprintf(Buffer, sizeof(Buffer), "%s", bprintf(" %s", Buffer));
                }

                Highscore.Name = strtok(Buffer, ";");
                Highscore.UniqueGameId2 = atoi(strtok(nullptr, ";"));

                int64_t k1 = atoll(strtok(nullptr, ";"));
                int64_t k2 = atoll(strtok(nullptr, ";"));
                int64_t k3 = atoll(strtok(nullptr, ";"));
                int64_t k4 = atoll(strtok(nullptr, ";"));
                int64_t k5 = atoll(strtok(nullptr, ";"));

                if ((k4 ^ k1 ^ k3) == (k5 ^ k2)) {
                    Highscore.Score = k5 ^ k2;
                } else {
                    Highscore.Score = 0;
                }
            }
        }
    } catch (...) {
    }
}

SLONG SIM::HoleKerosinPreis(SLONG typ) const {
    switch (typ) {
    case 0:
        return Kerosin * 2;
    case 1:
        return Kerosin;
    case 2:
        return Kerosin / 2;
    default:
        hprintf("Sim.cpp: Default case should not be reached.");
        DebugBreak();
    }
    return 0;
}

//--------------------------------------------------------------------------------------------
// Liest die Optionen aus der Registry:
//--------------------------------------------------------------------------------------------
void COptions::ReadOptions() {
    SLONG tmp = Sim.MaxDifficulty;
    SLONG tmp2 = Sim.MaxDifficulty2;
    SLONG tmp3 = Sim.MaxDifficulty3;
    SLONG c = 0;
    char Buffer[100];

    CRegistryAccess reg(chRegKey);

    // New Settings:
    if (!reg.ReadRegistryKey_l(OptionFullscreen)) {
        OptionFullscreen = 0;
    }
    if (!reg.ReadRegistryKey_b(OptionKeepAspectRatio)) {
        OptionKeepAspectRatio = 1;
    }

    // Falls Setup nicht geladen wurde dann Standard-Parameter initialisieren
    if (!reg.ReadRegistryKey_b(OptionPlanes)) {
        OptionPlanes = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionPassengers)) {
        OptionPassengers = TRUE;
    }
    if (!reg.ReadRegistryKey_l(OptionMusicType)) {
        OptionMusicType = 2;
    }
    if (!reg.ReadRegistryKey_b(OptionEnableDigi)) {
        OptionEnableDigi = TRUE;
    }
    if (!reg.ReadRegistryKey_l(OptionMusik)) {
        OptionMusik = 3;
    }
    if (!reg.ReadRegistryKey_l(OptionMasterVolume)) {
        OptionMasterVolume = 7;
    }
    if (!reg.ReadRegistryKey_l(OptionLoopMusik)) {
        OptionLoopMusik = 0;
    }
    if (!reg.ReadRegistryKey_l(OptionAmbiente)) {
        OptionAmbiente = 3;
    }
    if (!reg.ReadRegistryKey_b(OptionRealKuerzel)) {
        OptionRealKuerzel = TRUE;
    }
    if (!reg.ReadRegistryKey_l(OptionDurchsagen)) {
        OptionDurchsagen = 3;
    }
    if (!reg.ReadRegistryKey_l(OptionTalking)) {
        OptionTalking = 7;
    }
    if (!reg.ReadRegistryKey_l(OptionPlaneVolume)) {
        OptionPlaneVolume = 3;
    }
    if (!reg.ReadRegistryKey_l(OptionEffekte)) {
        OptionEffekte = 3;
    }
    if (!reg.ReadRegistryKey_b(OptionGirl)) {
        OptionGirl = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionBerater)) {
        OptionBerater = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionBriefBriefing)) {
        OptionBriefBriefing = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionBlenden)) {
        OptionBlenden = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionTransparenz)) {
        OptionTransparenz = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionSchatten)) {
        OptionSchatten = TRUE;
    }
    if (!reg.ReadRegistryKey_l(OptionAirport)) {
        OptionAirport = -1;
    }
    if (!reg.ReadRegistryKey_b(OptionThinkBubbles)) {
        OptionThinkBubbles = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionFlipping)) {
        OptionFlipping = FALSE;
    }
    if (!reg.ReadRegistryKey_l(Sim.MaxDifficulty)) {
        Sim.MaxDifficulty = 0;
    }
    if (!reg.ReadRegistryKey_l(Sim.MaxDifficulty2)) {
        Sim.MaxDifficulty2 = 11;
    }
    if (!reg.ReadRegistryKey_l(Sim.MaxDifficulty3)) {
        Sim.MaxDifficulty3 = 41;
    }
    if (!reg.ReadRegistryKey_b(OptionAutosave)) {
        OptionAutosave = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionFax)) {
        OptionFax = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionRoundNumber)) {
        OptionRoundNumber = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionRandomStartday)) {
        OptionRandomStartday = TRUE;
    }
    if (!reg.ReadRegistryKey_b(OptionViewedIntro)) {
        OptionViewedIntro = FALSE;
    }
    if (!reg.ReadRegistryKey_b(OptionSpeechBubble)) {
        if (gLanguage == LANGUAGE_O) {
            OptionSpeechBubble = TRUE;
        } else {
            OptionSpeechBubble = TRUE;
        }
    }

    std::strcpy(Buffer, "master.open-airlinetycoon.com");
    if (!reg.ReadRegistryKeyEx(Buffer, "OptionMasterServer")) {
        OptionMasterServer = "master.open-airlinetycoon.com"; // default server
    } else {
        OptionMasterServer = Buffer;
    }
    std::strcpy(Buffer, "\0");

    if (OptionMasterVolume == 0) {
        OptionMasterVolume = 7;
    }

#ifdef NO_D_VOICES
    if (gLanguage == LANGUAGE_D) {
        OptionSpeechBubble = TRUE;
        OptionTalking = FALSE;
    }
#endif
#ifdef NO_E_VOICES
    if (gLanguage == LANGUAGE_E &&) {
        OptionSpeechBubble = TRUE;
        OptionTalking = FALSE;
    }
#endif
#ifdef NO_N_VOICES
    if (gLanguage == LANGUAGE_N) {
        OptionSpeechBubble = TRUE;
        OptionTalking = FALSE;
    }
#endif

    if (bVoicesNotFound) {
        OptionSpeechBubble = TRUE;
        OptionTalking = FALSE;
    }

#ifdef NO_INTRO
    OptionViewedIntro = TRUE;
#endif

    if (!reg.ReadRegistryKey_l(OptionRoomDescription)) {
        OptionRoomDescription = 0;
    }
    if (!reg.ReadRegistryKey_l(OptionLastPlayer)) {
        OptionLastPlayer = 0;
    }
    if (!reg.ReadRegistryKey_l(OptionLastMission)) {
        OptionLastMission = 0;
    }
    if (!reg.ReadRegistryKey_l(OptionLastMission2)) {
        OptionLastMission2 = 11;
    }
    if (!reg.ReadRegistryKey_l(OptionLastMission3)) {
        OptionLastMission3 = 41;
    }
    if (!reg.ReadRegistryKey_l(OptionLastProvider)) {
        OptionLastProvider = 0;
    }

    if (!reg.ReadRegistryKeyEx_u(Sim.KeyHints[0], "KeyHints1")) {
        Sim.KeyHints[0] = 0;
    }
    if (!reg.ReadRegistryKeyEx_u(Sim.KeyHints[2], "KeyHints2")) {
        Sim.KeyHints[2] = 0;
    }
    Sim.KeyHints[1] = 0;

    // Namen der Spieler
    for (c = 0; c < 4; c++) {
        if (!reg.ReadRegistryKeyEx(Buffer, bprintf("Player%li", c))) {
            OptionPlayerNames[c] = "";
        } else {
            OptionPlayerNames[c] = Buffer;
        }

        OptionAirlineNames[c] = "";
        OptionAirlineAbk[c] = "";
    }

    // Schwierigkeitsgrad decodieren:
    for (c = 0; c < 6; c++) {
        if (MissionKeys[c] == Sim.MaxDifficulty) {
            Sim.MaxDifficulty = c;
            break;
        }
    }
    if (c == 6) {
        Sim.MaxDifficulty = 0;
    }

    // Schwierigkeitsgrad für AddOn decodieren:
    {
        for (c = 0; c < 10; c++) {
            if (MissionKeys2[c] == Sim.MaxDifficulty2) {
                Sim.MaxDifficulty2 = c + DIFF_ADDON01;
                break;
            }
        }
        if (c == 10) {
            Sim.MaxDifficulty2 = DIFF_ADDON01;
        }
    }

    // Schwierigkeitsgrad für AddOn Flight Security decodieren:
    {
        for (c = 0; c < 10; c++) {
            if (MissionKeys3[c] == Sim.MaxDifficulty3) {
                Sim.MaxDifficulty3 = c + DIFF_ATFS01;
                break;
            }
        }
        if (c == 10) {
            Sim.MaxDifficulty3 = DIFF_ATFS01;
        }
    }

    Sim.MaxDifficulty = max(tmp, Sim.MaxDifficulty);
    Sim.MaxDifficulty2 = max(tmp2, Sim.MaxDifficulty2);
    Sim.MaxDifficulty3 = max(tmp3, Sim.MaxDifficulty3);

    OptionLastMission = min(OptionLastMission, Sim.MaxDifficulty);
    OptionLastMission2 = min(OptionLastMission2, Sim.MaxDifficulty2);
    OptionLastMission3 = min(OptionLastMission3, Sim.MaxDifficulty3);
}

//--------------------------------------------------------------------------------------------
// Schreibt die Optionen in die Registry:
//--------------------------------------------------------------------------------------------
void COptions::WriteOptions() {
    SLONG tmp = Sim.MaxDifficulty;
    SLONG tmp2 = Sim.MaxDifficulty2;
    SLONG tmp3 = Sim.MaxDifficulty3;
    SLONG c = 0;

    Sim.MaxDifficulty = MissionKeys[Sim.MaxDifficulty];
    Sim.MaxDifficulty2 = MissionKeys2[Sim.MaxDifficulty2 - DIFF_ADDON01];
    Sim.MaxDifficulty3 = MissionKeys3[Sim.MaxDifficulty3 - DIFF_ATFS01];

    CRegistryAccess reg(chRegKey);

    reg.WriteRegistryKey_l(OptionFullscreen);
    reg.WriteRegistryKey_b(OptionKeepAspectRatio);
    reg.WriteRegistryKey_b(OptionPlanes);
    reg.WriteRegistryKey_b(OptionPassengers);
    reg.WriteRegistryKey_l(OptionMusicType);
    reg.WriteRegistryKey_b(OptionEnableDigi);
    reg.WriteRegistryKey_l(OptionMusik);
    reg.WriteRegistryKey_l(OptionLoopMusik);
    reg.WriteRegistryKey_l(OptionAmbiente);
    reg.WriteRegistryKey_b(OptionRealKuerzel);
    reg.WriteRegistryKey_l(OptionDurchsagen);
    reg.WriteRegistryKey_l(OptionPlaneVolume);
    reg.WriteRegistryKey_l(OptionEffekte);
    reg.WriteRegistryKey_b(OptionGirl);
    reg.WriteRegistryKey_b(OptionBerater);
    reg.WriteRegistryKey_b(OptionBriefBriefing);
    reg.WriteRegistryKey_b(OptionBlenden);
    reg.WriteRegistryKey_b(OptionTransparenz);
    reg.WriteRegistryKey_b(OptionSchatten);
    reg.WriteRegistryKey_l(OptionAirport);
    reg.WriteRegistryKey_b(OptionThinkBubbles);
    reg.WriteRegistryKey_b(OptionFlipping);
    reg.WriteRegistryKey_l(Sim.MaxDifficulty);
    reg.WriteRegistryKey_l(Sim.MaxDifficulty2);
    reg.WriteRegistryKey_l(Sim.MaxDifficulty3);
    reg.WriteRegistryKey_b(OptionAutosave);
    reg.WriteRegistryKey_b(OptionFax);
    reg.WriteRegistryKey_b(OptionRoundNumber);

    if ((gpSSE != nullptr) && gpSSE->IsSoundEnabled()) {
        reg.WriteRegistryKey_l(OptionMasterVolume);
    }

    // Den Intro-Eintrag nur schreiben, wenn wir keine Demo haben. Sonst schaut sich jemand eine
    // Demo ohne Intro an. Und wenn er dann das richtige Spiel kauft gäbe es keinen Intro.
#ifndef NO_INTRO
    reg.WriteRegistryKey_b(OptionViewedIntro);
#endif

    // Und fast das gleiche für die Sprechblasen
#ifdef NO_D_VOICES
    if (gLanguage == LANGUAGE_D)
        goto dont_save_talking;
#endif
#ifdef NO_E_VOICES
    if (gLanguage == LANGUAGE_E)
        goto dont_save_talking;
#endif
#ifdef NO_N_VOICES
    if (gLanguage == LANGUAGE_N)
        goto dont_save_talking;
#endif
    reg.WriteRegistryKey_l(OptionTalking);
    reg.WriteRegistryKey_b(OptionSpeechBubble);
    reg.WriteRegistryKey_b(OptionRandomStartday);

#ifdef NO_D_VOICES
dont_save_talking:
#endif
#ifdef NO_E_VOICES
dont_save_talking:
#endif
#ifdef NO_N_VOICES
dont_save_talking:
#endif

    reg.WriteRegistryKey_l(OptionRoomDescription);
    reg.WriteRegistryKeyEx(const_cast<char *>((LPCTSTR)AppPath), "&AppPath");
    reg.WriteRegistryKey_l(OptionLastPlayer);

    reg.WriteRegistryKeyEx_u(Sim.KeyHints[0], "KeyHints1");
    reg.WriteRegistryKeyEx_u(Sim.KeyHints[2], "KeyHints2");

    if (OptionLastMission != DIFF_FREEGAME) {
        reg.WriteRegistryKey_l(OptionLastMission);
    }

    reg.WriteRegistryKey_l(OptionLastMission3);
    reg.WriteRegistryKey_l(OptionLastMission2);
    reg.WriteRegistryKey_l(OptionLastProvider);
    reg.WriteRegistryKeyEx(OptionMasterServer.c_str(), "OptionMasterServer");

    // Namen der Spieler
    for (c = 0; c < 4; c++) {
        reg.WriteRegistryKeyEx(OptionPlayerNames[c], bprintf("Player%li", c));
    }

    Sim.MaxDifficulty = tmp;
    Sim.MaxDifficulty2 = tmp2;
    Sim.MaxDifficulty3 = tmp3;
}

//--------------------------------------------------------------------------------------------
// SValue::
//--------------------------------------------------------------------------------------------
// Konstruktor:
//--------------------------------------------------------------------------------------------
SValue::SValue() { Init(); }

//--------------------------------------------------------------------------------------------
// Init:
//--------------------------------------------------------------------------------------------
void SValue::Init() {
    Days.ReSize(1);
    Days.FillWith(0);
}

//--------------------------------------------------------------------------------------------
// Verwaltung für den neuen Tag:
//--------------------------------------------------------------------------------------------
void SValue::NewDay() {
    Days.ReSize(Days.AnzEntries() + 1);
    Days.back() = *(Days.rbegin() + 1);
}

//--------------------------------------------------------------------------------------------
// Gibt die Summe aus dem letzten Monat aus:
//--------------------------------------------------------------------------------------------
__int64 SValue::GetSum() { return std::accumulate(Days.begin(), Days.end(), 0); }


TEAKFILE &operator<<(TEAKFILE &File, const SValue &Value) {
    File << Value.Days;
    return (File);
}

TEAKFILE & operator>>(TEAKFILE &File, SValue &Value) {
    File >> Value.Days;

    if (SaveVersionSub < 200) {
        BUFFER_V<__int64> _months;
        File >> _months;
    }

    return (File);
}
