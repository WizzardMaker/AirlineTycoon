//============================================================================================
// Checkup.cpp : Prüft diverse Sachen der Rechnerkonfiguration [TH]
//============================================================================================
// Link: "Checkup.h"
//============================================================================================
//  Infos:
// ------------------------------------------------------------------------------------------
//  CSystemCheckup fragt diverse Sachen vom System ab. Dazu erzeugt man eine Instanz und
//  übergibt als Flags, welche Sachen abgefragt werden sollen (oder CHECKUP_ALL) und wie die
//  Registry mitspielt.
//
//  Die Daten können ohne Bezug auf die Registry einfach ermittelt werden (default) oder
//  können mittels CHECKUP_WRITE nach dem ermittelt in die Registry geschrieben werden.
//  Bei übergabe des Parameters CHECKUP_READ werden die Werte nicht ermittelt, sondern nur
//  aus der Registry gelesen. Ist eine bestimmte Eigenschaft in den Registry-Infos nicht
//  vorhanden, so wird diese noch ermittelt.
//
//  Läßt man den CDFile-Parameter leer, so wird der CD-Speed Check ggf. übersprungen.
//============================================================================================
#include "StdAfx.h"

#include "Checkup.h"

#include <jansson.h>

#define AT_Error(...) Hdu.HercPrintfMsg(SDL_LOG_PRIORITY_ERROR, "Reg", __VA_ARGS__)
#define AT_Log(...) AT_Log_I("Reg", __VA_ARGS__)

CString settingsPath;

void PrepareSettingsPath() { settingsPath = AppPath + CString("AT.json"); }

//--------------------------------------------------------------------------------------------
// CRegistryAccess::
//--------------------------------------------------------------------------------------------
// Konstruktor+Open:
//--------------------------------------------------------------------------------------------
CRegistryAccess::CRegistryAccess(const CString &RegistryPath) {
    PrepareSettingsPath();
    settingsJSON = nullptr;
    Open(RegistryPath);
}

//--------------------------------------------------------------------------------------------
// Destruktor:
//--------------------------------------------------------------------------------------------
CRegistryAccess::~CRegistryAccess() { Close(); }

//--------------------------------------------------------------------------------------------
// Öffnet den Zugriff auf einen Bereich der Registry; Gibt FALSE im Fehlerfall zurück:
//--------------------------------------------------------------------------------------------
bool CRegistryAccess::Open(const CString &RegistryPath) {
    if (IsOpen()) {
        Close();
    }

    json_error_t error;

    settingsJSON = json_load_file(settingsPath, JSON_INDENT(3), &error);
    if (settingsJSON == nullptr) {
        AT_Error("Encountered error during settings load: %s", error.text);
        settingsJSON = json_object();
    }
    if (!IsOpen()) {
        return false;
    }

#if USE_REG_MIGRATION
    DWORD dwDisposition = 0;

    if (FAILED(RegCreateKeyEx(HKEY_CURRENT_USER, RegistryPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition))) {
        return false; // Geht nicht
    }
#endif

    return true;
}

//--------------------------------------------------------------------------------------------
// Alten Zugriff schließen:
//--------------------------------------------------------------------------------------------
void CRegistryAccess::Close() {
    if (IsOpen()) {
        json_decref(settingsJSON);
#if USE_REG_MIGRATION
        RegCloseKey(hKey);
#endif
        settingsJSON = nullptr;
    }
}

//--------------------------------------------------------------------------------------------
// Gibt TRUE zurück, wenn z.Zt ein Registry-Zugriff offen ist:
//--------------------------------------------------------------------------------------------
bool CRegistryAccess::IsOpen() { return (settingsJSON != nullptr); }

void CRegistryAccess::WriteFile() {
    if (!IsOpen()) {
        return;
    }
    json_dump_file(settingsJSON, settingsPath, JSON_INDENT(3));
}

//--------------------------------------------------------------------------------------------
// Schreibt einen Registry-Key; Gibt FALSE im Fehlerfall zurück, sonst TRUE
//--------------------------------------------------------------------------------------------
bool CRegistryAccess::WriteRegistryKeyEx(const char *Text, const CString &EntryName) {
    if (settingsJSON == nullptr) {
        return false;
    }

    return (json_object_set_new(settingsJSON, EntryName, json_string(Text)) > 0);
}

bool CRegistryAccess::WriteRegistryKeyEx_b(const BOOL &Bool, const CString &EntryName) {
    if (settingsJSON == nullptr) {
        return false;
    }

    return (json_object_set_new(settingsJSON, EntryName, json_boolean(Bool)) > 0);
}

bool CRegistryAccess::WriteRegistryKeyEx_l(const SLONG &Long, const CString &EntryName) {
    if (settingsJSON == nullptr) {
        return false;
    }
    return (json_object_set_new(settingsJSON, EntryName, json_integer(Long)) > 0);
}

bool CRegistryAccess::WriteRegistryKeyEx_u(const ULONG &Long, const CString &EntryName) {
    if (settingsJSON == nullptr) {
        return false;
    }

    return (json_object_set_new(settingsJSON, EntryName, json_integer(Long)) > 0);
}

//--------------------------------------------------------------------------------------------
// Ließt einen Registry-Key; Gibt FALSE im Fehlerfall zurück, sonst TRUE
//--------------------------------------------------------------------------------------------
bool CRegistryAccess::ReadRegistryKeyEx(char *Text, const CString &EntryName) {
    if (settingsJSON == nullptr) {
        return false;
    }

    json_t *Entry = json_object_get(settingsJSON, EntryName);
    if (Entry == nullptr) {
#if USE_REG_MIGRATION
        unsigned long TempSize = 500;
        HRESULT res = RegQueryValueEx(hKey, EntryName, NULL, NULL, (UBYTE *)Text, &TempSize);
        if (res != S_OK) {
            return false;
        }
#endif

        this->WriteRegistryKeyEx(Text, EntryName);
#if USE_REG_MIGRATION
        return true;
#else
        return false;
#endif
    } else if (!json_is_string(Entry)) {
        return false;
    }
    return (snprintf(Text, json_string_length(Entry) + 1, "%s", json_string_value(Entry)) >= 0);
    // unsigned long TempSize = 500;
    // return (ERROR_SUCCESS == RegQueryValueEx(settingsJSON, EntryName, NULL, NULL, (UBYTE *)Text, &TempSize));
}

bool CRegistryAccess::ReadRegistryKeyEx_b(BOOL &Bool, const CString &EntryName) {
    if (settingsJSON == nullptr) {
        return (0) != 0;
    }

    json_t *Entry = json_object_get(settingsJSON, EntryName);
    if (Entry == nullptr) {
        char *Temp = new char[500];
        bool rc = ReadRegistryKeyEx(Temp, EntryName);

        if (rc)
            Bool = (BOOL)atoi(Temp);

        delete[] Temp;

        this->WriteRegistryKeyEx_b(Bool, EntryName);

        return (false);
    } else if (!json_is_boolean(Entry)) {
        return false;
    }

    Bool = json_boolean_value(Entry);
    return true;
}

bool CRegistryAccess::ReadRegistryKeyEx_l(SLONG &Long, const CString &EntryName) {
    if (settingsJSON == nullptr) {
        return (0) != 0;
    }

    json_t *Entry = json_object_get(settingsJSON, EntryName);
    if (Entry == nullptr) {
        char *Temp = new char[500];
        bool rc = ReadRegistryKeyEx(Temp, EntryName);
        if (rc)
            Long = atoi(Temp);

        delete[] Temp;

        this->WriteRegistryKeyEx_l(Long, EntryName);
        return (rc);
    } else if (!json_is_integer(Entry)) {
        return false;
    }

    Long = json_integer_value(Entry);
    return true;
}

bool CRegistryAccess::ReadRegistryKeyEx_u(ULONG &Long, const CString &EntryName) {
    if (settingsJSON == nullptr) {
        return (0) != 0;
    }

    json_t *Entry = json_object_get(settingsJSON, EntryName);
    if (Entry == nullptr) {
        char *Temp = new char[500];
        bool rc = ReadRegistryKeyEx(Temp, EntryName);

        if (rc)
            Long = strtoul(Temp, nullptr, 10);
        delete[] Temp;

        this->WriteRegistryKeyEx_u(Long, EntryName);
        return (rc);
    } else if (!json_is_integer(Entry)) {
        return false;
    }

    Long = json_integer_value(Entry);
    return true;
}
