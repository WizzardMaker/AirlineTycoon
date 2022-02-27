//============================================================================================
// Protect.cpp : Routinen zur Kopierschutz:
//============================================================================================
#include "StdAfx.h"
#include "SecurityManager.h"
#include "cd_prot.h"
#include "fillfile.h"
#include <chrono>

SLONG ReadLine(BUFFER_V<UBYTE> &Buffer, SLONG BufferStart, char *Line, SLONG LineLength);

BOOL CreditsFilesAreMissing = FALSE;

//--------------------------------------------------------------------------------------------
// Sucht aus, welchen String wir den heute mal so abfagen:
//--------------------------------------------------------------------------------------------
void GetProtectionString(const CString &TabFilename, SLONG *pPageAndWord, CString *pRightWord);

void GetProtectionString(const CString &TabFilename, SLONG *pPageAndWord, CString *pRightWord) {
    BUFFER_V<char> Line(300);
    SLONG FileP = 0;
    SLONG Num = 0;
    SLONG Count = 0;

    // Load Table header:
    auto FileData = *(LoadCompleteFile(FullFilename(TabFilename, ExcelPath)));

    // Die erste Zeile einlesen
    FileP = 0;
    FileP = ReadLine(FileData, FileP, Line.getData(), 300);

    while (true) {
        if (FileP >= FileData.AnzEntries()) {
            break;
        }
        FileP = ReadLine(FileData, FileP, Line.getData(), 300);

        TeakStrRemoveEndingCodes(Line.getData(), "\xd\xa\x1a\r");

        Num = atoi(strtok(Line.getData(), TabSeparator));

        if (Num >= 1000000) {
            Count++;
        }
    }

    TEAKRAND r;

    auto t = std::chrono::system_clock::now().time_since_epoch();
    long days = std::chrono::duration_cast<std::chrono::duration<long, std::ratio<86400>>>(t).count();
    r.SRandTime();
    Count = days % (Count - (Count / 8)) + r.Rand(Count / 8);

    // Load Table header:
    auto FileData2 = *(LoadCompleteFile(FullFilename(TabFilename, ExcelPath)));

    // Die erste Zeile einlesen
    FileP = 0;
    FileP = ReadLine(FileData2, FileP, Line.getData(), 300);

    bool bFirst = true;
    while (true) {
        if (FileP >= FileData2.AnzEntries()) {
            break;
        }
        FileP = ReadLine(FileData2, FileP, Line.getData(), 300);

        TeakStrRemoveEndingCodes(Line.getData(), "\xd\xa\x1a\r");

        Num = atoi(strtok(Line.getData(), TabSeparator));

        if (Num >= 1000000) {
            Count--;
            if (Count == 0 || bFirst) {
                bFirst = false;
                if (pPageAndWord != nullptr) {
                    *pPageAndWord = Num;
                }
                if (pRightWord != nullptr) {
                    *pRightWord = strtok(nullptr, TabSeparator);
                }
            }
        }
    }

    FileData.FillWith(0);
    FileData2.FillWith(0);
}
