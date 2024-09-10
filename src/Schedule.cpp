//============================================================================================
// Schedule.cpp - Der Bildschirm für den Scheduler
//============================================================================================
#include "StdAfx.h"
#include "AtNet.h"

#define AT_Error(...) Hdu.HercPrintfMsg(SDL_LOG_PRIORITY_ERROR, "Schedule", __VA_ARGS__)
#define AT_Warn(...) Hdu.HercPrintfMsg(SDL_LOG_PRIORITY_WARN, "Schedule", __VA_ARGS__)
#define AT_Info(...) Hdu.HercPrintfMsg(SDL_LOG_PRIORITY_INFO, "Schedule", __VA_ARGS__)
#define AT_Log(...) AT_Log_I("Schedule", __VA_ARGS__)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern SLONG FoodCosts[];

// Daten des aktuellen Savegames beim laden:
extern SLONG SaveVersion;
extern SLONG SaveVersionSub;

__int64 abs64(__int64 v);

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
__int64 min64(__int64 a, __int64 b);

__int64 min64(__int64 a, __int64 b) {
    if (a < b) {
        return (a);
    }
    return (b);
}

//============================================================================================
// CFlugplanEintrag::
//============================================================================================
//--------------------------------------------------------------------------------------------
// Konstruktor:
//--------------------------------------------------------------------------------------------
CFlugplanEintrag::CFlugplanEintrag(BOOL ObjectType, ULONG ObjectId) {
    CFlugplanEintrag::Okay = 0;
    CFlugplanEintrag::Gate = -1;
    CFlugplanEintrag::GateWarning = FALSE;
    CFlugplanEintrag::Startzeit = 0;
    CFlugplanEintrag::Landezeit = 0;
    CFlugplanEintrag::ObjectType = ObjectType;
    CFlugplanEintrag::ObjectId = ObjectId;
    CFlugplanEintrag::FlightBooked = FALSE;
}

//--------------------------------------------------------------------------------------------
// Speichert ein CFlugplanEintrag Datum:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CFlugplanEintrag &Eintrag) {
    if (SaveVersion == 1 && SaveVersionSub == 1) {
        File << Eintrag.Okay << Eintrag.HoursBefore << Eintrag.Passagiere << Eintrag.PassagiereFC << Eintrag.PArrived << Eintrag.Gate << Eintrag.VonCity
             << Eintrag.GateWarning << Eintrag.NachCity << Eintrag.Startzeit << Eintrag.Landezeit << Eintrag.Startdate << Eintrag.Landedate
             << Eintrag.ObjectType << Eintrag.ObjectId << Eintrag.Ticketpreis << Eintrag.TicketpreisFC;
    } else if (SaveVersion == 1 && SaveVersionSub >= 2) {
        File << Eintrag.ObjectType;

        if (Eintrag.ObjectType != 0) {
            File << Eintrag.Okay << Eintrag.HoursBefore << Eintrag.Passagiere << Eintrag.PassagiereFC << Eintrag.PArrived << Eintrag.Gate << Eintrag.VonCity
                 << Eintrag.GateWarning << Eintrag.NachCity << Eintrag.Startzeit << Eintrag.Landezeit << Eintrag.Startdate << Eintrag.Landedate
                 << Eintrag.ObjectId << Eintrag.Ticketpreis << Eintrag.TicketpreisFC << Eintrag.FlightBooked;
        }
    }

    return (File);
}

//--------------------------------------------------------------------------------------------
// Läd ein CFlugplanEintrag Datum:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CFlugplanEintrag &Eintrag) {
    if (SaveVersion == 1 && SaveVersionSub == 1) {
        File >> Eintrag.Okay >> Eintrag.HoursBefore >> Eintrag.Passagiere >> Eintrag.PassagiereFC >> Eintrag.PArrived >> Eintrag.Gate >> Eintrag.VonCity >>
            Eintrag.GateWarning >> Eintrag.NachCity >> Eintrag.Startzeit >> Eintrag.Landezeit >> Eintrag.Startdate >> Eintrag.Landedate >> Eintrag.ObjectType >>
            Eintrag.ObjectId >> Eintrag.Ticketpreis >> Eintrag.TicketpreisFC;
    } else if (SaveVersion == 1 && SaveVersionSub >= 2) {
        File >> Eintrag.ObjectType;

        if (Eintrag.ObjectType != 0) {
            File >> Eintrag.Okay >> Eintrag.HoursBefore >> Eintrag.Passagiere >> Eintrag.PassagiereFC >> Eintrag.PArrived >> Eintrag.Gate >> Eintrag.VonCity >>
                Eintrag.GateWarning >> Eintrag.NachCity >> Eintrag.Startzeit >> Eintrag.Landezeit >> Eintrag.Startdate >> Eintrag.Landedate >>
                Eintrag.ObjectId >> Eintrag.Ticketpreis >> Eintrag.TicketpreisFC;

            if (SaveVersionSub >= 200) {
                File >> Eintrag.FlightBooked;
            } else {
                Eintrag.FlightBooked = FALSE;
            }
        } else {
            Eintrag.Okay = 0;
            Eintrag.Gate = -1;
            Eintrag.GateWarning = FALSE;
            Eintrag.Startzeit = 0;
            Eintrag.Landezeit = 0;
            Eintrag.ObjectType = 0;
            Eintrag.ObjectId = -1;
            Eintrag.FlightBooked = FALSE;
        }
    }

    return (File);
}

//============================================================================================
// CFlugplan::
//============================================================================================
//--------------------------------------------------------------------------------------------
// Aktualisiert "Next Flight":
//--------------------------------------------------------------------------------------------
void CFlugplan::UpdateNextFlight() {
    NextFlight = -1;
    for (SLONG e = 0; e < Flug.AnzEntries(); e++) {
        if (NextFlight == -1 && (Flug[e].ObjectType != 0) &&
            (Flug[e].Landedate > Sim.Date || (Flug[e].Landedate == Sim.Date && Flug[e].Landezeit >= Sim.GetHour()))) {
            NextFlight = e;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Gibt true zurück, wenn der Flugplan einen der beiden ObjectIds enthält:
//--------------------------------------------------------------------------------------------
BOOL CFlugplan::ContainsFlight(ULONG ObjectType, SLONG ObjectId1, SLONG ObjectId2) {
    for (SLONG e = 0; e < Flug.AnzEntries(); e++) {
        if (Flug[e].ObjectType == SLONG(ObjectType) && (Flug[e].ObjectId == ObjectId1 || Flug[e].ObjectId == ObjectId2)) {
            return (TRUE);
        }
    }

    return (FALSE);
}

//--------------------------------------------------------------------------------------------
// Aktualisiert "Next Start":
//--------------------------------------------------------------------------------------------
void CFlugplan::UpdateNextStart() {
    NextStart = -1;
    for (SLONG e = 0; e < Flug.AnzEntries(); e++) {
        if ((Flug[e].ObjectType != 0) && (Flug[e].Startdate > Sim.Date || (Flug[e].Startdate == Sim.Date && Flug[e].Startzeit + 1 >= Sim.GetHour()))) {
            NextStart = e;
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Druckt einen Flugplan auf dem Herkules-Schirm:
//--------------------------------------------------------------------------------------------
void CFlugplan::Dump(bool bHercules) {
    char Buffer[500];

    AT_Log("------------------------------------------------------------");
    for (SLONG e = 0; e < Flug.AnzEntries(); e++) {
        switch (Flug[e].ObjectType) {
        case 1:
            snprintf(Buffer, sizeof(Buffer), "[%02d] %02d/%02d-%02d/%02d Route   %s-%s", e, Flug[e].Startzeit, Flug[e].Startdate, Flug[e].Landezeit,
                     Flug[e].Landedate, (LPCTSTR)Cities[Flug[e].VonCity].Name, (LPCTSTR)Cities[Flug[e].NachCity].Name);
            break;

        case 2:
            snprintf(Buffer, sizeof(Buffer), "[%02d] %02d/%02d-%02d/%02d Auftrag %s-%s", e, Flug[e].Startzeit, Flug[e].Startdate, Flug[e].Landezeit,
                     Flug[e].Landedate, (LPCTSTR)Cities[Flug[e].VonCity].Name, (LPCTSTR)Cities[Flug[e].NachCity].Name);
            break;

        case 3:
            snprintf(Buffer, sizeof(Buffer), "[%02d] %02d/%02i-%02d/%02d Auto    %s-%s", e, Flug[e].Startzeit, Flug[e].Startdate, Flug[e].Landezeit,
                     Flug[e].Landedate, (LPCTSTR)Cities[Flug[e].VonCity].Name, (LPCTSTR)Cities[Flug[e].NachCity].Name);
            break;

        case 4:
            snprintf(Buffer, sizeof(Buffer), "[%02d] %02d/%02i-%02d/%02d Fracht  %s-%s", e, Flug[e].Startzeit, Flug[e].Startdate, Flug[e].Landezeit,
                     Flug[e].Landedate, (LPCTSTR)Cities[Flug[e].VonCity].Name, (LPCTSTR)Cities[Flug[e].NachCity].Name);
            break;

        default:
            continue;
        }

        if (bHercules) {
            AT_Log(Buffer);
        } else {
            DisplayBroadcastMessage(Buffer);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Speichert ein CFlugplan Datum:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CFlugplan &Plan) {
    File << Plan.Flug << Plan.StartCity << Plan.NextFlight << Plan.NextStart;
    return (File);
}

//--------------------------------------------------------------------------------------------
// Läd ein CFlugplan Datum:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CFlugplan &Plan) {
    File >> Plan.Flug >> Plan.StartCity >> Plan.NextFlight >> Plan.NextStart;
    return (File);
}

//--------------------------------------------------------------------------------------------
// Berechnet die Zahl der Passagiere auf dem Flug, wenn der Flug in diesem Augenblick erstellt/
// geändert wird. Kurzfristige Flüge haben weniger passagiere
//--------------------------------------------------------------------------------------------
void CFlugplanEintrag::CalcPassengers(SLONG PlayerNum, CPlane &qPlane) {
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    // Routen automatisch ergänzen:
    if (ObjectType == 1) {
        VonCity = Routen[ObjectId].VonCity;
        NachCity = Routen[ObjectId].NachCity;

        SLONG Dauer = Cities.CalcFlugdauer(VonCity, NachCity, qPlane.ptGeschwindigkeit);
        Landezeit = (Startzeit + Dauer) % 24;
        Landedate = Startdate;
        if (Landezeit < Startzeit) {
            Landedate++;
        }
    }

    if (ObjectType == 1) // Route
    {
        CRoute &qRoute = Routen[ObjectId];
        // if (PlayerNum == 0) AT_Log ("CalcPassengers for player %li for %s-%s", PlayerNum, (LPCTSTR)Cities[qRoute.VonCity].Name,
        // (LPCTSTR)Cities[qRoute.NachCity].Name);

        // Normale Passagiere: Aber nicht mehr kurz vor dem Start ändern:
        if (Startdate > Sim.Date || (Startdate == Sim.Date && Sim.GetHour() + 1 < Startzeit)) {
            SLONG c = 0;
            SLONG tmp = 0;
            SLONG Gewichte[4];
            SLONG Gesamtgewicht = 0;

            // Bonusregelgungen für Spieler anhand von Image, Rang, Preis, ...
            for (c = 0; c < 4; c++) {
                PLAYER &qPlayer = Sim.Players.Players[c];
                CRentRoute &qRentRoute = qPlayer.RentRouten.RentRouten[Routen(ObjectId)];

                if (qRentRoute.Rang != 0) {
                    // Ticketpreis ist Basis der Rechnung:
                    if (Ticketpreis != 0) {
                        Gewichte[c] = 10000 / Ticketpreis;
                    } else {
                        Gewichte[c] = 100000;
                    }

                    SLONG Costs = CalculateFlightCost(Routen[ObjectId].VonCity, Routen[ObjectId].NachCity, 800, 800, -1) * 3 / 180 * 2;

                    if (Ticketpreis > Costs * 3) {
                        Gewichte[c] = Gewichte[c] * 9 / 10;
                    }
                    if (Ticketpreis > Costs * 5) {
                        Gewichte[c] = Gewichte[c] * 9 / 10;
                    }
                    if (Ticketpreis > Costs * 6) {
                        Gewichte[c] = Gewichte[c] * 8 / 10;
                    }

                    if (Gewichte[c] == 0) {
                        Gewichte[c] = 1;
                    }

                    // Zuschläge für den Rang:
                    if (qRentRoute.Rang == 1) {
                        Gewichte[c] = Gewichte[c] * 150 / 100;
                    }
                    if (qRentRoute.Rang == 2) {
                        Gewichte[c] = Gewichte[c] * 120 / 100;
                    }

                    // Abschläge, wenn er selten fliegt:
                    if (qRentRoute.AvgFlown > 4) {
                        Gewichte[c] = Gewichte[c] * 10 / (10 + min(qRentRoute.AvgFlown - 3, 10));
                        if (Gewichte[c] < 1) {
                            Gewichte[c] = 1;
                        }
                    }

                    // 25% Bonus für den Computer
                    if (qPlayer.Owner == 1 && qPlayer.RobotUse(ROBOT_USE_ROUTE_BONUS)) {
                        Gewichte[c] += Gewichte[c] / 4;
                    }
                } else {
                    Gewichte[c] = 0;

                    if (c == PlayerNum) {
                        Gewichte[c] = 1;
                    }
                }

                // if (PlayerNum == 0) AT_Log ("Gewichte[%li]=%li", c, Gewichte[c]);

                Gesamtgewicht += Gewichte[c];
            }

            // if (PlayerNum == 0) AT_Log ("qRoute.Bedarf=%li", qRoute.Bedarf);

            // Wie viele wollen mitfliegen?
#ifdef _DEBUG
            tmp = qRoute.Bedarf * Gewichte[PlayerNum] / Gesamtgewicht;
#else
            if (Gesamtgewicht > 0) {
                tmp = qRoute.Bedarf * Gewichte[PlayerNum] / Gesamtgewicht;
            } else {
                tmp = 0;
            }
#endif
            // Wie viele können mitfliegen (plus Toleranz)?
            tmp = min(tmp, qPlane.MaxPassagiere + qPlane.MaxPassagiere / 2);
            // if (PlayerNum == 0) AT_Log ("tmp=%li (passagiere+toleranz)", tmp);

            // NetGenericAsync (24004+ObjectId+Sim.Date*100, tmp);

            // Fliegt er heute oder morgen? Dann machen wir Abstriche!
            if (HoursBefore > 48) {
                HoursBefore = 48;
            }
            if (HoursBefore < 48) {
                tmp = tmp * (HoursBefore + 48) / (48 + 48);
            }
            // if (PlayerNum == 0) AT_Log ("tmp=%li (nach tag)", tmp);

            // NetGenericAsync (30000+ObjectId+Sim.Date*100, HoursBefore);
            // NetGenericAsync (23003+ObjectId+Sim.Date*100, tmp);

            // Abschläge bei Wucherpreisen:
            SLONG Cost = CalculateFlightCost(qRoute.VonCity, qRoute.NachCity, 800, 800, -1) * 3 / 180 * 2;

            // NetGenericAsync (31001+ObjectId+Sim.Date*100, Cost);
            // NetGenericAsync (32002+ObjectId+Sim.Date*100, Ticketpreis);

            Cost *= 3;
            if (Ticketpreis > Cost && Cost > 10 && Ticketpreis > 0) {
                tmp = tmp * (Cost - 10) / Ticketpreis;
            }
            // if (PlayerNum == 0) AT_Log ("tmp=%li (nach kosten)", tmp);

            // NetGenericAsync (19009+ObjectId+Sim.Date*100, tmp);

            // NetGenericAsync (33003+ObjectId+Sim.Date*100, Startzeit);
            // NetGenericAsync (34004+ObjectId+Sim.Date*100, Landezeit);

            // Schlechte Zeiten? Das gibt Abstriche!
            if (Startzeit < 5 || Startzeit > 22) {
                tmp = tmp * 5 / 6;
            }
            if (Landezeit < 5 || Landezeit > 22) {
                tmp = tmp * 5 / 6;
            }
            // if (PlayerNum == 0) AT_Log ("tmp=%li (nach zeit)", tmp);

            // NetGenericAsync (22002+ObjectId+Sim.Date*100, tmp);

            // Image berücksichtigen:
            SLONG ImageTotal = 0;

            {
                CRentRoute &qRentRoute = qPlayer.RentRouten.RentRouten[Routen(ObjectId)];

                // log: AT_Log ("qRentRoute.Image=%li, qPlayer.Image=%li",qRentRoute.Image, qPlayer.Image);
                ImageTotal = qRentRoute.Image * 4;
                ImageTotal += qPlayer.Image;
                ImageTotal += 200;

                if (ImageTotal < 0) {
                    ImageTotal = 0;
                }
                if (ImageTotal > 1000) {
                    ImageTotal = 1000;
                }

                // NetGenericAsync (30003+ObjectId+Sim.Date*100, qRentRoute.Image);
                // NetGenericAsync (30004+ObjectId+Sim.Date*100, qPlayer.Image);
            }
            tmp = UWORD(tmp * (400 + ImageTotal) / 1100);
            // if (PlayerNum == 0) AT_Log ("tmp=%li (nach image)", tmp);

            if (qPlane.ptLaerm > 60) {
                tmp = UWORD(tmp * 1000 / 1100);
            }
            if (qPlane.ptLaerm > 80) {
                tmp = UWORD(tmp * 1000 / 1100);
            }
            if (qPlane.ptLaerm > 100) {
                tmp = UWORD(tmp * 1000 / 1100);
            }
            if (qPlane.ptLaerm > 110) {
                tmp = UWORD(tmp * 1000 / 1100);
            }
            // if (PlayerNum == 0) AT_Log ("tmp=%li (nach lärm)", tmp);

            // NetGenericAsync (21001+ObjectId+Sim.Date*100, tmp);

            // Wie viele können wirklich mitfliegen?
            if (Gesamtgewicht > 0) {
                tmp = min(tmp, qRoute.Bedarf * Gewichte[PlayerNum] / Gesamtgewicht);
            }
            // if (PlayerNum == 0) AT_Log ("tmp=%li (begrenzung nach gewicht)", tmp);

            tmp = min(tmp, qPlane.MaxPassagiere);
            // if (PlayerNum == 0) AT_Log ("tmp=%li (begrenzung nach Flugzeugkapazität)\n\n", tmp);

            Passagiere = static_cast<UWORD>(tmp);

            if (qPlayer.Presseerklaerung != 0) {
                Passagiere = 1 + (Passagiere % 3);
            }

            // NetGenericAsync (20000+ObjectId+Sim.Date*100, Passagiere);
        }

        // Erste Klasse Passagiere: Aber nicht mehr kurz vor dem Start ändern:
        if (Startdate > Sim.Date || (Startdate == Sim.Date && Sim.GetHour() + 1 < Startzeit)) {
            SLONG c = 0;
            SLONG tmp = 0;
            SLONG Gewichte[4];
            SLONG Gesamtgewicht = 0;

            // Bonusregelgungen für Spieler anhand von Image, Rang, Preis, ...
            for (c = 0; c < 4; c++) {
                PLAYER &qPlayer = Sim.Players.Players[c];
                CRentRoute &qRentRoute = qPlayer.RentRouten.RentRouten[Routen(ObjectId)];

                if (qRentRoute.Rang != 0) {
                    // Ticketpreis ist Basis der Rechnung:
                    if (TicketpreisFC != 0) {
                        Gewichte[c] = 10000 / TicketpreisFC;
                    } else {
                        Gewichte[c] = 100000;
                    }

                    SLONG Costs = CalculateFlightCost(Routen[ObjectId].VonCity, Routen[ObjectId].NachCity, 800, 800, -1) * 3 / 180 * 2;

                    if (TicketpreisFC > Costs * 3 * 3) {
                        Gewichte[c] = Gewichte[c] * 9 / 10;
                    }
                    if (TicketpreisFC > Costs * 3 * 5) {
                        Gewichte[c] = Gewichte[c] * 9 / 10;
                    }
                    if (TicketpreisFC > Costs * 3 * 6) {
                        Gewichte[c] = Gewichte[c] * 8 / 10;
                    }

                    if (Gewichte[c] == 0) {
                        Gewichte[c] = 1;
                    }

                    // Zuschläge für den Rang:
                    if (qRentRoute.Rang == 1) {
                        Gewichte[c] = Gewichte[c] * 170 / 100;
                    }
                    if (qRentRoute.Rang == 2) {
                        Gewichte[c] = Gewichte[c] * 120 / 100;
                    }

                    // Abschläge, wenn er selten fliegt:
                    if (qRentRoute.AvgFlown > 4) {
                        Gewichte[c] = Gewichte[c] * 10 / (10 + min(qRentRoute.AvgFlown - 3, 10));
                        if (Gewichte[c] < 1) {
                            Gewichte[c] = 1;
                        }
                    }

                    // 25% Bonus für den Computer
                    if (qPlayer.Owner == 1 && qPlayer.RobotUse(ROBOT_USE_ROUTE_BONUS)) {
                        Gewichte[c] += Gewichte[c] / 4;
                    }
                } else {
                    Gewichte[c] = 0;

                    if (c == PlayerNum) {
                        Gewichte[c] = 1;
                    }
                }

                Gesamtgewicht += Gewichte[c];
            }

            // Wie viele wollen mitfliegen?
#ifdef _DEBUG
            tmp = qRoute.Bedarf * Gewichte[PlayerNum] / Gesamtgewicht;
#else
            if (Gesamtgewicht > 0) {
                tmp = qRoute.Bedarf * Gewichte[PlayerNum] / Gesamtgewicht;
            } else {
                tmp = 0;
            }
#endif
            // Wie viele können mitfliegen (plus Toleranz)?
            tmp = min(tmp, qPlane.MaxPassagiereFC + qPlane.MaxPassagiereFC / 2);

            // Fliegt er heute oder morgen oder übermorgen oder überübermorgen? Dann machen wir Abstriche!
            if (HoursBefore > 72) {
                HoursBefore = 72;
            }
            if (HoursBefore < 72) {
                tmp = tmp * (HoursBefore + 48) / (72 + 48);
            }

            // Abschläge bei Wucherpreisen:
            SLONG Cost = CalculateFlightCost(qRoute.VonCity, qRoute.NachCity, 800, 800, -1) * 3 / 180 * 2;

            Cost *= 3;
            if (TicketpreisFC > Cost * 3 && Cost > 10 && TicketpreisFC > 0) {
                tmp = tmp * (Cost - 10) / TicketpreisFC;
            }

            // Schlechte Zeiten? Das gibt Abstriche!
            if (Startzeit < 5 || Startzeit > 22) {
                tmp = tmp * 5 / 6;
            }
            if (Landezeit < 5 || Landezeit > 22) {
                tmp = tmp * 5 / 6;
            }

            // Luxus ist ganz wichtig für die: (Summe e [0..16])
            SLONG LuxusSumme =
                qPlane.Sitze + qPlane.Essen + qPlane.Tabletts + qPlane.Deco + qPlane.Triebwerk + qPlane.Reifen + qPlane.Elektronik + qPlane.Sicherheit;

            // Sicherheit ist auch wichtig: (Summe e [0..18])
            LuxusSumme += static_cast<SLONG>((qPlayer.SecurityFlags & (1 << 10)) != 0) + static_cast<SLONG>((qPlayer.SecurityFlags & (1 << 11)) != 0);

            if (LuxusSumme < 6) {
                tmp = 0;
            }
            if (LuxusSumme >= 6 && LuxusSumme <= 18) {
                tmp = tmp * (LuxusSumme - 6) / 8;
            }

            // Image berücksichtigen:
            SLONG ImageTotal = 0;

            {
                CRentRoute &qRentRoute = qPlayer.RentRouten.RentRouten[Routen(ObjectId)];

                ImageTotal = qRentRoute.Image * 4;
                ImageTotal += qPlayer.Image;
                ImageTotal += 200;

                if (ImageTotal < 0) {
                    ImageTotal = 0;
                }
                if (ImageTotal > 1000) {
                    ImageTotal = 1000;
                }
            }
            tmp = UWORD(tmp * (400 + ImageTotal) / 1100);

            if (qPlane.ptLaerm > 40) {
                tmp = UWORD(tmp * 1000 / 1100);
            }
            if (qPlane.ptLaerm > 60) {
                tmp = UWORD(tmp * 1000 / 1100);
            }
            if (qPlane.ptLaerm > 80) {
                tmp = UWORD(tmp * 1000 / 1100);
            }
            if (qPlane.ptLaerm > 100) {
                tmp = UWORD(tmp * 1000 / 1100);
            }

            // Wie viele können wirklich mitfliegen?
            if (Gesamtgewicht > 0) {
                tmp = min(tmp, qRoute.Bedarf * Gewichte[PlayerNum] / Gesamtgewicht);
            }
            tmp = min(tmp, qPlane.MaxPassagiereFC);

            PassagiereFC = static_cast<UWORD>(tmp);

            if (qPlayer.Presseerklaerung != 0) {
                PassagiereFC = 1 + (PassagiereFC % 3);
            }
        }
    } else if (ObjectType == 2) {
        // Auftrag:
        Passagiere = static_cast<UWORD>(min(qPlane.MaxPassagiere, SLONG(qPlayer.Auftraege[ObjectId].Personen)));

        PassagiereFC = static_cast<UWORD>(qPlayer.Auftraege[ObjectId].Personen - Passagiere);
    }
}

//--------------------------------------------------------------------------------------------
// Flug wird gestartet, die Kosten und Einnahmen werden verbucht:
//--------------------------------------------------------------------------------------------
void CFlugplanEintrag::BookFlight(CPlane *Plane, SLONG PlayerNum) {
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    if (FlightBooked) {
        AT_Error("Schedule.cpp: %ld/%ld: %s: Tried to book flight twice:", Sim.Date, Sim.GetHour(), (LPCTSTR)qPlayer.AirlineX);
        return;
    }
    FlightBooked = TRUE;

    __int64 Saldo = 0;
    SLONG Einnahmen = 0;
    SLONG Kerosin = 0;
    SLONG KerosinGekauft = 0;
    SLONG KerosinAusTank = 0;
    DOUBLE KerosinGesamtQuali = 2;
    SLONG AusgabenKerosin = 0;
    SLONG AusgabenKerosinOhneTank = 0;
    SLONG AusgabenEssen = 0;
    CString CityString;

    // Hat angebliche Asynchronitäten berichtet, obwohl der Flugplan gleich war!
    // NetGenericAsync (90000+ObjectId+Sim.Date*100+PlayerNum*1000, Startzeit);
    // if (ObjectType==1)
    //   NetGenericAsync (80000+ObjectId+Sim.Date*100+PlayerNum*1000, qPlayer.RentRouten.RentRouten[(SLONG)Routen(ObjectId)].Image);

    if (qPlayer.Owner == 0 && ObjectType == 1) {
        qPlayer.DoRoutes++;
        if (qPlayer.DoRoutes == 1) {
            qPlayer.DoRoutes++;
        }
    }

    CalcPassengers(PlayerNum, *Plane);

    if (Plane->MaxPassagiere > 0) {
        if (Plane->Auslastung == 0) {
            Plane->Auslastung = Passagiere * 100 / Plane->MaxPassagiere;
        } else {
            Plane->Auslastung = (Plane->Auslastung + Passagiere * 100 / Plane->MaxPassagiere) / 2;
        }
    } else {
        Plane->Auslastung = 0;
    }

    if (Plane->MaxPassagiereFC > 0) {
        if (Plane->AuslastungFC == 0) {
            Plane->AuslastungFC = PassagiereFC * 100 / Plane->MaxPassagiereFC;
        } else {
            Plane->AuslastungFC = (Plane->AuslastungFC + PassagiereFC * 100 / Plane->MaxPassagiereFC) / 2;
        }
    } else {
        Plane->AuslastungFC = 0;
    }

    Plane->Kilometer += Cities.CalcDistance(VonCity, NachCity) / 1000;
    Plane->SummePassagiere += Passagiere;
    Plane->SummePassagiere += PassagiereFC;

    // Insgesamt geflogene Meilen:
    SLONG miles = Cities.CalcDistance(VonCity, NachCity) / 1609;
    qPlayer.NumMiles += miles;

    // Für den Statistik-Screen:
    if (ObjectType == 1 || ObjectType == 2) {
        qPlayer.Statistiken[STAT_PASSAGIERE].AddAtPastDay(Passagiere);
        qPlayer.Statistiken[STAT_PASSAGIERE].AddAtPastDay(PassagiereFC);
        qPlayer.Statistiken[STAT_FLUEGE].AddAtPastDay(1);

        if (SLONG(VonCity) == Sim.HomeAirportId || SLONG(NachCity) == Sim.HomeAirportId) {
            qPlayer.Statistiken[STAT_PASSAGIERE_HOME].AddAtPastDay(Passagiere);
            qPlayer.Statistiken[STAT_PASSAGIERE_HOME].AddAtPastDay(PassagiereFC);
        }
    } else if (ObjectType == 4) {
        qPlayer.Statistiken[STAT_FLUEGE].AddAtPastDay(1);
    }

    if (ObjectType == 1 || ObjectType == 2) {
        Sim.Players.Players[PlayerNum].NumPassengers += Passagiere + PassagiereFC;
    }

    if (ObjectType == 1) {
        // In beide Richtungen vermerken:
        qPlayer.RentRouten.RentRouten[Routen(ObjectId)].HeuteBefoerdert += Passagiere + PassagiereFC;

        for (SLONG c = Routen.AnzEntries() - 1; c >= 0; c--) {
            if ((Routen.IsInAlbum(c) != 0) && Routen[c].VonCity == Routen[ObjectId].NachCity && Routen[c].NachCity == Routen[ObjectId].VonCity) {
                qPlayer.RentRouten.RentRouten[c].HeuteBefoerdert += Passagiere + PassagiereFC;
                break;
            }
        }
    }

    if (ObjectType == 4) {
        if (qPlayer.Frachten[ObjectId].TonsLeft > 0) {
            SLONG tons = min(qPlayer.Frachten[ObjectId].TonsLeft, Plane->ptPassagiere / 10);
            qPlayer.Statistiken[STAT_TONS].AddAtPastDay(tons);
            qPlayer.NumFracht += tons;

            if (qPlayer.Frachten[ObjectId].Praemie == 0) {
                qPlayer.NumFrachtFree += tons;
            }

            qPlayer.Frachten[ObjectId].TonsLeft -= Plane->ptPassagiere / 10;

            if (qPlayer.Frachten[ObjectId].TonsLeft <= 0) {
                qPlayer.Frachten[ObjectId].TonsLeft = 0;
                Einnahmen = GetEinnahmen(PlayerNum, *Plane);
            }
        }
    } else {
        Einnahmen = GetEinnahmen(PlayerNum, *Plane);
    }
    if (ObjectType == 2 && Okay != 0) {
        Einnahmen = 0;
    }

    // Gesamtmenge an benötigten Kerosin
    Kerosin = CalculateFlightKerosin(VonCity, NachCity, Plane->ptVerbrauch, Plane->ptGeschwindigkeit);

    // Kerosin aus dem Vorrat:
    if (Sim.Players.Players[PlayerNum].TankOpen != 0) {
        KerosinAusTank = std::min(Sim.Players.Players[PlayerNum].TankInhalt, Kerosin);

        qPlayer.TankInhalt -= KerosinAusTank;

        if (qPlayer.TankInhalt == 0 && KerosinAusTank > 0 && (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_KEROSIN) != 0)) {
            Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_KEROSIN, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 3030), (LPCTSTR)Plane->Name));
        }

        auto kosten = SLONG(KerosinAusTank * qPlayer.TankPreis);
        Plane->Salden[0] -= kosten;
    }

    // Rest kaufen
    KerosinGekauft = Kerosin - KerosinAusTank;
    KerosinGesamtQuali = (KerosinAusTank * qPlayer.KerosinQuali + KerosinGekauft * 1) / Kerosin;

    AusgabenKerosin = KerosinGekauft * Sim.HoleKerosinPreis(1);
    AusgabenKerosinOhneTank = Kerosin * Sim.HoleKerosinPreis(1);

    qPlayer.Bilanz.KerosinGespart += (AusgabenKerosinOhneTank - AusgabenKerosin);

    if (ObjectType == 1 || ObjectType == 2) {
        AusgabenEssen += Passagiere * FoodCosts[Plane->Essen];
    }

    Plane->Salden[0] -= AusgabenKerosin;
    Plane->Salden[0] -= AusgabenEssen;
    Plane->Salden[0] += Einnahmen;

    Saldo = Einnahmen - AusgabenKerosin - AusgabenEssen;

    // Versteckter Bonus für Computerspieler:
    __int64 delta = min64(qPlayer.Bonus, abs64(Saldo) / 10);
    Saldo += delta;
    qPlayer.Bonus -= delta;
    Einnahmen = Saldo + AusgabenKerosin + AusgabenEssen;

    CityString = Cities[VonCity].Kuerzel + "-" + Cities[NachCity].Kuerzel;
    if (ObjectType == 1) {
        qPlayer.ChangeMoney(Einnahmen, 2030, CityString);
    } else if (ObjectType == 2) {
        qPlayer.ChangeMoney(Einnahmen, 2061, CityString);
    } else if (ObjectType == 3) {
        qPlayer.ChangeMoney(Einnahmen, 2061, CityString);
    } else if (ObjectType == 4) {
        qPlayer.ChangeMoney(Einnahmen, 2066, CityString);
    }
    qPlayer.ChangeMoney(-AusgabenKerosin, 2021, CityString);
    qPlayer.ChangeMoney(-AusgabenEssen, 2022, CityString);
    qPlayer.ChangeMoney(Saldo, 2100 - 1 + ObjectType, CityString);

    // Müssen wir das Flugzeug umrüsten (Sitze raus/rein wegen Fracht)?
    if (ObjectType != 3) {
        if (Plane->OhneSitze != static_cast<SLONG>(ObjectType == 4)) {
            Plane->OhneSitze = static_cast<BOOL>(ObjectType == 4);
            qPlayer.ChangeMoney(-15000, 2111, Plane->Name);
        }
    }

    // Bei Routen den Bedarf bei den Leuten entsprechend verringern und die Bekanntheit verbessern:
    if (ObjectType == 1) {
        CRentRoute &qRRoute = qPlayer.RentRouten.RentRouten[Routen(ObjectId)];

        qRRoute.LastFlown = 0;

        if (Sim.Players.Players[PlayerNum].HasFlownRoutes == 0) {
            SLONG c = 0;
            SLONG Anz = 0;

            for (c = Anz = 0; c < 4; c++) {
                if (Sim.Players.Players[c].HasFlownRoutes != 0) {
                    Anz++;
                }
            }

            Sim.Headlines.AddOverride(1,
                                      bprintf(StandardTexte.GetS(TOKEN_MISC, 2010 + Anz), (LPCTSTR)Sim.Players.Players[PlayerNum].AirlineX,
                                              (LPCTSTR)Sim.Players.Players[PlayerNum].NameX),
                                      GetIdFromString("1") + Anz + PlayerNum * 100, 60);

            Sim.Players.Players[PlayerNum].HasFlownRoutes = TRUE;
        }

        // Auslastung der Route aktualisieren:
        if (Plane->MaxPassagiere > 0) {
            if (qRRoute.Auslastung == 0) {
                qRRoute.Auslastung = Passagiere * 100 / Plane->MaxPassagiere;
            } else {
                qRRoute.Auslastung = (qRRoute.Auslastung * 3 + (Passagiere * 100 / Plane->MaxPassagiere)) / 4;
            }
        } else {
            qRRoute.Auslastung = 0;
        }

        if (Plane->MaxPassagiereFC > 0) {
            if (qRRoute.AuslastungFC == 0) {
                qRRoute.AuslastungFC = PassagiereFC * 100 / Plane->MaxPassagiereFC;
            } else {
                qRRoute.AuslastungFC = (qRRoute.AuslastungFC * 3 + (PassagiereFC * 100 / Plane->MaxPassagiereFC)) / 4;
            }
        } else {
            qRRoute.AuslastungFC = 0;
        }

        Routen[ObjectId].Bedarf -= Passagiere;
        if (Routen[ObjectId].Bedarf < 0) {
            Routen[ObjectId].Bedarf = 0;
        }

        if (qRRoute.Image < 100) {
            qRRoute.Image++;
        }
    }
    // Bei Aufträgen, die Prämie verbuchen; Aufträge als erledigt markieren
    else if (ObjectType == 2) {
        if (Okay == 0 || Okay == 1) {
            qPlayer.Auftraege[ObjectId].InPlan = -1; // Auftrag erledigt
        } else {
            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2313), Plane->Name.c_str()));
        }

        // Add-On Mission 9
        if (Sim.Difficulty == DIFF_ADDON09) {
            if (qPlayer.Owner == 1 && qPlayer.RobotUse(ROBOT_UHRIG_FLIGHTS_AUTO)) {
                if (qPlayer.NumOrderFlightsToday < 5 - static_cast<SLONG>(((Sim.Date + qPlayer.PlayerNum) % 5) == 1) -
                                                       static_cast<SLONG>(((Sim.Date + qPlayer.PlayerNum) % 11) == 2) -
                                                       static_cast<SLONG>(((Sim.Date + qPlayer.PlayerNum) % 7) == 0) - ((Sim.Date + qPlayer.PlayerNum) % 2)) {
                    qPlayer.NumOrderFlights++;
                    qPlayer.NumOrderFlightsToday++;
                }
            } else if (qPlayer.Auftraege[ObjectId].bUhrigFlight != 0) {
                qPlayer.NumOrderFlights++;
                qPlayer.NumOrderFlightsToday++;
            }
        }
    }

    // Auswirkungen auf's Image verbuchen
    if (ObjectType == 1 || ObjectType == 2) {
        SLONG pn = qPlayer.PlayerNum;
        if (Sim.Players.Players[qPlayer.PlayerNum].WerbeBroschuere != -1) {
            pn = Sim.Players.Players[qPlayer.PlayerNum].WerbeBroschuere;
            if (Sim.Players.Players[qPlayer.PlayerNum].Owner == 0) {
                Sim.Players.Players[qPlayer.PlayerNum].Messages.AddMessage(
                    BERATERTYP_GIRL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2357), Plane->Name.c_str(), Sim.Players.Players[pn].AirlineX.c_str()));
            }
        }

        PLAYER &qPlayerX = Sim.Players.Players[pn];

        if (qPlayer.Owner != 1 || !qPlayer.RobotUse(ROBOT_USE_FAKE_PERSONAL)) {
            // ObjectId wirkt als deterministisches Random
            // log: AT_Log ("Player[%li].Image now (deter) = %li", PlayerNum, qPlayer.Image);
            if (Plane->PersonalQuality < 50 && (ObjectId) % 10 == 0) {
                qPlayerX.Image--;
            }
            if (Plane->PersonalQuality > 90 && (ObjectId) % 10 == 0) {
                qPlayerX.Image++;
            }
            // log: AT_Log ("Player[%li].Image now (deter) = %li", PlayerNum, qPlayer.Image);

            if (ObjectType == 1) { // Auswirkung auf Routenimage
                if (qPlayer.RentRouten.RentRouten[Routen(ObjectId)].Image > 0) {
                    qPlayer.RentRouten.RentRouten[Routen(ObjectId)].Image--;
                }
            }

            Limit(SLONG(-1000), qPlayerX.Image, SLONG(1000));
        }

        // Zustand des Flugzeugs:
        SLONG Add = 0;
        if (Plane->Zustand < 20) {
            Add -= 50;
        }
        if (Plane->Zustand < 60) {
            Add -= 10;
        }
        if (Plane->Zustand > 98) {
            Add += 10;
        }

        // Ausstattung des Flugzeuges:
        if (Plane->Sitze == 0) {
            Add--;
        }
        if (Plane->Essen == 0) {
            Add--;
        }
        if (Plane->Tabletts == 0) {
            Add--;
        }
        if (Plane->Deco == 0) {
            Add--;
        }
        if (Plane->Sitze == 2) {
            Add += 2;
        }
        if (Plane->Essen == 2) {
            Add += 2;
        }
        if (Plane->Tabletts == 2) {
            Add += 2;
        }
        if (Plane->Deco == 2) {
            Add += 2;
        }

        // Wieviel Personal haben wir an Bord?

        if (qPlayer.Owner != 1 || !qPlayer.RobotUse(ROBOT_USE_FAKE_PERSONAL)) { // Nur für reale Spieler:
            Add += (3 * Plane->AnzBegleiter / Plane->ptAnzBegleiter);
        }
        // Add+=(3*Plane->AnzBegleiter/PlaneTypes[Plane->TypeId].AnzBegleiter);

        // Preispolitik bewerten:
        SLONG TooExpensive = 0;
        if (ObjectType == 1) {
            SLONG Costs1 = Ticketpreis;
            SLONG Costs2 = CalculateFlightCost(VonCity, NachCity, 800, 800, -1) * 3 / 180 * 2 * 3;

            if (Costs1 < Costs2 / 2) {
                Add += 10;
            }
            if (Costs1 > Costs2) {
                Add -= 10;
            }
            if (Costs1 > Costs2 + Costs2 / 2) {
                Add -= 10;
            }
            if (Costs1 > Costs2 + Costs2) {
                Add -= 10;
            }

            if (TooExpensive > 100) {
                TooExpensive = 100;
            }
        }

        // NetGenericAsync (10000+ObjectId+Sim.Date*100, Add);

        qPlayerX.Image += Add / 10;
        if (ObjectType == 1 && SLONG(qPlayer.RentRouten.RentRouten[Routen(ObjectId)].Image) + Add / 10 > 0) {
            qPlayer.RentRouten.RentRouten[Routen(ObjectId)].Image += Add / 10;

            Limit(static_cast<UBYTE>(0), qPlayer.RentRouten.RentRouten[Routen(ObjectId)].Image, static_cast<UBYTE>(100));
        }
        Limit(SLONG(-1000), qPlayerX.Image, SLONG(1000));
    }

    if (ObjectType == 1) {
        AT_Log("Schedule.cpp: %s: %s $ (+%s $)  (%ld miles, %u * %s $ + %u * %s $ from tickets)", (LPCTSTR)qPlayer.AirlineX, (LPCTSTR)Insert1000erDots64(Saldo),
               (LPCTSTR)Insert1000erDots64(delta), miles, Passagiere, (LPCTSTR)Insert1000erDots(Ticketpreis), PassagiereFC,
               (LPCTSTR)Insert1000erDots(TicketpreisFC));
    } else {
        AT_Log("Schedule.cpp: %s: %s $ (+%s $)  (%ld miles, %u + %u passengers)", (LPCTSTR)qPlayer.AirlineX, (LPCTSTR)Insert1000erDots64(Saldo),
               (LPCTSTR)Insert1000erDots64(delta), miles, Passagiere, PassagiereFC);
    }

    // Flugzeugabnutzung verbuchen:
    auto faktorDistanz = (1 + Cities.CalcDistance(VonCity, NachCity) * 10 / 40040174);
    auto faktorBaujahr = (GetCurrentYear() - Plane->Baujahr);
    auto faktorKerosin = 1;
    if (KerosinGesamtQuali > 1.0) {
        faktorKerosin = static_cast<int>(3.0 * (KerosinGesamtQuali - 1.0) * (KerosinGesamtQuali - 1.0));
    }
    auto ZustandAlt = Plane->Zustand;
    Plane->Zustand = static_cast<UBYTE>(Plane->Zustand - faktorDistanz * faktorBaujahr * faktorKerosin / 15);
    AT_Log("Schedule.cpp: %s: Plane %s (%s): Zustand: %u => %u (worst = %u), faktorDistanz = %f, faktorBaujahr = %f, faktorKerosin = %f",
           (LPCTSTR)qPlayer.AirlineX, (LPCTSTR)Plane->Name, (Plane->TypeId == -1) ? "Designer" : (LPCTSTR)PlaneTypes[Plane->TypeId].Name, ZustandAlt,
           Plane->Zustand, Plane->WorstZustand, faktorDistanz, faktorBaujahr, faktorKerosin);
    if (Plane->Zustand > 200) {
        Plane->Zustand = 0;
    }
}

//--------------------------------------------------------------------------------------------
// Flug wurde geändert:
//--------------------------------------------------------------------------------------------
void CFlugplanEintrag::FlightChanged() { HoursBefore = UBYTE((Startdate - Sim.Date) * 24 + (Startzeit - Sim.GetHour())); }

//--------------------------------------------------------------------------------------------
// Sagt, wieviel der Flug bringen wird:
//--------------------------------------------------------------------------------------------
SLONG CFlugplanEintrag::GetEinnahmen(SLONG PlayerNum, const CPlane &qPlane) const {
    switch (ObjectType) {
    // Route:
    case 1:
        return (Ticketpreis * Passagiere + TicketpreisFC * PassagiereFC);
        break;

        // Auftrag:
    case 2:
        return (Sim.Players.Players[PlayerNum].Auftraege[ObjectId].Praemie);
        break;

        // Leerflug:
    case 3:
        return (qPlane.ptPassagiere * Cities.CalcDistance(VonCity, NachCity) / 1000 / 40);
        break;

        // Frachtauftrag:
    case 4:
        return (Sim.Players.Players[PlayerNum].Frachten[ObjectId].Praemie);
        break;

    default: // Eigentlich unmöglich:
        return (0);
    }

    return (0);
}

//--------------------------------------------------------------------------------------------
// Sagt, wieviel der Flug kosten wird:
//--------------------------------------------------------------------------------------------
SLONG CFlugplanEintrag::GetAusgaben(SLONG PlayerNum, const CPlane &qPlane) const {
    return (CalculateFlightCost(VonCity, NachCity, qPlane.ptVerbrauch, qPlane.ptGeschwindigkeit, PlayerNum));
}

//--------------------------------------------------------------------------------------------
// Speichert ein CGate-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CGate &Gate) {
    File << Gate.Nummer << Gate.Miete;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein CGate-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CGate &Gate) {
    File >> Gate.Nummer >> Gate.Miete;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Speichert ein CGates-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CGates &Gates) {
    File << Gates.Gates;
    File << Gates.Auslastung;
    File << Gates.NumRented;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein CGates-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CGates &Gates) {
    File >> Gates.Gates;
    File >> Gates.Auslastung;
    File >> Gates.NumRented;

    return (File);
}
