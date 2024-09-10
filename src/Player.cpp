//============================================================================================
// Player.cpp : Routinen zum verwalten der Spieler
//============================================================================================
#include "StdAfx.h"
#include "AtNet.h"
#define forall(c, object) for ((c) = 0; (c) < SLONG((object).AnzEntries()); (c)++)

#define AT_Error(...) Hdu.HercPrintfMsg(SDL_LOG_PRIORITY_ERROR, "Player", __VA_ARGS__)
#define AT_Warn(...) Hdu.HercPrintfMsg(SDL_LOG_PRIORITY_WARN, "Player", __VA_ARGS__)
#define AT_Info(...) Hdu.HercPrintfMsg(SDL_LOG_PRIORITY_INFO, "Player", __VA_ARGS__)
#define AT_Log(...) AT_Log_I("Player", __VA_ARGS__)

extern SLONG SabotagePrice[];
extern SLONG SabotagePrice2[];
extern SLONG SabotagePrice3[];

extern SLONG RocketPrices[];
extern SLONG StationPrices[];

// Preise verstehen sich pro Sitzplatz:
extern SLONG SeatCosts[];
extern SLONG FoodCosts[];
extern SLONG TrayCosts[];
extern SLONG DecoCosts[];

// Preise pro Flugzeuge:
extern SLONG TriebwerkCosts[];
extern SLONG ReifenCosts[];
extern SLONG ElektronikCosts[];
extern SLONG SicherheitCosts[];

// Öffnungszeiten:
extern SLONG timeDutyOpen;
extern SLONG timeDutyClose;
extern SLONG timeArabOpen;
extern SLONG timeLastClose;
extern SLONG timeMuseOpen;
extern SLONG timeReisClose;
extern SLONG timeMaklClose;
extern SLONG timeWerbOpen;

// Daten des aktuellen Savegames beim laden:
extern SLONG SaveVersion;
extern SLONG SaveVersionSub;

extern SLONG CheckGeneric;

//--------------------------------------------------------------------------------------------
// abs für 64-Bit Variablen
//--------------------------------------------------------------------------------------------
__int64 abs64(__int64 v);

__int64 abs64(__int64 v) {
    if (v < 0) {
        return (-v);
    }

    return (v);
}

//============================================================================================
// PLAYER::
//============================================================================================
// Konstruktor:
//============================================================================================
PLAYER::PLAYER() {
    SLONG c = 0;

    NewDir = 8;
    Buttons = 0;
    CursorPos = XY(-1, -1);
    LocationWin = nullptr;
    DialogWin = nullptr;
    TopLocation = 0;
    WalkSpeed = 3;
    WaitForRoom = 0;
    ThrownOutOfRoom = 0;
    IsOut = FALSE;
    PlayerSmoking = 0;
    PlayerStinking = 0;

    pSmack = nullptr;
    NetworkID = 0;

    RobotActions.ReSize(5);
    Sympathie.ReSize(4);
    Kooperation.ReSize(4);
    Messages.NewDay();

    MoneyPast.ReSize(20);
    MoneyPast.FillWith(0);

    Smoke.ReSize(20);

    for (c = 0; c < 10; c++) {
        Locations[c] = 0;
    }
    for (c = 0; c < 5; c++) {
        RobotActions[c].ActionId = 0;
    }
}

//--------------------------------------------------------------------------------------------
// Destruktiv:
//--------------------------------------------------------------------------------------------
PLAYER::~PLAYER() {
    delete pSmack;
    pSmack = nullptr;
}

//--------------------------------------------------------------------------------------------
// Fügt 5 Flüge vom Uhrig hinzu:
//--------------------------------------------------------------------------------------------
void PLAYER::Add5UhrigFlights() {
    if (Auftraege.GetNumFree() < 5) {
        Auftraege.ReSize(Auftraege.AnzEntries() + 5);
    }

    for (SLONG c = 0; c < 5; c++) {
        CAuftrag a;

        a.RefillForUhrig((c + 0) / 2, &Auftraege.Random);

        Auftraege += a;
    }
}

//--------------------------------------------------------------------------------------------
// Der Spieler kauft ein Flugzeug:
//--------------------------------------------------------------------------------------------
void PLAYER::BuyPlane(ULONG PlaneTypeId, TEAKRAND *pRnd) {
    ULONG Id = 0;

    if (Planes.GetNumFree() == 0) {
        Planes.ReSize(Planes.AnzEntries() + 10);
    }
    Id = (Planes += CPlane(PlaneNames.GetUnused(pRnd), PlaneTypeId + 0x10000000, 100, GetCurrentYear() + (Sim.Date / 365)));

    Planes[Id].GlobeAngle = 0;
    Planes[Id].MaxBegleiter = SLONG(PlaneTypes[PlaneTypeId + 0x10000000].AnzBegleiter * Planes.GetAvgBegleiter());

    if (Planes[Id].MaxBegleiter < 0) {
        Planes[Id].MaxBegleiter = 0;
    }
    if (Planes[Id].MaxBegleiter > Planes[Id].ptAnzBegleiter * 2) {
        Planes[Id].MaxBegleiter = Planes[Id].ptAnzBegleiter * 2;
    }
    // if (Planes[Id].MaxBegleiter>PlaneTypes[Planes[Id].TypeId].AnzBegleiter*2) Planes[Id].MaxBegleiter=PlaneTypes[Planes[Id].TypeId].AnzBegleiter*2;

    ChangeMoney(-PlaneTypes[PlaneTypeId + 0x10000000].Preis,
                2010, // Kauf des Flugzeuges
                Planes[Id].Name);

    Planes.Sort();
}

//--------------------------------------------------------------------------------------------
// Der Spieler kauft ein Flugzeug:
//--------------------------------------------------------------------------------------------
void PLAYER::BuyPlane(CXPlane &plane, TEAKRAND *pRnd) {
    ULONG Id = 0;

    if (Planes.GetNumFree() == 0) {
        Planes.ReSize(Planes.AnzEntries() + 10);
    }
    Id = (Planes += CPlane(PlaneNames.GetUnused(pRnd), -1, 100, GetCurrentYear() + (Sim.Date / 365)));

    CPlane &p = Planes[Id];

    p.MaxPassagiere = plane.CalcPassagiere() * 6 / 8;
    p.MaxPassagiereFC = plane.CalcPassagiere() * 1 / 8;
    p.MaxBegleiter = plane.CalcBegleiter();

    p.ptHersteller = "";
    p.ptErstbaujahr = 2002;
    p.ptName = plane.Name;
    p.ptReichweite = plane.CalcReichweite();
    p.ptGeschwindigkeit = plane.CalcSpeed();
    p.ptPassagiere = plane.CalcPassagiere();
    p.ptAnzPiloten = plane.CalcPiloten();
    p.ptAnzBegleiter = plane.CalcBegleiter();
    p.ptTankgroesse = plane.CalcTank();
    p.ptVerbrauch = plane.CalcVerbrauch();
    p.ptPreis = plane.CalcCost();
    p.ptLaerm = plane.CalcNoise();
    p.ptWartungsfaktor = FLOAT((plane.CalcWartung() + 100.0) / 100.0);
    p.ptKommentar = "";

    p.GlobeAngle = 0;
    p.MaxBegleiter = SLONG(plane.CalcBegleiter() * Planes.GetAvgBegleiter());

    if (p.MaxBegleiter < 0) {
        p.MaxBegleiter = 0;
    }
    if (p.MaxBegleiter > p.ptAnzBegleiter * 2) {
        p.MaxBegleiter = p.ptAnzBegleiter * 2;
    }

    p.XPlane = plane;

    ChangeMoney(-plane.CalcCost(),
                2010, // Kauf des Flugzeuges
                p.Name);

    Planes.Sort();
}

//--------------------------------------------------------------------------------------------
// Geld mit Grund verbuchen:
//--------------------------------------------------------------------------------------------
void PLAYER::ChangeMoney(__int64 Money, SLONG Reason, const CString &Par1, char *Par2) {

    if (LocationWin != nullptr) {
        (LocationWin)->StatusCount = 3;
    }

    if (Reason < 2100 || Reason > 2103) {
        /* 2100-2103 sind nur zur Informationen (Saldo), Ein-/Ausgaben für die
         * Flüge wurden schon separat verbucht */
        if (PLAYER::Money > DEBT_WARNLIMIT1 && PLAYER::Money + Money < DEBT_WARNLIMIT1) {
            Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2380));
        }
        if (PLAYER::Money > DEBT_WARNLIMIT2 && PLAYER::Money + Money < DEBT_WARNLIMIT2) {
            Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2381));
        }
        if (PLAYER::Money > DEBT_WARNLIMIT3 && PLAYER::Money + Money < DEBT_WARNLIMIT3) {
            Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2382));
        }

        PLAYER::Money += Money;
    }

    History.AddEntry(Money, bprintf(StandardTexte.GetS(TOKEN_MONEY, Reason), (LPCTSTR)Par1, Par2));

    switch (Reason) {
    case 2000:
        /* D::Kreditzinsen */
        Statistiken[STAT_A_ZINS].AddAtPastDay(Money);
        Bilanz.SollZinsen += Money;
        break;
    case 2002:
        /* D::Überziehungszinsen */
        Statistiken[STAT_A_ZINS].AddAtPastDay(Money);
        Bilanz.SollZinsen += Money;
        break;
    case 2001:
        /* D::Guthabenszinsen */
        Statistiken[STAT_E_ZINS].AddAtPastDay(Money);
        Bilanz.HabenZinsen += Money;
        break;
    case 2003:
        /* D::Kredit Neuaufnahme */
        Statistiken[STAT_E_KREDIT].AddAtPastDay(Money);
        Bilanz.KreditNeu += Money;
        break;
    case 2004:
        /* D::Kredit Tilgung */
        Statistiken[STAT_A_KREDIT].AddAtPastDay(Money);
        Bilanz.KreditTilgung += Money;
        break;
    case 2006:
        /* D::Fundsache (Geld im Papierkorb) */
        Statistiken[STAT_E_SONSTIGES].AddAtPastDay(Money);
        Bilanz.SonstigeEinnahmen += Money;
        break;
    case 2010:
        /* D::Kauf des Flugzeuges %s */
        Statistiken[STAT_A_FLUGZEUGE].AddAtPastDay(Money);
        Bilanz.FlugzeugKauf += Money;
        break;
    case 2011:
        /* D::Verkauf des Flugzeuges %s */
        Statistiken[STAT_E_FLUGZEUGE].AddAtPastDay(Money);
        Bilanz.FlugzeugVerkauf += Money;
        break;
    case 2020:
        /* D::Kauf von Kerosin auf Vorrat */
        Statistiken[STAT_A_KEROSIN].AddAtPastDay(Money);
        Bilanz.KerosinVorrat += Money;
        break;
    case 2021:
        /* D::Kauf von Kerosin für Flugzeug */
        Statistiken[STAT_A_KEROSIN].AddAtPastDay(Money);
        Bilanz.KerosinFlug += Money;
        break;
    case 2022:
        /* Kauf von Essen für Fluggäste */
        Statistiken[STAT_A_ESSEN].AddAtPastDay(Money);
        Bilanz.Essen += Money;
        break;
    case 2030:
        /* D::Ticketeinnahmen */
        Statistiken[STAT_E_ROUTEN].AddAtPastDay(Money);
        Bilanz.Tickets += Money;
        break;
    case 2040:
        /* Ersteigerung einer Niederlassung in %s */
        Statistiken[STAT_A_EXPANSION].AddAtPastDay(Money);
        Bilanz.ExpansionCity += Money;
        break;
    case 2041:
        /* Ersteigerung der Route %s */
        Statistiken[STAT_A_EXPANSION].AddAtPastDay(Money);
        Bilanz.ExpansionRouten += Money;
        break;
    case 2042:
        /* Ersteigerung eines Gates */
        Statistiken[STAT_A_EXPANSION].AddAtPastDay(Money);
        Bilanz.ExpansionGates += Money;
        break;
    case 2050:
        /* D::Miete für Gates im Heimatflughäfen */
        Statistiken[STAT_A_MIETEN].AddAtPastDay(Money);
        Bilanz.Gatemiete += Money;
        break;
    case 2051:
        /* D::Miete für Niederlassungen */
        Statistiken[STAT_A_MIETEN].AddAtPastDay(Money);
        Bilanz.Citymiete += Money;
        break;
    case 2052:
        /* D::Miete für Routen */
        Statistiken[STAT_A_MIETEN].AddAtPastDay(Money);
        Bilanz.Routenmiete += Money;
        break;
    case 2060:
        /* D::Strafe für Auftrag %s */
        Statistiken[STAT_A_STRAFEN].AddAtPastDay(Money);
        Bilanz.Vertragsstrafen += Money;

        // Für die Gewinn-Mission:
        Gewinn += Money;
        break;
    case 2061:
        /* D::Prämie für Auftrag %s */
        Statistiken[STAT_E_AUFTRAEGE].AddAtPastDay(Money);
        Bilanz.Auftraege += Money;
        NumAuftraege++;
        break;
    case 2065:
        /* D::Strafe für Frachtauftrag %s */
        Statistiken[STAT_A_STRAFEN].AddAtPastDay(Money);
        Bilanz.Vertragsstrafen += Money;

        // Für die Gewinn-Mission:
        Gewinn += Money;
        break;
    case 2066:
        /* D::Prämie für Frachtauftrag %s */
        Statistiken[STAT_E_FRACHT].AddAtPastDay(Money);
        Bilanz.FrachtAuftraege += Money;
        break;
    case 2070:
        /* D::Löhne & Gehälter */
        Statistiken[STAT_A_GEHAELTER].AddAtPastDay(Money);
        Bilanz.Personal += Money;
        break;
    case 2080:
        /* D::Sabotage */
        Statistiken[STAT_A_SABOTAGE].AddAtPastDay(Money);
        Bilanz.Sabotage += Money;
        break;
    case 2090:
        /* nicht benutzt */
        /* D::Kerosin */
        DebugBreak();
        break;
    case 2091:
        /* D::Kerosintanks, %sl x %s */
        Statistiken[STAT_A_EXPANSION].AddAtPastDay(Money);
        Bilanz.ExpansionTanks += Money;
        break;
    case 2100:
        /* D::Saldo für Routenflug %s */
        Gewinn += Money;
        break;
    case 2101:
        /* D::Saldo für Auftragsflug %s */
        Gewinn += Money;
        break;
    case 2102:
        /* D::Saldo für Leerflug %s */
        Gewinn += Money;
        break;
    case 2103:
        /* D::Saldo für Frachtflug %s */
        Gewinn += Money;
        break;
    case 2110:
        /* D::Flugzeugumrüstung der %s */
        Bilanz.FlugzeugUpgrades += Money;
        if (Money < 0) {
            Statistiken[STAT_A_FLUGZEUGE].AddAtPastDay(Money);
        }
        if (Money > 0) {
            Statistiken[STAT_E_FLUGZEUGE].AddAtPastDay(Money);
        }
        break;
    case 2111:
        /* D::Umbau der Sitze der %s */
        Statistiken[STAT_A_FLUGZEUGE].AddAtPastDay(Money);
        Bilanz.FlugzeugUmbau += Money;
        break;
    case 2200:
        /* D::Strafe wegen Sabotage */
        Statistiken[STAT_A_SABOTAGE].AddAtPastDay(Money);
        Bilanz.SabotageStrafe += Money;
        break;
    case 2201:
        /* D::Entschädigung wegen Sabotage */
        Statistiken[STAT_E_SONSTIGES].AddAtPastDay(Money);
        Bilanz.SabotageKomp += Money;
        break;
    case 3110:
        /* D::Flugzeugwartung */
        Statistiken[STAT_A_WARTUNG].AddAtPastDay(Money);
        Bilanz.Wartung += Money;
        break;
    case 3120:
    case 3121:
    case 3122:
    case 3123:
    case 3124:
    case 3125:
        /* Kosten für versch. Werbe-Kampagnen */
        Statistiken[STAT_A_AGENTUREN].AddAtPastDay(Money);
        Bilanz.WerbeKosten += Money;
        break;
    case 3130:
        /* D::Bodyguard-Rabatt */
        Statistiken[STAT_E_RABATT].AddAtPastDay(Money);
        Bilanz.BodyguardRabatt += Money;
        break;
    case 3140:
        /* D::Rendite-Ausschüttung */
        Statistiken[STAT_A_AKTIEN].AddAtPastDay(Money);
        Bilanz.SollRendite += Money;
        break;
    case 3141:
        /* D::Rendite-Einnahmen */
        Statistiken[STAT_E_AKTIEN].AddAtPastDay(Money);
        Bilanz.HabenRendite += Money;
        break;
    case 3150:
        /* D::Aktien-Kauf */
        Statistiken[STAT_A_AKTIEN].AddAtPastDay(Money);
        Bilanz.Aktienkauf += Money;
        break;
    case 3151:
        /* D::Aktien-Verkauf */
        Statistiken[STAT_E_AKTIEN].AddAtPastDay(Money);
        Bilanz.Aktienverkauf += Money;
        break;
    case 3160:
        /* D::Emissions-Gebühr */
        Statistiken[STAT_A_AKTIEN].AddAtPastDay(Money);
        Bilanz.AktienEmissionFee += Money;
        break;
    case 3161:
        /* D::Emissions-Entschädigung (gezahlt) */
        Statistiken[STAT_A_AKTIEN].AddAtPastDay(Money);
        Bilanz.AktienEmissionKompGez += Money;
        break;
    case 3162:
        /* D::Emissions-Einnahmen */
        Statistiken[STAT_E_AKTIEN].AddAtPastDay(Money);
        Bilanz.AktienEmission += Money;
        break;
    case 3163:
        /* D::Emissions-Entschädigung (erhalten) */
        Statistiken[STAT_E_AKTIEN].AddAtPastDay(Money);
        Bilanz.AktienEmissionKompErh += Money;
        break;
    case 3170:
        /* D::Flughafenausbau */
        Statistiken[STAT_A_EXPANSION].AddAtPastDay(Money);
        Bilanz.ExpansionGates += Money;
        break;
    case 3180:
        /* D::Vermögen von Übernahmen */
        Statistiken[STAT_E_SONSTIGES].AddAtPastDay(Money);
        Bilanz.Takeovers += Money;
        break;
    case 3181:
        /* D::Liquidierung von %s */
        Statistiken[STAT_E_SONSTIGES].AddAtPastDay(Money);
        Bilanz.Takeovers += Money;
        break;
    case 3200:
        /* D::Ortsgespräch */
        // nicht benutzt
        DebugBreak();
        break;
    case 3201:
        /* D::Ortsgespräch (Handy) */
        // nicht benutzt
        DebugBreak();
        break;
    case 3202:
        /* D::Ferngespräch */
        // nicht benutzt
        DebugBreak();
        break;
    case 3203:
        /* D::Ferngespräch (Handy) */
        // nicht benutzt
        DebugBreak();
        break;
    case 3204:
        /* D::Auslandsgespräch */
        // nicht benutzt
        DebugBreak();
        break;
    case 3205:
        /* D::Auslandsgespräch (Handy) */
        // nicht benutzt
        DebugBreak();
        break;
    case 3300:
        /* D::Münze für Fernglas */
        Statistiken[STAT_A_SONSTIGES].AddAtPastDay(Money);
        Bilanz.SonstigeAusgaben += Money;
        break;
    case 3400:
        /* D::Raketenteil */
        Statistiken[STAT_A_SONSTIGES].AddAtPastDay(Money);
        Bilanz.SonstigeAusgaben += Money;
        break;
    case 3500:
        /* D::Schaden durch Sabotage */
        Statistiken[STAT_A_SONSTIGES].AddAtPastDay(Money);
        Bilanz.SabotageSchaden += Money;
        break;
    case 3501:
        /* D::Schaden durch Unfall */
        Statistiken[STAT_A_SONSTIGES].AddAtPastDay(Money);
        Bilanz.PanneSchaden += Money;
        break;
    case 3502:
        /* D::Obskuras Services GmbH, Novosibirsk (Gewinn/Verlust durch Sabotage (Hacking eines Bankkontos)) */
        if (Money > 0) {
            Statistiken[STAT_E_SONSTIGES].AddAtPastDay(Money);
            Bilanz.SabotageGeklaut += Money;
        } else {
            Statistiken[STAT_A_SONSTIGES].AddAtPastDay(Money);
            Bilanz.SabotageSchaden += Money;
        }
        break;
    case 3503:
        /* D::Rechnung von Security Incorporated */
        Statistiken[STAT_A_AGENTUREN].AddAtPastDay(Money);
        Bilanz.SecurityKosten += Money;
        break;
    case 3600:
        /* D::Vermögenssteuer */
        Statistiken[STAT_A_SONSTIGES].AddAtPastDay(Money);
        Bilanz.Steuer += Money;
        break;
    case 3700:
        /* D::Überweisung von %s */
        Statistiken[STAT_E_SONSTIGES].AddAtPastDay(Money);
        Bilanz.GeldErhalten += Money;
        break;
    case 3701:
        /* D::Überweisung an %s */
        Statistiken[STAT_A_SONSTIGES].AddAtPastDay(Money);
        Bilanz.GeldGeschickt += Money;
        break;
    case 9999:
        /* shoppen im DutyFree */
        Statistiken[STAT_A_SONSTIGES].AddAtPastDay(Money);
        Bilanz.SonstigeAusgaben += Money;
        break;
    default:
        AT_Log("ChangeMoney: No category for %d", Reason);
    }

    if (LocationWin != nullptr) {
        (LocationWin)->AnnouceTipDataUpdate(TIP_MONEY);
    }
}

//--------------------------------------------------------------------------------------------
// Geht in einen Raum hinein (Fenster wird eröffnet)
//--------------------------------------------------------------------------------------------
void PLAYER::EnterRoom(SLONG RoomNum, bool bDontBroadcast) {
    SLONG c = 0;

    for (c = 0; c < 10; c++) {
        if ((Locations[c] & (~(ROOM_ENTERING | ROOM_LEAVING))) == RoomNum) {
            return;
        }
    }

    Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Running = FALSE;

    for (c = 0; c < 10; c++) {
        if (Locations[c] == 0) {
            Locations[c] = UWORD(RoomNum | ROOM_ENTERING);
            CalcRoom();

            if (RoomNum == ROOM_BURO_A + Sim.localPlayer * 10 && (WalkToGlobe != 0)) {
                Locations[c] &= (~ROOM_ENTERING);
                EnterRoom(ROOM_GLOBE);
                WalkToGlobe = 0;
            }

            if (!bDontBroadcast) {
                BroadcastRooms(ATNET_ENTERROOM);
                BroadcastPosition();
            }
            return;
        }
    }

    TeakLibW_Exception(FNL, ExcNever);
}

//--------------------------------------------------------------------------------------------
// Adds a new part to a rocket
//--------------------------------------------------------------------------------------------
void PLAYER::AddRocketPart(SLONG rocketPart, SLONG price) {
    RocketFlags |= rocketPart;
    this->ChangeMoney(-price, 3400, "");

    // Synchronize to other players
    NetSynchronizeFlags();
}

//--------------------------------------------------------------------------------------------
// Adds a new part to a space station
//--------------------------------------------------------------------------------------------
void PLAYER::AddSpaceStationPart(SLONG flag, SLONG rocketPart, SLONG price) {
    RocketFlags |= flag;
    this->ChangeMoney(-price, rocketPart, "");

    // Synchronize to other players
    NetSynchronizeFlags();
}

//--------------------------------------------------------------------------------------------
// Verläßt der Raum
//--------------------------------------------------------------------------------------------
void PLAYER::LeaveRoom() {
    SLONG c = 0;

    for (c = 9; c >= 0; c--) {
        if (Locations[c] != 0U) {
            Locations[c] |= ROOM_LEAVING;
            return;
        }
    }

    BroadcastPosition();
    BroadcastRooms(ATNET_LEAVEROOM);
}

//--------------------------------------------------------------------------------------------
// Verläßt alle Räume
//--------------------------------------------------------------------------------------------
void PLAYER::LeaveAllRooms() {
    SLONG c = 0;

    for (c = 9; c >= 0; c--) {
        if ((Locations[c] != 0U) && Locations[c] != ROOM_AIRPORT) {
            Locations[c] = UWORD(Locations[c] | ROOM_LEAVING);
        }
    }

    BroadcastRooms(ATNET_LEAVEROOM);
}

//--------------------------------------------------------------------------------------------
// Gibt den aktuellen Raum zurück:
//--------------------------------------------------------------------------------------------
UWORD PLAYER::GetRoom() { return (TopLocation); }

//--------------------------------------------------------------------------------------------
// Speed-up für GetRoom()
//--------------------------------------------------------------------------------------------
void PLAYER::CalcRoom() {
    SLONG c = 0;
    SLONG Room = 0;
    BOOL Found = FALSE;

    // Zählen, wie lange wir in diesem Raum schon sind:
    for (c = 9; c >= 0; c--) {
        if (Locations[c] != 0U) {
            SLONG l = (Locations[c] & 255);

            if (l == ROOM_REISEBUERO || l == ROOM_LAST_MINUTE || l == ROOM_SHOP1 || l == ROOM_BANK || l == ROOM_MUSEUM || l == ROOM_ARAB_AIR ||
                l == ROOM_MAKLER || l == ROOM_AUFSICHT || l == ROOM_ROUTEBOX || l == ROOM_RICKS || l == ROOM_SABOTAGE || l == ROOM_NASA || l == ROOM_RUSHMORE ||
                l == ROOM_INSEL || l == ROOM_WERKSTATT || l == ROOM_KIOSK || l == ROOM_FRACHT) {
                Room = Locations[c];
                Found = TRUE;
            }
        }
    }

    if (Found != 0) {
        if (LocationTime == -1) {
            LocationTime = Sim.Time;
        }

        if ((Sim.Time - LocationTime) / 60000 >= 2) {
            LocationForbidden = Room;
            LocationForbiddenTime = Sim.Time;

            for (c = 0; c < 10; c++) {
                if (Locations[c] == Room) {
                    for (; c < 10; c++) {
                        if (Locations[c] != 0U) {
                            Locations[c] |= ROOM_LEAVING;
                        }
                    }
                    break;
                }
            }
        }
    } else {
        LocationTime = -1;
    }

    if ((Sim.Time - LocationForbiddenTime) / 60000 >= 2) {
        LocationForbidden = -1;
    }

    // Top-Location berechnen:
    for (c = 9; c >= 0; c--) {
        if (Locations[c] != 0U) {
            TopLocation = UWORD(Locations[c] & ~(ROOM_LEAVING | ROOM_ENTERING));
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Broadcasts all the rooms we're in:
//--------------------------------------------------------------------------------------------
void PLAYER::BroadcastRooms(SLONG Message, SLONG RoomLeft) {
    if ((Owner == 0 || (Owner == 1 && (Sim.bIsHost != 0))) && (Sim.bNetwork != 0)) {
        TEAKFILE MessageBox;

        MessageBox.Announce(128);

        MessageBox << Message << PlayerNum << DirectToRoom << RoomLeft;

        for (SLONG c = 9; c >= 0; c--) {
            MessageBox << UWORD(Locations[c] & (~(ROOM_ENTERING | ROOM_LEAVING)));
        }

        SIM::SendMemFile(MessageBox);
    }
}

//--------------------------------------------------------------------------------------------
// Berechnet, was die anstehenden Umrüstungen zusammen kosten werden:
//--------------------------------------------------------------------------------------------
SLONG PLAYER::CalcPlanePropSum() {
    SLONG Costs = 0;

    for (SLONG c = Planes.AnzEntries() - 1; c >= 0; c--) {
        if (Planes.IsInAlbum(c) != 0) {
            CPlane &qPlane = Planes[c];

            if (qPlane.Sitze != qPlane.SitzeTarget) {
                Costs += qPlane.ptPassagiere * (SeatCosts[qPlane.SitzeTarget] - SeatCosts[qPlane.Sitze] / 2);
            }
            // Costs+=PlaneTypes[qPlane.TypeId].Passagiere*(SeatCosts[qPlane.SitzeTarget]-SeatCosts[qPlane.Sitze]/2);

            if (qPlane.Tabletts != qPlane.TablettsTarget) {
                Costs += qPlane.ptPassagiere * (TrayCosts[qPlane.TablettsTarget] - TrayCosts[qPlane.Tabletts] / 2);
            }
            // Costs+=PlaneTypes[qPlane.TypeId].Passagiere*(TrayCosts[qPlane.TablettsTarget]-TrayCosts[qPlane.Tabletts]/2);

            if (qPlane.Deco != qPlane.DecoTarget) {
                Costs += qPlane.ptPassagiere * (DecoCosts[qPlane.DecoTarget] - DecoCosts[qPlane.Deco] / 2);
            }
            // Costs+=PlaneTypes[qPlane.TypeId].Passagiere*(DecoCosts[qPlane.DecoTarget]-DecoCosts[qPlane.Deco]/2);

            if (qPlane.Triebwerk != qPlane.TriebwerkTarget) {
                Costs += (TriebwerkCosts[qPlane.TriebwerkTarget] - TriebwerkCosts[qPlane.Triebwerk] / 2);
            }

            if (qPlane.Reifen != qPlane.ReifenTarget) {
                Costs += (ReifenCosts[qPlane.ReifenTarget] - ReifenCosts[qPlane.Reifen] / 2);
            }

            if (qPlane.Elektronik != qPlane.ElektronikTarget) {
                Costs += (ElektronikCosts[qPlane.ElektronikTarget] - ElektronikCosts[qPlane.Elektronik] / 2);
            }

            if (qPlane.Sicherheit != qPlane.SicherheitTarget) {
                Costs += (SicherheitCosts[qPlane.SicherheitTarget] - SicherheitCosts[qPlane.Sicherheit] / 2);
            }
        }
    }

    return Costs;
}

//--------------------------------------------------------------------------------------------
// Berechnet was den Spieler die Sicherheit kostet:
//--------------------------------------------------------------------------------------------
SLONG PLAYER::CalcSecurityCosts(bool bFixOnly, bool bPlaneOnly) {
    SLONG costfix = 0;
    SLONG costplane = 0;

    if ((SecurityFlags & 0x0001) != 0U) {
        costfix += 25000; // Büro
    }
    if ((SecurityFlags & 0x0002) != 0U) {
        costfix += 20000; // Laptop
    }
    if ((SecurityFlags & 0x0004) != 0U) {
        costfix += 25000; // Personbüro
    }
    if ((SecurityFlags & 0x0008) != 0U) {
        costfix += 30000; // Bank
    }
    if ((SecurityFlags & 0x0010) != 0U) {
        costfix += 30000; // Routentafel
    }
    if ((SecurityFlags & 0x0020) != 0U) {
        costfix += 30000; // Verwaltung
    }
    if ((SecurityFlags & 0x0040) != 0U) {
        costplane += 30000; // Flugzeuge von innen
    }
    if ((SecurityFlags & 0x0080) != 0U) {
        costplane += 30000; // Flugzeuge von aussen
    }
    if ((SecurityFlags & 0x0100) != 0U) {
        costplane += 40000; // Gateupdatedate
    }

    if ((SecurityFlags & 0x0400) != 0U) {
        costplane += 50000; // Gateupdatedate II
    }
    if ((SecurityFlags & 0x0800) != 0U) {
        costplane += 50000; // Gateupdatedate III
    }

    if (bFixOnly) {
        return (costfix);
    }
    if (bPlaneOnly) {
        return (costplane);
    }

    return (costfix + costplane * Planes.GetNumUsed());
}

//--------------------------------------------------------------------------------------------
// Zieht Miete für Cities und Routen vom Geld ab:
//--------------------------------------------------------------------------------------------
void PLAYER::BookBuroRent() {
    SLONG c = 0;
    SLONG Gebuehr = 0;

    // Erst einmal die gate-Mieten:
    for (c = Gebuehr = 0; c < Gates.Gates.AnzEntries(); c++) {
        if (Gates.Gates[c].Miete != -1) {
            Gebuehr -= Gates.Gates[c].Miete / 30;
        }
    }
    ChangeMoney(Gebuehr, 2050, "");

    // Dann die Niederlassungs Mieten
    for (c = Gebuehr = 0; c < Cities.AnzEntries(); c++) {
        if ((Cities.IsInAlbum(c) != 0) && (RentCities.RentCities[c].Rang != 0U)) {
            Gebuehr -= RentCities.RentCities[c].Miete / 30;
        }
    }
    if (Gebuehr != 0) {
        ChangeMoney(Gebuehr, 2051, "");
    }

    // Und zuletzt die Routen Mieten
    for (c = Gebuehr = 0; c < Routen.AnzEntries(); c++) {
        if ((Routen.IsInAlbum(c) != 0) && (RentRouten.RentRouten[c].Rang != 0U)) {
            Gebuehr -= RentRouten.RentRouten[c].Miete / 30;
        }
    }
    if (Gebuehr != 0) {
        ChangeMoney(Gebuehr, 2052, "");
    }
}

//------------------------------------------------------------------------------
// Zieht Gehalt vom Geld ab
//------------------------------------------------------------------------------
void PLAYER::BookSalary() {
    SLONG c = 0;
    SLONG Money = 0;

    if (Owner != 2) {
        for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
            if (Workers.Workers[c].Employer == PlayerNum) {
                // Gehaltssumme berechnen:
                Money += Workers.Workers[c].Gehalt;

                // Flugzeugbilanz korrigieren:
                if (Workers.Workers[c].PlaneId != -1) {
                    Planes[Workers.Workers[c].PlaneId].Salden[0] -= Workers.Workers[c].Gehalt / 30;
                }
            }
        }

        if (Money != 0) {
            ChangeMoney(-Money / 30, 2070, "");
        }
    }
}

//------------------------------------------------------------------------------
// Gibt den aktuellen Missionserfolg zurück:
//------------------------------------------------------------------------------
SLONG PLAYER::GetMissionRating(bool bAnderer) {
    if (IsOut != 0) {
        return (-1);
    }
    switch (Sim.Difficulty) {
    case DIFF_TUTORIAL:
        return (NumAuftraege);
        break;

    case DIFF_FREEGAME:
    case DIFF_FIRST:
        return (NumPassengers);
        break;

    case DIFF_EASY:
        return (SLONG(min(0x7fffffff, Gewinn)));
        break;

    case DIFF_NORMAL:
        return (NumMissionRoutes);
        break;

    case DIFF_HARD:
        return (Image / 10);
        break;

    case DIFF_FINAL:
        return (GetAnzBits(RocketFlags));
        break;

        // AddOn-Missionen
    case DIFF_ADDON01: {
        __int64 rc = 0;

        if (Money > 0) {
            rc = Credit; // Nur Kredit
        } else {
            rc = Credit - Money; // Kredit + Überziehungskredit
        }

        return (SLONG(min(0x7fffffff, rc)));
    } break;

    case DIFF_ADDON02:
        return (NumFracht);
        break;

    case DIFF_ADDON03:
        return (NumFrachtFree);
        break;

    case DIFF_ADDON04:
        return (NumMiles);
        break;

    case DIFF_ADDON05: {
        NumServicePoints = 0;

        for (SLONG c = Planes.AnzEntries() - 1; c >= 0; c--) {
            if (Planes.IsInAlbum(c) != 0) {
                CPlane &qPlane = Planes[c];

                NumServicePoints += qPlane.Sitze;
                NumServicePoints += qPlane.Tabletts;
                NumServicePoints += qPlane.Deco;
                NumServicePoints += qPlane.Triebwerk;
                NumServicePoints += qPlane.Reifen;
                NumServicePoints += qPlane.Elektronik;
                NumServicePoints += qPlane.Sicherheit;
            }
        }

        if (Owner == 1 && RobotUse(ROBOT_USE_FAKE_PERSONAL)) {
            // Computerspieler simuliert Personal:
            for (SLONG c = Planes.AnzEntries() - 1; c >= 0; c--) {
                if (Planes.IsInAlbum(c) != 0) {
                    CPlane &qPlane = Planes[c];

                    // Piloten und Begleiter:
                    SLONG Anzahl = qPlane.ptAnzPiloten + qPlane.ptAnzBegleiter;
                    // SLONG Anzahl = PlaneTypes[qPlane.TypeId].AnzPiloten+PlaneTypes[qPlane.TypeId].AnzBegleiter;

                    // Nach und nach weitere Begleiter:
                    Anzahl += min(Sim.Date - 3 - (PlayerNum + 2) % 4, qPlane.ptAnzBegleiter);
                    // Anzahl+=min(Sim.Date-3-(PlayerNum+2)%4, PlaneTypes[qPlane.TypeId].AnzBegleiter);

                    // Durchschnittlich 80%
                    NumServicePoints += (80 - 50) / 15;
                }
            }
        } else {
            // Menschlicher Spieler hat Personal:
            for (SLONG c = 0; c < Workers.Workers.AnzEntries(); c++) {
                CWorker &qWorker = Workers.Workers[c];

                if (qWorker.Employer == PlayerNum && qWorker.PlaneId != -1) {
                    if (qWorker.Talent > 50) {
                        NumServicePoints += (qWorker.Talent - 60) / 15;
                    }
                }
            }
        }

        return (NumServicePoints);
    } break;

    case DIFF_ADDON06:
        return (SLONG(min(0x7fffffff, Statistiken[STAT_FIRMENWERT].GetAtPastDay(0))));
        break;

    case DIFF_ADDON07: {
        // Äußerung zu den Flugzeugen:
        SLONG d = 0;
        SLONG tmp = 0;
        SLONG anz = 0;
        for (d = tmp = anz = 0; d < Planes.AnzEntries(); d++) {
            if (Planes.IsInAlbum(d) != 0) {
                tmp += Planes[d].Zustand;
                anz++;
            }
        }

        if (anz != 0) {
            return (tmp / anz);
        }
        return (0);
    } break;

    case DIFF_ADDON08:
        return (SLONG(Kurse[0]));
        break;

    case DIFF_ADDON09:
        return (NumOrderFlights);
        break;

    case DIFF_ADDON10:
        return (GetAnzBits(RocketFlags));
        break;

    case DIFF_ATFS01:
        return (SLONG(Money));
        break;

    case DIFF_ATFS02: {
        // Äußerung zu den Flugzeugen:
        SLONG d = 0;
        SLONG anz = 0;
        for (d = anz = 0; d < Planes.AnzEntries(); d++) {
            if (Planes.IsInAlbum(d) != 0) {
                if (Planes[d].Zustand >= BTARGET_ZUSTAND && Planes[d].Reifen == 2 && Planes[d].Triebwerk == 2 && Planes[d].Sicherheit == 2 &&
                    Planes[d].Elektronik == 2) {
                    anz++;
                }
            }
        }

        return (anz);
    } break;

    case DIFF_ATFS03: {
        SLONG c = 0;
        __int64 p = 0;
        __int64 f = 0;

        p = Statistiken[STAT_PASSAGIERE].GetAtPastDay(1) - Statistiken[STAT_PASSAGIERE].GetAtPastDay(6);
        for (c = 0; c < min(Sim.Date, 5); c++) {
            f += Statistiken[STAT_FLUGZEUGE].GetAtPastDay(c + 1);
        }

        if (f != 0) {
            return (SLONG(p / f));
        }
        return (0);
    } break;

    case DIFF_ATFS04:
        return (DaysWithoutSabotage);
        break;

    case DIFF_ATFS05: {
        SLONG n = 0;
        for (SLONG d = 0; d < Planes.AnzEntries(); d++) {
            if (Planes.IsInAlbum(d) != 0) {
                if (Planes[d].ptPassagiere >= BTARGET_PLANESIZE && Planes[d].TypeId == -1) {
                    n++;
                }
            }
        }

        return (n);
    } break;

    case DIFF_ATFS06:
        return (DaysWithoutSabotage);
        break;

    case DIFF_ATFS07: {
        SLONG sum = 0;

        if (static_cast<SLONG>(bAnderer) == 0) {
            SLONG anz = 0;
            for (SLONG c = 1; c <= min(29, Sim.Date); c++) {
                sum += SLONG(Statistiken[STAT_AKTIENKURS].GetAtPastDay(c));
                anz++;
            }

            if (anz != 0) {
                return (sum / anz);
            }
            return (0);
        }

        for (SLONG c = 0; c <= 29; c++) {
            auto anz = Statistiken[STAT_AKTIEN_SA + PlayerNum].GetAtPastDay(c);
            auto gesamt = Statistiken[STAT_AKTIEN_ANZAHL].GetAtPastDay(c);
            if ((gesamt > 0) && anz * 100 / gesamt <= BTARGET_MEINANTEIL) {
                sum++;
            }
        }

        return (sum);

    } break;

    case DIFF_ATFS08: {
        SLONG n = 0;
        for (SLONG d = 0; d < Planes.AnzEntries(); d++) {
            if (Planes.IsInAlbum(d) != 0) {
                if (Planes[d].TypeId == -1 && Planes[d].ptPassagiere > 0 && Planes[d].ptGeschwindigkeit > 0) {
                    // SLONG Verbrauch = Planes[d].ptVerbrauch/Planes[d].ptPassagiere;

                    // Neue Formel:
                    // Verbrauch = Planes[d].ptVerbrauch*100/Planes[d].ptGeschwindigkeit*100/Planes[d].ptPassagiere;

                    SLONG Verbrauch = Planes[d].ptVerbrauch * 100 / Planes[d].ptGeschwindigkeit;

                    if (Verbrauch <= BTARGET_VERBRAUCH) {
                        n++;
                    }
                }
            }
        }

        return (n);
    } break;

    case DIFF_ATFS09:
        return (SLONG(min(0x7fffffff, Statistiken[STAT_FIRMENWERT].GetAtPastDay(0))));
        break;

    case DIFF_ATFS10:
        return (SLONG(min(0x7fffffff, Statistiken[STAT_FIRMENWERT].GetAtPastDay(0))));
        break;

    default:
        AT_Log("Player.cpp: Default case should not be reached.");
        DebugBreak();
    }

    return (0);
}

//------------------------------------------------------------------------------
// Did this player win the mission?
//------------------------------------------------------------------------------
BOOL PLAYER::HasWon() {
    auto MissionRating = GetMissionRating();

    if (Sim.Difficulty == DIFF_TUTORIAL && NumAuftraege >= 10) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_FIRST && NumPassengers >= TARGET_PASSENGERS) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_EASY && Gewinn >= TARGET_GEWINN) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_NORMAL && NumMissionRoutes >= TARGET_FLAGS) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_HARD && Image >= TARGET_IMAGE) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_FINAL && GetAnzBits(RocketFlags) >= 10) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ADDON01 && MissionRating == 0) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ADDON02 && NumFracht >= TARGET_FRACHT) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ADDON03) {
        for (SLONG c = 0; c < 4; c++) {
            if ((Sim.Players.Players[c].IsOut == 0) && Sim.Players.Players[c].NumFrachtFree > NumFrachtFree) {
                return (FALSE);
            }
        }

        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ADDON04) {
        for (SLONG c = 0; c < 4; c++) {
            if ((Sim.Players.Players[c].IsOut == 0) && Sim.Players.Players[c].NumMiles > NumMiles) {
                return (FALSE);
            }
        }

        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ADDON05 && NumServicePoints > TARGET_SERVICE) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ADDON06) {
        for (SLONG c = 0; c < 4; c++) {
            if ((Sim.Players.Players[c].IsOut == 0) &&
                Sim.Players.Players[c].Statistiken[STAT_FIRMENWERT].GetAtPastDay(0) > Statistiken[STAT_FIRMENWERT].GetAtPastDay(0)) {
                return (FALSE);
            }
        }

        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ADDON07) {
        SLONG anz = 0;
        for (SLONG d = 0; d < Planes.AnzEntries(); d++) {
            if (Planes.IsInAlbum(d) != 0) {
                if (Planes[d].Zustand >= 90) {
                    anz++;
                }
            }
        }

        return (anz >= 2);
    }
    if (Sim.Difficulty == DIFF_ADDON08 && Kurse[0] >= TARGET_SHARES) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ADDON09 && NumOrderFlights >= TARGET_NUM_UHRIG) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ADDON10 && GetAnzBits(RocketFlags) >= 10) {
        return (TRUE);
    }

    if (Sim.Difficulty == DIFF_ATFS01 && Money >= BTARGET_KONTO) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ATFS02 && MissionRating >= 5) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ATFS03 && Planes.GetNumUsed() >= 4 && MissionRating >= BTARGET_PASSAVG) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ATFS04 && Planes.GetNumUsed() >= 5 && DaysWithoutSabotage >= BTARGET_DAYSSABO) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ATFS05) {
        return static_cast<BOOL>(MissionRating >= 3);
    }
    if (Sim.Difficulty == DIFF_ATFS06 && Planes.GetNumUsed() >= 5 && DaysWithoutSabotage >= BTARGET_DAYSSABO) {
        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ATFS07 && MissionRating >= BTARGET_KURS) {
        for (SLONG c = 0; c <= 29; c++) {
            auto anz = Statistiken[STAT_AKTIEN_SA + PlayerNum].GetAtPastDay(c);
            auto gesamt = Statistiken[STAT_AKTIEN_ANZAHL].GetAtPastDay(c);
            if (anz * 100 / gesamt > BTARGET_MEINANTEIL) {
                return (FALSE);
            }
        }

        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ATFS08) {
        return static_cast<BOOL>(MissionRating >= 5);
    }
    if (Sim.Difficulty == DIFF_ATFS09) {
        for (SLONG c = 0; c < 4; c++) {
            if ((Sim.Players.Players[c].IsOut == 0) &&
                Sim.Players.Players[c].Statistiken[STAT_FIRMENWERT].GetAtPastDay(0) > Statistiken[STAT_FIRMENWERT].GetAtPastDay(0)) {
                return (FALSE);
            }
        }

        return (TRUE);
    }
    if (Sim.Difficulty == DIFF_ATFS10) {
        for (SLONG c = 0; c < 4; c++) {
            if ((Sim.Players.Players[c].IsOut == 0) &&
                Sim.Players.Players[c].Statistiken[STAT_FIRMENWERT].GetAtPastDay(0) > Statistiken[STAT_FIRMENWERT].GetAtPastDay(0)) {
                return (FALSE);
            }
        }

        return (TRUE);
    }

    return (FALSE);
}

//------------------------------------------------------------------------------
// Läßt neuen Tag beginnen
//------------------------------------------------------------------------------
void PLAYER::NewDay() {
    SLONG c = 0;
    SLONG d = 0;
    SLONG tmp = 0;
    SLONG Summe = 0;

    PlayerWalkRandom.SRand(Sim.Date + PlayerNum);
    PlayerExtraRandom.SRand(0);

    if (Sim.bNetwork != 0) {
        NetGenericSync(0x4211001);
        NetSynchronizePlanes();
        NetGenericSync(0x4211011);
        NetUpdateWorkers();
        NetGenericSync(0x4211012);
        NetSynchronizeMeeting();
        NetGenericSync(0x4211013);
    }

    MechAngry = 0;
    OutOfGates = 0;
    IsTalking = 0;
    IsWalking2Player = -1;
    OfficeState = 0;
    PlayerSmoking = 0;
    PlayerStinking = 0;
    HasAlkohol = TRUE;
    Koffein = 0;
    bWasInMuseumToday = FALSE;
    bHasPlanesUpgradedToday = FALSE;
    DaysWithoutStrike++;
    DaysWithoutSabotage++;

    // Holzkohle glüht nicht mehr:
    for (d = 0; d < 6; d++) {
        if (Items[d] == ITEM_GLKOHLE) {
            Items[d] = ITEM_KOHLE;
        }
    }

    if (Owner == 1 && RobotUse(ROBOT_USE_IMAGEBONUS)) {
        Image += (SLONG(Sim.Date) % 3);
        Limit(SLONG(-1000), Image, SLONG(1000));
    }

    if (Owner == 1 && RobotUse(ROBOT_USE_BONUS)) {
        if (Bonus == 0) {
            SLONG neg = 1; // Vorzeichen verdrehen, weil: wenig ist gut
            if (Sim.Difficulty == DIFF_ADDON01) {
                neg = -1;
            }

            if (Sim.Players.Players[Sim.localPlayer].GetMissionRating() * neg > GetMissionRating() * neg) {
                Bonus += 100000;
            }
            if (Sim.Players.Players[Sim.localPlayer].GetMissionRating() * neg * 2 / 3 > GetMissionRating() * neg) {
                Bonus += 100000;
            }
            if (Sim.Players.Players[Sim.localPlayer].GetMissionRating() * neg / 2 > GetMissionRating() * neg) {
                Bonus += 100000;
            }

            if (RobotUse(ROBOT_USE_BONUS_X2)) {
                Bonus *= 2;
            }
            if (RobotUse(ROBOT_USE_BONUS_X4)) {
                Bonus *= 4;
            }
            if (RobotUse(ROBOT_USE_BONUS_X8)) {
                Bonus *= 8;
            }
        }
    } else {
        Bonus = 0;
    }

    // Laptop wird über Nacht repariert:
    if (LaptopVirus == 2) {
        LaptopVirus = 3;
    }
    if ((LaptopVirus != 0) && Owner == 1 && RobotUse(ROBOT_USE_MISC_CHEATS)) {
        LaptopVirus = 0;
    }

    WerbeBroschuere = -1;
    TelephoneDown = 0;
    Presseerklaerung = 0;

    for (c = 0; c < 20; c++) {
        Smoke[c].TTL = 0;
    }

    SickTokay = FALSE;
    RunningToToilet = FALSE;

    LocationTime = -1;
    LocationForbidden = -1;
    LocationForbiddenTime = -1;

    ArabHints -= min(3, ArabHints);

    for (c = 0; c < STAT_ANZ; c++) {
        Statistiken[c].NewDay();
    }

    if (((Sim.Date & 1) != 0) && (CalledPlayer != 0)) {
        CalledPlayer--;
    }
    BoredOfPlayer = FALSE;

    switch (LaptopQuality) {
    case 0:
        // no Laptop
        break;
    case 1:
        LaptopBattery = 40;
        break;
    case 2:
        LaptopBattery = 80;
        break;
    case 3:
        LaptopBattery = 200;
        break;
    case 4:
        LaptopBattery = 1440;
        break;
    default:
        AT_Log("Player.cpp: Default case should not be reached.");
        DebugBreak();
    }

    // LastFlown-Feld bei den Routen aktualisieren:
    for (c = 0; c < Routen.AnzEntries(); c++) {
        auto &qRentRoute = RentRouten.RentRouten[c];
        if ((Routen.IsInAlbum(c) != 0) && (qRentRoute.Rang != 0U)) {
            if (qRentRoute.LastFlown < 99) {
                qRentRoute.LastFlown++;
            }

            qRentRoute.NeuerTag();
            qRentRoute.RoutenAuslastung = qRentRoute.SummeWocheBefoerdert() * 100 / (Routen[c].AnzPassagiere() * 7);
            if (qRentRoute.RoutenAuslastung > 100) {
                qRentRoute.RoutenAuslastung = 100;
            }

            qRentRoute.HeuteBefoerdert = 0;

            if (qRentRoute.RoutenAuslastung < 10) {
                qRentRoute.TageMitGering++;

                if (Owner == 0 && (IsOut == 0)) {
                    if (Routen[c].VonCity < Routen[c].NachCity) {
                        if (qRentRoute.TageMitGering == 10) {
                            Letters.AddLetter(TRUE, // Brief: Warnung
                                              CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 3000), (LPCTSTR)Cities[Routen[c].VonCity].Name,
                                                              (LPCTSTR)Cities[Routen[c].NachCity].Name)),
                                              CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 3001), qRentRoute.RoutenAuslastung, 10)),
                                              StandardTexte.GetS(TOKEN_LETTER, 3002), -1);
                        }

                        if (qRentRoute.TageMitGering == 15) {
                            Letters.AddLetter(TRUE, // Brief: Warnung
                                              CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 3000), (LPCTSTR)Cities[Routen[c].VonCity].Name,
                                                              (LPCTSTR)Cities[Routen[c].NachCity].Name)),
                                              CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 3001), qRentRoute.RoutenAuslastung, 5)),
                                              StandardTexte.GetS(TOKEN_LETTER, 3002), -1);
                        }

                        if (qRentRoute.TageMitGering == 20) {
                            Letters.AddLetter(TRUE, // Brief: Route weg
                                              CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 3100), (LPCTSTR)Cities[Routen[c].VonCity].Name,
                                                              (LPCTSTR)Cities[Routen[c].NachCity].Name)),
                                              StandardTexte.GetS(TOKEN_LETTER, 3101), StandardTexte.GetS(TOKEN_LETTER, 3102), -1);
                        }
                    }
                }

                // Route ggf. wegnehmen
                if (qRentRoute.TageMitGering == 20) {
                    RouteWegnehmen(c);
                }
            } else {
                qRentRoute.TageMitGering = 0;
            }
        } else if (qRentRoute.TageMitGering < 99) {
            qRentRoute.TageMitGering++;
        }
    }

    // Geld verschieben:
    for (c = 0; c < MoneyPast.AnzEntries() - 1; c++) {
        MoneyPast[c] = MoneyPast[c + 1];
    }

    MoneyPast[MoneyPast.AnzEntries() - 1] = Money;

    // Aktienkurseinträge verschieben:
    for (c = 9; c >= 1; c--) {
        Kurse[c] = Kurse[c - 1];
    }

    // Firma macht Verlust? Dann Kurse verschlechtern:
    if (Statistiken[STAT_FIRMENWERT].GetAtPastDay(1) < Statistiken[STAT_FIRMENWERT].GetAtPastDay(2)) {
        Kurse[c] *= 0.97;
    }
    if (Statistiken[STAT_FIRMENWERT].GetAtPastDay(1) < Statistiken[STAT_FIRMENWERT].GetAtPastDay(2) - 1000000) {
        Kurse[c] *= 0.97;
    }
    if (Statistiken[STAT_FIRMENWERT].GetAtPastDay(1) < Statistiken[STAT_FIRMENWERT].GetAtPastDay(2) - 5000000) {
        Kurse[c] *= 0.95;
    }

    // Mit der Zeit kann man mehr Aktien emittieren
    __int64 maxAktien = MaxAktien;
    MaxAktien = min((maxAktien * 105 / 100), 2500000);

    for (c = 0; c < 10; c++) {
        Locations[c] = 0;
    }

    if (Money > 0) {
        ChangeMoney(Money * HabenZins / 100 / 365, 2001, "");
    }
    if (Money < 0) {
        ChangeMoney(Money * HabenZins / 100 / 365, 2002, "");
    }
    if (Credit > 0) {
        ChangeMoney(-Money * SollZins / 100 / 365, 2000, "");
    }
    if (CalcSecurityCosts() > 0) {
        ChangeMoney(-CalcSecurityCosts(), 3503, "");
    }

    BookBuroRent();
    BookSalary();
    UpdateAuftraege();
    RobotInit();
    UpdateWalkSpeed();
    Messages.NewDay();

    // Die Aktien verbuchen:
    tmp = (AnzAktien - OwnsAktien[PlayerNum]) * Dividende;
    if (tmp != 0) {
        ChangeMoney(-tmp / 365, 3140, "");
    }

    for (c = tmp = 0; c < Sim.Players.Players.AnzEntries(); c++) {
        if (c != PlayerNum && (Sim.Players.Players[c].IsOut == 0)) {
            tmp += OwnsAktien[c] * Sim.Players.Players[c].Dividende;
        }
    }
    if (tmp != 0) {
        ChangeMoney(tmp / 365, 3141, "");
    }

    if (Bilanz.GetSumme() > 0 && ((SLONG(Sim.Date) & 1) != 0)) {
        if (TrustedDividende < Dividende) {
            TrustedDividende++;
        }
    }

    if (Bilanz.GetSumme() < 0) {
        if (TrustedDividende < Dividende / 2) {
            if ((SLONG(Sim.Date) & 1) != 0) {
                TrustedDividende++;
            }
        } else {
            TrustedDividende--;
        }
    }

    // Die Nachrichten verwalten:
    while (true) {
        c = rand() % 80;
        if (c > 16) {
            break;
        }

        if (Owner == 0 && (IsOut == 0)) {
            Letters.AddLetter(TRUE, StandardTexte.GetS(TOKEN_LETTER, 2000 + c * 10), StandardTexte.GetS(TOKEN_LETTER, 2001 + c * 10),
                              StandardTexte.GetS(TOKEN_LETTER, 2002 + c * 10), -1);
        }
    }

    // Flugzeuge warten:
    {
        TEAKRAND PlaneRand;

        PlaneRand.SRand(Sim.Date);

        Summe = 0;
        for (c = Planes.AnzEntries() - 1; c >= 0; c--) {
            if (Planes.IsInAlbum(c) != 0) {
                SLONG Improvement = 0;

                Planes[c].FlugplaeneFortfuehren(PlayerNum);

                // Reparaturkosten auch in die Salden (aber nur wenn etwas repariert werden soll)
                if (Planes[c].Zustand < Planes[c].TargetZustand + 2) {
                    UBYTE OldZustand = Planes[c].Zustand;

                    if (Planes[c].Zustand < Planes[c].WorstZustand) {
                        Planes[c].WorstZustand = Planes[c].Zustand;
                    }
                    UBYTE OldWorst = Planes[c].WorstZustand;

                    switch (MechMode) {
                    // Putzfrau:
                    case 0:
                        if (((c + Planes[c].Zustand + Sim.Date) & 1) == 0) {
                            Planes[c].Zustand -= 2;
                        }
                        if (Planes[c].Zustand > 200) {
                            Planes[c].Zustand = 0;
                        }
                        break;

                        // Lehrling:
                    case 1:
                        if (((c + Planes[c].Zustand + Sim.Date) & 7) == 0) {
                            Planes[c].Zustand -= 2;
                        } else {
                            Planes[c].Zustand = UBYTE(min(Planes[c].Zustand + 5, 100));
                        }
                        if (Planes[c].Zustand > 200) {
                            Planes[c].Zustand = 0;
                        }
                        break;

                        // Mechaniker:
                    case 2:
                        if (Planes[c].Zustand < 60) {
                            Planes[c].Zustand = UBYTE(min(Planes[c].Zustand + PlaneRand.Rand(5) + 2, 100));
                        } else {
                            Planes[c].Zustand = UBYTE(min(Planes[c].Zustand + PlaneRand.Rand(8) + 2, 100));
                        }
                        break;

                        // Diplom-Dingsbums:
                    case 3:
                        if (Planes[c].Zustand < 60) {
                            Planes[c].Zustand = UBYTE(min(Planes[c].Zustand + 18, 100));
                        } else {
                            Planes[c].Zustand = UBYTE(min(Planes[c].Zustand + 15, 100));
                        }
                        break;
                    default:
                        AT_Log("Player.cpp: Default case should not be reached.");
                        DebugBreak();
                    }

                    if (Planes[c].Zustand > Planes[c].TargetZustand) {
                        Planes[c].Zustand = Planes[c].TargetZustand;
                    }
                    if (Planes[c].Zustand > Planes[c].WorstZustand + 20) {
                        Improvement = Planes[c].Zustand - (Planes[c].WorstZustand + 20);

                        if (Planes[c].Zustand < 20) {
                            Planes[c].WorstZustand = 0;
                        } else {
                            Planes[c].WorstZustand = Planes[c].Zustand - 20;
                        }

                        // Planes[c].Zustand=UBYTE(Planes[c].WorstZustand+20);
                    }

                    // Wartungskosten berechnen:
                    SLONG salary = gRepairPrice[MechMode] / 30;
                    SLONG costImprovement = 0;
                    SLONG costRepairs = 0;

                    if (Planes[c].Zustand > OldZustand) {
                        Planes[c].WorstZustand = max(Planes[c].WorstZustand, Planes[c].Zustand - 20);

                        costImprovement = Improvement * Planes[c].ptPreis / 110;

                        costRepairs = SLONG((Planes[c].Zustand - OldZustand) * 10 * Planes[c].ptWartungsfaktor * (2100 - Planes[c].Baujahr) / 100 *
                                            (200 - Planes[c].Zustand) / 100);
                    }

                    SLONG delta = salary + costImprovement + costRepairs;
                    AT_Log("Player.cpp: %s: Repair of plane %s (%u => %u; worst %u => %u) costs: %ld+%ld+%ld=%ld", (LPCTSTR)AirlineX, (LPCTSTR)Planes[c].Name,
                           OldZustand, Planes[c].Zustand, OldWorst, Planes[c].WorstZustand, salary, costImprovement, costRepairs, delta);
                    if (delta < 0) {
                        delta = 0;
                        AT_Error("Player.cpp: Repair cost for Player %li negative!", PlayerNum);
                    }

                    Summe += delta;
                    Planes[c].Salden[0] -= delta;
                    Planes[c].Wartungskosten += delta;
                }

                for (d = 6; d >= 1; d--) {
                    Planes[c].Salden[d] = Planes[c].Salden[d - 1];
                }
                Planes[c].Salden[0] = 0;
            }
        }
        ChangeMoney(-Summe, 3110, "");
    }

    if (Sim.Difficulty == DIFF_ATFS10) {
        if (Sim.Date == 18) {
            Kurse[0] *= 0.20;
        }
        if (Sim.Date == 55) {
            Image = max(-1000, Image - 200);
        }

        if (Sim.Date == 40 && Planes.GetNumUsed() > 0) {
            SLONG i = Planes.GetRandomUsedIndex();

            Planes -= i;
            UpdateAuftragsUsage();
            UpdateFrachtauftragsUsage();
            MapWorkers(0);
        }

        if (Sim.Date == 35) {
            for (d = 0; d < Planes.AnzEntries(); d++) {
                if (Planes.IsInAlbum(d) != 0) {
                    Planes[d].Zustand = max(0, Planes[d].Zustand - 40);
                    Planes[d].TargetZustand = min(Planes[d].TargetZustand, Planes[d].Zustand + 10);
                }
            }
        }

        if ((Owner != 1 || !RobotUse(ROBOT_USE_FAKE_PERSONAL)) && (Sim.Date == 20 || Sim.Date == 45)) {
            Workers.AddHappiness(PlayerNum, -45);
        }
    }

    // Den Flugplan für die Routen updaten:
    if (DoRoutes != 0) {
        RobotPlanRoutes();
    }

    // Eine Vermögenssteuer. Speziell für Eric:
    __int64 Tax = 0;

    // MaxInt64: 18446744073709551615

    // Für ATP überall noch 0 Nullen hinzugefügt:
    if (Money > 50000000000000) {
        Tax += (Money - 50000000000000) / 10; // 10% für Vermögen über  50000 Mio
    }
    if (Money > 100000000000000) {
        Tax += (Money - 100000000000000) / 10; // nochmal 10% für über  100000 Mio
    }
    if (Money > 200000000000000) {
        Tax += (Money - 200000000000000) / 10; // nochmal 10% für über  200000 Mio
    }
    if (Money > 400000000000000) {
        Tax += (Money - 400000000000000) / 10; // nochmal 10% für über  400000 Mio
    }
    if (Money > 800000000000000) {
        Tax += (Money - 800000000000000) / 10; // nochmal 10% für über  800000 Mio
    }
    if (Money > 1200000000000000) {
        Tax += (Money - 1200000000000000) / 10; // nochmal 10% für über 1200000 Mio
    }
    if (Money > 1600000000000000) {
        Tax += (Money - 1600000000000000) / 10; // nochmal 10% für über 1600000 Mio
    }
    if (Money > 1700000000000000) {
        Tax += (Money - 1700000000000000) / 10; // nochmal 10% für über 1700000 Mio
    }
    if (Money > 1800000000000000) {
        Tax += (Money - 1800000000000000) / 10; // nochmal 10% für über 1800000 Mio
    }
    if (Money > 1900000000000000) {
        Tax += (Money - 1900000000000000) / 10; // nochmal 10% für über 1900000 Mio
    }

    if (Tax != 0) {
        ChangeMoney(-Tax, 3600, "");
    }
}

//--------------------------------------------------------------------------------------------
// Nimmt dem Spieler eine Route weg (wg. Sabotage oder geringer Auslastung
//--------------------------------------------------------------------------------------------
void PLAYER::RouteWegnehmen(SLONG Routenindex, SLONG NeuerBesitzer) {
    // MP: Sim.Players.Players[NeuerBesitzer].RentRouten.RentRouten[Routenindex].Rang != -1
    // geändert zu
    // Sim.Players.Players[NeuerBesitzer].RentRouten.RentRouten[Routenindex].Rang != 0
    if (NeuerBesitzer == -1 || Sim.Players.Players[NeuerBesitzer].RentRouten.RentRouten[Routenindex].Rang != 0) {
        for (SLONG e = 0; e < 4; e++) {
            if ((Sim.Players.Players[e].IsOut == 0) &&
                Sim.Players.Players[e].RentRouten.RentRouten[Routenindex].Rang > RentRouten.RentRouten[Routenindex].Rang) {
                Sim.Players.Players[e].RentRouten.RentRouten[Routenindex].Rang--;
            }
        }
    } else {
        PLAYER &qPlayer = Sim.Players.Players[NeuerBesitzer];

        qPlayer.RentRouten.RentRouten[Routenindex].Rang = RentRouten.RentRouten[Routenindex].Rang;
        qPlayer.RentRouten.RentRouten[Routenindex].TageMitGering = 0;

        qPlayer.RentRouten.RentRouten[Routenindex].Miete = Routen[Routenindex].Miete;
        qPlayer.RentRouten.RentRouten[Routenindex].Image = 0;

        qPlayer.RentRouten.RentRouten[Routenindex].LastFlown = 30;
        qPlayer.RentRouten.RentRouten[Routenindex].AvgFlown = 30;
        qPlayer.RentRouten.RentRouten[Routenindex].Auslastung = 0;
        qPlayer.RentRouten.RentRouten[Routenindex].AuslastungFC = 0;

        qPlayer.RentRouten.RentRouten[Routenindex].TageMitVerlust = 0;

        qPlayer.RentRouten.RentRouten[Routenindex].Ticketpreis =
            CalculateFlightCost(Routen[Routenindex].VonCity, Routen[Routenindex].NachCity, 800, 800, -1) * 3 / 180 * 2 * 2 / 10 * 10;
        qPlayer.RentRouten.RentRouten[Routenindex].TicketpreisFC = qPlayer.RentRouten.RentRouten[Routenindex].Ticketpreis * 2;
    }

    RentRouten.RentRouten[Routenindex].Rang = 0;
    RentRouten.RentRouten[Routenindex].TageMitGering = 0;

    // Aus Flugplan entfernen:
    for (SLONG d = 0; d < Planes.AnzEntries(); d++) {
        if (Planes.IsInAlbum(d) != 0) {
            CFlugplan &qPlan = Planes[d].Flugplan;

            for (SLONG e = Planes[d].Flugplan.Flug.AnzEntries() - 1; e >= 0; e--) {
                if (qPlan.Flug[e].ObjectType == 1 && Routen(qPlan.Flug[e].ObjectId) == ULONG(Routenindex)) {
                    if (qPlan.Flug[e].Startdate > Sim.Date || (qPlan.Flug[e].Startzeit >= 1 && qPlan.Flug[e].Startdate == Sim.Date)) {
                        // Löschen:
                        if (e == 0) {
                            qPlan.StartCity = qPlan.Flug[e].NachCity;
                        }

                        qPlan.Flug[e].ObjectType = 0;
                        qPlan.UpdateNextFlight();
                        qPlan.UpdateNextStart();
                        Planes[d].CheckFlugplaene(PlayerNum);
                        PlanGates();
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Bringt Aufträge aus neusten Stand; bucht Strafen
//--------------------------------------------------------------------------------------------
void PLAYER::UpdateAuftraege() {
    SLONG c = 0;

    // Aufträge anschauen
    for (c = 0; c < Auftraege.AnzEntries(); c++) {
        if (Auftraege.IsInAlbum(c) == 0) {
            continue;
        }
        if (Auftraege[c].BisDate == Sim.Date - 1) {
            if (Auftraege[c].InPlan != -1 && (Owner == 0 || Owner == 2 || !RobotUse(ROBOT_USE_NO_FINE))) { // ex: nur Owner==0
                if (!(Auftraege[c].InPlan == 1 && Auftraege[c].Okay == 1)) {
                    if (Auftraege[c].Strafe > 0) {
                        ChangeMoney(
                            -Auftraege[c].Strafe, 2060,
                            (LPCTSTR)CString(bprintf("%s-%s", (LPCTSTR)Cities[Auftraege[c].VonCity].Kuerzel, (LPCTSTR)Cities[Auftraege[c].NachCity].Kuerzel)));

                        if (Owner == 0 && (IsOut == 0)) {
                            Letters.AddLetter(TRUE, StandardTexte.GetS(TOKEN_LETTER, 1000),
                                              (LPCTSTR)CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 1001), (LPCTSTR)Cities[Auftraege[c].VonCity].Name,
                                                                       (LPCTSTR)Cities[Auftraege[c].NachCity].Name, Auftraege[c].Strafe)),
                                              StandardTexte.GetS(TOKEN_LETTER, 1002), -1);
                        }
                    } else {
                        if (Owner == 0 && (IsOut == 0)) {
                            Letters.AddLetter(TRUE, StandardTexte.GetS(TOKEN_LETTER, 1010),
                                              (LPCTSTR)CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 1011), (LPCTSTR)Cities[Auftraege[c].VonCity].Name,
                                                                       (LPCTSTR)Cities[Auftraege[c].NachCity].Name)),
                                              StandardTexte.GetS(TOKEN_LETTER, 1012), -1);
                        }
                    }
                }
            }
        } else if (Auftraege[c].BisDate == Sim.Date - 2) {
            SLONG d = 0;
            SLONG e = 0;
            BOOL CantDelete = FALSE;

            // Veralteten Auftrag aus Flugplan entfernen:
            for (d = 0; d < Planes.AnzEntries(); d++) {
                if (Planes.IsInAlbum(d) == 0) {
                    continue;
                }

                CFlugplan &qPlan = Planes[d].Flugplan;
                for (e = Planes[d].Flugplan.Flug.AnzEntries() - 1; e >= 0; e--) {
                    auto &qFPE = qPlan.Flug[e];
                    if (qFPE.ObjectType == 2 && Auftraege(qFPE.ObjectId) == ULONG(c)) {
                        if (qFPE.Startdate > Sim.Date || (qFPE.Startdate == Sim.Date && qFPE.Startzeit > Sim.GetHour() + 2)) {
                            // Löschen:
                            qFPE = {};
                            qPlan.UpdateNextFlight();
                            qPlan.UpdateNextStart();
                            Planes[d].CheckFlugplaene(PlayerNum);
                            UpdateAuftragsUsage();
                            if (DoRoutes == 0) {
                                DelayFlightsIfNecessary();
                            }
                        } else {
                            CantDelete = TRUE;
                        }
                    }
                }
            }

            if (CantDelete == 0) {
                Auftraege -= c;
            }
        }
    }

    // Das gleiche für Frachtaufträge:
    for (c = 0; c < Frachten.AnzEntries(); c++) {
        if (Frachten.IsInAlbum(c) == 0) {
            continue;
        }
        if (Frachten[c].BisDate == Sim.Date - 1) {
            if (Frachten[c].InPlan != -1 && (Owner == 0 || Owner == 2 || !RobotUse(ROBOT_USE_NO_FINE))) { // ex: nur Owner==0
                if (!(Frachten[c].InPlan == 1 && Frachten[c].Okay == 1)) {
                    if (Frachten[c].Strafe > 0) {
                        ChangeMoney(
                            -Frachten[c].Strafe, 2065,
                            (LPCTSTR)CString(bprintf("%s-%s", (LPCTSTR)Cities[Frachten[c].VonCity].Kuerzel, (LPCTSTR)Cities[Frachten[c].NachCity].Kuerzel)));

                        if (Owner == 0 && (IsOut == 0)) {
                            Letters.AddLetter(TRUE, StandardTexte.GetS(TOKEN_LETTER, 1005),
                                              (LPCTSTR)CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 1006), (LPCTSTR)Cities[Frachten[c].VonCity].Name,
                                                                       (LPCTSTR)Cities[Frachten[c].NachCity].Name, Frachten[c].Strafe)),
                                              StandardTexte.GetS(TOKEN_LETTER, 1007), -1);
                        }
                    } else {
                        if (Owner == 0 && (IsOut == 0)) {
                            Letters.AddLetter(TRUE, StandardTexte.GetS(TOKEN_LETTER, 1015),
                                              (LPCTSTR)CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 1016), (LPCTSTR)Cities[Frachten[c].VonCity].Name,
                                                                       (LPCTSTR)Cities[Frachten[c].NachCity].Name)),
                                              StandardTexte.GetS(TOKEN_LETTER, 1017), -1);
                        }
                    }
                }
            }
        } else if (Frachten[c].BisDate <= Sim.Date - 2) {
            SLONG d = 0;
            SLONG e = 0;
            BOOL CantDelete = FALSE;

            // Veralteten Frachtauftrag aus Flugplan entfernen:
            for (d = 0; d < Planes.AnzEntries(); d++) {
                if (Planes.IsInAlbum(d) == 0) {
                    continue;
                }
                CFlugplan &qPlan = Planes[d].Flugplan;

            start_loop_again:
                for (e = Planes[d].Flugplan.Flug.AnzEntries() - 1; e >= 0; e--) {
                    auto &qFPE = qPlan.Flug[e];
                    if (qFPE.ObjectType == 4 && Frachten(qFPE.ObjectId) == ULONG(c)) {
                        if (qFPE.Startdate > Sim.Date || (qFPE.Startdate == Sim.Date && qFPE.Startzeit > Sim.GetHour() + 2)) {
                            qFPE = {};
                            qPlan.UpdateNextFlight();
                            qPlan.UpdateNextStart();
                            Planes[d].CheckFlugplaene(PlayerNum);
                            UpdateFrachtauftragsUsage();
                            if (DoRoutes == 0) {
                                DelayFlightsIfNecessary();
                            }
                            goto start_loop_again;
                        } else {
                            CantDelete = TRUE;
                        }
                    }
                }
            }

            if (CantDelete == 0) {
                Frachten -= c;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Wurde ein Auftrag schon irgendwo eingetragen? Gibt ggf. Namen zurück
//--------------------------------------------------------------------------------------------
BOOL PLAYER::IsAuftragInUse(SLONG AuftragsId, CString *PlaneName) {
    SLONG c = 0;
    SLONG d = 0;

    for (c = 0; c < Planes.AnzEntries(); c++) {
        if (Planes.IsInAlbum(c) != 0) {
            for (d = Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
                if (Planes[c].Flugplan.Flug[d].ObjectType == 2 && Auftraege(Planes[c].Flugplan.Flug[d].ObjectId) == Auftraege(AuftragsId)) {
                    if (PlaneName != nullptr) {
                        (*PlaneName) = Planes[c].Name;
                    }

                    return (TRUE);
                }
            }
        }
    }

    return (FALSE);
}

//--------------------------------------------------------------------------------------------
// Mietet eine neue Route:
//--------------------------------------------------------------------------------------------
void PLAYER::RentRoute(SLONG City1, SLONG City2, SLONG Miete) {
    SLONG c = 0;
    SLONG d = 0;
    SLONG n = 0;

    for (c = 0; c < Routen.AnzEntries(); c++) {
        if (Routen.IsInAlbum(c) != 0) {
            if ((Routen[c].VonCity == static_cast<ULONG>(City1) && Routen[c].NachCity == static_cast<ULONG>(City2)) ||
                (Routen[c].VonCity == static_cast<ULONG>(City2) && Routen[c].NachCity == static_cast<ULONG>(City1))) {
                n = 1;

                for (d = 0; d < Sim.Players.AnzPlayers; d++) {
                    if (Sim.Players.Players[d].RentRouten.RentRouten[c].Rang != 0U) {
                        n++;
                    }
                }

                RentRouten.RentRouten[c].Rang = static_cast<UBYTE>(n);
                RentRouten.RentRouten[c].Miete = Miete;
                RentRouten.RentRouten[c].Image = 0;

                RentRouten.RentRouten[c].LastFlown = 30;
                RentRouten.RentRouten[c].AvgFlown = 30;
                RentRouten.RentRouten[c].Auslastung = 0;

                RentRouten.RentRouten[c].TageMitVerlust = 0;

                RentRouten.RentRouten[c].Ticketpreis = CalculateFlightCost(City1, City2, 800, 800, -1) * 3 / 180 * 2 * 2 / 10 * 10;
                RentRouten.RentRouten[c].TicketpreisFC = RentRouten.RentRouten[c].Ticketpreis * 2;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Mietet eine neues Gate:
//--------------------------------------------------------------------------------------------
void PLAYER::RentGate(SLONG Nummer, SLONG Miete) {
    SLONG c = 0;

    for (c = 0; c < Gates.Gates.AnzEntries(); c++) {
        if (Gates.Gates[c].Miete == -1) {
            Gates.NumRented++;
            Gates.Gates[c].Nummer = Nummer;
            Gates.Gates[c].Miete = Miete;
            return;
        }
    }

    TeakLibW_Exception(FNL, ExcNever);
}

//--------------------------------------------------------------------------------------------
// Berechnet, wieviel Kredit der Spieler noch aufnehmen kann.
//--------------------------------------------------------------------------------------------
SLONG PLAYER::CalcCreditLimit() const {
    __int64 cr = (Money - Credit) / 2 - Credit;

    if (Credit < 200000) {
        cr = max(cr, 200000 - Credit);
    }

    return (SLONG(min(0x7fffffff, max(0, cr))));
}

//--------------------------------------------------------------------------------------------
// Berechnet wieviele Flugzeuge heute diese Route fliegen
//--------------------------------------------------------------------------------------------
SLONG PLAYER::AnzPlanesOnRoute(ULONG RouteId) {
    SLONG c = 0;
    SLONG d = 0;
    SLONG rc = 0;

    for (c = rc = 0; c < Planes.AnzEntries(); c++) {
        if (Planes.IsInAlbum(c) != 0) {
            for (d = Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
                if (Planes[c].Flugplan.Flug[d].ObjectType == 1 && Routen(Planes[c].Flugplan.Flug[d].ObjectId) == Routen(RouteId)) {
                    rc++;
                }
            }
        }
    }

    return (rc);
}

//--------------------------------------------------------------------------------------------
// Der Spieler kann in mehreren Räumen zugleich sein (Büro, Scheduler, ...); Ist er im Parameter-Raum?
//--------------------------------------------------------------------------------------------
BOOL PLAYER::IsLocationInQueue(UWORD Location) {
    SLONG c = 0;

    for (c = 0; c < 10; c++) {
        if (Locations[c] == Location) {
            return (TRUE);
        }
        if ((Location == ROOM_BURO_A || Location == ROOM_BURO_B || Location == ROOM_BURO_C || Location == ROOM_BURO_D) &&
            (Locations[c] == ROOM_BURO_A || Locations[c] == ROOM_BURO_B || Locations[c] == ROOM_BURO_C || Locations[c] == ROOM_BURO_D)) {
            return (TRUE);
        }
        if ((Location == ROOM_PERSONAL_A || Location == ROOM_PERSONAL_B || Location == ROOM_PERSONAL_C || Location == ROOM_PERSONAL_D) &&
            (Locations[c] == ROOM_PERSONAL_A || Locations[c] == ROOM_PERSONAL_B || Locations[c] == ROOM_PERSONAL_C || Locations[c] == ROOM_PERSONAL_D)) {
            return (TRUE);
        }
    }

    return (FALSE);
}

//--------------------------------------------------------------------------------------------
// Ist ein geschlossener Raum in der Queue?
//--------------------------------------------------------------------------------------------
BOOL PLAYER::IsClosedLocationInQueue() {
    SLONG c = 0;

    for (c = 0; c < 10; c++) {
        if (c != 0) {
            SLONG l = Locations[c] & (~ROOM_ENTERING) & (~ROOM_ENTERING);

            if (l != ROOM_AIRPORT && l != ROOM_LAPTOP && l != ROOM_LAST_MINUTE && l != ROOM_LAST_MINUTE && l != ROOM_KIOSK && l != ROOM_ARAB_AIR) {
                return (TRUE);
            }
        }
    }

    return (FALSE);
}

//--------------------------------------------------------------------------------------------
// Hat der Spieler einen bestimmten Berater? Gibt ggf. die Qualität zurück
//--------------------------------------------------------------------------------------------
SLONG PLAYER::HasBerater(SLONG Berater) const {
    SLONG c = 0;
    SLONG Max = 0;

    if (CheatBerater != 0 && PlayerNum == Sim.localPlayer) {
        Max = CheatBerater;
    }

    for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
        if (Workers.Workers[c].Typ == Berater && Workers.Workers[c].Employer == PlayerNum) {
            Max = max(Max, Workers.Workers[c].Talent);
        }
    }

    return (Max);
}

//--------------------------------------------------------------------------------------------
// Hat sich so ein Berater beworben?
//--------------------------------------------------------------------------------------------
BOOL PLAYER::HasBeraterApplied(SLONG Berater) {
    SLONG c = 0;

    for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
        if (Workers.Workers[c].Typ == Berater && Workers.Workers[c].Employer == WORKER_JOBLESS) {
            return (TRUE);
        }
    }

    return (FALSE);
}

//--------------------------------------------------------------------------------------------
// Der Personalberater gibt Weisheiten von sich:
//--------------------------------------------------------------------------------------------
void PLAYER::UpdatePersonalberater(SLONG Toleranz) {
    if (HasBerater(BERATERTYP_PERSONAL) != 0) {
        if (xPiloten > Toleranz) {
            Messages.AddMessage(BERATERTYP_PERSONAL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 1100), xPiloten), MESSAGE_COMMENT);
        } else if (xBegleiter > Toleranz) {
            Messages.AddMessage(BERATERTYP_PERSONAL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 1101), xBegleiter), MESSAGE_COMMENT);
        } else if (xPiloten < 0 && xBegleiter < 0) {
            Messages.AddMessage(BERATERTYP_PERSONAL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 1104), -xPiloten, -xBegleiter), MESSAGE_COMMENT);
        } else if (xPiloten < 0) {
            Messages.AddMessage(BERATERTYP_PERSONAL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 1102), -xPiloten), MESSAGE_COMMENT);
        } else if (xBegleiter < 0) {
            Messages.AddMessage(BERATERTYP_PERSONAL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 1103), -xBegleiter), MESSAGE_COMMENT);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Welche Aufträge wurden wie oft verplant?:
//--------------------------------------------------------------------------------------------
void PLAYER::UpdateAuftragsUsage() {
    SLONG c = 0;
    SLONG d = 0;

    for (c = Auftraege.AnzEntries() - 1; c >= 0; c--) {
        if (Auftraege.IsInAlbum(c) != 0) {
            Auftraege[c].Okay = 1;

            if (Auftraege[c].InPlan != -1) {
                Auftraege[c].InPlan = 0;
            }
        }
    }

    for (c = 0; c < Planes.AnzEntries(); c++) {
        if (Planes.IsInAlbum(c) == 0) {
            continue;
        }

        CFlugplan *Plan = &Planes[c].Flugplan;
        for (d = Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
            CFlugplanEintrag &qFPE = Plan->Flug[d];
            if (qFPE.ObjectType != 2) {
                continue;
            }

            auto &job = Auftraege[qFPE.ObjectId];
            if (job.InPlan != 0) {
                continue;
            }

            qFPE.Okay = 0; // Alles klar
            job.InPlan = 1;

            // Nur bei Aufträgen von menschlichen Spielern
            if (Owner == 0 || Owner == 2 || !RobotUse(ROBOT_USE_NO_CHECK_FLIGHT)) { // ex: Nur Owner==0
                if (Planes[c].ptPassagiere < SLONG(job.Personen)) {
                    job.Okay = 0;
                    qFPE.Okay = 3; // Passagierzahl!
                }

                if (qFPE.Startdate < job.Date || qFPE.Startdate > job.BisDate) {
                    job.Okay = 0;
                    qFPE.Okay = 1; // Falscher Tag!
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Welche Frachtaufträge wurden wie oft verplant?:
//--------------------------------------------------------------------------------------------
void PLAYER::UpdateFrachtauftragsUsage() {
    SLONG c = 0;
    SLONG d = 0;

    // TonsOpen bei allen Frachtaufträge resetten:
    for (c = Frachten.AnzEntries() - 1; c >= 0; c--) {
        if (Frachten.IsInAlbum(c) != 0) {
            Frachten[c].Okay = 1;

            if (Frachten[c].InPlan != -1 && Frachten[c].TonsLeft > 0) {
                Frachten[c].InPlan = 0;
                Frachten[c].TonsOpen = Frachten[c].TonsLeft;
            }
        }
    }

    // TonsOpen bei allen Frachtaufträge neu berechnen:
    for (c = 0; c < Planes.AnzEntries(); c++) {
        if (Planes.IsInAlbum(c) == 0) {
            continue;
        }

        CFlugplan *Plan = &Planes[c].Flugplan;
        for (d = Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
            CFlugplanEintrag &qFPE = Plan->Flug[d];
            if (qFPE.ObjectType != 4) {
                continue;
            }

            auto &job = Frachten[qFPE.ObjectId];
            if (job.InPlan != 0) {
                continue;
            }

            // Nur bei Aufträgen von menschlichen Spielern
            if (Owner == 0 || Owner == 2 || !RobotUse(ROBOT_USE_NO_CHECK_FFLIGHT)) { // ex: Nur Owner==0
                if (qFPE.Startdate < job.Date || qFPE.Startdate > job.BisDate) {
                    job.Okay = 0;
                    qFPE.Okay = 1; // Falscher Tag!
                    continue;
                }
            }

            // Ist dieser Frachtflug überhaupt noch zu erledigen?
            if (job.TonsLeft != 0) {
                // Wir misbrauchen bei Frachtflügen das Passagierfeld um zu speichern, wieviel Fracht hier mitfliegt
                qFPE.Passagiere = Planes[c].ptPassagiere / 10;

                // Flug nur beachten, wenn er noch nicht gestartet ist:
                // Heute ist Tag 5 15:00
                // Flug ging an Tag 4 16:00 los. Tag < 5
                // Flug 2 geht an Tag 5 18:00 los

                BOOL ignoreFlight = 0;
                if (qFPE.Startdate < Sim.Date) {
                    ignoreFlight = 1;
                } // (qFPE.Startzeit==Sim.GetHour() && (Sim.GetHour()<30 || Planes[c].Ort!=-5)))
                if (qFPE.Startdate == Sim.Date && qFPE.Startzeit < Sim.GetHour()) {
                    ignoreFlight = 1;
                }

                if (!ignoreFlight) {
                    job.TonsOpen -= Planes[c].ptPassagiere / 10;
                }

                if (job.TonsOpen <= 0) {
                    qFPE.Passagiere -= UWORD(-job.TonsOpen);
                    job.TonsOpen = 0;
                    job.InPlan = 1; // New
                    qFPE.Okay = 0;  // Alles klar
                }

                // Bei Frachten warnen, wenn die Frachtmenge auf 0 schrumpft:
                if (qFPE.Passagiere == 0) {
                    qFPE.GateWarning = TRUE;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Plant die Gate-Vergabe neu:
//--------------------------------------------------------------------------------------------
// Fliegt ein Flug um 15:00 (heute) ab, dann ist ein Gate von 14:00-15:59 belegt. Geht ein
// Flug (laut Flugplan) bis 15:00, dann kommt er so gegen 14:30 an. Das Gate ist von 14:00 bis
// 14:59 belegt
//--------------------------------------------------------------------------------------------
struct flug_info_t {
    flug_info_t(CFlugplanEintrag *p, SLONG i, bool f) : flug(p), zeit(i), istAbflug(f) {}
    CFlugplanEintrag *flug{nullptr};
    SLONG zeit{};
    bool istAbflug{};
};

void PLAYER::PlanGates() {
    // Alte Gate-Einteilung löschen:
    for (SLONG c = 0; c < 24 * 7; c++) {
        Gates.Auslastung[c] = 0;
    }

    // Alle Flugzeuge durchgehen:
    std::vector<flug_info_t> list;
    for (SLONG c = 0; c < Planes.AnzEntries(); c++) {
        if (Planes.IsInAlbum(c) == 0) {
            continue;
        }

        CFlugplan *Plan = &Planes[c].Flugplan;
        for (SLONG d = Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
            Planes[c].Flugplan.Flug[d].GateWarning = FALSE;

            if (Plan->Flug[d].ObjectType != 1 && Plan->Flug[d].ObjectType != 2) {
                continue;
            }

            // Ggf. Gate nehmen:
            if (Plan->Flug[d].VonCity != static_cast<ULONG>(Sim.HomeAirportId) && Plan->Flug[d].NachCity != static_cast<ULONG>(Sim.HomeAirportId)) {
                // Flug hat nichts mit Heimatflughafen zu tun:
                Plan->Flug[d].Gate = -2; // Null Problemo
                continue;
            }

            // Erst einmal mit Default-Wert initialisieren:
            if (Plan->Flug[d].Startdate > Sim.Date || (Plan->Flug[d].Startdate == Sim.Date && Plan->Flug[d].Startzeit > Sim.GetHour() + 1)) {
                Plan->Flug[d].Gate = -1; // Kein Gate frei
            }

            SLONG tmp = 0;
            if (Plan->Flug[d].VonCity == static_cast<ULONG>(Sim.HomeAirportId)) {
                tmp = Plan->Flug[d].Startzeit + (Plan->Flug[d].Startdate - Sim.Date) * 24;
            } else {
                tmp = Plan->Flug[d].Landezeit - 1 + (Plan->Flug[d].Landedate - Sim.Date) * 24;
            }

            if (tmp <= 1 || tmp >= 24 * 7 - 1) {
                continue;
            }

            // Abflug oder Ankunft?
            if (Plan->Flug[d].VonCity == ULONG(Sim.HomeAirportId)) {
                list.emplace_back(&Plan->Flug[d], tmp - 1, true);
            } else // Ankunft!
            {
                list.emplace_back(&Plan->Flug[d], tmp, false);
            }
        }
    }
    std::sort(list.begin(), list.end(), [](const flug_info_t &a, const flug_info_t &b) { return a.zeit < b.zeit; });

    std::vector<std::vector<bool>> auslastungProGate;
    auslastungProGate.resize(Gates.NumRented);
    for (auto &i : auslastungProGate) {
        i.resize(24 * 7, false);
    }

    for (auto &i : list) {
        bool found = false;
        for (SLONG n = 0; n < Gates.NumRented && !found; n++) {
            if (auslastungProGate[n][i.zeit] || (auslastungProGate[n][i.zeit + 1] && i.istAbflug)) {
                continue;
            }

            // Gate wird für Flug veranschlagt:
            i.flug->Gate = Gates.Gates[n].Nummer;

            auslastungProGate[n][i.zeit] = true;
            ++Gates.Auslastung[i.zeit];

            if (i.istAbflug) {
                auslastungProGate[n][i.zeit + 1] = true;
                ++Gates.Auslastung[i.zeit + 1];
            }

            found = true;
        }

        if (!found) {
            // Kein Gate mehr frei:
            i.flug->Gate = -1;

            Gates.Auslastung[i.zeit] = static_cast<UBYTE>(Gates.NumRented + 1);

            if (i.istAbflug) {
                Gates.Auslastung[i.zeit + 1] = static_cast<UBYTE>(Gates.NumRented + 1);
            }

            // Andere Flüge warnen:
            if (Owner != 0) {
                continue;
            }
            for (SLONG e = 0; e < Planes.AnzEntries(); e++) {
                if (Planes.IsInAlbum(e) == 0) {
                    continue;
                }

                CFlugplan &qPlan = Planes[e].Flugplan;
                for (SLONG f = qPlan.Flug.AnzEntries() - 1; f >= 0; f--) {
                    CFlugplanEintrag &qEintrag = qPlan.Flug[f];
                    if (qEintrag.ObjectType != 1 && qEintrag.ObjectType != 2) {
                        continue;
                    }
                    SLONG tmp = i.zeit;
                    if (i.istAbflug) {
                        if (qEintrag.VonCity == ULONG(Sim.HomeAirportId) && abs(qEintrag.Startzeit + (qEintrag.Startdate - Sim.Date) * 24 - tmp) < 2) {
                            qEintrag.GateWarning = TRUE;
                        } else if (qEintrag.NachCity == ULONG(Sim.HomeAirportId) && (qEintrag.Landezeit + (qEintrag.Landedate - Sim.Date) * 24 == tmp ||
                                                                                     qEintrag.Landezeit + (qEintrag.Landedate - Sim.Date) * 24 == tmp + 1)) {
                            qEintrag.GateWarning = TRUE;
                        }
                    } else {
                        if (qEintrag.VonCity == ULONG(Sim.HomeAirportId) && (qEintrag.Startzeit + (qEintrag.Startdate - Sim.Date) * 24 == tmp ||
                                                                             qEintrag.Startzeit + (qEintrag.Startdate - Sim.Date) * 24 == tmp + 1)) {
                            qEintrag.GateWarning = TRUE;
                        } else if (qEintrag.NachCity == ULONG(Sim.HomeAirportId) && qEintrag.Landezeit - 1 + (qEintrag.Landedate - Sim.Date) * 24 == tmp) {
                            qEintrag.GateWarning = TRUE;
                        }
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Läßt einen Spieler zu einer Rune laufen: rc=FALSE, wenn Raum geschlossen
//--------------------------------------------------------------------------------------------
BOOL PLAYER::WalkToRoom(UBYTE RoomId) {
    if (PlayerNum == Sim.localPlayer) {
        bgWarp = FALSE;
    }

    if (RunningToToilet != 0) {
        return (TRUE);
    }
    if (IsOut != 0) {
        return (FALSE);
    }

    // Schauen, ob wir zu dem Raum gehen können, oder ob zu ist:
    SLONG MenuClosed = -1;
    if (RoomId == ROOM_SHOP1 && (SLONG(Sim.Time) < timeDutyOpen || ((Sim.Weekday == 5 || Sim.Weekday == 6) && SLONG(Sim.Time) > timeDutyClose))) {
        MenuClosed = MENU_CLOSED_DUTYFREE;
    } else if (RoomId == ROOM_ARAB_AIR && (SLONG(Sim.Time) < timeArabOpen || Sim.Weekday == 6)) {
        MenuClosed = MENU_CLOSED_ARAB_AIR;
    } else if (RoomId == ROOM_LAST_MINUTE && (SLONG(Sim.Time) > timeLastClose || Sim.Weekday == 5)) {
        MenuClosed = MENU_CLOSED_LAST_MINUTE;
    } else if (RoomId == ROOM_MUSEUM && (SLONG(Sim.Time) < timeMuseOpen || Sim.Weekday == 5 || Sim.Weekday == 6)) {
        MenuClosed = MENU_CLOSED_MUSEUM;
    } else if (RoomId == ROOM_REISEBUERO && SLONG(Sim.Time) > timeReisClose) {
        MenuClosed = MENU_CLOSED_REISEBUERO;
    } else if (RoomId == ROOM_MAKLER && SLONG(Sim.Time) > timeMaklClose) {
        MenuClosed = MENU_CLOSED_MAKLER;
    } else if (RoomId == ROOM_WERBUNG && (Sim.Difficulty >= DIFF_NORMAL || Sim.Difficulty == DIFF_FREEGAME) &&
               (SLONG(Sim.Time) < timeWerbOpen || Sim.Weekday == 5 || Sim.Weekday == 6)) {
        MenuClosed = MENU_CLOSED_WERBUNG;
    } else if (RoomId == ROOM_SECURITY && Sim.nSecOutDays > 0) {
        MenuClosed = MENU_CLOSED_NOSECURITY;
    }
    if (MenuClosed != -1) {
        if (Owner == 0 && (LocationWin != nullptr) && ((*LocationWin).MenuIsOpen() == 0) && ((*LocationWin).IsDialogOpen() == 0)) {
            (LocationWin)->MenuStart(MENU_CLOSED, MenuClosed);
            (LocationWin)->MenuSetZoomStuff(XY(320, 220), 0, FALSE);
        }

        return (FALSE);
    }

    if (RoomId == ROOM_BURO_A || RoomId == ROOM_BURO_B || RoomId == ROOM_BURO_C || RoomId == ROOM_BURO_D) {
        if (Sim.Players.Players[(RoomId - ROOM_BURO_A) / 10].OfficeState == 2) {
            if (Owner == 0 && (LocationWin != nullptr) && ((*LocationWin).MenuIsOpen() == 0) && ((*LocationWin).IsDialogOpen() == 0)) {
                if (Sim.ItemKohle != 0) {
                    BuyItem(ITEM_GLKOHLE);
                    if (HasItem(ITEM_GLKOHLE) != 0) {
                        Sim.ItemKohle = 0;
                        SIM::SendSimpleMessage(ATNET_TAKETHING, 0, ITEM_KOHLE);
                    }
                }

                (LocationWin)->MenuStart(MENU_REQUEST, MENU_REQUEST_DESTROYED);
                (LocationWin)->MenuSetZoomStuff(XY(320, 220), 0, FALSE);
            }
        }
    }

    PrimaryTarget = Airport.GetRandomTypedRune(RUNE_2SHOP, RoomId);

    /* always calculate SpeedCount, because Sim.CallItADay might be set any moment */
    {
        PERSON &qPerson = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)];

        SpeedCount = abs(qPerson.Position.x - PrimaryTarget.x);
        SpeedCount += abs(qPerson.Position.y - PrimaryTarget.y);

        if (abs(qPerson.Position.y - PrimaryTarget.y) > 4600) {
            SpeedCount -= 4600;
        }
        SpeedCount = max(1, SpeedCount);
    }

    PrimaryTarget.x /= 44;

    WaitForRoom = 0;
    DirectToRoom = RoomId;

    if (PrimaryTarget.y < 4000) {
        PrimaryTarget.y = PrimaryTarget.y / 22 + 5; // unten!
    } else {
        PrimaryTarget.y = (PrimaryTarget.y - 5000 + 2200) / 22 - 100; // oben!
    }

    UpdateWaypoints();
    BroadcastPosition();

    /* if we already are in the destination room */
    for (SLONG c = 0; c < 10; c++) {
        if ((Locations[c] & (~(ROOM_ENTERING | ROOM_LEAVING))) == RoomId) {
            Locations[c] |= (ROOM_ENTERING);
            Locations[c] &= (~ROOM_LEAVING);

            SpeedCount = 1;
            return (TRUE);
        }
    }

    return (TRUE);
}

//--------------------------------------------------------------------------------------------
// Läßt den Spieler zu einer bestimmten, angeklickten Stelle laufen:
//--------------------------------------------------------------------------------------------
void PLAYER::WalkToMouseClick(XY AirportClickPos) {
    SLONG Room = Airport.IsInMarkedArea(AirportClickPos);

    if (RunningToToilet != 0) {
        return;
    }
    if (PlayerNum == Sim.localPlayer) {
        bgWarp = FALSE;
    }

    if (Room == ROOM_WALL) {
        // Bei Klick auf die Wand eines Büros davor stehenbleiben
        if (AirportClickPos.y < 150) {
            AirportClickPos.y = 138;
        } else {
            AirportClickPos.y = 221 + 66;
        }
        Room = 0;
    }

    if (Sim.Date > 0) {
        if (Room == ROOM_BURO_A || Room == ROOM_BURO_B || Room == ROOM_BURO_C || Room == ROOM_BURO_D) {
            Sim.GiveHint(HINT_BUERO);
        }
        if (Room == ROOM_PERSONAL_A || Room == ROOM_PERSONAL_B || Room == ROOM_PERSONAL_C || Room == ROOM_PERSONAL_D) {
            Sim.GiveHint(HINT_PERSONALBUERO);
        }
        if (Room == ROOM_SHOP1) {
            Sim.GiveHint(HINT_DUTYFREE);
        }
        if (Room == ROOM_REISEBUERO) {
            Sim.GiveHint(HINT_AIRTRAVEL);
        }
        if (Room == ROOM_LAST_MINUTE) {
            Sim.GiveHint(HINT_LASTMINUTE);
        }
    }
    if (Sim.Date > 2) {
        if (Room == ROOM_KIOSK) {
            Sim.GiveHint(HINT_KIOSK);
        }
        if (Room == ROOM_ROUTEBOX) {
            Sim.GiveHint(HINT_ROUTEBOX);
        }
        if (Room == ROOM_WERKSTATT) {
            Sim.GiveHint(HINT_LASTMINUTE);
        }
        if (Room == ROOM_MUSEUM) {
            Sim.GiveHint(HINT_MUSEUM);
        }
        if (Room == ROOM_FRACHT) {
            Sim.GiveHint(HINT_FRACHT);
        }
    }
    if (Sim.Date > 4) {
        if (Room == ROOM_INSEL) {
            Sim.GiveHint(HINT_FERNGLASS);
        }
        if (Room == ROOM_RUSHMORE) {
            Sim.GiveHint(HINT_FERNGLASS);
        }
        if (Room == ROOM_WELTALL) {
            Sim.GiveHint(HINT_FERNGLASS);
        }
        if (Room == ROOM_FRACHT) {
            Sim.GiveHint(HINT_FRACHT);
        }
    }

    if ((Room != 0) && (Room < ROOM_FLIGHTDISPLAY || Room > ROOM_FLIGHTDISPLAY + 59)) {
        WalkToRoom(UBYTE(Room));
    } else {
        // Toleranzbereiche:
        if (AirportClickPos.y > 170 && AirportClickPos.y <= 220) {
            AirportClickPos.y = 221;
        }
        if (AirportClickPos.y > -20 && AirportClickPos.y <= 93) {
            AirportClickPos.y = 94;
        }

        WaitForRoom = 0;
        DirectToRoom = 0;

        // Klick nach unten bzw oben:
        if (AirportClickPos.y > 220 && AirportClickPos.y < 440) {
            PrimaryTarget.y = AirportClickPos.y - 220;
            PrimaryTarget.x = AirportClickPos.x + PrimaryTarget.y / 2 - 86;

            PrimaryTarget.x = (PrimaryTarget.x + 4400) / 44 - 100;
            PrimaryTarget.y = (PrimaryTarget.y + 2200) / 22 - 100 + 5;
        } else if (AirportClickPos.y > 93 && AirportClickPos.y < 170) {
            PrimaryTarget.y = AirportClickPos.y - 93;
            PrimaryTarget.x = AirportClickPos.x + PrimaryTarget.y / 2 - 88;

            PrimaryTarget.x = (PrimaryTarget.x + 4400) / 44 - 100;
            PrimaryTarget.y = (PrimaryTarget.y + 2200) / 22 - 100;
        }

        // Test: Ist Zielkachel blockiert? Dann evtl. südlich davon!
        while (PrimaryTarget.y != 3 && PrimaryTarget.y != 15) {
            if (((Airport.iPlate[PrimaryTarget.y + (PrimaryTarget.x << 4)] & 4) != 0) &&
                ((Airport.iPlate[PrimaryTarget.y + (PrimaryTarget.x << 4)] & 0xf0) != 0)) {
                break;
            }
            PrimaryTarget.y++;

            if (PrimaryTarget.y > 15) {
                // Nicht möglich.
                WalkStop();
                return;
            }
        }

        if (PrimaryTarget.y == 3 || PrimaryTarget.y == 15) {
            // Nicht möglich.
            WalkStop();
            return;
        }

        // Weiterer Test: Ist Ziel oben in der Luft?
        if (PrimaryTarget.y < 3) {
            SLONG y = PrimaryTarget.y;

            while (y < 3 && ((Airport.iPlate[y + (PrimaryTarget.x << 4)] & 32) != 0)) {
                y++;
            }

            // Ziel in der Luft wenn y==3:
            if (y == 3) {
                WalkStop();
                return;
            }
        }

        UpdateWaypoints();
        BroadcastPosition();
    }
}

//--------------------------------------------------------------------------------------------
// Läßt den Spieler zu einer bestimmten Stelle laufen:
//--------------------------------------------------------------------------------------------
void PLAYER::WalkToPlate(XY Plate) {
    if (RunningToToilet != 0) {
        return;
    }

    PrimaryTarget = Plate;

    if (PlayerNum == Sim.localPlayer) {
        bgWarp = FALSE;
    }

    UpdateWaypoints();
    BroadcastPosition();
}

//--------------------------------------------------------------------------------------------
// Der Spieler läuft nicht mehr automatisch:
//--------------------------------------------------------------------------------------------
void PLAYER::WalkStop() { iWalkActive = FALSE; }

//--------------------------------------------------------------------------------------------
// Spieler bleibt wirklich stehen
//--------------------------------------------------------------------------------------------
void PLAYER::WalkStopEx() {
    if (RunningToToilet != 0) {
        return;
    }

    PERSON &qPerson = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)];

    if (qPerson.StatePar == 0 && IsStuck == 0) {
        qPerson.Target = qPerson.Position;

        TertiaryTarget.x = (qPerson.Target.x - 22) / 44;

        if (qPerson.Target.y > 5000) {
            TertiaryTarget.y = (qPerson.Target.y - 11 - 5000) / 22;
        } else {
            TertiaryTarget.y = (qPerson.Target.y - 11) / 22 + 5;
        }

        PrimaryTarget = SecondaryTarget = TertiaryTarget;

        DirectToRoom = 0;
        iWalkActive = FALSE;
        NewDir = 8;

        UpdateWaypoints();

        if (PlayerNum == Sim.localPlayer) {
            TEAKFILE Message;
            Message.Announce(128);
            Message << ATNET_PLAYERSTOP << PlayerNum;
            SIM::SendMemFile(Message);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Berechnet das Sekundärziel für den Spieler:
//--------------------------------------------------------------------------------------------
void PLAYER::UpdateWaypoints() {
    SLONG cx = 0;
    SLONG cy = 0;
    SLONG xs = 0;
    SLONG refy = 0;

    iWalkActive = TRUE;

    // Default:
    SecondaryTarget = PrimaryTarget;

    // In einen Engen Bereich (z.B. Raum geklickt?) Dann Eingang suchen:
    for (cx = xs = 0; cx < 4; cx++) {
        if ((Airport.iPlate[PrimaryTarget.y + ((PrimaryTarget.x + cx) << 4)] & 64) != 0) {
            xs++;
        } else {
            break;
        }
    }

    for (cx = 0; cx < 4; cx++) {
        if ((Airport.iPlate[PrimaryTarget.y + ((PrimaryTarget.x - cx) << 4)] & 16) != 0) {
            xs++;
        } else {
            break;
        }
    }

    // Wie gesagt, nur wenn's eng ist:
    if (xs < 4) {
        for (refy = 0; refy < 3; refy++) {
            if ((Airport.iPlate[PrimaryTarget.y + refy + ((PrimaryTarget.x) << 4)] & 32) == 0) {
                break;
            }
        }

        // Und nur wenn Y-Platznot herrscht:
        if (refy < 3) {
            for (cx = 1; cx < 3; cx++) {
                if ((Airport.iPlate[PrimaryTarget.y + ((PrimaryTarget.x + cx) << 4)] & 16) != 0) {
                    for (cy = 0; cy < 3; cy++) {
                        if ((Airport.iPlate[PrimaryTarget.y + cy + ((PrimaryTarget.x + cx) << 4)] & 32) == 0) {
                            break;
                        }
                    }

                    if (cy > refy) {
                        SecondaryTarget.x += cx;
                        refy = cy;
                    }
                } else {
                    break;
                }
            }

            for (cx = 1; cx < 3; cx++) {
                if ((Airport.iPlate[PrimaryTarget.y + ((PrimaryTarget.x - cx) << 4)] & 64) != 0) {
                    for (cy = 0; cy < 3; cy++) {
                        if ((Airport.iPlate[PrimaryTarget.y + cy + ((PrimaryTarget.x - cx) << 4)] & 32) == 0) {
                            break;
                        }
                    }

                    if (cy > refy) {
                        SecondaryTarget.x -= cx;
                        refy = cy;
                    }
                } else {
                    break;
                }
            }
        }
    }

    // Ist Spieler oben und will nach unten (oder umgekeht)?
    if (SecondaryTarget.y < 5 && Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Position.y < 5000) {
        TertiaryTarget = Airport.GetBestStairs(ROOM_STAIRS1UP, PrimaryTarget.x * 44, Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Position.x);
        TertiaryTarget.x /= 44;
        TertiaryTarget.y /= 22;
        TertiaryTarget.y += 5;
    } else if (DirectToRoom != 123 && SecondaryTarget.y >= 5 && Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Position.y >= 5000) {
        TertiaryTarget = Airport.GetBestStairs(ROOM_STAIRS1DOWN, PrimaryTarget.x * 44, Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Position.x);
        TertiaryTarget.y -= 5000;
        TertiaryTarget.x /= 44;
        TertiaryTarget.y /= 22;
    } else {
        TertiaryTarget = SecondaryTarget;
    }

    if (Sim.CallItADay == 0) {
        PERSON &qPerson = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)];

        if (qPerson.StatePar == 0 && IsStuck == 0) {
            qPerson.Target = TertiaryTarget;
            qPerson.Target.x = qPerson.Target.x * 44 + 22;

            if (qPerson.Target.y < 5) {
                qPerson.Target.y = qPerson.Target.y * 22 + 5000 + 11;
            } else {
                qPerson.Target.y = (qPerson.Target.y - 5) * 22 + 11;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Läßt den Spieler in die richtige Richtung laufen:
//--------------------------------------------------------------------------------------------
void PLAYER::UpdateWaypointWalkingDirection() {
    if (IsOut != 0) {
        return;
    }

    if (iWalkActive != 0) {
        XY p = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Position;
        XY ArrayPos;

        if (p.y > 0 && p.y < 5000) {
            ArrayPos.x = (p.x + 4400) / 44 - 100 /*+5*/;
            ArrayPos.y = (p.y + 2200) / 22 - 100 + 5;
        } else {
            p.y -= 5000;
            ArrayPos.x = (p.x + 4400) / 44 - 100;
            ArrayPos.y = (p.y + 2200) / 22 - 100;
        }

        if (ArrayPos.y >= 15) {
            NewDir = 0;
        } else if (ArrayPos.x > TertiaryTarget.x) {
            NewDir = 6;
        } else if (ArrayPos.x < TertiaryTarget.x) {
            NewDir = 2;
        } else if (ArrayPos.y > TertiaryTarget.y) {
            NewDir = 0;
        } else if (ArrayPos.y < TertiaryTarget.y) {
            NewDir = 4;
        } else {
            if (TertiaryTarget != SecondaryTarget) {
                TertiaryTarget = SecondaryTarget;
                UpdateWaypointWalkingDirection();
            } else if (PrimaryTarget != SecondaryTarget) {
                TertiaryTarget = SecondaryTarget = PrimaryTarget;
                UpdateWaypointWalkingDirection();
            } else {
                NewDir = 8;
                WalkStop();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erledigt Dinge für den Roboter:
//--------------------------------------------------------------------------------------------
void PLAYER::RobotPump() {
    SLONG c = 0;
    PERSON *pPerson = nullptr;

    if ((Sim.bNetwork != 0) && WaitWorkTill != -1) {
        if (WaitWorkTill <= Sim.TimeSlice) {
            RobotExecuteAction();
        } else if (WaitWorkTill == 0 && Sim.TimeSlice > 0) {
            WaitWorkTill = -1;
        } else if (Sim.TimeSlice < WaitWorkTill) {
            return;
        }
    }

    if (Owner == 0 && GetRoom() == ROOM_AIRPORT && (LocationWin != nullptr) && ((LocationWin)->IsDialogOpen() == 0) && ((LocationWin)->MenuIsOpen() == 0) &&
        Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].LookDir == 8) {
        StandCount++;

        if (pPerson == nullptr) {
            pPerson = &Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)];
        }
        if (IsStuck == 0 && StandCount >= 20 * 20 &&
            (pPerson->Position.y / 22 + 5 != 13 || (Airport.iPlate[(pPerson->Position.y / 22 + 5) + ((pPerson->Position.x / 44) << 4)] & 240) != 80)) {
            pPerson->State = pPerson->State & ~PERSON_WAITFLAG;
            pPerson->LookDir = 9; // Gimmick starten
            pPerson->Phase = 0;

            NewDir = 8;
            pPerson->Target = pPerson->Position;

            SIM::SendSimpleMessage(ATNET_GIMMICK, 0, PlayerNum, 1);
            StandCount = 0;
        }
    } else {
        StandCount = 0;
    }

    if (pSmack != nullptr) {
        if (pSmack->Next(&Bricks[SLONG(0x10000000 + 734 + PlayerNum)].Bitmap[0]) == 0) {
            delete pSmack;
            pSmack = nullptr;
            AIRPORT::UpdateStaticDoorImage();
            Sim.FocusPerson = -1;

            if (RobotActions[0].ActionId == ACTION_NONE && RobotActions[1].ActionId == ACTION_NONE) {
                RobotPlan();
            }
            LastActionId = 0;
            RobotActions[0].ActionId = ACTION_NONE;

            // Alle Figuren in der Nähe der Tür "entminen"
            for (SLONG c = 0; c < SLONG(Airport.Runes.AnzEntries()); c++) {
                if (Airport.Runes[c].BrickId == 0x10000000 + RUNE_2SHOP && Airport.Runes[c].Par == ROOM_BURO_A + PlayerNum * 10) {
                    for (SLONG d = Sim.Persons.AnzEntries() - 1; d >= 0; d--) {
                        if ((Sim.Persons.IsInAlbum(d) != 0) && Clans[static_cast<SLONG>(Sim.Persons[d].ClanId)].Type >= CLAN_PLAYER1 &&
                            Clans[static_cast<SLONG>(Sim.Persons[d].ClanId)].Type <= CLAN_PLAYER4) {
                            if ((Airport.Runes[c].ScreenPos - XY(-100, 4900) - Sim.Persons[d].ScreenPos).abs() < 100) {
                                if (Sim.Persons[d].LookDir == 4) {
                                    PLAYER &qOpfer = Sim.Players.Players[static_cast<SLONG>(Sim.Persons[d].State)];

                                    if (qOpfer.Owner != 2) {
                                        Sim.Persons[d].Running = FALSE;
                                        Sim.Persons[d].LookDir = 0;

                                        qOpfer.PlayerSmoking = 99;
                                        qOpfer.BroadcastPosition();
                                        PLAYER::NetSynchronizeFlags();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ((SickTokay != 0) && (RunningToToilet == 0) && (IsStuck == 0)) {
        if ((Sim.TimeSlice % 127) == 0 && Sim.GetMinute() > 0 && (Sim.CallItADay == 0) && Sim.GetHour() >= 9 && Sim.GetHour() < 18) {
            if (LocationWin == nullptr || ((LocationWin)->MenuIsOpen() == 0)) {
                PlayerExtraRandom.SRand(Sim.TimeSlice);
                if (PlayerExtraRandom.Rand(4) == 0) {
                    if (PlayerNum == 0) {
                        WalkToRoom(ROOM_WC_F);
                    } else {
                        WalkToRoom(ROOM_WC_M);
                    }

                    if (pPerson == nullptr) {
                        pPerson = &Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)];
                    }

                    // Nur wenn Spieler nicht gerade Raum betritt oder verläßt und auch nicht auf Toilette ist:
                    if (pPerson->StatePar == 0 && GetRoom() != ROOM_WC_M && GetRoom() != ROOM_WC_F) {
                        pPerson->Running = TRUE;
                        LeaveAllRooms();
                        RunningToToilet = TRUE;
                        Stunned = 20;

                        pPerson->Mood = static_cast<UBYTE>(MoodPersonToilet);
                        pPerson->MoodCountdown = max(MOODCOUNT_START - 16, pPerson->MoodCountdown);
                    }
                }
            }
        }
    }

    if (Stunned != 0) {
        Stunned--;
        if (Stunned == 0) {
            if (RunningToToilet == 0) {
                if (Owner == 1) {
                    if (RobotActions[0].ActionId == ACTION_NONE && RobotActions[1].ActionId == ACTION_NONE) {
                        RobotPlan();
                    }
                    LastActionId = 0;
                    RobotActions[0].ActionId = ACTION_NONE;
                    Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Running = FALSE;
                }
            } else {
                Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Running = TRUE;
            }

            BroadcastPosition();
        }

        return;
    }

    if (Owner != 1 || (Editor != 0)) {
        return; // War Irtum, kein Computerspieler
    }
    if ((Sim.bNetwork != 0) && (Sim.bIsHost == 0)) {
        return; // Im Netzwerk bestimmt der Host, wie die Computerspieler sich verhalten
    }

    // Ist Computerspieler ohne Ziel?
    if ((iWalkActive == FALSE && GetRoom() == ROOM_AIRPORT && (IsTalking == 0)) || (SpeedCount == 0 && (Sim.CallItADay != 0))) {
        if (StandStillSince > 10) {
            RobotActions[0].ActionId = ACTION_NONE;
            RobotPlan();
        } else {
            if (WorkCountdown <= 0) {
                if (Sim.CallItADay != 0) {
                    StandStillSince++;
                } else if (Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].StatePar == 0) {
                    StandStillSince++;
                }
            }
        }
    } else {
        StandStillSince = 0;
    }

    if (RobotActions[0].ActionId != ACTION_NONE && GetRoom() == ROOM_AIRPORT) {
        return;
    }

    if (WorkCountdown > 0) {
        if (Editor == 0) {
            WorkCountdown--;
        }
        return;
    }

    if (RobotActions[0].ActionId == ACTION_NONE && RobotActions[1].ActionId != ACTION_NONE) {
        // Alles in der Queue einen weiterschieben
        for (c = 0; c < RobotActions.AnzEntries() - 1; c++) {
            RobotActions[c] = RobotActions[c + 1];
        }

        RobotActions[RobotActions.AnzEntries() - 1].ActionId = ACTION_NONE;

        if (GetRoom() != ROOM_AIRPORT) {
            LeaveRoom();
        }

        // Aufbewahren, damit wir nicht 2x das gleiche hintereinander machen:
        SLONG LastLastActionId = LastActionId;
        LastActionId = RobotActions[0].ActionId;

        BOOL rc = 0;
        switch (RobotActions[0].ActionId) {
        case ACTION_STARTDAY:
            rc = WalkToRoom(UBYTE(PlayerNum * 10 + 10));
            break;

        case ACTION_PERSONAL:
            rc = WalkToRoom(UBYTE(PlayerNum * 10 + 11));
            break;

            // Default-Visitroutinen
        case ACTION_VISITBANK:
            rc = WalkToRoom(ROOM_BANK);
            break;
        case ACTION_VISITARAB:
            rc = WalkToRoom(ROOM_ARAB_AIR);
            break;
        case ACTION_SABOTAGE:
            rc = WalkToRoom(ROOM_SABOTAGE);
            break;
        case ACTION_VISITKIOSK:
            rc = WalkToRoom(ROOM_KIOSK);
            break;
        case ACTION_VISITMECH:
            rc = WalkToRoom(ROOM_WERKSTATT);
            break;
        case ACTION_VISITMUSEUM:
            rc = WalkToRoom(ROOM_MUSEUM);
            break;
        case ACTION_VISITDUTYFREE:
            rc = WalkToRoom(ROOM_SHOP1);
            break;
        case ACTION_VISITAUFSICHT:
            rc = WalkToRoom(ROOM_AUFSICHT);
            break;
        case ACTION_BUYUSEDPLANE:
            rc = WalkToRoom(ROOM_MUSEUM);
            break;
        case ACTION_EMITSHARES:
            rc = WalkToRoom(ROOM_BANK);
            break;
        case ACTION_WERBUNG:
            rc = WalkToRoom(ROOM_WERBUNG);
            break;
        case ACTION_VISITNASA:
            rc = WalkToRoom(ROOM_NASA);
            break;
        case ACTION_VISITMAKLER:
            rc = WalkToRoom(ROOM_MAKLER);
            break;
        case ACTION_VISITRICK:
            rc = WalkToRoom(ROOM_RICKS);
            break;
        case ACTION_VISITROUTEBOX:
            rc = WalkToRoom(ROOM_ROUTEBOX);
            break;
        case ACTION_VISITSECURITY:
            rc = WalkToRoom(ROOM_SECURITY);
            break;
        case ACTION_VISITSECURITY2:
            rc = WalkToRoom(ROOM_SECURITY);
            break;
        case ACTION_CHECKAGENT1:
            rc = WalkToRoom(ROOM_LAST_MINUTE);
            break;
        case ACTION_CHECKAGENT2:
            rc = WalkToRoom(ROOM_REISEBUERO);
            break;
        case ACTION_CHECKAGENT3:
            rc = WalkToRoom(ROOM_FRACHT);
            break;
        case ACTION_VISITDESIGNER:
            rc = WalkToRoom(ROOM_DESIGNER);
            break;
        case ACTION_VISITTELESCOPE:
            if (Sim.Difficulty == DIFF_FINAL) {
                rc = WalkToRoom(ROOM_INSEL);
            } else {
                rc = WalkToRoom(ROOM_RUSHMORE);
            }
            break;
        default:
            DebugBreak();
        }

        SpeedCount = std::max(1, SpeedCount / WalkSpeed);
        if (RobotUse(ROBOT_USE_ALLRUN) && Sim.GetMinute() > 0) {
            Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Running = TRUE;
            SpeedCount = std::max(1, SpeedCount / 2);
            BroadcastPosition();
        } else {
            Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].Running = FALSE;
        }

        if (rc == 0) {
            if (RobotActions[0].ActionId == ACTION_NONE && RobotActions[1].ActionId == ACTION_NONE) {
                RobotPlan();
            }
            LastActionId = LastLastActionId;
            RobotActions[0].ActionId = ACTION_NONE;

            return;
        }
    }

    if (RobotActions[0].ActionId == ACTION_NONE && RobotActions[1].ActionId == ACTION_NONE) {
        RobotPlan();
    }
}

//--------------------------------------------------------------------------------------------
// Initialisiert den Computerspieler morgens:
//--------------------------------------------------------------------------------------------
void PLAYER::RobotInit() {
    SLONG c = 0;

    WorkCountdown = 10 + PlayerNum * 10;

    for (c = 0; c < 5; c++) {
        RobotActions[c].ActionId = ACTION_NONE;
    }

    WaitWorkTill = -1;
    RobotActions[1].ActionId = ACTION_STARTDAY;
    RobotActions[2].ActionId = ACTION_PERSONAL;

    if (DoRoutes == 0) {
        if (GlobalUse(USE_TRAVELHOLDING)) {
            if (((PlayerNum + Sim.Date) & 1) != 0) {
                RobotActions[3].ActionId = ACTION_CHECKAGENT1;
                RobotActions[4].ActionId = ACTION_CHECKAGENT2;

                if (RobotUse(ROBOT_USE_MUCH_FRACHT) || (((PlayerNum + Sim.Date) & 3) == 0 && RobotUse(ROBOT_USE_FRACHT))) {
                    RobotActions[2].ActionId = ACTION_CHECKAGENT3;
                }
            } else {
                RobotActions[4].ActionId = ACTION_CHECKAGENT1;
                RobotActions[3].ActionId = ACTION_CHECKAGENT2;

                if (RobotUse(ROBOT_USE_RUN_FRACHT)) {
                    RobotActions[1].ActionId = ACTION_CHECKAGENT3;
                } else if (RobotUse(ROBOT_USE_MUCH_FRACHT) || (((PlayerNum + Sim.Date) & 3) == 0 && RobotUse(ROBOT_USE_FRACHT))) {
                    RobotActions[2].ActionId = ACTION_CHECKAGENT3;
                }
            }
        } else if (RobotUse(ROBOT_USE_FRACHT)) {
            RobotActions[3].ActionId = ACTION_CHECKAGENT3;
        }
    }

    TimeBuro = TimePersonal = TimeAufsicht = TimeReiseburo = -1;
    RobotPlan();
}

//--------------------------------------------------------------------------------------------
// Strategie-Modell des Computerspielers:
//--------------------------------------------------------------------------------------------
void PLAYER::RobotPlan() {
    SLONG DefaultActions[] = {ACTION_VISITARAB,     ACTION_VISITMAKLER, ACTION_SABOTAGE,      ACTION_SABOTAGE,      ACTION_BUERO,          ACTION_BUERO,
                              ACTION_VISITBANK,     ACTION_VISITBANK,   ACTION_PERSONAL,      ACTION_PERSONAL,      ACTION_VISITKIOSK,     ACTION_VISITMECH,
                              ACTION_VISITMECH,     ACTION_VISITMUSEUM, ACTION_CHECKAGENT1,   ACTION_CHECKAGENT2,   ACTION_CHECKAGENT3,    ACTION_CHECKAGENT3,
                              ACTION_VISITDUTYFREE, ACTION_WERBUNG,     ACTION_WERBUNG,       ACTION_VISITAUFSICHT, ACTION_VISITTELESCOPE, ACTION_VISITRICK,
                              ACTION_VISITROUTEBOX, ACTION_VISITNASA,   ACTION_VISITSECURITY, ACTION_VISITSECURITY, ACTION_VISITSECURITY,  ACTION_VISITDESIGNER,
                              ACTION_VISITDESIGNER};

    if (Owner != 1 || (IsOut != 0)) {
        return; // War Irtum, kein Computerspieler
    }

    if (RobotActions[0].ActionId == ACTION_NONE && RobotActions[1].ActionId == ACTION_NONE) {
        // Zeit für default-Actions?
        if (Money > 5000000 && Image > -100 && (SavesForRocket == 0) && SLONG(Sim.Time) > timeMuseOpen && Sim.Weekday != 5 && Sim.Weekday != 6 &&
            (bWasInMuseumToday == 0)) {
            RobotActions[1].ActionId = ACTION_BUYUSEDPLANE;
        } else if (RobotUse(ROBOT_USE_GOODPLANES) && Sim.Date % 4 == PlayerNum && Sim.GetHour() == 9 && Sim.GetMinute() <= 35) {
            RobotActions[1].ActionId = ACTION_VISITMECH;
        } else if ((WantToDoRoutes != 0) && Image < 150 && PlayerWalkRandom.Rand(4) == 0) {
            RobotActions[1].ActionId = ACTION_WERBUNG;
        } else if ((BuyBigPlane != 0) && Money > 100000 && PlayerWalkRandom.Rand(4) == 0 && !RobotUse(ROBOT_USE_GROSSESKONTO)) {
            RobotActions[1].ActionId = ACTION_VISITMUSEUM;
        } else if (Sim.Date + 1 >= PlayerNum && (MaxAktien - AnzAktien) >= 25000 &&
                   (Money < 3000000 || Credit > 1000000 || RobotUse(ROBOT_USE_EMITMUCHSHARES))) {
            RobotActions[1].ActionId = ACTION_EMITSHARES;
        } else if (Sim.Time - TimePersonal > 3 * 60000) {
            RobotActions[1].ActionId = ACTION_PERSONAL;
        } else if (Sim.Time - TimeAufsicht > 3 * 60000 || (TimeAufsicht - Sim.Time > 1 * 60000 && Sim.Time > 16 * 60000 && RobotUse(ROBOT_USE_AUFSICHT))) {
            RobotActions[1].ActionId = ACTION_VISITAUFSICHT;
        } else if (Sim.Time - TimeReiseburo > 90000 && (DoRoutes == 0)) {
            RobotActions[1].ActionId = ACTION_CHECKAGENT1;
            RobotActions[2].ActionId = ACTION_CHECKAGENT2;
        } else if (TimeBuro - Sim.Time > 3 * 60000 && OfficeState != 2) {
            RobotActions[1].ActionId = ACTION_BUERO;
        } else {
        again:

            do {
                RobotActions[1].ActionId = DefaultActions[PlayerWalkRandom.Rand(sizeof(DefaultActions) / sizeof(DefaultActions[0]))];

                if (RobotActions[1].ActionId == ACTION_VISITNASA && !RobotUse(ROBOT_USE_NASA)) {
                    goto again;
                }

                if (RobotUse(ROBOT_USE_OFTENMECH) && (RobotActions[1].ActionId == ACTION_VISITTELESCOPE || RobotActions[1].ActionId == ACTION_VISITRICK ||
                                                      RobotActions[1].ActionId == ACTION_VISITKIOSK)) {
                    RobotActions[1].ActionId = ACTION_VISITMECH;
                }
                if (RobotUse(ROBOT_USE_EXTREME_SABOTAGE) && (RobotActions[1].ActionId == ACTION_VISITTELESCOPE ||
                                                             RobotActions[1].ActionId == ACTION_VISITRICK || RobotActions[1].ActionId == ACTION_VISITKIOSK)) {
                    RobotActions[1].ActionId = ACTION_SABOTAGE;
                }

                if (RobotUse(ROBOT_USE_NOCHITCHAT) && (RobotActions[1].ActionId == ACTION_VISITTELESCOPE || RobotActions[1].ActionId == ACTION_VISITRICK ||
                                                       RobotActions[1].ActionId == ACTION_VISITKIOSK)) {
                    goto again;
                }

                if (RobotUse(ROBOT_USE_EXTRA_SABOTAGE) && Sim.Date > 1 && PlayerWalkRandom.Rand(4) == 0) {
                    RobotActions[1].ActionId = ACTION_SABOTAGE;
                }
                if (RobotUse(ROBOT_USE_EXTRA_BANK) && Sim.Date > 1 && PlayerWalkRandom.Rand(3) == 0) {
                    RobotActions[1].ActionId = ACTION_VISITBANK;
                }

                // Nicht ins Museum wenn wir schon genug Flugzeuge haben:
                if (RobotActions[1].ActionId == ACTION_VISITMUSEUM && RobotUse(ROBOT_USE_MAX4PLANES) && Planes.GetNumUsed() >= 4) {
                    goto again;
                }
                if (RobotActions[1].ActionId == ACTION_VISITMUSEUM && RobotUse(ROBOT_USE_MAX5PLANES) && Planes.GetNumUsed() >= 5) {
                    goto again;
                }
                if (RobotActions[1].ActionId == ACTION_VISITMUSEUM && RobotUse(ROBOT_USE_MAX10PLANES) && Planes.GetNumUsed() >= 10) {
                    goto again;
                }

                // Nicht zum Makler gehen, wenn das in diesem Level nicht sein soll:
                if (RobotActions[1].ActionId == ACTION_VISITMAKLER && !RobotUse(ROBOT_USE_MAKLER)) {
                    goto again;
                }

                // Nicht zu PetrolAir gehen, wenn das in diesem Level nicht sein soll:
                if (RobotActions[1].ActionId == ACTION_VISITARAB && !RobotUse(ROBOT_USE_PETROLAIR)) {
                    goto again;
                }
            } while (LastActionId == RobotActions[1].ActionId);

            if (RobotActions[1].ActionId == ACTION_BUERO && LaptopQuality > 2) {
                goto again;
            }
            if ((RobotActions[1].ActionId == ACTION_CHECKAGENT1 || RobotActions[1].ActionId == ACTION_CHECKAGENT2) && (DoRoutes != 0) &&
                (HasFlownRoutes != 0) && (Sim.Time & 15) != 0) {
                goto again;
            }
            if ((RobotActions[1].ActionId == ACTION_CHECKAGENT1 || RobotActions[1].ActionId == ACTION_CHECKAGENT2) && !GlobalUse(USE_TRAVELHOLDING)) {
                goto again;
            }

            if (RobotActions[1].ActionId == ACTION_CHECKAGENT3 && !RobotUse(ROBOT_USE_FRACHT)) {
                goto again;
            }
            if (RobotActions[1].ActionId == ACTION_WERBUNG && !RobotUse(ROBOT_USE_WERBUNG)) {
                goto again;
            }
            if (RobotActions[1].ActionId == ACTION_VISITNASA && (Airport.DoesRuneExist(RUNE_2SHOP, ROOM_NASA) == 0)) {
                goto again;
            }
            if (RobotActions[1].ActionId == ACTION_VISITTELESCOPE && (Airport.DoesRuneExist(RUNE_2SHOP, ROOM_RUSHMORE) == 0) &&
                (Airport.DoesRuneExist(RUNE_2SHOP, ROOM_INSEL) == 0)) {
                goto again;
            }
        }

        while (LastActionId == RobotActions[1].ActionId) {
        again2:

            RobotActions[1].ActionId = DefaultActions[PlayerWalkRandom.Rand(sizeof(DefaultActions) / sizeof(DefaultActions[0]))];

            if (RobotUse(ROBOT_USE_OFTENMECH) && (RobotActions[1].ActionId == ACTION_VISITTELESCOPE || RobotActions[1].ActionId == ACTION_VISITRICK ||
                                                  RobotActions[1].ActionId == ACTION_VISITKIOSK)) {
                RobotActions[1].ActionId = ACTION_VISITMECH;
            }
            if (RobotUse(ROBOT_USE_NOCHITCHAT) && (RobotActions[1].ActionId == ACTION_VISITTELESCOPE || RobotActions[1].ActionId == ACTION_VISITRICK ||
                                                   RobotActions[1].ActionId == ACTION_VISITKIOSK)) {
                goto again;
            }

            if (RobotActions[1].ActionId == ACTION_VISITNASA && !RobotUse(ROBOT_USE_NASA)) {
                goto again2;
            }
            if ((RobotActions[1].ActionId == ACTION_CHECKAGENT1 || RobotActions[1].ActionId == ACTION_CHECKAGENT2) && !GlobalUse(USE_TRAVELHOLDING)) {
                goto again2;
            }

            if (RobotActions[1].ActionId == ACTION_CHECKAGENT3 && !RobotUse(ROBOT_USE_FRACHT)) {
                goto again2;
            }
            if (RobotActions[1].ActionId == ACTION_WERBUNG && !RobotUse(ROBOT_USE_WERBUNG)) {
                goto again2;
            }
            if (RobotActions[1].ActionId == ACTION_VISITNASA && (Airport.DoesRuneExist(RUNE_2SHOP, ROOM_NASA) == 0)) {
                goto again2;
            }
            if (RobotActions[1].ActionId == ACTION_VISITTELESCOPE && (Airport.DoesRuneExist(RUNE_2SHOP, ROOM_RUSHMORE) == 0) &&
                (Airport.DoesRuneExist(RUNE_2SHOP, ROOM_INSEL) == 0)) {
                goto again2;
            }
        }

        // Nicht zum Designer/Security-Raum gehen, wenn das in diesem Level nicht sein soll:
        if (RobotActions[1].ActionId == ACTION_VISITDESIGNER && !RobotUse(ROBOT_USE_DESIGNER)) {
            goto again;
        }
        if (RobotActions[1].ActionId == ACTION_VISITSECURITY && !RobotUse(ROBOT_USE_SECURTY_OFFICE)) {
            goto again;
        }

        if (RobotUse(ROBOT_USE_MUCHWERBUNG) && RobotActions[1].ActionId == ACTION_VISITBANK && (Sim.GetHour() & 1) == 0) {
            RobotActions[1].ActionId = ACTION_WERBUNG;
        }
        if (RobotActions[1].ActionId == ACTION_VISITARAB && Sim.Date == 0) {
            goto again;
        }
        if (RobotActions[1].ActionId == ACTION_SABOTAGE && Sim.Date < 1 + PlayerNum) {
            goto again;
        }
        if (RobotActions[1].ActionId == ACTION_VISITARAB && Sim.Date == 1 && PlayerNum != 2) {
            goto again;
        }
        if (RobotActions[1].ActionId == ACTION_BUERO && OfficeState == 2) {
            goto again;
        }
        if (RobotActions[1].ActionId == ACTION_BUERO && Sim.GetHour() >= 10 && (HasItem(ITEM_LAPTOP) != 0) && LaptopVirus == 0 &&
            PlayerWalkRandom.Rand(100) < 60) {
            TimeBuro = Sim.Time;
            goto again;
        }

        if (LastActionId == RobotActions[1].ActionId) {
            goto again;
        }
        if (!RobotUse(ROBOT_USE_SABOTAGE) && (RobotActions[1].ActionId == ACTION_VISITARAB || RobotActions[1].ActionId == ACTION_SABOTAGE)) {
            goto again;
        }
        if (!RobotUse(ROBOT_USE_ROUTEBOX) && (RobotActions[1].ActionId == ACTION_VISITROUTEBOX)) {
            goto again;
        }

        if (Sim.Date > 1 && RobotActions[2].ActionId == ACTION_NONE && PlayerWalkRandom.Rand(2) == 0) {
            if (RobotUse(ROBOT_USE_EXTRA_SABOTAGE) && RobotUse(ROBOT_USE_SABOTAGE) &&
                (RobotActions[1].ActionId == ACTION_VISITKIOSK || RobotActions[1].ActionId == ACTION_VISITDUTYFREE ||
                 RobotActions[1].ActionId == ACTION_VISITBANK)) {
                RobotActions[2].ActionId = ACTION_SABOTAGE;
            }
            if (RobotUse(ROBOT_USE_EXTRA_BANK) && (RobotActions[1].ActionId == ACTION_VISITKIOSK || RobotActions[1].ActionId == ACTION_VISITDUTYFREE ||
                                                   RobotActions[1].ActionId == ACTION_SABOTAGE)) {
                RobotActions[2].ActionId = ACTION_VISITBANK;
            }
        }

        if (RobotUse(ROBOT_USE_MUCH_FRACHT)) {
            if (RobotActions[1].ActionId == ACTION_CHECKAGENT1 || RobotActions[1].ActionId == ACTION_CHECKAGENT2) {
                RobotActions[2].ActionId = RobotActions[1].ActionId;
                RobotActions[1].ActionId = ACTION_CHECKAGENT3;
            }
        }

        if (RobotUse(ROBOT_USE_FREE_FRACHT)) {
            if (RobotActions[1].ActionId == ACTION_CHECKAGENT1 || RobotActions[1].ActionId == ACTION_VISITMUSEUM) {
                RobotActions[2].ActionId = RobotActions[1].ActionId;
                RobotActions[1].ActionId = ACTION_CHECKAGENT3;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Computerspieler plant seine Routen:
//--------------------------------------------------------------------------------------------
void PLAYER::RobotPlanRoutes() {
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;
    SLONG BestC = 0;
    SLONG BestD = 0;
    SLONG BestDValue = 0;
    BOOL FlightAdded = 0;

    if (Owner != 1 || (IsOut != 0)) {
        return; // War Irtum, kein Computerspieler
    }

    BUFFER_V<SLONG> LastCity(Planes.AnzEntries());
    BUFFER_V<SLONG> PlanesOnRoute(Routen.AnzEntries());

    LastCity.FillWith(0);
    PlanesOnRoute.FillWith(0);

    // Alte Routenpläne löschen:
    forall(c, Planes) if (Planes.IsInAlbum(c) != 0) {
        CFlugplan &qPlan = Planes[c].Flugplan;

        for (d = qPlan.Flug.AnzEntries() - 1; d >= 0; d--) {
            if (qPlan.Flug[d].ObjectType == 2 && qPlan.Flug[d].Startdate > Sim.Date) {
                qPlan.Flug[d].ObjectType = 0;
            }

            // Was fliegen wir dann überhaupt noch?
            if (qPlan.Flug[d].ObjectType == 1) {
                PlanesOnRoute[qPlan.Flug[d].ObjectId]++;
                LastCity[c] = qPlan.Flug[d].NachCity;
            } else if (qPlan.Flug[d].ObjectType == 1) {
                LastCity[c] = Auftraege[d].NachCity;
            }
        }
    }

    // Sind einige Routen in Gefahr? Dann mindestens 4 Flugzeuge darauf ansetzen:
    forall(c, Routen) if ((Routen.IsInAlbum(c) != 0) && (RentRouten.RentRouten[c].Rang != 0U) && RentRouten.RentRouten[c].TageMitGering >= 15) {
        SLONG PlanesFound = 0;

        forall(d, Planes) if (Planes.IsInAlbum(d) != 0) {
            CFlugplan &qPlan = Planes[d].Flugplan;
            BOOL FlightAdded = FALSE;

            for (e = 0; e < qPlan.Flug.AnzEntries(); e++) {
                if (qPlan.Flug[e].ObjectType == 0) {
                    qPlan.Flug[e].ObjectType = 1;
                    qPlan.Flug[e].ObjectId = c;

                    qPlan.Flug[e].Okay = 0;
                    qPlan.Flug[e].Startdate = Sim.Date;
                    qPlan.Flug[e].Startzeit = Sim.GetHour() + 2;
                    qPlan.Flug[e].HoursBefore = 50;
                    qPlan.Flug[e].Ticketpreis = RentRouten.RentRouten[c].Ticketpreis;
                    qPlan.Flug[e].TicketpreisFC = RentRouten.RentRouten[c].TicketpreisFC;
                    qPlan.Flug[e].Gate = -1;

                    while (qPlan.Flug[d].Startzeit >= 24) {
                        qPlan.Flug[d].Startzeit -= 24;
                        qPlan.Flug[d].Startdate++;
                    }

                    // Zahl der Passagiere berechnen:
                    qPlan.Flug[e].CalcPassengers(PlayerNum, Planes[d]);
                    // qPlan.Flug[e].CalcPassengers (Planes[d].TypeId, PlayerNum, Planes[d]);
                    qPlan.Flug[e].PArrived = 0;

                    Planes[d].CheckFlugplaene(PlayerNum, TRUE, FALSE); // FALSE=Gates erst später planen

                    LastCity[d] = Routen[c].NachCity;

                    // Wurde der Flug direkt wieder aussortiert?
                    if (qPlan.Flug[e].ObjectType == 0) {
                        LastCity[d] = -1;
                    } else {
                        FlightAdded = TRUE;
                    }

                    break;
                }
            }

            if (FlightAdded != 0) {
                PlanesFound++;
            }
            if (PlanesFound >= 4) {
                break;
            }
        }
    }

    PlanGates();

    // Test: Haben wir für jede Route ein Flugzeug, was groß genug ist?
    BuyBigPlane = 0;
    for (c = 0; c < SLONG(Routen.AnzEntries()); c++) {
        if ((Routen.IsInAlbum(c) != 0) && (RentRouten.RentRouten[c].Rang != 0U)) {
            if (Cities.CalcDistance(Routen[c].VonCity, Routen[c].NachCity) / 1000 > BuyBigPlane) {
                BuyBigPlane = Cities.CalcDistance(Routen[c].VonCity, Routen[c].NachCity) / 1000;
            }
        }
    }
    forall(d, Planes) if (Planes.IsInAlbum(d) != 0) {
        if (Planes[d].ptReichweite >= BuyBigPlane) {
            BuyBigPlane = 0;
        }
    }
    // if (PlaneTypes[Planes[d].TypeId].Reichweite>=BuyBigPlane) BuyBigPlane=0;

    // Neue Routenpläne verteilen
    do {
        FlightAdded = FALSE;

        // Beste Route aussuchen:
        BestC = -1;
        for (c = 0; c < SLONG(Routen.AnzEntries()); c++) {
            if ((Routen.IsInAlbum(c) != 0) && (RentRouten.RentRouten[c].Rang != 0U) && PlanesOnRoute[c] != 99999) {
                if (BestC == -1 || (Routen[c].Bedarf / (PlanesOnRoute[c] + 1) > Routen[BestC].Bedarf / (PlanesOnRoute[c] + 1))) {
                    BestC = c;
                }
            }
        }

        if (BestC == -1) {
            break;
        }

        // Das Flugzeug, was am besten passt heraussuchen:
        BestD = -1;
        {
            SLONG CityDist = Cities.CalcDistance(Routen[BestC].VonCity, Routen[BestC].NachCity);

            forall(d, Planes) if ((Planes.IsInAlbum(d) != 0) && LastCity[d] != -1) {
                // if (CityDist<=PlaneTypes[Planes[d].TypeId].Reichweite*1000)
                if (CityDist <= Planes[d].ptReichweite * 1000) {
                    if (LastCity[d] == 0) {
                        BestD = d;
                        BestDValue = 0;
                    } else if (BestD == -1 || Cities.CalcDistance(LastCity[d], Routen[BestC].VonCity) < BestDValue) {
                        BestD = d;
                        BestDValue = Cities.CalcDistance(LastCity[d], Routen[BestC].VonCity);
                    }
                }
            }
        }

        // Flug in den Plan einfügen:
        if (BestD != -1) {
            CFlugplan &qPlan = Planes[BestD].Flugplan;

            for (d = 0; d < qPlan.Flug.AnzEntries(); d++) {
                if (qPlan.Flug[d].ObjectType == 0) {
                    qPlan.Flug[d].ObjectType = 1;
                    qPlan.Flug[d].ObjectId = BestC;

                    qPlan.Flug[d].Okay = 0;
                    qPlan.Flug[d].Startdate = Sim.Date;
                    qPlan.Flug[d].Startzeit = Sim.GetHour() + 2;
                    qPlan.Flug[d].HoursBefore = 50;
                    qPlan.Flug[d].Ticketpreis = RentRouten.RentRouten[BestC].Ticketpreis;
                    qPlan.Flug[d].TicketpreisFC = RentRouten.RentRouten[BestC].TicketpreisFC;
                    qPlan.Flug[d].Gate = -1;

                    while (qPlan.Flug[d].Startzeit >= 24) {
                        qPlan.Flug[d].Startzeit -= 24;
                        qPlan.Flug[d].Startdate++;
                    }

                    // Zahl der Passagiere berechnen:
                    // qPlan.Flug[d].CalcPassengers (Planes[BestD].TypeId, PlayerNum, Planes[BestD]);
                    qPlan.Flug[d].CalcPassengers(PlayerNum, Planes[BestD]);
                    qPlan.Flug[d].PArrived = 0;

                    Planes[BestD].CheckFlugplaene(PlayerNum, TRUE, FALSE); // FALSE=Gates erst später planen

                    LastCity[BestD] = Routen[BestC].NachCity;

                    // Wurde der Flug direkt wieder aussortiert?
                    if (qPlan.Flug[d].ObjectType == 0) {
                        LastCity[BestD] = -1;
                    }

                    FlightAdded = TRUE;
                    break;
                }
            }

            if (FlightAdded == FALSE) {
                PlanesOnRoute[BestC] = 99999;
                continue;
            }

            for (d = qPlan.Flug.AnzEntries() - 1; d >= 0; d--) {
                if (qPlan.Flug[d].ObjectType != 0 && qPlan.Flug[d].Landedate >= Sim.Date + 5) {
                    LastCity[BestD] = -1;
                }
            }
        }
    } while (FlightAdded != 0);

    PlanGates();

    // Autoflüge ggf. durch Routen ersetzen:
    forall(d, Planes) if (Planes.IsInAlbum(d) != 0) {
        CFlugplan &qPlan = Planes[d].Flugplan;

        for (e = 0; e < qPlan.Flug.AnzEntries(); e++) {
            if (qPlan.Flug[e].ObjectType == 3) {
                forall(c, Routen) if ((Routen.IsInAlbum(c) != 0) && (RentRouten.RentRouten[c].Rang != 0U)) {
                    if (Routen[c].VonCity == qPlan.Flug[e].VonCity && Routen[c].NachCity == qPlan.Flug[e].NachCity) {
                        qPlan.Flug[e].ObjectType = 1;
                        qPlan.Flug[e].ObjectId = c;

                        qPlan.Flug[e].Okay = 0;
                        qPlan.Flug[e].HoursBefore = 50;

                        qPlan.Flug[e].Ticketpreis = RentRouten.RentRouten[c].Ticketpreis;
                        qPlan.Flug[e].TicketpreisFC = RentRouten.RentRouten[c].TicketpreisFC;

                        // Zahl der Passagiere berechnen:
                        // qPlan.Flug[e].CalcPassengers (Planes[d].TypeId, PlayerNum, Planes[d]);
                        qPlan.Flug[e].CalcPassengers(PlayerNum, Planes[d]);
                        qPlan.Flug[e].PArrived = 0;

                        // Planes[d].CheckFlugplaene(PlayerNum);

                        LastCity[d] = Routen[c].NachCity;
                    }
                }
            }
        }
    }

    // Und jetzt die Ticketpreise:
    for (c = 0; c < SLONG(Routen.AnzEntries()); c++) {
        if ((Routen.IsInAlbum(c) != 0) && (RentRouten.RentRouten[c].Rang != 0U)) {
            CRoute &qRoute = Routen[c];

            // Welche Konkurrenten haben die Route auch?
            for (d = e = 0; d < 4; d++) {
                if ((Sim.Players.Players[d].IsOut == 0) && (Sim.Players.Players[d].RentRouten.RentRouten[c].Rang != 0U) &&
                    RentRouten.RentRouten[c].LastFlown < 5) {
                    e++;
                }
            }

            SLONG Cost = CalculateFlightCost(qRoute.VonCity, qRoute.NachCity, 800, 800, -1) * 3 / 180;

            RentRouten.RentRouten[c].Ticketpreis = Cost * 2 * 3 / 10 * 10;
            RentRouten.RentRouten[c].TicketpreisFC = RentRouten.RentRouten[c].Ticketpreis * 2;

            if (e > 1) {
                for (d = e = 0; d < 4; d++) { // Nicht viel teuerer als die Freunde:
                    if ((Sim.Players.Players[d].IsOut == 0) && (Sim.Players.Players[d].RentRouten.RentRouten[c].Rang != 0U) &&
                        RentRouten.RentRouten[c].LastFlown < 5) {
                        RentRouten.RentRouten[c].Ticketpreis =
                            min(RentRouten.RentRouten[c].Ticketpreis, Sim.Players.Players[d].RentRouten.RentRouten[c].Ticketpreis * 2 / 10 * 10);
                        RentRouten.RentRouten[c].TicketpreisFC =
                            min(RentRouten.RentRouten[c].TicketpreisFC, Sim.Players.Players[d].RentRouten.RentRouten[c].TicketpreisFC * 2 / 10 * 10);
                    }
                }

                for (d = e = 0; d < 4; d++) { // Billiger als die Feinde:
                    if ((Sim.Players.Players[d].IsOut == 0) && (Sim.Players.Players[d].RentRouten.RentRouten[c].Rang != 0U) &&
                        RentRouten.RentRouten[c].LastFlown < 5 && Sympathie[d] < -40) {
                        RentRouten.RentRouten[c].Ticketpreis =
                            min(RentRouten.RentRouten[c].Ticketpreis, Sim.Players.Players[d].RentRouten.RentRouten[c].Ticketpreis / 10 * 10 - 10);
                        RentRouten.RentRouten[c].TicketpreisFC =
                            min(RentRouten.RentRouten[c].TicketpreisFC, Sim.Players.Players[d].RentRouten.RentRouten[c].TicketpreisFC / 10 * 10 - 10);
                    }
                }

                // Aber immer ein Mindestpreis:
                RentRouten.RentRouten[c].Ticketpreis = max(RentRouten.RentRouten[c].Ticketpreis, Cost / 2 / 10 * 10);
                RentRouten.RentRouten[c].TicketpreisFC = max(RentRouten.RentRouten[c].TicketpreisFC, Cost / 2 / 10 * 10 * 2);

                // Kein Preiskampf bei Finanzkrise:
                if (Money < 100000) {
                    RentRouten.RentRouten[c].Ticketpreis = max(RentRouten.RentRouten[c].Ticketpreis, Cost / 10 * 10);
                    RentRouten.RentRouten[c].TicketpreisFC = max(RentRouten.RentRouten[c].TicketpreisFC, Cost / 10 * 10 * 2);
                }
                if (Money < 0) {
                    RentRouten.RentRouten[c].Ticketpreis = max(RentRouten.RentRouten[c].Ticketpreis, Cost * 2 / 10 * 10);
                    RentRouten.RentRouten[c].TicketpreisFC = max(RentRouten.RentRouten[c].TicketpreisFC, Cost * 2 / 10 * 10 * 2);
                }

                // Machen wir auf der Route Verlust?
                if (RentRouten.RentRouten[c].Ticketpreis < max(RentRouten.RentRouten[c].Ticketpreis, Cost / 10 * 10)) {
                    RentRouten.RentRouten[c].TageMitVerlust++;
                } else {
                    RentRouten.RentRouten[c].TageMitVerlust--;
                }

                if (RentRouten.RentRouten[c].TageMitVerlust > 10 && !RobotUse(ROBOT_USE_NEVERGIVEROUTEUP)) {
                    // Spieler hat die Faxen dicke und will die Route nicht mehr:
                    TEAKRAND rnd;
                    if (Sim.Players.Players[Sim.localPlayer].HasBerater(BERATERTYP_INFO) >= rnd.Rand(100)) {
                        Sim.Players.Players[Sim.localPlayer].Messages.AddMessage(
                            BERATERTYP_INFO, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 9006), (LPCTSTR)NameX, (LPCTSTR)AirlineX,
                                                     (LPCTSTR)Cities[Routen[c].VonCity].Name, (LPCTSTR)Cities[Routen[c].NachCity].Name));
                    }

                    for (e = 0; e < 4; e++) {
                        if ((Sim.Players.Players[e].IsOut == 0) && Sim.Players.Players[e].RentRouten.RentRouten[c].Rang > RentRouten.RentRouten[c].Rang) {
                            Sim.Players.Players[e].RentRouten.RentRouten[c].Rang--;
                        }
                    }

                    RentRouten.RentRouten[c].Rang = 0;
                    RentRouten.RentRouten[c].TageMitGering = 99;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Computerspieler ist angekommen, jetzt wird Aktion ausgeführt:
//--------------------------------------------------------------------------------------------
void PLAYER::RobotExecuteAction() {
    SLONG c = 0;
    SLONG n = 0;
    SLONG dislike = 0;
    SLONG level = 0;
    TEAKRAND rnd;

    if (Owner != 1 || (IsOut != 0)) {
        return; // War Irtum, kein Computerspieler
    }

    TEAKRAND LocalRandom;
    LocalRandom.SRand(WaitWorkTill);

    if (Sim.bNetwork != 0) {
        // Synchronisieren (tagsüber):
        if (WaitWorkTill == -1) {
            if (Sim.bIsHost != 0) {
                // Wird das zu spät (Wegen der 18-Uhr-Grenze)?
                if (Sim.Time + 2000 > 18 * 60000) {
                    RobotActions[0].ActionId = ACTION_NONE;
                    return;
                }

                if (!(Sim.CallItADay == 1)) {
                    // Normal tagsüber synchronisieren:
                    WaitWorkTill = Sim.TimeSlice + 40; // Zwei Sekunden warten, bis die Aktion wirklich ausgeführt wird
                    WaitWorkTill2 = Sim.Time + 2000;   // Zwei Sekunden warten, bis die Aktion wirklich ausgeführt wird

                    TEAKFILE Message;

                    Message.Announce(128);

                    Message << ATNET_ROBOT_EXECUTE << PlayerNum << WaitWorkTill << WaitWorkTill2;

                    for (c = 0; c < 4; c++) {
                        Message << Sympathie[c];
                    }
                    for (c = 0; c < RobotActions.AnzEntries(); c++) {
                        Message << RobotActions[c];
                    }

                    // Sicherheitshalber, falls zum Feierabend gewechselt wird:
                    if (Sim.bWatchForReady == 0 && Sim.Time > 9 * 60000 + 500 && Sim.Players.Players[Sim.localPlayer].GetRoom() != ROOM_ABEND &&
                        TopWin == nullptr) {
                        for (c = 0; c < 4; c++) {
                            Sim.Players.Players[c].bReadyForMorning = 0;
                        }
                    }

                    SIM::SendMemFile(Message);

                    return;
                }
            } else {
                // Für Synchronität zwischen Host und Client
                if (!(Sim.CallItADay == 1)) {
                    return;
                }
            }
        }

        // Synchronisieren (nach Feierabend):
        if ((Sim.bNetwork != 0) && Sim.GetHour() >= 9 && Sim.GetHour() < 18 && Sim.CallItADay == 1) {
            WaitWorkTill = -1;

            if (Sim.bIsHost != 0) {
                // Feierabend synchronisieren: Die anderen Spieler auffordern, nachzuziehen:
                TEAKFILE Message;

                Message.Announce(128);

                LocalRandom.SRand(Sim.TimeSlice);

                WaitWorkTill2 = Sim.Time;
                Message << ATNET_ROBOT_EXECUTE << PlayerNum << Sim.TimeSlice << WaitWorkTill2;

                for (c = 0; c < 4; c++) {
                    Message << Sympathie[c];
                }
                for (c = 0; c < RobotActions.AnzEntries(); c++) {
                    Message << RobotActions[c];
                }

                SIM::SendMemFile(Message);

                // Darauf warten, daß die anderen Spieler ihr okay geben:
                if (Sim.Time > 9 * 60000 + 500) {
                    for (c = 0; c < 4; c++) {
                        Sim.Players.Players[c].bReadyForMorning = 0;
                    }
                }
            } else {
                // Okay, habe den Computerspieler durchgeführt:
                SIM::SendSimpleMessage(ATNET_READYFORMORNING, 0, Sim.localPlayer);
            }
        }
    } else {
        LocalRandom.SRand(time(nullptr));
    }

    // Manchmal führt der Client Sachen doppelt aus. Seltsam aber wahr. Hiermit wird's verhindert:
    {
        static SLONG LastActionId;
        static ULONG LastTime;
        static ULONG UniqueGameId;

        if (Sim.Time == LastTime && RobotActions[0].ActionId == LastActionId && UniqueGameId == Sim.UniqueGameId) {
            RobotActions[0].ActionId = ACTION_NONE;
            return;
        }

        LastTime = Sim.Time;
        LastActionId = RobotActions[0].ActionId;
        UniqueGameId = Sim.UniqueGameId;
    }

    // NetGenericSync (100, LocalRandom.GetSeed());
    // NetGenericSync (101, PlayerNum);

    n = 0;
    dislike = -1;
    for (c = 0; c < 4; c++) {
        if (Sympathie[c] < 0 && (Sim.Players.Players[c].IsOut == 0)) {
            n++;
        }
    }

    if (n != 0) {
        n = (LocalRandom.Rand(n)) + 1;
        for (c = 0; c < 4; c++) {
            if (Sympathie[c] < 0 && (Sim.Players.Players[c].IsOut == 0)) {
                n--;
                if (n == 0) {
                    dislike = c;
                    break;
                }
            }
        }
    } else {
        LocalRandom.Rand(2); // Sicherheitshalber, damit wir immer genau ein Random ausführen
    }
    TargetedPlayer = dislike;

    // NetGenericSync (100, LocalRandom.GetSeed());
    // NetGenericSync (101, RobotActions[0].ActionId);

    // Evtl. den Spieler anrufen:
    SLONG TargetPlayer = LocalRandom.Rand(4);
    if (Sim.Players.Players[TargetPlayer].Owner != 1 && Sim.Players.Players[TargetPlayer].IsOut != 1) {
        if (Sim.Players.Players[TargetPlayer].IsOkayToCallThisPlayer() != 0) {
            if (((Kooperation[TargetPlayer] != 0) && Sympathie[TargetPlayer] < -20) ||
                (Kooperation[TargetPlayer] == FALSE && Sympathie[TargetPlayer] > 50 && (CalledPlayer == 0))) {
                if (RobotActions[0].ActionId == ACTION_STARTDAY || (HasItem(ITEM_HANDY) != 0)) {
                    if (TargetPlayer == Sim.localPlayer) {
                        gUniversalFx.Stop();
                        gUniversalFx.ReInit("phone.raw");
                        gUniversalFx.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);

                        CalledPlayer = 2;
                        bgWarp = FALSE;
                        if (CheatTestGame == 0 && CheatAutoSkip == 0) {
                            Sim.Players.Players[TargetPlayer].GameSpeed = 0;
                            if (Sim.bNetwork != 0) {
                                SIM::SendSimpleMessage(ATNET_SETSPEED, 0, Sim.localPlayer, Sim.Players.Players[TargetPlayer].GameSpeed);
                            }
                        }

                        (Sim.Players.Players[TargetPlayer].LocationWin)->StartDialog(TALKER_COMPETITOR, MEDIUM_HANDY, PlayerNum, 1);
                        (Sim.Players.Players[TargetPlayer].LocationWin)->PayingForCall = FALSE;

                        if (Sim.bNetwork != 0) {
                            SIM::SendSimpleMessage(ATNET_DIALOG_LOCK, 0, PlayerNum);
                            SIM::SendSimpleMessage(ATNET_DIALOG_LOCK, 0, Sim.Players.Players[TargetPlayer].PlayerNum);

                            PERSON &qRobotPlayer = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)];
                            PERSON &qHumanPlayer = Sim.Persons[Sim.Persons.GetPlayerIndex(TargetPlayer)];

                            BroadcastPosition();
                            Sim.Players.Players[TargetPlayer].BroadcastPosition();
                            SIM::SendSimpleMessage(ATNET_PLAYERLOOK, 0, qRobotPlayer.State, qRobotPlayer.Phase);
                            SIM::SendSimpleMessage(ATNET_PLAYERLOOK, 0, qHumanPlayer.State, qHumanPlayer.Phase);
                        }
                    }
                }
            }
        }
    }

    // 6.3.02 Manchmal kommen wir als Client hier an, bevor in der Anrufenden Funktion der Shift durchgeführt wurde. Das holen wir dann hier mal schnell nach:
    if (RobotActions[0].ActionId == ACTION_NONE && RobotActions[1].ActionId != ACTION_NONE) {
        // Alles in der Queue einen weiterschieben
        for (c = 0; c < RobotActions.AnzEntries() - 1; c++) {
            RobotActions[c] = RobotActions[c + 1];
        }

        RobotActions[RobotActions.AnzEntries() - 1].ActionId = ACTION_NONE;
    }

    // Die exakte Zeit des Ausführens auf dem Server simulieren
    SLONG RealLocalTime = Sim.Time;

    /*AT_Log("Player %li: Action: %s, %s at %li/%li\n", PlayerNum, Translate_ACTION(RobotActions[0].ActionId), Translate_ACTION(RobotActions[1].ActionId),
           WaitWorkTill, WaitWorkTill2);*/
    NetGenericSync(770 + PlayerNum, RobotActions[0].ActionId);
    NetGenericSync(740 + PlayerNum, RobotActions[1].ActionId);

    if (Sim.bNetwork != 0) {
        Sim.Time = WaitWorkTill2;
        WaitWorkTill = -1;
    }

    switch (RobotActions[0].ActionId) {
    case 0:
        WorkCountdown = 2;
        break;

    case ACTION_STARTDAY:
        // Logik für wechsel zu Routen und sparen für Rakete oder Flugzeug:
        if (DoRoutes == 0) {
            if (RobotUse(ROBOT_USE_SUGGESTROUTES) || (PlayerNum + 30 < Sim.Date && Planes.GetNumUsed() > 6) ||
                (PlayerNum + 15 < Sim.Date &&
                 (Sim.Players.Players[(PlayerNum + 3) % 4].DoRoutes == 1 || Sim.Players.Players[(PlayerNum + 3) % 4].DoRoutes > 20))) {

                if (Image < 150) {
                    WantToDoRoutes = TRUE;

                    if (Sim.Players.Players[Sim.localPlayer].NumMissionRoutes > PlayerNum) {
                        DoRoutes = TRUE;
                    }
                } else {
                    DoRoutes = TRUE;
                }
            }

            if (RobotUse(ROBOT_USE_FORCEROUTES)) {
                DoRoutes = TRUE;
            }
        }

        if ((Image > -100 && (DoRoutes == 0)) || (Image > 100 || (DoRoutes != 0))) {
            for (c = n = 0; c < Sim.Players.AnzPlayers; c++) {
                if ((Sim.Players.Players[c].IsOut == 0) && c != PlayerNum) {
                    if (Planes.GetNumUsed() <= Sim.Players.Players[c].Planes.GetNumUsed()) {
                        n++;
                    }
                }
            }

            if (((SavesForRocket == 0) && n >= 3) || (n >= 2 && RobotUse(ROBOT_USE_MORE_PLANES)) || (n >= 1 && RobotUse(ROBOT_USE_MORE_PLANES_2))) {
                SavesForPlane = TRUE;
            }

            if ((SavesForRocket == 0) && dislike != -1 && Planes.GetNumUsed() + 2 <= Sim.Players.Players[dislike].Planes.GetNumUsed()) {
                SavesForPlane = TRUE;
            }

            if (dislike != -1 && Planes.GetNumUsed() > 3 && RocketFlags <= Sim.Players.Players[dislike].RocketFlags && RobotUse(ROBOT_USE_NASA)) {
                SavesForPlane = FALSE;
                SavesForRocket = TRUE;
            }
        }

        // Im Ausland anrufen:
        if ((DoRoutes == 0) && RobotUse(ROBOT_USE_ABROAD) && (!RobotUse(ROBOT_USE_MUCH_FRACHT) || Sim.GetHour() > 9)) {
            for (SLONG n = 0; n < SLONG(Cities.AnzEntries()); n++) {
                SLONG c = 0;

                if ((Sim.Players.Players[Sim.localPlayer].LocationWin != nullptr) &&
                    (Sim.Players.Players[Sim.localPlayer].LocationWin)->CurrentMenu == MENU_AUSLANDSAUFTRAG &&
                    Cities.GetIdFromIndex((Sim.Players.Players[Sim.localPlayer].LocationWin)->MenuPar1) == Cities.GetIdFromIndex(n)) {
                    continue; // Skip it, because Player is just phoning it.
                }

                if (RentCities.RentCities[n].Rang != 0U) {
                    goto okay_we_have_that_city;
                }

                for (c = 0; c < 4; c++) {
                    if ((Sim.Players.Players[c].IsOut == 0) && (Kooperation[c] != 0) && (Sim.Players.Players[c].RentCities.RentCities[n].Rang != 0U)) {
                        goto okay_we_have_that_city;
                    }
                }

                continue; // Skip that city, 'cause we're not present here

            okay_we_have_that_city:
                // Aber nicht den Heimatflughafen nehmen:
                if (n == SLONG(Cities(Sim.HomeAirportId))) {
                    continue;
                }

                CAuftraege &qAuftraege = AuslandsAuftraege[n];
                CFrachten &qFrachten = AuslandsFrachten[n];

                qAuftraege.RefillForAusland(n);
                qFrachten.RefillForAusland(n);

                // Normale Aufträge:
                for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
                    if (Planes.IsInAlbum(c) != 0) {
                        SLONG d = 0;
                        SLONG e = 0;
                        SLONG VonCity = 0;
                        SLONG VonZeit = 0;

                        for (level = 0; level <= 2; level++) {
                            VonCity = Planes[c].Flugplan.StartCity;
                            VonZeit = Sim.GetHour() + 2;

                            if (Sim.Date == 0 && Sim.GetHour() == 9) {
                                VonZeit = Sim.GetHour() + 2 + PlayerNum / 2;
                            }

                            for (d = Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
                                if (Planes[c].Flugplan.Flug[d].ObjectType != 0) {
                                    VonCity = Planes[c].Flugplan.Flug[d].NachCity;
                                    VonZeit = Planes[c].Flugplan.Flug[d].Landezeit + 1;
                                    VonZeit += (Planes[c].Flugplan.Flug[d].Landedate - Sim.Date) * 24;
                                    break;
                                }
                            }

                            if (VonZeit < Sim.GetHour() + 2) {
                                VonZeit = Sim.GetHour() + 2;
                            }

                            if (VonZeit >= 0 && VonZeit < 23 + 24) {
                                if (level == 0 || level == 1) {
                                    for (e = qAuftraege.AnzEntries() - 1; e >= 0; e--) {
                                        if ((qAuftraege[e].Praemie > 0) && qAuftraege[e].VonCity == ULONG(VonCity)) {
                                            if (RobotUse(ROBOT_USE_SHORTFLIGHTS) && Planes.GetNumUsed() == 4 &&
                                                (Cities.CalcDistance(qAuftraege[e].VonCity, qAuftraege[e].NachCity) > 3000000 ||
                                                 qAuftraege[e].Personen <= 90)) {
                                                continue;
                                            }

                                            SLONG Cost = ((CalculateFlightCost(VonCity, qAuftraege[e].VonCity, 8000, 700, -1)) + 99) / 100 * 100;
                                            if (Cost <= qAuftraege[e].Praemie * 8 / 10) {
                                                if (Auftraege.GetNumFree() < 2) {
                                                    Auftraege.ReSize(Auftraege.AnzEntries() + 1);
                                                }
                                                SLONG ObjectId = (Auftraege += qAuftraege[e]);

                                                if (Sim.Difficulty != DIFF_ADDON09 || NumOrderFlightsToday2 >= 5) {
                                                    Statistiken[STAT_AUFTRAEGE].AddAtPastDay(1);
                                                }

                                                NumOrderFlightsToday2++;

                                                qAuftraege[e].Praemie = 0;

                                                CFlugplanEintrag &fpe = Planes[c].Flugplan.Flug[Planes[c].Flugplan.Flug.AnzEntries() - 1];

                                                fpe.ObjectType = 2;
                                                fpe.ObjectId = ObjectId;
                                                fpe.Okay = 0;
                                                fpe.Startdate = Sim.Date + VonZeit / 24;
                                                fpe.Startzeit = VonZeit % 24;

                                                fpe.FlightChanged();
                                                // fpe.CalcPassengers (Planes[c].TypeId, PlayerNum, Planes[c]);
                                                fpe.CalcPassengers(PlayerNum, Planes[c]);
                                                fpe.PArrived = 0;

                                                Planes[c].Flugplan.UpdateNextFlight();
                                                Planes[c].Flugplan.UpdateNextStart();

                                                Planes[c].CheckFlugplaene(PlayerNum);
                                                UpdateAuftragsUsage();
                                                DelayFlightsIfNecessary();
                                                break;
                                            }
                                        }
                                    }
                                } else if (level == 2) {
                                    SLONG Min = 999999999;
                                    SLONG MinE = -1;

                                    for (e = qAuftraege.AnzEntries() - 1; e >= 0; e--) {
                                        if ((qAuftraege[e].Praemie > 0) &&
                                            VonZeit + Cities.CalcFlugdauer(VonCity, qAuftraege[e].VonCity, Planes[c].ptGeschwindigkeit) < 23 + 24) {
                                            // VonZeit+Cities.CalcFlugdauer (VonCity, qAuftraege[e].VonCity,
                                            // PlaneTypes[Planes[c].TypeId].Geschwindigkeit)<23+24)
                                            if (Cities.CalcDistance(VonCity, qAuftraege[e].VonCity) < Min) {
                                                if (RobotUse(ROBOT_USE_SHORTFLIGHTS) && Planes.GetNumUsed() == 4 &&
                                                    (Cities.CalcDistance(qAuftraege[e].VonCity, qAuftraege[e].NachCity) > 3000000 ||
                                                     qAuftraege[e].Personen <= 90)) {
                                                    continue;
                                                }

                                                Min = Cities.CalcDistance(VonCity, qAuftraege[e].VonCity);
                                                MinE = e;
                                            }
                                        }
                                    }

                                    if (MinE != -1 && Min < 9000000 && (Min < 5000000 || VonZeit < 34) && (Min < 3000000 || VonZeit < 24)) {
                                        e = MinE;

                                        SLONG Cost = ((CalculateFlightCost(VonCity, qAuftraege[e].VonCity, 8000, 700, -1)) + 99) / 100 * 100;
                                        if (Cost <= qAuftraege[e].Praemie) {
                                            if (Auftraege.GetNumFree() < 2) {
                                                Auftraege.ReSize(Auftraege.AnzEntries() + 1);
                                            }
                                            SLONG ObjectId = (Auftraege += qAuftraege[e]);
                                            qAuftraege[e].Praemie = 0;

                                            if (Sim.Difficulty != DIFF_ADDON09 || NumOrderFlightsToday2 >= 5) {
                                                Statistiken[STAT_AUFTRAEGE].AddAtPastDay(1);
                                            }

                                            NumOrderFlightsToday2++;

                                            CFlugplanEintrag &fpe = Planes[c].Flugplan.Flug[Planes[c].Flugplan.Flug.AnzEntries() - 1];

                                            // DisplayBroadcastMessage (bprintf ("B: %s took Order %s-%s", (LPCTSTR)NameX,
                                            // (LPCTSTR)Cities[qAuftraege[e].VonCity].Name, (LPCTSTR)Cities[qAuftraege[e].NachCity].Name));

                                            fpe.ObjectType = 2;
                                            fpe.ObjectId = ObjectId;
                                            fpe.Okay = 0;
                                            fpe.Startdate = Sim.Date + VonZeit / 24;
                                            fpe.Startzeit = VonZeit % 24;

                                            fpe.FlightChanged();
                                            fpe.CalcPassengers(PlayerNum, Planes[c]);
                                            // fpe.CalcPassengers (Planes[c].TypeId, PlayerNum, Planes[c]);
                                            fpe.PArrived = 0;

                                            // log: hprintf ("Player %li takes(2) %li Passengers %li-%li (%li, %li)", PlayerNum, fpe.Passagiere,
                                            // qAuftraege[e].VonCity, qAuftraege[e].NachCity, n, e);

                                            Planes[c].Flugplan.UpdateNextFlight();
                                            Planes[c].Flugplan.UpdateNextStart();

                                            Planes[c].CheckFlugplaene(PlayerNum);
                                            UpdateAuftragsUsage();
                                            DelayFlightsIfNecessary();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // ANFANG FRACHTAUFTRÄGE
                if ((DoRoutes == 0) && !(RobotUse(ROBOT_USE_SHORTFLIGHTS) && Planes.GetNumUsed() == 4)) {
                    SLONG Bewertungsbonus = 0;
                    if (RobotUse(ROBOT_USE_MUCH_FRACHT) && RobotUse(ROBOT_USE_MUCH_FRACHT_BONUS)) {
                        Bewertungsbonus = 150000;
                    }

                    for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
                        if (Planes.IsInAlbum(c) != 0) {
                            SLONG d = 0;
                            SLONG e = 0;
                            SLONG VonCity = 0;
                            SLONG VonZeit = 0;

                            if ((Planes[c].ptPassagiere / 10) <= 0) {
                                continue;
                            }

                            for (level = 0; level <= 2; level++) {
                                VonCity = Planes[c].Flugplan.StartCity;
                                VonZeit = Sim.GetHour() + 2;

                                if (Sim.Date == 0 && Sim.GetHour() == 9) {
                                    VonZeit = Sim.GetHour() + 2 + PlayerNum / 2;
                                }

                                for (d = Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
                                    if (Planes[c].Flugplan.Flug[d].ObjectType != 0) {
                                        VonCity = Planes[c].Flugplan.Flug[d].NachCity;
                                        VonZeit = Planes[c].Flugplan.Flug[d].Landezeit + 1;
                                        VonZeit += (Planes[c].Flugplan.Flug[d].Landedate - Sim.Date) * 24;
                                        break;
                                    }
                                }

                                if (VonZeit < Sim.GetHour() + 2) {
                                    VonZeit = Sim.GetHour() + 2;
                                }

                                if (VonZeit >= 0 && VonZeit < 23 + 24 + 24) {
                                    if (level == 0 || level == 1) {
                                        for (e = qFrachten.AnzEntries() - 1; e >= 0; e--) {
                                            CFracht &qFracht = qFrachten[e];

                                            if (qFracht.Praemie >= 0 && qFracht.VonCity == ULONG(VonCity)) {
                                                SLONG Cost = ((CalculateFlightCost(qFracht.VonCity, qFracht.NachCity, 8000, 700, -1)) + 99) / 100 * 100;

                                                // Multiplikator, weil wir mehrere Flüge durchführen müssen:
                                                int trips = (qFracht.Tons / (Planes[c].ptPassagiere / 10) + 1);
                                                Cost = Cost * 2 * trips;

                                                if ((trips < 4) && ((Cost <= qFracht.Praemie * 8 / 10 + Bewertungsbonus) ||
                                                                    (RobotUse(ROBOT_USE_FREE_FRACHT) && qFracht.Praemie == 0))) {
                                                    if (Frachten.GetNumFree() < 2) {
                                                        Frachten.ReSize(Frachten.AnzEntries() + 1);
                                                    }
                                                    Statistiken[STAT_FRACHTEN].AddAtPastDay(1);
                                                    SLONG ObjectId = (Frachten += qFracht);
                                                    qFracht.Praemie = -1;

                                                    while (qFracht.TonsOpen > 0) {
                                                        CFlugplanEintrag &fpe = Planes[c].Flugplan.Flug[Planes[c].Flugplan.Flug.AnzEntries() - 1];

                                                        fpe.ObjectType = 4;
                                                        fpe.ObjectId = ObjectId;
                                                        fpe.Okay = 0;
                                                        fpe.Startdate = Sim.Date + VonZeit / 24;
                                                        fpe.Startzeit = VonZeit % 24;

                                                        fpe.FlightChanged();
                                                        fpe.PArrived = 0;

                                                        Planes[c].Flugplan.UpdateNextFlight();
                                                        Planes[c].Flugplan.UpdateNextStart();

                                                        Planes[c].CheckFlugplaene(PlayerNum);
                                                        DelayFlightsIfNecessary();

                                                        Sim.Players.CheckFlighplans();
                                                        qFracht.TonsOpen -= Planes[c].ptPassagiere / 10;
                                                    }

                                                    break;
                                                }
                                            }
                                        }
                                    } else if (level == 2) {
                                        SLONG MinMin = 0; // Min muß mindestens so groß sein

                                        while (true) {
                                            SLONG Min = 999999999;
                                            SLONG MinE = -1;

                                            for (e = qFrachten.AnzEntries() - 1; e >= 0; e--) {
                                                if (qFrachten[e].Praemie >= 0 &&
                                                    VonZeit + Cities.CalcFlugdauer(VonCity, qFrachten[e].VonCity, Planes[c].ptGeschwindigkeit) < 23 + 24) {
                                                    if (Cities.CalcDistance(VonCity, qFrachten[e].VonCity) < Min &&
                                                        Cities.CalcDistance(VonCity, qFrachten[e].VonCity) > MinMin) {
                                                        Min = Cities.CalcDistance(VonCity, qFrachten[e].VonCity);
                                                        MinE = e;
                                                    }
                                                }
                                            }

                                            if (MinE == -1) {
                                                break;
                                            }
                                            MinMin = Min;

                                            if (MinE != -1 && Min < 15000000 && (Min < 6000000 || VonZeit < 34 + 24) && (Min < 3000000 || VonZeit < 24 + 24)) {
                                                e = MinE;

                                                CFracht &qFracht = qFrachten[e];

                                                SLONG Cost = ((CalculateFlightCost(qFracht.VonCity, qFracht.NachCity, 8000, 700, -1)) + 99) / 100 * 100;

                                                // Multiplikator, weil wir mehrere Flüge durchführen müssen:
                                                int trips = (qFracht.Tons / (Planes[c].ptPassagiere / 10) + 1);
                                                Cost = Cost * trips;

                                                // Anflug:
                                                Cost += ((CalculateFlightCost(VonCity, qFracht.VonCity, 8000, 700, -1)) + 99) / 100 * 100;

                                                if ((trips < 4) && ((Cost <= qFracht.Praemie + Bewertungsbonus) ||
                                                                    (RobotUse(ROBOT_USE_FREE_FRACHT) && qFracht.Praemie == 0))) {
                                                    if (Frachten.GetNumFree() < 2) {
                                                        Frachten.ReSize(Frachten.AnzEntries() + 1);
                                                    }
                                                    Statistiken[STAT_FRACHTEN].AddAtPastDay(1);
                                                    SLONG ObjectId = (Frachten += qFracht);
                                                    qFracht.Praemie = -1;

                                                    while (qFracht.TonsOpen > 0) {
                                                        CFlugplanEintrag &fpe = Planes[c].Flugplan.Flug[Planes[c].Flugplan.Flug.AnzEntries() - 1];

                                                        fpe.ObjectType = 4;
                                                        fpe.ObjectId = ObjectId;
                                                        fpe.Okay = 0;
                                                        fpe.Startdate = Sim.Date + VonZeit / 24;
                                                        fpe.Startzeit = VonZeit % 24;

                                                        fpe.FlightChanged();
                                                        fpe.PArrived = 0;

                                                        Planes[c].Flugplan.UpdateNextFlight();
                                                        Planes[c].Flugplan.UpdateNextStart();

                                                        Planes[c].CheckFlugplaene(PlayerNum);
                                                        DelayFlightsIfNecessary();

                                                        Sim.Players.CheckFlighplans();
                                                        qFracht.TonsOpen -= Planes[c].ptPassagiere / 10;
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
                // ENDE FRACHTAUFTRÄGE

                qAuftraege.RefillForAusland(n, 3);
                qFrachten.RefillForAusland(n, 3);
            }
        }
        if ((StrikeHours != 0) && LocalRandom.Rand(6) == 0 && RobotUse(ROBOT_USE_MISC_CHEATS)) {
            Sim.Players.Players[PlayerNum].StrikeHours = 0;
        }

        if (RobotUse(ROBOT_USE_LUXERY) && Planes.GetNumUsed() > 0 && Money > 200000 && (bHasPlanesUpgradedToday == 0)) {
            SLONG prob = 5;

            if (Money > 2500000) {
                prob = 3;
            }
            if (Money > 15000000) {
                prob = 2;
            }
            if (Money > 25000000) {
                prob = 1;
            }
            if (LocalRandom.Rand(prob) == 0) {
                bHasPlanesUpgradedToday = TRUE;
                __int64 moneyAvailable = (Money - 200000);
                SLONG anzahl = 4;
                while (moneyAvailable > 0 && anzahl-- > 0) {
                    for (SLONG bpass = 0; bpass < 3; bpass++) {
                        CPlane &qPlane = Planes[Planes.GetRandomUsedIndex(&LocalRandom)];

                        for (SLONG pass = 0; pass < 4; pass++) {
                            switch (LocalRandom.Rand(7)) {
                            case 0:
                                if (qPlane.SitzeTarget == 2 || SeatCosts[qPlane.SitzeTarget + 1] > moneyAvailable) {
                                    continue;
                                }
                                qPlane.SitzeTarget++;
                                moneyAvailable -= SeatCosts[qPlane.SitzeTarget];
                                break;
                            case 1:
                                if (qPlane.TablettsTarget == 2 || TrayCosts[qPlane.TablettsTarget + 1] > moneyAvailable) {
                                    continue;
                                }
                                qPlane.TablettsTarget++;
                                moneyAvailable -= TrayCosts[qPlane.TablettsTarget];
                                break;
                            case 2:
                                if (qPlane.DecoTarget == 2 || DecoCosts[qPlane.DecoTarget + 1] > moneyAvailable) {
                                    continue;
                                }
                                qPlane.DecoTarget++;
                                moneyAvailable -= DecoCosts[qPlane.DecoTarget];
                                break;
                            case 3:
                                if (qPlane.TriebwerkTarget == 2 || TriebwerkCosts[qPlane.TriebwerkTarget + 1] > moneyAvailable) {
                                    continue;
                                }
                                qPlane.TriebwerkTarget++;
                                moneyAvailable -= TriebwerkCosts[qPlane.TriebwerkTarget];
                                break;
                            case 4:
                                if (qPlane.ReifenTarget == 2 || ReifenCosts[qPlane.ReifenTarget + 1] > moneyAvailable) {
                                    continue;
                                }
                                qPlane.ReifenTarget++;
                                moneyAvailable -= ReifenCosts[qPlane.ReifenTarget];
                                break;
                            case 5:
                                if (qPlane.ElektronikTarget == 2 || ElektronikCosts[qPlane.ElektronikTarget + 1] > moneyAvailable) {
                                    continue;
                                }
                                qPlane.ElektronikTarget++;
                                moneyAvailable -= ElektronikCosts[qPlane.ElektronikTarget];
                                break;
                            case 6:
                                if (qPlane.SicherheitTarget == 2 || SicherheitCosts[qPlane.SicherheitTarget + 1] > moneyAvailable) {
                                    continue;
                                }
                                qPlane.SicherheitTarget++;
                                moneyAvailable -= SicherheitCosts[qPlane.SicherheitTarget];
                                break;
                            default:
                                AT_Log("Player.cpp: Default case should not be reached.");
                                DebugBreak();
                            }

                            bpass = 4;
                            break;
                        }
                    }
                }
            }
        }

        WorkCountdown = 20 * 5;
        TimeBuro = Sim.Time;
        break;

    case ACTION_VISITMAKLER:
        if (Planes.GetNumUsed() > 0) {
            if (!GlobalUse(USE_TRAVELHOLDING) && LocalRandom.Rand(8) == 0) {
                CPlane &qPlane = Planes[Planes.GetRandomUsedIndex(&LocalRandom)];

                switch (LocalRandom.Rand(7)) {
                case 0:
                    qPlane.SitzeTarget = min(2, qPlane.SitzeTarget + 1);
                    break;
                case 1:
                    qPlane.TablettsTarget = min(2, qPlane.TablettsTarget + 1);
                    break;
                case 2:
                    qPlane.DecoTarget = min(2, qPlane.DecoTarget + 1);
                    break;
                case 3:
                    qPlane.TriebwerkTarget = min(2, qPlane.TriebwerkTarget + 1);
                    break;
                case 4:
                    qPlane.ReifenTarget = min(2, qPlane.ReifenTarget + 1);
                    break;
                case 5:
                    qPlane.ElektronikTarget = min(2, qPlane.ElektronikTarget + 1);
                    break;
                case 6:
                    qPlane.SicherheitTarget = min(2, qPlane.SicherheitTarget + 1);
                    break;
                default:
                    AT_Log("Player.cpp: Default case should not be reached.");
                    DebugBreak();
                }
            }
        }
        WorkCountdown = 20 * 5;
        break;

    case ACTION_PERSONAL:
        WorkCountdown = 20 * 5;
        TimePersonal = Sim.Time;
        break;

    case ACTION_VISITARAB: {
        WorkCountdown = 20 * 5;
        auto Preis = Sim.HoleKerosinPreis(1);
        if (RobotUse(ROBOT_USE_TANKS) && Preis < 500 && Money > 1000000) {
            __int64 Menge = (Money - 2500000) / Preis;
            if (Preis < 400) {
                Menge = (Money - 2000000) / Preis;
            }
            if (Preis < 350) {
                Menge = (Money - 1500000) / Preis;
            }

            if (Menge > 0) {
                Menge = min(Menge, Tank - TankInhalt);
                TankInhalt += SLONG(Menge);
                ChangeMoney(-Menge * Preis, 2020, "");
            }
        }
    } break;

    case ACTION_SABOTAGE:
        if (dislike == -1 && (RobotUse(ROBOT_USE_EXTREME_SABOTAGE) || RobotUse(ROBOT_USE_EXTRA_SABOTAGE) || LocalRandom.Rand(3) == 0)) {
            dislike = LocalRandom.Rand(4);
            if (RobotUse(ROBOT_USE_EXTREME_SABOTAGE) && (dislike == PlayerNum || (Sim.Players.Players[dislike].IsOut != 0))) {
                SLONG r = -9999; // Besten Spieler als Sabotageziel wählen:
                for (SLONG c = 0; c < 4; c++) {
                    if ((Sim.Players.Players[c].IsOut == 0) && Sim.Players.Players[c].GetMissionRating() > r) {
                        dislike = c;
                        r = Sim.Players.Players[c].GetMissionRating();
                    }
                }
            }
            if (dislike == PlayerNum || (Sim.Players.Players[dislike].IsOut != 0)) {
                dislike = -1;
            }
        }

        TargetedPlayer = dislike;

        ArabTrust = max(1, ArabTrust); // Computerspieler müssen Araber nicht bestechen

        if (ArabHints < 100 && Image > -500 && ArabMode == 0 && ArabMode2 == 0 && ArabMode3 == 0) {
            if (((Sim.Date + PlayerNum) % 3 != 0 || RobotUse(ROBOT_USE_EXTREME_SABOTAGE) || LocalRandom.Rand(4) == 0 ||
                 (RobotUse(ROBOT_USE_EXTRA_SABOTAGE) && LocalRandom.Rand(2) == 0)) &&
                dislike != -1 && dislike != PlayerNum &&
                (ArabHints < 80 || (ArabHints < 90 && Credit < 1000000 && Money > 3000000 && RobotUse(ROBOT_USE_SABO_AFFORD_FINE) && (Sim.Date & 3) == 0)) &&
                (Sim.Players.Players[dislike].Owner == 1 || RobotUse(ROBOT_USE_MUCH_SABOTAGE))) {
                SLONG SecurityAnnoiance = 0;

                for (SLONG pass = 1; pass <= 2 + 3 * static_cast<SLONG>(RobotUse(ROBOT_USE_EXTREME_SABOTAGE)); pass++) {
                    switch (Sim.GetHour() % 3) {
                    case 0:
                        if (Sim.Players.Players[dislike].Planes.GetNumUsed() > 0) {
                            ArabOpfer = dislike;
                            ArabMode = min(4, ArabTrust);
                            if (pass > 2 && ArabMode > 0) {
                                ArabMode = LocalRandom.Rand(ArabMode);
                            }

                            if (RobotUse(ROBOT_USE_MILDER_SABOTAGE) && ArabTrust >= 2 && LocalRandom.Rand(3) != 0) {
                                ArabMode = min(ArabMode, 2);
                            }
                            if (RobotUse(ROBOT_USE_MILD_SABOTAGE) && ArabTrust >= 3 && LocalRandom.Rand(3) != 0) {
                                ArabMode = min(ArabMode, 3);
                            }

                            ArabActive = FALSE;
                            ArabPlane = Sim.Players.Players[dislike].Planes.GetRandomUsedIndex(&LocalRandom);
                        }
                        break;

                    case 1:
                        ArabOpfer2 = dislike;
                        ArabMode2 = min(4, ArabTrust);
                        if (pass > 2 && ArabMode2 > 0) {
                            ArabMode2 = LocalRandom.Rand(ArabMode2);
                        }

                        if (RobotUse(ROBOT_USE_MILDER_SABOTAGE) && LocalRandom.Rand(3) != 0) {
                            ArabMode2 = 1;
                        }
                        if (RobotUse(ROBOT_USE_MILD_SABOTAGE) && ArabTrust >= 3 && LocalRandom.Rand(3) != 0) {
                            ArabMode2 = min(ArabMode2, 3);
                        }
                        if (ArabMode2 == 1 && ArabTrust > ArabMode2 && rand() % 2 == 0) {
                            ArabMode2++;
                        }
                        if (ArabMode2 == 2 && (Sim.Players.Players[ArabOpfer2].HasItem(ITEM_LAPTOP) == 0)) {
                            ArabMode2--;
                        }
                        break;

                    case 2:
                        if (Sim.Players.Players[dislike].Planes.GetNumUsed() > 0) {
                            ArabOpfer3 = dislike;
                            ArabMode3 = min(5, ArabTrust);
                            if (pass > 2 && ArabMode3 > 0) {
                                ArabMode3 = LocalRandom.Rand(ArabMode3);
                            }

                            if (RobotUse(ROBOT_USE_MILDER_SABOTAGE) && LocalRandom.Rand(3) != 0) {
                                ArabMode3 = 1;
                            }
                            if (ArabMode3 == 5) {
                                ArabPlane = Sim.Players.Players[dislike].Planes.GetRandomUsedIndex(&LocalRandom);
                            }
                        }
                        break;
                    default:
                        AT_Log("Player.cpp: Default case should not be reached.");
                        DebugBreak();
                    }

                    if (ArabMode != 0 && Money - SabotagePrice[ArabMode] < DEBT_LIMIT) {
                        ArabMode = 0;
                    }
                    if (ArabMode2 != 0 && Money - SabotagePrice2[ArabMode2] < DEBT_LIMIT) {
                        ArabMode2 = 0;
                    }
                    if (ArabMode3 != 0 && Money - SabotagePrice3[ArabMode3] < DEBT_LIMIT) {
                        ArabMode3 = 0;
                    }

                    // Wegen Security-Office:
                    if (ArabMode == 1 && ((Sim.Players.Players[ArabOpfer].SecurityFlags & (1 << 6)) != 0U)) {
                        ArabMode = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode == 2 && ((Sim.Players.Players[ArabOpfer].SecurityFlags & (1 << 6)) != 0U)) {
                        ArabMode = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode == 3 && ((Sim.Players.Players[ArabOpfer].SecurityFlags & (1 << 7)) != 0U)) {
                        ArabMode = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode == 4 && ((Sim.Players.Players[ArabOpfer].SecurityFlags & (1 << 7)) != 0U)) {
                        ArabMode = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode == 5 && ((Sim.Players.Players[ArabOpfer].SecurityFlags & (1 << 6)) != 0U)) {
                        ArabMode = 0;
                        SecurityAnnoiance++;
                    }

                    if (ArabMode2 == 1 && ((Sim.Players.Players[ArabOpfer2].SecurityFlags & (1 << 0)) != 0U)) {
                        ArabMode2 = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode2 == 2 && ((Sim.Players.Players[ArabOpfer2].SecurityFlags & (1 << 1)) != 0U)) {
                        ArabMode2 = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode2 == 3 && ((Sim.Players.Players[ArabOpfer2].SecurityFlags & (1 << 0)) != 0U)) {
                        ArabMode2 = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode2 == 4 && ((Sim.Players.Players[ArabOpfer2].SecurityFlags & (1 << 2)) != 0U)) {
                        ArabMode2 = 0;
                        SecurityAnnoiance++;
                    }

                    if (ArabMode3 == 1 && ((Sim.Players.Players[ArabOpfer3].SecurityFlags & (1 << 8)) != 0U)) {
                        ArabMode3 = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode3 == 2 && ((Sim.Players.Players[ArabOpfer3].SecurityFlags & (1 << 5)) != 0U)) {
                        ArabMode3 = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode3 == 3 && ((Sim.Players.Players[ArabOpfer3].SecurityFlags & (1 << 5)) != 0U)) {
                        ArabMode3 = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode3 == 4 && ((Sim.Players.Players[ArabOpfer3].SecurityFlags & (1 << 3)) != 0U)) {
                        ArabMode3 = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode3 == 5 && ((Sim.Players.Players[ArabOpfer3].SecurityFlags & (1 << 8)) != 0U)) {
                        ArabMode3 = 0;
                        SecurityAnnoiance++;
                    }
                    if (ArabMode3 == 6 && ((Sim.Players.Players[ArabOpfer3].SecurityFlags & (1 << 4)) != 0U)) {
                        ArabMode3 = 0;
                        SecurityAnnoiance++;
                    }

                    // Wegen Security-Office II:
                    if (ArabMode == 1) {
                        Sim.Players.Players[ArabOpfer].SecurityNeeded |= (1 << 6);
                    }
                    if (ArabMode == 2) {
                        Sim.Players.Players[ArabOpfer].SecurityNeeded |= (1 << 6);
                    }
                    if (ArabMode == 3) {
                        Sim.Players.Players[ArabOpfer].SecurityNeeded |= (1 << 7);
                    }
                    if (ArabMode == 4) {
                        Sim.Players.Players[ArabOpfer].SecurityNeeded |= (1 << 7);
                    }
                    if (ArabMode == 5) {
                        Sim.Players.Players[ArabOpfer].SecurityNeeded |= (1 << 6);
                    }

                    if (ArabMode2 == 1) {
                        Sim.Players.Players[ArabOpfer2].SecurityNeeded |= (1 << 0);
                    }
                    if (ArabMode2 == 2) {
                        Sim.Players.Players[ArabOpfer2].SecurityNeeded |= (1 << 1);
                    }
                    if (ArabMode2 == 3) {
                        Sim.Players.Players[ArabOpfer2].SecurityNeeded |= (1 << 0);
                    }
                    if (ArabMode2 == 4) {
                        Sim.Players.Players[ArabOpfer2].SecurityNeeded |= (1 << 2);
                    }

                    if (ArabMode3 == 1) {
                        Sim.Players.Players[ArabOpfer3].SecurityNeeded |= (1 << 8);
                    }
                    if (ArabMode3 == 2) {
                        Sim.Players.Players[ArabOpfer3].SecurityNeeded |= (1 << 5);
                    }
                    if (ArabMode3 == 3) {
                        Sim.Players.Players[ArabOpfer3].SecurityNeeded |= (1 << 5);
                    }
                    if (ArabMode3 == 4) {
                        Sim.Players.Players[ArabOpfer3].SecurityNeeded |= (1 << 3);
                    }
                    if (ArabMode3 == 5) {
                        Sim.Players.Players[ArabOpfer3].SecurityNeeded |= (1 << 8);
                    }
                    if (ArabMode3 == 6) {
                        Sim.Players.Players[ArabOpfer3].SecurityNeeded |= (1 << 4);
                    }

                    if ((ArabMode != 0) || (ArabMode2 != 0) || (ArabMode3 != 0)) {
                        break;
                    }
                }

                TEAKRAND temp;
                if ((Sim.ItemZange != 0) && (Sim.Date & 1) == 0 && ArabMode == 0 && ArabMode2 == 0 && ArabMode3 == 0 && SecurityAnnoiance >= 2 &&
                    Airport.GetRandomTypedRune(RUNE_2SHOP, ROOM_SECURITY, true, &temp) != XY(-9999, -9999)) {
                    BuyItem(ITEM_ZANGE);
                    if (HasItem(ITEM_ZANGE) != 0) {
                        Sim.ItemZange = 0;
                        SIM::SendSimpleMessage(ATNET_TAKETHING, 0, ITEM_ZANGE);
                        RobotActions[1].ActionId = ACTION_VISITSECURITY2;
                    }
                }

                // Kosten verbuchen
                SLONG mode = 0;
                SLONG preis = 0;
                if (ArabMode != 0) {
                    mode = ArabMode;
                    preis = SabotagePrice[mode - 1];
                } else if (ArabMode2 != 0) {
                    mode = ArabMode2;
                    preis = SabotagePrice2[mode - 1];
                } else if (ArabMode3 != 0) {
                    mode = ArabMode3;
                    preis = SabotagePrice3[mode - 1];
                }
                if (mode != 0) {
                    ChangeMoney(-preis, 2080, "");
                    DoBodyguardRabatt(preis);
                    ArabTrust = min(6, mode + 1);
                }

                WorkCountdown = 20 * 10;
            } else {
                WorkCountdown = 20 * 5;
            }
        } else {
            WorkCountdown = 20 * 5;
        }
        break;

        // Umschulden, Aktien kaufen:
    case ACTION_VISITBANK:
        if (RobotUse(ROBOT_USE_HIGHSHAREPRICE)) {
            Dividende = 25;
        } else if (LocalRandom.Rand(10) == 0) {
            if (LocalRandom.Rand(5) == 0) {
                Dividende++;
            }
            if (LocalRandom.Rand(10) == 0) {
                Dividende++;
            }

            if (Dividende < 5) {
                Dividende = 5;
            }
            if (Dividende > 25) {
                Dividende = 25;
            }
        }

        // Kredite
        {
            if (RobotUse(ROBOT_USE_PAYBACK_CREDIT)) {
                if (Money > 750000 && Sim.Date > 1 && !RobotUse(ROBOT_USE_MAXKREDIT)) {
                    SLONG m = min(0x7fffffff, min(Credit, Money - 250000));
                    ChangeMoney(-m, 2004, "");
                    Credit -= m;
                }
            } else {
                SLONG limit = CalcCreditLimit() / 1000 * 1000;
                SLONG limitNPC = 1000000 + Sim.Date * 50000;
                if ((RobotUse(ROBOT_USE_ALT_KREDIT) || RobotUse(ROBOT_USE_MAXKREDIT)) && Credit < limitNPC) {
                    SLONG m = min(limit, limitNPC - Credit);
                    if (m > 0) {
                        ChangeMoney(m, 2003, "");
                        Credit += m;
                    }
                } else if (Money > 1500000 && Credit > 0 && !RobotUse(ROBOT_USE_ALT_KREDIT) && !RobotUse(ROBOT_USE_MAXKREDIT)) {
                    SLONG m = min(limit, min(Credit, Money - 1500000));
                    if (m > 0) {
                        ChangeMoney(-m, 2004, "");
                        Credit -= m;
                    }
                } else if (Money < 1000000 && !RobotUse(ROBOT_USE_ALT_KREDIT)) {
                    SLONG m = min(limit, 1400000 - Money);
                    if (m > 0) {
                        ChangeMoney(m, 2003, "");
                        Credit += m;
                    }
                }
            }
        }

        if (RobotUse(ROBOT_USE_MAX20PERCENT) && OwnsAktien[PlayerNum] * 100 / AnzAktien > BTARGET_MEINANTEIL && Kurse[0] >= BTARGET_KURS) {
            SLONG Sells = OwnsAktien[PlayerNum] - AnzAktien * BTARGET_MEINANTEIL / 100;

            if (Sells > 0) {
                TradeStock(PlayerNum, -Sells);
            }
        }
        if ((Credit > 1000000 && RobotUse(ROBOT_USE_SELLSHARES)) || Credit > 3000000) {
            if (RobotUse(ROBOT_USE_EAGER_SELLSHARES) || Money < -100000) {
                for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                    if (OwnsAktien[c] != 0) {
                        if (c != PlayerNum || Sim.Date > 10) {
                            SLONG Sells = min(OwnsAktien[c], 20000);

                            if (c != PlayerNum || Sim.Date > 20 || OwnsAktien[c] - Sells > AnzAktien / 2) {
                                TradeStock(c, -Sells);
                            }
                        }
                    }
                    if (Money > 2000000) {
                        break;
                    }
                }
            }
        }
        if ((SavesForPlane == 0) && (SavesForRocket == 0) && !RobotUse(ROBOT_USE_DONTBUYANYSHARES) &&
            ((Money > 3000000 && Credit < 1000000) || (Money > 2000000 && Kurse[0] < Dividende / 2))) {
            if (dislike != -1) {
                SLONG Anz = Sim.Players.Players[dislike].AnzAktien;
                for (SLONG c = 0; c < 4; c++) {
                    Anz -= Sim.Players.Players[c].OwnsAktien[dislike];
                }

                if (Kurse[0] < Dividende / 2) {
                    Anz = min(Anz, SLONG((Money - 2000000) / Sim.Players.Players[dislike].Kurse[0] / 100 * 100));
                } else {
                    Anz = min(Anz, SLONG((Money - 3000000) / Sim.Players.Players[dislike].Kurse[0] / 100 * 100));
                }

                if (Anz != 0) {
                    TradeStock(dislike, Anz);

                    if (dislike == Sim.localPlayer && (Sim.Players.Players[Sim.localPlayer].HasBerater(BERATERTYP_INFO) >= rnd.Rand(100))) {
                        Sim.Players.Players[Sim.localPlayer].Messages.AddMessage(
                            BERATERTYP_INFO, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 9005), (LPCTSTR)NameX, (LPCTSTR)AirlineX, Anz));
                    }
                }
            }
        }
        // ggf. eigene Aktien kaufen
        if (!RobotUse(ROBOT_USE_PAYBACK_CREDIT) && !RobotUse(ROBOT_USE_DONTBUYANYSHARES) && !RobotUse(ROBOT_USE_MAX20PERCENT)) {
            if ((Money > 3000000 && (Credit < 1000000 || RobotUse(ROBOT_USE_ALT_KREDIT)) && RobotUse(ROBOT_USE_BUYOWNSHARES)) ||
                (Money > 6000000 && (Credit < 1000000 || RobotUse(ROBOT_USE_ALT_KREDIT))) || (Money > 2000000 && Kurse[0] < Dividende / 2)) {
                SLONG Anz = AnzAktien;
                for (SLONG c = 0; c < 4; c++) {
                    Anz -= Sim.Players.Players[c].OwnsAktien[PlayerNum];
                }

                if (Kurse[0] < Dividende / 2) {
                    Anz = min(Anz, SLONG((Money - 2000000) / Kurse[0] / 100 * 100));
                } else {
                    Anz = min(Anz, SLONG((Money - 3000000) / Kurse[0] / 100 * 100));
                }

                if (Anz != 0) {
                    TradeStock(PlayerNum, Anz);
                }
            }
        }
        WorkCountdown = 20 * 5;
        break;

    case ACTION_VISITMECH:
        if (RobotUse(ROBOT_USE_REPAIRPLANES)) {
            SLONG c = 0;
            SLONG FreeMoney = min(0x7fffffff, Money);

            for (c = 0; c < Planes.AnzEntries(); c++) {
                if (Planes.IsInAlbum(c) != 0) {
                    Planes[c].TargetZustand = min(Planes[c].WorstZustand + 19, 100);

                    while (Planes[c].TargetZustand < 100 && FreeMoney > 300000) {
                        if (FreeMoney > 3000000 && Planes[c].TargetZustand < 92) {
                            Planes[c].TargetZustand = min(Planes[c].TargetZustand + 15, 100);
                            FreeMoney -= 3000000;
                        } else {
                            Planes[c].TargetZustand = min(Planes[c].TargetZustand + 3, 100);
                            FreeMoney -= 300000;
                        }
                    }
                }
            }

            MechMode = 3;
            break;
        } else if (RobotUse(ROBOT_USE_GOODPLANES)) {
            SLONG c = 0;

            for (c = 0; c < Planes.AnzEntries(); c++) {
                if (Planes.IsInAlbum(c) != 0) {
                    Planes[c].TargetZustand = min(Planes[c].TargetZustand + 3, 100);
                }
            }

            MechMode = 3;
            break;
        } else {
            SLONG c = 0;

            for (c = 0; c < Planes.AnzEntries(); c++) {
                if (Planes.IsInAlbum(c) != 0) {
                    if (RobotUse(ROBOT_USE_BARELY_REPAIR)) {
                        Planes[c].TargetZustand = min(Planes[c].TargetZustand + 2, 70);
                    } else {
                        Planes[c].TargetZustand = min(Planes[c].TargetZustand + 2, 100);
                    }
                }
            }

            if (MechMode == 2) {
                MechMode = 3;
            }
            break;
        }
        WorkCountdown = 20 * 5;
        break;

    case ACTION_VISITNASA:
        for (c = 0; c < 10; c++) {
            if ((RocketFlags & (1 << c)) == 0) {
                if (Sim.Difficulty == DIFF_FINAL) {
                    if (RocketPrices[c] < Money) {
                        ChangeMoney(-RocketPrices[c], 3400, "");

                        PlayFanfare();
                        RocketFlags |= (1 << c);

                        SavesForRocket = FALSE;
                    }
                } else if (Sim.Difficulty == DIFF_ADDON10) {
                    if (StationPrices[c] < Money) {
                        ChangeMoney(-StationPrices[c], 3400, "");

                        PlayFanfare();
                        RocketFlags |= (1 << c);

                        SavesForRocket = FALSE;
                    }
                }
                break;
            }
        }
        WorkCountdown = 20 * 5;
        break;

    case ACTION_VISITTELESCOPE:
    case ACTION_VISITRICK:
    case ACTION_VISITKIOSK:
        WorkCountdown = 20 * 5;
        break;

    case ACTION_VISITMUSEUM:
        if ((BuyBigPlane != 0) && !RobotUse(ROBOT_USE_GROSSESKONTO)) {
            Sim.UpdateUsedPlanes();
            for (SLONG d = 0; d < 3; d++) {
                for (c = 0; c < 3; c++) {
                    if (Sim.UsedPlanes[0x1000000 + c].Name.GetLength() > 0 && Sim.UsedPlanes[0x1000000 + c].Baujahr > 1960 &&
                        Sim.UsedPlanes[0x1000000 + c].Zustand > 40 && Sim.UsedPlanes[0x1000000 + c].CalculatePrice() < Money + 1000000 &&
                        Sim.UsedPlanes[0x1000000 + c].ptReichweite >= BuyBigPlane)
                    // if (Sim.UsedPlanes[0x1000000+c].Name.GetLength()>0 && Sim.UsedPlanes[0x1000000+c].Baujahr>1960 && Sim.UsedPlanes[0x1000000+c].Zustand>40
                    // && Sim.UsedPlanes[0x1000000+c].CalculatePrice()<Money+1000000 && PlaneTypes[Sim.UsedPlanes[0x1000000+c].TypeId].Reichweite>=BuyBigPlane)
                    {
                        if (Sim.Players.Players[Sim.localPlayer].HasBerater(BERATERTYP_INFO) >= rnd.Rand(100)) {
                            Sim.Players.Players[Sim.localPlayer].Messages.AddMessage(BERATERTYP_INFO, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 9000),
                                                                                                              (LPCTSTR)NameX, (LPCTSTR)AirlineX,
                                                                                                              Sim.UsedPlanes[0x1000000 + c].CalculatePrice()));
                        }

                        if (Planes.GetNumFree() == 0) {
                            Planes.ReSize(Planes.AnzEntries() + 10);
                        }
                        Sim.UsedPlanes[0x1000000 + c].WorstZustand = Sim.UsedPlanes[0x1000000 + c].Zustand - 20;
                        // Sim.UsedPlanes[0x1000000+c].MaxBegleiter = SLONG(PlaneTypes
                        // [Sim.UsedPlanes[0x1000000+c].TypeId].AnzBegleiter*Planes.GetAvgBegleiter());
                        Sim.UsedPlanes[0x1000000 + c].MaxBegleiter = SLONG(Sim.UsedPlanes[0x1000000 + c].ptAnzBegleiter * Planes.GetAvgBegleiter());
                        Sim.UsedPlanes[0x1000000 + c].SitzeTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_KOMFORT));
                        Sim.UsedPlanes[0x1000000 + c].EssenTarget = 0;
                        Sim.UsedPlanes[0x1000000 + c].TablettsTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_KOMFORT));
                        Sim.UsedPlanes[0x1000000 + c].DecoTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_KOMFORT));
                        Sim.UsedPlanes[0x1000000 + c].TriebwerkTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_TECH));
                        Sim.UsedPlanes[0x1000000 + c].ReifenTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_TECH));
                        Sim.UsedPlanes[0x1000000 + c].ElektronikTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_TECH));
                        Sim.UsedPlanes[0x1000000 + c].SicherheitTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_TECH));

                        Planes += Sim.UsedPlanes[0x1000000 + c];
                        ChangeMoney(-Sim.UsedPlanes[0x1000000 + c].CalculatePrice(),
                                    2010, // Kauf des Flugzeuges
                                    Sim.UsedPlanes[0x1000000 + c].Name);
                        Sim.UsedPlanes[0x1000000 + c].Name.Empty();

                        Sim.TickMuseumRefill = 0;
                        BuyBigPlane = 0;
                        SavesForPlane = FALSE;
                        break;
                    }
                }
            }

            WorkCountdown = 20 * 10;
        } else {
            WorkCountdown = 20 * 5;
        }
        break;

    case ACTION_VISITDUTYFREE:
        if (Sim.LaptopSoldTo == -1 && LaptopQuality < 4 && Sim.Date > DAYS_WITHOUT_LAPTOP && Money > 100000) {
            Sim.LaptopSoldTo = PlayerNum;
            LaptopQuality++;
            if (HasItem(ITEM_LAPTOP) == 0) {
                BuyItem(ITEM_LAPTOP);
            }
        }
        if ((HasItem(ITEM_HANDY) == 0) && Sim.Date > PlayerNum && Money > 100000) {
            BuyItem(ITEM_HANDY);
        }
        WorkCountdown = 20 * 5;
        break;

    case ACTION_EMITSHARES: {
        SLONG NeueAktien = (MaxAktien - AnzAktien) / 100 * 100;
        SLONG MarktAktien = NeueAktien * 8 / 10;
        auto AlterKurs = Kurse[0];
        auto EmissionsKurs = SLONG(Kurse[0] - 3);

        __int64 EmissionsWert = __int64(MarktAktien) * EmissionsKurs;
        __int64 EmissionsGebuehr = __int64(NeueAktien) * EmissionsKurs / 10 / 100 * 100;

        ChangeMoney(EmissionsWert, 3162, "");
        ChangeMoney(-EmissionsGebuehr, 3160, "");

        Kurse[0] = (Kurse[0] * __int64(AnzAktien) + EmissionsWert) / (AnzAktien + MarktAktien);
        if (Kurse[0] < 0) {
            Kurse[0] = 0;
        }

        // Entschädigung +/-
        auto kursDiff = (AlterKurs - Kurse[0]);
        ChangeMoney(-SLONG((AnzAktien - OwnsAktien[PlayerNum]) * kursDiff), 3161, "");
        for (c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
            if (c != PlayerNum) {
                auto entschaedigung = SLONG(Sim.Players.Players[c].OwnsAktien[PlayerNum] * kursDiff);
                Sim.Players.Players[c].ChangeMoney(entschaedigung, 3163, "");
            }
        }

        AnzAktien += NeueAktien;
        OwnsAktien[PlayerNum] += (NeueAktien - MarktAktien);

        if (Sim.Players.Players[Sim.localPlayer].HasBerater(BERATERTYP_INFO) >= rnd.Rand(100)) {
            Sim.Players.Players[Sim.localPlayer].Messages.AddMessage(
                BERATERTYP_INFO, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 9004), (LPCTSTR)NameX, (LPCTSTR)AirlineX, NeueAktien));
        }

        if (RobotUse(ROBOT_USE_REBUYSHARES)) {
            // Direkt wieder die Hälfte aufkaufen:
            TradeStock(PlayerNum, NeueAktien / 2);
        }

        if (RobotUse(ROBOT_USE_MAX20PERCENT) && OwnsAktien[PlayerNum] * 100 / AnzAktien > BTARGET_MEINANTEIL && Kurse[0] >= BTARGET_KURS) {
            SLONG Sells = OwnsAktien[PlayerNum] - AnzAktien * BTARGET_MEINANTEIL / 100;

            if (Sells > 0) {
                TradeStock(PlayerNum, -Sells);
            }
        }
    }
        WorkCountdown = 20 * 6;
        break;

        // Last-Minute
    case ACTION_CHECKAGENT1:
        if (DoRoutes == 0) {
            LastMinuteAuftraege.RefillForLastMinute();

            NetGenericAsync(17000 + Sim.Date * 100, PlayerNum, PlayerNum);
            NetGenericAsync(17010 + Sim.Date * 100, Planes.AnzEntries(), PlayerNum);
            for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
                if (Planes.IsInAlbum(c) != 0) {
                    SLONG d = 0;
                    SLONG e = 0;
                    SLONG VonCity = 0;
                    SLONG VonZeit = 0;

                    NetGenericAsync(17001 + Sim.Date * 100, c, PlayerNum);
                    for (level = 0; level <= 2; level++) {
                        VonCity = Planes[c].Flugplan.StartCity;
                        VonZeit = Sim.GetHour() + 2;

                        if (Sim.Date == 0 && Sim.GetHour() == 9) {
                            VonZeit = Sim.GetHour() + 2 + PlayerNum / 2;
                        }

                        for (d = Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
                            if (Planes[c].Flugplan.Flug[d].ObjectType != 0) {
                                VonCity = Planes[c].Flugplan.Flug[d].NachCity;
                                VonZeit = Planes[c].Flugplan.Flug[d].Landezeit + 1;
                                VonZeit += (Planes[c].Flugplan.Flug[d].Landedate - Sim.Date) * 24;
                                break;
                            }
                        }

                        if (VonZeit < Sim.GetHour() + 2) {
                            VonZeit = Sim.GetHour() + 2;
                        }

                        NetGenericAsync(17002 + Sim.Date * 100, VonZeit, PlayerNum);
                        if (VonZeit >= 0 && VonZeit < 23 + 24) {
                            if (level == 0 || level == 1) {
                                for (e = LastMinuteAuftraege.AnzEntries() - 1; e >= 0; e--) {
                                    if ((LastMinuteAuftraege[e].Praemie > 0) && LastMinuteAuftraege[e].VonCity == ULONG(VonCity)) {
                                        NetGenericAsync(17003 + Sim.Date * 100, e, PlayerNum);
                                        if (RobotUse(ROBOT_USE_SHORTFLIGHTS) && Planes.GetNumUsed() == 4 &&
                                            (Cities.CalcDistance(LastMinuteAuftraege[e].VonCity, LastMinuteAuftraege[e].NachCity) > 3000000 ||
                                             LastMinuteAuftraege[e].Personen <= 90)) {
                                            continue;
                                        }

                                        SLONG Cost = ((CalculateFlightCost(VonCity, LastMinuteAuftraege[e].VonCity, 8000, 700, -1)) + 99) / 100 * 100;

                                        NetGenericAsync(17004 + Cost * 100, e, PlayerNum);
                                        if (Cost <= LastMinuteAuftraege[e].Praemie * 8 / 10) {
                                            if ((!RobotUse(ROBOT_USE_TUTORIALLIMIT)) ||
                                                Statistiken[STAT_AUFTRAEGE].GetAtPastDay(0) < Sim.Date * 2 + 3 + (PlayerNum & 1)) {
                                                NetGenericAsync(17005 + Cost * 100, c + d + e + level, PlayerNum);

                                                if (Auftraege.GetNumFree() < 2) {
                                                    Auftraege.ReSize(Auftraege.AnzEntries() + 1);
                                                }
                                                SLONG ObjectId = (Auftraege += LastMinuteAuftraege[e]);
                                                Statistiken[STAT_AUFTRAEGE].AddAtPastDay(1);
                                                Statistiken[STAT_LMAUFTRAEGE].AddAtPastDay(1);
                                                LastMinuteAuftraege[e].Praemie = 0;

                                                CFlugplanEintrag &fpe = Planes[c].Flugplan.Flug[Planes[c].Flugplan.Flug.AnzEntries() - 1];

                                                fpe.ObjectType = 2;
                                                fpe.ObjectId = ObjectId;
                                                fpe.Okay = 0;
                                                fpe.Startdate = Sim.Date + VonZeit / 24;
                                                fpe.Startzeit = VonZeit % 24;

                                                fpe.FlightChanged();
                                                fpe.CalcPassengers(PlayerNum, Planes[c]);
                                                // fpe.CalcPassengers (Planes[c].TypeId, PlayerNum, Planes[c]);
                                                fpe.PArrived = 0;

                                                Planes[c].Flugplan.UpdateNextFlight();
                                                Planes[c].Flugplan.UpdateNextStart();

                                                Planes[c].CheckFlugplaene(PlayerNum);
                                                UpdateAuftragsUsage();
                                                DelayFlightsIfNecessary();
                                                break;
                                            }
                                        }
                                    }
                                }
                            } else if (level == 2) {
                                SLONG Min = 999999999;
                                SLONG MinE = -1;

                                for (e = LastMinuteAuftraege.AnzEntries() - 1; e >= 0; e--) {
                                    if ((LastMinuteAuftraege[e].Praemie > 0) &&
                                        VonZeit + Cities.CalcFlugdauer(VonCity, LastMinuteAuftraege[e].VonCity, Planes[c].ptGeschwindigkeit) < 23 + 24) {
                                        // VonZeit+Cities.CalcFlugdauer (VonCity, LastMinuteAuftraege[e].VonCity,
                                        // PlaneTypes[Planes[c].TypeId].Geschwindigkeit)<23+24)
                                        if (Cities.CalcDistance(VonCity, LastMinuteAuftraege[e].VonCity) < Min) {
                                            if (RobotUse(ROBOT_USE_SHORTFLIGHTS) && Planes.GetNumUsed() == 4 &&
                                                (Cities.CalcDistance(LastMinuteAuftraege[e].VonCity, LastMinuteAuftraege[e].NachCity) > 3000000 ||
                                                 LastMinuteAuftraege[e].Personen <= 90)) {
                                                continue;
                                            }

                                            Min = Cities.CalcDistance(VonCity, LastMinuteAuftraege[e].VonCity);
                                            MinE = e;
                                        }
                                    }
                                }

                                if (MinE != -1 && Min < 15000000 && (Min < 6000000 || VonZeit < 34) && (Min < 4000000 || VonZeit < 24)) {
                                    e = MinE;

                                    SLONG Cost = ((CalculateFlightCost(VonCity, LastMinuteAuftraege[e].VonCity, 8000, 700, -1)) + 99) / 100 * 100;
                                    if (Cost <= LastMinuteAuftraege[e].Praemie) {
                                        if ((!RobotUse(ROBOT_USE_TUTORIALLIMIT)) ||
                                            Statistiken[STAT_AUFTRAEGE].GetAtPastDay(0) < Sim.Date * 2 + 3 + (PlayerNum & 1)) {
                                            if (Auftraege.GetNumFree() < 2) {
                                                Auftraege.ReSize(Auftraege.AnzEntries() + 1);
                                            }
                                            SLONG ObjectId = (Auftraege += LastMinuteAuftraege[e]);
                                            Statistiken[STAT_AUFTRAEGE].AddAtPastDay(1);
                                            Statistiken[STAT_LMAUFTRAEGE].AddAtPastDay(1);
                                            LastMinuteAuftraege[e].Praemie = 0;

                                            CFlugplanEintrag &fpe = Planes[c].Flugplan.Flug[Planes[c].Flugplan.Flug.AnzEntries() - 1];

                                            fpe.ObjectType = 2;
                                            fpe.ObjectId = ObjectId;
                                            fpe.Okay = 0;
                                            fpe.Startdate = Sim.Date + VonZeit / 24;
                                            fpe.Startzeit = VonZeit % 24;

                                            fpe.FlightChanged();
                                            fpe.CalcPassengers(PlayerNum, Planes[c]);
                                            // fpe.CalcPassengers (Planes[c].TypeId, PlayerNum, Planes[c]);
                                            fpe.PArrived = 0;

                                            Planes[c].Flugplan.UpdateNextFlight();
                                            Planes[c].Flugplan.UpdateNextStart();

                                            Planes[c].CheckFlugplaene(PlayerNum);
                                            UpdateAuftragsUsage();
                                            DelayFlightsIfNecessary();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            LastMinuteAuftraege.RefillForLastMinute();
        }
        WorkCountdown = 20 * 7;
        TimeReiseburo = Sim.Time;
        break;

        // Reisebüro:
    case ACTION_CHECKAGENT2:
        if (DoRoutes == 0) {
            ReisebueroAuftraege.RefillForReisebuero();
            for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
                if (Planes.IsInAlbum(c) != 0) {
                    SLONG d = 0;
                    SLONG e = 0;
                    SLONG VonCity = 0;
                    SLONG VonZeit = 0;

                    for (level = 0; level <= 2; level++) {
                        VonCity = Planes[c].Flugplan.StartCity;
                        VonZeit = Sim.GetHour() + 2;

                        if (Sim.Date == 0 && Sim.GetHour() == 9) {
                            VonZeit = Sim.GetHour() + 2 + PlayerNum / 2;
                        }

                        for (d = Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
                            if (Planes[c].Flugplan.Flug[d].ObjectType != 0) {
                                VonCity = Planes[c].Flugplan.Flug[d].NachCity;
                                VonZeit = Planes[c].Flugplan.Flug[d].Landezeit + 1;
                                VonZeit += (Planes[c].Flugplan.Flug[d].Landedate - Sim.Date) * 24;
                                break;
                            }
                        }

                        if (VonZeit < Sim.GetHour() + 2) {
                            VonZeit = Sim.GetHour() + 2;
                        }

                        if (VonZeit >= 0 && VonZeit < 23 + 24 + 24) {
                            if (level == 0 || level == 1) {
                                for (e = ReisebueroAuftraege.AnzEntries() - 1; e >= 0; e--) {
                                    if ((ReisebueroAuftraege[e].Praemie > 0) && ReisebueroAuftraege[e].VonCity == ULONG(VonCity)) {
                                        if (RobotUse(ROBOT_USE_SHORTFLIGHTS) && Planes.GetNumUsed() == 4 &&
                                            (Cities.CalcDistance(ReisebueroAuftraege[e].VonCity, ReisebueroAuftraege[e].NachCity) > 3000000 ||
                                             ReisebueroAuftraege[e].Personen <= 90)) {
                                            continue;
                                        }

                                        SLONG Cost = ((CalculateFlightCost(VonCity, ReisebueroAuftraege[e].VonCity, 8000, 700, -1)) + 99) / 100 * 100;
                                        if (Cost <= ReisebueroAuftraege[e].Praemie * 8 / 10) {
                                            if ((!RobotUse(ROBOT_USE_TUTORIALLIMIT)) ||
                                                Statistiken[STAT_AUFTRAEGE].GetAtPastDay(0) < Sim.Date * 2 + 3 + (PlayerNum & 1)) {
                                                if (Auftraege.GetNumFree() < 2) {
                                                    Auftraege.ReSize(Auftraege.AnzEntries() + 1);
                                                }
                                                SLONG ObjectId = (Auftraege += ReisebueroAuftraege[e]);
                                                ReisebueroAuftraege[e].Praemie = 0;

                                                if (Sim.Difficulty != DIFF_ADDON09 || NumOrderFlightsToday2 >= 5) {
                                                    Statistiken[STAT_AUFTRAEGE].AddAtPastDay(1);
                                                }

                                                NumOrderFlightsToday2++;

                                                CFlugplanEintrag &fpe = Planes[c].Flugplan.Flug[Planes[c].Flugplan.Flug.AnzEntries() - 1];

                                                fpe.ObjectType = 2;
                                                fpe.ObjectId = ObjectId;
                                                fpe.Okay = 0;
                                                fpe.Startdate = Sim.Date + VonZeit / 24;
                                                fpe.Startzeit = VonZeit % 24;

                                                fpe.FlightChanged();
                                                fpe.CalcPassengers(PlayerNum, Planes[c]);
                                                // fpe.CalcPassengers (Planes[c].TypeId, PlayerNum, Planes[c]);
                                                fpe.PArrived = 0;

                                                Planes[c].Flugplan.UpdateNextFlight();
                                                Planes[c].Flugplan.UpdateNextStart();

                                                Planes[c].CheckFlugplaene(PlayerNum);
                                                UpdateAuftragsUsage();
                                                DelayFlightsIfNecessary();
                                                break;
                                            }
                                        }
                                    }
                                }
                            } else if (level == 2) {
                                SLONG Min = 999999999;
                                SLONG MinE = -1;

                                for (e = ReisebueroAuftraege.AnzEntries() - 1; e >= 0; e--) {
                                    if ((ReisebueroAuftraege[e].Praemie > 0) &&
                                        VonZeit + Cities.CalcFlugdauer(VonCity, ReisebueroAuftraege[e].VonCity, Planes[c].ptGeschwindigkeit) < 23 + 24) {
                                        // VonZeit+Cities.CalcFlugdauer (VonCity, ReisebueroAuftraege[e].VonCity,
                                        // PlaneTypes[Planes[c].TypeId].Geschwindigkeit)<23+24)
                                        if (Cities.CalcDistance(VonCity, ReisebueroAuftraege[e].VonCity) < Min) {
                                            if (RobotUse(ROBOT_USE_SHORTFLIGHTS) && Planes.GetNumUsed() == 4 &&
                                                (Cities.CalcDistance(ReisebueroAuftraege[e].VonCity, ReisebueroAuftraege[e].NachCity) > 3000000 ||
                                                 ReisebueroAuftraege[e].Personen <= 90)) {
                                                continue;
                                            }

                                            Min = Cities.CalcDistance(VonCity, ReisebueroAuftraege[e].VonCity);
                                            MinE = e;
                                        }
                                    }
                                }

                                if (MinE != -1 && Min < 15000000 && (Min < 6000000 || VonZeit < 34) && (Min < 3000000 || VonZeit < 24)) {
                                    e = MinE;

                                    SLONG Cost = ((CalculateFlightCost(VonCity, ReisebueroAuftraege[e].VonCity, 8000, 700, -1)) + 99) / 100 * 100;
                                    if (Cost <= ReisebueroAuftraege[e].Praemie) {
                                        if ((!RobotUse(ROBOT_USE_TUTORIALLIMIT)) ||
                                            Statistiken[STAT_AUFTRAEGE].GetAtPastDay(0) < Sim.Date * 2 + 3 + (PlayerNum & 1)) {
                                            if (Auftraege.GetNumFree() < 2) {
                                                Auftraege.ReSize(Auftraege.AnzEntries() + 1);
                                            }
                                            SLONG ObjectId = (Auftraege += ReisebueroAuftraege[e]);
                                            ReisebueroAuftraege[e].Praemie = 0;

                                            if (Sim.Difficulty != DIFF_ADDON09 || NumOrderFlightsToday2 >= 5) {
                                                Statistiken[STAT_AUFTRAEGE].AddAtPastDay(1);
                                            }

                                            NumOrderFlightsToday2++;

                                            CFlugplanEintrag &fpe = Planes[c].Flugplan.Flug[Planes[c].Flugplan.Flug.AnzEntries() - 1];

                                            fpe.ObjectType = 2;
                                            fpe.ObjectId = ObjectId;
                                            fpe.Okay = 0;
                                            fpe.Startdate = Sim.Date + VonZeit / 24;
                                            fpe.Startzeit = VonZeit % 24;

                                            fpe.FlightChanged();
                                            fpe.CalcPassengers(PlayerNum, Planes[c]);
                                            // fpe.CalcPassengers (Planes[c].TypeId, PlayerNum, Planes[c]);
                                            fpe.PArrived = 0;

                                            Planes[c].Flugplan.UpdateNextFlight();
                                            Planes[c].Flugplan.UpdateNextStart();

                                            Planes[c].CheckFlugplaene(PlayerNum);
                                            UpdateAuftragsUsage();
                                            DelayFlightsIfNecessary();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ReisebueroAuftraege.RefillForReisebuero();
        }
        WorkCountdown = 20 * 7;
        TimeReiseburo = Sim.Time;
        break;

        // Frachtbüro:
    case ACTION_CHECKAGENT3:
        if (RobotUse(ROBOT_USE_SHORTFLIGHTS) && Planes.GetNumUsed() == 4) {
            break;
        }

        if (DoRoutes == 0) {
            SLONG Bewertungsbonus = 0;
            if (RobotUse(ROBOT_USE_MUCH_FRACHT) && RobotUse(ROBOT_USE_MUCH_FRACHT_BONUS)) {
                Bewertungsbonus = 150000;
            }

            gFrachten.Refill();
            for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
                if (Planes.IsInAlbum(c) != 0) {
                    SLONG d = 0;
                    SLONG e = 0;
                    SLONG VonCity = 0;
                    SLONG VonZeit = 0;

                    if ((Planes[c].ptPassagiere / 10) <= 0) {
                        continue;
                    }

                    for (level = 0; level <= 2; level++) {
                        VonCity = Planes[c].Flugplan.StartCity;
                        VonZeit = Sim.GetHour() + 2;

                        if (Sim.Date == 0 && Sim.GetHour() == 9) {
                            VonZeit = Sim.GetHour() + 2 + PlayerNum / 2;
                        }

                        for (d = Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
                            if (Planes[c].Flugplan.Flug[d].ObjectType != 0) {
                                VonCity = Planes[c].Flugplan.Flug[d].NachCity;
                                VonZeit = Planes[c].Flugplan.Flug[d].Landezeit + 1;
                                VonZeit += (Planes[c].Flugplan.Flug[d].Landedate - Sim.Date) * 24;
                                break;
                            }
                        }

                        if (VonZeit < Sim.GetHour() + 2) {
                            VonZeit = Sim.GetHour() + 2;
                        }

                        if (VonZeit >= 0 && VonZeit < 23 + 24 + 24) {
                            if (level == 0 || level == 1) {
                                for (e = gFrachten.AnzEntries() - 1; e >= 0; e--) {
                                    CFracht &qFracht = gFrachten[e];

                                    if (qFracht.Praemie >= 0 && qFracht.VonCity == ULONG(VonCity)) {
                                        SLONG Cost = ((CalculateFlightCost(qFracht.VonCity, qFracht.NachCity, 8000, 700, -1)) + 99) / 100 * 100;

                                        // Multiplikator, weil wir mehrere Flüge durchführen müssen:
                                        int trips = (qFracht.Tons / (Planes[c].ptPassagiere / 10) + 1);
                                        Cost = Cost * 2 * trips;
                                        // Cost = Cost*2 * (qFracht.Tons / (PlaneTypes[Planes[c].TypeId].Passagiere/10) + 1);

                                        if ((trips < 4) && ((Cost <= qFracht.Praemie * 8 / 10 + Bewertungsbonus) ||
                                                            (RobotUse(ROBOT_USE_FREE_FRACHT) && qFracht.Praemie == 0))) {
                                            if (Frachten.GetNumFree() < 2) {
                                                Frachten.ReSize(Frachten.AnzEntries() + 1);
                                            }
                                            Statistiken[STAT_FRACHTEN].AddAtPastDay(1);
                                            SLONG ObjectId = (Frachten += qFracht);
                                            qFracht.Praemie = -1;

                                            while (qFracht.TonsOpen > 0) {
                                                CFlugplanEintrag &fpe = Planes[c].Flugplan.Flug[Planes[c].Flugplan.Flug.AnzEntries() - 1];

                                                fpe.ObjectType = 4;
                                                fpe.ObjectId = ObjectId;
                                                fpe.Okay = 0;
                                                fpe.Startdate = Sim.Date + VonZeit / 24;
                                                fpe.Startzeit = VonZeit % 24;

                                                fpe.FlightChanged();
                                                fpe.PArrived = 0;

                                                Planes[c].Flugplan.UpdateNextFlight();
                                                Planes[c].Flugplan.UpdateNextStart();

                                                Planes[c].CheckFlugplaene(PlayerNum);
                                                DelayFlightsIfNecessary();

                                                Sim.Players.CheckFlighplans();
                                                qFracht.TonsOpen -= Planes[c].ptPassagiere / 10;
                                            }

                                            break;
                                        }
                                    }
                                }
                            } else if (level == 2) {
                                SLONG MinMin = 0; // Min muß mindestens so groß sein

                                while (true) {
                                    SLONG Min = 999999999;
                                    SLONG MinE = -1;

                                    for (e = gFrachten.AnzEntries() - 1; e >= 0; e--) {
                                        if (gFrachten[e].Praemie >= 0 &&
                                            VonZeit + Cities.CalcFlugdauer(VonCity, gFrachten[e].VonCity, Planes[c].ptGeschwindigkeit) < 23 + 24) {
                                            if (Cities.CalcDistance(VonCity, gFrachten[e].VonCity) < Min &&
                                                Cities.CalcDistance(VonCity, gFrachten[e].VonCity) > MinMin) {
                                                Min = Cities.CalcDistance(VonCity, gFrachten[e].VonCity);
                                                MinE = e;
                                            }
                                        }
                                    }

                                    if (MinE == -1) {
                                        break;
                                    }
                                    MinMin = Min;

                                    if (MinE != -1 && Min < 15000000 && (Min < 6000000 || VonZeit < 34 + 24) && (Min < 3000000 || VonZeit < 24 + 24)) {
                                        e = MinE;

                                        CFracht &qFracht = gFrachten[e];

                                        SLONG Cost = ((CalculateFlightCost(qFracht.VonCity, qFracht.NachCity, 8000, 700, -1)) + 99) / 100 * 100;

                                        // Multiplikator, weil wir mehrere Flüge durchführen müssen:
                                        int trips = (qFracht.Tons / (Planes[c].ptPassagiere / 10) + 1);
                                        Cost = Cost * trips;

                                        // Anflug:
                                        Cost += ((CalculateFlightCost(VonCity, qFracht.VonCity, 8000, 700, -1)) + 99) / 100 * 100;

                                        if ((trips < 4) &&
                                            ((Cost <= qFracht.Praemie + Bewertungsbonus) || (RobotUse(ROBOT_USE_FREE_FRACHT) && qFracht.Praemie == 0))) {
                                            if (Frachten.GetNumFree() < 2) {
                                                Frachten.ReSize(Frachten.AnzEntries() + 1);
                                            }
                                            Statistiken[STAT_FRACHTEN].AddAtPastDay(1);
                                            SLONG ObjectId = (Frachten += qFracht);
                                            qFracht.Praemie = -1;

                                            while (qFracht.TonsOpen > 0) {
                                                CFlugplanEintrag &fpe = Planes[c].Flugplan.Flug[Planes[c].Flugplan.Flug.AnzEntries() - 1];

                                                fpe.ObjectType = 4;
                                                fpe.ObjectId = ObjectId;
                                                fpe.Okay = 0;
                                                fpe.Startdate = Sim.Date + VonZeit / 24;
                                                fpe.Startzeit = VonZeit % 24;

                                                fpe.FlightChanged();
                                                fpe.PArrived = 0;

                                                Planes[c].Flugplan.UpdateNextFlight();
                                                Planes[c].Flugplan.UpdateNextStart();

                                                Planes[c].CheckFlugplaene(PlayerNum);
                                                DelayFlightsIfNecessary();

                                                Sim.Players.CheckFlighplans();
                                                qFracht.TonsOpen -= Planes[c].ptPassagiere / 10;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            gFrachten.Refill();
        }
        WorkCountdown = 20 * 7;
        TimeReiseburo = Sim.Time;
        break;

    case ACTION_BUYUSEDPLANE:
        Sim.UpdateUsedPlanes();
        if ((Planes.GetNumUsed() < 3 || !RobotUse(ROBOT_USE_GROSSESKONTO)) && (Planes.GetNumUsed() < 5 || !RobotUse(ROBOT_USE_MAX5PLANES)) &&
            (Planes.GetNumUsed() < 4 || !RobotUse(ROBOT_USE_MAX4PLANES)) && (Planes.GetNumUsed() < 10 || !RobotUse(ROBOT_USE_MAX10PLANES))) {
            for (c = 0; c < 3; c++) {
                if (Sim.UsedPlanes[0x1000000 + c].Name.GetLength() > 0 && Sim.UsedPlanes[0x1000000 + c].Baujahr > 1950 &&
                    Sim.UsedPlanes[0x1000000 + c].Zustand > 65 && Sim.UsedPlanes[0x1000000 + c].CalculatePrice() < Money - 1000000) {
                    if (Sim.Players.Players[Sim.localPlayer].HasBerater(BERATERTYP_INFO) >= rnd.Rand(100)) {
                        Sim.Players.Players[Sim.localPlayer].Messages.AddMessage(
                            BERATERTYP_INFO,
                            bprintf(StandardTexte.GetS(TOKEN_ADVICE, 9000), (LPCTSTR)NameX, (LPCTSTR)AirlineX, Sim.UsedPlanes[0x1000000 + c].CalculatePrice()));
                    }

                    if (Planes.GetNumFree() == 0) {
                        Planes.ReSize(Planes.AnzEntries() + 10);
                    }
                    Sim.UsedPlanes[0x1000000 + c].WorstZustand = Sim.UsedPlanes[0x1000000 + c].Zustand - 20;
                    // Sim.UsedPlanes[0x1000000+c].MaxBegleiter = SLONG(PlaneTypes [Sim.UsedPlanes[0x1000000+c].TypeId].AnzBegleiter*Planes.GetAvgBegleiter());
                    Sim.UsedPlanes[0x1000000 + c].MaxBegleiter = SLONG(Sim.UsedPlanes[0x1000000 + c].ptAnzBegleiter * Planes.GetAvgBegleiter());
                    Sim.UsedPlanes[0x1000000 + c].SitzeTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_KOMFORT));
                    Sim.UsedPlanes[0x1000000 + c].EssenTarget = 0;
                    Sim.UsedPlanes[0x1000000 + c].TablettsTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_KOMFORT));
                    Sim.UsedPlanes[0x1000000 + c].DecoTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_KOMFORT));
                    Sim.UsedPlanes[0x1000000 + c].TriebwerkTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_TECH));
                    Sim.UsedPlanes[0x1000000 + c].ReifenTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_TECH));
                    Sim.UsedPlanes[0x1000000 + c].ElektronikTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_TECH));
                    Sim.UsedPlanes[0x1000000 + c].SicherheitTarget = 1 + static_cast<SLONG>(RobotUse(ROBOT_USE_UPGRADE_TECH));

                    Planes += Sim.UsedPlanes[0x1000000 + c];
                    ChangeMoney(-Sim.UsedPlanes[0x1000000 + c].CalculatePrice(),
                                2010, // Kauf des Flugzeuges
                                Sim.UsedPlanes[0x1000000 + c].Name);
                    Sim.UsedPlanes[0x1000000 + c].Name.Empty();

                    Sim.TickMuseumRefill = 0;
                    SavesForPlane = FALSE;
                    break;
                }
            }
        }
        WorkCountdown = 20 * 7;
        bWasInMuseumToday = TRUE;
        break;

    case ACTION_VISITAUFSICHT:
        if (OutOfGates > SLONG(Planes.GetNumUsed())) // Gates erwerben
        {
            SLONG Cheapest = 0;

            for (c = 0; c < 7; c++) {
                if ((TafelData.Gate[c].ZettelId > -1) && TafelData.Gate[c].Player != PlayerNum) {
                    break;
                }
            }

            if (c < 7) {
                SLONG n = -1;
                Cheapest = 99999999;
                for (c = 0; c < 7; c++) {
                    if ((TafelData.Gate[c].ZettelId > -1) && TafelData.Gate[c].Player != PlayerNum &&
                        (TafelData.Gate[c].Preis < Cheapest || TafelData.Gate[c].Player == dislike || RobotUse(ROBOT_ALWAYS_BUY_GATES))) {
                        Cheapest = TafelData.Gate[c].Preis;
                        n = c;
                    }
                }

                if (n != -1) {
                    if (TafelData.Gate[n].Player == Sim.localPlayer && (Sim.Players.Players[Sim.localPlayer].HasBerater(BERATERTYP_INFO) >= rnd.Rand(100))) {
                        Sim.Players.Players[Sim.localPlayer].Messages.AddMessage(
                            BERATERTYP_INFO, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 9001), (LPCTSTR)NameX, (LPCTSTR)AirlineX));
                    }
                    TafelData.Gate[n].Preis += TafelData.Gate[n].Preis / 10;
                    TafelData.Gate[n].Player = PlayerNum;
                }
            }
        }

        // Niederlassung erwerben:
        if (((SavesForPlane == 0) && (SavesForRocket == 0)) || Sim.Date < 5 || LocalRandom.Rand(25) == 0 ||
            (RobotUse(ROBOT_USE_BUY_MORE_ABROAD) && LocalRandom.Rand(3) == 0)) {
            for (c = 0; c < 7; c++) {
                if (TafelData.City[c].Player != PlayerNum && TafelData.City[c].ZettelId > -1 &&
                    RentCities.RentCities[Cities(TafelData.City[c].ZettelId)].Rang == 0) {
                    if (((TafelData.City[c].Player != -1 && (Sympathie[TafelData.City[c].Player] < 40 || RobotUse(ROBOT_USE_BUY_MORE_ABROAD) ||
                                                             LocalRandom.Rand(10) == 0 || (Sim.Date > 10 && LocalRandom.Rand(5) == 0))) ||
                         (TafelData.City[c].Player == -1 && LocalRandom.Rand(3) == 0)) &&
                        BilanzGestern.GetSumme() > TafelData.City[c].Preis * 10 && Credit * 2 < Money * 3 && Money > 0 &&
                        ((TafelData.Route[c].Player == dislike || RobotUse(ROBOT_USE_BUY_MORE_ABROAD) || RobotUse(ROBOT_USE_ABROAD)))) {
                        if (TafelData.City[c].Player == Sim.localPlayer &&
                            (Sim.Players.Players[Sim.localPlayer].HasBerater(BERATERTYP_INFO) >= rnd.Rand(100))) {
                            Sim.Players.Players[Sim.localPlayer].Messages.AddMessage(
                                BERATERTYP_INFO, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 9002), (LPCTSTR)NameX, (LPCTSTR)AirlineX,
                                                         (LPCTSTR)Cities[TafelData.City[c].ZettelId].Name));
                        }
                        TafelData.City[c].Preis += TafelData.City[c].Preis / 10;
                        TafelData.City[c].Player = PlayerNum;
                    }
                }
            }
        }

        TimeAufsicht = Sim.Time;
        WorkCountdown = 20 * 7;
        break;

    case ACTION_VISITROUTEBOX:
        do {
            SLONG c = 0;
            SLONG Anz = 0;
            BUFFER_V<BOOL> IsBuyable;

            // Mal schauen wie unser Routen/Flugzeug Verhältnis aussieht:
            for (c = 0; c < RentRouten.RentRouten.AnzEntries(); c++) {
                if (Routen.IsInAlbum(c) != 0) {
                    if (RentRouten.RentRouten[c].Rang != 0U) {
                        Anz++;
                        if (RentRouten.RentRouten[c].TageMitGering > 1) {
                            break;
                        }
                    }
                }
            }
            if (c < RentRouten.RentRouten.AnzEntries()) {
                break; // Nicht solange eine Route krieselt
            }

            // Neue Route kaufen:
            if ((DoRoutes != 0) && SLONG(Planes.GetNumUsed()) > (Anz / 2) * 3 / 2 && Anz < SLONG(Routen.GetNumUsed())) {
                IsBuyable.ReSize(Routen.AnzEntries());
                IsBuyable.FillWith(0);

                // Mal sehen, was wir kaufen können:
                for (SLONG d = Routen.AnzEntries() - 1; d >= 0; d--) {
                    if ((Routen.IsInAlbum(d) != 0) && RentRouten.RentRouten[d].Rang == 0) {
                        if (Routen[d].VonCity == static_cast<ULONG>(Sim.HomeAirportId) || Routen[d].NachCity == static_cast<ULONG>(Sim.HomeAirportId)) {
                            IsBuyable[d] = TRUE;
                        }
                    }
                }
                for (SLONG c = Routen.AnzEntries() - 1; c >= 0; c--) {
                    if (Routen.IsInAlbum(c) != 0) {
                        if (RentRouten.RentRouten[c].RoutenAuslastung >= 20) {
                            for (SLONG d = Routen.AnzEntries() - 1; d >= 0; d--) {
                                if ((Routen.IsInAlbum(d) != 0) && RentRouten.RentRouten[d].Rang == 0) {
                                    if (Routen[c].VonCity == Routen[d].VonCity || Routen[c].VonCity == Routen[d].NachCity ||
                                        Routen[c].NachCity == Routen[d].VonCity || Routen[c].NachCity == Routen[d].NachCity) {
                                        IsBuyable[d] = TRUE;
                                    }
                                }
                            }
                        }
                    }
                }
                for (SLONG d = Routen.AnzEntries() - 1; d >= 0; d--) {
                    if ((Routen.IsInAlbum(d) != 0) && RentRouten.RentRouten[d].Rang == 0 && RentRouten.RentRouten[d].TageMitGering < 7) {
                        IsBuyable[d] = FALSE;
                    }
                }

                SLONG Best = 0;
                SLONG BestC = -1;

                // Beste Route aussuchen:
                for (c = 0; c < RentRouten.RentRouten.AnzEntries(); c++) {
                    if (IsBuyable[c] != 0) {
                        // Ist die Route für die Mission wichtig?
                        if (RobotUse(ROBOT_USE_ROUTEMISSION)) {
                            SLONG d = 0;

                            for (d = 0; d < 6; d++) {
                                if ((Routen[c].VonCity == static_cast<ULONG>(Sim.HomeAirportId) &&
                                     Routen[c].NachCity == static_cast<ULONG>(Sim.MissionCities[d])) ||
                                    (Routen[c].NachCity == static_cast<ULONG>(Sim.HomeAirportId) &&
                                     Routen[c].VonCity == static_cast<ULONG>(Sim.MissionCities[d]))) {
                                    BestC = c;
                                    if (LocalRandom.Rand(2) == 0) {
                                        break;
                                    }
                                }
                            }
                            if (d < 6) {
                                break;
                            }
                        } else if (BestC == -1 || RentRouten.RentRouten[c].Miete / Routen[c].AnzPassagiere() < Best) {
                            Best = RentRouten.RentRouten[c].Miete / Routen[c].AnzPassagiere();
                            BestC = c;
                        }
                    }
                }

                if (BestC != -1) {
                    SLONG Rang = 1;

                    // hprintf ("Event: %s (Player %li) buys route %s-%s.", (LPCTSTR)NameX, PlayerNum+1, (LPCTSTR)Cities[Routen[BestC].VonCity].Name,
                    // (LPCTSTR)Cities[Routen[BestC].NachCity].Name);

                    for (SLONG d = 0; d < 4; d++) {
                        if ((Sim.Players.Players[d].IsOut == 0) && Sim.Players.Players[d].RentRouten.RentRouten[BestC].Rang >= Rang) {
                            Rang = Sim.Players.Players[d].RentRouten.RentRouten[BestC].Rang + 1;
                        }
                    }

                    RentRoute(Routen[BestC].VonCity, Routen[BestC].NachCity, Routen[BestC].Miete);

                    RentRouten.RentRouten[BestC].Rang = UBYTE(Rang);
                    RentRouten.RentRouten[BestC].TageMitGering = 0;

                    for (c = 0; c < RentRouten.RentRouten.AnzEntries(); c++) {
                        if ((Routen.IsInAlbum(c) != 0) && Routen[c].VonCity == Routen[BestC].NachCity && Routen[c].NachCity == Routen[BestC].VonCity) {
                            RentRouten.RentRouten[c].Rang = UBYTE(Rang);
                            RentRouten.RentRouten[c].TageMitGering = 0;
                            break;
                        }
                    }
                }
            }
        } while (false);
        WorkCountdown = 20 * 7;
        break;

    case ACTION_VISITSECURITY: {
        for (SLONG pass = 1; pass <= 2; pass++) {
            if (Money > 1000000) {
                ULONG newflag = (1 << LocalRandom.Rand(9));

                if ((SecurityNeeded & newflag) != 0U) {
                    SecurityFlags |= newflag;
                }
            } else {
                SecurityFlags &= ~(1 << LocalRandom.Rand(9));
            }
        }
    }
        WorkCountdown = 20 * 7;
        break;

    case ACTION_VISITSECURITY2:
        if (HasItem(ITEM_ZANGE) != 0) {
            DropItem(ITEM_ZANGE);

            Sim.nSecOutDays = 3;
            SIM::SendSimpleMessage(ATNET_SYNC_OFFICEFLAG, 0, 55, 3);

            for (SLONG c = 0; c < 4; c++) {
                if (Sim.Players.Players[c].IsOut == 0) {
                    Sim.Players.Players[c].SecurityFlags = 0;
                    Sim.Players.Players[c].NetSynchronizeFlags();
                }
            }
        }
        WorkCountdown = 20 * 1;
        break;

    case ACTION_VISITDESIGNER:
        if (RobotUse(ROBOT_USE_DESIGNER_BUY)) {
            CString fn = FullFilename(CString(bprintf("pl%li.pln", Sim.Difficulty)), MiscPath);
            if (DoesFileExist(fn) != 0) {
                CXPlane plane;
                plane.Load(fn);

                while (Money - plane.CalcCost() > 6000000) {
                    TEAKRAND rnd;
                    rnd.SRand(Sim.Date);

                    // Kauf des Flugzeuges:
                    BuyPlane(plane, &rnd);
                }
            }
        }
        WorkCountdown = 20 * 7;
        break;

    case ACTION_RAISEMONEY:
        WorkCountdown = 20 * 7;
        break;

    case ACTION_WERBUNG:
        if (!RobotUse(ROBOT_USE_GROSSESKONTO)) {
            if ((DoRoutes != 0) && Money > 500000) {
                for (SLONG c = 0; c < RentRouten.RentRouten.AnzEntries(); c++) {
                    if (RentRouten.RentRouten[c].Rang != 0U) {
                        if (RentRouten.RentRouten[c].Image < 70 || (RentRouten.RentRouten[c].Image < 80 && (SavesForPlane == 0) && (SavesForRocket == 0)) ||
                            (Money > 2000000 && RentRouten.RentRouten[c].Image < 100 && (SavesForPlane == 0) && (SavesForRocket == 0))) {
                            RentRouten.RentRouten[c].Image += UBYTE(gWerbePrice[6 + 3] / 2000);
                            Limit(static_cast<UBYTE>(0), RentRouten.RentRouten[c].Image, static_cast<UBYTE>(100));

                            ChangeMoney(-gWerbePrice[6 + 3], 3 + 3120, "");
                            break;
                        }
                    }
                }
            }
            if ((((Image < 0 || (((DoRoutes != 0) || (WantToDoRoutes != 0)) && Image < 300)) && Money > 1500000 && (SavesForPlane == 0) &&
                  (SavesForRocket == 0)) ||
                 (Money > 150000 && RobotUse(ROBOT_USE_MUCHWERBUNG) &&
                  (Image + 10 < Sim.Players.Players[(PlayerNum + 1) % 3].Image || (dislike != -1 && Image + 10 < Sim.Players.Players[dislike].Image)))) ||
                (Image < 1000 && Money - Credit > 4000000 && (SavesForPlane == 0) && (SavesForRocket == 0))) {
                if (((DoRoutes != 0) || (WantToDoRoutes != 0)) || RobotUse(ROBOT_USE_HARDWERBUNG)) {
                    if (Sim.Date % 4 == 0) {
                        n = 0;
                    } else {
                        if (Money <= 1800000) {
                            n = 1;
                        } else if (Money <= 2500000) {
                            n = 2;
                        } else if (Money <= 3000000) {
                            n = 3;
                        } else if (Money <= 4000000) {
                            n = 4;
                        } else {
                            n = 5;
                        }
                    }
                } else {
                    if (Money <= 1800000 || (Sim.Date % 3) == 0) {
                        n = 0;
                    } else if (Image > -20) {
                        n = 1;
                    } else if (Image > -40) {
                        n = 2;
                    } else if (Image > -60) {
                        n = 3;
                    } else {
                        n = 4;
                    }

                    if (Planes.GetNumUsed() > 5 && (Sim.Date % 3) != 0) {
                        n = min(4, n + 1);
                    }
                }

                Image += gWerbePrice[n] / 10000 * (n + 6) / 55;
                Limit(SLONG(-1000), Image, SLONG(1000));

                if (n == 0) {
                    for (c = 0; c < Sim.Players.AnzPlayers; c++) {
                        if (Sim.Players.Players[c].Owner == 0 && (Sim.Players.Players[c].IsOut == 0)) {
                            Sim.Players.Players[c].Letters.AddLetter(
                                TRUE, CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 9900), (LPCTSTR)AirlineX)),
                                CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 9901), (LPCTSTR)AirlineX)),
                                CString(bprintf(StandardTexte.GetS(TOKEN_LETTER, 9902), (LPCTSTR)NameX, (LPCTSTR)AirlineX)), -1);
                        }
                    }
                }

                ChangeMoney(-gWerbePrice[n], n + 3120, "");
            }
        }
        WorkCountdown = 20 * 7;
        break;

    default:
        DebugBreak();
    }

    // NetGenericSync (110, PlayerNum);
    // NetGenericSync (111, TimeReiseburo);
    // NetGenericSync (112, TimeBuro);
    // NetGenericSync (113, TimeAufsicht);
    // NetGenericSync (114, TimePersonal);

    if (RobotUse(ROBOT_USE_WORKQUICK_2) && WorkCountdown > 2) {
        WorkCountdown /= 2;
    }

    if (RobotUse(ROBOT_USE_WORKVERYQUICK) && WorkCountdown > 4) {
        WorkCountdown /= 4;
    } else if (RobotUse(ROBOT_USE_WORKQUICK) && WorkCountdown > 2) {
        WorkCountdown /= 2;
    }

    Sim.Players.CheckFlighplans();

    RobotActions[0].ActionId = ACTION_NONE;

    // Die exakte Zeit des Ausführens auf dem Server simulieren (Ende):
    if (Sim.bNetwork != 0) {
        Sim.Time = RealLocalTime;
    }

    NetGenericSync(680 + PlayerNum, RobotActions[0].ActionId);
}

//--------------------------------------------------------------------------------------------
// Ist es okay, diesen Spieler anzurufen?
//--------------------------------------------------------------------------------------------
BOOL PLAYER::IsOkayToCallThisPlayer() {
    if (Sim.GetHour() < 9 || Sim.GetHour() > 17 || (Sim.CallItADay != 0) || (CallItADay != 0)) {
        return (FALSE);
    }

    if (HasItem(ITEM_HANDY) == 0) {
        return (FALSE);
    }
    if (LocationWin == nullptr) {
        return (FALSE);
    }
    if (IsStuck != 0) {
        return (FALSE);
    }
    if (TelephoneDown != 0) {
        return (FALSE);
    }

    if (((LocationWin)->IsDialogOpen() != 0) || ((LocationWin)->MenuIsOpen() != 0)) {
        return (FALSE);
    }

    if (Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)].StatePar != 0) {
        return (FALSE);
    }

    return (TRUE);
}

//--------------------------------------------------------------------------------------------
// Verzögert Flüge um Gates auszulasten:
//--------------------------------------------------------------------------------------------
void PLAYER::DelayFlightsIfNecessary() {
    SLONG c = 0;
    SLONG d = 0;

    if (Owner != 1 || (IsOut != 0)) {
        return; // War Irtum, kein Computerspieler
    }

    PlanGates();
    for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
        if (Planes.IsInAlbum(c) == 0) {
            continue;
        }
        CFlugplan &qPlan = Planes[c].Flugplan;
        SLONG NumDelays = 0;

    Again:
        // for (d=qPlan.Flug.AnzEntries()-1; d>=0; d--)
        for (d = 0; d < qPlan.Flug.AnzEntries(); d++) {
            auto &qFPE = qPlan.Flug[d];
            if ((qFPE.ObjectType == 1 || qFPE.ObjectType == 2) &&
                (qFPE.VonCity == static_cast<ULONG>(Sim.HomeAirportId) || qFPE.NachCity == static_cast<ULONG>(Sim.HomeAirportId)) && qFPE.Gate == -1 &&
                NumDelays < 6) {
                if (qFPE.Startdate > Sim.Date || (qFPE.Startdate == Sim.Date && qFPE.Startzeit > Sim.GetHour() + 2)) {

                    qFPE.Startzeit++;
                    while (qFPE.Startzeit >= 24) {
                        qFPE.Startzeit -= 24;
                        qFPE.Startdate++;
                    }

                    NumDelays++;

                    Planes[c].CheckFlugplaene(PlayerNum);
                    PlanGates();
                    goto Again;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Der Berater gibt Nachrichten für zwischendurch aus:
//--------------------------------------------------------------------------------------------
void PLAYER::RandomBeraterMessage() {
    SLONG Which = rand() % 3;

    if ((HasBerater(BERATERTYP_PERSONAL) != 0) && Which == 0) {
        SLONG c = 0;
        SLONG d = 0;
        SLONG n = 0;

        d = -1;

        for (c = n = 0; c < Workers.Workers.AnzEntries(); c++) {
            if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Happyness < -40) {
                if (Workers.Workers[c].WarnedToday == 0) {
                    d = c;
                }
                n++;
            }
        }

        if (d != -1) {
            if (n > 10) {
                for (c = n = 0; c < Workers.Workers.AnzEntries(); c++) {
                    if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Happyness < -40) {
                        Workers.Workers[c].WarnedToday = TRUE;
                    }
                }

                Messages.AddMessage(BERATERTYP_PERSONAL, StandardTexte.GetS(TOKEN_ADVICE, 2299));
            } else {
                Workers.Workers[d].WarnedToday = TRUE;

                if (Workers.Workers[d].Typ < WORKER_PILOT) {
                    Messages.AddMessage(BERATERTYP_PERSONAL,
                                        bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2200 + Workers.Workers[d].Geschlecht * 10), Workers.Workers[d].Name.c_str()));
                }
                if (Workers.Workers[d].Typ == WORKER_PILOT) {
                    Messages.AddMessage(BERATERTYP_PERSONAL,
                                        bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2201 + Workers.Workers[d].Geschlecht * 10), Workers.Workers[d].Name.c_str()));
                }
                if (Workers.Workers[d].Typ == WORKER_STEWARDESS) {
                    Messages.AddMessage(BERATERTYP_PERSONAL,
                                        bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2202 + Workers.Workers[d].Geschlecht * 10), Workers.Workers[d].Name.c_str()));
                }
            }
        }
    } else if ((HasBerater(BERATERTYP_KEROSIN) != 0) && Which == 1) {
        if (Sim.Kerosin > 600 && (TankOpen == 0) && TankInhalt > 0) {
            // Tank aufmachen!
            Messages.AddMessage(BERATERTYP_KEROSIN, StandardTexte.GetS(TOKEN_ADVICE, 3011));
        } else if (Sim.Kerosin < 400 && TankInhalt > Tank / 2 && Money > 20000 && Tank < SLONG(Planes.GetNumUsed() * 4000)) {
            // Tanks kaufen:
            Messages.AddMessage(BERATERTYP_KEROSIN, StandardTexte.GetS(TOKEN_ADVICE, 3012));
        } else if (Sim.Kerosin < 470 && Sim.KerosinPast[9] > Sim.KerosinPast[8] && Sim.KerosinPast[7] > Sim.KerosinPast[8] &&
                   Sim.KerosinPast[6] > Sim.KerosinPast[7] && Sim.KerosinPast[5] > Sim.KerosinPast[6] && TankInhalt < Tank / 3 && Money > 20000) {
            // Kerosin kaufen:
            Messages.AddMessage(BERATERTYP_KEROSIN, StandardTexte.GetS(TOKEN_ADVICE, 3010));
        }
    } else if ((HasBerater(BERATERTYP_FLUGZEUG) != 0) && Which == 2) {
        SLONG c = 0;
        SLONG Anz = 0;
        CString PlaneList;
        CString Comment;

        if (MechMode == 0 || MechMode == 1 || MechMode == 2) {
            for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
                if (Planes.IsInAlbum(c) != 0) {
                    if (Planes[c].Zustand < 30) {
                        Anz++;
                        if (PlaneList.GetLength() > 0) {
                            PlaneList += ", ";
                        }
                        PlaneList += Planes[c].Name;
                    }
                }
            }

            if (Anz != 0) {
                if (Anz > 1) {
                    Comment = bprintf(StandardTexte.GetS(TOKEN_ADVICE, 4011), (LPCTSTR)PlaneList);
                } else {
                    Comment = bprintf(StandardTexte.GetS(TOKEN_ADVICE, 4010), (LPCTSTR)PlaneList);
                }

                if (MechMode == 0) {
                    Comment = Comment + " " + StandardTexte.GetS(TOKEN_ADVICE, 4030);
                }
                if (MechMode == 1) {
                    Comment = Comment + " " + StandardTexte.GetS(TOKEN_ADVICE, 4031);
                }
                Messages.AddMessage(BERATERTYP_FLUGZEUG, Comment);
            } else {
                for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
                    if (Planes.IsInAlbum(c) != 0) {
                        if (Planes[c].Zustand < 55) {
                            Anz++;
                            if (PlaneList.GetLength() > 0) {
                                PlaneList += ", ";
                            }
                            PlaneList += Planes[c].Name;
                        }
                    }
                }

                if (Anz != 0) {
                    if (Anz > 1) {
                        Comment = bprintf(StandardTexte.GetS(TOKEN_ADVICE, 4021), (LPCTSTR)PlaneList);
                    } else {
                        Comment = bprintf(StandardTexte.GetS(TOKEN_ADVICE, 4020), (LPCTSTR)PlaneList);
                    }

                    if (MechMode == 0) {
                        Comment = Comment + " " + StandardTexte.GetS(TOKEN_ADVICE, 4030);
                    }
                    if (MechMode == 1) {
                        Comment = Comment + " " + StandardTexte.GetS(TOKEN_ADVICE, 4031);
                    }
                    Messages.AddMessage(BERATERTYP_FLUGZEUG, Comment);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Spieler kauft einen Gegenstand:
//--------------------------------------------------------------------------------------------
void PLAYER::BuyItem(UBYTE Item) {
    if (HasSpaceForItem() != 0) {
        for (SLONG d = 0; d < 6; d++) {
            if (Items[d] == 0xff) {
                Items[d] = Item;
                break;
            }
        }

        ReformIcons();
    }
}

//--------------------------------------------------------------------------------------------
// Zeigt den Spieler mit dem Telefon in der Hand:
//--------------------------------------------------------------------------------------------
void PLAYER::DisplayAsTelefoning() const {
    PERSON &qPerson = Sim.Persons[static_cast<SLONG>(Sim.Persons.GetPlayerIndex(PlayerNum))];

    qPerson.Phase = qPerson.LookDir + 4;
    qPerson.LookDir = 8;
}

//--------------------------------------------------------------------------------------------
// Spieler verbraucht einen Gegenstand:
//--------------------------------------------------------------------------------------------
bool PLAYER::DropItem(UBYTE Item) {
    for (SLONG d = 0; d < 6; d++) {
        if (Items[d] == Item) {
            Items[d] = 0xff;
            ReformIcons();
            return (true);
        }
    }

    return (false);
}

//--------------------------------------------------------------------------------------------
// Hat der Spieler schon so etwas?
//--------------------------------------------------------------------------------------------
BOOL PLAYER::HasItem(UBYTE Item) {
    for (SLONG d = 0; d < 6; d++) {
        if (Items[d] == Item) {
            return (TRUE);
        }
    }

    return (FALSE);
}

//--------------------------------------------------------------------------------------------
// Hat der Spieler noch Platz für ein Item?
//--------------------------------------------------------------------------------------------
BOOL PLAYER::HasSpaceForItem() {
    for (SLONG d = 0; d < 6; d++) {
        if (Items[d] == 0xff) {
            return (TRUE);
        }
    }

    if (Owner == 0 && (LocationWin != nullptr) && ((*LocationWin).MenuIsOpen() == 0) && ((*LocationWin).IsDialogOpen() == 0)) {
        (LocationWin)->MenuStart(MENU_REQUEST, MENU_REQUEST_ITEMS);
        (LocationWin)->MenuSetZoomStuff(gMousePosition, 0, FALSE);
    }

    return (FALSE);
}

//--------------------------------------------------------------------------------------------
// Verteilt die Jobs an die Mitarbeiter; gibt ggf. einen Rat
//--------------------------------------------------------------------------------------------
void PLAYER::MapWorkerOverflow(BOOL Advice) {
    SLONG c = 0;
    SLONG d = 0;
    BOOL NeedAdvice = FALSE;

    // Zählung in den Flugzeugen vorbereiten:
    for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
        if (Planes.IsInAlbum(c) != 0) {
            Planes[c].AnzPiloten = 0;
            Planes[c].AnzBegleiter = 0;
        }
    }

    // Alle Angestellten durchgehen:
    for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
        if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Typ >= WORKER_PILOT) {
            if (Workers.Workers[c].PlaneId != -1 && (Planes.IsInAlbum(Workers.Workers[c].PlaneId) != 0)) {
                if (Workers.Workers[c].Typ == WORKER_PILOT) {
                    Planes[Workers.Workers[c].PlaneId].AnzPiloten++;
                }
                if (Workers.Workers[c].Typ == WORKER_STEWARDESS) {
                    Planes[Workers.Workers[c].PlaneId].AnzBegleiter++;
                }
            }
        }
    }

    // Ggf. Personen von den Flugzeugen abziehen:
    for (d = 0; d < SLONG(Planes.AnzEntries()); d++) {
        if (Planes.IsInAlbum(d) != 0) {
            // while (PlaneTypes[Planes[d].TypeId].AnzPiloten<Planes[d].AnzPiloten)
            while (Planes[d].ptAnzPiloten < Planes[d].AnzPiloten) {
                for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
                    if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Typ == WORKER_PILOT && Workers.Workers[c].PlaneId != -1) {
                        if (Workers.Workers[c].PlaneId != -1 && Planes(Workers.Workers[c].PlaneId) == static_cast<ULONG>(d)) {
                            Workers.Workers[c].PlaneId = -1;
                            Planes[d].AnzPiloten--;
                            break;
                        }
                    }
                }

                NeedAdvice = TRUE;
            }

            while (Planes[d].MaxBegleiter < Planes[d].AnzBegleiter) {
                for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
                    if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Typ == WORKER_STEWARDESS && Workers.Workers[c].PlaneId != -1) {
                        if (Workers.Workers[c].PlaneId != -1 && Planes(Workers.Workers[c].PlaneId) == static_cast<ULONG>(d)) {
                            Workers.Workers[c].PlaneId = -1;
                            Planes[d].AnzBegleiter--;
                            break;
                        }
                    }
                }

                NeedAdvice = TRUE;
            }
        }
    }

    for (d = 0; d < Planes.AnzEntries(); d++) {
        if (Planes.IsInAlbum(d) != 0) {
            Planes[d].UpdatePersonalQuality(PlayerNum);
        }
    }

    if ((NeedAdvice != 0) && (Advice != 0)) {
        Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 1260), MESSAGE_COMMENT);
    }

    if (Sim.bNetwork != 0) {
        NetUpdateWorkers();
    }
}

//--------------------------------------------------------------------------------------------
// Verteilt die Jobs an die Mitarbeiter; gibt ggf. einen Rat
//--------------------------------------------------------------------------------------------
void PLAYER::MapWorkers(BOOL Advice) {
    SLONG c = 0;
    SLONG d = 0;
    SLONG MinNeeded = 0;
    SLONG MinNeededIndex = 0;
    SLONG LastPiloten = xPiloten;
    SLONG LastBegleiter = xBegleiter;

    xPiloten = xBegleiter = 0;

    // Zählung in den Flugzeugen vorbereiten:
    for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
        if (Planes.IsInAlbum(c) != 0) {
            Planes[c].AnzPiloten = 0;
            Planes[c].AnzBegleiter = 0;
        }
    }

    // Alle Angestellten durchgehen:
    for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
        if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Typ >= WORKER_PILOT) {
            if (Workers.Workers[c].PlaneId == -1) {
                if (Workers.Workers[c].Typ == WORKER_PILOT) {
                    xPiloten++;
                }
                if (Workers.Workers[c].Typ == WORKER_STEWARDESS) {
                    xBegleiter++;
                }
            } else if (Planes.IsInAlbum(Workers.Workers[c].PlaneId) == 0) {
                Workers.Workers[c].PlaneId = -1;
                if (Workers.Workers[c].Typ == WORKER_PILOT) {
                    xPiloten++;
                }
                if (Workers.Workers[c].Typ == WORKER_STEWARDESS) {
                    xBegleiter++;
                }
            } else {
                if (Workers.Workers[c].Typ == WORKER_PILOT) {
                    Planes[Workers.Workers[c].PlaneId].AnzPiloten++;
                }
                if (Workers.Workers[c].Typ == WORKER_STEWARDESS) {
                    Planes[Workers.Workers[c].PlaneId].AnzBegleiter++;
                }
            }
        }
    }

    // Sind irgendwo mehr Stewardessen als gewünscht?:
    for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
        while ((Planes.IsInAlbum(c) != 0) && Planes[c].AnzBegleiter > Planes[c].MaxBegleiter) {
            for (d = 0; d < Workers.Workers.AnzEntries(); d++) {
                if (Workers.Workers[d].Employer == PlayerNum && Workers.Workers[d].Typ == WORKER_STEWARDESS && Workers.Workers[d].PlaneId != -1 &&
                    Planes(Workers.Workers[d].PlaneId) == ULONG(c)) {
                    Workers.Workers[d].PlaneId = -1;
                    Planes[c].AnzBegleiter--;
                    break;
                }
            }
        }
    }

again: // Zweiter Pass, wenn Stewardessen aus Luxusstellen umgebucht werden, damit Basisstellen besetzt werden

    // Die verbleibenden Angestellten an die Flugzeuge mappen:
    for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
        if (Workers.Workers[c].Employer == PlayerNum && (Workers.Workers[c].Typ == WORKER_PILOT || Workers.Workers[c].Typ == WORKER_STEWARDESS) &&
            Workers.Workers[c].PlaneId == -1) {
            MinNeeded = 999;
            MinNeededIndex = -1;

            for (d = 0; d < SLONG(Planes.AnzEntries()); d++) {
                if (Planes.IsInAlbum(d) != 0) {
                    // if ((Workers.Workers[c].Typ==WORKER_PILOT      && Planes[d].AnzPiloten<PlaneTypes[Planes[d].TypeId].AnzPiloten) ||
                    if ((Workers.Workers[c].Typ == WORKER_PILOT && Planes[d].AnzPiloten < Planes[d].ptAnzPiloten) ||
                        (Workers.Workers[c].Typ == WORKER_STEWARDESS && Planes[d].AnzBegleiter < Planes[d].ptAnzBegleiter))
                    //(Workers.Workers[c].Typ==WORKER_STEWARDESS && Planes[d].AnzBegleiter<PlaneTypes[Planes[d].TypeId].AnzBegleiter))
                    {
                        // if ((PlaneTypes[Planes[d].TypeId].AnzPiloten-Planes[d].AnzPiloten)*(Workers.Workers[c].Typ==WORKER_PILOT) +
                        // (PlaneTypes[Planes[d].TypeId].AnzBegleiter-Planes[d].AnzBegleiter)*(Workers.Workers[c].Typ==WORKER_STEWARDESS) < MinNeeded)
                        if ((Planes[d].ptAnzPiloten - Planes[d].AnzPiloten) * static_cast<SLONG>(Workers.Workers[c].Typ == WORKER_PILOT) +
                                (Planes[d].ptAnzBegleiter - Planes[d].AnzBegleiter) * static_cast<SLONG>(Workers.Workers[c].Typ == WORKER_STEWARDESS) <
                            MinNeeded) {
                            // MinNeeded      = (PlaneTypes[Planes[d].TypeId].AnzPiloten-Planes[d].AnzPiloten)*(Workers.Workers[c].Typ==WORKER_PILOT) +
                            // (PlaneTypes[Planes[d].TypeId].AnzBegleiter-Planes[d].AnzBegleiter)*(Workers.Workers[c].Typ==WORKER_STEWARDESS);
                            MinNeeded = (Planes[d].ptAnzPiloten - Planes[d].AnzPiloten) * static_cast<SLONG>(Workers.Workers[c].Typ == WORKER_PILOT) +
                                        (Planes[d].ptAnzBegleiter - Planes[d].AnzBegleiter) * static_cast<SLONG>(Workers.Workers[c].Typ == WORKER_STEWARDESS);
                            MinNeededIndex = d;
                        }
                    }
                }
            }

            // Dem Flugzeug zuweisen:
            if (MinNeededIndex != -1) {
                if (Workers.Workers[c].Typ == WORKER_PILOT) {
                    Workers.Workers[c].PlaneId = Planes.GetIdFromIndex(MinNeededIndex);
                    Planes[MinNeededIndex].AnzPiloten++;
                    xPiloten--;
                }

                // Das gleiche für Begleiter:
                if (Workers.Workers[c].Typ == WORKER_STEWARDESS) {
                    Workers.Workers[c].PlaneId = Planes.GetIdFromIndex(MinNeededIndex);
                    Planes[MinNeededIndex].AnzBegleiter++;
                    xBegleiter--;
                }
            } else // Kein Flugzeug gefunden? Für Stewardessen kann man noch was machen:
            {
                for (d = 0; d < SLONG(Planes.AnzEntries()); d++) {
                    if (Planes.IsInAlbum(d) != 0) {
                        if (Workers.Workers[c].Typ == WORKER_STEWARDESS && Planes[d].AnzBegleiter < Planes[d].MaxBegleiter) {
                            Workers.Workers[c].PlaneId = Planes.GetIdFromIndex(d);
                            Planes[d].AnzBegleiter++;
                            xBegleiter--;
                        }
                    }
                }
            }
        }
    }

    // Brauchen wir jetzt noch Begleiter? Und haben wir außerdem irgendwo mehr als unbedingt notwendig?
    {
        SLONG NeededBegleiter = 0;
        SLONG MovableBegleiter = 0;
        for (d = 0; d < SLONG(Planes.AnzEntries()); d++) {
            if (Planes.IsInAlbum(d) != 0) {
                // NeededBegleiter  += max (PlaneTypes[Planes[d].TypeId].AnzBegleiter-Planes[d].AnzBegleiter, 0);
                NeededBegleiter += max(Planes[d].ptAnzBegleiter - Planes[d].AnzBegleiter, 0);
                // MovableBegleiter += max (Planes[d].AnzBegleiter-PlaneTypes[Planes[d].TypeId].AnzBegleiter, 0);
                MovableBegleiter += max(Planes[d].AnzBegleiter - Planes[d].AnzBegleiter, 0);
            }
        }

        if (NeededBegleiter > 0 && MovableBegleiter > 0) {
            for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
                if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Typ == WORKER_STEWARDESS && Workers.Workers[c].PlaneId != -1) {
                    // if (Planes[Workers.Workers[c].PlaneId].AnzBegleiter>PlaneTypes[Planes[Workers.Workers[c].PlaneId].TypeId].AnzBegleiter)
                    if (Planes[Workers.Workers[c].PlaneId].AnzBegleiter > Planes[Workers.Workers[c].PlaneId].ptAnzBegleiter) {
                        // Personal freisetzen, damit es gemappt werden kann:
                        Planes[Workers.Workers[c].PlaneId].AnzBegleiter--;
                        Workers.Workers[c].PlaneId = -1;

                        NeededBegleiter--;
                        MovableBegleiter--;

                        if (NeededBegleiter <= 0 || MovableBegleiter <= 0) {
                            break;
                        }
                    }
                }
            }

            goto again;
        }
    }

    // Und jetzt noch die offenen Stellen zählen:
    for (d = 0; d < SLONG(Planes.AnzEntries()); d++) {
        if (Planes.IsInAlbum(d) != 0) {
            // if (PlaneTypes[Planes[d].TypeId].AnzPiloten>Planes[d].AnzPiloten)
            if (Planes[d].ptAnzPiloten > Planes[d].AnzPiloten) {
                xPiloten -= Planes[d].ptAnzPiloten - Planes[d].AnzPiloten;
            }
            // xPiloten   -= PlaneTypes[Planes[d].TypeId].AnzPiloten-Planes[d].AnzPiloten;

            if (Planes[d].MaxBegleiter > Planes[d].AnzBegleiter) {
                xBegleiter -= Planes[d].MaxBegleiter - Planes[d].AnzBegleiter;
            }
        }
    }

    if ((Advice != 0) && (HasBerater(BERATERTYP_PERSONAL) != 0)) {
        if (LastPiloten != xPiloten) {
            if (xPiloten > 0) {
                Messages.AddMessage(BERATERTYP_PERSONAL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 1200), xPiloten), MESSAGE_COMMENT);
            }
            if (xPiloten == 0) {
                Messages.AddMessage(BERATERTYP_PERSONAL, StandardTexte.GetS(TOKEN_ADVICE, 1201), MESSAGE_COMMENT);
            }
            if (xPiloten == -1) {
                Messages.AddMessage(BERATERTYP_PERSONAL, StandardTexte.GetS(TOKEN_ADVICE, 1202), MESSAGE_COMMENT);
            }
            if (xPiloten < -1) {
                Messages.AddMessage(BERATERTYP_PERSONAL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 1203), -xPiloten), MESSAGE_COMMENT);
            }
        } else if (LastBegleiter != xBegleiter) {
            if (xBegleiter > 0) {
                Messages.AddMessage(BERATERTYP_PERSONAL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 1250), xBegleiter), MESSAGE_COMMENT);
            }
            if (xBegleiter == 0) {
                Messages.AddMessage(BERATERTYP_PERSONAL, StandardTexte.GetS(TOKEN_ADVICE, 1251), MESSAGE_COMMENT);
            }
            if (xBegleiter == -1) {
                Messages.AddMessage(BERATERTYP_PERSONAL, StandardTexte.GetS(TOKEN_ADVICE, 1252), MESSAGE_COMMENT);
            }
            if (xBegleiter < -1) {
                Messages.AddMessage(BERATERTYP_PERSONAL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 1253), -xBegleiter), MESSAGE_COMMENT);
            }
        }
    }

    for (d = 0; d < Planes.AnzEntries(); d++) {
        if (Planes.IsInAlbum(d) != 0) {
            Planes[d].UpdatePersonalQuality(PlayerNum);
            NetUpdatePlaneProps(Planes.GetIdFromIndex(d));
        }
    }

    if (Sim.bNetwork != 0) {
        NetUpdateWorkers();
    }
}

//--------------------------------------------------------------------------------------------
// Aktualisiert die Werte NumPiloten und NumBegleiter:
//--------------------------------------------------------------------------------------------
void PLAYER::UpdatePilotCount() {
    SLONG c = 0;

    xPiloten = xBegleiter = 0;

    // Zählung in den Flugzeugen vorbereiten:
    for (c = 0; c < SLONG(Planes.AnzEntries()); c++) {
        if (Planes.IsInAlbum(c) != 0) {
            Planes[c].AnzPiloten = 0;
            Planes[c].AnzBegleiter = 0;
        }
    }

    // Alle Angestellten durchgehen:
    for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
        if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Typ >= WORKER_PILOT) {
            if (Workers.Workers[c].PlaneId != -1) {
                if (Planes.IsInAlbum(Workers.Workers[c].PlaneId) != 0) {
                    if (Workers.Workers[c].Typ == WORKER_PILOT) {
                        Planes[Workers.Workers[c].PlaneId].AnzPiloten++;
                    }
                    if (Workers.Workers[c].Typ == WORKER_STEWARDESS) {
                        Planes[Workers.Workers[c].PlaneId].AnzBegleiter++;
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Icons ggf. nach links verschieben:
//--------------------------------------------------------------------------------------------
void PLAYER::ReformIcons() {
    SLONG c = 0;
    SLONG d = 0;

    if (LocationWin != nullptr) {
        (LocationWin)->StatusCount = 3;
    }

    for (d = 0; d < 6; d++) {
        for (c = 0; c < 5; c++) {
            if (Items[c] == 0xff && Items[c + 1] != 0xff) {
                Items[c] = Items[c + 1];
                Items[c + 1] = 0xff;
            }
        }
    }

    for (c = 0; c < 5; c++) {
        if (Items[c] == 0xff || Items[c + 1] == 0xff) {
            break;
        }

        if (Items[c] > Items[c + 1]) {
            UBYTE Tmp = 0;

            Tmp = Items[c];
            Items[c] = Items[c + 1];
            Items[c + 1] = Tmp;
        }
    }

    NetSynchronizeItems();
}

//--------------------------------------------------------------------------------------------
// Aktualisiert die Laufgeschwindigkeit:
//--------------------------------------------------------------------------------------------
void PLAYER::UpdateWalkSpeed() {
    SLONG c = 0;

    if (Owner != 2) {
        WalkSpeed = 3;

        for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
            if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Typ == BERATERTYP_FITNESS) {
                WalkSpeed += (Workers.Workers[c].Talent - 30) / 30;
            }
        }

        Limit(SLONG(1), WalkSpeed, SLONG(5));
    }
}

//--------------------------------------------------------------------------------------------
// Läßt den Bodyguard ggf. einen Rabatt aushandeln:
//--------------------------------------------------------------------------------------------
void PLAYER::DoBodyguardRabatt(SLONG Money) {
    SLONG quality = HasBerater(BERATERTYP_SICHERHEIT);

    if (quality > 20) {
        if (GetRoom() == ROOM_SHOP1) {
            Messages.AddMessage(BERATERTYP_SICHERHEIT, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 6001), LPCTSTR(CString(Insert1000erDots(Money))), quality / 10),
                                MESSAGE_URGENT);
        } else {
            Messages.AddMessage(BERATERTYP_SICHERHEIT, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 6000), LPCTSTR(CString(Insert1000erDots(Money))), quality / 10),
                                MESSAGE_URGENT);
        }

        SLONG delta = Money / 100 * (quality / 10);
        ChangeMoney(delta, 3130, "");
        SIM::SendSimpleMessage(ATNET_BODYGUARD, 0, Sim.localPlayer, delta);
    }
}

//--------------------------------------------------------------------------------------------
// Wirft alle Arbeiter raus:
//--------------------------------------------------------------------------------------------
void PLAYER::SackWorkers() const {
    SLONG c = 0;

    for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
        if (Workers.Workers[c].Employer == PlayerNum) {
            Workers.Workers[c].Employer = WORKER_RESERVE;
            Workers.Workers[c].Gehalt = Workers.Workers[c].OriginalGehalt;
            if (Workers.Workers[c].TimeInPool > 0) {
                Workers.Workers[c].TimeInPool = 0;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// bringt Statistiken auf den neusten Stand:
//--------------------------------------------------------------------------------------------
void PLAYER::UpdateStatistics() {
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;
    __int64 value = 0;

    // STAT_KONTO:
    Statistiken[STAT_KONTO].SetAtPastDay(Money);

    // STAT_KREDIT:
    if (Money > 0) {
        Statistiken[STAT_KREDIT].SetAtPastDay(Credit); // Nur Kredit
    } else {
        Statistiken[STAT_KREDIT].SetAtPastDay(Credit - Money); // Kredit + Überziehungskredit
    }

    // STAT_FLUGZEUGE:
    Statistiken[STAT_FLUGZEUGE].SetAtPastDay(Planes.GetNumUsed());

    // STAT_PASSAGIERE: in CFlugplanEintrag::BookFlight (...
    // STAT_PASSAGIERE_HOME: in CFlugplanEintrag::BookFlight (...

    // STAT_AKTIENKURS:
    Statistiken[STAT_AKTIENKURS].SetAtPastDay(SLONG(Kurse[0]));

    // STAT_FLUEGE: in CFlugplanEintrag::BookFlight (...
    // STAT_AUFTRAEGE: in CReisebuero::OnLButtonDown (... und CLastMinute::OnLButtonDown(...
    // STAT_LMAUFTRAEGE: CLastMinute::OnLButtonDown (...

    // STAT_STRAFE: in PLAYER::UpdateAuftraege (...

    // STAT_FIRMENWERT:
    value = 0;
    for (c = 0; c < Planes.AnzEntries(); c++) { // Flugzeuge
        if (Planes.IsInAlbum(c) == 0) {
            continue;
        }

        value += Planes[c].CalculatePrice();

        // Add value of plane upgrades
        CPlane &qPlane = Planes[c];
        auto ptPassagiere = qPlane.ptPassagiere;
        value += ptPassagiere * (SeatCosts[qPlane.Sitze] - SeatCosts[0]);
        value += ptPassagiere * (TrayCosts[qPlane.Tabletts] - TrayCosts[0]);
        value += ptPassagiere * (DecoCosts[qPlane.Deco] - DecoCosts[0]);
        value += (ReifenCosts[qPlane.Reifen] - ReifenCosts[0]);
        value += (TriebwerkCosts[qPlane.Triebwerk] - TriebwerkCosts[0]);
        value += (SicherheitCosts[qPlane.Sicherheit] - SicherheitCosts[0]);
        value += (ElektronikCosts[qPlane.Elektronik] - ElektronikCosts[0]);
    }

    for (c = 0; c < 4; c++) { // Aktien
        if (OwnsAktien[c] != 0) {
            value += SLONG(OwnsAktien[c] * Sim.Players.Players[c].Kurse[0]);
        }
    }

    value += Money - Credit;

    value += TankInhalt * Sim.HoleKerosinPreis(2);         // Current value of stored kerosene (assuming worst quality)
    value += static_cast<__int64>(std::round(Tank * 600)); // Value of tank itself (assuming cheapest tank)

    // calculate 'value' of company image by how often best ad campaign you would need to buy (adCampaignSize := 5)
    value += Image * 10000 * 55 / (5 + 6);

    Statistiken[STAT_FIRMENWERT].SetAtPastDay(value);
    // Statistiken[STAT_FIRMENWERT].SetAtPastDay (0, SLONG(value));

    // STAT_UNZUFR_PASSAGIERE: in PlaneTyp.cpp
    // STAT_ZUFR_PASSAGIERE: in PlaneTyp.cpp

    // STAT_VERSPAETUNG: in SIM::DoTimeStep (...

    // STAT_UNFAELLE: in Sim::NewDay()
    // STAT_SABOTIERT: in Sim::NewDay()

    // STAT_MITARBEITER:
    if (Owner == 0 || (Owner == 1 && !RobotUse(ROBOT_USE_FAKE_PERSONAL))) {
        for (c = d = 0; c < Workers.Workers.AnzEntries(); c++) {
            if (Workers.Workers[c].Employer == PlayerNum) {
                d++;
            }
        }
        Statistiken[STAT_MITARBEITER].SetAtPastDay(d);
    } else if (Owner == 1) {
        for (c = d = 0; c < Planes.AnzEntries(); c++) {
            if (Planes.IsInAlbum(c) != 0) {
                // d+=PlaneTypes[Planes[c].TypeId].AnzPiloten;
                d += Planes[c].ptAnzPiloten;
                // d+=PlaneTypes[Planes[c].TypeId].AnzBegleiter;
                d += Planes[c].ptAnzBegleiter;
            }
        }
        Statistiken[STAT_MITARBEITER].SetAtPastDay(d);
    }
    // Owner==2 über Network

    // STAT_ZUFR_PERSONAL:
    if (Owner == 0 || (Owner == 1 && !RobotUse(ROBOT_USE_FAKE_PERSONAL))) {
        SLONG Anz = 0;

        for (c = d = 0; c < Workers.Workers.AnzEntries(); c++) {
            if (Workers.Workers[c].Employer == PlayerNum) {
                d += (Workers.Workers[c].Happyness + 100) / 2;
                Anz++;
            }
        }

        // if (Anz>0) Statistiken[STAT_ZUFR_PERSONAL].SetAtPastDay (0, d/Anz);
        if (Anz > 0) {
            Statistiken[STAT_ZUFR_PERSONAL].SetAtPastDay(d / 100);
        } else {
            Statistiken[STAT_ZUFR_PERSONAL].SetAtPastDay(0);
        }
    } else if (Owner == 1) {
        c = SLONG(Statistiken[STAT_MITARBEITER].GetAtPastDay(0));

        c = c * (min(500, Image) + 2500) / 3000;

        Statistiken[STAT_ZUFR_PERSONAL].SetAtPastDay(c);
    }
    // Owner==2 über Network

    if (Owner == 0 && (Sim.bCheatedSession == 0)) {
        Sim.AddHighscore((LPCTSTR)NameX, Sim.UniqueGameId2, Statistiken[STAT_FIRMENWERT].GetAtPastDay(0));
    }

    // STAT_MISSIONSZIEL:
    switch (Sim.Difficulty) {
    case DIFF_TUTORIAL:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(NumAuftraege * 100 / 10);
        break;

    case DIFF_FIRST:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(NumPassengers * 100 / TARGET_PASSENGERS);
        break;

    case DIFF_EASY:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(Gewinn * 100 / TARGET_GEWINN);
        break;

    case DIFF_NORMAL:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(NumMissionRoutes * 100 / TARGET_FLAGS);
        break;

    case DIFF_HARD:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(Image * 100 / TARGET_IMAGE);
        break;

    case DIFF_FINAL:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(GetAnzBits(RocketFlags) * 10);
        break;

    case DIFF_ADDON02:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(NumFracht * 100 / TARGET_FRACHT);
        break;

    case DIFF_ADDON05:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(NumServicePoints * 100 / TARGET_SERVICE);
        break;

    case DIFF_ADDON08:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(Kurse[0] * 100 / TARGET_SHARES);
        break;

    case DIFF_ADDON09:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(NumOrderFlights * 100 / TARGET_NUM_UHRIG);
        break;

    case DIFF_ADDON10:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(GetAnzBits(RocketFlags) * 10);
        break;

    case DIFF_ATFS01:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(Money * 100 / BTARGET_KONTO);
        break;

    case DIFF_ATFS02:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(GetMissionRating() * 20);
        break;

    case DIFF_ATFS03:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(GetMissionRating() * 100 / BTARGET_PASSAVG);
        break;

    case DIFF_ATFS04:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(DaysWithoutSabotage * 100 / BTARGET_DAYSSABO);
        break;

    case DIFF_ATFS05:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(GetMissionRating() * 100 / 3);
        break;

    case DIFF_ATFS06:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(DaysWithoutSabotage * 100 / BTARGET_DAYSSABO);
        break;

    case DIFF_ATFS08:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(GetMissionRating() * 20);
        break;

    default:
        Statistiken[STAT_MISSIONSZIEL].SetAtPastDay(GetMissionRating());
        break;
    }

    // STAT_WARTUNG: in CPlayer::NewDay ()

    // STAT_NIEDERLASSUNGEN:
    for (c = d = 0; c < RentCities.RentCities.AnzEntries(); c++) {
        if (RentCities.RentCities[c].Rang != 0U) {
            d++;
        }
    }
    Statistiken[STAT_NIEDERLASSUNGEN].SetAtPastDay(d);

    // STAT_ROUTEN:
    for (c = d = 0; c < RentRouten.RentRouten.AnzEntries(); c++) {
        if (RentRouten.RentRouten[c].Rang != 0U) {
            d++;
        }
    }
    Statistiken[STAT_ROUTEN].SetAtPastDay(d);

    // STAT_GEHALT:
    if (Owner == 0) {
        /*
        for (c = d = 0; c < Workers.Workers.AnzEntries(); c++) {
            if (Workers.Workers[c].Employer == PlayerNum) {
                d += Workers.Workers[c].Gehalt;
            }
        }
        Statistiken[STAT_GEHALT].SetAtPastDay(-d);
        */
    } else if (Owner == 1 && RobotUse(ROBOT_USE_FAKE_PERSONAL)) {
        e = 0;
        SLONG NumIgnore = PlayerNum * 2;

        for (c = d = 0; c < Planes.AnzEntries(); c++) {
            if (Planes.IsInAlbum(c) != 0) {
                // d+=PlaneTypes[Planes[c].TypeId].AnzPiloten;
                d += Planes[c].ptAnzPiloten;
            }
        }

        for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
            if (Workers.Workers[c].Typ == WORKER_PILOT) {
                NumIgnore--;
                if (NumIgnore < 0) {
                    d--;
                    if (d < 0) {
                        break;
                    }
                    e += Workers.Workers[c].OriginalGehalt;
                }
            }
        }

        NumIgnore = PlayerNum * 2;
        for (c = d = 0; c < Planes.AnzEntries(); c++) {
            if (Planes.IsInAlbum(c) != 0) {
                // d+=PlaneTypes[Planes[c].TypeId].AnzBegleiter;
                d += Planes[c].ptAnzBegleiter;
            }
        }

        for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
            if (Workers.Workers[c].Typ == WORKER_STEWARDESS) {
                NumIgnore--;
                if (NumIgnore < 0) {
                    d--;
                    if (d < 0) {
                        break;
                    }
                    e += Workers.Workers[c].OriginalGehalt;
                }
            }
        }

        Statistiken[STAT_GEHALT].SetAtPastDay(-(e + e / 4));
    }

    // STAT_BEKANNTHEIT:
    Statistiken[STAT_BEKANNTHEIT].SetAtPastDay(Image / 10);

    // STAT_AKTIEN_ANZAHL bis STAT_AKTIEN_HA
    Statistiken[STAT_AKTIEN_ANZAHL].SetAtPastDay(AnzAktien);
    for (c = 0; c < 4; c++) {
        Statistiken[STAT_AKTIEN_SA + c].SetAtPastDay(OwnsAktien[c]);
    }
}

//--------------------------------------------------------------------------------------------
// bringt die Ticketpreise der Routen auf den neusten Stand:
//--------------------------------------------------------------------------------------------
void PLAYER::UpdateTicketpreise(SLONG RouteId, SLONG Ticketpreis, SLONG TicketpreisFC) {
    for (SLONG c = Planes.AnzEntries() - 1; c >= 0; c--) {
        if (Planes.IsInAlbum(c) != 0) {
            CPlane &qPlane = Planes[c];

            for (SLONG d = qPlane.Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
                CFlugplanEintrag &qFPE = qPlane.Flugplan.Flug[d];

                if (qFPE.ObjectType == 1 && (qFPE.Startdate > Sim.Date || (qFPE.Startdate == Sim.Date && qFPE.Startzeit > Sim.GetHour() + 1))) {
                    if (Routen(qFPE.ObjectId) == Routen(RouteId)) {
                        if (Ticketpreis > qFPE.Ticketpreis || TicketpreisFC > qFPE.TicketpreisFC) {
                            qFPE.Ticketpreis = Ticketpreis;
                            qFPE.TicketpreisFC = TicketpreisFC;
                            qFPE.FlightChanged();
                            // qFPE.CalcPassengers (qPlane.TypeId, PlayerNum, (LPCTSTR)qPlane);
                            qFPE.CalcPassengers(PlayerNum, qPlane);
                        }

                        qFPE.Ticketpreis = Ticketpreis;
                        qFPE.TicketpreisFC = TicketpreisFC;
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Aktienhandel durchführen
//--------------------------------------------------------------------------------------------
SLONG PLAYER::TradeStock(SLONG airlineNum, SLONG amount) {
    if (amount == 0) {
        return 0;
    }

    __int64 aktienWert = 0;
    if (amount > 0) {
        /* kaufen */

        /* Handel durchführen */
        aktienWert = __int64(Sim.Players.Players[airlineNum].Kurse[0] * amount);
        auto gesamtPreis = aktienWert + aktienWert / 10 + 100;
        if (Money - gesamtPreis < DEBT_LIMIT) {
            return 0;
        }
        ChangeMoney(-gesamtPreis, 3150, "");
        OwnsAktien[airlineNum] += amount;

        /* aktualisiere Aktienwert */
        AktienWert[airlineNum] += aktienWert;

        /* aktualisiere Aktienkurs */
        auto anzAktien = static_cast<DOUBLE>(Sim.Players.Players[airlineNum].AnzAktien);
        Sim.Players.Players[airlineNum].Kurse[0] *= anzAktien / (anzAktien - amount / 2);
        if (Sim.Players.Players[airlineNum].Kurse[0] < 0) {
            Sim.Players.Players[airlineNum].Kurse[0] = 0;
        }
    } else {
        /* verkaufen */

        auto _amount = std::abs(amount);
        if (_amount > OwnsAktien[airlineNum]) {
            TeakLibW_Exception(FNL, ExcNever);
            return 0;
        }

        /* aktualisiere Aktienwert */
        {
            auto num = static_cast<DOUBLE>(OwnsAktien[airlineNum]);
            AktienWert[airlineNum] *= (num - _amount) / num;
        }

        /* Handel durchführen */
        aktienWert = __int64(Sim.Players.Players[airlineNum].Kurse[0] * _amount);
        auto gesamtPreis = aktienWert - aktienWert / 10 - 100;
        ChangeMoney(gesamtPreis, 3151, "");
        OwnsAktien[airlineNum] -= _amount;

        /* aktualisiere Aktienkurs */
        {
            auto anzAktien = static_cast<DOUBLE>(Sim.Players.Players[airlineNum].AnzAktien);
            Sim.Players.Players[airlineNum].Kurse[0] *= (anzAktien - _amount / 2) / anzAktien;
            if (Sim.Players.Players[airlineNum].Kurse[0] < 0) {
                Sim.Players.Players[airlineNum].Kurse[0] = 0;
            }
        }
    }

    return aktienWert;
}

//--------------------------------------------------------------------------------------------
// Sendet die aktuelle Position an alle Anderen im Netzwerk:
//--------------------------------------------------------------------------------------------
void PLAYER::BroadcastPosition(bool bForce) {
    if ((Sim.bNetwork != 0) && (Owner == 0 || (Owner == 1 && (Sim.bIsHost != 0))) && Sim.Time > 9 * 60000) {
        if (GetRoom() == ROOM_AIRPORT || bForce) {
            UpdateWaypointWalkingDirection();

            TEAKFILE Message;

            Message.Announce(128);

            Message << ATNET_PLAYERPOS << PlayerNum;

            PLAYER &qPlayer = (*this);
            PERSON &qPerson = Sim.Persons[Sim.Persons.GetPlayerIndex(PlayerNum)];

            // if (qPerson.Position.x==qPerson.Target.x && qPerson.Position.y==qPerson.Target.y+5000) DebugBreak();

            // Read the message data:
            Message << qPlayer.PrimaryTarget.x << qPlayer.PrimaryTarget.y;
            Message << qPlayer.SecondaryTarget.x << qPlayer.SecondaryTarget.y;
            Message << qPlayer.TertiaryTarget.x << qPlayer.TertiaryTarget.y;
            Message << qPlayer.DirectToRoom << qPlayer.iWalkActive;
            Message << qPlayer.TopLocation << qPlayer.ExRoom;
            Message << qPlayer.NewDir << qPlayer.WalkSpeed;
            Message << qPerson.Target.x << qPerson.Target.y;
            Message << qPerson.Position.x << qPerson.Position.y;
            Message << qPerson.ScreenPos.x << qPerson.ScreenPos.y;
            Message << qPerson.StatePar << qPerson.Running;
            Message << qPerson.Dir << qPerson.LookDir;
            Message << qPerson.Phase;

            // Message time is different from local time. Adapt data:
            Message << Sim.TimeSlice;

            SIM::SendMemFile(Message);
        }
    }
}

//============================================================================================
// PLAYERS::
//============================================================================================
// Konstruirt das Spieler-Array
//============================================================================================
PLAYERS::PLAYERS() {
    AnzPlayers = 4;
    Players.ReSize(AnzPlayers);

    for (SLONG c = 0; c < AnzPlayers; c++) {
        Players[c].PlayerNum = c;
    }
}

//--------------------------------------------------------------------------------------------
// Überprüft alle Flugpläne auf tote Einträge: (crasht bei toten Einträgen; nur zum testen)
//--------------------------------------------------------------------------------------------
void PLAYERS::CheckFlighplans() {
    for (SLONG c = 0; c < AnzPlayers; c++) {
        if (Players[c].IsOut == 0) {
            for (SLONG d = 0; d < Players[c].Planes.AnzEntries(); d++) {
                if (Players[c].Planes.IsInAlbum(d) != 0) {
                    CPlane &qPlane = Players[c].Planes[d];

                    for (SLONG e = qPlane.Flugplan.Flug.AnzEntries() - 1; e >= 0; e--) {
                        CFlugplan &qPlan = qPlane.Flugplan;
                        CFlugplanEintrag &qFPE = qPlan.Flug[e];

                        switch (qFPE.ObjectType) {
                        case 0:
                            // nix
                            break;
                        case 1:
                            Routen[qFPE.ObjectId];
                            break;

                        case 2:
                            Players[c].Auftraege[qFPE.ObjectId];
                            break;

                        case 3:
                            // Automatik
                            break;

                        case 4:
                            Players[c].Frachten[qFPE.ObjectId];
                            break;

                        default:
                            AT_Log("Player.cpp: Default case should not be reached.");
                            DebugBreak();
                        }
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Bringt die Statistiken auf den neusten Stand:
//--------------------------------------------------------------------------------------------
void PLAYERS::UpdateStatistics() {
    for (SLONG c = 0; c < AnzPlayers; c++) {
        if (Players[c].IsOut == 0) {
            Players[c].UpdateStatistics();
        }
    }
}

//--------------------------------------------------------------------------------------------
// Gibt die Zahl der menschlichen (Nicht-Computer) Spieler:
//--------------------------------------------------------------------------------------------
SLONG PLAYERS::GetAnzHumanPlayers() const {
    SLONG c = 0;
    SLONG Anz = 0;

    for (c = Anz = 0; c < AnzPlayers; c++) {
        if ((Players[c].IsOut == 0) && Players[c].Owner != 1) {
            Anz++;
        }
    }

    return (Anz);
}

//--------------------------------------------------------------------------------------------
// Gibt die Zahl der nicht-menschlichen (Computer) Spieler:
//--------------------------------------------------------------------------------------------
SLONG PLAYERS::GetAnzRobotPlayers() const {
    SLONG c = 0;
    SLONG Anz = 0;

    for (c = Anz = 0; c < AnzPlayers; c++) {
        if ((Players[c].IsOut == 0) && Players[c].Owner == 1) {
            Anz++;
        }
    }

    return (Anz);
}

//--------------------------------------------------------------------------------------------
// Gibt den Index des x. menschlichen (Nicht-Computer) Spielers zurück:
//--------------------------------------------------------------------------------------------
SLONG PLAYERS::GetIndexOfHumanPlayerNumberX(SLONG x) {
    SLONG c = 0;
    SLONG Anz = 0;

    for (c = Anz = 0; c < AnzPlayers; c++) {
        if (Players[c].Owner != 1) {
            Anz++;
            if (Anz == x) {
                return (c);
            }
        }
    }

    TeakLibW_Exception(FNL, ExcNever);
    return (0);
}

//--------------------------------------------------------------------------------------------
// Schaut nach, ob das Logo schon von einem anderen Spieler verwendet wird:
//--------------------------------------------------------------------------------------------
BOOL PLAYERS::IsLogoInUse(SLONG Player, UBYTE Logo) {
    SLONG c = 0;

    // Default-Logos zuweisen:
    for (c = 0; c < 4; c++) {
        if (Sim.Players.Players[c].Logo == Logo && Player != c) {
            return (TRUE);
        }
    }

    return (FALSE);
}

//--------------------------------------------------------------------------------------------
// Schaut nach, ob der Flugzeugname schon verwendet wird:
//--------------------------------------------------------------------------------------------
BOOL PLAYERS::IsPlaneNameInUse(const CString &PlaneName) {
    SLONG c = 0;

    for (c = 0; c < AnzPlayers; c++) {
        if (Players[c].Planes.IsPlaneNameInUse(PlaneName) != 0) {
            return (TRUE);
        }
    }

    return (FALSE);
}

//--------------------------------------------------------------------------------------------
// Läßt alle Spieler zum Raum der Flughafenaufsicht laufen:
//--------------------------------------------------------------------------------------------
void PLAYERS::WalkToStartingPoint() {
    SLONG c = 0;

    for (c = 0; c < AnzPlayers; c++) {
        if (Sim.Players.Players[c].IsOut == 0) {
            Players[c].WalkToRoom(ROOM_AUFSICHT);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erledigt Dinge für den Roboter:
//--------------------------------------------------------------------------------------------
void PLAYERS::RobotPump() {
    SLONG c = 0;
    BOOL DoIt = TRUE;
    static SLONG LastTenMinutes = 0;

    for (c = 0; c < AnzPlayers; c++) {
        if ((Sim.Players.Players[c].IsOut == 0) && (DoIt != 0)) {
            if ((Sim.Players.Players[c].Locations[1] != 0U) || (Sim.Players.Players[c].DialogWin != nullptr) ||
                (Sim.CallItADay == 0 && (Sim.Persons[Sim.Persons.GetPlayerIndex(c)].StatePar != 0))) {
                DoIt = FALSE;
            }
        }

        if ((Sim.Players.Players[c].IsOut == 0) && (Sim.Players.Players[c].IsTalking == 0)) {
            Players[c].RobotPump();
        }
    }

    // Hack: Ab & Zu, wenn alle Player im Flughafen sind und keiner telefoniert, werden alle Räume freigegeben...
    if (Sim.CallItADay == 0) {
        if ((DoIt != 0) && LastTenMinutes != Sim.GetMinute() / 10) {
            for (c = 0; c < 256; c++) {
                if (c < ROOM_REISE_X1 || c > ROOM_MONITOR6) {
                    Sim.RoomBusy[c] = 0;
                }
            }

            for (c = 0; c < Talkers.Talkers.AnzEntries(); c++) {
                Talkers.Talkers[c].Locking = 0;
                Talkers.Talkers[c].NumRef = 0;
            }
        }

        LastTenMinutes = Sim.GetMinute() / 10;
    }
}

//--------------------------------------------------------------------------------------------
// Aktualisiert die Sprechblase der Berater:
//--------------------------------------------------------------------------------------------
void PLAYERS::MessagePump() { Players[Sim.localPlayer].Messages.Pump(); }

//--------------------------------------------------------------------------------------------
// Initialisiert den Computerspieler morgens:
//--------------------------------------------------------------------------------------------
void PLAYERS::RobotInit() {
    SLONG c = 0;

    for (c = 0; c < AnzPlayers; c++) {
        Players[c].RobotInit();
    }
}

void PLAYERS::RandomBeraterMessage() {
    SLONG c = 0;

    for (c = 0; c < AnzPlayers; c++) {
        Players[c].RandomBeraterMessage();
    }
}

//============================================================================================
// HISTORY::
//============================================================================================
//--------------------------------------------------------------------------------------------
// Alles löschen:
//--------------------------------------------------------------------------------------------
void HISTORY::ReInit() {
    SLONG c = 0;

    for (c = 0; c < 100; c++) {
        HistoryLine[c].Description = "*";
        HistoryLine[c].Money = 0;
        HistoryLine[c].ByPhone = FALSE;
    }
}

//--------------------------------------------------------------------------------------------
// Einen Eintrag loggen:
//--------------------------------------------------------------------------------------------
void HISTORY::AddEntry(__int64 Money, const CString &Description) {
    SLONG c = 0;

    HistoricMoney += HistoryLine[0].Money;

    for (c = 0; c < 99; c++) {
        HistoryLine[c] = HistoryLine[c + 1];
    }

    HistoryLine[99].ByPhone = 0;
    HistoryLine[99].Money = Money;
    HistoryLine[99].Description = Description;
}

//--------------------------------------------------------------------------------------------
// Bit 0: Handy, Bit 1: Ferngespräch
//--------------------------------------------------------------------------------------------
void HISTORY::AddNewCall(SLONG Type) {
    SLONG c = 0;

    HistoricMoney += HistoryLine[0].Money;

    for (c = 0; c < 99; c++) {
        HistoryLine[c] = HistoryLine[c + 1];
    }

    HistoryLine[99].ByPhone = 1;
    HistoryLine[99].Money = -1;
    HistoryLine[99].Description = StandardTexte.GetS(TOKEN_MONEY, 3200 + Type);
}

//--------------------------------------------------------------------------------------------
// Aktualisiert die Kosten für Gespräche
//--------------------------------------------------------------------------------------------
void HISTORY::AddCallCost(SLONG Money) {
    for (SLONG c = 99; c >= 0; c--) {
        if (HistoryLine[c].ByPhone != 0) {
            HistoryLine[c].Money -= Money;
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Speichert ein HISTORYLINE-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const HISTORYLINE &h) {
    File << h.Money << h.Description << h.ByPhone;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein HISTORYLINE-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, HISTORYLINE &h) {
    File >> h.Money >> h.Description >> h.ByPhone;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Speichert ein HISTORY-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const HISTORY &h) {
    SLONG c = 0;

    if (SaveVersion == 1 && SaveVersionSub >= 100) {
        for (c = 0; c < 100; c++) {
            File << h.HistoryLine[c];
        }
    } else {
        for (c = 90; c < 100; c++) {
            File << h.HistoryLine[c];
        }
    }

    File << h.HistoricMoney;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein HISTORY-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, HISTORY &h) {
    SLONG c = 0;

    if (SaveVersion == 1 && SaveVersionSub >= 100) {
        for (c = 0; c < 100; c++) {
            File >> h.HistoryLine[c];
        }
    } else {
        h.ReInit();
        for (c = 90; c < 100; c++) {
            File >> h.HistoryLine[c];
        }
    }

    File >> h.HistoricMoney;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Speichert ein PLAYER-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const PLAYER &Player) {
    SLONG c = 0;

    // Generelles:
    File << Player.PlayerNum << Player.Name << Player.NameX;
    File << Player.IsOut;
    File << Player.Airline << Player.AirlineX << Player.Abk;
    File << Player.Owner << Player.Logo << Player.Money;
    File << Player.MoneyPast << Player.Credit;
    File << Player.Image << Player.KerosinQuali << Player.KerosinKind;
    File << Player.Tank << Player.TankOpen << Player.TankInhalt;
    File << Player.TankPreis << Player.GameSpeed << Player.ArabTrust;
    File << Player.ArabMode << Player.ArabOpfer << Player.ArabHints;
    File << Player.ArabPlane << Player.ArabActive << Player.ArabMode2;
    File << Player.MechMode << Player.MechTrust << Player.EarthAlpha;
    File << Player.MechAngry << Player.GlobeOiled << Player.ReferencePlane;
    File << Player.ArabOpfer2 << Player.SickTokay << Player.RunningToToilet;
    File << Player.Stunned << Player.OfficeState << Player.PlayerSmoking << Player.PlayerStinking;
    File << Player.LaptopVirus;
    File << Player.ArabMode3 << Player.ArabOpfer3;
    File << Player.WerbeBroschuere << Player.TelephoneDown << Player.Presseerklaerung;

    if (SaveVersion == 1 && SaveVersionSub >= 5) {
        File << Player.StrikeHours << Player.StrikeNotified;
        File << Player.StrikeEndCountdown << Player.StrikeEndType;

        if (SaveVersion == 1 && SaveVersionSub >= 6) {
            File << Player.DaysWithoutStrike << Player.StrikePlanned;
            File << Player.DaysWithoutSabotage;

            if (SaveVersion == 1 && SaveVersionSub >= 10) {
                File << Player.Bonus;

                if (SaveVersion == 1 && SaveVersionSub >= 100) {
                    File << Player.HasAlkohol << Player.Koffein;
                    File << Player.Frachten << Player.IsDrunk;
                    File << Player.KioskTrust << Player.SecurityFlags;

                    File << Player.NumFracht << Player.NumFrachtFree;
                    File << Player.NumMiles << Player.NumServicePoints;
                    File << Player.bWasInMuseumToday << Player.bHasPlanesUpgradedToday;
                    File << Player.NumOrderFlights << Player.NumOrderFlightsToday;
                    File << Player.NumOrderFlightsToday2 << Player.IsStuck;

                    File << Player.WaitWorkTill << Player.WaitWorkTill2;

                    File << Player.PlayerWalkRandom << Player.PlayerExtraRandom;

                    if (SaveVersion == 1 && SaveVersionSub >= 102) {
                        File << Player.SecurityNeeded;
                    }
                }
            }
        }

        File << Player.TrinkerTrust << Player.SeligTrust;
        File << Player.SpiderTrust << Player.WerbungTrust;
        File << Player.DutyTrust;
    }

    File << Player.DisplayRoutes;
    File << Player.DisplayPlanes;

    // Größere Daten:
    File << Player.Planes << Player.Auftraege << Player.Gates;
    File << Player.Items << Player.RentCities << Player.RentRouten;
    File << Player.CursorPos << Player.History << Player.Blocks;
    File << Player.Messages << Player.Letters;
    File << Player.LaptopBattery << Player.LaptopQuality;

    if (SaveVersion == 1 && SaveVersionSub >= 9) {
        File << Player.CalledCities;
    }

    // Aktien & Geld:
    File << Player.Bilanz << Player.BilanzGestern << Player.BilanzGesamt;
    File << Player.BilanzWoche;
    File << Player.SollZins << Player.HabenZins;
    File << Player.MaxAktien;
    File << Player.AnzAktien << Player.Dividende << Player.TrustedDividende;
    for (c = 0; c < 4; c++) {
        File << Player.OwnsAktien[c];
        File << Player.AktienWert[c];
    }
    for (c = 0; c < 10; c++) {
        File << Player.Kurse[c];
    }

    File << Player.HasFlownRoutes << Player.NumPassengers << Player.NumAuftraege;
    File << Player.Gewinn << Player.NumMissionRoutes;
    File << Player.RocketFlags << Player.LastRocketFlags;
    File << Player.Statistiken;

    // Interne Verwaltung:
    File << Player.ViewPos << Player.IslandViewPos << Player.CameraSpeed;
    File << Player.NewDir;
    File << Player.WinP1 << Player.WinP2;
    for (c = 0; c < 10; c++) {
        File << Player.Locations[c];
    }
    File << Player.LocationTime << Player.LocationForbidden << Player.LocationForbiddenTime;

    File << Player.TopLocation << Player.Windows << Player.Position;
    File << Player.Buttons << Player.xPiloten << Player.xBegleiter;
    File << Player.CallItADay << Player.NumFlights << Player.RoutePage;

    if (SaveVersion == 1 && SaveVersionSub >= 7) {
        File << Player.StandCount;
    }
    if (SaveVersion == 1 && SaveVersionSub >= 100) {
        File << Player.TalkedToNasa << Player.ExRoom;
    }

    // Laufen:
    File << Player.iWalkActive << Player.PrimaryTarget << Player.SecondaryTarget;
    File << Player.TertiaryTarget << Player.WasInRoom;
    File << Player.WalkSpeed << Player.WaitForRoom << Player.ThrownOutOfRoom;
    File << Player.DirectToRoom;
    if (SaveVersion == 1 && SaveVersionSub >= 8) {
        File << Player.WalkToGlobe << Player.GlobeFileOpen;
    }

    // Computerspieler:
    File << Player.ImageGotWorse << Player.StandStillSince;
    File << Player.RobotActions << Player.LastActionId << Player.WorkCountdown;
    File << Player.TimeBuro << Player.TimePersonal << Player.TimeAufsicht;
    File << Player.TimeReiseburo << Player.OutOfGates << Player.Kooperation;
    File << Player.Sympathie << Player.DoRoutes << Player.SavesForPlane;
    File << Player.WantToDoRoutes;
    File << Player.BuyBigPlane << Player.SavesForRocket;
    File << Player.TargetedPlayer;
    File << Player.PlayerDialog << Player.PlayerDialogState;
    File << Player.CalledPlayer << Player.BoredOfPlayer;
    File << Player.IsTalking << Player.IsWalking2Player;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein PLAYER-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, PLAYER &Player) {
    SLONG c = 0;

    // Generelles:
    File >> Player.PlayerNum >> Player.Name >> Player.NameX;
    File >> Player.IsOut;
    File >> Player.Airline >> Player.AirlineX >> Player.Abk;
    File >> Player.Owner >> Player.Logo >> Player.Money;
    File >> Player.MoneyPast >> Player.Credit;

    File >> Player.Image;
    if (SaveVersionSub >= 200) {
        File >> Player.KerosinQuali;
    } else {
        File.Skip(sizeof(SLONG)); // old SLONG BadKerosin
        Player.KerosinQuali = 0;
    }
    File >> Player.KerosinKind;

    File >> Player.Tank >> Player.TankOpen >> Player.TankInhalt;
    File >> Player.TankPreis >> Player.GameSpeed >> Player.ArabTrust;
    File >> Player.ArabMode >> Player.ArabOpfer >> Player.ArabHints;
    File >> Player.ArabPlane >> Player.ArabActive >> Player.ArabMode2;
    File >> Player.MechMode >> Player.MechTrust >> Player.EarthAlpha;
    File >> Player.MechAngry >> Player.GlobeOiled >> Player.ReferencePlane;
    File >> Player.ArabOpfer2 >> Player.SickTokay >> Player.RunningToToilet;
    File >> Player.Stunned >> Player.OfficeState >> Player.PlayerSmoking >> Player.PlayerStinking;
    File >> Player.LaptopVirus;
    File >> Player.ArabMode3 >> Player.ArabOpfer3;
    File >> Player.WerbeBroschuere >> Player.TelephoneDown >> Player.Presseerklaerung;

    if (SaveVersion == 1 && SaveVersionSub >= 5) {
        File >> Player.StrikeHours >> Player.StrikeNotified;
        File >> Player.StrikeEndCountdown >> Player.StrikeEndType;

        if (SaveVersion == 1 && SaveVersionSub >= 6) {
            File >> Player.DaysWithoutStrike >> Player.StrikePlanned;
            File >> Player.DaysWithoutSabotage;

            if (SaveVersion == 1 && SaveVersionSub >= 10) {
                File >> Player.Bonus;

                if (SaveVersion == 1 && SaveVersionSub >= 100) {
                    File >> Player.HasAlkohol >> Player.Koffein;
                    File >> Player.Frachten >> Player.IsDrunk;
                    File >> Player.KioskTrust >> Player.SecurityFlags;

                    File >> Player.NumFracht >> Player.NumFrachtFree;
                    File >> Player.NumMiles >> Player.NumServicePoints;
                    File >> Player.bWasInMuseumToday;
                    if (SaveVersionSub >= 200) {
                        File >> Player.bHasPlanesUpgradedToday;
                    } else {
                        Player.bHasPlanesUpgradedToday = FALSE;
                    }
                    File >> Player.NumOrderFlights >> Player.NumOrderFlightsToday;
                    File >> Player.NumOrderFlightsToday2 >> Player.IsStuck;

                    File >> Player.WaitWorkTill >> Player.WaitWorkTill2;

                    File >> Player.PlayerWalkRandom >> Player.PlayerExtraRandom;

                    if (SaveVersion == 1 && SaveVersionSub >= 102) {
                        File >> Player.SecurityNeeded;
                    } else {
                        Player.SecurityNeeded = 0;
                    }
                } else {
                    Player.HasAlkohol = 1;
                    Player.Koffein = 0;
                    Player.IsStuck = 0;
                    Player.Frachten.ReSize(0);
                    Player.Frachten.IsInAlbum(static_cast<SLONG>(0));
                    Player.IsDrunk = 0;
                    Player.KioskTrust = 0;
                    Player.NumFracht = 0;
                    Player.NumFrachtFree = 0;
                    Player.NumMiles = 0;
                    Player.NumServicePoints = 0;
                    Player.bWasInMuseumToday = FALSE;
                    Player.NumOrderFlights = 0;
                    Player.NumOrderFlightsToday = 0;
                    Player.NumOrderFlightsToday2 = 0;
                    Player.WaitWorkTill = -1;
                    Player.WaitWorkTill2 = Sim.Time;
                    Player.PlayerWalkRandom.SRand(0);
                    Player.PlayerExtraRandom.SRand(0);
                }
            } else {
                Player.Bonus = 0;
            }
        }

        File >> Player.TrinkerTrust >> Player.SeligTrust;
        File >> Player.SpiderTrust >> Player.WerbungTrust;
        File >> Player.DutyTrust;
    }

    File >> Player.DisplayRoutes;
    File >> Player.DisplayPlanes;

    // Größere Daten:
    File >> Player.Planes >> Player.Auftraege;
    if (SaveVersionSub >= 200) {
        File >> Player.Gates;
    } else { // Shift to a buffer makes this neccessary
        File >> Player.Gates.Gates;
        UBYTE gates[24 * 7]{};

        File.Read(gates, 24 * 7);
        for (int i = 0; i < 24 * 7; i++) {
            Player.Gates.Auslastung[i] = gates[i];
        }

        File >> Player.Gates.NumRented;
    }
    File >> Player.Items >> Player.RentCities >> Player.RentRouten;
    File >> Player.CursorPos >> Player.History >> Player.Blocks;
    File >> Player.Messages >> Player.Letters;
    File >> Player.LaptopBattery >> Player.LaptopQuality;

    // Mehr Strecken im AddOn ==> RentCities ggf. ergänzen
    if (Player.RentCities.RentCities.AnzEntries() != Cities.AnzEntries()) {
        Player.RentCities.RentCities.ReSize(Cities.AnzEntries());
    }

    if (SaveVersion == 1 && SaveVersionSub >= 9) {
        File >> Player.CalledCities;
    } else {
        Player.CalledCities.FillWith(0);
    }

    // Aktien & Geld:
    File >> Player.Bilanz >> Player.BilanzGestern;
    if (SaveVersionSub >= 200) {
        File >> Player.BilanzGesamt >> Player.BilanzWoche;
    }
    File >> Player.SollZins >> Player.HabenZins;
    File >> Player.MaxAktien;
    File >> Player.AnzAktien >> Player.Dividende >> Player.TrustedDividende;
    for (c = 0; c < 4; c++) {
        File >> Player.OwnsAktien[c];

        if (SaveVersion == 1 && SaveVersionSub >= 103) {
            File >> Player.AktienWert[c];
        } else {
            ULONG tmp = 0;
            File >> tmp;
            Player.AktienWert[c] = tmp;
        }
    }
    for (c = 0; c < 10; c++) {
        File >> Player.Kurse[c];
    }

    File >> Player.HasFlownRoutes >> Player.NumPassengers >> Player.NumAuftraege;
    File >> Player.Gewinn >> Player.NumMissionRoutes;
    File >> Player.RocketFlags >> Player.LastRocketFlags;
    File >> Player.Statistiken;

    Player.Statistiken.ReSize(STAT_ANZ);

    // Interne Verwaltung:
    File >> Player.ViewPos >> Player.IslandViewPos >> Player.CameraSpeed;
    File >> Player.NewDir;
    File >> Player.WinP1 >> Player.WinP2;
    for (c = 0; c < 10; c++) {
        File >> Player.Locations[c];
    }
    File >> Player.LocationTime >> Player.LocationForbidden >> Player.LocationForbiddenTime;

    File >> Player.TopLocation >> Player.Windows >> Player.Position;
    File >> Player.Buttons >> Player.xPiloten >> Player.xBegleiter;
    File >> Player.CallItADay >> Player.NumFlights >> Player.RoutePage;

    if (SaveVersion == 1 && SaveVersionSub >= 7) {
        File >> Player.StandCount;
    }
    if (SaveVersion == 1 && SaveVersionSub >= 100) {
        File >> Player.TalkedToNasa >> Player.ExRoom;
    } else {
        Player.TalkedToNasa = 0;
        Player.ExRoom = 0;
    }

    // Laufen:
    File >> Player.iWalkActive >> Player.PrimaryTarget >> Player.SecondaryTarget;
    File >> Player.TertiaryTarget >> Player.WasInRoom;
    File >> Player.WalkSpeed >> Player.WaitForRoom >> Player.ThrownOutOfRoom;
    File >> Player.DirectToRoom;
    if (SaveVersion == 1 && SaveVersionSub >= 8) {
        File >> Player.WalkToGlobe >> Player.GlobeFileOpen;
    }

    // Computerspieler:
    File >> Player.ImageGotWorse >> Player.StandStillSince;
    File >> Player.RobotActions >> Player.LastActionId >> Player.WorkCountdown;
    File >> Player.TimeBuro >> Player.TimePersonal >> Player.TimeAufsicht;
    File >> Player.TimeReiseburo >> Player.OutOfGates >> Player.Kooperation;
    File >> Player.Sympathie >> Player.DoRoutes >> Player.SavesForPlane;
    File >> Player.WantToDoRoutes;
    File >> Player.BuyBigPlane >> Player.SavesForRocket;
    if (SaveVersionSub >= 200) {
        File >> Player.TargetedPlayer;
    }
    File >> Player.PlayerDialog >> Player.PlayerDialogState;
    File >> Player.CalledPlayer >> Player.BoredOfPlayer;
    File >> Player.IsTalking >> Player.IsWalking2Player;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Speichert ein PLAYERS-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const PLAYERS &Players) {
    File << Players.AnzPlayers << Players.Players;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein PLAYERS-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, PLAYERS &Players) {
    File >> Players.AnzPlayers >> Players.Players;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Unterstützen die Roboter im aktuellen die Level ein bestimmtes Feature?:
//--------------------------------------------------------------------------------------------
bool GlobalUse(SLONG FeatureId) {
    switch (FeatureId) {
    case USE_TRAVELHOLDING:
        return (Sim.Difficulty != DIFF_ADDON06);
    default:
        TeakLibW_Exception(FNL, ExcNever);
    }
    return false;
}

bool PLAYER::RobotUse(SLONG FeatureId) const {
    SLONG Level = 0;

    // Die verschiedenen Levelstrukturen in eine Reihe bringen:
    if (Sim.Difficulty >= DIFF_TUTORIAL && Sim.Difficulty <= DIFF_FINAL) {
        Level = Sim.Difficulty;
    } else if (Sim.Difficulty == DIFF_FREEGAME || Sim.Difficulty == DIFF_FREEGAMEMAP) {
        Level = 6;
    } else if (Sim.Difficulty >= DIFF_ADDON01 && Sim.Difficulty <= DIFF_ADDON10) {
        Level = Sim.Difficulty - DIFF_ADDON01 + 7;
    } else {
        Level = Sim.Difficulty - DIFF_ATFS01 + 7 + 10;
    }

    // Tabelle ermitteln für welche Level ein Feature ermittelt wird:
    const char *pFeatureDesc = nullptr;
    switch (FeatureId) {
    // 0-5 : Level im Basisspiel
    // F  : Freies Spiel
    // 0-9 : Level in Addon / FlighSecurity
    //
    //"-." : Disabled
    //"xX" : Enabled
    //"!"  : Enabled, but wasn't enabled in AT 1.0
    //"?"  : Enabled, but isn't enabled in AT First Class
    //                                            Basisspiel       Addon        FlightSecu
    //                                                012345   F   0123456789   0123456789
    case ROBOT_USE_SABOTAGE:
        pFeatureDesc = "-XXXXX"
                       "X"
                       "XXXXXXXXXX"
                       "-XXXXXXXXX";
        break;
    case ROBOT_USE_FRACHT:
        pFeatureDesc = "------"
                       "X"
                       "-XXXXXXXXX"
                       "XXXXXXXXXX";
        break;
    case ROBOT_USE_WERBUNG:
        pFeatureDesc = "---XXX"
                       "!"
                       "XXXXXXXXXX"
                       "-XX-X-XXXX";
        break;
    case ROBOT_USE_AUFSICHT:
        pFeatureDesc = "----XX"
                       "!"
                       "----------"
                       "----------";
        break;
    case ROBOT_USE_NASA:
        pFeatureDesc = "-----X"
                       "."
                       "---------X"
                       "----------";
        break;
    case ROBOT_USE_NEVERGIVEROUTEUP:
        pFeatureDesc = "----X-"
                       "."
                       "----------"
                       "----------";
        break;
    case ROBOT_USE_ROUTES:
        pFeatureDesc = "---X-X"
                       "X"
                       "----XX-X-X"
                       "XXXXXXXXXX";
        break;
    case ROBOT_USE_SUGGESTROUTES:
        pFeatureDesc = "---X--"
                       "."
                       "----------"
                       "----------";
        break;
    case ROBOT_USE_FORCEROUTES:
        pFeatureDesc = "------"
                       "."
                       "-----X----"
                       "----------";
        break;
    case ROBOT_USE_HARDWERBUNG:
        pFeatureDesc = "----XX"
                       "."
                       "------XXXX"
                       "----------";
        break;
    case ROBOT_USE_SELLSHARES:
        if (PlayerNum != 2) {
            return false;
        }
        pFeatureDesc = "XXXX--"
                       "."
                       "XXX-------"
                       "X---------";
        break;
    case ROBOT_USE_BUYOWNSHARES:
        pFeatureDesc = "----XX"
                       "."
                       "-------XXX"
                       "----------";
        break;
    case ROBOT_USE_REBUYSHARES:
        if (PlayerNum != 3) {
            return true;
        }
        pFeatureDesc = "-----X"
                       "."
                       "--------XX"
                       "----------";
        break;
    case ROBOT_USE_TUTORIALLIMIT:
        pFeatureDesc = "X-----"
                       "."
                       "----------"
                       "----------";
        break;
    case ROBOT_USE_ROUTEMISSION:
        pFeatureDesc = "---X--"
                       "."
                       "----------"
                       "----------";
        break;
    case ROBOT_USE_MUCHWERBUNG:
        pFeatureDesc = "----X-"
                       "."
                       "----------"
                       "----------";
        break;
    case ROBOT_USE_BONUS_X2:
        pFeatureDesc = "--XXX-"
                       "."
                       "XXX-------"
                       "----------";
        break;
    case ROBOT_USE_BONUS_X4:
        pFeatureDesc = "-----X"
                       "."
                       "---X--X---"
                       "----------";
        break;
    case ROBOT_USE_BONUS_X8:
        pFeatureDesc = "------"
                       "X"
                       "----XX-XXX"
                       "----------";
        break;
    case ROBOT_USE_ROUTEBOX:
        pFeatureDesc = "-XXXXX"
                       "X"
                       "XXXXXXXXXX"
                       "XXX-X-XXXX";
        break;
    case ROBOT_USE_ABROAD:
        pFeatureDesc = "--XXXX"
                       "X"
                       "XXXXXXXXXX"
                       "XXXXXXXXXX";
        break;
    case ROBOT_USE_MUCH_SABOTAGE:
        pFeatureDesc = "---XXX"
                       "!"
                       "XXXXXXXXXX"
                       "XXXXXXXXXX";
        break;
    case ROBOT_USE_PAYBACK_CREDIT:
        pFeatureDesc = "------"
                       "."
                       "X---------"
                       "----------";
        break;
    case ROBOT_USE_MUCH_FRACHT:
        pFeatureDesc = "------"
                       "."
                       "-X--------"
                       "----------";
        break;
    case ROBOT_USE_FREE_FRACHT:
        pFeatureDesc = "------"
                       "."
                       "--X-------"
                       "----------";
        break;
    case ROBOT_USE_RUN_FRACHT:
        pFeatureDesc = "------"
                       "."
                       "--X-------"
                       "----------";
        break;
    case ROBOT_USE_LUXERY:
        pFeatureDesc = "------"
                       "."
                       "----X-----"
                       "-X--------";
        break;
    case ROBOT_USE_IMAGEBONUS:
        /* We enable it for normal bots whenever they use routes */
        return (DoRoutes > 0);
        /*pFeatureDesc = "------"
                       "."
                       "-----X----"
                       "----------";*/
        break;
    case ROBOT_USE_GOODPLANES:
        pFeatureDesc = "------"
                       "."
                       "-----X----"
                       "-X--------";
        break;
    case ROBOT_USE_REPAIRPLANES:
        pFeatureDesc = "------"
                       "."
                       "------X---"
                       "----------";
        break;
    case ROBOT_USE_HIGHSHAREPRICE:
        if (PlayerNum == 3) {
            return true;
        }
        pFeatureDesc = "------"
                       "."
                       "-------X--"
                       "XXXXXXXXXX";
        break;
    case ROBOT_USE_WORKQUICK:
        pFeatureDesc = "------"
                       "."
                       "---XXXXXXX"
                       "XXXXXXXXXX";
        break;
    case ROBOT_USE_GROSSESKONTO:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "X---------";
        break;
    case ROBOT_USE_EMITMUCHSHARES:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "X---X--X--";
        break;
    case ROBOT_USE_WORKVERYQUICK:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "-----XXXXX";
        break;
    case ROBOT_USE_ALLRUN:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "---------X";
        break;
    case ROBOT_USE_DONTBUYANYSHARES:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "X---------";
        break;
    case ROBOT_USE_MAXKREDIT:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "XXX-----XX";
        break;
    case ROBOT_USE_MAX4PLANES:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "--X-X--X--";
        break;
    case ROBOT_USE_MAX5PLANES:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "---X-X----";
        break;
    case ROBOT_USE_MAX10PLANES:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "-X--------";
        break;
    case ROBOT_USE_NOCHITCHAT:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "--X----XXX";
        break;
    case ROBOT_USE_OFTENMECH:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "-X--X--XXX";
        break;
    case ROBOT_USE_SHORTFLIGHTS:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "--X-------";
        break;
    case ROBOT_USE_EXTREME_SABOTAGE:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "---X-X----";
        break;
    case ROBOT_USE_SECURTY_OFFICE:
        pFeatureDesc = "------"
                       "?"
                       "----------"
                       "---X-X-X-X";
        break;
    case ROBOT_USE_MAKLER:
        pFeatureDesc = "XXXXXX"
                       "!"
                       "XXXXXXXXXX"
                       "XXX-X-XXXX";
        break;
    case ROBOT_USE_PETROLAIR:
        pFeatureDesc = "XXXXXX"
                       "!"
                       "XXXXXXXXXX"
                       "XXXXX-XX--";
        break;
    case ROBOT_USE_MAX20PERCENT:
        pFeatureDesc = "XXXXXX"
                       "!"
                       "XXXXXXXXXX"
                       "XXXXX-XXXX";
        break;
    case ROBOT_USE_TANKS:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "--------XX";
        break;
    case ROBOT_USE_DESIGNER:
        pFeatureDesc = "------"
                       "?"
                       "----------"
                       "---XX-XX--";
        break;
    case ROBOT_USE_DESIGNER_BUY:
        pFeatureDesc = "------"
                       "."
                       "----------"
                       "----X--X--";
        break;
    case ROBOT_USE_BONUS:
        return true;
    case ROBOT_USE_ROUTE_BONUS:
        return true;
    case ROBOT_USE_FAKE_PERSONAL:
        return true;
    case ROBOT_USE_NO_FINE:
        return true;
    case ROBOT_USE_NO_CHECK_FLIGHT:
        return true;
    case ROBOT_USE_NO_CHECK_FFLIGHT:
        return true;
    case ROBOT_USE_MISC_CHEATS:
        return true;
    case ROBOT_UHRIG_FLIGHTS_AUTO:
        pFeatureDesc = "------"
                       "."
                       "--------X-"
                       "----------";
        break;

    /* specialization of computer players */
    case ROBOT_USE_WORKQUICK_2:
        return (PlayerNum == 2);
    case ROBOT_USE_EXTRA_SABOTAGE:
        return (PlayerNum == 0);
    case ROBOT_USE_SABO_AFFORD_FINE:
        return (PlayerNum == 1);
    case ROBOT_USE_MILD_SABOTAGE:
        return (PlayerNum == 2);
    case ROBOT_USE_MILDER_SABOTAGE:
        return (PlayerNum == 0);
    case ROBOT_USE_EXTRA_BANK:
        return (PlayerNum == 1);
    case ROBOT_USE_MORE_PLANES:
        return (PlayerNum == 2);
    case ROBOT_USE_MORE_PLANES_2:
        return (PlayerNum == 0);
    case ROBOT_USE_MUCH_FRACHT_BONUS:
        return (PlayerNum == 2);
    case ROBOT_USE_ALT_KREDIT:
        return (PlayerNum == 1);
    case ROBOT_USE_EAGER_SELLSHARES:
        return (PlayerNum == 1);
    case ROBOT_USE_BARELY_REPAIR:
        return (PlayerNum == 3);
    case ROBOT_USE_UPGRADE_KOMFORT:
        return (PlayerNum == 0);
    case ROBOT_USE_UPGRADE_TECH:
        return (PlayerNum == 1);
    case ROBOT_USE_BUY_MORE_ABROAD:
        return (PlayerNum == 0);
    case ROBOT_ALWAYS_BUY_GATES:
        return (PlayerNum == 0);

    default:
        TeakLibW_Exception(FNL, ExcNever);
    }

    return (pFeatureDesc[Level] != '-' && pFeatureDesc[Level] != '.') && ((bFirstClass == 0) || pFeatureDesc[Level] != '?');
}
