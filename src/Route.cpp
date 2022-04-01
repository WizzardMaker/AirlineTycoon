//============================================================================================
// Route.cpp : Routinen zum verwalten der Flugrouten (CRoute, CRouten)
//============================================================================================
#include "StdAfx.h"

SLONG ReadLine(BUFFER_V<UBYTE> &Buffer, SLONG BufferStart, char *Line, SLONG LineLength);

// Daten des aktuellen Savegames beim laden:
extern SLONG SaveVersion;
extern SLONG SaveVersionSub;

//============================================================================================
// CRoute::
//============================================================================================
// So viele fliegen (Potentiell) hier
//============================================================================================
SLONG CRoute::AnzPassagiere() const {
    SLONG DayFaktor = 0;

    if (Sim.Date < 3) {
        DayFaktor = 1000;
    } else if (Sim.Date < 9) {
        DayFaktor = 900;
    } else if (Sim.Date < 29) {
        DayFaktor = 750;
    } else {
        DayFaktor = 650;
    }

    return (static_cast<SLONG>(sqrt(Cities[VonCity].Einwohner * double(Cities[NachCity].Einwohner)) * Faktor / DayFaktor / 2 * 3 / 4));
}

BOOL CRoute::operator>(const CRoute &p) const { return static_cast<BOOL>(Cities[VonCity].Name > Cities[p.VonCity].Name); }
BOOL CRoute::operator<(const CRoute &p) const { return static_cast<BOOL>(Cities[VonCity].Name < Cities[p.VonCity].Name); }

//--------------------------------------------------------------------------------------------
// Eine CRoute-Objekt speichern:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CRoute &r) {
    File << r.Ebene << r.VonCity << r.NachCity;
    File << r.Miete << r.Faktor << r.Bedarf;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Eine CRoute-Objekt laden:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CRoute &r) {
    File >> r.Ebene >> r.VonCity >> r.NachCity;
    File >> r.Miete >> r.Faktor >> r.Bedarf;

    return (File);
}

//============================================================================================
// CRouten::
//============================================================================================
// Konstruktor:
//============================================================================================
CRouten::CRouten(const CString & /*TabFilename*/) : ALBUM_V<CRoute>("Routen") {
    // ReInit (TabFilename);
    DebugBreak();
}

//--------------------------------------------------------------------------------------------
// Dateien neu laden:
//--------------------------------------------------------------------------------------------
void CRouten::ReInit(const CString &TabFilename, bool bNoDoublettes) {
    // CStdioFile    Tab;
    BUFFER_V<char> Line(300);

    // Load Table header:
    auto FileData = LoadCompleteFile(FullFilename(TabFilename, ExcelPath));
    SLONG FileP = 0;

    // Die erste Zeile einlesen
    FileP = ReadLine(FileData, FileP, Line.getData(), 300);

    ReSize(0);
    ReSize(MAX_ROUTES);

    while (true) {
        if (FileP >= FileData.AnzEntries()) {
            break;
        }
        FileP = ReadLine(FileData, FileP, Line.getData(), 300);

        // if (!Tab.ReadString (Line, 300)) break;
        TeakStrRemoveEndingCodes(Line.getData(), "\xd\xa\x1a\r");

        SLONG HelperEbene = atoi(strtok(Line.getData(), TabSeparator));
        CString Helper1 = strtok(nullptr, TabSeparator);
        CString Helper2 = strtok(nullptr, TabSeparator);
        ULONG VonCity = Cities.GetIdFromName(const_cast<char *>((LPCTSTR)KorrigiereUmlaute(Helper1)));
        ULONG NachCity = Cities.GetIdFromName(const_cast<char *>((LPCTSTR)KorrigiereUmlaute(Helper2)));

        // Looking for doubles (may be turned off for compatibility)
        bool skip = false;
        if (bNoDoublettes) {
            for (SLONG c = 0; c < AnzEntries(); c++) {
                if ((IsInAlbum(c) != 0) && (*this)[c].VonCity == VonCity && (*this)[c].NachCity == NachCity) {
                    skip = true;
                }
            }
        }

        if (!skip) {
            CRoute routeHin;
            routeHin.Ebene = HelperEbene;
            routeHin.VonCity = VonCity;
            routeHin.NachCity = NachCity;
            routeHin.Miete = atol(strtok(nullptr, TabSeparator));
            routeHin.Faktor = atof(strtok(nullptr, TabSeparator));
            routeHin.Bedarf = 0;
            routeHin.bNewInDeluxe = static_cast<BOOL>(Cities[VonCity].bNewInAddOn == 2 || Cities[NachCity].bNewInAddOn == 2);

            CRoute routeHer = routeHin;
            std::swap(routeHer.VonCity, routeHer.NachCity);

            (*this) += routeHin;
            (*this) += routeHer;
        }
    }

    this->Sort();
}

//--------------------------------------------------------------------------------------------
// Dateien neu laden:
//--------------------------------------------------------------------------------------------
void CRouten::ReInitExtend(const CString &TabFilename) {
    // CStdioFile    Tab;
    BUFFER_V<char> Line(300);
    long linenumber = 0;

    // Load Table header:
    auto FileData = LoadCompleteFile(FullFilename(TabFilename, ExcelPath));
    SLONG FileP = 0;

    // Die erste Zeile einlesen
    FileP = ReadLine(FileData, FileP, Line.getData(), 300);

    ReSize(MAX_ROUTES);
    auto NumUsed = GetNumUsed();

    while (true) {
        if (FileP >= FileData.AnzEntries()) {
            break;
        }
        FileP = ReadLine(FileData, FileP, Line.getData(), 300);

        if (linenumber < NumUsed / 2) {
            linenumber++;
            continue;
        }
        linenumber++;

        TeakStrRemoveEndingCodes(Line.getData(), "\xd\xa\x1a\r");

        SLONG HelperEbene = atoi(strtok(Line.getData(), TabSeparator));
        CString Helper1 = strtok(nullptr, TabSeparator);
        CString Helper2 = strtok(nullptr, TabSeparator);
        ULONG VonCity = Cities.GetIdFromName(const_cast<char *>((LPCTSTR)KorrigiereUmlaute(Helper1)));
        ULONG NachCity = Cities.GetIdFromName(const_cast<char *>((LPCTSTR)KorrigiereUmlaute(Helper2)));

        CRoute routeHin;
        routeHin.Ebene = HelperEbene;
        routeHin.VonCity = VonCity;
        routeHin.NachCity = NachCity;
        routeHin.Miete = atol(strtok(nullptr, TabSeparator));
        routeHin.Faktor = atof(strtok(nullptr, TabSeparator));
        routeHin.Bedarf = 0;
        routeHin.bNewInDeluxe = static_cast<BOOL>(Cities[VonCity].bNewInAddOn == 2 || Cities[NachCity].bNewInAddOn == 2);

        CRoute routeHer = routeHin;
        std::swap(routeHer.VonCity, routeHer.NachCity);

        (*this) += routeHin;
        (*this) += routeHer;
    }

    this->Sort();
}

//--------------------------------------------------------------------------------------------
// Wie viele Leute warten auf einen Flug?:
//--------------------------------------------------------------------------------------------
void CRouten::NewDay() {
    SLONG c = 0;

    if (Sim.Date == 0) {
        for (c = 0; c < Routen.AnzEntries(); c++) {
            if (IsInAlbum(c) != 0) {
                Routen[c].Bedarf = SLONG(Routen[c].AnzPassagiere() * 4.27);
            }
        }
    } else {
        for (c = 0; c < Routen.AnzEntries(); c++) {
            if (IsInAlbum(c) != 0) {
                Routen[c].Bedarf = (Routen[c].Bedarf * 6 + (SLONG(Routen[c].AnzPassagiere() * 4.27))) / 7;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Speichert ein CRouten Datum:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CRouten &r) {
    File << *((const ALBUM_V<CRoute> *)&r);

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein CRouten Datum:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CRouten &r) {
    File >> *((ALBUM_V<CRoute> *)&r);

    return (File);
}

//============================================================================================
// CRentRoute::
//============================================================================================
//--------------------------------------------------------------------------------------------
// Gibt zurück, wieviele Routen der Spieler besitzt:
//--------------------------------------------------------------------------------------------
SLONG CRentRouten::GetNumUsed() {
    SLONG c = 0;
    SLONG Anz = 0;

    for (c = 0; c < SLONG(Routen.AnzEntries()); c++) {
        if ((Routen.IsInAlbum(c) != 0) && RentRouten[c].Rang != 0) {
            Anz++;
        }
    }

    return (Anz);
}

//--------------------------------------------------------------------------------------------
// Speichert ein RentRouten-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const CRentRoute &r) {
    File << r.Rang << r.LastFlown << r.AvgFlown;
    File << r.Auslastung << r.Image << r.Miete;
    File << r.Ticketpreis << r.TicketpreisFC << r.TageMitVerlust << r.TageMitGering;
    File << r.RoutenAuslastung << r.HeuteBefoerdert << r.AuslastungFC;
    File << r.WocheBefoerdert;

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein RentRouten-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, CRentRoute &r) {
    File >> r.Rang >> r.LastFlown >> r.AvgFlown;
    File >> r.Auslastung >> r.Image >> r.Miete;
    File >> r.Ticketpreis >> r.TicketpreisFC >> r.TageMitVerlust >> r.TageMitGering;
    File >> r.RoutenAuslastung >> r.HeuteBefoerdert >> r.AuslastungFC;
    File >> r.WocheBefoerdert;

    return (File);
}
