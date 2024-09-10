//============================================================================================//============================================================================================
// Auftrag.cpp : Routinen für die Aufträge die die Spieler haben
//============================================================================================
#include "StdAfx.h"
#include "AtNet.h"

// Daten des aktuellen Savegames beim laden:
extern SLONG SaveVersion;
extern SLONG SaveVersionSub;

SLONG PlayerMaxPassagiere = 90;
SLONG PlayerMaxLength = 2000000;
SLONG PlayerMinPassagiere = 90;
SLONG PlayerMinLength = 2000000;

void CalcPlayerMaximums(bool bForce = false);

//--------------------------------------------------------------------------------------------
// Berechnet, was für Aufträge der Spieler maximal annehmen kann:
//--------------------------------------------------------------------------------------------
void CalcPlayerMaximums(bool bForce) {
    static SLONG LastHour = -1;

    if (LastHour == Sim.GetHour() && !bForce) {
        return;
    }

    LastHour = Sim.GetHour();

    PlayerMaxPassagiere = 90;
    PlayerMaxLength = 2000;
    PlayerMinPassagiere = 2147483647;
    PlayerMinLength = 2147483647;

    for (SLONG c = 0; c < Sim.Players.AnzPlayers; c++) {
        if ((Sim.Players.Players[c].IsOut == 0) && (Sim.Players.Players[c].Owner == 0 || Sim.Players.Players[c].Owner == 2)) {
            PLAYER &qPlayer = Sim.Players.Players[c];

            for (SLONG d = 0; d < qPlayer.Planes.AnzEntries(); d++) {
                if (qPlayer.Planes.IsInAlbum(d) != 0) {
                    if (qPlayer.Planes[d].MaxPassagiere + qPlayer.Planes[d].MaxPassagiereFC > PlayerMaxPassagiere) {
                        PlayerMaxPassagiere = qPlayer.Planes[d].ptPassagiere;
                    }

                    if (qPlayer.Planes[d].ptReichweite > PlayerMaxLength) {
                        PlayerMaxLength = qPlayer.Planes[d].ptReichweite;
                    }

                    if (qPlayer.Planes[d].MaxPassagiere + qPlayer.Planes[d].MaxPassagiereFC < PlayerMinPassagiere) {
                        PlayerMinPassagiere = qPlayer.Planes[d].ptPassagiere;
                    }

                    if (qPlayer.Planes[d].ptReichweite < PlayerMinLength) {
                        PlayerMinLength = qPlayer.Planes[d].ptReichweite;
                    }
                }
            }
        }
    }

    PlayerMaxLength *= 1000;
    PlayerMinLength *= 1000;
}

//============================================================================================
// Läßt nötigenfalls den Auftragsberater mit einem Spruch erscheien
//============================================================================================
void PLAYER::CheckAuftragsBerater(const CAuftrag &Auftrag) {
    if (HasBerater(BERATERTYP_AUFTRAG) != 0) {
        SLONG d = 0;
        SLONG Okay = 0;

        for (d = 0, Okay = FALSE; d < Planes.AnzEntries(); d++) {
            if (Planes.IsInAlbum(d) != 0) {
                Okay |= Auftrag.FitsInPlane(Planes[d]);
                // Okay|=Auftrag.FitsInPlane (PlaneTypes[(SLONG)Planes[d].TypeId]);
            }
        }

        for (d = 0; d < Planes.AnzEntries(); d++) {
            if (Planes.IsInAlbum(d) != 0) {
                CPlane &qPlane = Planes[d];

                for (SLONG e = 0; e < qPlane.Flugplan.Flug.AnzEntries(); e++) {
                    if (qPlane.Flugplan.Flug[e].ObjectType == 3) {
                        CFlugplanEintrag &qFPE = qPlane.Flugplan.Flug[e];

                        if (qFPE.Startdate > Sim.Date || (qFPE.Startdate == Sim.Date && qFPE.Startzeit > Sim.GetHour() + 1)) {
                            if (Auftrag.VonCity == qFPE.VonCity && Auftrag.NachCity == qFPE.NachCity && Auftrag.Date <= qFPE.Startdate &&
                                Auftrag.BisDate >= qFPE.Startdate && SLONG(Auftrag.Personen) <= qPlane.MaxPassagiere + qPlane.MaxPassagiereFC) {
                                // Auftrag ersetzt Autoflug:
                                Messages.AddMessage(BERATERTYP_AUFTRAG, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 7013), (LPCTSTR)qPlane.Name), MESSAGE_COMMENT,
                                                    SMILEY_GREAT);
                                return;
                            }
                        }
                    }
                }
            }
        }

        if (Okay == 0) {
            // Kein Flugzeug, was die Entfernung schafft:
            Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7010), MESSAGE_COMMENT, SMILEY_BAD);
        } else {
            for (d = 0, Okay = FALSE; d < Planes.AnzEntries(); d++) {
                if (Planes.IsInAlbum(d) != 0) {
                    Okay |= static_cast<SLONG>(SLONG(Auftrag.Personen) <= Planes[d].MaxPassagiere + Planes[d].MaxPassagiereFC);
                }
            }

            if (Okay == 0) {
                // Kein Flugzeug, was so viele Personen schafft:
                Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7011), MESSAGE_COMMENT, SMILEY_BAD);
            } else {
                for (d = 0, Okay = FALSE; d < Planes.AnzEntries(); d++) {
                    if (Planes.IsInAlbum(d) != 0) {
                        Okay |= static_cast<SLONG>((SLONG(Auftrag.Personen) <= Planes[d].MaxPassagiere + Planes[d].MaxPassagiereFC) &&
                                                   (Auftrag.FitsInPlane(Planes[d]) != 0));
                        // Auftrag.FitsInPlane (PlaneTypes[(SLONG)Planes[d].TypeId]) );
                    }
                }

                if (Okay == 0) {
                    // Kein Flugzeug, was so viele Personen schafft:
                    Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7012), MESSAGE_COMMENT, SMILEY_BAD);
                } else {
                    SLONG Cost = ((CalculateFlightCost(Auftrag.VonCity, Auftrag.NachCity, 8000, 700, -1)) + 99) / 100 * 100;

                    if (Auftrag.Strafe == 0) {
                        Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7000), MESSAGE_COMMENT, SMILEY_NEUTRAL);
                    } else if (Cost <= Auftrag.Praemie * 5 / 10) {
                        Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7001), MESSAGE_COMMENT, SMILEY_GREAT);
                    } else if (Cost <= Auftrag.Praemie * 9 / 10) {
                        Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7002), MESSAGE_COMMENT, SMILEY_GOOD);
                    } else if (Cost <= Auftrag.Praemie * 14 / 10) {
                        Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7003), MESSAGE_COMMENT, SMILEY_NEUTRAL);
                    } else {
                        Messages.AddMessage(BERATERTYP_AUFTRAG, StandardTexte.GetS(TOKEN_ADVICE, 7004), MESSAGE_COMMENT, SMILEY_BAD);
                    }
                }
            }
        }
    }
}

//============================================================================================
// CAuftrag::
//============================================================================================
// Konstruktor:
//============================================================================================
CAuftrag::CAuftrag(ULONG VonCity, ULONG NachCity, ULONG Personen, UWORD Date, SLONG Praemie, SLONG Strafe) {
    CAuftrag::VonCity = VonCity;
    CAuftrag::NachCity = NachCity;
    CAuftrag::Personen = Personen;
    CAuftrag::Date = Date;
    CAuftrag::InPlan = 0;
    CAuftrag::Praemie = Praemie;
    CAuftrag::Strafe = Strafe;
    CAuftrag::bUhrigFlight = FALSE;
}

//--------------------------------------------------------------------------------------------
// Konstruktor:
//--------------------------------------------------------------------------------------------
CAuftrag::CAuftrag(ULONG VonCity, ULONG NachCity, ULONG Personen, UWORD Date) {
    CAuftrag::VonCity = VonCity;
    CAuftrag::NachCity = NachCity;
    CAuftrag::Personen = Personen;
    CAuftrag::Date = Date;
    CAuftrag::InPlan = 0;
    CAuftrag::Praemie = 100; //==>+<==
    CAuftrag::Strafe = 100;
    CAuftrag::bUhrigFlight = FALSE;
}

//--------------------------------------------------------------------------------------------
// Konstruktor:
//--------------------------------------------------------------------------------------------
CAuftrag::CAuftrag(char *VonCity, char *NachCity, ULONG Personen, UWORD Date, SLONG Praemie, SLONG Strafe) {
    CAuftrag::VonCity = Cities.GetIdFromName(VonCity);
    CAuftrag::NachCity = Cities.GetIdFromName(NachCity);
    CAuftrag::Personen = Personen;
    CAuftrag::Date = Date;
    CAuftrag::InPlan = 0;
    CAuftrag::Praemie = Praemie;
    CAuftrag::Strafe = Strafe;
    CAuftrag::bUhrigFlight = FALSE;
}

//--------------------------------------------------------------------------------------------
// Konstruktor:
//--------------------------------------------------------------------------------------------
CAuftrag::CAuftrag(char *VonCity, char *NachCity, ULONG Personen, UWORD Date) {
    CAuftrag::VonCity = Cities.GetIdFromName(VonCity);
    CAuftrag::NachCity = Cities.GetIdFromName(NachCity);
    CAuftrag::Personen = Personen;
    CAuftrag::Date = Date;
    CAuftrag::InPlan = 0;
    CAuftrag::Praemie = 100; //==>+<==
    CAuftrag::Strafe = 100;
    CAuftrag::bUhrigFlight = FALSE;
}

//--------------------------------------------------------------------------------------------
// Legt Städte fest: (AreaType: 0=Region-gleicher Region, 1=Europa-Europa, 2=alles
//--------------------------------------------------------------------------------------------
void CAuftrag::RandomCities(SLONG AreaType, SLONG HomeCity, TEAKRAND *pRandom) {
    SLONG TimeOut = 0;

    do {
        switch (AreaType) {
        // Region-Gleiche Region:
        case 0: {
            SLONG Area = 1 + pRandom->Rand(4);

            VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Area, pRandom));
            NachCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Area, pRandom));
        } break;

            // Europa:
        case 1:
            VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Cities[Sim.HomeAirportId].Areacode, pRandom));
            NachCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Cities[Sim.HomeAirportId].Areacode, pRandom));
            switch (pRandom->Rand(2)) {
            case 0:
                VonCity = HomeCity;
                break;
            case 1:
                NachCity = HomeCity;
                break;
            default:
                hprintf("Auftrag.cpp: Default case should not be reached.");
                DebugBreak();
            }
            break;

            // Irgendwas:
        case 2:
            VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(pRandom));
            NachCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(pRandom));
            break;

            // Die ersten Tage:
        case 4:
            VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Cities[Sim.HomeAirportId].Areacode, pRandom));
            NachCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(Cities[Sim.HomeAirportId].Areacode, pRandom));
            switch (pRandom->Rand(2)) {
            case 0:
                VonCity = HomeCity;
                break;
            case 1:
                NachCity = HomeCity;
                break;
            default:
                hprintf("Auftrag.cpp: Default case should not be reached.");
                DebugBreak();
            }
            break;
        default:
            hprintf("Auftrag.cpp: Default case should not be reached.");
            DebugBreak();
        }

        TimeOut++;

        if (VonCity < 0x1000000) {
            VonCity = Cities.GetIdFromIndex(VonCity);
        }
        if (NachCity < 0x1000000) {
            NachCity = Cities.GetIdFromIndex(NachCity);
        }

        if (TimeOut > 300 && VonCity != NachCity) {
            break;
        }
    } while (VonCity == NachCity || (AreaType == 4 && Cities.CalcDistance(VonCity, NachCity) > 10000000));
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag ein:
//--------------------------------------------------------------------------------------------
void CAuftrag::RefillForLastMinute(SLONG AreaType, TEAKRAND *pRandom) {
    SLONG TimeOut = 0;

    // NetGenericSync (2000, AreaType);
    // NetGenericSync (2001, pRandom->GetSeed());

too_large:
    RandomCities(AreaType, Sim.HomeAirportId, pRandom);

    Date = UWORD(Sim.Date);
    BisDate = Date;
    InPlan = 0;

    // Kopie dieser Formel auch bei Last-Minute
    Praemie = ((CalculateFlightCost(VonCity, NachCity, 8000, 700, -1)) + 99) / 100 * 130;
    Strafe = Praemie / 2 * 100 / 100;

    Praemie = Praemie * 5 / 4; // Last-Minute Zuschlag

    SLONG Type = pRandom->Rand(100);

    // Typ A = Normal, Gewinn möglich, etwas Strafe
    if (Type >= 0 && Type < 50) {
    }
    // Typ B = Hoffmann, Gewinn möglich, keine Strafe
    else if (Type >= 50 && Type < 60) {
        Date++;
        BisDate++;
    }
    // Typ C = Zeit knapp, viel Gewinn, viel Strafe
    else if (Type >= 60 && Type < 80) {
        Praemie *= 2;
        Strafe = Praemie * 4;
    }
    // Typ D = Betrug, kein Gewinn möglich, etwas Strafe
    else if (Type >= 80 && Type < 95) {
        Praemie /= 2;
    }
    // Typ E = Glücksfall, viel Gewinn, keine Strafe
    else if (Type >= 95 && Type < 100) {
        Praemie *= 2;
        Strafe = 0;
    }

    Type = pRandom->Rand(100);

    if (Type == 0) {
        Personen = 1;
        Praemie = Praemie * 4;
        Strafe = Praemie * 4;
    } else if (Type < 15 || (Sim.Date < 4 && Type < 30) || (Sim.Date < 8 && Type < 20) || (Sim.Difficulty == DIFF_TUTORIAL && Type < 70)) {
        Personen = 90;
        Praemie = Praemie * 3 / 4;
    } else if (Type < 40 || Sim.Difficulty == DIFF_TUTORIAL) {
        Personen = 180;
        /* Praemie bleibt gleich */
    } else if (Type < 70) {
        Personen = 280;
        Praemie = Praemie * 5 / 4;
    } else if (Type < 90) {
        Personen = 340;
        Praemie = Praemie * 6 / 4;
    } else {
        Personen = 430;
        Praemie = Praemie * 7 / 4;
    }

    if (AreaType == 1) {
        Praemie = Praemie * 3 / 2;
    }
    if (AreaType == 2) {
        Praemie = Praemie * 8 / 5;
    }

    if ((SLONG(Personen) > PlayerMaxPassagiere || Cities.CalcDistance(VonCity, NachCity) > PlayerMaxLength) && TimeOut++ < 100) {
        goto too_large;
    }
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag ein:
//--------------------------------------------------------------------------------------------
void CAuftrag::RefillForReisebuero(SLONG AreaType, TEAKRAND *pRandom) {
    SLONG TimeOut = 0;

    // NetGenericSync (3000, AreaType);
    // NetGenericSync (3001, pRandom->GetSeed());

too_large:
    RandomCities(AreaType, Sim.HomeAirportId, pRandom);

    Personen = 180;
    Date = UWORD(Sim.Date + 1 + pRandom->Rand(3));
    BisDate = Date;
    InPlan = 0;

    // Kopie dieser Formel auch bei Last-Minute
    Praemie = ((CalculateFlightCost(VonCity, NachCity, 8000, 700, -1)) + 99) / 100 * 105;
    Strafe = Praemie / 2 * 100 / 100;

    if (pRandom->Rand(5) == 4) {
        BisDate += ((pRandom->Rand(5)));
    }

    SLONG Type = pRandom->Rand(100);

    // Typ A = Normal, Gewinn möglich, etwas Strafe
    if (Type >= 0 && Type < 50) {
    }
    // Typ B = Hoffmann, Gewinn möglich
    else if (Type >= 50 && Type < 60) {
        Date = UWORD(Sim.Date);
        BisDate = UWORD(Sim.Date + 4 + pRandom->Rand(3));
    }
    // Typ C = Zeit knapp, viel Gewinn, viel Strafe
    else if (Type >= 70 && Type < 80) {
        Praemie *= 2;
        Strafe = Praemie * 2;
        BisDate = Date = UWORD(Sim.Date + 1);
    }
    // Typ D = Betrug, kein Gewinn möglich, etwas Strafe
    else if (Type >= 80 && Type < 95) {
        Praemie /= 2;
    }
    // Typ E = Glücksfall, viel Gewinn, keine Strafe
    else if (Type >= 95 && Type < 100) {
        Praemie *= 2;
        Strafe = 0;
    }

    Type = pRandom->Rand(100);

    if (Type == 0) {
        Personen = 2;
        Praemie = Praemie * 4;
        Strafe = Praemie * 4;
    } else if (Type < 15 || (Sim.Date < 4 && Type < 30) || (Sim.Date < 8 && Type < 20) || (Sim.Difficulty == DIFF_TUTORIAL && Type < 70)) {
        Personen = 90;
        Praemie = Praemie * 3 / 4;
    } else if (Type < 40 || Sim.Difficulty == DIFF_TUTORIAL) {
        Personen = 180;
        /* Praemie bleibt gleich */
    } else if (Type < 70) {
        Personen = 280;
        Praemie = Praemie * 5 / 4;
    } else if (Type < 90) {
        Personen = 340;
        Praemie = Praemie * 6 / 4;
    } else {
        Personen = 430;
        Praemie = Praemie * 7 / 4;
    }

    if (AreaType == 1) {
        Praemie = Praemie * 3 / 2;
    }
    if (AreaType == 2) {
        Praemie = Praemie * 8 / 5;
    }
    if (Date != BisDate) {
        Date = static_cast<UWORD>(Sim.Date);
        Praemie = Praemie * 4 / 5;
    }

    if ((SLONG(Personen) > PlayerMaxPassagiere || Cities.CalcDistance(VonCity, NachCity) > PlayerMaxLength) && TimeOut++ < 100) {
        goto too_large;
    }
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag für den Spielbeginn ein:
//--------------------------------------------------------------------------------------------
void CAuftrag::RefillForBegin(SLONG AreaType, TEAKRAND *pRandom) {
    SLONG TimeOut = 0;

    // NetGenericSync (4000, AreaType);
    // NetGenericSync (4001, pRandom->GetSeed());

too_large:
    do {
        RandomCities(AreaType, Sim.HomeAirportId, pRandom);

        VonCity = Sim.HomeAirportId;
    } while (VonCity == NachCity || Cities.CalcDistance(VonCity, NachCity) > 5000000);

    Date = UWORD(Sim.Date);
    BisDate = UWORD(Sim.Date);
    Personen = 90;
    InPlan = 0;
    Praemie = ((CalculateFlightCost(VonCity, NachCity, 8000, 700, -1)) + 99) / 100 * 100;
    Strafe = Praemie / 2 * 100 / 100;

    SLONG Type = pRandom->Rand(100);

    // Typ A = Normal, Gewinn möglich, etwas Strafe
    if (Type >= 0 && Type < 50) {
    }
    // Typ B = Hoffmann, Gewinn möglich, keine Strafe
    else if (Type >= 50 && Type < 60) {
        BisDate = UWORD(Sim.Date + 6);
    }
    // Typ C = Zeit knapp, viel Gewinn, viel Strafe
    else if (Type >= 70 && Type < 95) {
        Praemie *= 2;
        Strafe = Praemie * 2;
        BisDate = UWORD(Sim.Date + 1);
    }
    // Typ E = Glücksfall, viel Gewinn, keine Strafe
    else if (Type >= 95 && Type < 100) {
        Praemie *= 2;
        Strafe = 0;
    }

    if (AreaType == 1) {
        Praemie = Praemie * 3 / 2;
    }
    if (AreaType == 2) {
        Praemie = Praemie * 8 / 5;
    }
    if (Date != BisDate) {
        Praemie = Praemie * 4 / 5;
    }

    TimeOut++;

    if ((SLONG(Personen) > PlayerMaxPassagiere || Cities.CalcDistance(VonCity, NachCity) > PlayerMaxLength) && TimeOut < 80) {
        goto too_large;
    }

    if ((SLONG(Personen) > PlayerMinPassagiere || Cities.CalcDistance(VonCity, NachCity) > PlayerMinLength) && TimeOut < 60) {
        goto too_large;
    }

    if ((SLONG(Personen) > PlayerMaxPassagiere || Cities.CalcDistance(VonCity, NachCity) > PlayerMaxLength)) {
        Strafe = 0;
        Praemie = Praemie * 2;
        Date = static_cast<UWORD>(Sim.Date);
        BisDate = Date + 5;
    }
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag für Uhrig ein:
//--------------------------------------------------------------------------------------------
void CAuftrag::RefillForUhrig(SLONG AreaType, TEAKRAND *pRandom) {
    SLONG TimeOut = 0;

    // NetGenericSync (5000, AreaType);
    // NetGenericSync (5001, pRandom->GetSeed());

too_large:
    RandomCities(AreaType, Sim.HomeAirportId, pRandom);

    Personen = 180;
    Date = UWORD(Sim.Date + 1 + pRandom->Rand(3));
    BisDate = Date;
    InPlan = 0;
    bUhrigFlight = TRUE;

    // Kopie dieser Formel auch bei Last-Minute
    Praemie = ((CalculateFlightCost(VonCity, NachCity, 8000, 700, -1)) + 99) / 100 * 115;
    Strafe = Praemie / 2 * 100 / 100 * 2;

    if (pRandom->Rand(5) == 4) {
        BisDate += ((pRandom->Rand(5)));
    }

    SLONG Type = pRandom->Rand(100);

    // Typ A = Normal, Gewinn möglich, etwas Strafe
    if (Type >= 0 && Type < 50) {
    }
    // Typ B = Hoffmann, Gewinn möglich
    else if (Type >= 50 && Type < 60) {
        Date = UWORD(Sim.Date);
        BisDate = UWORD(Sim.Date + 4 + pRandom->Rand(3));
    }
    // Typ C = Zeit knapp, viel Gewinn, viel Strafe
    else if (Type >= 70 && Type < 80) {
        Praemie *= 2;
        Strafe = Praemie * 2;
        BisDate = Date = UWORD(Sim.Date + 1);
    }
    // Typ D = Betrug, kein Gewinn möglich, etwas Strafe
    else if (Type >= 80 && Type < 95) {
        Praemie /= 2;
    }
    // Typ E = Glücksfall, viel Gewinn, keine Strafe
    else if (Type >= 95 && Type < 100) {
        Praemie *= 2;
        Strafe = 0;
    }

    Type = pRandom->Rand(100);

    if (Type == 0) {
        Personen = 2;
        Praemie = Praemie * 4;
        Strafe = Praemie * 4;
    } else if (Type < 15 || (Sim.Date < 4 && Type < 30) || (Sim.Date < 8 && Type < 20) || (Sim.Difficulty == DIFF_TUTORIAL && Type < 70)) {
        Personen = 90;
        Praemie = Praemie * 3 / 4;
    } else if (Type < 40 || Sim.Difficulty == DIFF_TUTORIAL) {
        Personen = 180;
        /* Praemie bleibt gleich */
    } else if (Type < 70) {
        Personen = 280;
        Praemie = Praemie * 5 / 4;
    } else if (Type < 90) {
        Personen = 340;
        Praemie = Praemie * 6 / 4;
    } else {
        Personen = 430;
        Praemie = Praemie * 7 / 4;
    }

    if (AreaType == 1) {
        Praemie = Praemie * 3 / 2;
    }
    if (AreaType == 2) {
        Praemie = Praemie * 8 / 5;
    }
    if (Date != BisDate) {
        Date = static_cast<UWORD>(Sim.Date);
        Praemie = Praemie * 4 / 5;
    }

    if ((SLONG(Personen) > PlayerMaxPassagiere || Cities.CalcDistance(VonCity, NachCity) > PlayerMaxLength) && TimeOut++ < 100) {
        goto too_large;
    }
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag für den Spielbeginn ein:
//--------------------------------------------------------------------------------------------
void CAuftrag::RefillForAusland(SLONG AreaType, SLONG CityNum, TEAKRAND *pRandom) {
    SLONG TimeOut = 0;

    TEAKRAND localRand;
    localRand.SRand(pRandom->Rand());

    if (CityNum < 0x1000000) {
        CityNum = Cities.GetIdFromIndex(CityNum);
    }

too_large:

    do {
        VonCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(&localRand));
        NachCity = Cities.GetIdFromIndex(Cities.GetRandomUsedIndex(&localRand));
        switch (localRand.Rand(2)) {
        case 0:
            VonCity = CityNum;
            break;
        case 1:
            NachCity = CityNum;
            break;
        default:
            hprintf("Auftrag.cpp: Default case should not be reached.");
            DebugBreak();
        }
    } while (VonCity == NachCity || (AreaType == 4 && Cities.CalcDistance(VonCity, NachCity) > 10000000));

    Personen = 180;
    Date = UWORD(Sim.Date + 1 + localRand.Rand(3));
    BisDate = Date;
    InPlan = 0;

    // Kopie dieser Formel auch bei Last-Minute
    Praemie = ((CalculateFlightCost(VonCity, NachCity, 8000, 700, -1)) + 99) / 100 * 120;
    Strafe = Praemie / 2 * 100 / 100;

    if (localRand.Rand(5) == 4) {
        BisDate += ((localRand.Rand(5)));
    }

    SLONG Type = localRand.Rand(100);

    // Typ A = Normal, Gewinn möglich, etwas Strafe
    if (Type >= 0 && Type < 50) {
    }
    // Typ B = Hoffmann, Gewinn möglich
    else if (Type >= 50 && Type < 65) {
        Date = UWORD(Sim.Date);
        BisDate = UWORD(Sim.Date + 4 + localRand.Rand(3));
    }
    // Typ C = Zeit knapp, viel Gewinn, viel Strafe
    else if (Type >= 65 && Type < 80) {
        Praemie *= 2;
        Strafe = Praemie * 2;
        BisDate = Date = UWORD(Sim.Date + 1);
    }
    // Typ D = Betrug, kein Gewinn möglich, etwas Strafe
    else if (Type >= 80 && Type < 95) {
        Praemie /= 2;
    }
    // Typ E = Glücksfall, viel Gewinn, keine Strafe
    else if (Type >= 95 && Type < 100) {
        Praemie *= 2;
        Strafe = 0;
    }

    // Type = pRandom->Rand (100);
    Type = localRand.Rand(100);

    if (Type == 0) {
        Personen = 2;
        Praemie = Praemie * 4;
        Strafe = Praemie * 4;
    } else if (Type < 15 || (Sim.Date < 4 && Type < 30) || (Sim.Date < 8 && Type < 20) || (Sim.Difficulty == DIFF_TUTORIAL && Type < 70)) {
        Personen = 90;
        Praemie = Praemie * 3 / 4;
    } else if (Type < 40 || Sim.Difficulty == DIFF_TUTORIAL) {
        Personen = 180;
        /* Praemie bleibt gleich */
    } else if (Type < 70) {
        Personen = 280;
        Praemie = Praemie * 5 / 4;
    } else if (Type < 90) {
        Personen = 340;
        Praemie = Praemie * 6 / 4;
    } else {
        Personen = 430;
        Praemie = Praemie * 7 / 4;
    }

    if (AreaType == 1) {
        Praemie = Praemie * 3 / 2;
    }
    if (AreaType == 2) {
        Praemie = Praemie * 8 / 5;
    }
    if (Date != BisDate) {
        Date = static_cast<UWORD>(Sim.Date);
        Praemie = Praemie * 4 / 5;
    }

    if ((SLONG(Personen) > PlayerMaxPassagiere || Cities.CalcDistance(VonCity, NachCity) > PlayerMaxLength) && TimeOut++ < 100) {
        goto too_large;
    }
}

//--------------------------------------------------------------------------------------------
// Das Raster zum ausgrauen der ungültigen Tage zeichnen:
//--------------------------------------------------------------------------------------------
void CAuftrag::BlitGridAt(SBBM *pBitmap, XY Offset, BOOL Tagesansicht, SLONG /*Page*/) const {
    SLONG c = 0;

    if (Tagesansicht == 0) {
        for (c = 0; c < 7; c++) {
            if (Sim.Date + c < Date || Sim.Date + c > BisDate) {
                pBitmap->BlitFromT(FlugplanBms[51], Offset.x, Offset.y + FlugplanBms[51].Size.y * c);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Passt der Auftrag in ein Flugzeug von der Sorte?
//--------------------------------------------------------------------------------------------
BOOL CAuftrag::FitsInPlane(const CPlane &Plane) const {
    if (Cities.CalcDistance(VonCity, NachCity) > Plane.ptReichweite * 1000) {
        return (FALSE);
    }
    // if ((Cities.CalcDistance (VonCity, NachCity)/PlaneType.Geschwindigkeit+999)/1000+1+2>=24)
    // if (Cities.CalcFlugdauer (VonCity, NachCity, PlaneType.Geschwindigkeit)>=24)
    if (Cities.CalcFlugdauer(VonCity, NachCity, Plane.ptGeschwindigkeit) >= 24) {
        return (FALSE);
    }

    return (TRUE);
}

//--------------------------------------------------------------------------------------------
// Ein CAuftrag-Datum speichern:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CAuftrag &Auftrag) {
    File << Auftrag.VonCity << Auftrag.NachCity << Auftrag.Personen << Auftrag.Date << Auftrag.InPlan << Auftrag.Okay << Auftrag.Praemie << Auftrag.Strafe
         << Auftrag.BisDate;

    if (SaveVersionSub >= 100) {
        File << Auftrag.bUhrigFlight;
    }

    return (File);
}

//--------------------------------------------------------------------------------------------
// Ein CAuftrag-Datum laden:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CAuftrag &Auftrag) {
    File >> Auftrag.VonCity >> Auftrag.NachCity >> Auftrag.Personen >> Auftrag.Date >> Auftrag.InPlan >> Auftrag.Okay >> Auftrag.Praemie >> Auftrag.Strafe >>
        Auftrag.BisDate;

    if (SaveVersionSub >= 100) {
        File >> Auftrag.bUhrigFlight;
    } else {
        Auftrag.bUhrigFlight = FALSE;
    }

    return (File);
}

//============================================================================================
// CAuftraege::
//============================================================================================
// Fügt eine Reihe von neuen Aufträgen ein:
//============================================================================================
void CAuftraege::FillForLastMinute() {
    CalcPlayerMaximums();

    ReSize(6); // ex:10
    FillAlbum();
    
    for (auto &a : *this) {
        a.RefillForLastMinute(1, &Random);
    }

    if (Sim.Difficulty == DIFF_ATFS10 && Sim.Date >= 20 && Sim.Date <= 30) {
        for (auto &a : *this) {
            a.Praemie = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag ein:
//--------------------------------------------------------------------------------------------
void CAuftraege::RefillForLastMinute(SLONG Minimum) {
    SLONG Anz = min(AnzEntries(), Sim.TickLastMinuteRefill);

    NetGenericSync(7000, Minimum);
    NetGenericSync(7001, Sim.TickLastMinuteRefill);
    NetGenericSync(7002, Random.GetSeed());

    CalcPlayerMaximums();

    ReSize(6); // ex:10
    FillAlbum();
    
    for (auto &a : *this) {
        if (Anz <= 0) {
            break;
        }
        if (a.Praemie == 0) {
            a.RefillForLastMinute(1, &Random);
            Anz--;
        }
    }

    for (auto &a : *this) {
        if (Anz <= 0) {
            break;
        }
        if (a.Praemie != 0) {
            Minimum--;
        }
    }
    for (auto &a : *this) {
        if (Anz <= 0) {
            break;
        }
        if (a.Praemie == 0 && Minimum > 0) {
            a.RefillForLastMinute(1, &Random);
            Minimum--;
        }
    }

    Sim.TickLastMinuteRefill = 0;

    if (Sim.Difficulty == DIFF_ATFS10 && Sim.Date >= 20 && Sim.Date <= 30) {
        for (auto &a : *this) {
            a.Praemie = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Fügt eine Reihe von neuen Aufträgen ein:
//--------------------------------------------------------------------------------------------
void CAuftraege::FillForReisebuero() {

    CalcPlayerMaximums();

    ReSize(6);
    FillAlbum();

    SLONG c = 0;
    for (auto &a : *this) {
        if (Sim.Date < 5 && c < 5) {
            a.RefillForAusland(4, Sim.HomeAirportId, &Random);
        } else if (Sim.Date < 10 && c < 3) {
            a.RefillForAusland(4, Sim.HomeAirportId, &Random);
        } else {
            a.RefillForAusland(1, Sim.HomeAirportId, &Random);
        }
        c++;
    }

    if (Sim.Difficulty == DIFF_ATFS10 && Sim.Date >= 20 && Sim.Date <= 30) {
        for (auto &a : *this) {
            a.Praemie = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag ein:
//--------------------------------------------------------------------------------------------
void CAuftraege::RefillForReisebuero(SLONG Minimum) {
    SLONG Anz = min(AnzEntries(), Sim.TickReisebueroRefill);

    // NetGenericSync (8000, Minimum);
    // NetGenericSync (8001, Random.GetSeed());

    CalcPlayerMaximums();

    ReSize(6);
    FillAlbum();

    SLONG c = 0;
    for (auto &a : *this) {
        if (Anz <= 0) {
            break;
        }
        if (a.Praemie == 0) {
            if (Sim.Date < 5 && c < 5) {
                a.RefillForAusland(4, Sim.HomeAirportId, &Random);
            } else if (Sim.Date < 10 && c < 3) {
                a.RefillForAusland(4, Sim.HomeAirportId, &Random);
            } else {
                a.RefillForAusland(1, Sim.HomeAirportId, &Random);
            }

            Anz--;
        }
        c++;
    }

    for (auto &a : *this) {
        if (Anz <= 0) {
            break;
        }
        if (a.Praemie != 0) {
            Minimum--;
        }
    }

    c = 0;
    for (auto &a : *this) {
        if (Anz <= 0) {
            break;
        }
        if (a.Praemie == 0 && Minimum > 0) {
            if (Sim.Date < 5 && c < 5) {
                a.RefillForAusland(4, Sim.HomeAirportId, &Random);
            } else if (Sim.Date < 10 && c < 3) {
                a.RefillForAusland(4, Sim.HomeAirportId, &Random);
            } else {
                a.RefillForAusland(1, Sim.HomeAirportId, &Random);
            }

            Minimum--;
        }
        c++;
    }

    Sim.TickReisebueroRefill = 0;

    if (Sim.Difficulty == DIFF_ATFS10 && Sim.Date >= 20 && Sim.Date <= 30) {
        for (auto &a : *this) {
            a.Praemie = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Fügt eine Reihe von neuen Aufträgen ein:
//--------------------------------------------------------------------------------------------
void CAuftraege::FillForAusland(SLONG CityNum) {

    CalcPlayerMaximums();

    ReSize(6); // ex:10
    FillAlbum();

    SLONG c = 0;
    for (auto &a : *this) {
        if (Sim.Date < 5 && c < 5) {
            a.RefillForAusland(4, CityNum, &Random);
        } else if (Sim.Date < 10 && c < 3) {
            a.RefillForAusland(4, CityNum, &Random);
        } else {
            a.RefillForAusland(1, CityNum, &Random);
        }
        c++;
    }
}

//--------------------------------------------------------------------------------------------
// Fügt einen neuen Auftrag ein:
//--------------------------------------------------------------------------------------------
void CAuftraege::RefillForAusland(SLONG CityNum, SLONG Minimum) {
    SLONG Anz = min(AnzEntries(), AuslandsRefill[CityNum]);

    CalcPlayerMaximums();

    ReSize(6);
    FillAlbum();

    SLONG c = 0;
    for (auto &a : *this) {
        if (Anz <= 0) {
            break;
        }
        if (a.Praemie == 0) {
            if (Sim.Date < 5 && c < 5) {
                a.RefillForAusland(4, CityNum, &Random);
            } else if (Sim.Date < 10 && c < 3) {
                a.RefillForAusland(4, CityNum, &Random);
            } else {
                a.RefillForAusland(1, CityNum, &Random);
            }

            Anz--;
        }
        c++;
    }

    for (auto &a : *this) {
        if (Anz <= 0) {
            break;
        }
        if (a.Praemie != 0) {
            Minimum--;
        }
    }

    c = 0;
    for (auto &a : *this) {
        if (Anz <= 0) {
            break;
        }
        if (a.Praemie == 0 && Minimum > 0) {
            if (Sim.Date < 5 && c < 5) {
                a.RefillForAusland(4, CityNum, &Random);
            } else if (Sim.Date < 10 && c < 3) {
                a.RefillForAusland(4, CityNum, &Random);
            } else {
                a.RefillForAusland(1, CityNum, &Random);
            }

            Minimum--;
        }
        c++;
    }

    AuslandsRefill[CityNum] = 0;
}

//--------------------------------------------------------------------------------------------
// Returns the number of open Order flights which are due today:
//--------------------------------------------------------------------------------------------
SLONG CAuftraege::GetNumDueToday() {
    SLONG Anz = 0;

    for (SLONG c = 0; c < AnzEntries(); c++) {
        if ((IsInAlbum(c) != 0) && at(c).BisDate >= Sim.Date) {
            if (at(c).InPlan == 0 && at(c).BisDate == Sim.Date) {
                Anz++;
            }
        }
    }

    return (Anz);
}

//--------------------------------------------------------------------------------------------
// Returns the number of open Order flights which still need to be done:
//--------------------------------------------------------------------------------------------
SLONG CAuftraege::GetNumOpen() {
    SLONG Anz = 0;

    for (SLONG c = 0; c < AnzEntries(); c++) {
        if ((IsInAlbum(c) != 0) && at(c).BisDate >= Sim.Date) {
            if (at(c).InPlan == 0) {
                Anz++;
            }
        }
    }

    return (Anz);
}

//--------------------------------------------------------------------------------------------
// Speichert ein CAuftrag Datum:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CAuftraege &Auftraege) {
    File << *((const ALBUM_V<CAuftrag> *)&Auftraege);

    if (SaveVersionSub >= 100) {
        File << Auftraege.Random;
    }

    return (File);
}

//--------------------------------------------------------------------------------------------
// Läd ein CAuftrag Datum:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CAuftraege &Auftraege) {
    File >> *((ALBUM_V<CAuftrag> *)&Auftraege);

    if (SaveVersionSub >= 100) {
        File >> Auftraege.Random;
    } else {
        Auftraege.Random.SRand(0);
    }

    return (File);
}
