//============================================================================================
// Checkup.h : Prüft diverse Sachen der Rechnerkonfiguration und liefert die Basis für
//             Registry-Zugriffe                                                          [TH]
// Hinweise  : 1. Die CD-Geschwindigkeit kann noch nicht zuverlässig ermittelt werden.
//             2. Auf einem 486er kann die Taktgeschwindigket noch nicht ermittelt
//                werden. Daher wird einfach 33Mhz angenommen. Das Problem wird sich
//                durch Tod des Antragstellers erledigen...
//============================================================================================
// Link: "Checkup.h"
//============================================================================================

//--------------------------------------------------------------------------------------------
// Gibt Zugriff auf die Registry und bedient sich fast wie eine Fileklasse; speichert die
// Daten nicht digital, sondern auf ASCII-Basis (und somit editierbar)
//--------------------------------------------------------------------------------------------
#define ReadRegistryKey(vp) ReadRegistryKeyEx(vp, #vp)
#define WriteRegistryKey(vp) WriteRegistryKeyEx(vp, #vp)
#define ReadRegistryKey_b(vp) ReadRegistryKeyEx_b(vp, #vp)
#define WriteRegistryKey_b(vp) WriteRegistryKeyEx_b(vp, #vp)
#define ReadRegistryKey_l(vp) ReadRegistryKeyEx_l(vp, #vp)
#define WriteRegistryKey_l(vp) WriteRegistryKeyEx_l(vp, #vp)
#define ReadRegistryKey_u(vp) ReadRegistryKeyEx_u(vp, #vp)
#define WriteRegistryKey_u(vp) WriteRegistryKeyEx_u(vp, #vp)

#ifdef WIN32
#define USE_REG_MIGRATION 1
#else
#define USE_REG_MIGRATION 0
#endif // WIN32

class CRegistryAccess {
  private:
    struct json_t *settingsJSON{nullptr};
#if USE_REG_MIGRATION
    HKEY hKey;
#endif

public:
    CRegistryAccess() = default;
    CRegistryAccess(const CString &RegistryPath);
    ~CRegistryAccess();
    bool Open(const CString &RegistryPath);
    void Close(void);

    bool IsOpen(void);

    bool ReadRegistryKeyEx(char *Text, const CString &EntryName);
    bool ReadRegistryKeyEx_b(BOOL &Bool, const CString &EntryName);
    bool ReadRegistryKeyEx_l(SLONG &Long, const CString &EntryName);
    bool ReadRegistryKeyEx_u(ULONG &Long, const CString &EntryName);
    bool WriteRegistryKeyEx(const char *Text, const CString &EntryName);
    bool WriteRegistryKeyEx_b(const BOOL &Bool, const CString &EntryName);
    bool WriteRegistryKeyEx_l(const SLONG &Long, const CString &EntryName);
    bool WriteRegistryKeyEx_u(const ULONG &Long, const CString &EntryName);
};
